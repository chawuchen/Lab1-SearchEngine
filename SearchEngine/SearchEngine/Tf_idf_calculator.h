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
	using sparse_vector_type = std::pair<std::vector<int>,		// indx[]��index ���飬�� mkl sparse BLAS һ��
										 std::vector<double>>;	// x[]��ֵ���飬�� mkl sparse BLAS һ��

	static std::unordered_set<std::string> stop_words;
	static cppjieba::Jieba jieba;
	std::vector<std::string> doc_names;					// �ĵ���� -> �ĵ������� int ������߲��ж�
	std::map<std::string, std::pair<int, size_t>> df;	// {���{�����ţ��ĵ�Ƶ��}}��Ҳ�ṩ�ʻ����
	std::vector<std::map<std::string, double>> tf;		// �ĵ���� ->{����1��tf1}{����2��tf2}...
	std::vector<sparse_vector_type> tf_idf;				// �ĵ���� -> tf-idf ϡ������
	

	bool is_not_stop_word(const std::string &s) const 
			{ return stop_words.find(s) == stop_words.cend(); }

	static std::vector<std::string> get_segmentation(const std::string &sentence);
};

