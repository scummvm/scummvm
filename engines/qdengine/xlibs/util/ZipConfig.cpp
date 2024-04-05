#include "StdAfx.h"

#include "Serialization\Serialization.h"
#include "Serialization\RangedWrapper.h"
#include "Serialization\StringTableImpl.h"

#include "ZipConfig.h"

WRAP_LIBRARY(ZipConfigTable, "ZipConfigTable", "Настройки zip архивов", "Scripts\\Content\\ZipConfig", 0, 0);

ZipConfig::ZipConfig(const char* name) : StringTableBase(name),
	filesMask_("*.*"),
	excludeFilesMask_(""),
	compressionLevel_(0)
{
}

ZipConfig::~ZipConfig()
{
}

void ZipConfig::serialize(Archive& ar)
{
	__super::serialize(ar);

	ar.serialize(zipName_, "zipName", "Имя zip файла");
	ar.serialize(path_, "path", "Путь к файлам");
	ar.serialize(filesMask_, "filesMask", "Паковать файлы");
	ar.serialize(excludeFilesMask_, "excludeFilesMask", "Исключить файлы");
	ar.serialize(RangedWrapperi(compressionLevel_, 0, 9), "compressionLevel", "Степень сжатия");
}

bool ZipConfig::isEmpty() const
{
	return zipName_.empty() || path_.empty() || filesMask_.empty();
}

bool ZipConfig::initArchives()
{
	ZipConfigTable::Strings::const_iterator it;
	for(it = ZipConfigTable::instance().strings().begin(); it != ZipConfigTable::instance().strings().end(); ++it){
		if(!it->isEmpty())
			XZipArchiveManager::instance().openArchive(it->zipName());
	}

	return true;
}

namespace ZipConfigCallback{

	static ZipConfig::ProgressCallback totalCallback;
	static int numFiles;
	static int currentFile;

	static void oneFileCallback(int percent, const char* fileName){
		if(totalCallback)
			totalCallback(round((float(currentFile) + float(percent) * 0.01f) / float(numFiles) * 100.0f), percent, fileName);
	}

};

bool ZipConfig::makeArchives(ProgressCallback progressCallback)
{
	ZipConfigTable::Strings::const_iterator it;
	
	ZipConfigCallback::totalCallback = progressCallback;
	ZipConfigCallback::numFiles = ZipConfigTable::instance().strings().size();
	ZipConfigCallback::currentFile = 0;

	for(it = ZipConfigTable::instance().strings().begin(); it != ZipConfigTable::instance().strings().end(); ++it){
		if(!it->isEmpty()){
			XZipArchiveMaker make(it->zipName(), it->path(), it->filesMask(), it->excludeFilesMask(), it->compressionLevel(), true, &ZipConfigCallback::oneFileCallback);
			make.buildArchive();
		}
		++ZipConfigCallback::currentFile;
	}

	return true;
}

