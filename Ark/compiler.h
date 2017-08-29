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

	//TODO: deal with includes in a good way?

	ss << "#include <iostream>" << std::endl;
	ss << "void main()" << std::endl << "{" << std::endl;

	ss << "//DATA" << std::endl;

	for each (Json::Value node in data_list)
	{
		std::string datafilename = "../ArkLib/Data/" + node["type"].asString() + ".arkdata";

		filereader.open(datafilename);
		//filereader.seekg(0, std::ios::beg);
		std::string datatype;
		filereader >> datatype;
		std::string dataname = node["name"].asString();
		for each (Json::Value output in node["outputs"])
		{
			ss << datatype << " " << dataname << "_" << output["name"].asString() << " = " << node["data"].asInt() << ";" << std::endl;

		}
		filereader.close();
	}
	ss << "//OPERATIONS" << std::endl << std::endl;

	for each (Json::Value node in operation_list)
	{
		//Start header for operation
		std::string opfilename = "../ArkLib/Operation/" + node["type"].asString() + ".arkop";

		filereader.open(opfilename);
		//filereader.seekg(0, std::ios::beg);
		std::string optype = node["type"].asString();
		std::string opname = node["name"].asString();

		ss << "//OPERAION: " << opname << std::endl;
		ss << "//TYPE: " << optype << std::endl;

		//Create external variables
		ss << "//OUTPUTS" << std::endl;
		for each (Json::Value output in node["outputs"])
		{
			ss << output["type"].asString() << " " << opname << "_" << output["name"].asString() << ";" << std::endl;
		}

		ss << "{" << std::endl;

		//Create internal variables
		ss << "//INPUTS" << std::endl;
		for each (Json::Value input in node["inputs"])
		{
			ss << input["type"].asString() << " " << input["name"].asString() << ";" << std::endl;
		}

		//TODO: do connection sin a better way? We search everytime atm
		//Do connections
		ss << "//SET INPUTS" << std::endl;
		for each (Json::Value connection in connection_list)
		{

			for each (Json::Value destination in connection["destinations"])
			{
				if (destination["block"].asString() == opname)
					ss << destination["port"].asString() << " = " << connection["source"]["block"].asString() << "_" << connection["source"]["port"].asString() << ";" << std::endl;
			}
		}

		//Do operation
		ss << "//OPERATION" << std::endl;
		std::string operation_str;

		//Operations must be 1 line per output with addition instructions at the end
		for each (Json::Value output in node["outputs"])
		{
			filereader >> operation_str;

			//TODO: Better way to isolate ouputs, on global scope
			ss << opname << "_" << operation_str << ";" << std::endl;
		}
		//optionnal additionnal stuff
		while (!filereader.eof())
		{
			filereader >> operation_str;
			ss << operation_str << std::endl;
		}


		ss << "}" << std::endl << std::endl;

		filereader.close();
	}

	ss << "return;" << std::endl << "}" << std::endl;

	//Output the result to a file
	std::ofstream filewriter("../PrecompilerTest/" + root["schema"].asString() + ".cpp");
	filewriter << ss.str();

	return;
}