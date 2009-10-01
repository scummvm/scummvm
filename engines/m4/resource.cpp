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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "m4/m4.h"
#include "m4/resource.h"
#include "m4/events.h"

namespace M4 {

FileSystem::FileSystem(const char *hashFilename) {

	for (int i = 0; i < 10; i++) {
		_hagEntries[i].filename[0] = '\0';
		_hagEntries[i].fileIndex = 0; // Was -1
		_hagEntries[i].hagFile = 0;
	}

	Common::File hashFile;
	uint32 hashSize;

	hashFile.open(hashFilename);

	if (!hashFile.isOpen()) {
		printf("FileSystem::FileSystem: error opening hash %s\n", hashFilename);
	}

	hashSize = hashFile.readUint32LE();

	//printf("FileSystem::FileSystem: hashSize = %d\n", hashSize);

	/* load file records and add them to the hash list */
	for (uint i = 0; i < hashSize; i++) {
		HashFileEntry entry;
		hashFile.read(entry.filename, kM4MaxFilenameSize);
		str_lower(entry.filename);
		entry.hagfile = hashFile.readByte();
		hashFile.readByte();
		entry.offset = hashFile.readUint32LE();
		entry.size = hashFile.readUint32LE();
		hashFile.readUint32LE();

		if (entry.filename[0]) {
			/*
			printf("  filename: %s\n", entry.filename);
			printf("  hagfile: %d\n", entry.hagfile);
			printf("  disks: %d\n", entry.disks);
			printf("  offset: %08X\n", entry.offset);
			printf("  size: %d\n", entry.size);
			printf("  next: %08X\n", entry.next);
			*/
			_fileEntries[entry.filename] = entry;
		}

	}

	/* load hagfile records and update the list */
	while (!hashFile.eos()) {
		HashHagEntry entry;
		hashFile.read(entry.filename, kM4MaxFilenameSize);
		entry.fileIndex = hashFile.readByte();
		if (hashFile.eos())
			break;

		changeExtension(_hagEntries[entry.fileIndex].filename, entry.filename, "HAG");
		_hagEntries[entry.fileIndex].fileIndex = entry.fileIndex;

		_hagEntries[entry.fileIndex].hagFile = new Common::File();
		_hagEntries[entry.fileIndex].hagFile->open(_hagEntries[entry.fileIndex].filename);

		if (!_hagEntries[entry.fileIndex].hagFile->isOpen()) {
			printf("FileSystem::FileSystem: error opening hag %s\n", _hagEntries[entry.fileIndex].filename);
		}

	}

	hashFile.close();

}

FileSystem::~FileSystem() {

	for (int i = 0; i < 10; i++) {
		if (_hagEntries[i].hagFile)
			delete _hagEntries[i].hagFile;
	}

}

Common::SeekableReadStream *FileSystem::loadFile(const char *resourceName, bool preloadFlag) {
	const HashFileEntry *hfe = getHashFileEntry(resourceName);
	Common::SeekableReadStream *result = NULL;

	if (hfe) {
		//printf("FileSystem::loadFile() success opening %s\n", filename);
		HashHagEntry *hagEntry = &_hagEntries[hfe->hagfile];

		if (preloadFlag) {
			// Creates a MemoryReadStream object that contains all of the resource in memory
			hagEntry->hagFile->seek(hfe->offset);
			result = _hagEntries[hfe->hagfile].hagFile->readStream(hfe->size);
		}
		else
			// Creates a SeekableSubReadStream, which will read the data in from disk as the
			// caller reads in data
			result = new Common::SeekableSubReadStream(hagEntry->hagFile, hfe->offset,
				hfe->offset + hfe->size);

	} else {
		printf("FileSystem::loadFile() error opening %s\n", resourceName);
	}

	return result;
}

const FileSystem::HashFileEntry *FileSystem::getHashFileEntry(const char *filename) {
	char resourceName[20];
	strcpy(resourceName, filename);
	str_lower(resourceName);

	FileHashMap::const_iterator entry = _fileEntries.find(filename);
	if (entry != _fileEntries.end())
		return &(entry->_value);
	else
		return NULL;
}

void FileSystem::changeExtension(char *destName, const char *sourceName, const char *extension) {
	if (sourceName != destName)
		strcpy(destName, sourceName);
	char *dot = strrchr(destName, '.');
	if (dot != NULL)
		*dot = 0;

	strcat(destName, ".");
	strcat(destName, extension);

	str_upper(destName);
}

//--------------------------------------------------------------------------

ResourceManager::~ResourceManager() {
	ResourceIterator i;
	for (i = _resources.begin(); i != _resources.end(); ++i) {
		Resource *r = (*i).get();
		delete r->stream;
	}
}

Common::SeekableReadStream *ResourceManager::get(const char *resourceName, bool preloadFlag) {
	char lowerName[kM4MaxFilenameSize];

	strcpy(lowerName, resourceName);
	str_lower(lowerName);

	// Check whether the resource is already loaded
	ResourceIterator i;
	for (i = _resources.begin(); i != _resources.end(); ++i) {
		Resource *r = (*i).get();
		if (!strcmp(r->name, resourceName)) {
			// Just in case resource was marked to be purged, reactive it again
			r->flags &= ~kResourcePurge;

			// Return the existing copy of the resource
			r->stream->seek(0, SEEK_SET);
			return r->stream;
		}
	}

	// the resource wasn't found in the list, load it from disk
	Resource *newRes = new Resource();
	strncpy(newRes->name, resourceName, 63);
	newRes->name[63] = '\0';
	newRes->flags = 0;
	newRes->stream = loadResource(resourceName, preloadFlag);

	_resources.push_back(ResourceList::value_type(newRes));
	return newRes->stream;
}

void ResourceManager::toss(const char *resourceName) {
	ResourceIterator i;
	for (i = _resources.begin(); i != _resources.end(); ++i) {
		Resource *r = (*i).get();

		if (!strcmp(r->name, resourceName)) {
			r->flags |= kResourcePurge;
			//printf("M4ResourceManager::toss: mark resource %s to be purged\n", resourceName);
		}
	}
}

void ResourceManager::purge() {
	ResourceIterator i = _resources.begin();
	while (i != _resources.end()) {
		Resource *r = (*i).get();

		if (r->flags & kResourcePurge) {
			delete r->stream;
			i = _resources.erase(i);
		} else {
			++i;
		}
	}
}

void ResourceManager::dump() {
	_vm->_events->getConsole()->DebugPrintf("Scene resources:\n");

	int index = 0;
	ResourceIterator i;
	for (i = _resources.begin(); i != _resources.end(); ++i) {
		Resource *r = (*i).get();

		if (!(r->flags & kResourcePurge)) {
			_vm->_events->getConsole()->DebugPrintf(
				"Resource #%i, name: %s, handle pointer: %p, size: %d, flags: %02X\n",
				index++, r->name, r->buffer, r->stream->size(), r->flags);
		}
	}
}

//--------------------------------------------------------------------------

const char *madsConcatString = "MADSCONCAT";

ResourceType MADSResourceManager::getResourceType(const char *resourceName) {
	if (!strncmp(resourceName, "RM", 2)) {
		// Room resource
		return RESTYPE_ROOM;
	} else if (!strncmp(resourceName, "SC", 2)) {
		// SC resource
		return RESTYPE_SC;
	} else if (strstr(resourceName, ".TXT")) {
		// Text resource
		return RESTYPE_TEXT;
	} else if (strstr(resourceName, ".QUO")) {
		// QUO resource
		return RESTYPE_QUO;
	} else if (*resourceName == 'I') {
		// I resource
		return RESTYPE_I;
	} else if (!strncmp(resourceName, "OB", 2)) {
		// OB resource
		return RESTYPE_OB;
	} else if (!strncmp(resourceName, "FONT", 4)) {
		// FONT resource
		return RESTYPE_FONT;
	} else if (!strncmp(resourceName, "SOUND", 5)) {
		// SOUND resource
		return RESTYPE_SOUND;
	} else if (!strncmp(resourceName, "SPCHC", 5)) {
		// SPEECH resource
		return RESTYPE_SPEECH;
	}

	// Check for a known extension
	const char *extPos = strchr(resourceName, '.');
	if (extPos) {
		++extPos;
		if (!strcmp(extPos, "FL") || !strcmp(extPos, "LBM") || !strcmp(extPos, "ANM") ||
			!strcmp(extPos, "AA") || !strcmp(extPos, "SS")) {
			return RESTYPE_HAS_EXT;
		}
	}

	return RESTYPE_NO_EXT;
}

const char *MADSResourceManager::getResourceFilename(const char *resourceName) {
	static char outputFilename[64];

	ResourceType resType = getResourceType(resourceName);

	strcpy(outputFilename, "GLOBAL.HAG");

	if ((resType == RESTYPE_ROOM) || (resType == RESTYPE_SC)) {
		int value = atoi(resourceName + 2);
		int hagFileNum = (resType == RESTYPE_ROOM) ? value / 100 : value;

		if (hagFileNum > 0)
			sprintf(outputFilename, "SECTION%d.HAG", hagFileNum);
	}

	if (resType == RESTYPE_SPEECH)
		strcpy(outputFilename, "SPEECH.HAG");

	return outputFilename;
}

Common::SeekableReadStream *MADSResourceManager::loadResource(const char *resourceName, bool loadFlag) {
	Common::File hagFile;
	uint32 offset = 0, size = 0;

	// If the first character is a '@' then look for an external file

	if (*resourceName == '@') {
		++resourceName;

		hagFile.open(resourceName);
		if (loadFlag)
			return hagFile.readStream(hagFile.size());
		else
			return new Common::SeekableSubReadStream(&hagFile, 0, hagFile.size());
	}

	// If the first character is the wildcard (resource indicator), skip over it
	if (*resourceName == '*')
		++resourceName;

	char resName[20];
	strcpy(resName, resourceName);
	str_upper(resName);

	hagFile.open(getResourceFilename(resName));

	// Validate hag file header
	char headerBuffer[16];
	if ((hagFile.read(headerBuffer, 16) != 16) || (strncmp(headerBuffer, madsConcatString, 10) != 0))
		error("Invalid HAG file opened");

	int numEntries = hagFile.readUint16LE();

	int resIndex = -1;
	while (++resIndex < numEntries) {
		// Read in the details of the next resource
		char resourceBuffer[14];
		offset = hagFile.readUint32LE();
		size = hagFile.readUint32LE();
		hagFile.read(resourceBuffer, 14);

		if (!strcmp(resName, resourceBuffer))
			break;
	}

	if (resIndex == numEntries)
		error("Invalid resource '%s' specified", resourceName);

	// Get the resource, either loading it in it's entirely or getting a stream reference

	if (loadFlag) {
		hagFile.seek(offset);
		return hagFile.readStream(size);
	} else {
		return new Common::SeekableSubReadStream(&hagFile, offset, offset + size);
	}
}

bool MADSResourceManager::resourceExists(const char *resourceName) {
	Common::File hagFile;
	uint32 offset, size;

	// If the first character is the wildcard (resource indicator), skip over it
	if (*resourceName == '*')
		++resourceName;

	char resName[20];
	strcpy(resName, resourceName);
	str_upper(resName);

	hagFile.open(getResourceFilename(resName));

	// Validate hag file header
	char headerBuffer[16];
	if ((hagFile.read(headerBuffer, 16) != 16) || (strncmp(headerBuffer, madsConcatString, 10) != 0))
		error("Invalid HAG file opened");

	int numEntries = hagFile.readUint16LE();

	int resIndex = -1;
	while (++resIndex < numEntries) {
		// Read in the details of the next resource
		char resourceBuffer[14];
		offset = hagFile.readUint32LE();
		size = hagFile.readUint32LE();
		hagFile.read(resourceBuffer, 14);

		if (!strcmp(resName, resourceBuffer))
			break;
	}

	if (resIndex == numEntries)
		return false;
	else
		return true;
}

//--------------------------------------------------------------------------

M4ResourceManager::M4ResourceManager(M4Engine *vm): ResourceManager(vm) {
	_hfs = new FileSystem(_vm->getGameFile(kFileTypeHash));
}

M4ResourceManager::~M4ResourceManager() {
}

Common::SeekableReadStream *M4ResourceManager::loadResource(const char *resourceName, bool preloadFlag) {
	//printf("M4ResourceManager::loadResource() loading resource %s\n", resourceName);
	Common::SeekableReadStream* result = NULL;
	if (_hfs) {
		// actually load the resource
		result = _hfs->loadFile(resourceName, preloadFlag);
		if (!result) {
			error("M4ResourceManager::loadResource() Resource %s not found", resourceName);
		}
	} else {
		error("M4ResourceManager::loadResource() No FileSystem attached");
	}
	return result;
}

bool M4ResourceManager::resourceExists(const char *resourceName) {
	return (_hfs->getHashFileEntry(resourceName) != NULL);
}

} // End of namespace M4
