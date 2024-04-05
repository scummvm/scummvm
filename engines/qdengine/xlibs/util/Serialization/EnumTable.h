#ifndef __SERIALIZATION_ENUM_TABLE_H_INCLUDED__
#define __SERIALIZATION_ENUM_TABLE_H_INCLUDED__

#include <vector>
#include "Serialization.h"
#include "Serialization\EnumDescriptor.h"

template<class Enum, class Type>
class EnumTable : public std::vector<Type>
{
public:
	EnumTable() {
		assign( getEnumDescriptor(Enum()).size(), Type());
	}

	const Type& operator()(Enum enumID) const {
		xassert(getEnumDescriptor(Enum()).name(enumID) && enumID < size());
		return operator[](enumID);
	}

	void serialize(Archive& ar) {
		if(ar.inPlace())
			ar.serialize(static_cast<std::vector<Type> >(*this), 0, 0);
		else {
			const EnumDescriptor& descr = getEnumDescriptor(Enum());
			for(int i = 0; i < size(); i++)
				ar.serialize(at(i), descr.name(Enum(i)), descr.nameAlt(Enum(i)));
		}
	}
};

#endif //__SERIALIZATION_ENUM_TABLE_H_INCLUDED__
