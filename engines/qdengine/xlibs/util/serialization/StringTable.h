//////////////////////////////////////////////
//		Ѕиблиотека типов 
//////////////////////////////////////////////
#ifndef __TYPE_LIBRARY_H__
#define __TYPE_LIBRARY_H__

#include "XTL\StaticMap.h"
#include "Serialization\LibraryWrapper.h"
#include "Serialization\Serializer.h" // дл€ редактора

class Archive;

template<class String>
class StringTable : public LibraryWrapper<StringTable<String> >
{
public:
	typedef String StringType;
	typedef vector<String> Strings;
	typedef Strings Map;

	StringTable();

	void add(const String& data);
	void add(const char* name) { add(String(name)); }
	void remove(const char* name);
	bool exists(const char* name) const { return find(name) != -1; }

	void serialize(Archive& ar);

	const String& operator[](int index) const { return strings_[index];	}

	bool empty() const { return strings_.empty(); }

	int size() const { return int(strings_.size()); }

	const Strings& strings() const { return strings_; }
	const Strings& map() const { return strings_; }

	void buildComboList();

	const char* comboList() const {	return comboList_.c_str(); }
	const char* comboList(bool canAlwaysDereference) const { return canAlwaysDereference ? comboListAlwaysDereference_.c_str() : comboList_.c_str(); }

	const String* find(int key) const;
	int find(const char* name) const;
	const char* findCStr(int key) const;

    // дл€ редактора, virtuals:
    const char*         editorComboList() const{ return comboList_.c_str(); }
	const char*         editorGroupsComboList() const{ return String::editorGroupsComboList(); }
	string         editorGroupName(int index) const;
	void				editorGroupMoveBefore(int index, int beforeIndex);
	bool				editorDynamicGroups() const { return String::editorDynamicGroups(); }
	bool				editorAllowRename() const { return String::editorAllowRename(); }
	bool				editorAllowDrag() const { return String::editorAllowDrag(); }
    size_t         editorSize() const { return size(); }

	string			editorAddElement(const char* name, const char* group = "");
	void				editorAddGroup(const char* name);
	void                editorElementRenamed();
	void                editorElementMoveBefore(int index, int beforeIndex);
	void                editorElementSetName(int index, const char* newName);
	const char*         editorElementName(int index) const;
	void				editorElementErase(int index);
	bool				editorElementVisible(int index) const;
	void				editorSort();

	Serializer       editorElementSerializer(int index, const char* name, const char* nameAlt, bool protectedName);
	string         editorElementGroup(int index) const;
	void                editorElementSetGroup(int index, const char* group);

private:
	Strings strings_;
	string comboList_;
	string comboListAlwaysDereference_;
	vector<string> headerStrings_;
};

#endif //__TYPE_LIBRARY_H__
