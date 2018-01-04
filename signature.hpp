#ifndef mmocSIGNATURE_HPP
#define mmocSIGNATURE_HPP

#include <string>
#include <typeindex>
#include <cxxabi.h>

namespace mmoc
{

std::string signature(std::type_index type_id)
{
	std::string tname=type_id.name();
	#if defined(__clang__) || defined(__GNUG__)
	int status;
	char* demangled_name=abi::__cxa_demangle(tname.c_str(),NULL,NULL,&status);
	if(status==0)
		{ tname=demangled_name; std::free(demangled_name); }
	#endif
	return tname;
}

} //end of namespace mmoc

#endif