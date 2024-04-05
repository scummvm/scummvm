#include "StdAfx.h"
#include "MultiArchive.h"

string makeBinName(const char* fileName, const char* subDir, const char* extention)
{
	string str = fileName;
	if(strlen(subDir)){
		unsigned int pos = str.rfind("\\");
		if(pos == string::npos)
			pos = 0;
		str.insert(pos, "\\");
		++pos;
		str.insert(pos, subDir);
	}
	unsigned int pos = str.rfind(".");
	if(pos != string::npos && pos != 0 && str[pos - 1] != '.')
		str.erase(pos, str.size());
	if(strlen(extention)){
		str += ".";
		str += extention;
	}
	return str;
}

MultiOArchive::MultiOArchive(ArchiveType mode)
: mode_(mode)
{}

MultiOArchive::MultiOArchive(const char* fname, const char* binSubDir, const char* binExtention, ArchiveType mode)
: mode_(mode)
{
	open(fname, binSubDir, binExtention);
}

MultiOArchive::~MultiOArchive()
{
	close();
}

void MultiOArchive::open(const char* fname, const char* binSubDir, const char* binExtention)
{
	archives_.push_back(new XPrmOArchive(fname));
	switch(mode_){
	case ARCHIVE_BINARY_TEXT:
		archives_.push_back(new BinaryOArchive(makeBinName(fname, binSubDir, binExtention).c_str()));
		break;
	case ARCHIVE_IN_PLACE_TEXT:
		archives_.push_back(new InPlaceOArchive(makeBinName(fname, binSubDir, binExtention).c_str(), false));
		break;
	}
}

bool MultiOArchive::close()
{
	bool log = true;
	Archives::const_iterator i;
	FOR_EACH(archives_, i)
		log &= (*i)->close();
	archives_.clear();
	return log;
}

MultiIArchive::MultiIArchive(ArchiveType mode)
: ar_(0), crc_(0), mode_(mode)
{}

MultiIArchive::MultiIArchive(const char* fname, const char* binSubDir, const char* binExtention, ArchiveType mode)
: ar_(0), crc_(0), mode_(mode)
{
	open(fname, binSubDir, binExtention);
}

MultiIArchive::~MultiIArchive()
{
	close();
	delete ar_;
}

bool MultiIArchive::open(const char* fname, const char* binSubDir, const char* binExtention)
{
	switch(mode_){
	case ARCHIVE_TEXT: 
	case ARCHIVE_IN_PLACE_TEXT: {
		XPrmIArchive* xar = new XPrmIArchive();
		if(xar->open(fname)){
			ar_ = xar;
			crc_ = xar->crc();
			return true;
		}
		else{
			delete xar;
			return false;
		}
		break;
	}

	case ARCHIVE_BINARY_TEXT: {
		BinaryIArchive* bar = new BinaryIArchive();
		if(bar->open(makeBinName(fname, binSubDir, binExtention).c_str())){
			ar_ = bar;
			crc_ = bar->crc();
			return true;
		}
		else{
			delete bar;
			XPrmIArchive* xar = new XPrmIArchive();
			if(xar->open(fname)){
				ar_ = xar;
				crc_ = xar->crc();
				return true;
			}
			else{
				delete xar;
				return false;
			}
		}
		break;
	}
	}
	return false;
}

bool MultiIArchive::close()
{
	return ar_ ? ar_->close() : false;
}

