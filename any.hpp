#ifndef mmocANY_HPP
#define mmocANY_HPP

//WORK IN PROGRESS

#include <stdexcept>
#include <memory>
#include <typeindex>
#include <functional>
#include "signature.hpp"
#include "throw.hpp"

#define mmocSETFUNC(ObjectType,NewValueType) \
	[](void* mmoc_arg__obj,const void* mmoc_arg__new_val)->void \
	{ \
		ObjectType* obj=reinterpret_cast<ObjectType*>(mmoc_arg__obj); \
		const NewValueType* new_val=reinterpret_cast<const NewValueType*>(mmoc_arg__new_val);
		//[code for set function would go here, then followed by a closing brace]

#define mmocGETFUNC(ObjectType,T) \
	[](char* mmoc_arg__ret_by_val,const void** mmoc_arg__ret_ptr_ptr,void* mmoc_arg__obj)->void \
	{ \
		const ObjectType* obj=reinterpret_cast<const ObjectType*>(mmoc_arg__obj); \
		T* const ret_by_val=reinterpret_cast<T*>(mmoc_arg__ret_by_val); \
		const T*& ret_ptr=*reinterpret_cast<const T**>(mmoc_arg__ret_ptr_ptr);
		//[code for get function would go here, then followed by a closing brace]

//so you can use commas within macro arguments
#define mmocSEP ,

//so that code editors don't get confused by a seeming lack of an open bracket
#define mmocEND }

namespace mmoc
{

struct Object; //forward declaration (is it really needed?)

//exceptions
struct TypeConflictException : std::runtime_error
	{ TypeConflictException(std::string what_in) : runtime_error(what_in) {} };
struct NullAccessException : std::runtime_error
	{ NullAccessException(std::string what_in) : runtime_error(what_in) {} };

//////////////////////////////////////////////////////////////////////////////////////////////////
// Any class which can either be used to store any type of data or as a "proxy" to some external
// data which it does not own, and then to manipulate said data in certain ways in a typesafe way
// regardless of whether the Any is acting as a proxy to or is storing its own data.
struct Any
{
	friend mmoc::Object;
	
	struct RTTI //Run-Time Type Information
	{
		void (* assign_func)(Any&,const Any&);
		void (* delete_func)(Any&);
		std::type_index type_id;
		bool owns_data;
		RTTI(std::type_index type_id_in,bool owns_data_in)
			: type_id(type_id_in), owns_data(owns_data_in) {}
	};
public: //private:
	//instance storing the corresponding RTTI instance which also encompasses
	//additional things specific to whether the Any owns its data or is a proxy
	//to external data (see get_RTTI() for details on how the RTTI instance is
	//filled depending on type and whether the Any owns its own data or not)
	RTTI* rtti;
	struct Proxy
	{
		void* object;
		void(* set_func)(void*,const void*);
		void(* get_func)(char*,const void**,void*);
	};
	union Data
	{
		char embedded_data[sizeof(Proxy)];
		void* data_ptr;
		Proxy proxy;
	};
	Data data;
//public:
	template<typename T,bool OwnsData> static RTTI* get_RTTI()
	{
		static std::unique_ptr<RTTI> rtti(new RTTI(std::type_index(typeid(T)),OwnsData));
		static bool first_time=true;
		
		if(!first_time)
			return &*rtti;
		
		rtti->assign_func=[](Any& dest,const Any& src)
		{
			//for getting value of get function (forbids by-value get for sizeof(T)>64)
			char temp_val[sizeof(T)<=64 ? sizeof(T) : 0];
			const void* temp_ptr=temp_val;
			
			//getting (pointer to) value from src
			if(src.rtti->owns_data)
				temp_ptr = sizeof(T)<=sizeof(Proxy) ? src.data.embedded_data : src.data.data_ptr;
			else src.data.proxy.get_func(temp_val,&temp_ptr,src.data.proxy.object);
			
			//assigning dest to gotten (pointer to) value from src
			if(OwnsData)
				*reinterpret_cast<T*>( sizeof(T)<=sizeof(Proxy) ?
						dest.data.embedded_data : dest.data.data_ptr
					) = *reinterpret_cast<const T*>(temp_ptr)
					;
			else dest.data.proxy.set_func(dest.data.proxy.object,temp_ptr);
		};
		
		//there's only something to delete if the Any owns data
		if(OwnsData)
			rtti->delete_func=[](Any& to_delete)
			{
				reinterpret_cast<T*>( sizeof(T)<=sizeof(Proxy) ?
						to_delete.data.embedded_data : to_delete.data.data_ptr
					) -> ~T()
					;
			};
		else rtti->delete_func=[](Any& to_delete){};
		
		//prevent redundant repeated initialisations of the RTTI instance ("rtti") and
		//return its address of it (by first dereferencing it and then getting its address
		//so that you get a return value of RTTI* rather than std::unique_ptr<RTTI>)
		first_time=false;
		return &*rtti;
	}
	//default constructor only provided for use by containers for which their
	//type need be default constructible (it should not be used) and for use in
	//the static Any creation functions to create a "blank" (uninitialised) Any
	Any() : rtti(nullptr) {}
	//destructor needs to call destructor of any stored objects (if any)
	~Any()
		{ rtti->delete_func(*this); }
	
	Any& assign_to(const Any& new_val)
	{
		if(rtti->type_id!=new_val.rtti->type_id)
			mmocTHROW( TypeConflictException,
				"assigning Any of type "+mmoc::signature(rtti->type_id)+
				" to Any of type "+mmoc::signature(new_val.rtti->type_id)
				);
		rtti->assign_func(*this,new_val);
		return *this;
	}
	
	//Any creation functions
	template<typename T> static Any create_from_data(const T& data_in)
	{
		Any to_return;
		to_return.rtti=get_RTTI<T,true>();
		if(sizeof(T)<=sizeof(Proxy))
			*reinterpret_cast<T*>(to_return.data.embedded_data)=data_in;
		else to_return.data.data_ptr=new T(data_in);
		return to_return;
	}
	template<typename T> static Any create_proxy(T* data_to_proxy)
	{
		Any to_return;
		to_return.rtti=get_RTTI<T,false>();
		to_return.data.proxy.object=data_to_proxy;
		to_return.data.proxy.set_func=[](void* obj,const void* new_val)
			{ *reinterpret_cast<T*>(obj)=*reinterpret_cast<const T*>(new_val); }
			;
		to_return.data.proxy.get_func=[](char* ret_by_val,const void** ret_ptr_ptr,void* obj)
			{ *ret_ptr_ptr=obj; }
			;
		return to_return;
	}
	template<typename T,typename Object,typename SetFunc,typename GetFunc>
	static Any create_proxy(Object* object,SetFunc set_func_in,GetFunc get_func_in)
	{
		Any to_return;
		to_return.rtti=get_RTTI<T,false>();
		to_return.data.proxy.object=object;
		to_return.data.proxy.set_func=set_func_in;
		to_return.data.proxy.get_func=get_func_in;
		return to_return;
	}
	
	//good to use for all cases but requires inputting an argument to recieve the return value
	template<typename T> void get(T& ret_ref) const
	{
		if(std::type_index(typeid(T))!=rtti->type_id)
			mmocTHROW( TypeConflictException,
				"getting Any as invalid type "+mmoc::signature(std::type_index(typeid(T)))+
				" when it is of type "+mmoc::signature(rtti->type_id)
				);
		if(!rtti->owns_data)
		{
			void* temp_ptr=reinterpret_cast<void*>(&ret_ref);
			data.proxy.get_func(reinterpret_cast<char*>(&ret_ref),&temp_ptr,data.proxy.object);
		} else if(sizeof(T)<=sizeof(Proxy))
			ret_ref=*reinterpret_cast<const T*>(data.embedded_data);
		else ret_ref=*reinterpret_cast<const T*>(data.data_ptr);
	}
	//only use if Any is not a proxy which returns by value
	template<typename T> const T& get_by_ref() const
	{
		if(std::type_index(typeid(T))!=rtti->type_id)
			mmocTHROW( TypeConflictException,
				"getting Any as invalid type "+mmoc::signature(std::type_index(typeid(T)))+
				" when it is of type "+mmoc::signature(rtti->type_id)
				);
		if(!rtti->owns_data)
		{
			void* temp_ptr;
			data.proxy.get_func(nullptr,&temp_ptr,data.proxy.object);
			return *reinterpret_cast<const T*>(temp_ptr);
		} else if(sizeof(T)<=sizeof(Proxy))
			return *reinterpret_cast<const T*>(data.embedded_data);
		else return *reinterpret_cast<const T*>(data.data_ptr);
	}
	//same as above except returns by const value instead of const reference
	template<typename T> const T get_by_val() const
	{
		if(std::type_index(typeid(T))!=rtti->type_id)
			mmocTHROW( TypeConflictException,
				"getting Any as invalid type "+mmoc::signature(std::type_index(typeid(T)))+
				" when it is of type "+mmoc::signature(rtti->type_id)
				);
		if(!rtti->owns_data)
		{
			char temp_val[sizeof(T)<=64 ? sizeof(T) : 0];
			void* temp_ptr=temp_val;
			data.proxy.get_func(temp_val,&temp_ptr,data.proxy.object);
			return *reinterpret_cast<T*>(temp_val);
		} else if(sizeof(T)<=sizeof(Proxy))
			return *reinterpret_cast<const T*>(data.embedded_data);
		else return *reinterpret_cast<const T*>(data.data_ptr);
	}
	//this works as you'd expect
	template<typename T> void set(const T& new_val)
	{
		if(std::type_index(typeid(T))!=rtti->type_id)
			mmocTHROW( TypeConflictException,
				"setting Any as invalid type "+mmoc::signature(std::type_index(typeid(T)))+
				" when it is of type "+mmoc::signature(rtti->type_id)
				);
		if(rtti->owns_data)
			if(sizeof(T)<=sizeof(Proxy))
				*reinterpret_cast<T*>(data.embedded_data)=new_val;
			else *reinterpret_cast<T*>(data.data_ptr)=new_val;
		else data.proxy.set_func(data.proxy.object,&new_val);
	}
	
	//informational functions
	std::type_index get_type_index() const
		{ return rtti->type_id; }
	std::string get_type_name() const
		{ return mmoc::signature(rtti->type_id); }
	template<typename T> bool is() const
		{ return std::type_index(typeid(T))==rtti->type_id; }
	bool owns_data() const
		{ return rtti->owns_data; }
};

} //end of namespace mmoc

#endif