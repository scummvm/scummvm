/* MISSING.C
   Implementation for standard and semi-standard C library calls missing in WinCE
   environment.
   by Vasyl Tsvirkunov
*/

#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <stdlib.h>
#include "sys/stat.h"
#ifndef __GNUC__
#include "sys/time.h"
#else
#include <stdio.h>
#endif
#include "time.h"
#include "dirent.h"

/* forward declaration */

#if _WIN32_WCE < 300

#define _STDAFX_H
#include "portdefs.h"

#else

char *strdup(const char *strSource);

#endif

#ifdef __GNUC__
#define EXT_C extern "C"
#else
#define EXT_C
#endif

// common missing functions required by both gcc and evc

void *bsearch(const void *key, const void *base, size_t nmemb,
			  size_t size, int (*compar)(const void *, const void *)) {
	size_t i;

	for (i=0; i<nmemb; i++)
		if (compar(key, (void*)((size_t)base + size * i)) == 0)
			return (void*)((size_t)base + size * i);
	return NULL;
}

static WIN32_FIND_DATA wfd;

/* Very limited implementation of stat. Used by UI.C, MEMORY-P.C (latter is not critical) */
int stat(const char *fname, struct stat *ss)
{
	TCHAR fnameUnc[MAX_PATH+1];
	HANDLE handle;
	int len;

	if (fname == NULL || ss == NULL)
		return -1;

	/* Special case (dummy on WinCE) */
	len = strlen(fname);
	if (len >= 2 && fname[len-1] == '.' && fname[len-2] == '.' &&
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
	FindClose(handle);
	if (handle == INVALID_HANDLE_VALUE)
		return -1;
	else
	{
		/* That's everything implemented so far */
		memset(ss, 0, sizeof(struct stat));
		ss->st_size = wfd.nFileSizeLow;
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			ss->st_mode |= S_IFDIR;
	}
	return 0;
}

char cwd[MAX_PATH+1] = "";
EXT_C char *getcwd(char *buffer, int maxlen)
{
	TCHAR fileUnc[MAX_PATH+1];
	char* plast;

	if (cwd[0] == 0)
	{
		GetModuleFileName(NULL, fileUnc, MAX_PATH);
		WideCharToMultiByte(CP_ACP, 0, fileUnc, -1, cwd, MAX_PATH, NULL, NULL);
		plast = strrchr(cwd, '\\');
		if (plast)
			*plast = 0;
		/* Special trick to keep start menu clean... */
		if (_stricmp(cwd, "\\windows\\start menu") == 0)
			strcpy(cwd, "\\Apps");
	}
	if (buffer)
		strncpy(buffer, cwd, maxlen);
	return cwd;
}

#ifdef __GNUC__
#undef GetCurrentDirectory
#endif
EXT_C void GetCurrentDirectory(int len, char *buf)
{
	getcwd(buf,len);
};

/*
Windows CE fopen has non-standard behavior -- not
fully qualified paths refer to root folder rather
than current folder (concept not implemented in CE).
*/
#undef fopen
EXT_C FILE *wce_fopen(const char* fname, const char* fmode)
{
	char fullname[MAX_PATH+1];

	if (!fname || fname[0] == '\0')
		return NULL;
	if (fname[0] != '\\' && fname[0] != '/')
	{
		getcwd(fullname, MAX_PATH);
		strncat(fullname, "\\", MAX_PATH-strlen(fullname)-1);
		strncat(fullname, fname, MAX_PATH-strlen(fullname)-strlen(fname));
		return fopen(fullname, fmode);
	}
	else
		return fopen(fname, fmode);
}

/* Remove file by name */
int remove(const char* path)
{
	TCHAR pathUnc[MAX_PATH+1];
	MultiByteToWideChar(CP_ACP, 0, path, -1, pathUnc, MAX_PATH);
	return !DeleteFile(pathUnc);
}


/* check out file access permissions */
int _access(const char *path, int mode) {
	TCHAR fname[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, path, -1, fname, sizeof(fname)/sizeof(TCHAR));

	WIN32_FIND_DATA ffd;
	HANDLE h=FindFirstFile(fname, &ffd);
	FindClose(h);

	if (h == INVALID_HANDLE_VALUE)
		return -1;  //Can't find file

	if (ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
		// WORKAROUND: WinCE (or the emulator) sometimes returns bogus direcotry
		// hits for files that don't exist. Checking for the same fname twice
		// seems to weed out those false positives.
		HANDLE h=FindFirstFile(fname, &ffd);
		FindClose(h);
		if (h == INVALID_HANDLE_VALUE)
			return -1;  //Can't find file

		return 0; //Always return success if target is directory and exists
	}
	switch (mode) {
		case 00: //Check existence
			return 0;
		case 06: //Check Read & Write permission
		case 02: //Check Write permission
			return ffd.dwFileAttributes&FILE_ATTRIBUTE_READONLY?-1:0;
		case 04: //Check Read permission
			return 0; //Assume always have read permission
	}
	//Bad mode value supplied, return failure
	return -1;
}

// evc only functions follow
#ifndef __GNUC__

/* Limited dirent implementation. Used by UI.C and DEVICES.C */
DIR* opendir(const char* fname)
{
	DIR* pdir;
	char fnameMask[MAX_PATH+1];
	TCHAR fnameUnc[MAX_PATH+1];
	char nameFound[MAX_PATH+1];

	if (fname == NULL)
		return NULL;

	strcpy(fnameMask, fname);
	if (!strlen(fnameMask) || fnameMask[strlen(fnameMask)-1] != '\\')
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
	if ((pdir->dd_handle = (long)FindFirstFile(fnameUnc, &wfd)) == (long)INVALID_HANDLE_VALUE)
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

	if (dir->dd_stat == 0)
	{
		dummy.d_name = ".";
		dummy.d_namlen = 1;
		dir->dd_stat ++;
		return &dummy;
	}
	else if (dir->dd_stat == 1)
	{
		dummy.d_name = "..";
		dummy.d_namlen = 2;
		dir->dd_stat ++;
		return &dummy;
	}
	else if (dir->dd_stat == 2)
	{
		dir->dd_stat++;
		return &dir->dd_dir;
	}
	else
	{
		if (FindNextFile((HANDLE)dir->dd_handle, &wfd) == 0)
		{
			dir->dd_stat = -1;
			return NULL;
		}
		WideCharToMultiByte(CP_ACP, 0, wfd.cFileName, -1, nameFound, MAX_PATH, NULL, NULL);

		if (dir->dd_dir.d_name)
			free(dir->dd_dir.d_name);

		dir->dd_dir.d_name = strdup(nameFound);
		dir->dd_dir.d_namlen = strlen(nameFound);

		dir->dd_stat ++;

		return &dir->dd_dir;
	}
}

int closedir(DIR* dir)
{
	if (dir == NULL)
		return 0;

	if (dir->dd_handle)
		FindClose((HANDLE)dir->dd_handle);

	if (dir->dd_dir.d_name)
		free(dir->dd_dir.d_name);
	free(dir);
	return 1;
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
	if (*path == '/')
		*path = '\\';
	/* Run through the string and attempt creating all subdirs on the path */
	for (ptr = path+1; *ptr; ptr ++)
	{
		if (*ptr == '\\' || *ptr == '/')
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

#if 0

char *tmpnam(char *string)
{
	TCHAR pTemp[MAX_PATH+1];
	static char buffer[MAX_PATH+1];
	GetTempFileName(TEXT("."), TEXT("A8_"), 0, pTemp);
	WideCharToMultiByte(CP_ACP, 0, pTemp, -1, buffer, MAX_PATH, NULL, NULL);

	if (string)
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
	if (!GetTempFileName(TEXT("."), TEXT("A8_"), 0, pTemp))
		return _wfopen(pTemp, TEXT("w+b"));
	else
		return 0;
}

#endif

void rewind(FILE *stream)
{
	fseek(stream, 0, SEEK_SET);
}


#if _WIN32_WCE < 300

int isalnum(int c) {
	return ((c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9'));
}

char *_strdup(const char *strSource)
#else
char *strdup(const char *strSource)
#endif
{
	char* buffer;
	buffer = (char*)malloc(strlen(strSource)+1);
	if (buffer)
		strcpy(buffer, strSource);
	return buffer;
}

/* Very limited implementation of sys/time.h */
void usleep(long usec)
{
	long msec = usec/1000;
	if (msec <= 0)
		Sleep(0);
	else
		Sleep(msec);
}

/* This may provide for better sync mechanism */
unsigned int clock()
{
	return GetTickCount();
}

/* And why do people use this? */
#if _WIN32_WCE >= 300
void abort()
{
	exit(1);
}
#endif

/*
IMHO, no project should use this one, it is not portable at all. This implementation
at least allows some projects to work.
*/
char* getenv(char* name)
{
	static char buffer[MAX_PATH+1];
	if (strcmp(name, "HOME") == 0 || strcmp(name, "HOMEDIR") == 0)
	{
		getcwd(buffer, MAX_PATH);
		return buffer;
	}
	else
		return "";
}

#if _WIN32_WCE < 300 || defined(_TEST_HPC_STDIO)

void *calloc(size_t n, size_t s) {
	void *result = malloc(n * s);
	if (result)
		memset(result, 0, n * s);

	return result;
}

char *strpbrk(const char *s, const char *accept) {
	int i;

	if (!s || !accept)
		return NULL;

	for (i=0; i<strlen(s); i++) {
		int j;
		for (j=0; j<strlen(accept); j++)
			if (s[i] == accept[j])
				return (char*)&s[i];
	}

	return NULL;
}

#ifndef _TEST_HPC_STDIO

int isdigit(int c) {
	return (c >='0' && c <= '9');
}

int isprint(int c) {
	return (c >= ' ' && c <= '~');
}

int isspace(int c) {
	return (c == ' ');
}

#endif

#ifndef WIN32_PLATFORM_HPCPRO


int printf(const char *format, ...) {
	// useless anyway :)
	return 0;
}

FILE *fopen(const char *path, const char *mode) {
	TCHAR tempo[MAX_PATH];
	HANDLE result;
	bool writeAccess = (mode[0] == 'W' || mode[0] == 'w');

	MultiByteToWideChar(CP_ACP, 0, path, strlen(path) + 1, tempo, sizeof(tempo));

	result = CreateFile(tempo, ( writeAccess ? GENERIC_WRITE : GENERIC_READ), 0, NULL, (writeAccess ? CREATE_ALWAYS : OPEN_EXISTING), FILE_ATTRIBUTE_NORMAL, NULL);
	if (result == INVALID_HANDLE_VALUE)
		return NULL;
	else
		return (FILE*)result;
}

FILE * _wfopen(const TCHAR *path, const TCHAR *mode) {
	HANDLE result;
	bool writeAccess = (mode[0] == 'W' || mode[0] == 'w');
	result = CreateFile(path, ( writeAccess ? GENERIC_WRITE : GENERIC_READ), 0, NULL, (writeAccess ? CREATE_ALWAYS : OPEN_EXISTING), FILE_ATTRIBUTE_NORMAL, NULL);
	if (result == INVALID_HANDLE_VALUE)
		return NULL;
	else
		return (FILE*)result;
}

FILE *_wfreopen(const TCHAR *path, const TCHAR *mode, FILE *stream) {
	fclose(stream);
	stream = _wfopen(path, mode);
	return stream;
}

int fclose(FILE *stream) {
	CloseHandle((HANDLE)stream);
	return 1;
}

int fseek(FILE *stream, long offset, int whence) {
	SetFilePointer((HANDLE)stream, offset, NULL, (whence == SEEK_CUR ? FILE_CURRENT : whence == SEEK_END ? FILE_END : FILE_BEGIN));
	return 0;
}

long ftell(FILE *stream) {
	return (SetFilePointer((HANDLE)stream, 0, NULL, FILE_CURRENT));
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	DWORD sizeWritten;

	WriteFile((HANDLE)stream, ptr, size * nmemb, &sizeWritten, NULL);

	if (size != 0)
		return sizeWritten / size;
	else
		return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	DWORD sizeRead;

	ReadFile((HANDLE)stream, ptr, size * nmemb, &sizeRead, NULL);

	if (size != 0)
		return sizeRead / size;
	else
		return 0;
}

int fgetc(FILE *stream) {
	unsigned char c;
	if (fread(&c, 1, 1, stream) != 1)
		return -1;
	else
		return c;
}

char *fgets(char *s, int size, FILE *stream) {
	int i = 0;
	char tempo[1];

	memset(s, 0, size);
	while (fread(tempo, 1, 1, stream)) {
		//if (tempo[0] == '\r')
		//	break;
		if (tempo[0] == '\r')
			continue;
		s[i++] = tempo[0];
		if (tempo[0] == '\n')
			break;
		if (i == size)
			break;
	}
	if (!i)
		return NULL;
	else
		return s;
}

int feof(FILE *stream) {
	DWORD fileSize;
	DWORD filePos;
	fileSize = GetFileSize((HANDLE)stream, NULL);
	filePos = SetFilePointer((HANDLE)stream, 0, 0, FILE_CURRENT);
	return (filePos == 0xFFFFFFFF || filePos > (fileSize - 1));
}

int ferror(FILE *stream) {
	return 0; // FIXME !
}

int fprintf(FILE *stream, const char *format, ...) {
	char buf[1024];
	va_list va;

	va_start(va, format);
	vsnprintf(buf, 1024, format, va);
	va_end(va);

	if (buf[strlen(buf) - 1] == '\n') {
		int i = strlen(buf) - 1;
		buf[i] = '\r';
		buf[i + 1] = '\n';
		buf[i + 2] = 0;
	}

	return fwrite(buf, 1, strlen(buf), stream);
}

FILE* _getstdfilex(int) {
	return NULL;
}

void clearerr(FILE *stream) {
}

int fflush(FILE *stream) {
	return 0;
}

#endif

int stricmp( const char *string1, const char *string2 ) {
	char src[4096];
	char dest[4096];
	int i;

	for (i=0; i<strlen(string1); i++)
		if (string1[i] >= 'A' && string1[i] <= 'Z')
			src[i] = string1[i] + 32;
		else
			src[i] = string1[i];
	src[i] = 0;

	for (i=0; i<strlen(string2); i++)
		if (string2[i] >= 'A' && string2[i] <= 'Z')
			dest[i] = string2[i] + 32;
		else
			dest[i] = string2[i];
	dest[i] = 0;

	return strcmp(src, dest);
}

char *strrchr(const char *s, int c) {
	int i;

	for (i = strlen(s) - 1; i > 0; i--)
		if (s[i] == c)
			return (char*)(s + i);

	return NULL;
}

long int strtol(const char *nptr, char **endptr, int base) {
	// not correct but that's all we are using

	long int result;
	sscanf(nptr, "%ld", &result);
	return result;
}


#endif


// gcc build only functions follow
#else // defined(__GNUC__)

#ifndef __MINGW32CE__
int islower(int c)
{
	return (c>='a' && c<='z');
}

int isspace(int c)
{
	return (c==' ' || c=='\f' || c=='\n' || c=='\r' || c=='\t' || c=='\v');
}

int isalpha(int c)
{
	return (islower(c) || (c>='A' && c<='Z'));
}

int isalnum(int c)
{
	return (isalpha(c) || (c>='0' && c<='9'));
}

int isprint(int c)
{
	static char punct[] = "!\"#%&'();<=>?[\\]*+,-./:^_{|}~";
	int i = 0, flag = 0;
	while ((punct[i] != 0) && (flag = (punct[i] != c)))
		i++;
	return (isalnum(c) || flag);
}

extern "C" int atexit(void (*function)(void))
{
	return 0;
}
#endif

#endif
