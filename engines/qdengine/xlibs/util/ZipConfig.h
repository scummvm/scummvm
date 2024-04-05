#ifndef __ZIP_CONFIG_H__
#define __ZIP_CONFIG_H__

#include "Serialization\StringTableBase.h"
#include "Serialization\StringTable.h"

class ZipConfig : public StringTableBase
{
public:
	typedef void (*ProgressCallback)(int, int, const char*);

	ZipConfig(const char* name = "");
	~ZipConfig();

	void serialize(Archive& ar);

	bool isEmpty() const;

	const char* zipName() const { return zipName_.c_str(); }
	const char* path() const { return path_.c_str(); }
	const char* filesMask() const { return filesMask_.c_str(); }
	const char* excludeFilesMask() const { return excludeFilesMask_.c_str(); }

	int compressionLevel() const { return compressionLevel_; }

	/// инициализация zipов, надо вызывать чтобы ресурсы брались из них
	static bool initArchives();
	/// сборка ресурсов в zipы
	static bool makeArchives(ProgressCallback progressCallback = 0);

private:

	/// имя zip файла
	std::string zipName_;
	/// путь к ресурсам, которые надо поместить в zip
	/// допускается несколько через ";"
	std::string path_;
	/// маска имени фвйлов, которые надо поместить в zip
	/// допускается несколько через ";"
	std::string filesMask_;
	/// маска имени фвйлов, которые не надо класть в zip
	/// допускается несколько через ";"
	std::string excludeFilesMask_;

	/// степень сжатия, 0-9
	int compressionLevel_;
};

typedef StringTable<ZipConfig> ZipConfigTable;

#endif // __ZIP_CONFIG_H__

