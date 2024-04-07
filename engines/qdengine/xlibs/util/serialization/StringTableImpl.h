#ifndef __TYPE_LIBRARY_IMPL_H__
#define __TYPE_LIBRARY_IMPL_H__

#include "Serialization\StringTable.h"
#include "Serialization\StringTableBase.h"
#include "Serialization\StringTableReference.h"
#include "Serialization\StringTableReferencePolymorphic.h"
#include "Serialization\Factory.h"
#include "Console.h"

///////////////////////////////////////////////////////////////
//			StringTable
///////////////////////////////////////////////////////////////
template<class String>
StringTable<String>::StringTable() 
{
	VTableFactory::addVtable(*this);
	add("");
	VTableFactory::addVtable(strings_.back());
	VTableFactory::addVtable(StringTableReference<String, true>());
	VTableFactory::addVtable(StringTableReference<String, false>());
}

template<class String>
int StringTable<String>::find(const char* name) const
{
	Strings::const_iterator i;		
	FOR_EACH(strings_, i)
		if(!strcmp(i->c_str(), name) && i->index_ != -1) // index устанавливается только после полного прочтения
			return i->index_;

	vector<string>::const_iterator si;
	FOR_EACH(headerStrings_, si)
		if(!strcmp(si->c_str(), name))
			return si - headerStrings_.begin();
	
	return -1;
}

template<class String>
void StringTable<String>::add(const String& data) 
{
	const char* name = data.c_str();
	int maxIndex = 0;
	Strings::iterator i;		
	FOR_EACH(strings_, i){
		if(!strcmp(i->c_str(), name))
			return; 
		maxIndex = max(maxIndex, i->index_);
	}

	//xassert(strings_.empty() || strings_.back().index == maxIndex);
	int new_index = !strings_.empty() ? maxIndex + 1 : 0;;
	strings_.push_back(data);
	strings_.back().index_ = new_index;

	if(strings_.size() > 1){
		comboListAlwaysDereference_ += "|";
		comboList_ += "|";
	}
	comboListAlwaysDereference_ += name;
	comboList_ += name;
}

template<class String>
void StringTable<String>::remove(const char* name) 
{
	Strings::iterator i;		
	FOR_EACH(strings_, i)
		if(!strcmp(i->c_str(), name)){
			strings_.erase(i);
			break;
		}
	if(empty())
		add("");

	buildComboList();
}

template<class String>
void StringTable<String>::buildComboList()
{
    comboListAlwaysDereference_ = "";
    Strings::iterator i;		
    FOR_EACH(strings_, i){
        if(i != strings_.begin())
            comboListAlwaysDereference_ += "|";
        comboListAlwaysDereference_ += i->c_str();
    }
    comboList_ = "|";
    comboList_ += comboListAlwaysDereference_;
}

template<class String>
struct StringTableUnwrap
{
	typedef String Type;
	typedef StringTableReference<String, true> ReferenceTrue;
	typedef StringTableReference<String, false> ReferenceFalse;
};

template<class String>
struct StringTableUnwrap<StringTableBasePolymorphic<String> >
{
	typedef String Type;
	typedef StringTableReferencePolymorphic<String, true> ReferenceTrue;
	typedef StringTableReferencePolymorphic<String, false> ReferenceFalse;
};

template<class String>
void StringTable<String>::serialize(Archive& ar) 
{
	if(!ar.isEdit()){
		if(ar.isOutput() && !inPlaceCreated()){
			headerStrings_.clear();
			Strings::iterator i;
			FOR_EACH(strings_, i)
				headerStrings_.push_back(i->c_str());
		}

		ar.serialize(headerStrings_, "header", 0);
	}

	ar.serialize(strings_, "strings", editName_);
	
	if(!inPlaceCreated())
		headerStrings_.clear();

	if(ar.isInput()){
		buildComboList();
		
		if(strings_.empty())
			add("");

		if(!ar.isEdit()){
			int index = 0;
			Strings::iterator i;		
			FOR_EACH(strings_, i)
				i->index_ = index++;
		} else {
            int last_index = 0;
			Strings::iterator i;		
			FOR_EACH(strings_, i)
				if (i->index_ > last_index)
					last_index = i->index_;
			FOR_EACH(strings_, i) {
				if (i->index_ == -1) {
					i->index_ = ++last_index;
				}
			}
		}

		if(ar.isOutput()){ // Инстанцирование кода, никогда не выполняется
			typedef StringTableUnwrap<String>::Type Type;
			static StringTableUnwrap<String>::ReferenceTrue dummyReferenceTrue("");
			dummyReferenceTrue.serialize(ar, "", "");
			static const Type& stringTrue = *dummyReferenceTrue;
			static const char* constCharTrue = dummyReferenceTrue.c_str();

			static StringTableUnwrap<String>::ReferenceFalse dummyReferenceFalse("");
			dummyReferenceFalse.serialize(ar, "", "");
			static const Type& stringFalse = *dummyReferenceFalse;
			static const char* constCharFalse = dummyReferenceFalse.c_str();

			remove("");
		}
	}
}

template<class String>
const String* StringTable<String>::find(int key) const
{
	if(key < 0)
		return 0;
    
	if(key < strings_.size() && strings_[key].index_ == key)
		return &strings_[key];

	Strings::const_iterator i;		
	FOR_EACH(strings_, i)
		if(i->index_ == key)
			return &*i;

	return 0;
}

template<class String>
const char* StringTable<String>::findCStr(int key) const
{
	if(key < 0)
		return "";
    
	if(key < strings_.size() && strings_[key].index_ == key)
		return strings_[key].c_str();

	Strings::const_iterator i;		
	FOR_EACH(strings_, i)
		if(i->index_ == key)
			return i->c_str();

	if(!headerStrings_.empty())
		return headerStrings_[key].c_str();
	else
		return "";
}

template<class String>
Serializer StringTable<String>::editorElementSerializer(int index, const char* name, const char* nameAlt, bool protectedName)
{
	xassert(index >= 0 && index < strings_.size());
	String& str = strings_[index];
	return str.editorSerializer(&str, name, nameAlt, protectedName);
}

template<class String>
std::string StringTable<String>::editorElementGroup(int index) const
{
	xassert(index >= 0 && index < strings_.size());
	const String& str = strings_[index];
	return str.editorGroupName();
}

template<class String>
void StringTable<String>::editorElementSetGroup(int index, const char* group)
{
    xassert(index >= 0 && index < size());

	String& str = strings_[index];
	str.editorSetGroup(group);
}

template<class String>
std::string StringTable<String>::editorGroupName(int index) const
{
    xassert(index >= 0 && index < size());
	const String& str = strings_[index];
	return str.editorGroupName();
}

template<class String>
void StringTable<String>::editorGroupMoveBefore(int index, int beforeIndex)
{
	String::editorGroupMoveBefore(index, beforeIndex);
}

template<class String>
void StringTable<String>::editorElementErase(int index)
{
    xassert(index >= 0 && index < size());
	remove(editorElementName(index));
	buildComboList();
}

template<class String>
void StringTable<String>::editorElementRenamed()
{
	buildComboList();
}

template<class String>
bool StringTable<String>::editorElementVisible(int index) const
{
    xassert(index >= 0 && index < size());
	return strings_[index].editorVisible();
}

template<class String>
const char* StringTable<String>::editorElementName(int index) const
{
    xassert(index >= 0 && index < size());
	return strings_[index].c_str();
}

template<class String>
void StringTable<String>::editorElementSetName(int index, const char* newName)
{
    xassert(index >= 0 && index < size());
    String& str = strings_[index];
    std::string oldName = str.c_str();
    str.setName(newName);

    buildComboList();
}

template<class String>
void StringTable<String>::editorAddGroup(const char* name)
{
	String::editorAddGroup(name);
}

template<class String>
std::string StringTable<String>::editorAddElement(const char* name, const char* group)
{
    int index = size();
    add(name);
	if(index < size()){
		strings_[index].editorCreate(name, group);
		buildComboList();
		return strings_[index].c_str();
	}
	else{
		xassert(0 && "Не могу добавить элемент в StringTable");
		return "";
	}
}

template<class String>
void StringTable<String>::editorElementMoveBefore(int index, int beforeIndex)
{
    String temp;

	xassert(index >= 0 && index < size());
	xassert(beforeIndex >= 0 && beforeIndex <= size());

	temp = strings_[index];
	strings_.erase(strings_.begin() + index);
	if(index <= beforeIndex)
		--beforeIndex;
    strings_.insert(strings_.begin() + beforeIndex, temp);

	buildComboList();
}

struct CStrLess{
	bool operator()(const StringTableBase& a, const StringTableBase& b){
        return stricmp(a.c_str(), b.c_str()) < 0;
	}
	bool operator()(const std::string& a, const std::string& b){
        return stricmp(a.c_str(), b.c_str()) < 0;
	}
};

template<class String>
void StringTable<String>::editorSort()
{
	std::sort(strings_.begin(), strings_.end(), CStrLess());
	buildComboList();
}


///////////////////////////////////////////////////////////////
//			StringTableReference
///////////////////////////////////////////////////////////////
template<class String, bool canAlwaysDereference>
StringTableReference<String, canAlwaysDereference>::StringTableReference(const char* name) 
{
	key_ = StringTable<String>::instance().find(name);
//	xassertStr(!strlen(name) || !strcmp(name, "None") || key_ != -1 && "Не найдена строка в таблице: ", name);
	if(canAlwaysDereference){
		if(key_ == -1)
			key_ = 0;
	}
	else if(!strlen(name))
		key_ = -1;
}

template<class String, bool canAlwaysDereference>
const String* StringTableReference<String, canAlwaysDereference>::getInternal() const 
{
	const String* data = StringTable<String>::instance().find(key_);
	if(data)
		return data;
	else if(canAlwaysDereference){
		xassertStr(0 && "Не найдена строка в таблице, используется первая", StringTable<String>::instance().editName());
		return &StringTable<String>::instance().strings().front();
	}
	else
		return 0;
}

template<class String, bool canAlwaysDereference>
const char* StringTableReference<String, canAlwaysDereference>::c_str() const 
{ 
	return Table::instance().findCStr(key_);
}

template<class String, bool canAlwaysDereference>
const char* StringTableReference<String, canAlwaysDereference>::comboList() const
{
	return canAlwaysDereference ? StringTable<String>::instance().comboListAlwaysDereference_.c_str() : StringTable<String>::instance().comboList_.c_str();
}

template<class String, bool canAlwaysDereference>
const char* StringTableReference<String, canAlwaysDereference>::editorLibraryName()
{
	return Table::instance().name();
}

template<class String, bool canAlwaysDereference>
bool StringTableReference<String, canAlwaysDereference>::serialize(Archive& ar, const char* name, const char* nameAlt) 
{
	if(ar.inPlace())
		return true;
	
	bool isEdit = ar.isEdit();
	if(isEdit && !ar.openStruct(*this, name, nameAlt, editorTypeName()))
		return true;

	ComboListString comboStr(StringTable<String>::instance().comboList(), c_str());
	bool result = ar.serialize(comboStr, name, isEdit ? "&^" : nameAlt);
	if(ar.isInput()){
		*this = StringTableReference(comboStr);
		if(strlen(comboStr) && key_ == -1)
			kdWarning("&Shura", XBuffer() < "Не найдена строка в таблице: " < (const char*)comboStr);
	}

	if(isEdit){
		ar.closeStruct(name);
		return true;
	}
	return result;
}

///////////////////////////////////////////////////////////////
//			StringTableReferencePolymorphic
///////////////////////////////////////////////////////////////
template<class String, bool canAlwaysDereference, class StringWrapped>
StringTableReferencePolymorphic<String, canAlwaysDereference, StringWrapped>::StringTableReferencePolymorphic(const char* name) 
{
	key_ = Table::instance().find(name);
//	xassertStr(!strlen(name) || !strcmp(name, "None") || key_ != -1 && "Не найдена строка в таблице: ", name);
	if(canAlwaysDereference){
		if(key_ == -1)
			key_ = 0;
	}
	else if(!strlen(name))
		key_ = -1;
}

template<class String, bool canAlwaysDereference, class StringWrapped>
const String* StringTableReferencePolymorphic<String, canAlwaysDereference, StringWrapped>::getInternal() const 
{
	const StringWrapped* data = Table::instance().find(key());
	if(data)
		return data->get();
	else if(canAlwaysDereference){
		xassertStr(0 && "Не найдена строка в таблице, используется первая", Table::instance().editName());
		return Table::instance().strings().front().get();
	}
	else
		return 0;
}

template<class String, bool canAlwaysDereference, class StringWrapped>
StringTableReferencePolymorphic<String, canAlwaysDereference, StringWrapped>::StringTableReferencePolymorphic(const String* type)
{
	const Table& table = Table::instance();
	Table::Strings::const_iterator i;		
	FOR_EACH(table.strings(), i)
		if(i->get() == type){
			setKey(i->stringIndex());
			return;
		}

	setKey(-1);
}

template<class String, bool canAlwaysDereference, class StringWrapped>
const char* StringTableReferencePolymorphic<String, canAlwaysDereference, StringWrapped>::c_str() const 
{ 
	return Table::instance().findCStr(key_);
}

template<class String, bool canAlwaysDereference, class StringWrapped>
const char* StringTableReferencePolymorphic<String, canAlwaysDereference, StringWrapped>::editorLibraryName()
{
	return Table::instance().name();
}

template<class String, bool canAlwaysDereference, class StringWrapped>
bool StringTableReferencePolymorphic<String, canAlwaysDereference, StringWrapped>::serialize(Archive& ar, const char* name, const char* nameAlt) 
{
	if(ar.inPlace())
		return true;

	bool isEdit = ar.isEdit();
	if(isEdit && !ar.openStruct(*this, name, nameAlt, editorTypeName()))
		return true;

	ComboListString comboStr(Table::instance().comboList(), c_str());
	bool result = ar.serialize(comboStr, name, isEdit ? "&^" : nameAlt);
	if(ar.isInput()){
		*this = StringTableReferencePolymorphic(comboStr);
		if(strlen(comboStr) && key_ == -1)
			kdWarning("&Shura", XBuffer() < "Не найдена строка в таблице: " < (const char*)comboStr);
	}

	if(isEdit){
		ar.closeStruct(name);
		return true;
	}
	return result;
}


///////////////////////////////////////////////////////////////
//			StringTableBaseSerializerImpl
///////////////////////////////////////////////////////////////
template<class T>
class StringTableBaseSerializerImpl : public SerializerImpl<T>{
public:
	StringTableBaseSerializerImpl(const T& object, const char* name, const char* nameAlt, bool protectName, EditorLibraryInterface* editorLibrary = 0)
	: SerializerImpl<T>(object, name, nameAlt)
	, protectName_(protectName)
	, editorLibrary_(editorLibrary)
	{
	}

    bool serialize(Archive& ar, const char* name, const char* nameAlt){
		std::string stringName;
		if(protectName_)
			stringName = data_.c_str();
		int stringIndex = data_.stringIndex();
		std::string group;

		if(T::editorDynamicGroups())
			group = data_.editorGroupName();

		bool result = ar.serialize(data_, name, nameAlt);

		if(protectName_){
			if(stringName != data_.c_str()){
				data_.setName(stringName.c_str());	
				if(editorLibrary_)
					editorLibrary_->editorElementRenamed();
			}
		}
		data_.setStringIndex(stringIndex);
		if(T::editorDynamicGroups())
			data_.editorSetGroup(group.c_str());
		return result;
	}
protected:
	EditorLibraryInterface* editorLibrary_;
	bool protectName_;
};

template<class T>
Serializer StringTableBase::editorSerializer(T* self, const char* name, const char* nameAlt, bool protectName)
{
	//return Serializer(*self, name, nameAlt);
	Serializer result; 
	result.setImpl(new StringTableBaseSerializerImpl<T>(*self, name, nameAlt, protectName, &StringTable<T>::instance()));
	return result;
}

template<class T>
template<class U>
Serializer StringTableBasePolymorphic<T>::editorSerializer(U* self, const char* name, const char* nameAlt, bool editOnly) 
{
	return type_ ? Serializer(*self->type_, name, nameAlt) : Serializer();
}

template<class T>
void StringTableBasePolymorphic<T>::editorSetGroup(const char* group) 
{
	const char* name = FactorySelector<T>::Factory::instance().nameByNameAlt(group);
	type_ = (strcmp(group, "") == 0) ? 0 : FactorySelector<T>::Factory::instance().find(name).create();
}

template<class T>
void StringTableBasePolymorphic<T>::serialize(Archive& ar) 
{
	StringTableBase::serialize(ar);

	ar.serialize(type_, "|type|second", "&Значение"); // CONVERSION 31.07.07

	if(ar.isInput() && ar.isOutput()){ // Инстанцирование кода, никогда не выполняется
		static StringTableReferencePolymorphic<T, true> dummyTrue((const T*)0);
		static StringTableReferencePolymorphic<T, false> dummyFalse((const T*)0);
	}
}

#endif //__TYPE_LIBRARY_IMPL_H__
