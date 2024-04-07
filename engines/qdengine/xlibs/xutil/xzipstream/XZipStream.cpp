#include "XZipStream.h"

#define CONV_BUFFER_LEN	63
char ConvertBuffer[CONV_BUFFER_LEN + 1];

static char* openMSG	 = "OPEN FAILURE(file don't exist or in not valid)";
static char* openInMSG	 = "OPEN FAILURE FILE IN ARCHIVE";
static char* closeInMSG  = "CLOSE FAILURE FILE IN ARCHIVE";
static char* closeMSG	 = "CLOSE FAILURE";
static char* appendMSG	 = "APPENDING FAILURE";
static char* sizeMSG	 = "FILE SIZE CALCULATION ERROR";
static char* surfMSG	 = "SURFING IN ARCHIVE ERROR";
static char* locateMSG   = "LOCATE FILE ERROR";
static char *timeMSG	 = "FILE GET TIME ERROR";

XZipStream::XZipStream(int err)
{
	ErrHUsed_ = err;
	unzFile_ = NULL;
	curfname_ = new char[256];
	curFileOpened_ = false;

	archive_.setErrHUsed(err);
}

XZipStream::~XZipStream()
{
	close();
	if(curfname_)
		delete curfname_;
}

XZipStream::XZipStream(const char* name, int err)
{
	unzFile_ = NULL;
	ErrHUsed_ = err;
	curfname_ = new char[256];
	archive_.setErrHUsed(err);
	curFileOpened_ = false;

	open(name);
}

void XZipStream::getTime(unsigned& fdate,unsigned& ftime) const
{
	uLong ulDosDate = fileInfo_.dosDate;
	fdate = ulDosDate >> 16;
	ulDosDate = fileInfo_.dosDate;
	ftime = ulDosDate & 0xFFFF;
}

void XZipStream::ZipArchive::getTime(unsigned& fdate,unsigned& ftime)
{
	fdate=0; ftime=0;
	unsigned short dt,tm;

	FILETIME ft;
	if (!GetFileTime(handler, 0, 0, &ft))
		if (ErrHUsed_) 
			ErrH.Abort(timeMSG, XERR_USER, GetLastError(), fname_);
		else
		    return;
	if (!FileTimeToDosDateTime(&ft, &dt, &tm))
		if (ErrHUsed_) 
			ErrH.Abort(timeMSG, XERR_USER, GetLastError(), fname_);
		else
		     return;

	fdate = dt;
	ftime = tm;
}
int XZipStream::open(const char* name)
{
	if((unzFile_ = unzOpen(name)) == NULL)
		if(ErrHUsed_) 
			ErrH.Abort(openMSG, XERR_USER, 0, name);
		else 
			return 0;

	int err = unzGetCurrentFileInfo(unzFile_, &fileInfo_, curfname_, 256, NULL, 0, NULL, 0);
    if (err != UNZ_OK)
    {
		if(ErrHUsed_) 
			ErrH.Abort(openMSG, XERR_USER, 0, name);
		else 
			return 0;
    }

	curFileOpened_ = false;

	if(!archive_.open(name))
		return 0;

	return 1;
}

int XZipStream::openCurrentFile()
{
	if(unzOpenCurrentFile(unzFile_) == UNZ_OK)
	{
		curFileOpened_ = true;
		return 1;
	}

	if(ErrHUsed_) 
		ErrH.Abort(openInMSG, XERR_USER, 0, curfname_);
	else 
		return 0;

	return 0;
}

int XZipStream::closeCurrentFile()
{
	if(unzCloseCurrentFile(unzFile_) == UNZ_OK)
	{
		curFileOpened_ = false;
		return 1;
	}

	if(ErrHUsed_) 
		ErrH.Abort(closeInMSG, XERR_USER, 0, curfname_);
	else 
		return 0;

	return 0;
}

XZipStream::ZipArchive::ZipArchive()
{
	ErrHUsed_ = 1;
	fname_ = NULL;
	handler = INVALID_HANDLE_VALUE;
}

int XZipStream::ZipArchive::open(const char* name)
{
	handler = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(handler == INVALID_HANDLE_VALUE)
		if(ErrHUsed_) 
			ErrH.Abort(openMSG, XERR_USER, GetLastError(), name);
		else 
			return 0;

	fname_ = name;

	return 1;
}

void XZipStream::ZipArchive::close()
{
	if(handler == INVALID_HANDLE_VALUE) 
		return;

	if(!CloseHandle(handler) && ErrHUsed_)
		ErrH.Abort(closeMSG, XERR_USER, GetLastError(), fname_);

	handler = INVALID_HANDLE_VALUE;
	fname_ = NULL;
}

void XZipStream::close()
{
	if(unzFile_ == NULL) return;

	int errCode;
	if((errCode = unzClose(unzFile_)) != UNZ_OK && ErrHUsed_)
		ErrH.Abort(closeMSG, XERR_USER, errCode, curfname_);

	curFileOpened_ = false;
	unzFile_ = NULL;

	archive_.close();
}

long XZipStream::ZipArchive::size() const
{
	long tmp=GetFileSize(handler,0);
	if (tmp == -1L)
		if (ErrHUsed_) 
			ErrH.Abort(sizeMSG, XERR_USER, GetLastError(), fname_);
		else 
			return -1;

	return tmp;
}

long XZipStream::size() const
{
	return fileInfo_.uncompressed_size;
}

int XZipStream::goToFirstFile()
{
	if(curFileOpened_){ 
		closeCurrentFile();
	}	
	if(unzGoToFirstFile(unzFile_) == UNZ_OK)
	{
		int err = unzGetCurrentFileInfo(unzFile_, &fileInfo_, curfname_, 256, NULL, 0, NULL, 0);
		if (err != UNZ_OK)
		{
			if(ErrHUsed_) 
				ErrH.Abort(openMSG, XERR_USER, 0, archive_.fname_);
			else 
				return 0;
		}
		
		curFileOpened_ = false;
		return 1;
	}
	else
	{
		if(curFileOpened_){ 
			openCurrentFile();
		}	

		if(ErrHUsed_) 
			ErrH.Abort(surfMSG, XERR_USER, 0, archive_.fname_);
		else 
			return 0;

		return 0;
	}
}

int XZipStream::goToNextFile()
{
	if(curFileOpened_){ 
		closeCurrentFile();
	}	 
	int error = unzGoToNextFile(unzFile_);
	if(error == UNZ_OK)
	{
		int err = unzGetCurrentFileInfo(unzFile_, &fileInfo_, curfname_, 256, NULL, 0, NULL, 0);
		if (err != UNZ_OK)
		{
			if(ErrHUsed_) 
				ErrH.Abort(openMSG, XERR_USER, 0, archive_.fname_);
			else 
				return 0;
		}

		curFileOpened_ = false;
		return 1;
	}
	else
	{
		if(curFileOpened_){ 
			openCurrentFile();
		}	 

		if(error != UNZ_END_OF_LIST_OF_FILE){
			if(ErrHUsed_) 
				ErrH.Abort(surfMSG, XERR_USER, 0, archive_.fname_);
			else 
				return 0;
		}
		
		return 0;
	}

}

int XZipStream::goToFile(const char* name)
{
	if(curFileOpened_){ 
		closeCurrentFile();
	}	 

	if(unzLocateFile(unzFile_, name, 0) == UNZ_OK)
	{
		int err = unzGetCurrentFileInfo(unzFile_, &fileInfo_, curfname_, 256, NULL, 0, NULL, 0);
		if (err != UNZ_OK)
		{
			if(ErrHUsed_) 
				ErrH.Abort(openMSG, XERR_USER, 0, archive_.fname_);
			else 
				return 0;
		}
		
		curFileOpened_ = false;
		return 1;
	}

	if(curFileOpened_){ 
		openCurrentFile();
	}

	if(ErrHUsed_) 
		ErrH.Abort(locateMSG, XERR_USER, 0, name);
	else 
		return 0;

	return 0;
}

long XZipStream::tell() const
{ 
	return unztell(unzFile_); 
}

int XZipStream::eof() const
{
	return unzeof(unzFile_);
}

unsigned long XZipStream::read(void* buf, unsigned long len)
{
	int ret = unzReadCurrentFile(unzFile_, buf, len);
	if(ret > 0)
		return ret;
	else
		return 0;
}

char* XZipStream::getLine(char* buf, unsigned len)
{
	int i = -1;
	do {
		i++;
		read(&buf[i],1);
	   } while(buf[i] != '\n' && i < (int)len && !eof());
	if(eof())
		buf[i] = '\0';
	else
		buf[i - 1] = '\0';
	return buf;
}

XZipStream& XZipStream::operator> (char* v)
{
	read(v,(unsigned)strlen(v));
	return *this;
}

XZipStream& XZipStream::operator> (char& v)
{
	read(&v,(unsigned)sizeof(char));
	return *this;
}

XZipStream& XZipStream::operator> (unsigned char& v)
{
	read(&v,(unsigned)sizeof(char));
	return *this;
}

XZipStream& XZipStream::operator> (short& v)
{
	read(&v,(unsigned)sizeof(short));
	return *this;
}

XZipStream& XZipStream::operator> (unsigned short& v)
{
	read(&v,(unsigned)sizeof(unsigned short));
	return *this;
}

XZipStream& XZipStream::operator> (int& v)
{
	read(&v,(unsigned)sizeof(int));
	return *this;
}

XZipStream& XZipStream::operator> (unsigned& v)
{
	read(&v,(unsigned)sizeof(unsigned));
	return *this;
}

XZipStream& XZipStream::operator> (long& v)
{
	read(&v,(unsigned)sizeof(long));
	return *this;
}

XZipStream& XZipStream::operator> (unsigned long& v)
{
	read(&v,(unsigned)sizeof(unsigned long));
	return *this;
}

XZipStream& XZipStream::operator> (double& v)
{
	read(&v,(unsigned)sizeof(double));
	return *this;
}

XZipStream& XZipStream::operator> (float& v)
{
	read(&v,(unsigned)sizeof(float));
	return *this;
}

XZipStream& XZipStream::operator> (long double& v)
{
	read(&v,(unsigned)sizeof(long double));
	return *this;
}

int XZipStream::open(XZipStream* owner)
{
	archive_.fname_ = owner->archive_.fname_;
	open(archive_.fname_);
	strcpy(curfname_, owner->curfname_);
	if(curfname_ != NULL){
		goToFile(curfname_);
		curFileOpened_ = owner->curFileOpened_;
		if(curFileOpened_)
			openCurrentFile();
	}
	if(curFileOpened_){
		int i = 0;
		while(i != owner->tell())
		{
			char tmp;
			read(&tmp, 1);
			i++;
		}
	}

	return 1;
}