/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * A replacement for ADP's src-common/os_file.cpp (the HAS_VIRTUALFILESYSTEM
 * half of it) and src-unix/files.cpp + src-unix/error.cpp
 *
 * The mount dispatch is kept as it is upstream, 
 * because dim.cpp implements disk image support (.dsk, .st, .adf, .dmg) 
 * on top of File_Open and a fair number of DAAD releases are only distributed as disk images:
 *
 *     memory files -> mounted disk images -> plain files in the game folder
 *
 * Only the "plain file" leaf changes: instead of fopen() it goes through the
 * game folder's FSNode list for reads, and through the ScummVM save file
 * manager for writes (saved positions, and the transcript file if a game
 * asks for one).
 */

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/stream.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "daad/adp_prefix.h"
#include <dim.h>
#include <os_file.h>
#include <os_lib.h>
#include <os_mem.h>

namespace DAAD {

// ---------------------------------------------------------------- Game folder

/**
 * A cached listing of the folder the game was detected in.
 *
 * ADP asks for files by their original (usually uppercase, 8.3) name, while
 * the files on disk may be in any case; and it enumerates the folder with
 * wildcards to find the parts of a multi-part game. Both are answered from
 * this list rather than from SearchMan, so that the matching rules are ours
 * and identical in both paths.
 */
class GameFolder {
public:
	GameFolder() : _scanned(false) {}

	void refresh() {
		_files.clear();
		_scanned = true;

		Common::FSNode dir(ConfMan.getPath("path"));
		if (!dir.exists() || !dir.isDirectory())
			return;

		dir.getChildren(_files, Common::FSNode::kListFilesOnly);
	}

	const Common::FSList &files() {
		if (!_scanned)
			refresh();
		return _files;
	}

	/**
	 * Case insensitive lookup by leaf name. Any directory part of the
	 * requested name is ignored: ADP builds paths like "./PART1.DDB" or
	 * "GAME/PART1.DDB" in a few places, but everything really lives in the
	 * one folder ScummVM knows about.
	 */
	bool find(const char *name, Common::FSNode &node) {
		Common::String leaf(leafOf(name));
		if (leaf.empty())
			return false;

		const Common::FSList &list = files();
		for (Common::FSList::const_iterator i = list.begin(); i != list.end(); ++i) {
			if (leaf.equalsIgnoreCase(i->getName())) {
				node = *i;
				return true;
			}
		}

		return false;
	}

	static Common::String leafOf(const char *name) {
		if (name == nullptr)
			return Common::String();

		const char *leaf = name;
		for (const char *p = name; *p; ++p) {
			if (*p == '/' || *p == '\\')
				leaf = p + 1;
		}

		return Common::String(leaf);
	}

private:
	Common::FSList _files;
	bool _scanned;
};

// A file scope instance rather than a function local static on purpose: a
// function local static needs a __cxa_guard_acquire/release pair, and the
// Dreamcast's plugin loader only handles R_SH_DIR32 relocations - keeping the
// generated code boring is worth more here than the lazy initialization.
static GameFolder g_gameFolder;

/**
 * Saved positions and transcripts go to the save file manager, prefixed with
 * the target name so that two DAAD games installed side by side cannot
 * overwrite each other's slots.
 */
static Common::String saveFileName(const char *name) {
	Common::String leaf(GameFolder::leafOf(name));
	leaf.toLowercase();

	return Common::String::format("%s-%s", ConfMan.getActiveDomainName().c_str(), leaf.c_str());
}

// ---------------------------------------------------------------- Error state

static FileError fileError = FileError_None;
static char fileErrorString[256];

static void File_ClearErrorState() {
	fileError = FileError_None;
	fileErrorString[0] = 0;
}

static void File_SetErrorState(FileError error, const char *detail) {
	fileError = error;
	if (detail == nullptr) {
		fileErrorString[0] = 0;
		return;
	}
	StrCopy(fileErrorString, sizeof(fileErrorString), detail);
}

} // End of namespace DAAD

using namespace DAAD;

// ----------------------------------------------------------- Stream backed files

/**
 * ADP's File is a hand rolled vtable struct; the concrete instances add the
 * ScummVM stream behind it. Only one of the two streams is ever set.
 */
struct ScummVMFile {
	File base;
	Common::SeekableReadStream *in;
	Common::WriteStream *out;
	uint64_t written;
};

static void Stream_Close(File *file) {
	ScummVMFile *f = (ScummVMFile *)file;
	if (f->out != nullptr) {
		f->out->finalize();
		delete f->out;
	}
	delete f->in;
	Free(f);
}

static bool Stream_Seek(File *file, uint64_t position) {
	ScummVMFile *f = (ScummVMFile *)file;
	if (f->in == nullptr)
		return false;
	if (!f->in->seek((int64)position, SEEK_SET))
		return false;
	file->pos = position;
	return true;
}

static uint64_t Stream_GetPosition(File *file) {
	ScummVMFile *f = (ScummVMFile *)file;
	if (f->in != nullptr)
		return (uint64_t)f->in->pos();
	return f->written;
}

static bool Stream_Truncate(File *file, uint64_t size) {
	// The save file manager streams are created empty and written once, so a
	// truncate to the current length is a no-op and anything else is refused.
	return size == Stream_GetPosition(file);
}

static uint64_t Stream_GetSize(File *file) {
	ScummVMFile *f = (ScummVMFile *)file;
	if (f->in != nullptr)
		return (uint64_t)f->in->size();
	return f->written;
}

static uint64_t Stream_Read(File *file, void *buffer, uint64_t bytes) {
	ScummVMFile *f = (ScummVMFile *)file;
	if (f->in == nullptr) {
		File_SetErrorState(FileError_NotReadable, "File is not open for reading");
		return 0;
	}

	uint32 read = f->in->read(buffer, (uint32)bytes);
	file->pos += read;
	return read;
}

static uint64_t Stream_Write(File *file, const void *buffer, uint64_t bytes) {
	ScummVMFile *f = (ScummVMFile *)file;
	if (f->out == nullptr) {
		File_SetErrorState(FileError_NotWritable, "File is not open for writing");
		return 0;
	}

	uint32 written = f->out->write(buffer, (uint32)bytes);
	if (written != bytes)
		File_SetErrorState(FileError_WriteError, "Short write");
	f->written += written;
	file->pos += written;
	return written;
}

static bool Stream_Flush(File *file) {
	ScummVMFile *f = (ScummVMFile *)file;
	if (f->out != nullptr)
		return f->out->flush();
	return true;
}

static ScummVMFile *Stream_Alloc() {
	ScummVMFile *f = Allocate<ScummVMFile>("DAAD file");
	if (f == nullptr) {
		File_SetErrorState(FileError_OutOfMemory, "Out of memory");
		return nullptr;
	}

	f->base.close = Stream_Close;
	f->base.seek = Stream_Seek;
	f->base.getPosition = Stream_GetPosition;
	f->base.truncate = Stream_Truncate;
	f->base.getSize = Stream_GetSize;
	f->base.read = Stream_Read;
	f->base.write = Stream_Write;
	f->base.flush = Stream_Flush;
	f->base.data = nullptr;
	f->base.pos = 0;
	f->base.size = 0;
	f->in = nullptr;
	f->out = nullptr;
	f->written = 0;
	return f;
}

static File *Native_Open(const char *name, FileOpenMode mode) {
	File_ClearErrorState();

	if (mode == ReadOnly) {
		// Game data first, then a previously written save/transcript
		Common::FSNode node;
		Common::SeekableReadStream *stream = nullptr;

		if (g_gameFolder.find(name, node))
			stream = node.createReadStream();
		if (stream == nullptr)
			stream = g_system->getSavefileManager()->openForLoading(saveFileName(name));

		if (stream == nullptr) {
			File_SetErrorState(FileError_FileNotFound, "File not found");
			return nullptr;
		}

		ScummVMFile *f = Stream_Alloc();
		if (f == nullptr) {
			delete stream;
			return nullptr;
		}

		f->in = stream;
		f->base.size = (uint64_t)stream->size();
		return &f->base;
	}

	return File_Create(name);
}

File *File_Create(const char *name) {
	File_ClearErrorState();

	// Never write into the game folder: ScummVM keeps game data read only.
	Common::OutSaveFile *stream = g_system->getSavefileManager()->openForSaving(saveFileName(name), false);
	if (stream == nullptr) {
		File_SetErrorState(FileError_NotWritable, "Unable to create file");
		return nullptr;
	}

	ScummVMFile *f = Stream_Alloc();
	if (f == nullptr) {
		delete stream;
		return nullptr;
	}

	f->out = stream;
	return &f->base;
}

// ------------------------------------------------------------- Memory files

static void Memory_Close(File *file) {
	if (file->data != nullptr)
		Free(file->data);
	Free(file);
}

static bool Memory_Seek(File *file, uint64_t position) {
	if (position > file->size)
		return false;
	file->pos = position;
	return true;
}

static uint64_t Memory_GetPosition(File *file) {
	return file->pos;
}

static bool Memory_Truncate(File *file, uint64_t size) {
	if (size > file->size)
		return false;
	file->size = size;
	if (file->pos > size)
		file->pos = size;
	return true;
}

static uint64_t Memory_GetSize(File *file) {
	return file->size;
}

static uint64_t Memory_Read(File *file, void *buffer, uint64_t bytes) {
	if (file->pos + bytes > file->size)
		bytes = file->size - file->pos;
	MemCopy(buffer, (uint8_t *)file->data + file->pos, (size_t)bytes);
	file->pos += bytes;
	return bytes;
}

static uint64_t Memory_Write(File *file, const void *buffer, uint64_t bytes) {
	if (file->pos + bytes > file->size)
		bytes = file->size - file->pos;
	MemCopy((uint8_t *)file->data + file->pos, buffer, (size_t)bytes);
	file->pos += bytes;
	return bytes;
}

static bool Memory_Flush(File *file) {
	return true;
}

File *Memory_Open(void *data, uint64_t dataSize) {
	File *file = Allocate<File>("DAAD memory file");
	if (file == nullptr) {
		File_SetErrorState(FileError_OutOfMemory, "Out of memory");
		return nullptr;
	}

	file->close = Memory_Close;
	file->seek = Memory_Seek;
	file->getPosition = Memory_GetPosition;
	file->truncate = Memory_Truncate;
	file->getSize = Memory_GetSize;
	file->read = Memory_Read;
	file->write = Memory_Write;
	file->flush = Memory_Flush;
	file->data = data;
	file->pos = 0;
	file->size = dataSize;
	return file;
}

// ------------------------------------------------------------ Mount dispatch

// Several DAAD releases ship one disk image per part
// (an Amiga two disk set, two .st files, etc)
//
// Enumerating and opening present a merged view over all of them,
// so the player sees every part's files as if they were on one disk.
#define MAX_MOUNTED_DISKS 8
static DIM_Disk *mountedDisks[MAX_MOUNTED_DISKS] = { nullptr };
static int mountedDiskCount = 0;

#define MAX_MOUNTED_MEMORY_FILES 16
struct MountedMemoryFile {
	char name[FILE_MAX_PATH];
	uint8_t *data;
	uint32_t size;
};
static MountedMemoryFile mountedMemoryFiles[MAX_MOUNTED_MEMORY_FILES];
static int mountedMemoryFileCount = 0;

bool File_MountMemoryFile(const char *name, const void *data, uint64_t size) {
	if (name == nullptr || data == nullptr || size == 0 || size > 0xFFFFFFFFULL ||
		mountedMemoryFileCount >= MAX_MOUNTED_MEMORY_FILES)
		return false;

	uint8_t *copy = Allocate<uint8_t>("Mounted memory file", (size_t)size);
	if (copy == nullptr)
		return false;

	MemCopy(copy, data, (size_t)size);
	MountedMemoryFile &file = mountedMemoryFiles[mountedMemoryFileCount++];
	StrCopy(file.name, sizeof(file.name), name);
	file.data = copy;
	file.size = (uint32_t)size;
	return true;
}

void File_UnmountMemoryFiles() {
	for (int n = 0; n < mountedMemoryFileCount; n++) {
		Free(mountedMemoryFiles[n].data);
		mountedMemoryFiles[n].data = nullptr;
	}
	mountedMemoryFileCount = 0;
}

bool File_MountDisk(const char *file) {
	if (mountedDiskCount >= MAX_MOUNTED_DISKS)
		return false;

	DIM_Disk *disk = DIM_OpenDisk(file);
	if (disk == nullptr)
		return false;

	mountedDisks[mountedDiskCount++] = disk;
	return true;
}

void File_UnmountDisk() {
	for (int i = 0; i < mountedDiskCount; i++)
		DIM_CloseDisk(mountedDisks[i]);
	mountedDiskCount = 0;
}

bool File_IsDiskMounted() {
	return mountedDiskCount > 0;
}

int File_GetMountedDiskType() {
	// A multi-disk set is always the same medium 
	// (e.g. two .st are both DIM_FAT),
	// so the first disk's type represents the target platform.
	return mountedDiskCount > 0 ? (int)mountedDisks[0]->type : -1;
}

File *File_Open(const char *fileName, FileOpenMode mode) {
	if (mountedMemoryFileCount > 0 && mode == ReadOnly) {
		Common::String leaf(GameFolder::leafOf(fileName));
		for (int n = 0; n < mountedMemoryFileCount; n++) {
			MountedMemoryFile &mounted = mountedMemoryFiles[n];
			if (!leaf.equalsIgnoreCase(mounted.name))
				continue;

			uint8_t *copy = Allocate<uint8_t>("Opened memory file", mounted.size);
			if (copy == nullptr)
				return nullptr;
			MemCopy(copy, mounted.data, mounted.size);
			return Memory_Open(copy, mounted.size);
		}
	}

	for (int i = 0; i < mountedDiskCount; i++) {
		FindFileResults result;
		if (!DIM_FindFile(mountedDisks[i], &result, fileName))
			continue;

		uint8_t *data = Allocate<uint8_t>("File", result.fileSize + 1);
		if (data == nullptr) {
			File_SetErrorState(FileError_OutOfMemory, "Out of memory");
			return nullptr;
		}

		data[result.fileSize] = 0;
		uint32_t size = DIM_ReadFile(mountedDisks[i], result.fileName, data, result.fileSize);
		if (size == 0) {
			Free(data);
			File_SetErrorState(FileError_ReadError, "Failed to read mounted file");
			return nullptr;
		}

		File *file = Memory_Open(data, size);
		if (file == nullptr)
			Free(data);
		return file;
	}

	return Native_Open(fileName, mode);
}

uint64_t File_GetSizeByName(const char *fileName) {
	File *file = File_Open(fileName, ReadOnly);
	if (file == nullptr)
		return 0;

	uint64_t size = File_GetSize(file);
	File_Close(file);
	return size;
}

// ------------------------------------------------------------- Enumeration

struct FindFileInternal {
	int index;
	char pattern[FILE_MAX_PATH];
};

// The iteration state is kept inside FindFileResults,
// in the fixed size scratch area ADP reserves for the backend (os_file.h). 
// ScummVM's STATIC_ASSERT is statement scoped, so use the array trick here.
typedef char DAAD_FindFileInternal_fits[(sizeof(FindFileInternal) <= 384) ? 1 : -1];

static bool FillFindFileResults(FindFileResults *results, const Common::FSNode &node) {
	Common::String name(node.getName());

	StrCopy(results->fileName, sizeof(results->fileName), name.c_str());
	results->description[0] = 0;
	results->attributes = FileAttribute_ReadOnly;
	results->modifyTime = 0;

	Common::SeekableReadStream *stream = node.createReadStream();
	results->fileSize = stream != nullptr ? (uint32_t)stream->size() : 0;
	delete stream;
	return true;
}

bool OS_FindFirstFile(const char *pattern, FindFileResults *results) {
	FindFileInternal *i = (FindFileInternal *)&results->internalData;

	Common::String leaf(GameFolder::leafOf(pattern));
	StrCopy(i->pattern, sizeof(i->pattern), leaf.c_str());
	i->index = 0;

	return OS_FindNextFile(results);
}

bool OS_FindNextFile(FindFileResults *results) {
	FindFileInternal *i = (FindFileInternal *)&results->internalData;
	const Common::FSList &list = g_gameFolder.files();

	while (i->index < (int)list.size()) {
		const Common::FSNode &node = list[i->index++];
		Common::String name(node.getName());

		if (!DIM_MatchWildcards(name.c_str(), (int)name.size(), i->pattern, (int)StrLen(i->pattern)))
			continue;

		return FillFindFileResults(results, node);
	}

	return false;
}

bool File_FindFirst(const char *pattern, FindFileResults *results) {
	if (mountedMemoryFileCount > 0) {
		Common::String leaf(GameFolder::leafOf(pattern));
		for (int n = 0; n < mountedMemoryFileCount; n++) {
			MountedMemoryFile &file = mountedMemoryFiles[n];
			if (!DIM_MatchWildcards(file.name, (int)StrLen(file.name), leaf.c_str(), (int)leaf.size()))
				continue;

			StrCopy(results->fileName, sizeof(results->fileName), file.name);
			results->fileSize = file.size;
			results->attributes = FileAttribute_ReadOnly;
			results->modifyTime = 0;
			results->description[0] = 0;
			results->mergeDiskIndex = n;
			StrCopy(results->mergePattern, sizeof(results->mergePattern), leaf.c_str());
			return true;
		}
		return false;
	}

	if (mountedDiskCount > 0) {
		StrCopy(results->mergePattern, sizeof(results->mergePattern), pattern);
		for (results->mergeDiskIndex = 0; results->mergeDiskIndex < mountedDiskCount; results->mergeDiskIndex++) {
			if (DIM_FindFirstFile(mountedDisks[results->mergeDiskIndex], results, pattern))
				return true;
		}
		return false;
	}

	return OS_FindFirstFile(pattern, results);
}

bool File_FindNext(FindFileResults *results) {
	if (mountedMemoryFileCount > 0) {
		for (int n = results->mergeDiskIndex + 1; n < mountedMemoryFileCount; n++) {
			MountedMemoryFile &file = mountedMemoryFiles[n];
			if (!DIM_MatchWildcards(file.name, (int)StrLen(file.name), results->mergePattern,
				(int)StrLen(results->mergePattern)))
				continue;

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

	if (mountedDiskCount > 0) {
		if (DIM_FindNextFile(mountedDisks[results->mergeDiskIndex], results))
			return true;

		// Current disk exhausted: continue from the start of the next one.
		for (results->mergeDiskIndex++; results->mergeDiskIndex < mountedDiskCount; results->mergeDiskIndex++) {
			if (DIM_FindFirstFile(mountedDisks[results->mergeDiskIndex], results, results->mergePattern))
				return true;
		}
		return false;
	}

	return OS_FindNextFile(results);
}

FileError File_GetError() {
	return fileError;
}

const char *File_GetErrorString() {
	return fileErrorString;
}

// ------------------------------------------------------------ Directory / OS

bool OS_GetCurrentDirectory(char *buffer, size_t bufferSize) {
	if (buffer == nullptr || bufferSize == 0)
		return false;

	StrCopy(buffer, (uint32_t)bufferSize, ".");
	return true;
}

bool OS_ChangeDirectory(const char *path) {
	// There is only ever one folder, the one the game was detected in.
	return path != nullptr;
}

bool OS_RemountBootMedia() {
	// Nothing is bound to a physical volume, but the user may have added files
	// (an extracted second disk) since the folder was last listed.
	g_gameFolder.refresh();
	return true;
}

void OSInit() {
	g_gameFolder.refresh();
}

void OSSyncFS() {
	// The save file manager writes through on finalize().
}

void OSError(const char *message) {
	// Called by ADP for unrecoverable startup failures. Do not exit(): the
	// caller (DDB_RunPlayer) unwinds and DAAD::runGame reports the error.
	warning("DAAD: %s", message != nullptr ? message : "unknown error");
}