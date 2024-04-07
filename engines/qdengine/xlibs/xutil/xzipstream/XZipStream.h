#include "XUtil.h"
#include "XZip\unzip.h"
#include <windows.h>

struct XZipStream
{
	struct ZipArchive
	{
		void* handler;
		const char* fname_;
		int	ErrHUsed_;
		
		ZipArchive();

		void setErrHUsed(int err = 1) { ErrHUsed_ = err; }
		int	open(const char* name);
		bool isOpen() const { return handler != INVALID_HANDLE_VALUE; }
		void close();

		const char*	getFileName() const { return fname_; }
		void getTime(unsigned& fdate,unsigned& ftime);
		long size() const; 

		void* getHandler() { return handler; }
	};

	ZipArchive archive_;
	unzFile unzFile_;
	unz_file_info fileInfo_;
	int	ErrHUsed_;
	char* curfname_;
	bool curFileOpened_;

	XZipStream(int err = 1);
	XZipStream(const char* name, int err = 1);
	~XZipStream();

	int	open(const char* name);
	int open(XZipStream* owner);
	void close();
	ZipArchive& archive() { return archive_; }

	int goToFile(const char* name);
	int goToFirstFile();
	int goToNextFile(); 

	const char* getFileName() const { return curfname_; }
	void getTime(unsigned& fdate,unsigned& ftime) const;
	int openCurrentFile();
	bool isCurrentFileOpen() const { return curFileOpened_; }
	int closeCurrentFile();
	long size() const;
	long tell() const; 
	int eof() const;

	XZipStream& operator> (char*);
	XZipStream& operator> (char&);
	XZipStream& operator> (unsigned char&);
	XZipStream& operator> (short&);
	XZipStream& operator> (unsigned short&);
	XZipStream& operator> (int&);
	XZipStream& operator> (unsigned int&);
	XZipStream& operator> (long&);
	XZipStream& operator> (unsigned long&);
	XZipStream& operator> (float&);
	XZipStream& operator> (double&);
	XZipStream& operator> (long double&);

	unsigned long read(void* buf, unsigned long len);
	char* getLine(char* buf, unsigned len);

	int operator! (){ if(archive_.getHandler() != INVALID_HANDLE_VALUE) return 1; else return 0; } // Obsolete
	operator void* (){ if(archive_.getHandler() != INVALID_HANDLE_VALUE) return 0; else return this; } // Obsolete

	template<class T> XZipStream& read(T& v){ read(&v, sizeof(T)); return *this; }
};
