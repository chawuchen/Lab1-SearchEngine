#include "Query_result.h"
#include "String_convert.h"
#include "Csv.h"

std::ostream &operator<<(std::ostream &os, const Query_result &result) {
	for (int i : result.docs_idx) {
		os << result.docs.doc_names[i] << "   \t"
			<< String_convert::utf8_to_string(result.docs.doc_titles[i]) << "\n";
	}
	os << std::flush;
	return os;
}

std::ostream &Query_result::print_csv(std::ostream &os, const std::string &query_id) {
	//if (std::set<int>(docs_idx.begin(), docs_idx.end()).size() != 20) {
	//	std::cerr << "err" << std::endl; std::cin.get();
	//}
	for (int id : docs_idx) {
		if (docs.learn.find(docs.doc_names[id]) == docs.learn.end())
			os << query_id << "," << docs.doc_names[id] << "\n";
		else
			os << query_id << "," << docs.learn[docs.doc_names[id]] << "\n";
	}
	return os;
}
