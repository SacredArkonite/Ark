#include <iostream>
void main()
{
//DATA
int const1_out = 2;
int const2_out = 3;
int const3_out = 4;
//OPERATIONS

//OPERAION: mul1
//TYPE: multiplier
//OUTPUTS
int mul1_out;
{
//INPUTS
int in1;
int in2;
//SET INPUTS
in1 = const2_out;
in2 = const3_out;
//OPERATION
mul1_out=in1*in2;;
}

//OPERAION: add1
//TYPE: adder
//OUTPUTS
int add1_out;
{
//INPUTS
int in1;
int in2;
//SET INPUTS
in1 = const1_out;
in2 = mul1_out;
//OPERATION
add1_out=in1+in2;;
}

//OPERAION: prnt_wait
//TYPE: print_and_wait
//OUTPUTS
{
//INPUTS
int in;
//SET INPUTS
in = add1_out;
//OPERATION
std::cout<<"Value:"<<in<<std::endl;
char
t;
std::cin>>t;
}

return;
}
