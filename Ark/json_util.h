#include "jsoncpp\json\json.h"

void interpret(std::string filename, Json::Value& root)
{
	std::ifstream file;
	file.open(filename);

	if (!file.good()) {
		std::cout << "Could not open file!" << std::endl;
		return;
	}

	Json::Reader reader;

	reader.parse(file, root);
}