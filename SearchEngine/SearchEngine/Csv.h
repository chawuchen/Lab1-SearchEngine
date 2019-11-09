#pragma once

#include <vector>
#include <string>
#include <stdexcept>

class Csv {
public:
	static void read(const std::string &file_name, 
					 int cols, 
					 std::vector<std::vector<std::string>> &vec);

	static void write(const std::string &file_name, 
					  const std::vector<std::vector<std::string>> &vec);
};

