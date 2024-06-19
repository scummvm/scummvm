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

#include "common/file.h"

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/stream.h"
#include "common/compression/deflate.h"

#include "prisoner/resourcemgr.h"

namespace Prisoner {

/* PrisonerResourceLoader */

PrisonerResourceLoader::PrisonerResourceLoader(char languageChar) {

	// NOTE: An alternative to hardcoding the data would be to load it from the game Exe
	// (The file-based loadDirectory supports this already.)

	// Same in DOS, Win95
	static const _PakDirectoryEntry kPrisonerKSVGADirectory[] = {
		{"SA02", 0},
		{"SA03", 179},
		{"SA06", 380},
		{"SA08", 459},
		{"SA09", 544},
		{"SA10", 609},
		{"SA11", 688},
		{"SA12", 724},
		{"SB02", 815},
		{"SB03", 882},
		{"SB06", 1001},
		{"SB08", 1039},
		{"SB09", 1057},
		{"SB10", 1076},
		{"SB11", 1099},
		{"SB12", 1108},
		{"SM02", 1138},
		{"SM03", 1180},
		{"SM06", 1234},
		{"SM08", 1251},
		{"SM09", 1262},
		{"SM10", 1271},
		{"SM11", 1286},
		{"SM12", 1292},
		{"SOBJECT", 1306},
		{"S_CURSOR", 1315},
		{"S_DEBUG", 1316},
		{"S_FONT", 1317},
		{"S_PANEL", 1320},
		{NULL, 0}
	};

	static const _PakDirectoryEntry kPrisonerDOSKSOUNDDirectory[] = {
		{"MUS02", 0},
		{"MUS03", 6},
		{"MUS06", 23},
		{"MUS08", 32},
		{"MUS09", 34},
		{"MUS10", 38},
		{"MUS11", 41},
		{"MUS12", 42},
		{"SMP02", 45},
		{"SMP03", 113},
		{"SMP06", 165},
		{"SMP08", 196},
		{"SMP09", 222},
		{"SMP10", 244},
		{"SMP11", 261},
		{"SMP12", 277},
		{"SOUNDTST", 305},
		{"WUS02", 308},
		{"WUS03", 314},
		{"WUS06", 331},
		{"WUS08", 340},
		{"WUS09", 342},
		{"WUS10", 346},
		{"WUS11", 349},
		{"WUS12", 350},
		{NULL, 0}
	};

	static const int16 kPrisonerKSVGAResourceTypes[] = {0, 1, 2, 4, 6, 7, 8, 9, 10, 11, 14, 17, -1};
	static const int16 kPrisonerKSOUNDResourceTypes[] = {12, 13, 18, -1};
	static const int16 kPrisonerKLANGResourceTypes[] = {3, 16, 19, -1};

	/*
	// Same as in the DOS version minus several PAKs
	// I'm not sure yet if the other PAKs are defined elsewhere or
	// completely absent here.
	static const _PakDirectoryEntry kPrisonerWinKSOUNDDirectory[] = {
		{"MUS02", 0},
		{"MUS03", 6},
		{"MUS06", 23},
		{"MUS08", 32},
		{"MUS09", 34},
		{"MUS10", 38},
		{"MUS11", 41},
		{"MUS12", 42},
		{"SMP02", 45},
		{"SMP03", 113},
		{"SMP06", 165},
		{"SMP08", 196},
		{"SMP09", 222},
		{NULL, 0}
	};
	*/

	addArchive("KSVGA.KRO", kPrisonerKSVGADirectory, kPrisonerKSVGAResourceTypes);
	addArchive("KSOUND.KRO", kPrisonerDOSKSOUNDDirectory, kPrisonerKSOUNDResourceTypes);

	Common::String kroName = Common::String::format("%c_KLANG.KRO", languageChar);
	Common::String binName = Common::String::format("%c_KLANG.BIN", languageChar);

	addArchive(kroName.c_str(), binName.c_str(), 0, true, kPrisonerKLANGResourceTypes);

}

PrisonerResourceLoader::~PrisonerResourceLoader() {
	for (Common::Array<KroArchive*>::iterator iter = _archives.begin(); iter != _archives.end(); iter++)
		delete *iter;
}

byte *PrisonerResourceLoader::load(Common::String &pakName, int16 pakSlot, int16 type, uint32 &dataSize) {
	KroArchive *archive = getArchiveForType(type);
	uint32 baseIndex = archive->getPakBaseIndex(pakName);
	byte *data = archive->load(baseIndex + pakSlot);
	dataSize = archive->getSize(baseIndex + pakSlot);
	return data;
}

KroArchive *PrisonerResourceLoader::getArchiveForType(int16 type) {
	for (Common::Array<KroArchive*>::iterator iter = _archives.begin(); iter != _archives.end(); iter++) {
		KroArchive *archive = *iter;
		if (archive->hasResourceType(type))
			return archive;
	}
	error("PrisonerResourceLoader::getArchiveForType() Unknown resource type %d", type);
	return NULL;
}

void PrisonerResourceLoader::addArchive(const char *filename, const _PakDirectoryEntry directory[], const int16 *resourceTypes) {
	KroArchive *archive = new KroArchive();
	archive->open(filename);
	archive->loadDirectory(directory);
	archive->setResourceTypes(resourceTypes);
	_archives.push_back(archive);
}

void PrisonerResourceLoader::addArchive(const char *filename, const char *directoryFilename, uint32 offset, bool isEncrypted, const int16 *resourceTypes) {
	KroArchive *archive = new KroArchive();
	archive->open(filename);
	archive->loadDirectory(directoryFilename, offset, isEncrypted);
	archive->setResourceTypes(resourceTypes);
	_archives.push_back(archive);
}

/* ResourceManager */

ResourceManager::ResourceManager(ResourceLoader *loader)
	: _loader(loader), _freeSlotsCount(kMaxResourceSlots) {
}

ResourceManager::~ResourceManager() {
	// Free all resources
	purge(true);
}

void ResourceManager::purge(bool all) {
	debug(1, "ResourceManager::purge(%d)", all);
	for (int16 slotIndex = 0; slotIndex < kMaxResourceSlots; slotIndex++) {
		if (_slots[slotIndex].resource && (all || _slots[slotIndex].refCount == 0)) {
			delete _slots[slotIndex].resource;
			_slots[slotIndex].resource = NULL;
			_slots[slotIndex].type = -1;
			_slots[slotIndex].refCount = 0;
			_freeSlotsCount++;
		}
	}
}

void ResourceManager::unload(int16 slotIndex) {

	if (slotIndex < 0 || slotIndex >= kMaxResourceSlots) {
		error("Invalid slotIndex %d", slotIndex);
	}

	ResourceSlot *slot = &_slots[slotIndex];
	if (slot->refCount > 0)
		slot->refCount--;
}

int16 ResourceManager::find(Common::String &pakName, int16 pakSlot, int16 type) {
	for (int16 i = 0; i < kMaxResourceSlots; i++) {
		ResourceSlot *slot = &_slots[i];
		if (slot->type != -1 && slot->pakSlot == pakSlot && slot->pakName == pakName)
			return i;
	}
	return -1;
}

int16 ResourceManager::add(Common::String &pakName, int16 pakSlot, int16 type) {

	int16 slotIndex;

	if (_freeSlotsCount == 0)
		purge(false);

	for (slotIndex = 0; slotIndex < kMaxResourceSlots; slotIndex++) {
		if (_slots[slotIndex].type == -1)
			break;
	}

	ResourceSlot *slot = &_slots[slotIndex];
	slot->type = type;
	slot->refCount = 1;
	slot->pakName = pakName;
	slot->pakSlot = pakSlot;

	_freeSlotsCount--;

	return slotIndex;
}

void ResourceManager::dump(Common::String &pakName, int16 pakSlot, int16 type) {
	Common::DumpFile fd;
	Common::String filename = Common::String::format("%s%d.0", pakName.c_str(), pakSlot);
	byte *data;
	uint32 dataSize;
	data = _loader->load(pakName, pakSlot, type, dataSize);
	fd.open(Common::Path(filename));
	fd.write(data, dataSize);
	delete[] data;
}

} // End of namespace Prisoner
