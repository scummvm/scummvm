#pragma once

#include <typeinfo>

template<class String>
class StringTable;

template<class T>
class StringTableBasePolymorphic;

template<class String, bool canAlwaysDereference, class StringWrapped = StringTableBasePolymorphic<String> >
class StringTableReferencePolymorphic 
{
public:
	typedef StringTable<StringWrapped> Table;

	StringTableReferencePolymorphic() { key_ = canAlwaysDereference? 0 : -1; }
	explicit StringTableReferencePolymorphic(const char* name);
	explicit StringTableReferencePolymorphic(const String* type);
	
	StringTableReferencePolymorphic& operator=(const String* type) { *this = StringTableReferencePolymorphic(type); return *this; }

	int key() const { return key_; }
	void setKey(int key) { key_ = key; }

	const char* c_str() const;

	bool operator==(const StringTableReferencePolymorphic& rhs) const { return key_ == rhs.key_; }
	bool operator<(const StringTableReferencePolymorphic& rhs) const { return key_ < rhs.key_; }

	bool serialize(Archive& ar, const char* name, const char* nameAlt);

	const String* get() const { return getInternal(); }
	const String* operator->() const { return get(); }
	const String& operator*() const { return *get(); }
	operator const String*() const { return get(); }

	virtual bool refineComboList() const { return false; } 
	virtual bool validForComboList(const String& data) const { return true; }
	virtual const char* editorTypeName() const { return typeid(StringTableReferencePolymorphic).name(); }
	static const char* editorLibraryName();
	
	static bool canAlwaysBeDerefenced() { return canAlwaysDereference; }

private:
	int key_;

	const String* getInternal() const;
};
