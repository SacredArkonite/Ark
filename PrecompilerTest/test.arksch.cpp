#include <algorithm>
#include <iostream>
#include <string>

void main()
{
//DATA
std::string const1 = "Hello World!";
char const2 = 'l';

//VARIABLES ON STACK FOR EACH OUTPUTS


std::string rm_new_text;

//OPERATIONS AS LAMBDAS

//OPERATION print_and_wait
auto prnt = [](  std::string text)
{
std::cout<<text<<std::endl;
char c;
std::cin>>c;
};

//OPERATION remove_char
auto rm = [ &rm_new_text](  std::string text, char character)
{
rm_new_text = text;
rm_new_text.erase (std::remove(rm_new_text.begin(), rm_new_text.end(), character), rm_new_text.end());
};

//CREATE THE EXECUTION FLOW
rm(  const1, const2);
prnt(  rm_new_text);
return;
}
