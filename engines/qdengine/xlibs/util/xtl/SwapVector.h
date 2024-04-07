#ifndef __SWAP_VECTOR_H__
#define __SWAP_VECTOR_H__

// Вектор с ускоренным erase 
// для несортированных последовательностей
template<class T>
class SwapVector : public vector<T>
{
public:
	iterator erase(iterator it) {
		iterator last = end();
		--last;
        if(it != last){
			std::swap(*it, *last);
			int index = it - begin();
			pop_back();
			return begin() + index;
		}
		else{
			pop_back();
			return end();
		}
	}
	
	void erase(const T& t) {
		iterator i = std::find(begin(), end(), t);
		if(i != end())
			erase(i);
		xassert(std::find(begin(), end(), t) == end());
	}

	bool serialize(Archive& ar, const char* name, const char* nameAlt) {
		return ar.serialize(static_cast<vector<T>&>(*this), name, nameAlt);
	}
};

#endif // __SWAP_VECTOR_H__

