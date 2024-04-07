#include "StdAfx.h"
#include "ResourceSelector.h"
#include "Serialization\Serialization.h"
#include "FileUtils\FileUtils.h"

bool ExportInterface::export_;
ExportInterface::ModelSelectorCallBack ExportInterface::modelSelectorCallBack_;

ResourceSelector::Options ResourceSelector::DEFAULT_OPTIONS("*.*", "RESOURCE\\", "Will select ANY file", false);
ModelSelector::Options    ModelSelector::DEFAULT_OPTIONS("*.3dx", "Resource\\Models", "Will select location of 3DX model", true, false);

void ExportInterface::export(const char* name)
{
	if(!isExport() || !strlen(name))
		return;
	string dest = string("Output\\") + name;
	bool log = false;
	if(!isFileExists(dest.c_str()) && isFileExists(name)){
		log = CopyFile(name, dest.c_str(), false);
	}
}

bool ResourceSelector::serialize(Archive& ar, const char* name, const char* nameAlt) 
{
	if(ar.isEdit()){
		if(ar.openStruct(*this, name, nameAlt)){
			ar.serialize(fileName_, "fileName", "ָלÿ פאיכא");
			ar.serialize(options_, "options_", 0);
			ar.closeStruct (name);
		}
		return true;
	}
	else{
		if(!fileNamePtr_)
			fileNamePtr_ = &fileName_;
		bool result = ar.serialize(*fileNamePtr_, name, nameAlt);

		if(ExportInterface::isExport() && options_.export)
			ExportInterface::export(fileNamePtr_->c_str());

		fileNamePtr_ = 0;
		return result;
	}
}

bool ModelSelector::serialize(Archive& ar, const char* name, const char* nameAlt) 
{
	if(ar.isEdit()){
		if(ar.openStruct(*this, name, nameAlt)){
			ar.serialize(fileName_, "fileName", "^");
			ar.closeStruct (name);
		}
		return true;
	}
	else{
		if(!fileNamePtr_)
			fileNamePtr_ = &fileName_;
        bool result = ar.serialize(*fileNamePtr_, name, nameAlt);

		if(ExportInterface::isExport() && !fileNamePtr_->empty()){
			xassert(ExportInterface::modelSelectorCallBack());
			ExportInterface::modelSelectorCallBack()(fileNamePtr_->c_str());
		}

		fileNamePtr_ = 0;
		return result;
	}
}



ResourceSelector::Options::Options(const char* _filter, const char* _initialDir, const char* _title, bool _copy /*= true*/, bool _export /*= true*/) 
: filter(_filter), initialDir(_initialDir), title(_title), copy(_copy), export(_export)
{
}

void ResourceSelector::Options::serialize(Archive& ar) 
{
	ar.serialize(filter, "filter", 0);
	ar.serialize(initialDir, "initialDir", 0);
	ar.serialize(title, "title", 0);
	ar.serialize(copy, "copy", 0);
	ar.serialize(export, "export", 0);
}

