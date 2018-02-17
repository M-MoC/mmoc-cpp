#include <iostream>
#include "any.hpp"

using mmoc::Any;

namespace mmocf
{

auto add=[](int a,int b)-> int { return a+b; };
auto sub=[](int a,int b)-> int { return a-b; };

} //end of namespace mmocf

int main()
{
	auto
		a_f_add=Any::create_from_data<int(*)(int,int)>(mmocf::add),
		a_f_sub=Any::create_from_data<int(*)(int,int)>(mmocf::sub);
	
	std::cout<<
		"a_f_add.get_by_val<int(*)(int,int)>()(2,3) => "
			<<a_f_add.get_by_val<int(*)(int,int)>()(2,3)<<"\n"
		"a_f_sub.get_by_val<int(*)(int,int)>()(2,3) => "
			<<a_f_sub.get_by_val<int(*)(int,int)>()(2,3)<<"\n"
		"\n"
		;
	
	char close; std::cin>>close;
}
