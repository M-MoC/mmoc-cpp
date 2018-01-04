 /*
 * Example usage:

#include <iostream>

int main()
{
	try
	{
		mmocTHROW(std::runtime_error,"this is where you describe the error");
	} catch(std::runtime_error& e)
		{ std::cout<<e.what(); }
	return 0;
}

 * would produce the output:

std::runtime_error thrown at line 12 of program.cpp: this is where you describe the error

 * assuming the file containing the program is saved as "program.cpp"
*/

#ifndef mmocTHROW_HPP
#define mmocTHROW_HPP

#include <string>

#define mmocTHROW(Exception,description) throw Exception(std::string("\n" #Exception \
	" thrown at line ")+std::to_string(__LINE__)+" of "+__FILE__+": "+description+"\n")

#endif