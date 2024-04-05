#ifndef __MULTI_ARCHIVE_H__
#define __MULTI_ARCHIVE_H__

#include "Handle.h"
#include "Serialization\Serialization.h"
#include "BinaryArchive.h"
#include "Serialization\XPrmArchive.h"
#include "InPlaceArchive.h"

// Многорежимные архивы, для output-архива необходимо при сериализации пробежать по списку возможных архивов.

enum ArchiveType
{
	ARCHIVE_TEXT,
	ARCHIVE_BINARY_TEXT,
	ARCHIVE_IN_PLACE_TEXT,
};

class MultiOArchive 
{
public:
	typedef vector<ShareHandle<Archive> > Archives;

	MultiOArchive(ArchiveType mode);
	MultiOArchive(const char* fname, const char* binSubDir, const char* binExtention, ArchiveType mode);
	~MultiOArchive();

	void open(const char* fname, const char* binSubDir, const char* binExtention); 
	bool close();  // true if there were changes, so file was updated

	template<class T>
	void serialize(T& t, const char* name, const char* nameAlt) {
		Archives::const_iterator i;
		FOR_EACH(archives_, i)
			(*i)->serialize(t, name, nameAlt);
	}

	const Archives& archives() { return archives_; }

private:
	ArchiveType mode_;
	Archives archives_;
};

class MultiIArchive 
{
public:
	MultiIArchive(ArchiveType mode);
	MultiIArchive(const char* fname, const char* binSubDir, const char* binExtention, ArchiveType mode);
	~MultiIArchive();

	bool open(const char* fname, const char* binSubDir, const char* binExtention);  // true if file exists
	bool close();

	void setVersion(int version) { ar_->setVersion(version); } 

	unsigned int crc() { return crc_; }

	template<class T>
	void serialize(T& t, const char* name, const char* nameAlt) {
		ar_->serialize(t, name, nameAlt);
	}

	Archive& archive() { return *ar_; }

protected:
	Archive* ar_;
	unsigned int crc_;
	ArchiveType mode_;
};

string makeBinName(const char* fileName, const char* subDir, const char* extention);

#endif //__MULTI_ARCHIVE_H__
