#pragma once
#include <regex>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "json_util.h"
#include "jsoncpp\json\json.h"

void getUniqueIncludeList(Json::Value& operation_list, std::vector<std::string>& unique_include_list)
{
	for each(Json::Value node in operation_list)
	{
		//Read arkop file as Json
		std::string opfilename = "../ArkLib/Operation/" + node["operation"].asString() + ".arkop";
		Json::Value op_root;
		interpret(opfilename, op_root);

		for each(Json::Value d in op_root["dependancies"])
		{
			unique_include_list.push_back(d["d"].asString());
		}
	}
	std::sort(unique_include_list.begin(), unique_include_list.end());
	unique_include_list.erase(std::unique(unique_include_list.begin(), unique_include_list.end()), unique_include_list.end());
}

int getInputIndex(std::string arkop_name, std::string input_name)
{
	std::string opfilename = "../ArkLib/Operation/" + arkop_name + ".arkop";

	Json::Value root;
	interpret(opfilename, root);

	int index = 0;

	for each (Json::Value input in root["inputs"])
	{
		if (input["name"].asString() == input_name)
			return index;
		index++;
	}
	return -1;
}

//Find all strings from find in text and append prefix if they are whole word.
std::string appendPrefix(std::string text, std::vector<std::string> find, std::string prefix)
{
	std::string regex_str;
	for(auto it : find)
	{
		regex_str += "\\b";
		regex_str += it;
		regex_str += "\\b";
		regex_str += "|";
	}
	regex_str.pop_back();
	std::regex r(regex_str);

	return std::regex_replace(text, r, prefix + "_$&");
}

void precompile(std::string arkfile)
{
	Json::Value root;
	interpret(arkfile, root);

	Json::Value data_list = root["datas"];
	Json::Value operation_list = root["operations"];
	Json::Value connection_list = root["connections"];

	auto findOperation = [&operation_list](std::string opname)
	{
		for each (Json::Value op in operation_list)
		{
			if (op["name"].asString() == opname)
				return op["operation"].asString();
		}
	};

	std::stringstream ss;
	std::ifstream filereader;

	//List all dependencies and sort them
	std::vector<std::string> include_list;
	getUniqueIncludeList(operation_list, include_list);

	for each (std::string i in include_list)
	{
		ss << "#include <"<<i<<">" << std::endl;
	}
	
	ss << std::endl << "void main()" << std::endl << "{" << std::endl;

	ss << "//DATA" << std::endl;
	for each (Json::Value data in data_list)
	{
		ss << data["type"].asString() << " " << data["name"].asString() << " = " << data["data"].asString() << ";" << std::endl;
	}
	ss << std::endl;

	ss << "//VARIABLES ON STACK FOR EACH OUTPUTS" << std::endl << std::endl;

	for each (Json::Value node in operation_list)
	{
		//Load arkop file as Json
		std::string opfilename = "../ArkLib/Operation/" + node["operation"].asString() + ".arkop";

		Json::Value op_root;
		interpret(opfilename, op_root);

		//Create variables
		for each (Json::Value output in op_root["outputs"])
		{
			ss << output["type"].asString() << " " << node["name"].asString() << "_" << output["name"].asString() << ";" << std::endl;
		}
		ss << std::endl;
	}

	ss << "//OPERATIONS AS LAMBDAS" << std::endl << std::endl;

	for each (Json::Value node in operation_list)
	{
		std::string opfilename = "../ArkLib/Operation/" + node["operation"].asString() + ".arkop";

		Json::Value op_root;
		interpret(opfilename, op_root);

		std::string optype = node["operation"].asString();
		std::string opname = node["name"].asString();
		std::vector<std::string> output_names;

		ss << "//OPERATION " << optype << std::endl;
		//Write outputs
		ss << "auto " << opname << " = [ ";
		for each (Json::Value output in op_root["outputs"])
		{
			ss << "&" << opname << "_" << output["name"].asString() << ",";
			output_names.push_back(output["name"].asString());
		}
		ss.seekp(-1, ss.cur);
		ss << "]( ";

		//Write inputs
		for each (Json::Value input in op_root["inputs"])
		{
			//Cannot put const here, but I want to be sure the input is not modified by the method but still pass it by ref by default
			ss << " " /* << "const "*/ << input["type"].asString() /*<< "&"*/ << " " << input["name"].asString() << ",";
		}
		ss.seekp(-1, ss.cur);
		ss << ")" << std::endl << "{" << std::endl;

		//Write operations
		//Operations. We need to spaw the outputs name for their new names if necessary.
		if(output_names.size()>0)
			for each (Json::Value op_str in op_root["instructions"])
			{

				ss << appendPrefix(op_str["op"].asString(), output_names, opname) << std::endl;
			}
		else
			for each (Json::Value op_str in op_root["instructions"])
			{

				ss << op_str["op"].asString() << std::endl;
			}

		ss << "};" << std::endl << std::endl;
	}

	//Do connections
	ss << "//CREATE THE EXECUTION FLOW" << std::endl;
	for each (Json::Value block in connection_list)
	{
		std::string dest_block = block["block"].asString();
		std::string dest_arkop = findOperation(dest_block);

		std::vector<std::string> input_map(block["inputs"].size());
		//Map with destination port index. This is to allow swap of inputs in an arkop without needing to adjust the arksch.
		for each (Json::Value input in block["inputs"])
		{
			//Collect the source of the connection
			std::string source_name;
			if (input["source"]["output"].asString() == "")
				source_name = input["source"]["block"].asString();
			else
				source_name = input["source"]["block"].asString() + "_" + input["source"]["output"].asString();

			//Find input index
			int index = getInputIndex(dest_arkop, input["input"].asString());
			if (index != -1)
				input_map[index] = source_name;
			else
				std::cout << input["input"].asString() << " not found in " << dest_arkop << "." << std::endl;
		}

		ss << block["block"].asString() << "( ";
		for (auto it : input_map)
		{
			ss << " " << it << ",";
		}

		ss.seekp(-1, ss.cur);
		ss << ");" << std::endl;
	}


	ss << "return;" << std::endl << "}" << std::endl;

	//Output the result to a file
	std::ofstream filewriter("../PrecompilerTest/" + root["schema"].asString() + ".cpp");
	filewriter << ss.str();

	return;
}