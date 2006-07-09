/* ScummVMDS - Scumm Interpreter DS Port
 * Copyright (C) 2002-2004 The ScummVM project and Neil Millstone
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 
#ifndef _DS_FS_H
#define _DS_FS_H


#include "stdafx.h"
#include "common/array.h"
#include "common/str.h"

//#include <NDS/ARM9/console.h>
#include "fs.h"
#include "zipreader.h"
#include "ramsave.h"
#include "scummconsole.h"
#include "gba_nds_fat.h"
#include "backends/fs/abstract-fs.h"
//#include "backends/fs/fs.h"

// Fix name clash with FOTAQ engine, cutaway.h
#undef MAX_FILENAME_LENGTH
// This class is used when a Flash cart is in use

namespace DS {

class DSFileSystemNode : public AbstractFilesystemNode {
protected:
	static ZipFile* _zipFile;

	typedef class Common::String String;

	String _displayName;
	bool _isDirectory;
	bool _isValid;
	String _path;
	int _refCountVal;
	
public:
	DSFileSystemNode();
	DSFileSystemNode(const String &path);
	DSFileSystemNode(const DSFileSystemNode *node);
	DSFileSystemNode(const String& path, bool isDir);
	
	virtual String displayName() const {  return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }
	
	virtual bool listDir(AbstractFSList &list, ListMode mode = FilesystemNode::kListDirectoriesOnly) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *clone() const { return new DSFileSystemNode(this); }
	virtual AbstractFilesystemNode *child(const Common::String& name) const;
	static ZipFile* getZip() { return _zipFile; }
};


// This class is used when the GBAMP (GBA Movie Player) is used with a CompactFlash card

class GBAMPFileSystemNode : public AbstractFilesystemNode {
protected:
	typedef class Common::String String;

	String _displayName;
	bool _isDirectory;
	bool _isValid;
	String _path;
	
	int _refCountVal;
	
public:
	GBAMPFileSystemNode();
	GBAMPFileSystemNode(const String &path);
	GBAMPFileSystemNode(const String &path, bool isDirectory);
	GBAMPFileSystemNode(const GBAMPFileSystemNode *node);

	virtual String displayName() const {  return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }
	virtual bool listDir(AbstractFSList &list, ListMode mode = FilesystemNode::kListDirectoriesOnly) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *clone() const { return new GBAMPFileSystemNode(this); }
	virtual AbstractFilesystemNode *child(const Common::String& name) const;
	
};


// File reading


struct fileHandle {
	int pos;
	bool used;
	char* data;
	int size;
	
	DSSaveFile* sramFile;
};

// These functions replease the standard library functions of the same name.
// As this header is included after the standard one, I have the chance to #define
// all of these to my own code.
//
// A #define is the only way, as redefinig the functions would cause linker errors.

// These functions need to be #undef'ed, as their definition is done with #includes
#undef feof
#undef stderr
#undef stdout
#undef stdin
#undef clearerr
#undef getc
#undef ferror

#define stdout ((DS::fileHandle*) -1)
#define stderr ((DS::fileHandle*) -2)
#define stdin ((DS::fileHandle*) -3)

#define FILE DS::fileHandle
//#define size_t int

//#define FAT_chdir FAT_CWD

FILE* 	std_fopen(const char* name, const char* mode);
void 	std_fclose(FILE* handle);
size_t 	std_fread(const void* ptr, size_t size, size_t numItems, FILE* handle);
size_t 	std_fwrite(const void* ptr, size_t size, size_t numItems, FILE* handle);
void 	std_fprintf(FILE* handle, const char* fmt, ...);
bool 	std_feof(FILE* handle);
void 	std_fflush(FILE* handle);
char* 	std_fgets(char* str, int size, FILE* file);
long int std_ftell(FILE* handle);
int 	std_fseek(FILE* handle, long int offset, int whence);
void 	std_clearerr(FILE* handle);
int 	std_getc(FILE* handle);
char* 	std_getcwd(char* dir, int dunno);
void 	std_cwd(char* dir);
int 	std_ferror(FILE* handle);

// Only functions used in the ScummVM source have been defined here!
#define fopen(name, mode) 					DS::std_fopen(name, mode)
#define fclose(handle) 						DS::std_fclose(handle)
#define fread(ptr, size, items, file)		DS::std_fread(ptr, size, items, file)
#define fwrite(ptr, size, items, file)		DS::std_fwrite(ptr, size, items, file)
#define feof(handle)						DS::std_feof(handle)
//#define fprintf(file, fmt, ...)				DS::fprintf(file, fmt, ##__VA_ARGS__)
#define fprintf(file, fmt, ...)				{ char str[128]; sprintf(str, fmt, ##__VA_ARGS__); DS::std_fwrite(str, strlen(str), 1, file); }
#define printf(fmt, ...)					consolePrintf(fmt, ##__VA_ARGS__)
#define fflush(file)						DS::std_fflush(file)
#define fgets(str, size, file)				DS::std_fgets(str, size, file)
#define ftell(handle)						DS::std_ftell(handle)
#define fseek(handle, offset, whence)		DS::std_fseek(handle, offset, whence)
#define clearerr(handle)					DS::std_clearerr(handle)
#define getc(handle)						DS::std_getc(handle)
#define getcwd(dir, dunno)					DS::std_getcwd(dir, dunno)
#define ferror(handle)						DS::std_ferror(handle)


}

#endif
/* ScummVMDS - Scumm Interpreter DS Port
 * Copyright (C) 2002-2004 The ScummVM project and Neil Millstone
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 
#ifndef _DS_FS_H
#define _DS_FS_H


#include "stdafx.h"
#include "common/array.h"
#include "common/str.h"

//#include <NDS/ARM9/console.h>
#include "fs.h"
#include "zipreader.h"
#include "ramsave.h"
#include "scummconsole.h"
#include "gba_nds_fat.h"
#include "backends/fs/abstract-fs.h"
//#include "backends/fs/fs.h"

// Fix name clash with FOTAQ engine, cutaway.h
#undef MAX_FILENAME_LENGTH
// This class is used when a Flash cart is in use

namespace DS {

class DSFileSystemNode : public AbstractFilesystemNode {
protected:
	static ZipFile* _zipFile;

	typedef class Common::String String;

	String _displayName;
	bool _isDirectory;
	bool _isValid;
	String _path;
	int _refCountVal;
	
public:
	DSFileSystemNode();
	DSFileSystemNode(const String &path);
	DSFileSystemNode(const DSFileSystemNode *node);
	DSFileSystemNode(const String& path, bool isDir);
	
	virtual String displayName() const {  return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }
	
	virtual bool listDir(AbstractFSList &list, ListMode mode = FilesystemNode::kListDirectoriesOnly) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *clone() const { return new DSFileSystemNode(this); }
	virtual AbstractFilesystemNode *child(const Common::String& name) const;
	static ZipFile* getZip() { return _zipFile; }
};


// This class is used when the GBAMP (GBA Movie Player) is used with a CompactFlash card

class GBAMPFileSystemNode : public AbstractFilesystemNode {
protected:
	typedef class Common::String String;

	String _displayName;
	bool _isDirectory;
	bool _isValid;
	String _path;
	
	int _refCountVal;
	
public:
	GBAMPFileSystemNode();
	GBAMPFileSystemNode(const String &path);
	GBAMPFileSystemNode(const String &path, bool isDirectory);
	GBAMPFileSystemNode(const GBAMPFileSystemNode *node);

	virtual String displayName() const {  return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }
	virtual bool listDir(AbstractFSList &list, ListMode mode = FilesystemNode::kListDirectoriesOnly) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *clone() const { return new GBAMPFileSystemNode(this); }
	virtual AbstractFilesystemNode *child(const Common::String& name) const;
	
};


// File reading


struct fileHandle {
	int pos;
	bool used;
	char* data;
	int size;
	
	DSSaveFile* sramFile;
};

// These functions replease the standard library functions of the same name.
// As this header is included after the standard one, I have the chance to #define
// all of these to my own code.
//
// A #define is the only way, as redefinig the functions would cause linker errors.

// These functions need to be #undef'ed, as their definition is done with #includes
#undef feof
#undef stderr
#undef stdout
#undef stdin
#undef clearerr
#undef getc
#undef ferror

#define stdout ((DS::fileHandle*) -1)
#define stderr ((DS::fileHandle*) -2)
#define stdin ((DS::fileHandle*) -3)

#define FILE DS::fileHandle
//#define size_t int

//#define FAT_chdir FAT_CWD

FILE* 	std_fopen(const char* name, const char* mode);
void 	std_fclose(FILE* handle);
size_t 	std_fread(const void* ptr, size_t size, size_t numItems, FILE* handle);
size_t 	std_fwrite(const void* ptr, size_t size, size_t numItems, FILE* handle);
void 	std_fprintf(FILE* handle, const char* fmt, ...);
bool 	std_feof(FILE* handle);
void 	std_fflush(FILE* handle);
char* 	std_fgets(char* str, int size, FILE* file);
long int std_ftell(FILE* handle);
int 	std_fseek(FILE* handle, long int offset, int whence);
void 	std_clearerr(FILE* handle);
int 	std_getc(FILE* handle);
char* 	std_getcwd(char* dir, int dunno);
void 	std_cwd(char* dir);
int 	std_ferror(FILE* handle);

// Only functions used in the ScummVM source have been defined here!
#define fopen(name, mode) 					DS::std_fopen(name, mode)
#define fclose(handle) 						DS::std_fclose(handle)
#define fread(ptr, size, items, file)		DS::std_fread(ptr, size, items, file)
#define fwrite(ptr, size, items, file)		DS::std_fwrite(ptr, size, items, file)
#define feof(handle)						DS::std_feof(handle)
//#define fprintf(file, fmt, ...)				DS::fprintf(file, fmt, ##__VA_ARGS__)
#define fprintf(file, fmt, ...)				{ char str[128]; sprintf(str, fmt, ##__VA_ARGS__); DS::std_fwrite(str, strlen(str), 1, file); }
#define printf(fmt, ...)					consolePrintf(fmt, ##__VA_ARGS__)
#define fflush(file)						DS::std_fflush(file)
#define fgets(str, size, file)				DS::std_fgets(str, size, file)
#define ftell(handle)						DS::std_ftell(handle)
#define fseek(handle, offset, whence)		DS::std_fseek(handle, offset, whence)
#define clearerr(handle)					DS::std_clearerr(handle)
#define getc(handle)						DS::std_getc(handle)
#define getcwd(dir, dunno)					DS::std_getcwd(dir, dunno)
#define ferror(handle)						DS::std_ferror(handle)


}

#endif
