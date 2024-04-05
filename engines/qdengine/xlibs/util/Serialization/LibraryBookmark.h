#ifndef __LIBRARY_BOOKMARK_H_INCLUDED__
#define __LIBRARY_BOOKMARK_H_INCLUDED__

#include "Serialization/Serialization.h"

class LibraryBookmark;
class LibraryLocation{
public:
	virtual bool getBookmark(LibraryBookmark& bookmark) const = 0;
};

class LibraryBookmark{
public:
    LibraryBookmark(const char* libraryName = "")
	: libraryName_(libraryName)
	, elementName_("")
	{
	}

    LibraryBookmark(const char* libraryName, const char* elementName, const ComboStrings& attribEditorPath)
    : libraryName_(libraryName)
    , elementName_(elementName)
    , attribEditorPath_(attribEditorPath)
    {
    }

    const char* elementName() const{
        return elementName_.c_str();
    }
	const char* libraryName() const{
		return libraryName_.c_str();
	}
    const ComboStrings& attribEditorPath() const{
        return attribEditorPath_;
    }

	void setAttribEditorPath(const ComboStrings& attribEditorPath){
		attribEditorPath_ = attribEditorPath;
	}
	const char* subElementName() const{
		return subElementName_.c_str();
	}
    void setElementName(const char* elementName){
        elementName_ = elementName;
    }
	void setSubElementName(const char* subElementName){
		subElementName_ = subElementName;
	}
    void setLibraryName(const char* libraryName){
        libraryName_ = libraryName;
    }
	bool operator==(const LibraryBookmark& rhs) const{
		return elementName_ == rhs.elementName_ && libraryName_ == rhs.libraryName_ && attribEditorPath_ == rhs.attribEditorPath_;
	}
	void serialize(Archive& ar);
private:
    std::string libraryName_;
    std::string elementName_;
	std::string subElementName_;
    ComboStrings attribEditorPath_;
};

#endif
