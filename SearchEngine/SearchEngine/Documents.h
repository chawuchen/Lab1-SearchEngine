#pragma once

#include "cppjieba/Jieba.hpp"
#include <vector>
#include <string>
#include <stdexcept>
#include <unordered_set>
#include <map>

class Documents {
	friend class Query;
	friend class Query_result;
	friend std::ostream &operator<<(std::ostream &os, const Query_result &result);
public:
	using sparse_vector_type = std::pair<std::vector<int>,		// indx[]，index 数组，与 mkl sparse BLAS 一致
										 std::vector<double>>;	// x[]，值数组，与 mkl sparse BLAS 一致

	static void set_stop_words(const std::string &stop_words_file);
	static void set_synonym(const std::string &synonym_file);
	static void set_error_words(const std::string err_file);
	static void set_learn(const std::string learn_file);
	void calculate(std::vector<std::vector<std::string>> &&doc_vec);	// 请使用右值参数节省内存
	size_t get_docs_num() const { return doc_names.size(); }
	const sparse_vector_type &get_doc_sparse_tfidf_vec(int doc_id) const { return tf_idf[doc_id]; }
	double get_doc_sparse_tfidf_vec_nrm2(int doc_id) const { return nrm2[doc_id]; }
	std::ostream &print_doc_info(const std::string &doc_id, int n = 12, std::ostream &os = std::cout);
	const std::vector<std::string> &get_doc_names() const { return doc_names; }
private:
	static constexpr int TITLE_FACTOR = 70;
	static constexpr int URL_FACTOR = 3;
	static std::unordered_set<std::string> stop_words;
	static std::map<std::string, std::string> synonym;
	static std::map<std::string, std::set<std::string>> learn;
	static std::set<std::string> error_words;			// 查询中可能出错的关键词
	static cppjieba::Jieba jieba;
	std::vector<std::string> doc_names;					// 文档编号 -> 文档名，用 int 索引提高并行度
	std::vector<std::string> doc_titles;				// 文档编号 -> 文档名，用 int 索引提高并行度
	std::vector<std::string> doc_urls;					// 文档编号 -> url
	std::vector<std::string> doc_contents;				// 文档编号 -> 文档名，用 int 索引提高并行度
	std::map<std::string, std::pair<int, size_t>> df;	// {词语，{词语编号，文档频率}}，也提供词汇表功能
	std::vector<std::map<std::string, double>> tf;		// 文档编号 ->{词语1，tf1}{词语2，tf2}...
	std::vector<sparse_vector_type> tf_idf;				// 文档编号 -> tf-idf 稀疏向量
	std::vector<double> nrm2;							// 文档编号 -> 向量2范数
	std::vector<std::set<std::string>> title_word_set;	// 文档编号 -> 文档标题的词集

	static bool is_not_stop_word(const std::string &s) 
			{ return stop_words.find(s) == stop_words.cend(); }

	static std::vector<std::string> get_segmentation(const std::string &sentence);
	static std::string &remove_useless(std::string &str);
};

