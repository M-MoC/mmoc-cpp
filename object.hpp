#ifndef mmocOBJECT_HPP
#define mmocOBJECT_HPP

#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include "throw.hpp"

namespace mmoc
{

class PushMap
{
	friend bool operator== (const PushMap&,const PushMap&);
	friend bool operator!= (const PushMap&,const PushMap&);
	
	//pair.first = attribute to push , pair.second = attribute to push to
	//a push is the reverse of an assign, A.push(B) is identical to B.assign_to(A)
	using Container = std::vector<std::pair<std::string,std::string>>;
	Container push_map;
	
public:
	using value_type = Container::value_type;
	using reference = Container::reference;
	using const_reference = Container::const_reference;
	using iterator = Container::iterator;
	using const_iterator = Container::const_iterator;
	using difference_type = Container::difference_type;
	using size_type = Container::size_type;
	
	PushMap& map(const std::string& from,const std::string& to)
	{
		if(from.empty() || to.empty())
			mmocTHROW(std::runtime_error, "in a PushMap, you may not map to or from"
				" an empty string (from = \""+from+"\", to = \""+to+"\")"
				);
		
		value_type to_add(from,to);
		
		iterator it=push_map.begin();
		bool already_inserted=false;
		while(it!=push_map.end())
		{
			if(*it<to_add)
				++it;
			else
			{
				if(*it>to_add)
					{ push_map.insert(it,to_add); already_inserted=true; }
				break;
			}
		}
		if(!already_inserted)
			push_map.insert(it,to_add);
		
		return *this;
	}
	
	iterator begin()
		{ return push_map.begin(); }
	const_iterator begin() const
		{ return push_map.begin(); }
	const_iterator cbegin() const
		{ return push_map.cbegin(); }
	iterator end()
		{ return push_map.end(); }
	const_iterator end() const
		{ return push_map.end(); }
	const_iterator cend() const
		{ return push_map.cend(); }
	//we use empty maps (meaning the mapping is default) to signify that for each attribute
	//that is named [a], it will "push" exactly and only to the attribute named [a] (on the
	//condition that attribute [a] exists both in the 'pusher' and the 'pushee' of course)
	bool is_default() const
		{ return push_map.empty(); }
	
	std::string operator[] (const std::string& key) const
		{ for(auto it : push_map) if(it.first==key) return it.second; return ""; }
};
bool operator== (const PushMap& left,const PushMap& right)
	{ return left.push_map==right.push_map; }
bool operator!= (const PushMap& left,const PushMap& right)
	{ return left.push_map!=right.push_map; }

} //end of namespace mmoc

#endif