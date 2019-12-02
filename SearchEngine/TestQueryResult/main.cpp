#include "../SearchEngine/Csv.h"
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <array>
#include <iterator>
#include "../SearchEngine/String_convert.h"
using namespace std;

bool cmp(const string &s1, const string &s2) {
	return stoi(s1.substr(1)) < stoi(s2.substr(1));
}

int main() {
	vector<vector<string>> vec;
	Csv::read("data_compare.csv", 4, vec);
	map<string, pair<string, set<string>>, decltype(cmp) *> m(cmp);
	map<string, pair<string, map<string, int>>, decltype(cmp) *> ms(cmp);
	map<string /*docid*/, set<string /*queryname*/>> dq;
	for (auto &v : vec) {
		dq[v[1]].insert(v[3]);
		ms[v[0]].first = v[3];
		ms[v[0]].second[v[1]] = stoi(v[2]);	// stoi(v[2]) 相关值
		m[v[0]].first = std::move(v[3]);
		m[v[0]].second.emplace(std::move(v[1]));
	}

	vec.clear();
	Csv::read("data_submission.csv", 2, vec);
	map<string, set<string>, decltype(cmp) *> q(cmp);
	for (auto &v : vec) {
		q[v[0]].insert(std::move(v[1]));
	}

	vec.clear();
	try {
		Csv::read("submission.csv", 2, vec);
	} catch (const exception &exp) {
		cout << exp.what() << endl; 
	} 
	map<string, set<string>, decltype(cmp) *> sub(cmp);
	map<string, set<string>, decltype(cmp) *> sub2(cmp);
	for (auto &v : vec) {
		sub[v[0]].insert(v[1]);
		sub2[v[1]].insert(std::move(v[0]));
	}

	vec.clear();
	try {
		Csv::read("test_querys.csv", 2, vec);
	} catch (const exception & exp) {
		cout << exp.what() << endl;
	}
	map<string, string> testquery;
	for (auto &v : vec) {
		testquery[v[1]] = std::move(v[0]);
	}

	vec.clear();
	vec.shrink_to_fit();

	int err_num = 0;
	int hit_num = 0;
	int tsc = 0;
	map<string, vector<string>> errm;
	map<string, vector<string>> shouldm;

	set<string> errf;
	ifstream inf("errfiles.list");
	if(inf){
		istream_iterator<string> it(inf), eof;
		copy(it, eof, inserter(errf, errf.begin()));
	}

	ostringstream oss;
	for (const auto &p : q) {
		const auto &my_doc_set_20 = p.second;
		const auto &all_doc_set = m[p.first].second;
		vector<string> errorvec;
		set_difference(my_doc_set_20.begin(), my_doc_set_20.end(), all_doc_set.begin(),
					   all_doc_set.end(), back_inserter(errorvec));
		
		int errs = errorvec.size();
		err_num += errorvec.size();
		hit_num += (int)my_doc_set_20.size() - errs;
		if (inf && errf.find(m[p.first].first) != errf.end())cout << "----";
		//if ((int)my_doc_set_20.size() != 20) {
		//	printf("查询 %4s 错误！！！\n", p.first.c_str()); cin.get();
		//}
		printf("查询 %4s 错误率 %2d/20\t%-20s  %s\n", p.first.c_str(), errs, 
			   string(errs, '*').c_str(), String_convert::utf8_to_string(m[p.first].first).c_str());
		
		//cout << "错误搜出来的文档有：\n";
		//for_each(errorvec.begin(), errorvec.end(), [](const auto &s) {cout << s << endl; });
		errm[p.first] = std::move(errorvec);
		set_difference(all_doc_set.begin(), all_doc_set.end(), my_doc_set_20.begin(),
					   my_doc_set_20.end(), back_inserter(shouldm[p.first]));

		for (const auto &s : errm[p.first]) {
			if (sub2.find(s) != sub2.end()) {
				oss << "query " << m[p.first].first << " has error doc " << s
					<< ", which occurs in submission.csv in querys: ";
				for (const auto ss : sub2[s]) {
					oss << testquery[ss] << "(" << ss << "), ";
				}
				oss << " right files are: ";
				for (const auto ss : shouldm[p.first]) {
					if (sub2.find(ss) != sub2.end())
						oss << ss << "(" << ms[p.first].second[ss] << "),";
				}
				oss << "\n";
			}
		}

		int tsci = 1;
		for (const auto &s : my_doc_set_20) {
			if (ms[p.first].second.find(s) != ms[p.first].second.end()) {
				if (tsci == 1)tsc += ms[p.first].second[s];
				else tsc += ms[p.first].second[s] / log2(tsci);
				tsci++;
				if (ms[p.first].second[s] == 3 && sub.find(s) != sub.end()) {
					oss << "query " << m[p.first].first << " has score-" << ms[p.first].second[s]
						<< " doc " << s << ", which occurs in submission.csv in querys: ";
					for (const auto ss : sub[s]) {
						oss << testquery[ss] << "(" << ss << "), ";
					}
					oss << " right files are: ";
					for (const auto ss : shouldm[p.first]) {
						if (sub.find(ss) != sub.end())
							oss << ss << "(" << ms[p.first].second[ss] << "),";
					}
					oss << "\n";
				}
			}
		}
	}

	for (auto &p : sub) {
		auto &query_id = p.first;
		for (auto &doc_id : p.second) {
			if (dq.find(doc_id) != dq.end()) {
				oss << "query " << testquery[query_id] << "(" << query_id << ") doc " << doc_id
					<< " occurs in: ";
				for (auto &query_name : dq[doc_id]) {
					oss << query_name << " ";
				}
				oss << "\n";
			}
		}
	}
	ofstream ofile("may_be_error.txt");
	ofile << oss.str();
	ofile.close();
#if 0
	/*ofstream off("learn.utf8");
	off.close();*/
	ifstream iff("okquery.list");
	set<string> oklist;
	istream_iterator<string> iit(iff), eof;
	copy(iit, eof, inserter(oklist, oklist.begin()));
	iff.close();
	try {
		Csv::read("test_docs.csv", 4, vec);
	} catch (const exception & exp) {
		cout << exp.what() << endl;
	}
	map<string, string> docid_title;
	for (auto &v : vec) {
		docid_title[std::move(v[0])] = std::move(v[2]);
	}
	int qn = -1;
	array<string, 20> arr;
	for (auto &p : sub) {
		system("cls");
		++qn;
		auto &query_id = p.first;
		if (oklist.find(query_id) != oklist.end()) continue;
		cout << qn << " " << String_convert::utf8_to_string(testquery[query_id]) << "(" << query_id << ")\n\n";
		int cnt = 0;
		for (auto &doc_id : p.second) {
			cout << cnt << " " << String_convert::utf8_to_string(docid_title[doc_id]) << "\n";
			arr[cnt++] = doc_id;
		}
		cout << "\n请输入错误的编号，空格隔开，如果认为本页没问题，请输入20，以后不会再出现：" << endl;
		string line, w;
		getline(cin, line);
		istringstream iss(line);
		ostringstream oss;
		oss << query_id << " ";
		bool haserr = false;
		while (iss >> w) {
			if (w == "20") {
				ofstream off("okquery.list", ofstream::app);
				off << query_id << "\n";
				off.close();
				break;
			}
			haserr = true;
			oss << arr[stoi(w)] << ' ';
		}
		oss << "\n";
		if (haserr) {
			ofstream off("learn.utf8", ofstream::app);
			off << oss.str();
			off.close();
		}
		
	}
#endif
	cout << "查询命中： " << hit_num << "\n查询错误：" << err_num  << "\n总分：" << tsc << endl;
	cout << "------------------------------------------------" << endl;
	cout << "输入查询号(q***)获取详细信息：" << ends;

	string qs;
	while (cin >> qs) {
		if (m.find(qs) == m.end()) { cout << "输入错误，重新输入："; continue;}
		cout << String_convert::utf8_to_string(m[qs].first) << " 错误的答案有：" << endl;
		const auto &errorvec = errm[qs];
		for_each(errorvec.begin(), errorvec.end(), [](const auto &s) {cout << s << ends; });

		cout << endl << "未查询到的正确答案有：" << endl;
		const auto &shouldvec = shouldm[qs];
		for_each(shouldvec.begin(), shouldvec.end(), [](const auto &s) {cout << s << ends; });
		cout << endl<< "------------------------------------------------" << endl;
		cout << "输入查询号(q***)获取详细信息：" << ends;
	}

	system("pause");
}