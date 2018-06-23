#ifndef mmocANY_HPP
#define mmocANY_HPP

#include <stdexcept>
#include <memory>
#include <typeindex>
#include <functional>
#include "signature.hpp"
#include "throw.hpp"

#define mmocSETFUNC(ObjectType,NewValueType) \
	[](void* mmocarg__obj,const void* mmocarg__new_val)->void \
	{ \
		ObjectType* obj=reinterpret_cast<ObjectType*>(mmocarg__obj); \
		const NewValueType* new_val=reinterpret_cast<const NewValueType*>(mmocarg__new_val);
		//[the code for set function would go after the use of the macro,
		//followed by "mmocEND" to end the function]
		//  explanation:
		//obj is a pointer to whatever object is pointed to in the Any - it can be used to do
		//something with that object; new_val is a pointer to what the Any is being assigned to

#define mmocGETFUNC(ObjectType,T) \
	[](char* mmocarg__ret_by_val_ptr,const void** mmocarg__ret_ptr_ptr,void* mmocarg__obj)->void \
	{ \
		const ObjectType* obj=reinterpret_cast<const ObjectType*>(mmocarg__obj); \
		T* const ret_by_val=reinterpret_cast<T*>(mmocarg__ret_by_val_ptr); \
		const T*& ret_ptr=*reinterpret_cast<const T**>(mmocarg__ret_ptr_ptr);
		//[the code for get function would go after the use of the macro,
		//followed by "mmocEND" to end the function]
		//  explanation:
		//*ret_by_val is assigned to the value that's intended to be returned by value, while
		//ret_ptr is assigned to a pointer pointing to the return value; this is useful if you
		//don't want to copy the value and/or want to allow 'fresh' access to the value
		//without having to call the get function again

//so that code editors don't get confused by a seeming lack of an open bracket
#define mmocEND }

//so that you can use commas within macro arguments
#define mmocSEP ,

namespace mmoc
{

struct TypeConflictException : std::runtime_error
	{ TypeConflictException(std::string what_in) : runtime_error(what_in) {} };

struct AttrList;

// DESCRIPTION NEEDS UPDATING
//////////////////////////////////////////////////////////////////////////////////////////////////
// Any class which can either be used to store any type of data or to act as a proxy to external
// data which it does not own. Data can be retrieved or assigned via get_by_ref(), get_by_cref(),
// get_by_val(), set() and assign_to() (with the lattermost accepting an Any and set() accepting
// const T& where T is the correct type of data).
// It uses the small object optimisation (meaning that when owning data, it will embed the data
// within itself if it fits to avoid a heap allocation).
// Note that this acts as a replacement for std::any which simultaneously further extends
// the capability of the Any by providing the ability to act as a proxy.
// Also note that the contents of or the data being proxied by a given Any must not be const.
class Any
{
	friend mmoc::AttrList;
	
	struct RTTI //Run-Time Type Information
	{
		void (* assign_func)(Any&,const Any&);
		void (* delete_func)(Any&);
		void* (* heap_alloc)(void*);
		std::type_index type_id;
		//0 = does not own data (is a proxy)
		//1 = owns data and data is (small enough to be) embedded
		//2 = owns data and data is not (small enough to be) embedded
		char owns_data;
		RTTI(std::type_index type_id_in,char owns_data_in)
			: type_id(type_id_in), owns_data(owns_data_in) {}
	};
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
	
public:
	
	// The below is a function to get the RTTI instance for a type "T" and based on
	// whether it should be responsible for owning data (and thus managing its lifetime)
	// or merely acting as a proxy to it (with said distinction signified by the template
	// parameter "bool OwnsData").
	//
	// It always returns the address of the corresponding RTTI instance; if said instance does not
	// yet exist (if it is the first time entering that specific instantiation of the function)
	// it will fill in the data for the RTTI instance accordingly and then return the
	// address of the instance.
	//
	// The function's mechanism relies on the fact that static variables are specific
	// to the template instantiation of the function rather than the broader function.
	// 
	// The address for a given RTTI instance will never change within the program's lifetime.
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
		{
			rtti->delete_func=[](Any& to_delete)
			{
				//is this really safe? must investigate
				reinterpret_cast<T*>( sizeof(T)<=sizeof(Proxy) ?
						to_delete.data.embedded_data : to_delete.data.data_ptr
					) -> ~T()
					;
			};
		} else rtti->delete_func=[](Any& to_delete){};
		
		//0 = does not own data (is a proxy)
		//1 = owns data and data is (small enough to be) embedded
		//2 = owns data and data is not (small enough to be) embedded
		if(OwnsData)
			if(sizeof(T)<=sizeof(Proxy))
				rtti->owns_data=1;
			else
			{
				rtti->owns_data=2;
				rtti->heap_alloc=[](void* value)-> void*
					{ return new T(*reinterpret_cast<T*>(value)); }
					;
			}
		else rtti->owns_data=0;
		
		//prevent redundant repeated initialisations of the RTTI instance ("rtti") and
		//return its address (by first dereferencing it and then getting its address
		//so that you get a return value of RTTI* rather than std::unique_ptr<RTTI>)
		first_time=false;
		return &*rtti;
	}
	//default constructor only provided for use by containers for which
	//their type need be default constructible (it should not be used)
	Any() : rtti(nullptr) {}
	//Any's destructor needs to call destructor of any owned object
	//(if Any is a proxy/does not own an object, delete_func does nothing)
	~Any()
		{ rtti->delete_func(*this); }
	Any(const Any& other)
	{
		rtti=other.rtti;
		data=other.data;
		if(other.rtti->owns_data==2)
			data.data_ptr=other.rtti->heap_alloc(other.data.data_ptr);
	}
	Any& operator= (const Any& other)
	{
		rtti=other.rtti;
		data=other.data;
		if(other.rtti->owns_data==2)
			data.data_ptr=other.rtti->heap_alloc(other.data.data_ptr);
	}
	Any(Any&&) = default;
	Any& operator= (Any&&) = default;
	
	//Any creation functions
	template<typename T> static Any create_from_data(T&& data_in)
	{
		Any to_return;
		to_return.rtti=get_RTTI<T,true>();
		if(sizeof(T)<=sizeof(Proxy))
			*reinterpret_cast<T*>(to_return.data.embedded_data)=data_in;
		else to_return.data.data_ptr=new T(data_in);
		return to_return;
	}
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
		to_return.data.proxy.set_func=[](void* object,const void* new_val)
			{ *reinterpret_cast<T*>(object)=*reinterpret_cast<const T*>(new_val); }
			;
		to_return.data.proxy.get_func=[](char* ret_by_val,const void** ret_ptr_ptr,void* object)
			{ *ret_ptr_ptr=object; }
			;
		return to_return;
	}
	template<typename T,typename Object,typename SetFunc,typename GetFunc>
	static Any create_proxy(Object* object_in,SetFunc set_func_in,GetFunc get_func_in)
	{
		Any to_return;
		to_return.rtti=get_RTTI<T,false>();
		to_return.data.proxy.object=reinterpret_cast<void*>(object_in);
		to_return.data.proxy.set_func=set_func_in;
		to_return.data.proxy.get_func=get_func_in;
		return to_return;
	}
	
	//// functions for getting and setting data ////
	//only use get_by_ref() if Any is not a proxy which returns by value
	template<typename T> const T& get_by_ref() const
	{
		if(!is<T>())
			mmocTHROW( TypeConflictException,
				"getting Any as invalid type "+signature<T>()+
				" when it is of type "+get_type_name()
				);
		if(!rtti->owns_data)
		{
			const void* temp_ptr=nullptr;
			data.proxy.get_func(nullptr,&temp_ptr,data.proxy.object);
			return *reinterpret_cast<const T*>(temp_ptr);
		} else if(sizeof(T)<=sizeof(Proxy))
			return *reinterpret_cast<const T*>(data.embedded_data);
		else return *reinterpret_cast<const T*>(data.data_ptr);
	}
	template<typename T> inline const T& get_by_cref() const
		{ return get_by_ref<T>(); }
	template<typename T> inline T& get_by_ref()
		{ return *const_cast<T*>(&get_by_cref<T>()); }
	//use this if Any contains a proxy which returns by value or contains
	//data which does not take up significant space in memory (e.g an int)
	//(it can be used for any type of Any, but using it for Any's with large
	//types is a bit silly; use get_by_ref() for that)
	template<typename T> T get_by_val() const
	{
		if(!is<T>())
			mmocTHROW( TypeConflictException,
				"getting Any as invalid type "+signature<T>()+
				" when it is of type "+get_type_name()
				);
		if(!rtti->owns_data)
		{
			char temp_val[sizeof(T)<=64 ? sizeof(T) : 0];
			const void* temp_ptr=temp_val;
			data.proxy.get_func(temp_val,&temp_ptr,data.proxy.object);
			return *reinterpret_cast<const T*>(temp_ptr);
		} else if(sizeof(T)<=sizeof(Proxy))
			return *reinterpret_cast<const T*>(data.embedded_data);
		else return *reinterpret_cast<const T*>(data.data_ptr);
	}
	//this works as you'd expect
	template<typename T> void set(const T& new_val)
	{
		if(!is<T>())
			mmocTHROW( TypeConflictException,
				"setting Any as invalid type "+signature<T>()+
				" when it is of type "+get_type_name()
				);
		if(rtti->owns_data)
			if(sizeof(T)<=sizeof(Proxy))
				*reinterpret_cast<T*>(data.embedded_data)=new_val;
			else *reinterpret_cast<T*>(data.data_ptr)=new_val;
		else data.proxy.set_func(data.proxy.object,&new_val);
	}
	//for assigning one Any to another such that they may be any type at any
	//time (the type is not fixed at compile-time) but on the condition that
	//their types match (else a TypeConflictException is thrown)
	Any& assign_to(const Any& other)
	{
		if(get_type_index()!=other.get_type_index())
			mmocTHROW( TypeConflictException,
				"assigning Any of type "+get_type_name()+
				" to Any of type "+other.get_type_name()
				);
		rtti->assign_func(*this,other);
		return *this;
	}
	
	//// functions for proxy checking, setting and getting ////
	template<typename Object> bool proxies(const Object& object_in) const
		{ return data.proxy.object==reinterpret_cast<void*>(object_in); }
	// For the below templated functions, you MUST make sure that the type Object or the type
	// pointed to by object_in is of the same type as the originally proxied object or the
	// behaviour is undefined. The behaviour is ALSO undefined if it is not a proxy.
	Any& set_proxy(void* object_in)
		{ data.proxy.object=object_in; return *this; }
	void* get_proxy()
		{ return data.proxy.object; }
	template<typename Object> Object* get_proxy()
		{ return reinterpret_cast<Object*>(data.proxy.object); }
	const void* get_proxy() const
		{ return data.proxy.object; }
	template<typename Object> const Object* get_proxy() const
		{ return reinterpret_cast<const Object*>(data.proxy.object); }
	const void* cget_proxy() const
		{ return data.proxy.object; }
	template<typename Object> const Object* cget_proxy() const
		{ return reinterpret_cast<const Object*>(data.proxy.object); }
	
	//// type-informational functions ////
	std::type_index get_type_index() const
		{ return rtti->type_id; }
	std::string get_type_name() const
		{ return mmoc::signature(rtti->type_id); }
	template<typename T> bool is() const
		{ return std::type_index(typeid(T))==rtti->type_id; }
	bool is_same_type_as(const Any& other) const
		{ return rtti->type_id==other.rtti->type_id; }
	char owns_data() const
		{ return rtti->owns_data; }
};

} //end of namespace mmoc

#endif