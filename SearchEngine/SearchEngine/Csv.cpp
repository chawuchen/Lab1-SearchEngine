#include "Csv.h"
#include <fstream>
#include <omp.h>

using namespace std;

void Csv::read(const string &file_name, int cols, vector<vector<string>> &vec) {
	ifstream infile(file_name);
	if (infile.fail()) throw runtime_error("cannot open file " + file_name);

	string s;
	vector<string> lines;
	while (getline(infile, s)) lines.push_back(std::move(s));
	int n = lines.size();

	#pragma omp parallel for
	for (int i = 0; i < n; ++i) {
		const string &line = lines[i];
		vector<string> v;
		auto line_begin = line.cbegin(), line_end = line.cend(), b = line_begin, e = line_begin;
		for (int j = 1; j < cols; ++j) {	// cols - 1 次
			e = find(b, line_end, ',');
			v.emplace_back(b, e);
			b = e + 1;
		}
		v.emplace_back(b, line_end);		// 最后一项
		#pragma omp critical
		vec.push_back(std::move(v));
	}
}

void Csv::write(const string &file_name, const vector<vector<string>> &vec) {
	ofstream outfile(file_name);
	if (outfile.fail()) throw runtime_error("cannot open file " + file_name);

	for (const auto &line_vec : vec) {
		auto it = line_vec.cbegin(), e = line_vec.cend();
		while (it != e) {
			outfile << *it;
			outfile << (++it == e ? "\n" : ",");
		}
	}
}
