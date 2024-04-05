#include "StdAfx.h"
#include "FileUtils.h"

DirIterator DirIterator::end;

void replaceSubString(string& s, const char*src, const char* dest)
{
	int pos = 0;
	while(1){
		pos = s.find(src, pos);
		if(pos >= s.size())
			break;
		s.replace(pos, strlen(src), dest);
		pos += strlen(dest);
		}
}

bool createDirectory(const char* name)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(name, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
		return CreateDirectory(name, 0) != 0;
	else{
		FindClose(hFind);
		return false;
	}
}

void removeDirectoryRecursive(const char* _dir)
{
	string dir = _dir;
	if(dir == "."){
		xassert(false && "Attempt to delete root directory");
		ErrH.Exit();
		return;
	}
	//dir += "\\";
	DirIterator i((dir + "\\*.*").c_str());
	for(; i != DirIterator::end; ++i){
		if(i.isFile(true))
			DeleteFile(i.fullName().c_str());
		else if(i.isDirectory(true))
			removeDirectoryRecursive(i.fullName().c_str());
	}
	RemoveDirectory(dir.c_str());
}

string setExtention(const char* file_name, const char* extention)
{
	string str = file_name;
	unsigned int pos = str.rfind(".");
	if(pos != string::npos)
		str.erase(pos, str.size());
	if(!*extention)
		return str;
	return str + "." + extention;
}

string getExtention(const char* file_name)
{
	string str = file_name;
	unsigned int pos = str.rfind(".");
	if(pos != string::npos){
		str.erase(0, pos + 1);
		if(str.empty())
			return "";
		strlwr((char*)str.c_str());
		while(isspace(str[str.size() - 1]))
			str.erase(str.size() - 1);
		return str;
	}
	else
		return "";
}

string cutPathToResource(const char* nameIn)
{
	string name = nameIn;
	strlwr((char*)name.c_str());
	size_t pos = name.rfind("scripts\\resource\\");
	if(pos != string::npos){
		name.erase(0, pos);
	}
	else{
		pos = name.rfind("resource\\");
		if(pos != string::npos)
			name.erase(0, pos);
	}
	return name;
}

bool isFileExists(const char* fileName)
{
	DWORD desiredAccess = GENERIC_READ;
	DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	DWORD creationDeposition = OPEN_EXISTING;
	DWORD flagsAndAttributues = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS;
	HANDLE file = CreateFile(fileName, desiredAccess, shareMode, 0, creationDeposition, flagsAndAttributues, 0);
	if(file == INVALID_HANDLE_VALUE)
		return false;
	else{
		CloseHandle(file);
		return true; 
	}
}

string extractFileBase(const char* pathName)
{
	char nameBuf[_MAX_FNAME];
	memset(nameBuf, 0, sizeof(nameBuf));

    _splitpath (pathName, 0, 0, &nameBuf[0], 0);
	return nameBuf;
}

string extractFileExt(const char* pathName)
{
	char extBuf[_MAX_EXT];
	memset(extBuf, 0, sizeof(extBuf));

    _splitpath (pathName, 0, 0, 0, &extBuf[0]);
	strlwr(extBuf);
    return extBuf;
}

string extractFileName(const char* pathName)
{
	char nameBuf[_MAX_FNAME];
	memset(nameBuf, 0, sizeof(nameBuf));
	char extBuf[_MAX_EXT];
	memset(extBuf, 0, sizeof(extBuf));

    _splitpath (pathName, 0, 0, &nameBuf[0], &extBuf[0]);
    return string(nameBuf) + (extBuf);
}

string extractFilePath(const char* pathName)
{
	char driveBuf[_MAX_FNAME];
	memset(driveBuf, 0, sizeof(driveBuf));
	char pathBuf[_MAX_FNAME];
	memset(pathBuf, 0, sizeof(pathBuf));

    _splitpath(pathName, &driveBuf[0], &pathBuf[0], 0, 0);
	return string(&driveBuf[0]) + string(&pathBuf[0]);
}

bool compareFileName(const char* lhs, const char* rhs)
{
	char buffer1[_MAX_PATH];
	char buffer2[_MAX_PATH];
	if(_fullpath(buffer1, lhs, _MAX_PATH) == 0)
		return stricmp(lhs, rhs) == 0;
	if(_fullpath(buffer2, rhs, _MAX_PATH) == 0)
		return stricmp(lhs, rhs) == 0;
	return stricmp(buffer1, buffer2) == 0;
}

string normalizePath(const char* path)
{
	if(path==0)
		return string();

	if(path[0]=='.' && (path[1]=='\\' || path[1]=='/'))
	{
		path+=2;
	}

	string out_path;
	out_path.reserve(strlen(path));
	for(const char* p=path;*p;p++)
	{
		if(p==path)//Для сетевых путей, типа \\CENTER\MODELS\...
		{
			out_path+=*p;
			continue;
		}

		if(*p!='\\')
			out_path+=*p;
		else
		{
			while(p[1]=='\\')
				p++;
			out_path+=*p;
		}
	}
	strupr((char*)out_path.c_str());
	xassert(out_path.size() < _MAX_PATH);
	return out_path;

	/*
	string result;
	if(!patch){
		xassert(patch);
		return string();
	}

	if(patch[0] == '.' && (patch[1] == '\\' || patch[1] == '/'))
		patch += 2;

	result.reserve(strlen(patch));
	for(const char* p = patch; *p ;p++){
		if(p == patch) // Для сетевых путей, типа \\CENTER\MODELS\...
		{
			result += *p;
			continue;
		}
		if(*p != '\\')
			result += *p;
		else{
			while(p[1] == '\\')
				++p;
			result += *p;
		}
	}
	char* loweredString = strlwr(strdup(result.c_str()));
	result = loweredString;
	free(loweredString);
	xassert(result.size() < _MAX_PATH);
	return result;
	*/
}

string localizePath(const char* path)
{
	string ret;
	ret.reserve(strlen(path));

	char cur_path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, cur_path);

	char full_path[MAX_PATH];
	_fullpath(full_path, cur_path, MAX_PATH);
	_fullpath(cur_path, path, MAX_PATH);

	strlwr(cur_path);
	strlwr(full_path);

	if(!strncmp(full_path, cur_path, strlen(full_path)))
		ret = cur_path + strlen(full_path) + 1;
	else
		ret = cur_path;

	return ret;
}

// --------------------------------------------------------------------------
DirIterator::DirIterator(const char* path)
{
	ZeroMemory(&findFileData_, sizeof(findFileData_));
	if(path){
		handle_ = FindFirstFile(path, &findFileData_);
		while(handle_ != INVALID_HANDLE_VALUE){
			if(strcmp(c_str(), "..") == 0 || strcmp(c_str(), ".") == 0){
				if(FindNextFile (handle_, &findFileData_) == false)
					handle_ = INVALID_HANDLE_VALUE;
			}
			else
				break;
		}
	}
	else
		handle_ = INVALID_HANDLE_VALUE;
	path_ = path;
	int pos = path_.rfind("\\");
	if(pos != string::npos)
		path_.erase(pos + 1);
}

DirIterator::~DirIterator()
{
	if(handle_ != INVALID_HANDLE_VALUE)
		FindClose (handle_);
}

DirIterator& DirIterator::operator++()
{
	xassert(handle_ && handle_ != INVALID_HANDLE_VALUE && "Incrementing bad DirIterator!");
	if (FindNextFile (handle_, &findFileData_) == false) {
		handle_ = INVALID_HANDLE_VALUE;
	}
	return *this;
}

const DirIterator DirIterator::operator++(int)
{
	DirIterator oldValue (*this);
	++(*this);
	return oldValue;
}

const char* DirIterator::c_str() const
{
	xassert (handle_ != INVALID_HANDLE_VALUE && "Dereferencing bad DirIterator!");
	return findFileData_.cFileName;
}

bool DirIterator::isDirectory(bool includeHidden) const
{
	if(!(findFileData_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !strlen(findFileData_.cFileName) || findFileData_.cFileName[0] == '..')
		return false;
	return includeHidden || findFileData_.cFileName[0] != '.';
}

DirIterator::operator bool() const
{
	return handle_ && handle_ != INVALID_HANDLE_VALUE;
}

bool DirIterator::operator==(const DirIterator& rhs)
{
	return handle_ == rhs.handle_;
}

bool DirIterator::operator!=(const DirIterator& rhs)
{
	return handle_ != rhs.handle_;
}

bool DirIterator::isFile(bool includeHidden) const
{
	if(findFileData_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY || !strlen(findFileData_.cFileName) || findFileData_.cFileName[0] == '..')
		return false;
	return includeHidden || findFileData_.cFileName != ".";
}

// --------------------------------------------------------------------------- 
CurrentDirectorySaver::CurrentDirectorySaver(bool autoRestore)
: autoRestore_(autoRestore)
{
	char buffer[_MAX_PATH + 1];
	GetCurrentDirectory(_MAX_PATH, buffer);
	directory_ = buffer;
}

CurrentDirectorySaver::~CurrentDirectorySaver()
{
	if(autoRestore_)
		restore();
}

void CurrentDirectorySaver::setAutoRestore(bool autoRestore)
{
	autoRestore_ = autoRestore;
}

void CurrentDirectorySaver::restore()
{
	SetCurrentDirectory(directory_.c_str());
}

