#pragma once

#include <os_types.h>

#define FILE_MAX_PATH 256

enum FileOpenMode
{
	ReadOnly	 = 0x01,
	ReadWrite 	 = 0x02,
};

enum FileError
{
	FileError_None,
	FileError_OutOfMemory,
	FileError_FileNotFound,
	FileError_NotWritable,
	FileError_NotReadable,
	FileError_NotSupported,
	FileError_ReadError,
	FileError_WriteError,
	FileError_OutOfBounds,
};

enum FileAttributes
{
	FileAttribute_Directory = 0x01,
	FileAttribute_Hidden    = 0x02,
	FileAttribute_System    = 0x04,
	FileAttribute_Archive   = 0x08,
	FileAttribute_ReadOnly  = 0x10,
};

struct FindFileResults
{
	char     fileName[FILE_MAX_PATH];
	char     description[80];
	uint32_t fileSize;
	int      attributes;
	uint32_t modifyTime;

	uint8_t  internalData[384];

	// Iteration state for enumerating across several mounted disks at once. The
	// per-disk backend keeps its own cursor (and the pattern) inside internalData,
	// which DIM_FindFirstFile reinitialises per disk, so the merged walk retains
	// the disk index and the pattern here.
	int      mergeDiskIndex;
	char     mergePattern[FILE_MAX_PATH];
};

extern bool OS_FindFirstFile (const char* pattern, FindFileResults* results);
extern bool OS_FindNextFile  (FindFileResults* results);
extern bool OS_GetCurrentDirectory (char* buffer, size_t bufferSize);
extern bool OS_ChangeDirectory     (const char* path);

// Re-binds the current directory to whatever removable medium is now present
// in the boot drive, releasing any state bound to the previous volume. Called
// after the user confirms a physical disk change; a no-op on platforms whose
// filesystem access is not tied to a specific volume. Returns false if no
// medium could be bound (the caller may prompt again).
extern bool OS_RemountBootMedia    ();

#ifdef HAS_VIRTUALFILESYSTEM

struct File
{
	void     (*close)       (File *file);
	bool     (*seek)        (File *file, uint64_t position);
	uint64_t (*getPosition) (File *file);
	bool     (*truncate)    (File *file, uint64_t size);
	uint64_t (*getSize)     (File *file);
	uint64_t (*read)        (File *file, void *buffer, uint64_t bytes);
	uint64_t (*write)       (File *file, const void *buffer, uint64_t bytes);
	bool     (*flush)       (File *file);

	void*    data;
	uint64_t pos;
	uint64_t size;
};


extern bool     File_MountDisk (const char* file);
extern bool     File_MountMemoryFile (const char* name, const void* data, uint64_t size);
extern void     File_UnmountMemoryFiles ();
extern void     File_UnmountDisk ();
extern bool     File_IsDiskMounted ();
extern int      File_GetMountedDiskType ();  // DIM_DiskType, or -1 if none mounted
extern File*    File_Open      (const char* file, FileOpenMode mode = ReadOnly);
extern File*    File_Create    (const char* file);
extern uint64_t File_GetSizeByName(const char* file);
extern bool     File_FindFirst (const char* pattern, FindFileResults* results);
extern bool     File_FindNext  (FindFileResults* results);

inline uint64_t File_GetSize     (File *file)                                     { return file->getSize(file);              }
inline uint64_t File_Read        (File *file, void *buffer, uint64_t bytes)       { return file->read(file, buffer, bytes);  }
inline uint64_t File_Write       (File *file, const void *buffer, uint64_t bytes) { return file->write(file, buffer, bytes); }
inline bool     File_Flush       (File *file)                                     { return file->flush(file);                }
inline bool     File_Seek        (File *file, uint64_t position)                  { return file->seek(file, position);       }
inline uint64_t File_GetPosition (File *file)                                     { return file->getPosition(file);          }
inline bool     File_Truncate    (File *file, uint64_t size)                      { return file->truncate(file, size);       }
inline void     File_Close       (File *file)                                     { return file->close(file);                }

extern File*    Memory_Open      (void *data, uint64_t dataSize);

#else

struct File;

extern File*       File_Open           (const char* file, FileOpenMode mode = ReadOnly);
extern File*       File_Create         (const char* file);
extern uint64_t    File_GetSizeByName  (const char* file);
extern uint64_t    File_GetSize        (File* file);
extern uint64_t    File_Read           (File* file, void* buffer, uint64_t bytes);
extern uint64_t    File_Write          (File* file, const void* buffer, uint64_t bytes);
extern bool        File_Flush          (File* file);
extern bool        File_Seek           (File* file, uint64_t position);
extern uint64_t    File_GetPosition    (File* file);
extern bool        File_Truncate       (File* file, uint64_t size);
extern void        File_Close          (File* file);

inline bool File_FindFirst (const char* pattern, FindFileResults* results)  { return OS_FindFirstFile(pattern, results); }
inline bool File_FindNext  (FindFileResults* results)                       { return OS_FindNextFile(results); }

#endif

extern FileError   File_GetError       ();
extern const char* File_GetErrorString ();
