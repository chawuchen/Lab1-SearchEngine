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
	static std::unordered_set<std::string> stop_words;
	static cppjieba::Jieba jieba;
	std::map<std::string, size_t> df;	// {词语，文档频率}
	std::vector<std::map<std::string, double>> tf_idf; // int ->{词语1，值1}{词语2，值2}...
	std::vector<std::string> doc_names;	// int -> doc_name，用 int 索引提高并行度
	

	bool is_not_stop_word(const std::string &s) const 
			{ return stop_words.find(s) == stop_words.cend(); }
};

