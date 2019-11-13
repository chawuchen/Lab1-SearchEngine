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
	for (int id : docs_idx) {
		os << query_id << "," << docs.doc_names[id] << "\n";
	}
	return os;
}
