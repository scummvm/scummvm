#include <os_lib.h>
#include <os_mem.h>
#include <os_file.h>

#ifdef HAS_VIRTUALFILESYSTEM

#include <dim.h>

#ifdef _UNIX
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

static FileError fileError = FileError_None;
static char fileErrorString[256];

static void File_ClearErrorState()
{
	fileError = FileError_None;
	fileErrorString[0] = 0;
}

static void File_SetErrorState(FileError error, const char* detail)
{
	fileError = error;
	if (detail == 0)
	{
		fileErrorString[0] = 0;
		return;
	}
	StrCopy(fileErrorString, sizeof(fileErrorString), detail);
}

static void File_SetErrnoState(FileError defaultError)
{
	FileError mapped = defaultError;
	switch (errno)
	{
		case 0:
			break;
		#ifdef ENOENT
		case ENOENT:
			mapped = FileError_FileNotFound;
			break;
		#endif
		#if defined(EACCES) && defined(EPERM)
		case EACCES:
		case EPERM:
		#elif defined(EACCES)
		case EACCES:
		#elif defined(EPERM)
		case EPERM:
		#endif
			mapped = defaultError == FileError_NotReadable ? FileError_NotReadable : FileError_NotWritable;
			break;
		default:
			break;
	}

	const char* detail = strerror(errno);
	File_SetErrorState(mapped, detail != 0 ? detail : "");
}

#ifdef _UNIX
static bool Native_ResolveReadOnlyPathCaseInsensitive(const char* name, char* resolved, size_t resolvedSize)
{
	if (name == 0 || resolved == 0 || resolvedSize == 0)
		return false;

	const char* slash = StrRChr(name, '/');
	char directory[FILE_MAX_PATH];
	const char* leaf = name;

	if (slash != 0)
	{
		size_t dirLength = (size_t)(slash - name);
		if (dirLength >= sizeof(directory))
			return false;
		MemCopy(directory, name, dirLength);
		directory[dirLength] = 0;
		leaf = slash + 1;
	}
	else
	{
		StrCopy(directory, sizeof(directory), ".");
	}

	DIR* dir = opendir(directory);
	if (dir == 0)
		return false;

	bool found = false;
	for (;;)
	{
		struct dirent* entry = readdir(dir);
		if (entry == 0)
			break;
		if (StrIComp(entry->d_name, leaf) != 0)
			continue;

		if (slash != 0)
		{
			StrCopy(resolved, resolvedSize, directory);
			StrCat(resolved, resolvedSize, "/");
			StrCat(resolved, resolvedSize, entry->d_name);
		}
		else
		{
			StrCopy(resolved, resolvedSize, entry->d_name);
		}
		found = true;
		break;
	}

	closedir(dir);
	return found;
}
#endif

// ----- Native files

uint64_t Native_GetPosition(File *file)
{
	return ftell((FILE*)file->data);
}

bool Native_Truncate(File *file, uint64_t size)
{
#ifdef _UNIX
	return ftruncate(fileno((FILE*)file->data), size) == 0;
#elif defined(_WINDOWS)
	return _chsize(fileno((FILE*)file->data), size) == 0;
#else
	return false;
#endif
}

void Native_Close(File *file)
{
	fclose((FILE*)file->data);
	Free(file);
}

uint64_t Native_GetSize(File *file)
{
	FILE* f = (FILE*)file->data;
	long pos = ftell(f);
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, pos, SEEK_SET);
	return (uint64_t)size;
}

uint64_t Native_Read(File *file, void *buffer, uint64_t bytes)
{
	return fread(buffer, 1, bytes, (FILE*)file->data);
}

uint64_t Native_Write(File *file, const void *buffer, uint64_t bytes)
{
	return fwrite(buffer, 1, bytes, (FILE*)file->data);
}

bool Native_Flush(File *file)
{
	return fflush((FILE*)file->data) == 0;
}

bool Native_Seek(File *file, uint64_t position)
{
	return fseek((FILE*)file->data, position, SEEK_SET) == 0;
}

File *Native_Open(const char *name, FileOpenMode mode)
{
	void* nativeFile = (void*) fopen(name, mode == ReadOnly ? "rb" : "rb+");
	#ifdef _UNIX
	char resolvedName[FILE_MAX_PATH * 2];
	if (nativeFile == 0 && mode == ReadOnly && Native_ResolveReadOnlyPathCaseInsensitive(name, resolvedName, sizeof(resolvedName)))
		nativeFile = (void*)fopen(resolvedName, "rb");
	#endif
	if (nativeFile == 0)
	{
		File_SetErrnoState(mode == ReadOnly ? FileError_NotReadable : FileError_NotWritable);
		return 0;
	}

	File* file = Allocate<File>("File", 1);
	if (file == 0)
	{
		fclose((FILE*)nativeFile);
		File_SetErrorState(FileError_OutOfMemory, "Out of memory");
		return 0;
	}
	file->close       = Native_Close;
	file->seek        = Native_Seek;
	file->getPosition = Native_GetPosition;
	file->truncate    = Native_Truncate;
	file->getSize     = Native_GetSize;
	file->read        = Native_Read;
	file->write       = Native_Write;
	file->flush       = Native_Flush;
	file->data        = nativeFile;
	File_ClearErrorState();
	return file;
}

File *Native_Create(const char *name)
{
	FILE* nativeFile = fopen(name, "wb+");
	if (nativeFile == 0)
	{
		File_SetErrnoState(FileError_NotWritable);
		return 0;
	}

	File* file = Allocate<File>("File", 1);
	if (file == 0)
	{
		fclose(nativeFile);
		File_SetErrorState(FileError_OutOfMemory, "Out of memory");
		return 0;
	}
	file->close       = Native_Close;
	file->seek        = Native_Seek;
	file->getPosition = Native_GetPosition;
	file->truncate    = Native_Truncate;
	file->getSize     = Native_GetSize;
	file->read        = Native_Read;
	file->write       = Native_Write;
	file->flush       = Native_Flush;
	file->data        = nativeFile;
	File_ClearErrorState();
	return file;
}

// ----- Memory files

uint64_t Memory_GetPosition(File *file)
{
	return (uint64_t)file->pos;
}

bool Memory_Truncate(File *file, uint64_t size)
{
	if (size > file->getSize(file))
		return false;
	file->size = size;
	return true;
}

void Memory_Close(File *file)
{
	Free(file->data);
	Free(file);
}

uint64_t Memory_GetSize(File *file)
{
	return (uint64_t)file->size;
}

uint64_t Memory_Read(File *file, void *buffer, uint64_t bytes)
{
	if (file->pos + bytes > file->size)
		bytes = file->size - file->pos;
	memcpy(buffer, (uint8_t*)file->data + file->pos, bytes);
	file->pos += bytes;
	return bytes;
}

uint64_t Memory_Write(File *file, const void *buffer, uint64_t bytes)
{
	if (file->pos + bytes > file->size)
		bytes = file->size - file->pos;
	MemCopy((uint8_t*)file->data + file->pos, buffer, bytes);
	file->pos += bytes;
	return bytes;
}

bool Memory_Flush(File *file)
{
	return true;
}

bool Memory_Seek(File *file, uint64_t position)
{
	if (position > file->size)
		return false;
	file->pos = position;
	return true;
}

File *Memory_Open(void *data, uint64_t dataSize)
{
	File* file = Allocate<File>("File", 1);
	if (file == 0)
	{
		File_SetErrorState(FileError_OutOfMemory, "Out of memory");
		return 0;
	}

	file->close       = Memory_Close;
	file->seek        = Memory_Seek;
	file->getPosition = Memory_GetPosition;
	file->truncate    = Memory_Truncate;
	file->getSize     = Memory_GetSize;
	file->read        = Memory_Read;
	file->write       = Memory_Write;
	file->flush       = Memory_Flush;
	file->data        = data;
	file->size        = dataSize;
	file->pos         = 0;
	File_ClearErrorState();
	return file;
}

// ----- Interface

// Several disk images can be mounted at once (e.g. a multi-disk game supplied as
// two .st files): enumeration and open present a merged view over all of them, so
// the player sees every part's files as if they were on one disk.
#define MAX_MOUNTED_DISKS 8
static DIM_Disk* mountedDisks[MAX_MOUNTED_DISKS] = { 0 };
static int       mountedDiskCount = 0;

#define MAX_MOUNTED_MEMORY_FILES 16
struct MountedMemoryFile
{
	char name[FILE_MAX_PATH];
	uint8_t* data;
	uint32_t size;
};
static MountedMemoryFile mountedMemoryFiles[MAX_MOUNTED_MEMORY_FILES];
static int mountedMemoryFileCount = 0;

bool File_MountMemoryFile(const char* name, const void* data, uint64_t size)
{
	if (name == 0 || data == 0 || size == 0 || size > 0xFFFFFFFFULL ||
		mountedMemoryFileCount >= MAX_MOUNTED_MEMORY_FILES)
		return false;
	uint8_t* copy = Allocate<uint8_t>("Mounted memory file", (size_t)size);
	if (copy == 0) return false;
	MemCopy(copy, data, (size_t)size);
	MountedMemoryFile& file = mountedMemoryFiles[mountedMemoryFileCount++];
	StrCopy(file.name, sizeof(file.name), name);
	file.data = copy;
	file.size = (uint32_t)size;
	return true;
}

void File_UnmountMemoryFiles()
{
	for (int n = 0; n < mountedMemoryFileCount; n++)
	{
		Free(mountedMemoryFiles[n].data);
		mountedMemoryFiles[n].data = 0;
	}
	mountedMemoryFileCount = 0;
}

bool File_MountDisk (const char* file)
{
	if (mountedDiskCount >= MAX_MOUNTED_DISKS)
		return false;
	DIM_Disk* disk = DIM_OpenDisk(file);
	if (disk == 0)
		return false;
	mountedDisks[mountedDiskCount++] = disk;
	return true;
}

void File_UnmountDisk ()
{
	for (int i = 0; i < mountedDiskCount; i++)
		DIM_CloseDisk(mountedDisks[i]);
	mountedDiskCount = 0;
}

bool File_IsDiskMounted ()
{
	return mountedDiskCount > 0;
}

int File_GetMountedDiskType ()
{
	// A multi-disk set is always the same medium (e.g. two .st are both DIM_FAT),
	// so the first disk's type represents the platform for the machine override.
	return mountedDiskCount > 0 ? (int)mountedDisks[0]->type : -1;
}

File *File_Open(const char *fileName, FileOpenMode mode)
{
	if (mountedMemoryFileCount > 0 && mode == ReadOnly)
	{
		const char* leaf = StrRChr(fileName, '/');
		const char* backslash = StrRChr(fileName, '\\');
		if (backslash != 0 && (leaf == 0 || backslash > leaf)) leaf = backslash;
		leaf = leaf ? leaf + 1 : fileName;
		for (int n = 0; n < mountedMemoryFileCount; n++)
		{
			MountedMemoryFile& mounted = mountedMemoryFiles[n];
			if (StrIComp(leaf, mounted.name) != 0) continue;
			uint8_t* copy = Allocate<uint8_t>("Opened memory file", mounted.size);
			if (copy == 0) return 0;
			MemCopy(copy, mounted.data, mounted.size);
			return Memory_Open(copy, mounted.size);
		}
	}
	for (int i = 0; i < mountedDiskCount; i++)
	{
		FindFileResults result;
		if (!DIM_FindFile(mountedDisks[i], &result, fileName))
			continue;

		uint8_t* data = Allocate<uint8_t>("File", result.fileSize + 1);
		if (data == 0)
		{
			File_SetErrorState(FileError_OutOfMemory, "Out of memory");
			return 0;
		}

		data[result.fileSize] = 0;
		uint32_t size = DIM_ReadFile(mountedDisks[i], result.fileName, data, result.fileSize);
		if (size == 0)
		{
			Free(data);
			File_SetErrorState(FileError_ReadError, "Failed to read mounted file");
			return 0;
		}
		File* file = Memory_Open(data, size);
		if (file == 0)
			Free(data);
		return file;
	}
	return Native_Open(fileName, mode);
}

File* File_Create(const char* fileName)
{
	// Should be write to the mounted disk instead?
	// Makes sense for a tool, but not for a save game position.
	
	return Native_Create(fileName);
}

uint64_t File_GetSizeByName(const char* fileName)
{
	File* file = File_Open(fileName, ReadOnly);
	if (file == 0)
		return 0;
	uint64_t size = File_GetSize(file);
	File_Close(file);
	return size;
}

bool File_FindFirst (const char* pattern, FindFileResults* results)
{
	if (mountedMemoryFileCount > 0)
	{
		const char* leaf = StrRChr(pattern, '/');
		const char* backslash = StrRChr(pattern, '\\');
		if (backslash != 0 && (leaf == 0 || backslash > leaf)) leaf = backslash;
		leaf = leaf ? leaf + 1 : pattern;
		for (int n = 0; n < mountedMemoryFileCount; n++)
		{
			MountedMemoryFile& file = mountedMemoryFiles[n];
			if (!DIM_MatchWildcards(file.name, (int)StrLen(file.name), leaf, (int)StrLen(leaf))) continue;
			StrCopy(results->fileName, sizeof(results->fileName), file.name);
			results->fileSize = file.size;
			results->attributes = FileAttribute_ReadOnly;
			results->modifyTime = 0;
			results->description[0] = 0;
			results->mergeDiskIndex = n;
			StrCopy(results->mergePattern, sizeof(results->mergePattern), leaf);
			return true;
		}
		return false;
	}
	if (mountedDiskCount > 0)
	{
		StrCopy(results->mergePattern, sizeof(results->mergePattern), pattern);
		for (results->mergeDiskIndex = 0; results->mergeDiskIndex < mountedDiskCount; results->mergeDiskIndex++)
		{
			if (DIM_FindFirstFile(mountedDisks[results->mergeDiskIndex], results, pattern))
				return true;
		}
		return false;
	}
	return OS_FindFirstFile(pattern, results);
}

bool File_FindNext (FindFileResults* results)
{
	if (mountedMemoryFileCount > 0)
	{
		for (int n = results->mergeDiskIndex + 1; n < mountedMemoryFileCount; n++)
		{
			MountedMemoryFile& file = mountedMemoryFiles[n];
			if (!DIM_MatchWildcards(file.name, (int)StrLen(file.name), results->mergePattern,
				(int)StrLen(results->mergePattern))) continue;
			StrCopy(results->fileName, sizeof(results->fileName), file.name);
			results->fileSize = file.size;
			results->attributes = FileAttribute_ReadOnly;
			results->modifyTime = 0;
			results->description[0] = 0;
			results->mergeDiskIndex = n;
			return true;
		}
		return false;
	}
	if (mountedDiskCount > 0)
	{
		if (DIM_FindNextFile(mountedDisks[results->mergeDiskIndex], results))
			return true;
		// Current disk exhausted: continue from the start of the next one.
		for (results->mergeDiskIndex++; results->mergeDiskIndex < mountedDiskCount; results->mergeDiskIndex++)
		{
			if (DIM_FindFirstFile(mountedDisks[results->mergeDiskIndex], results, results->mergePattern))
				return true;
		}
		return false;
	}
	return OS_FindNextFile(results);
}

FileError File_GetError()
{
	return fileError;
}

const char* File_GetErrorString()
{
	if (fileErrorString[0] != 0)
		return fileErrorString;

	switch (fileError)
	{
		case FileError_None: return "No error";
		case FileError_OutOfMemory: return "Out of memory";
		case FileError_FileNotFound: return "File not found";
		case FileError_NotWritable: return "File is not writable";
		case FileError_NotReadable: return "File is not readable";
		case FileError_NotSupported: return "Operation not supported";
		case FileError_ReadError: return "Read error";
		case FileError_WriteError: return "Write error";
		case FileError_OutOfBounds: return "Out of bounds";
		default: return "Unknown file error";
	}
}

#elif _STDCLIB

#ifdef _DOS

#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>

static FileError fileError = FileError_None;
static char fileErrorString[256];

struct DosFileHandle
{
	int handle;
};

static void File_ClearErrorState()
{
	fileError = FileError_None;
	fileErrorString[0] = 0;
}

static void File_SetErrorState(FileError error, const char* detail)
{
	fileError = error;
	if (detail == 0)
	{
		fileErrorString[0] = 0;
		return;
	}
	StrCopy(fileErrorString, sizeof(fileErrorString), detail);
}

static void File_SetDosErrorState(unsigned dosError, FileError defaultError)
{
	FileError mapped = defaultError;
	switch (dosError)
	{
		case 2:
		case 3:
			mapped = FileError_FileNotFound;
			break;
		case 5:
		case 6:
			mapped = defaultError == FileError_NotReadable ? FileError_NotReadable : FileError_NotWritable;
			break;
		default:
			break;
	}

	(void)dosError;
	File_SetErrorState(mapped, "DOS I/O error");
}

static bool Dos_SeekHandle(int handle, uint32_t position, uint32_t* newPosition)
{
	long outPos = lseek(handle, (long)position, SEEK_SET);
	if (outPos < 0)
		return false;
	if (newPosition != 0)
		*newPosition = (uint32_t)outPos;
	return true;
}

File *File_Open(const char *file, FileOpenMode mode)
{
	DosFileHandle* f = Allocate<DosFileHandle>("DOS File", 1);
	if (f == 0)
	{
		File_SetErrorState(FileError_OutOfMemory, "Out of memory");
		return 0;
	}

	int handle = 0;
	unsigned result = _dos_open(file, mode == ReadOnly ? 0 : 2, &handle);
	if (result != 0)
	{
		Free(f);
		File_SetDosErrorState(result, mode == ReadOnly ? FileError_NotReadable : FileError_NotWritable);
		return 0;
	}

	f->handle = handle;
	File_ClearErrorState();
	return (File*)f;
}

File *File_Create(const char *file)
{
	DosFileHandle* f = Allocate<DosFileHandle>("DOS File", 1);
	if (f == 0)
	{
		File_SetErrorState(FileError_OutOfMemory, "Out of memory");
		return 0;
	}

	int handle = 0;
	unsigned result = _dos_creat(file, 0, &handle);
	if (result != 0)
	{
		Free(f);
		File_SetDosErrorState(result, FileError_NotWritable);
		return 0;
	}

	f->handle = handle;
	File_ClearErrorState();
	return (File*)f;
}

uint64_t File_GetSizeByName(const char* file)
{
	File* f = File_Open(file, ReadOnly);
	if (f == 0)
		return 0;
	uint64_t size = File_GetSize(f);
	File_Close(f);
	return size;
}

uint64_t File_GetPosition(File *file)
{
	DosFileHandle* f = (DosFileHandle*)file;
	long outPos = lseek(f->handle, 0L, SEEK_CUR);
	if (outPos < 0)
		return 0;
	return (uint64_t)(uint32_t)outPos;
}

bool File_Truncate(File *file, uint64_t size)
{
	DosFileHandle* f = (DosFileHandle*)file;
	if (size > 0xFFFFFFFFULL)
		return false;
	if (!Dos_SeekHandle(f->handle, (uint32_t)size, 0))
		return false;
	unsigned written = 0;
	unsigned result = _dos_write(f->handle, "", 0, &written);
	if (result != 0)
		return false;
	return true;
}

void File_Close(File *file)
{
	DosFileHandle* f = (DosFileHandle*)file;
	_dos_close((unsigned)f->handle);
	Free(f);
}

uint64_t File_GetSize(File *file)
{
	DosFileHandle* f = (DosFileHandle*)file;
	long pos = 0;
	long end = 0;
	pos = lseek(f->handle, 0L, SEEK_CUR);
	if (pos < 0)
		return 0;
	end = lseek(f->handle, 0L, SEEK_END);
	if (end < 0)
		return 0;
	lseek(f->handle, pos, SEEK_SET);
	return (uint64_t)(uint32_t)end;
}

uint64_t File_Read(File *file, void *buffer, uint64_t bytes)
{
	DosFileHandle* f = (DosFileHandle*)file;
	uint8_t* out = (uint8_t*)buffer;
	uint64_t total = 0;
	while (bytes > 0)
	{
		unsigned chunk = bytes > 0x7FFFu ? 0x7FFFu : (unsigned)bytes;
		unsigned got = 0;
		unsigned result = _dos_read(f->handle, out, chunk, &got);
		if (result != 0)
		{
			if (total == 0)
				File_SetDosErrorState(result, FileError_ReadError);
			break;
		}
		total += got;
		if (got < chunk)
			break;
		out += got;
		bytes -= got;
	}
	return total;
}

uint64_t File_Write(File *file, const void *buffer, uint64_t bytes)
{
	DosFileHandle* f = (DosFileHandle*)file;
	const uint8_t* in = (const uint8_t*)buffer;
	uint64_t total = 0;
	while (bytes > 0)
	{
		unsigned chunk = bytes > 0x7FFFu ? 0x7FFFu : (unsigned)bytes;
		unsigned wrote = 0;
		unsigned result = _dos_write(f->handle, in, chunk, &wrote);
		if (result != 0)
		{
			if (total == 0)
				File_SetDosErrorState(result, FileError_WriteError);
			break;
		}
		total += wrote;
		if (wrote < chunk)
			break;
		in += wrote;
		bytes -= wrote;
	}
	return total;
}

bool File_Flush(File *file)
{
	return true;
}

bool File_Seek(File *file, uint64_t position)
{
	DosFileHandle* f = (DosFileHandle*)file;
	if (position > 0xFFFFFFFFULL)
		return false;
	if (!Dos_SeekHandle(f->handle, (uint32_t)position, 0))
		return false;
	return true;
}

FileError File_GetError()
{
	return fileError;
}

const char* File_GetErrorString()
{
	if (fileErrorString[0] != 0)
		return fileErrorString;

	switch (fileError)
	{
		case FileError_None: return "No error";
		case FileError_OutOfMemory: return "Out of memory";
		case FileError_FileNotFound: return "File not found";
		case FileError_NotWritable: return "File is not writable";
		case FileError_NotReadable: return "File is not readable";
		case FileError_NotSupported: return "Operation not supported";
		case FileError_ReadError: return "Read error";
		case FileError_WriteError: return "Write error";
		case FileError_OutOfBounds: return "Out of bounds";
		default: return "Unknown file error";
	}
}

#else

#ifdef _UNIX
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

static FileError fileError = FileError_None;
static char fileErrorString[256];

static void File_ClearErrorState()
{
	fileError = FileError_None;
	fileErrorString[0] = 0;
}

static void File_SetErrorState(FileError error, const char* detail)
{
	fileError = error;
	if (detail == 0)
	{
		fileErrorString[0] = 0;
		return;
	}
	StrCopy(fileErrorString, sizeof(fileErrorString), detail);
}

static void File_SetErrnoState(FileError defaultError)
{
	FileError mapped = defaultError;
	switch (errno)
	{
		case 0:
			break;
		#ifdef ENOENT
		case ENOENT:
			mapped = FileError_FileNotFound;
			break;
		#endif
		#if defined(EACCES) && defined(EPERM)
		case EACCES:
		case EPERM:
		#elif defined(EACCES)
		case EACCES:
		#elif defined(EPERM)
		case EPERM:
		#endif
			mapped = defaultError == FileError_NotReadable ? FileError_NotReadable : FileError_NotWritable;
			break;
		default:
			break;
	}

	const char* detail = strerror(errno);
	File_SetErrorState(mapped, detail != 0 ? detail : "");
}

File *File_Open(const char *file, FileOpenMode mode)
{
	FILE* f = fopen(file, mode == ReadOnly ? "rb" : "rb+");
	if (f == 0)
	{
		File_SetErrnoState(mode == ReadOnly ? FileError_NotReadable : FileError_NotWritable);
		return 0;
	}
	File_ClearErrorState();
	return (File*) f;
}

File *File_Create(const char *file)
{
	FILE* f = fopen(file, "wb+");
	if (f == 0)
	{
		File_SetErrnoState(FileError_NotWritable);
		return 0;
	}
	File_ClearErrorState();
	return (File*) f;
}

uint64_t File_GetSizeByName(const char* file)
{
	FILE* f = fopen(file, "rb");
	if (f == 0)
		return 0;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fclose(f);
	return (uint64_t)size;
}

uint64_t File_GetPosition(File *file)
{
	return ftell((FILE*)file);
}

bool File_Truncate(File *file, uint64_t size)
{
#ifdef _UNIX
	return ftruncate(fileno((FILE*)file), size) == 0;
#elif defined(_WINDOWS)
	return _chsize(fileno((FILE*)file), size) == 0;
#else
	return false;
#endif
}

void File_Close(File *file)
{
	fclose((FILE*)file);
}

uint64_t File_GetSize(File *file)
{
	FILE* f = (FILE*)file;
	long pos = ftell(f);
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, pos, SEEK_SET);
	return (uint64_t)size;
}

uint64_t File_Read(File *file, void *buffer, uint64_t bytes)
{
	return fread(buffer, 1, bytes, (FILE*)file);
}

uint64_t File_Write(File *file, const void *buffer, uint64_t bytes)
{
	return fwrite(buffer, 1, bytes, (FILE*)file);
}

bool File_Flush(File *file)
{
	return fflush((FILE*)file) == 0;
}

bool File_Seek(File *file, uint64_t position)
{
	return fseek((FILE*)file, position, SEEK_SET) == 0;
}

FileError File_GetError()
{
	return fileError;
}

const char* File_GetErrorString()
{
	if (fileErrorString[0] != 0)
		return fileErrorString;

	switch (fileError)
	{
		case FileError_None: return "No error";
		case FileError_OutOfMemory: return "Out of memory";
		case FileError_FileNotFound: return "File not found";
		case FileError_NotWritable: return "File is not writable";
		case FileError_NotReadable: return "File is not readable";
		case FileError_NotSupported: return "Operation not supported";
		case FileError_ReadError: return "Read error";
		case FileError_WriteError: return "Write error";
		case FileError_OutOfBounds: return "Out of bounds";
		default: return "Unknown file error";
	}
}

#endif

#endif
