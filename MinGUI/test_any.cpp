#include <iostream>
#include "any.hpp"

// Note that while I was running the test.cpp file and fixing errors in it I also got to see that
// TypeConflictException's were being thrown only when there were type mismatches so I have
// confirmation that that part also works as intended.

using mmoc::Any;

class BigThing
{
	//sizeof(BigThing)=48 for sizeof(int64_t)=8"
	int64_t a=0,b=0,c=0,d=0,e=0;
	mutable int64_t sum=0;
public:
	BigThing(int64_t a_in,int64_t b_in,int64_t c_in,int64_t d_in,int64_t e_in)
		: a(a_in), b(b_in), c(c_in), d(d_in), e(e_in) {}
	void set_component(int64_t new_val,unsigned index = 0)
	{
		switch(index)
		{
			case 0: a=new_val; break;
			case 1: b=new_val; break;
			case 2: c=new_val; break;
			case 3: d=new_val; break;
			case 4: e=new_val; break;
			default: a=new_val;
		}
	}
	const int64_t get_component(unsigned index = 5) const
	{
		switch(index)
		{
			case 0: return a;
			case 1: return b;
			case 2: return c;
			case 3: return d;
			case 4: return e;
			default: return sum=a+b+c+d+e;
		}
	}
};

int main()
{
	//int's and BigThing's to proxy
	int proxied_int_1=11,proxied_int_2=22;
	BigThing
		proxied_big_thing_1(-1,-2,-3,-4,-5),
		//-1-2-3-4-5 = -15
		proxied_big_thing_2(-2,-3,-4,-5,-6)
		//-1-2-3-4-5+(-1-1-1-1-1) = -15+(-5) = -20
		;
	
	Any //Any's to test
		a_int_1=Any::create_from_data(1),
		a_int_2=Any::create_from_data(2),
		a_big_1=Any::create_from_data(BigThing(1,2,3,4,5)),
		//sum = 1+2+3+4+5 = 15
		a_big_2=Any::create_from_data(BigThing(2,3,4,5,6)),
		//sum = 2+3+4+5+6 = 1+2+3+4+5 + (1+1+1+1+1) = 15 + 5 = 20
		a_autoproxy_1=Any::create_proxy(&proxied_int_1),
		a_autoproxy_2=Any::create_proxy(&proxied_int_2),
		a_customproxy_1=Any::create_proxy<int64_t>(
			&proxied_big_thing_1,
			mmocSETFUNC(BigThing,int64_t) obj->set_component(*new_val); mmocEND,
			mmocGETFUNC(BigThing,int64_t) *ret_by_val=obj->get_component(); mmocEND
		),
		a_customproxy_2=Any::create_proxy<int64_t>(
			&proxied_big_thing_2,
			mmocSETFUNC(BigThing,int64_t) obj->set_component(*new_val); mmocEND,
			mmocGETFUNC(BigThing,int64_t) *ret_by_val=obj->get_component(); mmocEND
		),
		a_big_3=Any::create_from_data(BigThing(3,4,5,6,7)),
		//sum = 15 + 2+2+2+2+2 = 15 + 10 = 25
		a_customproxy_3=Any::create_proxy<int64_t>(
			&a_big_3.get_by_ref<BigThing>(),
			mmocSETFUNC(BigThing,int64_t) obj->set_component(*new_val); mmocEND,
			mmocGETFUNC(BigThing,int64_t) *ret_by_val=obj->get_component(); mmocEND
		);
	
	//checking .get_by_val() and .get_by_cref()
	std::cout
		<<a_int_1.get_by_val<int>()<<" = 1\n"
		<<a_int_2.get_by_cref<int>()<<" = 2\n"
		<<a_big_1.get_by_val<BigThing>().get_component()<<" = 15\n"
		<<a_big_2.get_by_cref<BigThing>().get_component()<<" = 20\n"
		<<a_autoproxy_1.get_by_val<int>()<<" = 11\n"
		<<a_autoproxy_2.get_by_cref<int>()<<" = 22\n"
		<<a_customproxy_1.get_by_val<int64_t>()<<" = -15\n"
		<<a_customproxy_2.get_by_val<int64_t>()<<" = -20\n"
		<<a_big_3.get_by_cref<BigThing>().get_component()<<" = 25\n"
		<<a_customproxy_3.get_by_val<int64_t>()<<" = 25\n"
		"\n"
		;
	
	//checking .set() (and that modification of object returned by .get_by_ref()
	//works in the case where the object is a BigThing)
	a_int_1.set<int>(10);
	a_int_2.set<int>(20);
	a_big_1.get_by_ref<BigThing>().set_component(0); //sum = 14
	a_big_2.get_by_ref<BigThing>().set_component(0); //sum = 18
	a_autoproxy_1.set<int>(a_autoproxy_1.get_by_val<int>()*10); //110
	a_autoproxy_2.set<int>(a_autoproxy_2.get_by_val<int>()*10); //220
	a_customproxy_1.set<int64_t>(0); //-14
	a_customproxy_2.set<int64_t>(0); //-18
	a_customproxy_3.set<int64_t>(0); //22
	
	std::cout
		<<a_int_1.get_by_val<int>()<<" = 10\n"
		<<a_int_2.get_by_ref<int>()<<" = 20\n"
		<<a_big_1.get_by_val<BigThing>().get_component()<<" = 14\n"
		<<a_big_2.get_by_ref<BigThing>().get_component()<<" = 18\n"
		<<proxied_int_1<<" = 110\n"
		<<a_autoproxy_1.get_by_val<int>()<<" = 110\n"
		<<proxied_int_2<<" = 220\n"
		<<a_autoproxy_2.get_by_val<int>()<<" = 220\n"
		<<proxied_big_thing_1.get_component()<<" = -14\n"
		<<a_customproxy_1.get_by_val<int64_t>()<<" = -14\n"
		<<proxied_big_thing_2.get_component()<<" = -18\n"
		<<a_customproxy_2.get_by_val<int64_t>()<<" = -18\n"
		<<a_big_3.get_by_ref<BigThing>().get_component()<<" = 22\n"
		<<a_customproxy_3.get_by_val<int64_t>()<<" = 22\n"
		"\n"
		;
	
	//checking .assign_to()
	a_int_1.assign_to(a_int_2);
	a_big_1.assign_to(a_big_2);
	a_autoproxy_1.assign_to(a_autoproxy_2);
	a_customproxy_1.assign_to(a_customproxy_2); //-14 + -18 = -32
	a_customproxy_2.assign_to(a_customproxy_3); //-18 + 22 = 4
	a_customproxy_3.assign_to(a_customproxy_3); //22 + 22 = 44
	
	std::cout
		<<a_int_1.get_by_val<int>()<<" = 20\n"
		<<a_int_2.get_by_ref<int>()<<" = 20\n"
		<<a_big_1.get_by_val<BigThing>().get_component()<<" = 18\n"
		<<a_big_2.get_by_ref<BigThing>().get_component()<<" = 18\n"
		<<proxied_int_1<<" = 220\n"
		<<a_autoproxy_1.get_by_val<int>()<<" = 220\n"
		<<proxied_int_2<<" = 220\n"
		<<a_autoproxy_2.get_by_ref<int>()<<" = 220\n"
		<<proxied_big_thing_1.get_component()<<" = -32\n"
		<<proxied_big_thing_1.get_component(0)<<" = -18\n"
		<<a_customproxy_1.get_by_val<int64_t>()<<" = -32\n"
		<<proxied_big_thing_2.get_component()<<" = 4\n"
		<<proxied_big_thing_2.get_component(0)<<" = 22\n"
		<<a_customproxy_2.get_by_val<int64_t>()<<" = 4\n"
		<<a_big_3.get_by_ref<BigThing>().get_component()<<" = 44\n"
		<<a_big_3.get_by_ref<BigThing>().get_component(0)<<" = 22\n"
		<<a_customproxy_3.get_by_val<int64_t>()<<" = 44\n"
		;
	for(int i=1;i<=4;++i)
		std::cout<<a_big_3.get_by_ref<BigThing>().get_component(i)<<" = "<<(i+3)<<"\n";
	std::cout<<"\n";
	
	//testing that the modification through object references returned by get_by_ref()
	//works for int's and BigThing's correctly
	a_autoproxy_1.get_by_ref<int>()=1234;
	a_autoproxy_2.get_by_ref<int>()=5678;
	std::cout
		<<a_autoproxy_1.get_by_ref<int>()<<" = 1234\n"
		<<a_autoproxy_2.get_by_ref<int>()<<" = 5678\n"
		;
	for(int i=0;i<=4;++i)
	{
		a_big_3.get_by_ref<BigThing>().set_component(i+1,i);
		std::cout<<a_big_3.get_by_ref<BigThing>().get_component(i)<<" = "<<(i+1)<<"\n";
	}
	std::cout<<"\n";
	
	//printing out the information for all the Any's tested
	std::cout
		<<"a_int_1.get_type_name() = "<<a_int_1.get_type_name()
			<<", owns_data = "<<(int)a_int_1.owns_data()<<"\n"
		<<"a_int_2.get_type_name() = "<<a_int_2.get_type_name()
			<<", owns_data = "<<(int)a_int_2.owns_data()<<"\n"
		<<"a_big_1.get_type_name() = "<<a_big_1.get_type_name()
			<<", owns_data = "<<(int)a_big_1.owns_data()<<"\n"
		<<"a_big_2.get_type_name() = "<<a_big_2.get_type_name()
			<<", owns_data = "<<(int)a_big_2.owns_data()<<"\n"
		<<"a_autoproxy_1.get_type_name() = "<<a_autoproxy_1.get_type_name()
			<<", owns_data = "<<(int)a_autoproxy_1.owns_data()<<"\n"
		<<"a_autoproxy_2.get_type_name() = "<<a_autoproxy_2.get_type_name()
			<<", owns_data = "<<(int)a_autoproxy_2.owns_data()<<"\n"
		<<"a_customproxy_1.get_type_name() = "<<a_customproxy_1.get_type_name()
			<<", owns_data = "<<(int)a_customproxy_1.owns_data()<<"\n"
		<<"a_customproxy_2.get_type_name() = "<<a_customproxy_2.get_type_name()
			<<", owns_data = "<<(int)a_customproxy_2.owns_data()<<"\n"
		<<"a_big_3.get_type_name() = "<<a_big_3.get_type_name()
			<<", owns_data = "<<(int)a_big_3.owns_data()<<"\n"
		<<"a_customproxy_3.get_type_name() = "<<a_customproxy_3.get_type_name()
			<<", owns_data = "<<(int)a_customproxy_3.owns_data()<<"\n"
		"\n"
		;
	
	mmoc::Any
		copied_int=a_int_1, //20
		copied_big=a_big_1, //18
		copied_autoproxy=a_autoproxy_1, //1234
		copied_customproxy=a_customproxy_1 //-32
		;
	
	std::cout
		<<"copied_int.get_by_val<int>() => "
			<<copied_int.get_by_val<int>()<<" = 20\n"
		<<"copied_big.get_by_ref<BigThing>().get_component() => "
			<<copied_big.get_by_ref<BigThing>().get_component()<<" = 18\n"
		<<"copied_autoproxy.get_by_val<int>() => "
			<<copied_autoproxy.get_by_val<int>()<<" = 1234\n"
		<<"copied_customproxy.get_by_val<int64_t>() => "
			<<copied_customproxy.get_by_val<int64_t>()<<" = -32\n"
		"\n"
		;
	
	copied_int.set<int>(copied_int.get_by_val<int>()/2); //20/2=10
	for(int i=0;i<5;++i)
		//before: 18, after: 5*10/2=5*5=25
		copied_big.get_by_ref<BigThing>().set_component(copied_int.get_by_val<int>()/2,i);
	//1234*2=2468, however proxy is not changed so it applies to original too
	copied_autoproxy.set<int>(copied_autoproxy.get_by_val<int>()*2);
	copied_customproxy.set<int64_t>(proxied_big_thing_1.get_component(0)+8); //-32+8=-24
	
	std::cout
		<<a_int_1.get_by_val<int>()<<" = 20\n"
		<<"copied_int.get_by_val<int>() => "
			<<copied_int.get_by_val<int>()<<" = 10\n"
		
		<<a_big_1.get_by_ref<BigThing>().get_component()<<" = 18\n"
		<<"copied_big.get_by_ref<BigThing>().get_component() => "
			<<copied_big.get_by_ref<BigThing>().get_component()<<" = 25\n"
		
		<<a_autoproxy_1.get_by_val<int>()<<" = 2468 "
			"(proxy is not changed so it applies to original too)\n"
		<<"copied_autoproxy.get_by_val<int>() => "
			<<copied_autoproxy.get_by_val<int>()<<" = 2468\n" //proxy is not changed so it applies to original too
		
		<<a_customproxy_1.get_by_val<int64_t>()<<" = -24 "
			"(proxy is not changed so it applies to original too)\n"
		<<"copied_customproxy.get_by_val<int64_t>() => "
			<<copied_customproxy.get_by_val<int64_t>()<<" = -24\n"
		
		"\n"
		
		<<"copied_autoproxy.proxies_same_as(a_autoproxy_1) => "
			<<copied_autoproxy.proxies_same_as(a_autoproxy_1)<<" = 1\n"
		<<"copied_int.proxies_same_as(a_int_2) => "
			<<copied_int.proxies_same_as(a_int_2)<<" = 0\n"
		<<"a_int_1.is_same_type_as(a_autoproxy_2) => "
			<<a_int_1.is_same_type_as(a_autoproxy_2)<<" = 1\n"
		<<"copied_big.is_same_type_as(a_big_2) => "
			<<copied_big.is_same_type_as(a_big_2)<<" = 1\n"
		;
	int new_int_to_proxy=1;
	copied_autoproxy.set_proxy(&new_int_to_proxy);
	std::cout
		<<"copied_autoproxy.get_by_val<int>() => "
			<<copied_autoproxy.get_by_val<int>()<<" = 1\n"
		;
	copied_autoproxy.set<int>(12);
	std::cout
		<<"copied_autoproxy.get_by_val<int>() => "
			<<copied_autoproxy.get_by_val<int>()<<" = 12\n"
		<<"a_autoproxy_1.get_by_val<int>() => "
			<<a_autoproxy_1.get_by_val<int>()<<" = 2468\n"
		<<"copied_autoproxy.proxies_same_as(a_autoproxy_1) => "
			<<copied_autoproxy.proxies_same_as(a_autoproxy_1)<<" = 0\n"
		<<"copied_autoproxy.get_proxy<int>()==&new_int_to_proxy => "
			<<(copied_autoproxy.get_proxy<int>()==&new_int_to_proxy)<<" = 1\n"
		<<"copied_autoproxy.get_proxy<void>()==(void*)&new_int_to_proxy => "
			<<(copied_autoproxy.get_proxy<void>()==(void*)&new_int_to_proxy)
			<<" = 1\n"
		"\n"
		;
	
	char close; std::cin>>close;
}
