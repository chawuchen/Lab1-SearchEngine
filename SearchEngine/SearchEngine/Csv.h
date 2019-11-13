#pragma once

#include <vector>
#include <string>
#include <stdexcept>

class Csv {
public:
	// 第一行不读，没有处理转义
	static void read(const std::string &file_name, 
					 int cols, 
					 std::vector<std::vector<std::string>> &vec);

	static void write(const std::string &file_name, 
					  const std::vector<std::vector<std::string>> &vec);
};

