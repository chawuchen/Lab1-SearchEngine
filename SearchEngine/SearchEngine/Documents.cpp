#include "Documents.h"
#include "String_convert.h"
#include <algorithm>
#include <omp.h>
#include <functional>
#include <iostream>
#define DEBUG_PRINT

using namespace std;

 
constexpr char *const DICT_PATH = "../dict/jieba.dict.utf8";
constexpr char *const HMM_PATH = "../dict/hmm_model.utf8";
constexpr char *const USER_DICT_PATH = "../dict/user.dict.utf8";
constexpr char *const IDF_PATH = "../dict/idf.utf8";
constexpr char *const STOP_WORD_PATH = "../dict/stop_words.utf8";
 
constexpr int Documents::TITLE_FACTOR;

constexpr int Documents::URL_FACTOR;

std::unordered_set<std::string> Documents::stop_words;

std::map<std::string, std::string> Documents::synonym;

std::map<std::string, std::set<std::string>> Documents::learn;

std::set<std::string> Documents::error_words;

cppjieba::Jieba Documents::jieba(DICT_PATH,
								 HMM_PATH,
								 USER_DICT_PATH,
								 IDF_PATH,
								 STOP_WORD_PATH);
 
void Documents::set_stop_words(const std::string &stop_words_file) {
	// 不包括空白字符
	ifstream infile(stop_words_file);
	if (infile.fail()) 
		throw runtime_error("cannot open file " + stop_words_file);
	istream_iterator<string> in(infile), eof;
	copy(in, eof, inserter(stop_words, stop_words.begin()));

	// 添加空白
	stop_words.insert({ " ", "\t", "\n", "\r", "" });

#ifdef DEBUG_PRINT
	std::cout << "\ttotal: " << stop_words.size() 
			  << " stop words" << endl;
#endif
}

void Documents::set_synonym(const std::string &synonym_file) {
	ifstream infile(synonym_file);
	if (infile.fail())
		throw runtime_error("cannot open file " + synonym_file);
	string line;
	while (getline(infile, line)) {
		istringstream iss(line);
		string s, w;
		iss >> s;
		while (iss >> w) synonym[std::move(w)] = s;
	}
}

void Documents::set_error_words(const std::string &err_file) {
	ifstream infile(err_file);
	if (infile.fail())
		throw runtime_error("cannot open file " + err_file);
	istream_iterator<string> it(infile), eof;
	copy(it, eof, inserter(error_words, error_words.begin()));
}

void Documents::set_learn(const std::string &learn_file) {
	ifstream infile(learn_file);
	if (infile.fail())
		throw runtime_error("cannot open file " + learn_file);
	string line;
	while (getline(infile, line)) {
		istringstream iss(line);
		string s, w;
		iss >> s;
		while (iss >> w) learn[s].insert(std::move(w));
	}
}

vector<string> Documents::get_segmentation(const string &sentence) {
	// 使用 jieba 提取关键词
		/*const size_t topk = 20;
		vector<cppjieba::KeywordExtractor::Word> keywordres;
		jieba.extractor.Extract(content, keywordres, topk);
		ofstream of("out.txt");
		of << content << endl;
		of << keywordres << endl;
		of.close();
		cin.get(); continue;*/

		// 提取所有可能的分词
	vector<string> words_all;
	jieba.CutForSearch(sentence, words_all);
	//cout << String_convert::utf8_to_string(limonp::Join
	//		(words_content.begin(), words_content.end(), "/")) << endl;	

	// 处理近义词
	for_each(words_all.begin(), words_all.end(), [](string &s) {
		if (synonym.find(s) != synonym.end()) {
			/*cout << String_convert::utf8_to_string(s) << " -> "
				<< String_convert::utf8_to_string(synonym[s]) << endl;*/
			s = synonym[s];
		}
			 });
	return words_all;
}

string &Documents::remove_useless(string &str) {
	istringstream iss(String_convert::utf8_to_string(str));
	string w, ret;
	while (iss >> w) {
		if (w.size() > 20) {
			ret += " " + w;
			//cout << "add " << String_convert::utf8_to_string(w) << endl;
		} else {
			//cout << "remove " << String_convert::utf8_to_string(w) << endl;
		}
	}
	return str = String_convert::string_to_utf8(ret);
}

void Documents::calculate(vector<vector<string>> &&vec) {
	auto strlower = [](string &s) -> string &
		{ transform(s.begin(), s.end(), s.begin(), tolower); return s; };
	int n = vec.size();
	tf.resize(n);
	tf_idf.resize(n);
	nrm2.resize(n);
	title_word_set.resize(n);
	doc_names.resize(n);
	doc_titles.resize(n);
	doc_urls.resize(n);
	doc_contents.resize(n);

	// 统计 tf df
#ifdef DEBUG_PRINT
	std::cout << "\treading documents   0%%" << flush;
	int cnt_debug = 0;
	int percentage_debug = 0;
	double begin_time = omp_get_wtime();
#endif
	#pragma omp parallel for
	for (int i = 0; i < n; ++i) {
		if (error_words.find(vec[i][0]) != error_words.end()) {
			vec[i][1] = "";
			vec[i][2] = "";
			vec[i][3] = "";
		}
		const string &doc_id = strlower(vec[i][0]);
		const string &url = strlower(vec[i][1]);
		const string &title = strlower(vec[i][2]);
		string &content = strlower(vec[i][3]);
		vector<string> words_content = get_segmentation(content);
		vector<string> words_title = get_segmentation(title);
		vector<string> words_url = get_segmentation(url);

		// 统计 tf df
		for (const string &word : words_content) {
			if (is_not_stop_word(word)) {
				if (++tf[i][word] == 1)		// 计算 tf
					#pragma omp critical
					++df[word].second;		// 计算 df
			}
		}	
		for (const string &word : words_title) {
			if (is_not_stop_word(word)) {
				if (tf[i][word] == 0)		// 计算 tf
					#pragma omp critical
					++df[word].second;		// 计算 df
				tf[i][word] += TITLE_FACTOR * content.size() / title.size();
			}
		}
		for (const string &word : words_url) {
			if (word.size() > 2 && is_not_stop_word(word)) {
				if (tf[i][word] == 0)		// 计算 tf
					#pragma omp critical
					++df[word].second;		// 计算 df
				tf[i][word] += URL_FACTOR * content.size() / url.size();
			}
		}
		copy(make_move_iterator(words_title.begin()), 
			 make_move_iterator(words_title.end()), 
			 inserter(title_word_set[i], title_word_set[i].begin()));

		// 记录文档信息，不需要的可以注释
		doc_names[i] = std::move(vec[i][0]);
		doc_titles[i] = std::move(vec[i][2]);
		doc_urls[i] = std::move(vec[i][1]);
		doc_contents[i] = std::move(vec[i][3]);
#ifdef DEBUG_PRINT
		#pragma omp critical
		++cnt_debug;
		
		if (cnt_debug * 100 / n > percentage_debug) 
		#pragma omp critical
		{
			percentage_debug = cnt_debug * 100 / n;
			printf("\b\b\b\b%3d%%", percentage_debug);
			std::cout << flush;
		}
#endif
	}

#ifdef DEBUG_PRINT
	std::cout << "   time: " 
		      << omp_get_wtime() - begin_time << " s\n"
			  << "\tcalculating tfidf   0%%" << flush;
	cnt_debug = 0;
	percentage_debug = 0;
	begin_time = omp_get_wtime();
#endif
	// 计算词语编号
	int word_cnt = 0;
	for (auto &pair : df) pair.second.first = word_cnt++;

	// 计算 tf-idf
	#pragma omp parallel for
	for (int i = 0; i < n; ++i) {
		vector<int> &indx = tf_idf[i].first;
		vector<double> &x = tf_idf[i].second;
		double dsum = 0;
		for (auto &pair : tf[i]) {
			const auto &p = df[pair.first];
			int word_idx = p.first;
			double word_df = p.second;
			double word_tf = pair.second;
			double word_tf_idf = (1 + log(word_tf)) * log(n / word_df);
			indx.push_back(word_idx);
			x.push_back(word_tf_idf);
			dsum += word_tf_idf * word_tf_idf;
		}
		nrm2[i] = sqrt(dsum);
#ifdef DEBUG_PRINT
#pragma omp critical
		++cnt_debug;

		if (cnt_debug * 100 / n > percentage_debug)
#pragma omp critical
		{
			percentage_debug = cnt_debug * 100 / n;
			printf("\b\b\b\b%3d%%", percentage_debug);
			std::cout << flush;
		}
#endif
	}
#ifdef DEBUG_PRINT
	std::cout << "   time: " 
			  << omp_get_wtime() - begin_time << " s\n";
#endif
}

ostream &Documents::print_doc_info(const string &doc_id, int n /*= 12*/, ostream &os /*= std::cout*/) {
	auto it = find(doc_names.begin(), doc_names.end(), doc_id);
	if (it == doc_names.end()) {
		os << "cannot find " << doc_id << endl;
	} else {
		int idx = it - doc_names.begin();
		os << "url: " << doc_urls[idx] << "\n"
			<< String_convert::utf8_to_string(doc_titles[idx]) << "\n"
			<< String_convert::utf8_to_string(doc_contents[idx]) << "\n"
			<< "-----------------------------------------------------------\n"
			<< "tf-idf 信息：\n";
		
		const auto &indx = tf_idf[idx].first;
		const auto &x = tf_idf[idx].second;
		vector<pair<double, int>> v;
		for (int i = 0; i < x.size(); ++i) {
			v.emplace_back(-x[i], indx[i]);
		}
		partial_sort(v.begin(), v.begin() + n, v.end());

		auto print_word_info = [&](const pair<double, int> &p)
		{
			string word; int dfv = -1;
			for (const auto &dfp : df)
				if (dfp.second.first == p.second)
					{ word = dfp.first; dfv = dfp.second.second; }
			os	<< String_convert::utf8_to_string(word) << "\ttf-idf: " << -p.first 
				<< "\t\ttf: " << tf[idx][word] << "\t\tdf: " << dfv << "\n";
		};

		for_each(v.begin(), v.begin() + n, print_word_info);
	}
	os << flush;
	return os;
}
