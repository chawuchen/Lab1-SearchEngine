#pragma once

#include "Documents.h"
#include <vector>
#include <string>

// 使用时不可释放 Documents 对象
class Query_result {
	friend class Query;
	friend std::ostream &operator<<(std::ostream &os, const Query_result &result);
public:
	std::ostream &print_csv(std::ostream &os, const std::string &query_id);
private:
	Query_result(std::vector<int> &&vec, const Documents &docs) : 
				docs_idx(std::move(vec)), docs(docs) {}
	const Documents &docs;
	std::vector<int> docs_idx;
};

std::ostream &operator<<(std::ostream &os, const Query_result &result);