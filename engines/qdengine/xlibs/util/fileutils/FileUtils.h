#ifndef __FILE_UTILS_H_INCLUDED__
#define __FILE_UTILS_H_INCLUDED__

#include <string>
using std::string;
void replaceSubString(string& s, const char*src, const char* dest);

bool createDirectory(const char* name);
void removeDirectoryRecursive(const char* dir);

string setExtention(const char* file_name, const char* extention);
string getExtention(const char* file_name);
string cutPathToResource(const char* nameIn);

string extractFileBase(const char* pathName);
string extractFileExt(const char* pathName); // .ext
string extractFileName(const char* pathName); // name.ext
string extractFilePath(const char* pathName);
bool compareFileName(const char* lhs, const char* rhs);
string normalizePath(const char* patch);
string localizePath(const char* path);

bool isFileExists(const char* fileName);

class DirIterator
{
public:
    DirIterator(const char* path = "");
	~DirIterator();

    DirIterator& operator++();
    const DirIterator operator++(int);
    const char* c_str() const;
    const char* operator* () const{ return c_str(); }
	string fullName() const { return path_ + c_str(); }

    bool isDirectory(bool includeHidden = false) const;
    bool isFile(bool includeHidden = false) const;
	operator bool() const;
    bool operator==(const DirIterator& rhs);
    bool operator!=(const DirIterator& rhs);

	static DirIterator end;

private:
	string path_;
    WIN32_FIND_DATA findFileData_;
    HANDLE handle_;
};

class CurrentDirectorySaver{
public:
	CurrentDirectorySaver(bool autoRestore = true);
	~CurrentDirectorySaver();

	void setAutoRestore(bool autoRestore);
	void restore();
	const char* c_str() const{ return directory_.c_str(); }
protected:
	std::string directory_;
	bool autoRestore_;
};

#endif
