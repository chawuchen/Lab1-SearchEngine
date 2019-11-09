#include "Csv.h"
#include "Tf_idf_calculator.h"
#include <iostream>
using namespace std;

int main() {
	vector<vector<string>> v;
	try {
		cout << "reading csv ..." << endl;
		Csv::read("a.csv", 4, v);

		Tf_idf_calculator t;
		cout << "setting stop words ..." << endl;
		t.set_stop_words("../dict/stop_words.utf8");

		cout << "calculating tf-idf ..." << endl;
		t.calculate(v);
		//Csv::write("b.csv", v);
	} catch (const exception &exp) {
		cerr << exp.what() << endl; 
	} 
}