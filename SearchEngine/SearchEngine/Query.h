#pragma once

#include "Documents.h"
#include "cppjieba/Jieba.hpp"
#include "Query_result.h"
#include <string>
#include <vector>

// ʹ��ʱ�����ͷ� Documents ����
class Query {
public:
	Query(const Documents &documents) : docs(documents) {}

	// str Ϊ�����ַ�����n Ϊ��Ҫ���ٸ�����������ĵ� id
	Query_result query(const std::string &str, int n = 20) 
					{ query_str = str; return do_query(n); }
	Query_result query(std::string &&str, int n = 20)
					{ query_str = std::move(str); return do_query(n); }

private:
	static constexpr double TITLE_SIMILARITY_FACTOR = 1;		// tfidf��cosֵϵ��Ϊ1��������������
	static constexpr double TFIDF_ERROR_THRESHOULD = 0.55;		// value �������ֵ�ĳ��� n/2 �������������

	Query_result do_query(int n);	// ���в�ѯ��������˳��������º���

	void clean_member();					// �����Ա�������ǰ���й���ѯ
	void segment();							// �з� query_str ���� query_words
	void calculate_tfidf_sparse_vec();		// ���� query_words ���� query_tfidf_sparse_vec ��ѯ����
	void sparse_vec_to_full_storage();		// �� query_tfidf_sparse_vec ת��Ϊ�����洢������㣬������nrm2
	void calculate_query_docs_cos();		// ���� query_tfidf_vec �� docs �������нǶ�
	void calculate_title_similarity();		// ���ݼ��Ͻ����ж����ƶ�
	void calculate_doc_score();				// �������ϸ��ֵ÷ּ����ۺϵ÷�
	void sort_first_n_docs(int n);			// ����ǰ n ƪ����ĵ�

	double get_cos(const Documents::sparse_vector_type &vec1, 
				   const Documents::sparse_vector_type &vec2,
				   double *full_storage_zero_vec);	// ����������Ϊ��ʼ��Ϊ 0����ʱ�ռ䣬������������ vec

	double get_cos(const Documents::sparse_vector_type &vec_doc,
				   double vec_doc_nrm2);

	const Documents &docs;
	int next_word_id_not_use = 0;
	std::string query_str;
	std::map<std::string, int> query_words;		// �ʣ�Ƶ�� tf
	Documents::sparse_vector_type query_tfidf_sparse_vec;
	std::unique_ptr<double[]> query_tfidf_full_vec;
	double query_tfidf_full_vec_nrm2 = 0;
	std::vector<double> query_doc_cos;		// doc_id��doc_cos_value �� pair
	std::vector<double> query_title_set_similarity;	// doc_id��doc_cos_value �� pair
	std::vector<std::pair<int, double>> query_doc_score;	// doc_id���÷ֵ� pair

	// ���⴦��
	void try_query_match(std::list<int> &rank_vec, int nn);
};

