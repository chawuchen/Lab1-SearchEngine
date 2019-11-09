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
	std::map<std::string, size_t> df;	// {����ĵ�Ƶ��}
	std::vector<std::map<std::string, double>> tf_idf; // int ->{����1��ֵ1}{����2��ֵ2}...
	std::vector<std::string> doc_names;	// int -> doc_name���� int ������߲��ж�
	

	bool is_not_stop_word(const std::string &s) const 
			{ return stop_words.find(s) == stop_words.cend(); }
};

