#pragma once

#include "cppjieba/Jieba.hpp"
#include <vector>
#include <string>
#include <stdexcept>
#include <unordered_set>
#include <map>

class Tf_idf_calculator {
public:
	static void set_stop_words(const std::string &stop_words_file);
	void calculate(const std::vector<std::vector<std::string>> &vec);

private:
	using sparse_vector_type = std::pair<std::vector<int>,		// indx[]，index 数组，与 mkl sparse BLAS 一致
										 std::vector<double>>;	// x[]，值数组，与 mkl sparse BLAS 一致

	static std::unordered_set<std::string> stop_words;
	static cppjieba::Jieba jieba;
	std::vector<std::string> doc_names;					// 文档编号 -> 文档名，用 int 索引提高并行度
	std::map<std::string, std::pair<int, size_t>> df;	// {词语，{词语编号，文档频率}}，也提供词汇表功能
	std::vector<std::map<std::string, double>> tf;		// 文档编号 ->{词语1，tf1}{词语2，tf2}...
	std::vector<sparse_vector_type> tf_idf;				// 文档编号 -> tf-idf 稀疏向量
	

	bool is_not_stop_word(const std::string &s) const 
			{ return stop_words.find(s) == stop_words.cend(); }

	static std::vector<std::string> get_segmentation(const std::string &sentence);
};

