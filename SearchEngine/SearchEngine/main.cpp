#include "Csv.h"
#include "Query.h"
#include "String_convert.h"
#include <iostream>
using namespace std;

constexpr auto QUERYS_FILE = "data_querys.csv";
constexpr auto DOCS_FILE = "data_docs.csv";
constexpr auto RESULT_FILE = "submission.csv";


//#define TEST
int main() {
	Documents docs;
	try {
		cout << "reading csv ..." << endl;
		vector<vector<string>> v;
		Csv::read(DOCS_FILE, 4, v);

		cout << "setting stop words ..." << endl;
		docs.set_stop_words("../dict/stop_words.utf8");

		cout << "calculating tf-idf ..." << endl;
		docs.calculate(std::move(v));
	} catch (const exception &exp) {
		cerr << exp.what() << endl; 
		system("pause");
		return 1;
	}	// 销毁 v，保留 docs
#ifdef TEST
	while (1) {
		Query q(docs);
		system("cls");
		cout << "请输入查询内容：" << endl;
		string str;
		getline(cin, str);
		str = String_convert::string_to_utf8(str);
		auto result = q.query(str);
		cout << result << endl;
		system("pause");
	}
#else
	try {
		cout << "reading query csv ..." << endl;
		vector<vector<string>> v;
		Csv::read(QUERYS_FILE, 2, v);

		cout << "querying ..." << endl;
		Query q(docs);
		ostringstream oss;
		for (const auto &vec : v) {
			string query_id = vec[1].substr(vec[1].find('q'));
			q.query(vec[0]).print_csv(oss, std::move(query_id));
		}
		
		cout << "writing result" << endl;
		ofstream outfile(RESULT_FILE);
		if (outfile.fail()) throw runtime_error("cannot open file "s + RESULT_FILE);
		outfile << u8"query_id,doc_id" << endl;
		outfile << oss.str();
	} catch (const exception &exp) {
		cout << exp.what() << endl; 
		system("pause");
		return 1;
	} 	
#endif
}