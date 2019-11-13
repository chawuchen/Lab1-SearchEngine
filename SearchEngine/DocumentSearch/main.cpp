#include "../SearchEngine/Csv.h"
#include "../SearchEngine/Query.h"
#include "../SearchEngine/String_convert.h"
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
	} catch (const exception & exp) {
		cerr << exp.what() << endl;
		system("pause");
		return 1;
	}	// 销毁 v，保留 docs
	
	cout << "请输入 doc_id：";
	string s;
	while (cin >> s) {
		system("cls");
		cout << s << "\t";
		docs.print_doc_info(s);
		cout << endl << "请输入 doc_id：";
	}
	
}