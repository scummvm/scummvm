#include "stdafx.h"
#include "LibraryWrapper.h"
#include "Serialization\XPrmArchive.h"
#include "BinaryArchive.h"
#include "InPlaceArchive.h"
#include "LibrariesManager.h"
#include "MultiArchive.h"
#include "profiler.h"
#include "kdw/PropertyEditor.h"

LibraryWrapperBase::LibraryWrapperBase()
{
	flags_ = 0;
	crc_ = 0;
	inPlaceCreated_ = false;
}

LibraryWrapperBase* LibraryWrapperBase::loadLibrary() 
{
	start_timer_auto();

	if(inPlaceEnabled()){
		InPlaceIArchive ia(0);
		if(ia.open(makeBinName(fileName_.c_str(), "..\\ContentBin", "").c_str())){
			LibraryWrapperBase* ptr;
			ia.construct(ptr);
			inPlaceCreated_ = true;
			memcpy(ptr, this, sizeof(LibraryWrapperBase));
			return ptr;
		}
	}

	XPrmIArchive ia(0);
	if(ia.open(fileName_.c_str())){												
		crc_ = ia.crc();
		int versionOld = 0;												
		ia.serialize(versionOld, "Version", 0);							
		ia.setVersion(versionOld);										
		serializeLibrary(ia);								
		if(versionOld != version_)										
			saveLibrary();											

		if(inPlaceEnabled()){
			InPlaceOArchive oa(makeBinName(fileName_.c_str(), "..\\ContentBin", "").c_str(), false);
			serializeLibrary(oa);									
		}
	}

	if(check_command_line(sectionName_)){								
		editLibrary();											
		ErrH.Exit();													
	}	

	return this;
}

void LibraryWrapperBase::saveLibrary() 
{
	XPrmOArchive oa(fileName_.c_str());											
	oa.serialize(version_, "Version", 0);		
	serializeLibrary(oa);									

	if(inPlaceEnabled()){
		InPlaceOArchive oa(makeBinName(fileName_.c_str(), "..\\ContentBin", "").c_str(), false);
		serializeLibrary(oa);									
	}
}

struct LibrarySerializer{
public:
	LibrarySerializer(LibraryWrapperBase* library)
	: library_(library)
	{
	}
	void serialize(Archive& ar){
		library_->serializeLibrary(ar);
	}

	LibraryWrapperBase* library_;
};

bool LibraryWrapperBase::editLibrary(bool translatedOnly) 
{
	string setupName = string("Scripts\\TreeControlSetups\\") + sectionName_ + "State";
	LibrarySerializer lib(this);
	if(kdw::edit(Serializer(lib), setupName.c_str(), translatedOnly ? kdw::ONLY_TRANSLATED : 0)){
		saveLibrary();
		return true;
	}
	return false;														
}

bool LibrariesManager::registerLibrary(const char* name, LibraryInstanceFunc func, bool editor)
{
	if(editor){
		editorLibraries_[name] = func;
	}
	libraries_[name] = func;
	return true;
}

LibrariesManager::LibrariesManager()
{
}

LibraryInstanceFunc LibrariesManager::findInstanceFunc(const char* name)
{
    Libraries::iterator it = libraries_.find(name);
    if(it == libraries_.end())
        return 0;
    else
        return it->second;
}

EditorLibraryInterface* LibrariesManager::find(const char* name)
{
    Libraries::iterator it = libraries_.find(name);
    if(it == libraries_.end())
		return 0;
	else
		return &it->second();
}

LibrariesManager& LibrariesManager::instance()
{
    static LibrariesManager the;
    return the;
}

void EditorLibraryInterface::editorElementErase(const char* name)
{
	int index = editorFindElement(name);
	xassert(index >= 0 && index < editorSize());
	editorElementErase(index);
}

void EditorLibraryInterface::editorElementMoveBefore(const char* name, const char* beforeName)
{
	int index = editorFindElement(name);
	int beforeIndex = beforeName ? editorFindElement(beforeName) : editorSize();
	xassert(index >= 0 && index < editorSize());
	xassert(beforeIndex >= 0 && beforeIndex <= editorSize());
    
	editorElementMoveBefore(index, beforeIndex);
}

void EditorLibraryInterface::editorElementSetName(const char* name, const char* newName)
{
	int index = editorFindElement(name);
	xassert(index >= 0 && index < editorSize());
	editorElementSetName(index, newName);
}

Serializer EditorLibraryInterface::editorElementSerializer(const char* editorName, const char* name, const char* nameAlt, bool protectedName)
{
	int index = editorFindElement(editorName);
	xassert(index >= 0 && index < editorSize());
    return editorElementSerializer(index, name, nameAlt, protectedName);
}

void EditorLibraryInterface::editorElementSetGroup(const char* name, const char* group)
{
	int index = editorFindElement(name);
	xassert(index >= 0 && index < editorSize());
	editorElementSetGroup(index, group);
}

std::string EditorLibraryInterface::editorElementGroup(const char* elementName) const
{
	int index = editorFindElement(elementName);
	xassert(index >= 0 && index < editorSize());
	return editorElementGroup(index);
}

int EditorLibraryInterface::editorFindElement(const char* elementName) const
{
    int count = editorSize();
    for(int i = 0; i < count; ++i)
        if(strcmp(editorElementName(i), elementName) == 0)
            return i;
    return -1;
}
