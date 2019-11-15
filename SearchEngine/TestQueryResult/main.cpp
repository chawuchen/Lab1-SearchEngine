#include "../SearchEngine/Csv.h"
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "../SearchEngine/String_convert.h"
using namespace std;

bool cmp(const string &s1, const string &s2) {
	return stoi(s1.substr(1)) < stoi(s2.substr(1));
}

int main() {
	vector<vector<string>> vec;
	Csv::read("data_compare.csv", 4, vec);
	map<string, pair<string, set<string>>, decltype(cmp) *> m(cmp);
	for (auto &v : vec) {
		m[v[0]].first = std::move(v[3]);
		m[v[0]].second.emplace(std::move(v[1]));	// stoi(v[2]) ���ֵ��ʱû�õ�
	}

	vec.clear();
	Csv::read("submission.csv", 2, vec);
	map<string, set<string>, decltype(cmp) *> q(cmp);
	for (auto &v : vec) {
		q[v[0]].insert(std::move(v[1]));
	}

	vec.clear();
	vec.shrink_to_fit();

	int err_num = 0;
	int hit_num = 0;
	map<string, vector<string>> errm;
	map<string, vector<string>> shouldm;
	for (const auto &p : q) {
		const auto &my_doc_set_20 = p.second;
		const auto &all_doc_set = m[p.first].second;
		vector<string> errorvec;
		set_difference(my_doc_set_20.begin(), my_doc_set_20.end(), all_doc_set.begin(),
					   all_doc_set.end(), back_inserter(errorvec));

		int errs = errorvec.size();
		err_num += errorvec.size();
		hit_num += (int)my_doc_set_20.size() - errs;
		//if ((int)my_doc_set_20.size() != 20) {
		//	printf("��ѯ %4s ���󣡣���\n", p.first.c_str()); cin.get();
		//}
		printf("��ѯ %4s ������ %2d/20\t%-20s  %s\n", p.first.c_str(), errs, 
			   string(errs, '*').c_str(), String_convert::utf8_to_string(m[p.first].first).c_str());
		//cout << "�����ѳ������ĵ��У�\n";
		//for_each(errorvec.begin(), errorvec.end(), [](const auto &s) {cout << s << endl; });
		errm[p.first] = std::move(errorvec);
		set_difference(all_doc_set.begin(), all_doc_set.end(), my_doc_set_20.begin(),
					   my_doc_set_20.end(), back_inserter(shouldm[p.first]));
	}
	cout << "��ѯ���У� " << hit_num << "\n��ѯ����" << err_num << endl;
	cout << "------------------------------------------------" << endl;
	cout << "�����ѯ��(q***)��ȡ��ϸ��Ϣ��" << ends;

	string qs;
	while (cin >> qs) {
		if (m.find(qs) == m.end()) { cout << "��������������룺"; continue;}
		cout << String_convert::utf8_to_string(m[qs].first) << " ����Ĵ��У�" << endl;
		const auto &errorvec = errm[qs];
		for_each(errorvec.begin(), errorvec.end(), [](const auto &s) {cout << s << ends; });

		cout << endl << "δ��ѯ������ȷ���У�" << endl;
		const auto &shouldvec = shouldm[qs];
		for_each(shouldvec.begin(), shouldvec.end(), [](const auto &s) {cout << s << ends; });
		cout << endl<< "------------------------------------------------" << endl;
		cout << "�����ѯ��(q***)��ȡ��ϸ��Ϣ��" << ends;
	}

	system("pause");
}