//INCLUDES
#include<algorithm>
#include<fstream>
#include<functional>
#include<iostream>
#include<regex>
#include<sstream>
#include<string>
#include"json_util.h"
#include"jsoncpp\json\json.h"

void main()
{
//DATAS
std::string arksch_filename = "../precompiler2.arksch";


//CREATE VARIABLES ON STACK FOR EACH OUTPUTS
std::string append_prefix_clean_text;

int get_input_index_index;

std::string find_operation_ops;

std::stringstream format_connections_ss;

std::stringstream format_datas_ss;

std::stringstream format_includes_ss;

std::stringstream format_lambdas_ss;

std::stringstream format_outputs_ss;

std::stringstream format_precompiler_ss;

std::vector<std::string> get_unique_include_list_c;
std::vector<std::string> get_unique_include_list_l;

Json::Value split_arksch_data_list;
Json::Value split_arksch_operation_list;
Json::Value split_arksch_connection_list;
std::string split_arksch_output_filename;




//OPERATIONS AS LAMBDAS
//OPERATION ArkCompiler/append_prefix
auto append_prefix = [ &append_prefix_clean_text ] ( std::string text, std::vector<std::string> find, std::string prefix)
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
};

//OPERATION ArkCompiler/get_input_index
auto get_input_index = [ &get_input_index_index ] ( std::string arkop_name, std::string input_name)
{ 
std::string opfilename = "../ArkLib/Operation/" + arkop_name + ".arkop";

Json::Value root;
interpret(opfilename, root);

int count = 0;
get_input_index_index = -1;

for each (Json::Value input in root["inputs"])
{
if (input["name"].asString() == input_name)
get_input_index_index = count;
count++;
}
};

//OPERATION ArkCompiler/find_operation
auto find_operation = [ &find_operation_ops ] ( Json::Value operation_list, std::string block_name)
{ 
for each (Json::Value op in operation_list)
if (op["name"].asString() == block_name)
find_operation_ops = op["operation"].asString();
};

//OPERATION ArkCompiler/format_connections
auto format_connections = [ &format_connections_ss,get_input_index,find_operation,&find_operation_ops,&get_input_index_index ] ( Json::Value connection_list, Json::Value operation_list)
{ 
format_connections_ss << "//CREATE THE EXECUTION FLOW" << std::endl;
for each (Json::Value block in connection_list)
{
std::string dest_block = block["block"].asString();
find_operation(operation_list, dest_block);

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
get_input_index(find_operation_ops, input["input"].asString());  //BAD CODE WE CANNOT ASSUME THE OUTPUT WE NEED TO CALL! (find_operation_ops)
if (get_input_index_index != -1) //BAD CODE WE CANNOT ASSUME THE OUTPUT WE NEED TO CALL! (get_input_index_index)
input_map[get_input_index_index] = source_name;
else
std::cout << input["input"].asString() << "not found in " << find_operation_ops << "." << std::endl;
}

format_connections_ss << block["block"].asString() << "(";
for (auto it : input_map)
format_connections_ss << " " << it << "," ;

format_connections_ss.seekp(-1, format_connections_ss.cur);
format_connections_ss << ");" << std::endl;
}
};

//OPERATION ArkCompiler/format_datas
auto format_datas = [ &format_datas_ss ] ( Json::Value data_list)
{ 
format_datas_ss << "//DATAS" << std::endl;
for each (Json::Value data in data_list)
format_datas_ss << data["type"].asString() << " " << data["name"].asString() << " = " << data["data"].asString() << ";" << std::endl;
format_datas_ss << std::endl;
};

//OPERATION ArkCompiler/format_includes
auto format_includes = [ &format_includes_ss ] ( std::vector<std::string> include_list_c, std::vector<std::string> include_list_l)
{ 
format_includes_ss << "//INCLUDES" << std::endl;
for each (std::string i in include_list_c)
format_includes_ss << "#include"<<"<"<<i<<">" << std::endl;
for each (std::string i in include_list_l)
format_includes_ss << "#include"<< "\"" <<i<< "\"" << std::endl;
};

//OPERATION ArkCompiler/format_lambdas
auto format_lambdas = [ &format_lambdas_ss,append_prefix ] ( Json::Value operation_list)
{ 
format_lambdas_ss << "//OPERATIONS AS LAMBDAS" << std::endl;
for each (Json::Value node in operation_list)
{
std::string opfilename = "../ArkLib/Operation/" + node["operation"].asString() + ".arkop";

Json::Value op_root;
interpret(opfilename, op_root);

std::string optype = node["operation"].asString();
std::string opname = node["name"].asString();
std::vector<std::string> output_names;

format_lambdas_ss << "//OPERATION " << optype << std::endl;
//Write outputs
format_lambdas_ss << "auto " << opname << " = [ " ;
for each (Json::Value output in op_root["outputs"])
{
format_lambdas_ss << "&" << opname << "_" << output["name"].asString() << ",";
output_names.push_back(output["name"].asString());
}
for each (Json::Value output in op_root["additionnal"])
format_lambdas_ss << output["name"].asString() << ",";
format_lambdas_ss.seekp(-1, format_lambdas_ss.cur);
format_lambdas_ss << " ] (" ;

//Write inputs
for each (Json::Value input in op_root["inputs"])
//Cannot put const here, but I want to be sure the input is not modified by the method but still pass it by ref by default
format_lambdas_ss << " " /* << "const "*/ << input["type"].asString() /*<< "&"*/ << " " << input["name"].asString() << ",";

format_lambdas_ss.seekp(-1, format_lambdas_ss.cur);
format_lambdas_ss << ")" << std::endl << "{ " << std::endl;

//Write operations
//Operations. We need to spaw the outputs name for their new names if necessary.
if(output_names.size()>0)
for each (Json::Value op_str in op_root["instructions"])
format_lambdas_ss << append_prefix(op_str["op"].asString(), output_names, opname) << std::endl;
else
for each (Json::Value op_str in op_root["instructions"])
format_lambdas_ss << op_str["op"].asString() << std::endl;

format_lambdas_ss << "};" << std::endl << std::endl;
}
};

//OPERATION ArkCompiler/format_outputs_init
auto format_outputs = [ &format_outputs_ss ] ( Json::Value operation_list)
{ 
format_outputs_ss << "//CREATE VARIABLES ON STACK FOR EACH OUTPUTS" << std::endl;
for each (Json::Value node in operation_list)
{
//Load arkop file as Json
std::string opfilename = "../ArkLib/Operation/" + node["operation"].asString() + ".arkop";

Json::Value op_root;
interpret(opfilename, op_root);

//Create variables
for each (Json::Value output in op_root["outputs"])
format_outputs_ss << output["type"].asString() << " " << node["name"].asString() << "_" << output["name"].asString() << ";" << std::endl;
format_outputs_ss << std::endl;
}
format_outputs_ss << std::endl;
};

//OPERATION ArkCompiler/format_precompiler
auto format_precompiler = [ &format_precompiler_ss ] ( std::stringstream& ss_includes, std::stringstream& ss_datas, std::stringstream& ss_outputs, std::stringstream& ss_lambdas, std::stringstream& ss_connections)
{ 
format_precompiler_ss << ss_includes.str() << std::endl << "void main()" << std::endl << "{" << std::endl << ss_datas.str() << std::endl << ss_outputs.str() << std::endl << ss_lambdas.str() << std::endl << ss_connections.str() << std::endl << "return;" << std::endl << "}" << std::endl;
};

//OPERATION ArkCompiler/get_unique_include_list
auto get_unique_include = [ &get_unique_include_list_c,&get_unique_include_list_l ] ( Json::Value operation_list)
{ 
for each(Json::Value node in operation_list)
{
//Read arkop file as Json
std::string opfilename = "../ArkLib/Operation/" + node["operation"].asString() + ".arkop";
Json::Value op_root;
interpret(opfilename, op_root);

for each(Json::Value d in op_root["dependancies"])
{
get_unique_include_list_c.push_back(d["c"].asString());
get_unique_include_list_l.push_back(d["l"].asString());
}
}
std::sort(get_unique_include_list_c.begin(), get_unique_include_list_c.end());
get_unique_include_list_c.erase(std::unique(get_unique_include_list_c.begin(), get_unique_include_list_c.end()), get_unique_include_list_c.end());
get_unique_include_list_c.erase(std::remove_if(get_unique_include_list_c.begin(), get_unique_include_list_c.end(), std::mem_fun_ref(&std::string::empty)), get_unique_include_list_c.end());
std::sort(get_unique_include_list_l.begin(), get_unique_include_list_l.end());
get_unique_include_list_l.erase(std::unique(get_unique_include_list_l.begin(), get_unique_include_list_l.end()), get_unique_include_list_l.end());
get_unique_include_list_l.erase(std::remove_if(get_unique_include_list_l.begin(), get_unique_include_list_l.end(), std::mem_fun_ref(&std::string::empty)), get_unique_include_list_l.end());
};

//OPERATION ArkCompiler/split_arksch
auto split_arksch = [ &split_arksch_data_list,&split_arksch_operation_list,&split_arksch_connection_list,&split_arksch_output_filename ] ( std::string filename)
{ 
Json::Value root;
interpret(filename, root);
split_arksch_data_list = root["datas"];
split_arksch_operation_list = root["operations"];
split_arksch_connection_list = root["connections"];
split_arksch_output_filename = root["schema"].asString();
};

//OPERATION ArkCompiler/write_file
auto write_file = [ ] ( std::string filename, std::stringstream& data)
{ 
std::ofstream filewriter("../PrecompilerTest/" + filename + ".cpp");
filewriter << data.str();
filewriter.close();
};


//CREATE THE EXECUTION FLOW
split_arksch( arksch_filename);
get_unique_include( split_arksch_operation_list);
format_includes( get_unique_include_list_c, get_unique_include_list_l);
format_datas( split_arksch_data_list);
format_outputs( split_arksch_operation_list);
format_lambdas( split_arksch_operation_list);
format_connections( split_arksch_connection_list, split_arksch_operation_list);
format_precompiler( format_includes_ss, format_datas_ss, format_outputs_ss, format_lambdas_ss, format_connections_ss);
write_file( split_arksch_output_filename, format_precompiler_ss);

return;
}
