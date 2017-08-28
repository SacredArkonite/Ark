#include <iostream>
#include <fstream>
#include "json_util.h"
#include "jsoncpp\json\json.h"


void main()
{
	Json::Value root;
	interpret("../test.json",root);

	std::cout << root["value"].asFloat() << std::endl;

	char t;
	std::cin >> t;

	return;
}