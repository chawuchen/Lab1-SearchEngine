#include "../SearchEngine/Csv.h"
#include "../SearchEngine/Query.h"
#include "../SearchEngine/String_convert.h"
#include <iostream>
using namespace std;

constexpr auto QUERYS_FILE = "data_querys.csv";
constexpr auto DOCS_FILE = "data_docs.csv";
constexpr auto RESULT_FILE = "data_submission.csv";


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
		if (s == "all") break;
		else if (s == "q" || s == "quit" || s == "exit") return 0;
		system("cls");
		cout << s << "\t";
		docs.print_doc_info(s);
		cout << endl << "请输入 doc_id：";
	}
	if (cin.fail()) return 0;

	//auto total = docs.get_docs_num();
	//size_t num = 0;
	//ostringstream oss;
	//cin.get();
	//for (const string &name : docs.get_doc_names()) {
	//	system("cls");
	//	docs.print_doc_info(name);
	//	cout << ++num << " / " << total;
	//	cout << "\n请输入评分(0-4)，回车表示3，0表示删除这个文档，q表示结束: ";
	//	char c;
	//	int score;
	//again:	 cin.get(c);
	//	if (c == '\n') score = 3;
	//	else if (c == '0') {
	//		score = 0; cin.get();
	//	}
	//	else if (c == '1') {
	//		score = 1; cin.get();
	//	} else if (c == '2') {
	//		score = 2; cin.get();
	//	} else if (c == '3') {
	//		score = 3; cin.get();
	//	} else if (c == '4') {
	//		score = 4; cin.get();
	//	} else if (c == 'q') break;
	//	else {
	//		cout << "输入错误，请重新输入" << endl;
	//		cin.get();
	//		goto again;
	//	}
	//	oss << name << "," << score << "\n";
	//}
	//ofstream off("score.csv");
	//off << oss.str();
}