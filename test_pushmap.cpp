#include <iostream>
#include "object.hpp"

//alignment:
// <-1 = left (only pad on right; keep to_pad on left)
// -1 = centre-left (centre but on centremost-left if not able to be perfectly in centre)
// 1 = centre-right (centre but on centremost-right if not able to be perfectly in centre)
// 0 or >1 = right (only pad on the left; keep to_pad on right)
std::string pad(const std::string& to_pad,int pad_size,char alignment = 0)
{
	int pad_spaces=pad_size-to_pad.size();
	if(alignment<-1)
		return to_pad+std::string(pad_spaces,' ');
	else if(alignment==-1)
		return std::string(pad_spaces/2,' ')+to_pad+std::string(pad_spaces/2+pad_spaces%2,' ');
	else if(alignment==1)
		return std::string(pad_spaces/2+pad_spaces%2,' ')+to_pad+std::string(pad_spaces/2,' ');
	else return std::string(pad_spaces,' ')+to_pad;
}
template<typename T> bool modulast(T val,T divisor)
	{ return val%divisor==divisor-1; }
//the above two functions are just for helping laying out stuff in a neat table

int main()
{
	std::vector<std::string> options{ "a","aa","ab","ba","bb","b","z","0","1","_" };
	mmoc::PushMap pm1,pm2;
	
	for(int i=0;i<options.size()*options.size();++i)
	{
		pm1.map( options[i/options.size()] , options[i%options.size()] );
		pm2.map(
			options[ options.size()-(i/options.size())-1 ],
			options[ options.size()-(i%options.size())-1 ]
			);
	}
	
	auto print_pushmap=[&](const mmoc::PushMap& push_map_in)
	{
		int i=0; for(auto it : push_map_in)
			std::cout<<pad(it.first,2)<<" => "<<pad(it.second+",",3)+(modulast(i++,8)?"\n":" ");
	};
	
	std::cout<<"First PushMap (pm1)'s contents:\n";
	print_pushmap(pm1);
	std::cout<<"\n\nSecond PushMap (pm2)'s contents:\n";
	print_pushmap(pm2);
	std::cout<<"\n\n";
	
	std::cout<<
		"pm1==pm2 = "<<(pm1==pm2)<<"\n"
		"pm1!=pm2 = "<<(pm1!=pm2)<<"\n"
		"mmoc::PushMap().is_default() = "<<mmoc::PushMap().is_default()<<"\n"
		"mmoc::PushMap().map(\"a\",\"b\").is_default() = "
			<<mmoc::PushMap().map("a","b").is_default()<<"\n"
		"\n"
		;
	
	std::cout<<"Trying mmoc::PushMap().map(\"\",\"aa\")...\n";
	try { pm1.map("","aa"); }
	catch(std::runtime_error& e)
		{ std::cout<<"Caught exception! what(): "<<e.what()<<"\n"; }
	
	std::cout<<"Trying mmoc::PushMap().map(\"aa\",\"\")...\n";
	try { pm1.map("aa",""); }
	catch(std::runtime_error& e)
		{ std::cout<<"Caught exception! what():"<<e.what()<<"\n"; }
	
	std::cout<<"Trying mmoc::PushMap().map(\"\",\"\")...\n";
	try { pm1.map("",""); }
	catch(std::runtime_error& e)
		{ std::cout<<"Caught exception! what():"<<e.what()<<"\n"; }
	
	char close; std::cin>>close;
}
