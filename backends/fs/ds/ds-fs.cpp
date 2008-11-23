/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "common/str.h"
#include "common/util.h"
//#include <NDS/ARM9/console.h> //basic print funcionality
#include "backends/fs/ds/ds-fs.h"
#include "backends/fs/stdiostream.h"
#include "dsmain.h"
#include "fat/gba_nds_fat.h"



namespace DS {

//////////////////////////////////////////////////////////////
// DSFileSystemNode - Flash ROM file system using Zip files //
//////////////////////////////////////////////////////////////

ZipFile*	DSFileSystemNode::_zipFile = NULL;
char		currentDir[128];
bool		readPastEndOfFile = false;

DSFileSystemNode::DSFileSystemNode() {
	_displayName = "ds:/";
	_path = "ds:/";
	_isValid = true;
	_isDirectory = true;
	_path = "ds:/";

/*	if (!_archive) {
		_archive = (GBFS_FILE *) find_first_gbfs_file(scummdata);
		if (!_archive) consolePrintf("No GBFS archive found!\n");
	}*/

	if (!_zipFile) {
		_zipFile = new ZipFile();
	}
}

DSFileSystemNode::DSFileSystemNode(const Common::String& path) {
//	consolePrintf("--%s ",path.c_str());

	char disp[128];
	char* pathStr = (char *) path.c_str();

	int lastSlash = 3;
	for (int r = 0; r < (int) strlen(pathStr) - 1; r++) {
		if (path[r] == '\\') {
			lastSlash = r;
		}
	}

	strcpy(disp, pathStr + lastSlash + 1);

	_displayName = Common::String(disp);
	_path = path;
//	_isValid = true;
//	_isDirectory = false;

	if (!strncmp(pathStr, "ds:/", 4)) {
		pathStr += 4;
	}

	if (*pathStr == '\0') {
		_isValid = true;
		_isDirectory = true;
		return;
	}

	_zipFile->setAllFilesVisible(true);
	if (_zipFile->findFile(pathStr)) {
		_isValid = true;
		_isDirectory = _zipFile->isDirectory();
	} else {
		_isValid = false;
		_isDirectory = false;
	}
	_zipFile->setAllFilesVisible(false);

//	consolePrintf("%s - Found: %d, Dir: %d\n", pathStr, _isValid, _isDirectory);
}

DSFileSystemNode::DSFileSystemNode(const Common::String& path, bool isDir) {
//	consolePrintf("--%s ",path.c_str());

	char disp[128];
	char* pathStr = (char *) path.c_str();
	int lastSlash = 3;
	for (int r = 0; r < (int) strlen(pathStr) - 1; r++) {
		if (path[r] == '\\') {
			lastSlash = r;
		}
	}

	strcpy(disp, pathStr + lastSlash + 1);

	_displayName = Common::String(disp);
	_path = path;
	_isValid = true;
	_isDirectory = isDir;

//	consolePrintf("Found: %d, Dir: %d\n", _isValid, _isDirectory);
}

DSFileSystemNode::DSFileSystemNode(const DSFileSystemNode* node) {
	//TODO: not implemented?
}

AbstractFSNode *DSFileSystemNode::getChild(const Common::String& n) const {
	if (_path.lastChar() == '\\') {
		return new DSFileSystemNode(_path + n);
	} else {
		return new DSFileSystemNode(_path + "\\" + n);
	}

	return NULL;
}

bool DSFileSystemNode::getChildren(AbstractFSList &dirList, ListMode mode, bool hidden) const {
//	consolePrintf("Listdir\n");
//	consolePrintf("Directory\n");

	//TODO: honor the hidden flag

	char temp[128];
	strcpy(temp, _path.c_str());

//	consolePrintf("This dir: %s\n", temp);

	if ((temp[0] == 'd') && (temp[1] == 's') && (temp[2] == ':') && (temp[3] == '/')) {
		if (strlen(temp) != 4) {
			_zipFile->changeDirectory(&temp[4]);
		} else {
			_zipFile->changeToRoot();

/*			// This is the root dir, so add the RAM folder
			DSFileSystemNode* dsfsn = new DSFileSystemNode("ds:/ram");
			dsfsn->_isDirectory = true;
			dirList->push_back(wrap(dsfsn));
*/
		}
	} else {
		_zipFile->changeDirectory(temp);
	}

	if (_zipFile->restartFile()) {
		do {
			char n[128];
			_zipFile->getFileName(n);

//			consolePrintf("file: %s\n", n);
			if ( (_zipFile->isDirectory() && ((mode == Common::FSNode::kListDirectoriesOnly) || (mode == Common::FSNode::kListAll)) )
				|| (!_zipFile->isDirectory() && ((mode == Common::FSNode::kListFilesOnly) || (mode == Common::FSNode::kListAll)) ) )
			{
				DSFileSystemNode* dsfsn = new DSFileSystemNode("ds:/" + Common::String(n), _zipFile->isDirectory());
				dsfsn->_isDirectory = _zipFile->isDirectory();
				dirList.push_back((dsfsn));
			}

		} while (_zipFile->skipFile());
	}

	return true;
}

AbstractFSNode* DSFileSystemNode::getParent() const {
//	consolePrintf("parent\n");
	DSFileSystemNode *p;

	if (_path != "ds:/") {
		char *path = (char *) _path.c_str();
		int lastSlash = 4;

		for (int r = 4; r < (int) strlen((char *) path); r++) {
			if (path[r] == '\\') {
				lastSlash = r;
			}
		}

		p = new DSFileSystemNode(Common::String(path, lastSlash));
		((DSFileSystemNode *) (p))->_isDirectory = true;
	} else {
		p = new DSFileSystemNode();
	}

	return p;
}

Common::SeekableReadStream *DSFileSystemNode::openForReading() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

Common::WriteStream *DSFileSystemNode::openForWriting() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}

//////////////////////////////////////////////////////////////////////////
// GBAMPFileSystemNode - File system using GBA Movie Player and CF card //
//////////////////////////////////////////////////////////////////////////

GBAMPFileSystemNode::GBAMPFileSystemNode() {
	_displayName = "mp:/";
	_path = "mp:/";
	_isValid = true;
	_isDirectory = true;
	_path = "mp:/";
}

GBAMPFileSystemNode::GBAMPFileSystemNode(const Common::String& path) {
//	consolePrintf("'%s'",path.c_str());

	char disp[128];
	char* pathStr = (char *) path.c_str();
	int lastSlash = 3;
	for (int r = 0; r < (int) strlen(pathStr) - 1; r++) {
		if ((path[r] == '\\') || (path[r] == '/')) {
			lastSlash = r;
		}
	}

	strcpy(disp, pathStr + lastSlash + 1);

	char check[128];
	int fileOrDir;

	if (!strcmp(pathStr, "mp:/")) {
		// This is the root directory
		_isDirectory = true;
		_isValid = false;		// Old code returned false here, but I'm not sure why
	} else if ((strlen(pathStr) > 4) && (!strncmp(pathStr, "mp:/", 4))) {
		// Files which start with mp:/
		
		// Clear the filename to 128 bytes, because a libfat bug occationally tries to read in this area.
		memset(check, 0, 128);
		strcpy(check, pathStr + 3);

		// Remove terminating slash - FileExists fails without this
		if (check[strlen(check) - 1] == '/') {
			check[strlen(check) - 1] = 0;
		}
		fileOrDir = FAT_FileExists(check);

		_isDirectory = fileOrDir == FT_DIR;
		_isValid = fileOrDir == FT_FILE;
	} else {
		// Files which don't start with mp:/ (like scummvm.ini in default implementation)

		// Clear the filename to 128 bytes, because a libfat bug occationally tries to read in this area.
		memset(check, 0, 128);
		strcpy(check, pathStr);

		// Remove terminating slash - FileExists fails on directories without this
		if (check[strlen(check) - 1] == '/') {
			check[strlen(check) - 1] = 0;
		}
		fileOrDir = FAT_FileExists(check);

		_isDirectory = fileOrDir == FT_DIR;
		_isValid = fileOrDir == FT_FILE;

	}
				

//	consolePrintf("Path: %s \n", check);

	_displayName = Common::String(disp);
	_path = path;
}

GBAMPFileSystemNode::GBAMPFileSystemNode(const Common::String& path, bool isDirectory) {
	//consolePrintf("'%s'",path.c_str());

	char disp[128];
	char* pathStr = (char *) path.c_str();
	int lastSlash = 3;
	for (int r = 0; r < (int) strlen(pathStr) - 1; r++) {
		if ((path[r] == '\\') || (path[r] == '/')) {
			lastSlash = r;
		}
	}

	strcpy(disp, pathStr + lastSlash + 1);

	_displayName = Common::String(disp);
	_path = path;
	_isValid = true;
	_isDirectory = isDirectory;
}


GBAMPFileSystemNode::GBAMPFileSystemNode(const GBAMPFileSystemNode* node) {
	//TODO: not implemented?
}

AbstractFSNode *GBAMPFileSystemNode::getChild(const Common::String& n) const {
	if (_path.lastChar() == '\\') {
		return new DSFileSystemNode(_path + n);
	} else {
		return new DSFileSystemNode(_path + "\\" + n);
	}

	return NULL;
}

bool GBAMPFileSystemNode::getChildren(AbstractFSList& dirList, ListMode mode, bool hidden) const {
//	consolePrintf("Listdir\n");

	//TODO: honor the hidden flag

	enum { TYPE_NO_MORE = 0, TYPE_FILE = 1, TYPE_DIR = 2 };

	char temp[128], fname[256], *path, *pathTemp;
	strcpy(temp, _path.c_str());

	path = temp + 3;

	pathTemp = path;
	while (*pathTemp) {
		if (*pathTemp == '\\') {
			*pathTemp = '/';
		}
		pathTemp++;
	}

	// consolePrintf("This dir: %s\n", path);
	FAT_chdir(path);

	int entryType = FAT_FindFirstFileLFN(fname);

	while (entryType != TYPE_NO_MORE) {

		if ( ((entryType == TYPE_DIR) && ((mode == Common::FSNode::kListDirectoriesOnly) || (mode == Common::FSNode::kListAll)))
		||   ((entryType == TYPE_FILE) && ((mode == Common::FSNode::kListFilesOnly) || (mode == Common::FSNode::kListAll))) ) {
			GBAMPFileSystemNode* dsfsn;

			//consolePrintf("Fname: %s\n", fname);

			if (strcmp(fname, ".") && strcmp(fname, "..")) {

				if (!strcmp(path, "/")) {
					dsfsn = new GBAMPFileSystemNode("mp:" + Common::String(path) + Common::String(fname), entryType == TYPE_DIR);
				} else {
					dsfsn = new GBAMPFileSystemNode("mp:" + Common::String(path) + Common::String("/") + Common::String(fname), entryType == TYPE_DIR);
				}

//				dsfsn->_isDirectory = entryType == DIR;
				dirList.push_back((dsfsn));
			}
		} else {
//			consolePrintf("Skipping %s\n", fname);
		}

		entryType = FAT_FindNextFileLFN(fname);
	}

//	consolePrintf("No more");

	FAT_chdir("/");

	return true;
}

AbstractFSNode* GBAMPFileSystemNode::getParent() const {
//	consolePrintf("parent\n");
	GBAMPFileSystemNode *p;

	if (_path != "mp:/") {
		char *path = (char *) _path.c_str();
		int lastSlash = 4;

		for (int r = 4; r < (int) strlen((char *) path); r++) {
			if (path[r] == '/') {
				lastSlash = r;
			}
		}

		p = new GBAMPFileSystemNode(Common::String(path, lastSlash));
		p->_isDirectory = true;
	} else {
		p = new GBAMPFileSystemNode();
	}

	return p;
}

Common::SeekableReadStream *GBAMPFileSystemNode::openForReading() {
//	consolePrintf("Opening: %s\n", getPath().c_str());

	if (!strncmp(getPath().c_str(), "mp:/", 4)) {
		return StdioStream::makeFromPath(getPath().c_str() + 3, false);
	} else {
		return StdioStream::makeFromPath(getPath().c_str(), false);
	}
}

Common::WriteStream *GBAMPFileSystemNode::openForWriting() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}

// Stdio replacements
#define MAX_FILE_HANDLES 32

bool inited = false;
DS::fileHandle handle[MAX_FILE_HANDLES];

FILE* std_fopen(const char* name, const char* mode) {
	if (!inited) {
		for (int r = 0; r < MAX_FILE_HANDLES; r++) {
			handle[r].used = false;
		}
		inited = true;
		currentDir[0] = '\0';
	}

	char* realName = (char *) name;

	// Remove file system prefix
	if ((name[0] == 'd') && (name[1] == 's') && (name[2] == ':') && (name[3] == '/')) {
		realName += 4;
	}

	if ((name[0] == 'm') && (name[1] == 'p') && (name[2] == ':') && (name[3] == '/')) {
		realName += 4;
	}

//	consolePrintf("Open file:");
//	consolePrintf("'%s', [%s]", name, realName);

	if (DS::isGBAMPAvailable()) {
		FAT_chdir("/");

		// Turn all back slashes into forward slashes for gba_nds_fat
		char* p = realName;
		while (*p) {
			if (*p == '\\') *p = '/';
			p++;
		}

		FAT_FILE* result = FAT_fopen(realName, mode);

		if (result == 0) {
//			consolePrintf("Error code %d\n", result);
			//consolePrintf("Opening file %s\n", realName);
		} else {
//			consolePrintf("Opened file %d\n", result);
		}
//		MT_memoryReport();

		return (FILE *) result;
	}

	// Fail to open file for writing.  It's in ROM!

	// Allocate a file handle
	int r = 0;
	while (handle[r].used) {
		r++;
		assert(r < MAX_FILE_HANDLES);
	}

#ifdef GBA_SRAM_SAVE
	if (strchr(mode, 'w')) {
//		consolePrintf("Writing %s\n", realName);
		handle[r].sramFile = (DSSaveFile *) DSSaveFileManager::instance()->openSavefile(realName, true);
	} else {
//		consolePrintf("Reading %s\n", realName);
		handle[r].sramFile = (DSSaveFile *) DSSaveFileManager::instance()->openSavefile(realName, false);
	}
#endif

	if (handle[r].sramFile) {
		handle[r].used = true;
		handle[r].pos = 0;
		handle[r].data = NULL;
		handle[r].size = handle[r].sramFile->getSize();
//		consolePrintf("Found it");
		return &handle[r];
	}

//	consolePrintf("Not in SRAM!");

	char* data;

	ZipFile* zip = DSFileSystemNode::getZip();
	if (!zip) {
//		consolePrintf("No zip yet!");
		return NULL;
	}

	// Grab the data if it exists

	zip->setAllFilesVisible(true);

	if (currentDir[0] != 0) {
		char nameWithPath[128];
		sprintf(nameWithPath, "%s\%s", currentDir, realName);
		strcpy(realName, nameWithPath);
	}

//	consolePrintf("fopen(%s, %s)\n", realName, name);

	if (zip->findFile(realName)) {
		data = zip->getFile();
		zip->setAllFilesVisible(false);

		// Allocate a file handle
		int r = 0;
		while (handle[r].used) r++;


		handle[r].used = true;
		handle[r].pos = 0;
		handle[r].data = data;
		handle[r].size = zip->getFileSize();

//		consolePrintf("Opened file %d: %s (%s)   ", r, realName, name);
		return &handle[r];
	} else {
		zip->setAllFilesVisible(false);
//		consolePrintf("Not found: %s (%s)  ", realName, name);
		return NULL;
	}
}

void std_fclose(FILE* handle) {

	if (DS::isGBAMPAvailable()) {
		FAT_fclose((FAT_FILE *) handle);
		return;
	}

	handle->used = false;
	if (handle->sramFile) {
		delete handle->sramFile;
		handle->sramFile = NULL;
	}
}

size_t std_fread(const void* ptr, size_t size, size_t numItems, FILE* handle) {
//	consolePrintf("fread %d,%d %d ", size, numItems, ptr);

	if (DS::isGBAMPAvailable()) {
		readPastEndOfFile = false;

		int bytes = FAT_fread((void *) ptr, size, numItems, (FAT_FILE *) handle);
		if (!FAT_feof((FAT_FILE *) handle)) {
			return numItems;
		} else {
//			consolePrintf("Read past end of file: %d read out of %d\n", bytes / size, numItems);
			if ((size_t)bytes != size * numItems) readPastEndOfFile = true;			
			return bytes / size;
		}
		return numItems;
	}

	if (handle->sramFile) {
		int bytes = 0;
		int result = 1;
		//consolePrintf("fread size=", size * numItems);
		for (int r = 0; (r < (s32) size * (s32) numItems) && (result > 0); r++) {
			result = handle->sramFile->read((void *) ( ((char *) (ptr)) + r), 1);
			bytes += result;
			//consolePrintf("'%d',", ((char *) (ptr))[0]);
		}

		handle->pos += bytes;

		return bytes / size;
	}

	if ((size_t)(handle->pos + size * numItems) > handle->size) {
		numItems = (handle->size - handle->pos) / size;
		if (numItems < 0) numItems = 0;
	}

//	consolePrintf("read %d  ", size * numItems);

	memcpy((void *) ptr, handle->data + handle->pos, size * numItems);
	handle->pos += size * numItems;

	return numItems;
}

size_t std_fwrite(const void* ptr, size_t size, size_t numItems, FILE* handle) {
	if ((handle == stdin)) return 0;

	if ((handle == stderr) || (handle == stdout)) {
//		consolePrintf((char *) ptr);
		return size;
	}

	//consolePrintf("fwrite size=%d\n", size * numItems);

	if (DS::isGBAMPAvailable()) {
		FAT_fwrite(((char *) (ptr)), size, numItems, (FAT_FILE *) handle);
		return numItems;

		int length = size * numItems;
		int pos = 0;

		while (pos < length) {
			int amount = length > 512? 512: length;

			FAT_fwrite(((char *) (ptr)) + pos, 1, amount, (FAT_FILE *) handle);
			length -= amount;
			pos += amount;
		}

		return numItems;
	}

	if (handle->sramFile) {
		handle->sramFile->write(ptr, size);
		return size;
	} else {
		return 0;
	}
}

bool std_feof(FILE* handle) {
//	consolePrintf("feof ");

	if (DS::isGBAMPAvailable()) {
		return readPastEndOfFile && FAT_feof((FAT_FILE *) handle);
	}

	if (handle->sramFile) {
		return handle->sramFile->eos();
	}

//	consolePrintf("feof %s", handle->pos >= handle->size? "true": "false");
	return handle->pos >= handle->size;
}

int std_fflush(FILE* handle) {
	//FIXME: not implemented?
//	consolePrintf("fflush ");
	return 0;
}

long int std_ftell(FILE* handle) {
	if (DS::isGBAMPAvailable()) {
		return FAT_ftell((FAT_FILE *) handle);
	}

	return handle->pos;
}

int std_fseek(FILE* handle, long int offset, int whence) {
//	consolePrintf("fseek %d %d ", offset, whence);

	if (DS::isGBAMPAvailable()) {
		return FAT_fseek((FAT_FILE *) handle, offset, whence);
	}

	switch (whence) {
		case SEEK_CUR:
			handle->pos += offset;
			break;
		case SEEK_SET:
			handle->pos = offset;
			break;
		case SEEK_END:
			handle->pos = handle->size + offset;
			break;
		default:
			handle->pos = offset;
			break;
	}

	return 0;
}

int std_ferror(FILE* handle) {
	//FIXME: not implemented?
//	consolePrintf("ferror ");
	
	return readPastEndOfFile;
}

void std_clearerr(FILE* handle) {
	//FIXME: not implemented?
	readPastEndOfFile = false;
//	consolePrintf("clearerr ");
}

void std_fprintf(FILE* handle, const char* fmt, ...) {
	consolePrintf(fmt);
}


} // namespace DS
