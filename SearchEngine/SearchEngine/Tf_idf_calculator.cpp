#include "Tf_idf_calculator.h"
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
 
std::unordered_set<std::string> Tf_idf_calculator::stop_words;

cppjieba::Jieba Tf_idf_calculator::jieba(DICT_PATH,
										 HMM_PATH,
										 USER_DICT_PATH,
										 IDF_PATH,
										 STOP_WORD_PATH);
 
void Tf_idf_calculator::set_stop_words(const std::string &stop_words_file) {
	// 不包括空白字符
	ifstream infile(stop_words_file);
	if (infile.fail()) throw runtime_error("cannot open file " + stop_words_file);
	istream_iterator<string> in(infile), eof;
	copy(in, eof, inserter(stop_words, stop_words.begin()));

	// 添加空白
	stop_words.insert({ " ", "\t", "\n", "\r", "" });

#ifdef DEBUG_PRINT
	cout << "\ttotal: " << stop_words.size() << " stop words" << endl;
#endif
}

void Tf_idf_calculator::calculate(const vector<vector<string>> &vec) {
	int n = vec.size();
	doc_names.resize(n);
	tf_idf.resize(n);

	// 统计 tf df
#ifdef DEBUG_PRINT
	cout << "\treading documents   0%%" << flush;
	int cnt_debug = 0;
	int percentage_debug = 0;
#endif
	#pragma omp parallel for
	for (int i = 0; i < n; ++i) {
		const string &doc_id = vec[i][0];
		const string &title = vec[i][2];
		const string &content = vec[i][3];

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
		jieba.CutForSearch(content, words_all);
		//cout << String_convert::utf8_to_string(limonp::Join
		//		(words_all.begin(), words_all.end(), "/")) << endl;	

		// 统计 tf df
		doc_names[i] = doc_id;
		for (const string &word : words_all) {
			if (is_not_stop_word(word)) {
				if (++tf_idf[i][word] == 1)	// 计算 tf
					#pragma omp critical
					++df[word];			// 计算 df
			}
		}	
#ifdef DEBUG_PRINT
		#pragma omp critical
		++cnt_debug;
		
		if (cnt_debug * 100 / n > percentage_debug) 
		#pragma omp critical
		{
			percentage_debug = cnt_debug * 100 / n;
			printf("\b\b\b\b%3d%%", percentage_debug);
			cout << flush;
		}
#endif
	}

#ifdef DEBUG_PRINT
	cout << "\n\tcalculating tfidf   0%%" << flush;
	cnt_debug = 0;
	percentage_debug = 0;
#endif
	// 计算 tf-idf
	#pragma omp parallel for
	for (int i = 0; i < n; ++i) {
		for (auto &pair : tf_idf[i]) {
			double _df = df[pair.first];
			double _tf = pair.second;
			pair.second = (1 + log(_tf)) * log(n / _df);
		}
#ifdef DEBUG_PRINT
#pragma omp critical
		++cnt_debug;

		if (cnt_debug * 100 / n > percentage_debug)
#pragma omp critical
		{
			percentage_debug = cnt_debug * 100 / n;
			printf("\b\b\b\b%3d%%", percentage_debug);
			cout << flush;
		}
#endif
	}
#ifdef DEBUG_PRINT
	cout << endl;
#endif
}
