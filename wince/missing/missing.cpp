/* MISSING.C
   Implementation for standard and semi-standard C library calls missing in WinCE
   environment.
   (C) 2001 Vasyl Tsvirkunov
*/
#include <windows.h>
#include <tchar.h>
#include <string.h>
#include "sys/stat.h"
#include "sys/time.h"
#include "time.h"
#include "dirent.h"

/* forward declaration */
char *strdup(const char *strSource);

/* Limited dirent implementation. Used by UI.C and DEVICES.C */
static WIN32_FIND_DATA wfd;

DIR* opendir(const char* fname)
{
	DIR* pdir;
	char fnameMask[MAX_PATH+1];
	TCHAR fnameUnc[MAX_PATH+1];
	char nameFound[MAX_PATH+1];
	
	if(fname == NULL)
		return NULL;
	
	strcpy(fnameMask, fname);
	if(!strlen(fnameMask) || fnameMask[strlen(fnameMask)-1] != '\\')
		strncat(fnameMask, "\\", MAX_PATH-strlen(fnameMask)-1);
	strncat(fnameMask, "*.*", MAX_PATH-strlen(fnameMask)-4);
	
	pdir = (DIR*)malloc(sizeof(DIR)+strlen(fname));
	pdir->dd_dir.d_ino = 0;
	pdir->dd_dir.d_reclen = 0;
	pdir->dd_dir.d_name = 0;
	pdir->dd_dir.d_namlen = 0;
	
	pdir->dd_handle = 0;
	pdir->dd_stat = 0;
	strcpy(pdir->dd_name, fname); /* it has exactly enough space for fname and nul char */
	
	MultiByteToWideChar(CP_ACP, 0, fnameMask, -1, fnameUnc, MAX_PATH);
	if((pdir->dd_handle = (long)FindFirstFile(fnameUnc, &wfd)) == (long)INVALID_HANDLE_VALUE)
	{
		free(pdir);
		return NULL;
	}
	else
	{
		WideCharToMultiByte(CP_ACP, 0, wfd.cFileName, -1, nameFound, MAX_PATH, NULL, NULL);
		
		pdir->dd_dir.d_name = strdup(nameFound);
		pdir->dd_dir.d_namlen = strlen(nameFound);
	}
	return pdir;
}

struct dirent*	readdir(DIR* dir)
{
	char nameFound[MAX_PATH+1];
	static struct dirent dummy;
	
	if(dir->dd_stat == 0)
	{
		dummy.d_name = ".";
		dummy.d_namlen = 1;
		dir->dd_stat ++;
		return &dummy;
	}
	else if(dir->dd_stat == 1)
	{
		dummy.d_name = "..";
		dummy.d_namlen = 2;
		dir->dd_stat ++;
		return &dummy;
	}
	else if(dir->dd_stat == 2)
	{
		dir->dd_stat++;
		return &dir->dd_dir;
	}
	else
	{
		if(FindNextFile((HANDLE)dir->dd_handle, &wfd) == 0)
		{
			dir->dd_stat = -1;
			return NULL;
		}
		WideCharToMultiByte(CP_ACP, 0, wfd.cFileName, -1, nameFound, MAX_PATH, NULL, NULL);
		
		if(dir->dd_dir.d_name)
			free(dir->dd_dir.d_name);
		
		dir->dd_dir.d_name = strdup(nameFound);
		dir->dd_dir.d_namlen = strlen(nameFound);
		
		dir->dd_stat ++;
		
		return &dir->dd_dir;
	}
}

int closedir(DIR* dir)
{
	if(dir == NULL)
		return 0;
	
	if(dir->dd_handle)
		FindClose((HANDLE)dir->dd_handle);
	
	if(dir->dd_dir.d_name)
		free(dir->dd_dir.d_name);
	free(dir);
	return 1;
}

/* Very limited implementation of stat. Used by UI.C, MEMORY-P.C (latter is not critical) */
int stat(const char *fname, struct stat *ss)
{
	TCHAR fnameUnc[MAX_PATH+1];
	HANDLE handle;
	int len;
	
	if(fname == NULL || ss == NULL)
		return -1;
	
	/* Special case (dummy on WinCE) */
	len = strlen(fname);
	if(len >= 2 && fname[len-1] == '.' && fname[len-2] == '.' &&
		(len == 2 || fname[len-3] == '\\'))
	{
		/* That's everything implemented so far */
		memset(ss, 0, sizeof(struct stat));
		ss->st_size = 1024;
		ss->st_mode |= S_IFDIR;
		return 0;
	}
	
	MultiByteToWideChar(CP_ACP, 0, fname, -1, fnameUnc, MAX_PATH);
	handle = FindFirstFile(fnameUnc, &wfd);
	if(handle == INVALID_HANDLE_VALUE)
		return -1;
	else
	{
		/* That's everything implemented so far */
		memset(ss, 0, sizeof(struct stat));
		ss->st_size = wfd.nFileSizeLow;
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			ss->st_mode |= S_IFDIR;
		
		FindClose(handle);
	}
	return 0;
}

/* Remove file by name */
int remove(const char* path)
{
	TCHAR pathUnc[MAX_PATH+1];
	MultiByteToWideChar(CP_ACP, 0, path, -1, pathUnc, MAX_PATH);
	return !DeleteFile(pathUnc);
}

/* in our case unlink is the same as remove */
int unlink(const char* path)
{
	return remove(path);
}

/* Make directory, Unix style */
void mkdir(char* dirname, int mode)
{
	char  path[MAX_PATH+1];
	TCHAR pathUnc[MAX_PATH+1];
	char* ptr;
	strncpy(path, dirname, MAX_PATH);
	if(*path == '/')
		*path = '\\';
	/* Run through the string and attempt creating all subdirs on the path */
	for(ptr = path+1; *ptr; ptr ++)
	{
		if(*ptr == '\\' || *ptr == '/')
		{
			*ptr = 0;
			MultiByteToWideChar(CP_ACP, 0, path, -1, pathUnc, MAX_PATH);
			CreateDirectory(pathUnc, 0);
			*ptr = '\\';
		}
	}
	MultiByteToWideChar(CP_ACP, 0, path, -1, pathUnc, MAX_PATH);
	CreateDirectory(pathUnc, 0);
}

/* Used in DEVICES.C and UI.C for some purpose. Not critical in this port */
int system(const char* path) { return 0; }


char *tmpnam(char *string)
{
	TCHAR pTemp[MAX_PATH+1];
	static char buffer[MAX_PATH+1];
	GetTempFileName(TEXT("."), TEXT("A8_"), 0, pTemp);
	WideCharToMultiByte(CP_ACP, 0, pTemp, -1, buffer, MAX_PATH, NULL, NULL);
	
	if(string)
	{
		strcpy(string, buffer);
		return string;
	}
	else
		return buffer;
}

FILE *tmpfile()
{
	TCHAR pTemp[MAX_PATH+1];
	if(!GetTempFileName(TEXT("."), TEXT("A8_"), 0, pTemp))
		return _wfopen(pTemp, TEXT("w+b"));
	else
		return 0;
}

void rewind(FILE *stream)
{
	fseek(stream, 0, SEEK_SET);
}


char *strdup(const char *strSource)
{
	char* buffer;
	buffer = (char*)malloc(strlen(strSource)+1);
	if(buffer)
		strcpy(buffer, strSource);
	return buffer;
}

/* Used in UI.C */
char cwd[MAX_PATH+1] = "";
char *getcwd(char *buffer, int maxlen)
{
	TCHAR fileUnc[MAX_PATH+1];
	char* plast;
	
	if(cwd[0] == 0)
	{
		GetModuleFileName(NULL, fileUnc, MAX_PATH);
		WideCharToMultiByte(CP_ACP, 0, fileUnc, -1, cwd, MAX_PATH, NULL, NULL);
		plast = strrchr(cwd, '\\');
		if(plast)
			*plast = 0;
		/* Special trick to keep start menu clean... */
		if(_stricmp(cwd, "\\windows\\start menu") == 0)
			strcpy(cwd, "\\Apps");
	}
	if(buffer)
		strncpy(buffer, cwd, maxlen);
	return cwd;
}

/* Limited implementation of time.h. time_t formula is possibly incorrect. */
time_t time(time_t* res)
{
	time_t t;
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	t = (time_t)(((((((st.wYear-1970)*12+st.wMonth)*31+st.wDay)*7+st.wDayOfWeek)*24+st.wHour)*60+st.wMinute)*60+st.wSecond);
	
	if(res)
		*res = t;
	return t;
}

struct tm* localtime(time_t* timer)
{
	static struct tm tmLocalTime;
	unsigned long rem = *timer;
	
	tmLocalTime.tm_sec  = (short)(rem % 60);
	rem /= 60;
	tmLocalTime.tm_min  = (short)(rem % 60);
	rem /= 60;
	tmLocalTime.tm_hour = (short)(rem % 24);
	rem /= 24;
	tmLocalTime.tm_mday = (short)(rem % 7);
	rem /= 7;
	tmLocalTime.tm_mday = (short)(rem % 31);
	rem /= 31;
	tmLocalTime.tm_mon  = (short)(rem % 12);
	rem /= 12;
	tmLocalTime.tm_year = (short)(rem+1970);
	
	return &tmLocalTime;
}

/* Very limited implementation of sys/time.h */
void gettimeofday(struct timeval* tp, void* dummy)
{
	DWORD dt = GetTickCount();
	tp->tv_sec = dt/1000;
	tp->tv_usec = dt*1000;
}

void usleep(long usec)
{
	long msec = usec/1000;
	if(msec <= 0)
		Sleep(0);
	else
		Sleep(msec);
}

/*
Windows CE fopen has non-standard behavior -- not
fully qualified paths refer to root folder rather
than current folder (concept not implemented in CE).
*/
#undef fopen

FILE* wce_fopen(const char* fname, const char* fmode)
{
	char fullname[MAX_PATH+1];
	
	if(!fname || fname[0] == '\0')
		return NULL;
	if(fname[0] != '\\' && fname[0] != '/')
	{
		getcwd(fullname, MAX_PATH);
		strncat(fullname, "\\", MAX_PATH-strlen(fullname)-1);
		strncat(fullname, fname, MAX_PATH-strlen(fullname)-strlen(fname));
		return fopen(fullname, fmode);
	}
	else
		return fopen(fname, fmode);
}

/* This may provide for better sync mechanism */
unsigned int clock()
{
	return GetTickCount();
}

/* And why do people use this? */
void abort()
{
	exit(1);
}

/*
IMHO, no project should use this one, it is not portable at all. This implementation
at least allows some projects to work.
*/
char* getenv(char* name)
{
	static char buffer[MAX_PATH+1];
	if(strcmp(name, "HOME") == 0 || strcmp(name, "HOMEDIR") == 0)
	{
		getcwd(buffer, MAX_PATH);
		return buffer;
	}
	else
		return "";
}

