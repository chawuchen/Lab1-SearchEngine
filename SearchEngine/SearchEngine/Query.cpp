#include "Query.h"
#include <mkl.h>
#include "String_convert.h"
using namespace std;

Query_result Query::do_query(int n) {
	clean_member();
	segment();
	calculate_tfidf_sparse_vec();
	sparse_vec_to_full_storage();
	calculate_query_docs_cos();
	calculate_title_similarity();
	calculate_doc_score();
	get_first_n_docs(n);
	//sort_first_n_docs(n);

	list<int> vec;
	auto ce = query_doc_score.begin() + min((size_t)n, query_doc_score.size());

	int cnt = 0;
	for (auto it = query_doc_score.cbegin(); it != ce; ++it) {
		vec.push_back(it->first);
		if (it->second < TFIDF_ERROR_THRESHOULD) ++cnt;
	}
	if (cnt >= (n >> 1)) {		// value 失效了，应该进行其他处理
		cout << "额外查找：" << String_convert::utf8_to_string(query_str) << endl;
		try_query_match(vec, n);
	}
	return Query_result(std::move(vec), docs);
}

void Query::clean_member() {
	query_words.clear();
	query_tfidf_sparse_vec.first.clear();
	query_tfidf_sparse_vec.second.clear();
	query_tfidf_full_vec.reset();
	query_doc_score.clear();
}

void Query::segment() {
	transform(query_str.begin(), query_str.end(), query_str.begin(), tolower);
	vector<string> words_all;
	
	docs.jieba.Cut(query_str, words_all);
	for_each(words_all.begin(), words_all.end(), [this](const string &word)
			 {
				 if (Documents::is_not_stop_word(word)) {
					 if (docs.synonym.find(word) != docs.synonym.end())
						 ++query_words[docs.synonym[word]];
					 else
						 ++query_words[word];
				 }

			 });
}

void Query::calculate_tfidf_sparse_vec() {
	const auto &vocabulary = docs.df;
	next_word_id_not_use = vocabulary.size();
	for (const auto &pair : query_words) {
		const string &word = pair.first;
		double tf = pair.second;

		double tfidf; int indx;
		auto it = vocabulary.find(word);
		if (it == vocabulary.cend()) {
			// 词汇表中没有查询中的词汇
			indx = next_word_id_not_use++;
			tfidf = (1 + log(tf)) * log(docs.get_docs_num());
		} else {
			// 词汇表中有查询中的词汇
			indx = it->second.first;
			tfidf = (1 + log(tf)) * log(docs.get_docs_num() / it->second.second);
		}

		query_tfidf_sparse_vec.first.push_back(indx);
		query_tfidf_sparse_vec.second.push_back(tfidf);
	}
	
}

void Query::sparse_vec_to_full_storage() {
	query_tfidf_full_vec.reset(new double[next_word_id_not_use]);
	memset(query_tfidf_full_vec.get(), 0, next_word_id_not_use);
	cblas_dsctr(query_tfidf_sparse_vec.first.size(), 
				&query_tfidf_sparse_vec.second[0], 
				&query_tfidf_sparse_vec.first[0], 
				query_tfidf_full_vec.get());
	query_tfidf_full_vec_nrm2 = cblas_dnrm2(query_tfidf_sparse_vec.second.size(), 
											&query_tfidf_sparse_vec.second[0], 
											1);
}

void Query::calculate_query_docs_cos() {
	int n = docs.get_docs_num();
	query_doc_cos.resize(n);
	#pragma omp parallel for
	for (int i = 0; i < n; ++i) {
		double cos_value = get_cos(docs.get_doc_sparse_tfidf_vec(i),
								   docs.get_doc_sparse_tfidf_vec_nrm2(i));
		query_doc_cos[i] = cos_value;
	}
}

void Query::calculate_title_similarity() {
	int n = docs.get_docs_num();
	query_title_set_similarity.resize(n);
	#pragma omp parallel for
	for (int i = 0; i < n; ++i) {
		int mm = 0;
		for (const auto &p : query_words)
			if (docs.title_word_set[i].find(p.first) != docs.title_word_set[i].end())
				++mm;
		query_title_set_similarity[i] = (double)mm / query_words.size();
	}
}

void Query::calculate_doc_score() {
	int n = docs.get_docs_num();
	query_doc_score.resize(n);
	doc_sort_score.resize(n);
	double factor = 0.037, k = 1;

	#pragma omp parallel for
	for (int i = 0; i < n; ++i) {
		double score;
		if (docs.learn.find(query_id) == docs.learn.end() ||
			docs.learn[query_id].find(docs.doc_names[i]) == docs.learn[query_id].end()) {
			score = query_doc_cos[i] + factor * log(docs.doc_contents[i].size()) +
					TITLE_SIMILARITY_FACTOR * query_title_set_similarity[i];
		} else
			score = 0;
		query_doc_score[i] = { i, score };
		doc_sort_score[i] = query_doc_cos[i]; // not use
	}
}

void Query::get_first_n_docs(int n) {
	using ty = decltype(query_doc_score)::const_reference;
	auto sort_func = [](ty a, ty b) { return a.second > b.second; };
	partial_sort(query_doc_score.begin(), query_doc_score.begin() + n, query_doc_score.end(), sort_func);
}

void Query::sort_first_n_docs(int n) {
	using ty = decltype(query_doc_score)::const_reference;
	auto sort_func = [this](ty a, ty b) { return doc_sort_score[a.first] > doc_sort_score[b.first]; };
	sort(query_doc_score.begin(), query_doc_score.begin() + n, sort_func);
}

double Query::get_cos(const Documents::sparse_vector_type &vec1,
					  const Documents::sparse_vector_type &vec2,
					  double *full_storage_zero_vec) {

	cblas_dsctr(vec1.first.size(), &vec1.second[0], &vec1.first[0], full_storage_zero_vec);
	double dot = cblas_ddoti(vec2.first.size(), &vec2.second[0], &vec2.first[0], full_storage_zero_vec);
	double vec1_nrm2 = cblas_dnrm2(vec1.second.size(), &vec1.second[0], 1);
	double vec2_nrm2 = cblas_dnrm2(vec2.second.size(), &vec2.second[0], 1);
	double cos_value = dot / (vec1_nrm2 * vec2_nrm2);
	return cos_value;
}

double Query::get_cos(const Documents::sparse_vector_type &vec_doc, 
					  double vec_doc_nrm2) {
	double dot = cblas_ddoti(vec_doc.first.size(), &vec_doc.second[0], 
							 &vec_doc.first[0], query_tfidf_full_vec.get());
	return dot / (vec_doc_nrm2 * query_tfidf_full_vec_nrm2);
}

void Query::try_query_match(list<int> &rank_vec, int nn) {
	string str1, str2;
	auto pos = query_str.find('+');
	if (pos != string::npos) {
		cout << "\t发现分隔符，分割查找" << endl;
		str1 = query_str.substr(0, pos);
		str2 = query_str.substr(pos + 1);
	} else {
		str1 = query_str;
	}
	int n = docs.get_docs_num();
	vector<pair<double, int>> v;
	#pragma omp parallel for
	for (int i = 0; i < n; ++i) {
		if (docs.doc_titles[i].find(str1) != string::npos &&
			(str2.empty() || docs.doc_titles[i].find(str2) != string::npos) ||
			docs.doc_contents[i].find(str1) != string::npos &&
			(str2.empty() || docs.doc_contents[i].find(str2) != string::npos)) {
				#pragma omp critical
				v.emplace_back(-query_doc_score[i].second, i);
		}
	}
	sort(v.begin(), v.end());
	if (v.size() >= nn) {
		rank_vec.clear();
		for (int i = 0; i < nn; ++i) rank_vec.push_back(v[i].second);
	} else {
		list<int> tmp;
		for (const auto &p : v) tmp.push_back(p.second);
		auto it = rank_vec.begin();
		for (int i = v.size(); i < nn; ++i) {
			while (find(tmp.begin(), tmp.end(), *it) != tmp.end()) ++it;
			tmp.push_back(*it);
		}
		rank_vec = std::move(tmp);
	}
}

constexpr double Query::TITLE_SIMILARITY_FACTOR;
