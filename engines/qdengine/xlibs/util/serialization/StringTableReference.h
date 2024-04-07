#ifndef __STRING_TABLE_REFERENCE_H__
#define __STRING_TABLE_REFERENCE_H__

class Archive;

template<class String>
class StringTable;

template<class String, bool canAlwaysDereference>
class StringTableReference
{
public:
	typedef StringTable<String> Table;

	StringTableReference() { key_ = canAlwaysDereference? 0 : -1; }
	explicit StringTableReference(const char* name);

	int key() const { return key_; }
	void setKey(int key) { key_ = key; }

	const String* get() const { return getInternal(); }
	const String* operator->() const { return get(); }
	const String& operator*() const { return *get(); }
	operator const String*() const { return get(); }

	const char* c_str() const;

	bool operator==(const StringTableReference& rhs) const { return key_ == rhs.key_; }
	bool operator<(const StringTableReference& rhs) const { return key_ < rhs.key_; }

    const char* comboList() const;
	bool serialize(Archive& ar, const char* name, const char* nameAlt);

	virtual bool refineComboList() const { return false; } 
	virtual bool validForComboList(const String& data) const { return true; }
	virtual const char* editorTypeName() const { return typeid(StringTableReference).name(); }
	static const char* editorLibraryName();
	
	static bool canAlwaysBeDerefenced() { return canAlwaysDereference; }
    
protected:
	int key_;

	const String* getInternal() const;

	friend StringTable<String>;
};

#endif //__STRING_TABLE_REFERENCE_H__
