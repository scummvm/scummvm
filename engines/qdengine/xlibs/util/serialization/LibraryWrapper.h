#ifndef __LIBRARY_WRAPPER_H__
#define __LIBRARY_WRAPPER_H__

#include "Serializer.h"
#include "LibrariesManager.h"

enum LibraryMode
{
	LIBRARY_EDITABLE = 1,
	LIBRARY_IN_PLACE = 0
};

class LibraryWrapperBase 
{
public:
	LibraryWrapperBase();
	LibraryWrapperBase* loadLibrary();
	void saveLibrary();
	bool editLibrary(bool translatedOnly = false);

	unsigned int crc() { return crc_; }

	const char* editName() { return editName_; }
	const char* name() { return sectionName_; }
	bool visibleInEditor() const{ return flags_ & LIBRARY_EDITABLE; }
	bool inPlaceEnabled() const { return flags_ & LIBRARY_IN_PLACE; }
	bool inPlaceCreated() const { return inPlaceCreated_; }

protected:
	virtual void serializeLibrary(Archive& ar) = 0;

	int version_;
	const char* sectionName_;
	string fileName_;
	const char* editName_;
	const char* name_;
	unsigned int crc_;
	int flags_;
	bool inPlaceCreated_;

	template<class T>
	friend class LibraryWrapper;
	template<class T>
	friend class LocLibraryWrapper;
	friend struct LibrarySerializer;
};

class EditorLibraryInterface : public LibraryWrapperBase{
public:
	class EditorIterator{
		friend EditorLibraryInterface;
	public:
		Serializer operator*(){
			if(index_ >= int(library_->editorSize())){
				xassert(0);
				return Serializer();
			}
			else
				return library_->editorElementSerializer(index_, "", "", true);
		}
		EditorIterator& operator++(int){
            ++index_;
			return *this;
		}
		EditorIterator operator++(){
			EditorIterator temp = *this;
            ++index_;
			return temp;
		}

		std::string name() const{
			return library_->editorElementName(index_);
		}

		bool operator==(const EditorIterator& rhs)const{
			xassert(library_ == rhs.library_);
			return (index_ == rhs.index_);
		}
		bool operator!=(const EditorIterator& rhs)const{
			xassert(library_ == rhs.library_);
			return (index_ != rhs.index_);
		}
	protected:
		EditorIterator(EditorLibraryInterface* library, int index)
		: library_(library)
		, index_(index)
		{
		}
	private:
		int index_;
		EditorLibraryInterface* library_;
	};
	

	EditorIterator editorBegin(){ return EditorIterator(this, 0); }
	EditorIterator editorEnd(){ return EditorIterator(this, (int)editorSize()); }

    // editor methods
	virtual bool            editorDynamicGroups() const { return false; }
	virtual bool			editorAllowDrag() const{ return false; }
	virtual bool			editorAllowRename() const{ return true; }
    virtual int             editorFindElement(const char* elementName) const;

	virtual std::string     editorAddElement(const char* name, const char* group = "") { return ""; };
	virtual void			editorAddGroup(const char* name) { xassert(0); };
	virtual void            editorElementErase(int index) {}
	void					editorElementErase(const char* name);
	virtual void            editorElementMoveBefore(int index, int beforeIndex) {};
	void					editorElementMoveBefore(const char* name, const char* beforeName);
	virtual void            editorElementRenamed() {}
    
	virtual const char*     editorGroupsComboList() const { return 0; }
	virtual std::string     editorGroupName(int index) const{ return ""; }
	virtual int				editorGroupIndex(const char* name) { return -1; }
	void					editorGroupMoveBefore(int index, int beforeIndex) {}
	virtual const char*     editorComboList() const { return ""; }
	virtual std::size_t     editorSize() const { return 0; }

	virtual bool		    editorElementVisible(int index) const{ return true; }
	virtual const char*     editorElementName(int index) const{ xassert(0); return ""; }
	virtual void            editorElementSetName(int index, const char* newName) { xassert(0); }
	void                    editorElementSetName(const char* oldName, const char* newName);

	virtual Serializer   editorElementSerializer(int index, const char* name, const char* nameAlt, bool protectedName){ return Serializer(); };
    Serializer           editorElementSerializer(const char* elementName, const char* name, const char* nameAlt, bool protectedName);

	virtual std::string     editorElementGroup(int index) const{ return ""; }
	std::string             editorElementGroup(const char* name) const;

	virtual void            editorElementSetGroup(int index, const char* group) {};
	void                    editorElementSetGroup(const char* name, const char* group);
	virtual void			editorSort() {}

};

template<class T>
class LibraryWrapper : public EditorLibraryInterface
{
public:
	void serializeLibrary(Archive& ar);

	static T& instance();
};

#define WRAP_LIBRARY(Type, sectionName, editName, fileName, version, flags)	\
	template<> Type& LibraryWrapper<Type>::instance() {							\
		static Type* t;															\
		if(!t){																	\
			static Type tt;														\
			t = &tt;															\
			t->sectionName_ = sectionName;										\
			t->flags_ = flags;										\
			t->editName_ = editName;											\
			t->fileName_ = fileName;											\
			t->version_ = version;												\
			t = (Type*)t->loadLibrary();										\
		}																		\
		return *t;																\
	}																			\
	template<> void LibraryWrapper<Type>::serializeLibrary(Archive& ar) {		\
		ar.serialize(instance(), sectionName_, "Библиотека");					\
	}																			\
    namespace{                                                                  \
		bool registered_##Type = LibrariesManager::instance().registerLibrary(sectionName, (LibraryInstanceFunc)(&LibraryWrapper<Type>::instance), flags & LIBRARY_EDITABLE);       \
    };																			


#endif //__LIBRARY_WRAPPER_H__
