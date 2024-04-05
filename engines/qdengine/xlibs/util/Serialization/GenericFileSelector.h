#ifndef __GENERIC_FILE_SELECTOR_H_INCLUDED__
#define __GENERIC_FILE_SELECTOR_H_INCLUDED__

#include <string>
#include "Serialization\Serialization.h"

class GenericFileSelector {
public:
	struct Options {
		Options(const char* filter, const char* initialDir, const char* title = "", bool onlyInitialDir = false, bool save = false)
			: filter_(filter)
			, initialDir_(initialDir)
			, title_(title)
			, onlyInitialDir_(onlyInitialDir)
			, save_(save)
		{}
		void serialize(Archive& ar) {
			ar.serialize(title_, "title_", 0);
			ar.serialize(filter_, "filter_", 0);
			ar.serialize(initialDir_, "initialDir_", 0);
			ar.serialize(onlyInitialDir_, "onlyInitialDir_", 0);
			ar.serialize(save_, "save_", 0);
		}
		const char* initialDir() const { return initialDir_.c_str(); }

	private:
		friend GenericFileSelector;
		string title_;
		string filter_;
		string initialDir_;
        bool onlyInitialDir_;
		bool save_;
	};
	static Options DEFAULT_OPTIONS;
	explicit GenericFileSelector(string& fileName, const Options& options = DEFAULT_OPTIONS)
	: fileNamePtr_(&fileName)
	, options_(options)
    {
		fileName_ = fileName;
	}
	
	GenericFileSelector()
	: fileNamePtr_(0)
	, options_(DEFAULT_OPTIONS)
	{
	}
	~GenericFileSelector()
	{
		if(fileNamePtr_)
			*fileNamePtr_ = fileName_;
	}
	GenericFileSelector(const GenericFileSelector& original)
	: fileNamePtr_(0)
	, fileName_(original.fileName_)
	, options_(original.options_)
	{
	}

	GenericFileSelector& operator=(const GenericFileSelector& rhs)
	{
		fileName_ = rhs.fileName_;
		if(fileNamePtr_ && !rhs.fileNamePtr_)
			*fileNamePtr_ = rhs.fileName_;
		return *this;
	}

    const char* filter() const { return options_.filter_.c_str(); }
    const char* initialDir() const { return options_.initialDir_.c_str(); }
	const char* title() const { return options_.title_.c_str(); }
	bool onlyInitialDir() const { return options_.onlyInitialDir_; }
	bool save() const {return options_.save_; } 

	void setFileName(const char* fileName) { fileName_ = fileName; }
    operator const char* () const { return fileName_.c_str(); }

    GenericFileSelector& operator= (const char* fileName) {
        fileName_ = fileName;
		return *this;
    }
    GenericFileSelector& operator= (const string& fileName) {
        fileName_ = fileName;
		return *this;
    }

	bool serialize(Archive& ar, const char* name, const char* nameAlt) {
        if(ar.isEdit()) {
			if(ar.openStruct(*this, name, nameAlt)){
				ar.serialize(fileName_, "fileName", "Имя файла");
				ar.serialize(options_, "options_", 0); // нужно для совместимости со старым EditArchive-ов
				ar.closeStruct(name);
			}
			return true;
        }
		else{
			if(fileNamePtr_){
				if(ar.serialize(*fileNamePtr_, name, nameAlt)){
					fileName_ = *fileNamePtr_;
					return true;
				}
				else{
					return false;
				}
			}
			else
				return ar.serialize(fileName_, name, nameAlt);
        }
	}
private:
    string fileName_;
	string* fileNamePtr_;
	const Options& options_;
};

#endif
