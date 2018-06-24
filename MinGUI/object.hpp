#ifndef mmocOBJECT_HPP
#define mmocOBJECT_HPP

#include <unordered_map>
#include "any.hpp"
#include "throw.hpp"

namespace mmoc
{

struct Object
{
	using AttrList = std::unordered_map<std::string,Any>;
	AttrList attrs;
	
	AttrList::iterator get_attr(const std::string& name)
	{
		auto attr=attrs.find(name);
		if(attr==attrs.end())
			mmocTHROW( std::out_of_range,
				"trying to get attribute \""+name+"\" when it doesn't exist."
				);
		return attr;
	}
	
	//for chain-adding attributes
	template<typename T> Object& add(const std::string& name)
		{ attrs[name]=Any::create_from_data<T>(T()); return *this; }
	template<typename T> Object& add(const std::string& name,const T& data)
		{ attrs[name]=Any::create_from_data<T>(data); return *this; }
	template<typename T> Object& add_proxy(const std::string& name,T* to_proxy)
		{ attrs[name]=Any::create_proxy<T>(to_proxy); return *this; }
	template<typename T,typename ProxyObject,typename SetFunc,typename GetFunc> Object& add_proxy(
		const std::string& name, ProxyObject* object_in, SetFunc set_func_in, GetFunc get_func_in
		)
		{ attrs[name]=Any::create_proxy<T>(object_in,set_func_in,get_func_in); return *this; }
	//template<typename R,typename T>
	//add(R(* func)(Object&,const std::string&,T*),const std::string& name,T* to_proxy)
		//{ func(*this,name,to_proxy); return *this; }
	template<typename InterfaceAddingFunc,typename T>
	Object& add(InterfaceAddingFunc func,const std::string& name,T* to_proxy)
		{ func( *this,name+".",to_proxy ); return *this; }
	template<typename InterfaceAddingFunc,typename T>
	Object& add(InterfaceAddingFunc func,const std::string& name,const std::string& to_proxy)
		{ func( *this,name+".",raw_data_ptr(to_proxy) ); return *this; }
	template<typename InterfaceAddingFunc,typename T>
	Object& add(InterfaceAddingFunc func,const std::string& to_proxy)
		{ func( *this,to_proxy+".",raw_data_ptr(to_proxy) ); return *this; }
	
	//getters and setters corresponding to those of mmoc::Any's
	template<typename T> Object& set(const std::string& name,const T& new_val)
		{ get_attr(name)->second.set(new_val); return *this; }
	template<typename T> T val(const std::string& name)
		{ return get_attr(name)->second.get_by_val<T>(); }
	template<typename T> T& ref(const std::string& name)
		{ return get_attr(name)->second.get_by_ref<T>(); }
	void* raw_data_ptr(const std::string& name)
		{ return get_attr(name)->second.raw_data_ptr(); }
	
	//other
	template<typename T> bool has_attr(const std::string& name) const
		{ return attrs.find(name)!=attrs.end(); }
};

} //end of namespace mmoc

#endif