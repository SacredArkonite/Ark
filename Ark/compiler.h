#pragma once
#include <string>
#include <fstream>
#include <sstream>

#include "json_util.h"
#include "jsoncpp\json\json.h"

void precompile(std::string arkfile)
{
	Json::Value root;
	interpret(arkfile, root);

	Json::Value data_list = root["datas"];
	Json::Value operation_list = root["operations"];
	Json::Value connection_list = root["connections"];

	std::stringstream ss;
	std::ifstream filereader;

	for each (Json::Value node in data_list)
	{
		std::string datafilename = "../Ark/Data/" + node["type"].asString() + ".arkdata";

		filereader.open(datafilename);
		std::string datatype;
		filereader >> datatype;
		std::string dataname = node["name"].asString();

		ss << datatype << 

	}

	return;
}