#include "StdAfx.h"
#include "EnumDescriptor.h"

//////////////////////////////////////////////////
const char* EnumDescriptor::name(int key) const 
{
	KeyToName::const_iterator i = keyToName_.find(key);
	if(i != keyToName_.end())
		return i->second;

	if(!ignoreErrors_)
		xassert(0 && "Unregistered enum key");
	return "";
}

const char* EnumDescriptor::nameAlt(int key) const 
{
	KeyToName::const_iterator i = keyToNameAlt_.find(key);
	if(i != keyToNameAlt_.end())
		return i->second;

	if(!ignoreErrors_)
		xassert(0 && "Unregistered enum key");
	return "";
}

int EnumDescriptor::keyByName(const char* name) const 
{
	NameToKey::const_iterator i = nameToKey_.find(name);
	if(i != nameToKey_.end())
		return i->second;

	if(!ignoreErrors_)
		xassertStr(!strlen(name) && "Unknown enum identificator will be ignored: ",(std::string(typeName_) + "::" + name).c_str());
	return 0;
}

int EnumDescriptor::keyByNameAlt(const char* nameAlt) const 
{
	NameToKey::const_iterator i = nameAltToKey_.find(nameAlt);
	if(i != nameAltToKey_.end())
		return i->second;

	if(!ignoreErrors_)
		xassertStr(!strlen(nameAlt) && "Unknown enum identificator will be ignored: ",(std::string(typeName_) + "::" + nameAlt).c_str());
	return 0;
}

bool EnumDescriptor::nameExists(const char* name) const
{
	return nameToKey_.find(name) != nameToKey_.end();
}

bool EnumDescriptor::nameAltExists(const char* nameAlt) const
{
	return nameAltToKey_.find(nameAlt) != nameAltToKey_.end();
}

string EnumDescriptor::nameCombination(int bitVector, const char* separator) const 
{
	string value;
	for(KeyToName::const_iterator i = keyToName_.begin(); i != keyToName_.end(); ++i)
		if((bitVector & i->first) == i->first){
			bitVector &= ~i->first;
			if(!value.empty())
				value += separator;
			value += i->second;
		}
		return value;
}

void EnumDescriptor::nameCombinationStrings(int bitVector, NameCombination& strings) const 
{
	for(KeyToName::const_iterator i = keyToName_.begin(); i != keyToName_.end(); ++i)
		if((bitVector & i->first) == i->first){
			bitVector &= ~i->first;
			strings.push_back(i->second.c_str());
		}
}

string EnumDescriptor::nameAltCombination(int bitVector, const char* separator) const 
{
	string value;
	for(KeyToName::const_iterator i = keyToNameAlt_.begin(); i != keyToNameAlt_.end(); ++i)
		if(i->second && (bitVector & i->first) == i->first){
			bitVector &= ~i->first;
			if(!value.empty())
				value += separator;
			value += i->second;
		}
		return value;
}

int EnumDescriptor::defaultValue() const 
{ 
	xassert(!keyToName_.empty()); 
	return (int)keyToName_.begin()->first; 
}

void EnumDescriptor::add(int key, const char* name, const char* nameAlt)
{
	//	xassert(name && strlen(name) && stlren(nameAlt));
	keyToName_[key] = name;
	nameToKey_[name] = key;
	if(!comboList_.empty())
		comboList_ += "|";
	comboList_ += name;
	comboStrings_.push_back(name);

	keyToNameAlt_[key] = nameAlt;
	if(nameAlt){
		nameAltToKey_[nameAlt] = key;
		if(!comboListAlt_.empty())
			comboListAlt_ += "|";
		comboListAlt_ += nameAlt;
		comboStringsAlt_.push_back(nameAlt);
	}
}

void EnumDescriptor::clear()
{
	keyToName_.clear();
	keyToNameAlt_.clear();
	nameToKey_.clear();
	nameAltToKey_.clear();
	comboList_ = "";
	comboListAlt_ = "";
	comboStrings_.clear();
	comboStringsAlt_.clear();
}

EnumDescriptor::Key::Key(int value) 
: value_(value) 
{
	value =(value & 0x55555555) +((value >> 1) & 0x55555555);
	value =(value & 0x33333333) +((value >> 2) & 0x33333333);
	value =(value & 0x0f0f0f0f) +((value >> 4) & 0x0f0f0f0f);
	value =(value & 0x00FF00FF) +((value >> 8) & 0x00FF00FF);
	value =(value & 0x0000FFFF) +((value >> 16) & 0x0000FFFF);
	bitsNumber_ = value;
}

bool EnumDescriptor::Key::operator<(const Key& rhs) const 
{
	return bitsNumber_ == rhs.bitsNumber_ ? value_ < rhs.value_ : bitsNumber_ > rhs.bitsNumber_;
}

const EnumDescriptor* EnumDescriptor::descriptorByTypeName(const char* enumTypeName)
{
	NameToEnumDescriptor::const_iterator it = descriptors().find(enumTypeName);
	if(it != descriptors().end())
		return it->second;
	else
		return 0;
}

EnumDescriptor::NameToEnumDescriptor& EnumDescriptor::descriptors()
{
	static NameToEnumDescriptor descriptorsMap;
	return descriptorsMap;
}


