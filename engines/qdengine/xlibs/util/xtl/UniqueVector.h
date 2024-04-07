#ifndef __UNIQUE_VECTOR_H__
#define __UNIQUE_VECTOR_H__

#include <vector>
#include <algorithm>

using namespace std;

template<class T>
class UniqueVector : public vector<T>
{
	typedef vector<T> BaseClass;

public:
	iterator find(const T& element)
	{
		return std::find(begin(), end(), element);
	}

	void add(const T& element)
	{
		if(find(element) == end())
			push_back(element);
	}

	void remove(const T& element)
	{
		iterator it = find(element);
		if(it != end())
			BaseClass::erase(it);
	}

	bool exists(const T& element)
	{
		iterator it = find(element);
		return it != end();
	}

	bool serialize(Archive& ar, const char* name, const char* nameAlt)
	{
		return ar.serialize((BaseClass&)*this, name, nameAlt);
	}
};

#endif //__UNIQUE_VECTOR_H__
