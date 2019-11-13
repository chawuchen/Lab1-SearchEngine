#pragma once

#include "Documents.h"
#include "cppjieba/Jieba.hpp"
#include "Query_result.h"
#include <string>
#include <vector>

// 使用时不可释放 Documents 对象
class Query {
public:
	Query(const Documents &documents) : docs(documents) {}

	// str 为问题字符串，n 为需要多少个结果，返回文档 id
	Query_result query(const std::string &str, int n = 20) 
					{ query_str = str; return do_query(n); }
	Query_result query(std::string &&str, int n = 20)
					{ query_str = std::move(str); return do_query(n); }

private:
	Query_result do_query(int n);	// 进行查询工作，按顺序调用以下函数

	void clean_member();					// 清理成员，如果以前进行过查询
	void segment();							// 切分 query_str 设置 query_words
	void calculate_tfidf_sparse_vec();		// 根据 query_words 设置 query_tfidf_sparse_vec 查询向量
	void sparse_vec_to_full_storage();		// 将 query_tfidf_sparse_vec 转换为完整存储方便计算，并计算nrm2
	void calculate_query_docs_cos();		// 根据 query_tfidf_vec 和 docs 计算所有角度
	void sort_first_n_docs(int n);			// 排序前 n 篇相关文档

	double get_cos(const Documents::sparse_vector_type &vec1, 
				   const Documents::sparse_vector_type &vec2,
				   double *full_storage_zero_vec);	// 第三个参数为初始化为 0的临时空间，且能容纳任意 vec

	double get_cos(const Documents::sparse_vector_type &vec_doc,
				   double vec_doc_nrm2);

	const Documents &docs;
	int next_word_id_not_use = 0;
	std::string query_str;
	std::map<std::string, int> query_words;		// 词，频率 tf
	Documents::sparse_vector_type query_tfidf_sparse_vec;
	std::unique_ptr<double[]> query_tfidf_full_vec;
	double query_tfidf_full_vec_nrm2 = 0;
	std::vector<std::pair<int, double>> query_doc_cos;	// doc_id，doc_cos_value 的 pair
};

