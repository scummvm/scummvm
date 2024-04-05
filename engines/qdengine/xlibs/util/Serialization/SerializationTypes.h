#ifndef __SERIALIZATION_TYPES_H_INCLUDED__
#define __SERIALIZATION_TYPES_H_INCLUDED__

#include <string>
#include "Handle.h"
using namespace std;

class Archive;

/// Строка с редактируемыми значениями из списка
class ComboListString
{
public:
	ComboListString(){}
	ComboListString(const char* comboList, const char* value = "") : comboList_(comboList), value_(value) {}
	
	ComboListString& operator=(const char* value) { value_ = value ? value : ""; return *this; }
	ComboListString& operator=(const string& value) { value_ = value; return *this; }

	operator const char*() const { return value_.c_str(); }
	const char* comboList() const { return comboList_.c_str(); }
	void setComboList(const char* comboList) { comboList_ = comboList; }

	string& value() { return value_; }
	const string& value() const { return value_; }

protected:
	string value_;
	string comboList_;
};

/// Вектор энумерованных бит
template<class Enum>
class BitVector
{
	typedef int Value;
public:
	BitVector(Value value = 0) : value_(value) {}

	operator Value() const { return value_; }

	BitVector& operator |= (Enum value) { value_ |= value; return *this; }
	BitVector& operator |= (Value value) { value_ |= value; return *this; }
	BitVector& operator &= (Value value) { value_ &= value; return *this; }

private:
	Value value_;
};

/// Обертка для сериализации полиморфных указателей по значению enum'а
template<class Enum, class Type, Enum zeroValue, class TypeFactory = Factory<Enum, Type> >
class EnumToClassSerializer
{
public:
	EnumToClassSerializer(Enum key = zeroValue) : type_(0), key_(zeroValue) { 
		setKey(key); 
	}
	EnumToClassSerializer(Enum key, Type* setType) : type_(setType), key_(zeroValue) { 
		xassert(!strcmp(TypeFactory::instance().typeName(key_), typeid(type_).name()));
	}

	void setKey(Enum key) {
		if(key_ != key){
			type_ = key != zeroValue ? TypeFactory::instance().create(key, true) : 0;
			key_ = key;
		}
	}

	Enum key() const { return key_; }
	Type* type() const { return type_; }

	operator Type* () const { return type_; }
	Type* operator->() const { return type_; }
	Type& operator*() const { return *type_; }

private:
	Enum key_;
	ShareHandle<Type> type_;
	friend class Archive;
};

#endif // __SERIALIZATION_TYPES_H_INCLUDED__
