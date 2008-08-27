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
#include "dsmain.h"
#include "fat/gba_nds_fat.h"



namespace DS {

//////////////////////////////////////////////////////////////
// DSFileSystemNode - Flash ROM file system using Zip files //
//////////////////////////////////////////////////////////////

ZipFile*	DSFileSystemNode::_zipFile = NULL;
char		currentDir[128];

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

DSFileSystemNode::DSFileSystemNode(const String& path) {
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

	_displayName = String(disp);
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

DSFileSystemNode::DSFileSystemNode(const String& path, bool isDir) {
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

	_displayName = String(disp);
	_path = path;
	_isValid = true;
	_isDirectory = isDir;

//	consolePrintf("Found: %d, Dir: %d\n", _isValid, _isDirectory);
}

DSFileSystemNode::DSFileSystemNode(const DSFileSystemNode* node) {
	//TODO: not implemented?
}

AbstractFilesystemNode *DSFileSystemNode::getChild(const Common::String& n) const {
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
			if ( (_zipFile->isDirectory() && ((mode == FilesystemNode::kListDirectoriesOnly) || (mode == FilesystemNode::kListAll)) )
				|| (!_zipFile->isDirectory() && ((mode == FilesystemNode::kListFilesOnly) || (mode == FilesystemNode::kListAll)) ) )
			{
				DSFileSystemNode* dsfsn = new DSFileSystemNode("ds:/" + String(n), _zipFile->isDirectory());
				dsfsn->_isDirectory = _zipFile->isDirectory();
				dirList.push_back((dsfsn));
			}

		} while (_zipFile->skipFile());
	}

	return true;
}

AbstractFilesystemNode* DSFileSystemNode::getParent() const {
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

		p = new DSFileSystemNode(String(path, lastSlash));
		((DSFileSystemNode *) (p))->_isDirectory = true;
	} else {
		p = new DSFileSystemNode();
	}

	return p;
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

GBAMPFileSystemNode::GBAMPFileSystemNode(const String& path) {
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
	int success;

	memset(check, 0, 128);
	if (strlen(pathStr) > 3) {
		strcpy(check, pathStr + 3);
		if (check[strlen(check) - 1] == '/') {
			check[strlen(check) - 1] = 0;
		}
		success = FAT_FileExists(check);
	} else {
		success = FT_DIR;
	}
//	consolePrintf("Path: %s  (%d)\n", check, success);

	_displayName = String(disp);
	_path = path;
	_isValid = success == FT_FILE;
	_isDirectory = success == FT_DIR;
}

GBAMPFileSystemNode::GBAMPFileSystemNode(const String& path, bool isDirectory) {
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

	_displayName = String(disp);
	_path = path;
	_isValid = true;
	_isDirectory = isDirectory;
}


GBAMPFileSystemNode::GBAMPFileSystemNode(const GBAMPFileSystemNode* node) {
	//TODO: not implemented?
}

AbstractFilesystemNode *GBAMPFileSystemNode::getChild(const Common::String& n) const {
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

	consolePrintf("This dir: %s\n", path);
	FAT_chdir(path);

	int entryType = FAT_FindFirstFileLFN(fname);

	while (entryType != TYPE_NO_MORE) {

		if ( ((entryType == TYPE_DIR) && ((mode == FilesystemNode::kListDirectoriesOnly) || (mode == FilesystemNode::kListAll)))
		||   ((entryType == TYPE_FILE) && ((mode == FilesystemNode::kListFilesOnly) || (mode == FilesystemNode::kListAll))) ) {
			GBAMPFileSystemNode* dsfsn;

			consolePrintf("Fname: %s\n", fname);

			if (strcmp(fname, ".") && strcmp(fname, "..")) {

				if (!strcmp(path, "/")) {
					dsfsn = new GBAMPFileSystemNode("mp:" + String(path) + String(fname), entryType == TYPE_DIR);
				} else {
					dsfsn = new GBAMPFileSystemNode("mp:" + String(path) + String("/") + String(fname), entryType == TYPE_DIR);
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

AbstractFilesystemNode* GBAMPFileSystemNode::getParent() const {
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

		p = new GBAMPFileSystemNode(String(path, lastSlash));
		p->_isDirectory = true;
	} else {
		p = new GBAMPFileSystemNode();
	}

	return p;
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
//	consolePrintf("'%s', [%s]", realName, mode);

	if (DS::isGBAMPAvailable()) {
		FAT_chdir("/");

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
	while (handle[r].used) r++;

	if (strchr(mode, 'w')) {
//		consolePrintf("Writing %s\n", realName);
		handle[r].sramFile = (DSSaveFile *) DSSaveFileManager::instance()->openSavefile(realName, true);
	} else {
//		consolePrintf("Reading %s\n", realName);
		handle[r].sramFile = (DSSaveFile *) DSSaveFileManager::instance()->openSavefile(realName, false);
	}

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
		int bytes = FAT_fread((void *) ptr, size, numItems, (FAT_FILE *) handle);
		if (!std_feof(handle)) {
			return numItems;
		} else {
//			consolePrintf("Read past end of file: %d read out of %d\n", bytes / size, numItems);
			return bytes / size;
		}
		return numItems;

/*		int item = 0;
		u8* data = (u8 *) ptr;
		while ((item < numItems) && (!FAT_feof((FAT_FILE *) handle))) {


			int bytes = 0;
			while ((bytes < size) && (!FAT_feof((FAT_FILE *) handle))) {
				*data++ = FAT_fgetc((FAT_FILE *) handle);
				bytes++;
			}

			item++;

		}

		return item;
*/
		int items = 0;

		//for (int r = 0; r < numItems; r++) {
			if (!std_feof(handle)) {
/*				for (int t = 0; t < size; t++) {
					if (feof(handle)) eof = true;
					*(((char *) (ptr)) + r * size + t) = getc(handle);
				}*/
				int left = size * numItems;
				int bytesRead = -1;

				while ((left > 0) && (!FAT_feof((FAT_FILE *) handle))) {
					int amount = left > 8192? 8192: left;
//					do {
						bytesRead = FAT_fread((void *) ptr, 1, amount, (FAT_FILE *) handle);
/*						if (bytesRead == 0) {
							consolePrintf("Pos:%d items:%d num:%d amount:%d read:%d\n", ftell(handle), items, numItems, amount, bytesRead);
							left++;

							int pos = ftell(handle);

							fseek(handle, 0, SEEK_SET);
							int c = getc(handle);
							fseek(handle, pos - 1024, SEEK_SET);
							fread(ptr, 1024, 1, handle);
							swiWaitForVBlank();
							//while (true);
						}

					} while (bytesRead == 0);
*/
					left -= bytesRead;
					ptr = ((char *) (ptr)) + bytesRead;
				}

				items = numItems - (left / size);

//				FAT_fread((void *) ptr, size, 1, ((int) (handle)) - 1);
//				ptr = ((char *) (ptr)) + size;
			}
//		}

//		consolePrintf("...done %d \n", items)

		return items;
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

	if (handle->pos + size * numItems > handle->size) {
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

void std_fprintf(FILE* handle, const char* fmt, ...) {
	consolePrintf(fmt);
}

bool std_feof(FILE* handle) {
//	consolePrintf("feof ");

	if (DS::isGBAMPAvailable()) {
		return FAT_feof((FAT_FILE *) handle);
	}

	if (handle->sramFile) {
		return handle->sramFile->eos();
	}

//	consolePrintf("feof %s", handle->pos >= handle->size? "true": "false");
	return handle->pos >= handle->size;
}

void std_fflush(FILE* handle) {
	//FIXME: not implemented?
//	consolePrintf("fflush ");
}

char* std_fgets(char* str, int size, FILE* file) {
//	consolePrintf("fgets file=%d ", file);

	if (DS::isGBAMPAvailable()) {
		char* s = str;
		while ((*s++ = std_getc(file)) >= 32) {
//			consolePrintf("%d ", *s);
		}
		*s = 0;

//		consolePrintf("Read:%s\n", str);

		return str;
	}

	if (file->sramFile) {
		file->pos--;
		int p = -1;
		do {
			file->pos++;
			p++;
			file->sramFile->read((char *) &str[p], 1);
//			consolePrintf("%d,", str[p]);
		} while ((str[p] >= 32) && (!std_feof(file)) && (p < size));
		str[p + 1] = 0;
		file->pos++;
//		consolePrintf("Read:%s\n", str);
		return str;
	}

	return NULL;
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

void std_clearerr(FILE* handle) {
	//FIXME: not implemented?
//	consolePrintf("clearerr ");
}

int std_getc(FILE* handle) {
	if (DS::isGBAMPAvailable()) {
		char c;
		FAT_fread(&c, 1, 1, (FAT_FILE *) handle);

		return c;
	}

//	consolePrintf("fgetc ");
	return 0;				// Not supported yet
}

char* std_getcwd(char* dir, int dunno) {
//	consolePrintf("getcwd ");
	dir[0] = '\0';
	return dir;			// Not supported yet
}

void std_cwd(char* dir) {
	char buffer[128];
	strcpy(buffer, dir);
	char* realName = buffer;

	if (DS::isGBAMPAvailable()) {
		if ((strlen(dir) >= 4) && (dir[0] == 'm') && (dir[1] == 'p') && (dir[2] == ':') && (dir[3] == '/')) {
			realName += 4;
		}

	//	consolePrintf("Real cwd:%d\n", realName);

		char* p = realName;
		while (*p) {
			if (*p == '\\') *p = '/';
			p++;
		}

	//	consolePrintf("Real cwd:%d\n", realName);
		FAT_chdir(realName);
	} else {
		if ((strlen(dir) >= 4) && (dir[0] == 'd') && (dir[1] == 's') && (dir[2] == ':') && (dir[3] == '/')) {
			realName += 4;
		}

		char* p = realName;
		while (*p) {
			if (*p == '\\') *p = '/';
			p++;
		}

		strcpy(currentDir, realName);
		if (*(currentDir + strlen(currentDir) - 1) == '/') {
			*(currentDir + strlen(currentDir) - 1) = '\0';
		}
//		consolePrintf("CWD: %s\n", currentDir);
	}
}

int std_ferror(FILE* handle) {
	return 0;
}

} // namespace DS
