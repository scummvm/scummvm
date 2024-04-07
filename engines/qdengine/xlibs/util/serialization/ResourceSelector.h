#ifndef __RESOURCE_SELECTOR_H_INCLUDED__
#define __RESOURCE_SELECTOR_H_INCLUDED__

#include <string>
#include "Serialization\Serialization.h"

class ExportInterface
{
public:
	typedef void (*ModelSelectorCallBack)(const char* name);

	static void setExport(bool export, ModelSelectorCallBack modelSelectorCallBack) { export_ = export; modelSelectorCallBack_ = modelSelectorCallBack; }
	static bool isExport() { return export_; }
	static ModelSelectorCallBack modelSelectorCallBack() { return modelSelectorCallBack_; }
	static void export(const char* name);

private:
	static bool export_;
	static ModelSelectorCallBack modelSelectorCallBack_;
};

struct ResourceSelector {
	struct Options {
		Options(const char* _filter, const char* _initialDir, const char* _title = "", bool _copy = true, bool _export = true);
		void serialize (Archive& ar);
		string filter;
        string initialDir;
        string title;
        bool copy;
		bool export;
	};

	ResourceSelector ()
		: fileNamePtr_ (0)
		, options_ (ResourceSelector::DEFAULT_OPTIONS)
	{
	}
	~ResourceSelector()
	{
		if(fileNamePtr_)
			*fileNamePtr_ = fileName_;
	}

	ResourceSelector(const ResourceSelector& original)
	: fileNamePtr_(0)
	, fileName_(original.fileName_)
	, options_(original.options_)
	{
	}
	
	ResourceSelector& operator=(const ResourceSelector& rhs){
		fileName_ = rhs.fileName_;
		if(fileNamePtr_ && !rhs.fileNamePtr_)
			*fileNamePtr_ = rhs.fileName_;
		return *this;
	}
    ResourceSelector& operator= (const std::string& fileName) {
        fileName_ = fileName;
		return *this;
    }

    explicit ResourceSelector (string& fileName, const Options& options = DEFAULT_OPTIONS)
    : fileNamePtr_ (&fileName)
	, options_ (options)
    {
		fileName_ = fileName;
    }

    operator const char* () const {
        return fileName_.c_str ();
    }

	Options& options () {
		return options_;
	}

	const Options& options () const {
		return options_;
	}

    bool serialize(Archive& ar, const char* name, const char* nameAlt);

	void setFileName (const char* fileName) {
		fileName_ = fileName;
	}

	static Options DEFAULT_OPTIONS;

	static void setExport() { export_ = true; }

protected:
    Options options_;
	string* fileNamePtr_;
	string fileName_;
	
	static bool export_;
};

struct ModelSelector : ResourceSelector {
	ModelSelector ()
		: ResourceSelector ()
	{
		fileNamePtr_ = 0;
	}
	ModelSelector (string& fileName, const Options& options = DEFAULT_OPTIONS)
		: ResourceSelector(fileName, options)
	{}
    operator const char* () const {
        return fileName_.c_str ();
    }
    bool serialize(Archive& ar, const char* name, const char* nameAlt);

	static Options DEFAULT_OPTIONS;
};

#endif
