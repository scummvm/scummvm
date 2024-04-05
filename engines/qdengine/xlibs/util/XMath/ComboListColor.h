#ifndef __COMBO_LIST_COLOR_H_INCLUDED__
#define __COMBO_LIST_COLOR_H_INCLUDED__

#include <vector>
#include "XMath\Colors.h"

typedef std::vector<Color4f> ColorContainer;

class ComboListColor
{
public:
    ComboListColor(const ColorContainer& comboList, const Color4f& value)
	: comboList_(comboList){
        index_ = indexOf(value);
	}
	ComboListColor() {
		index_ = 0;
	}
    ComboListColor& operator=(const Color4f& value) { index_ = indexOf(value); return *this; }

    operator const Color4f&() const { return value(); }
    const ColorContainer& comboList() const { return comboList_; }
    void setComboList(const ColorContainer& comboList) { comboList_ = comboList; }

	const Color4f& value() const{
		if(index_ >= 0 && index_ < comboList().size())
			return comboList()[index_];
		else
			return Color4f::WHITE;
	}

	int index() const{ return index_; }
	int indexOf(Color4f value) const{
		for(int i = 0; i < comboList_.size(); ++i){
			if(comboList_[i] == value)
				return i;
		}
		return 0;
	}
	void setIndex(int index){
		index_ = index;
	}
	void serialize (Archive& ar);
private:
	int index_;
    ColorContainer comboList_;
};

#endif
