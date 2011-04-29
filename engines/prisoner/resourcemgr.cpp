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
#include "common/zlib.h"

#include "prisoner/resourcemgr.h"

namespace Prisoner {

/* PrisonerResourceLoader */

PrisonerResourceLoader::PrisonerResourceLoader() {

	/* TODO: Later the vga and sound pak directories should
		be loaded from the exe or some other external file.
	*/

	_vgaArchive = new KroArchive();
	_vgaArchive->open("KSVGA.KRO");
	_vgaPakDirectory = new PakDirectory();
	_vgaPakDirectory->load("KSVGA.0", 0, false);

	_soundArchive = new KroArchive();
	_soundArchive->open("KSOUND.KRO");
	_soundPakDirectory = new PakDirectory();
	_soundPakDirectory->load("KSOUND.0", 0, false);

	_langArchive = new KroArchive();
	_langArchive->open("E_KLANG.KRO");
	_langPakDirectory = new PakDirectory();
	_langPakDirectory->load("E_KLANG.0", 0, false);

}

PrisonerResourceLoader::~PrisonerResourceLoader() {

	delete _vgaArchive;
	delete _vgaPakDirectory;

	delete _soundArchive;
	delete _soundPakDirectory;

	delete _langArchive;
	delete _langPakDirectory;

}

byte *PrisonerResourceLoader::load(Common::String &pakName, int16 pakSlot, int16 type, uint32 &dataSize) {

	byte *data = NULL;
	KroArchive *archive = NULL;
	PakDirectory *pakDirectory = NULL;
	uint32 baseIndex = 0;

	switch (type) {
	case 0:
	case 1:
	case 2:
	case 4:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 14:
	case 17:
		archive = _vgaArchive;
		pakDirectory = _vgaPakDirectory;
		break;
	case 12:
	case 13:
	case 18:
		archive = _soundArchive;
		pakDirectory = _soundPakDirectory;
		break;
	case 3:
	case 16:
	case 19:
		archive = _langArchive;
		pakDirectory = _langPakDirectory;
		break;
	default:
		error("ResourceManager::load(%s, %d, %d) Unknown resource type", pakName.c_str(), pakSlot, type);
	}

	baseIndex = pakDirectory->getBaseIndex(pakName);
	data = archive->load(baseIndex + pakSlot);
	dataSize = archive->getSize(baseIndex + pakSlot);

	return data;

}

/* ResourceManager */

ResourceManager::ResourceManager(ResourceLoader *loader)
	: _loader(loader), _freeSlotsCount(kMaxResourceSlots) {
}

ResourceManager::~ResourceManager() {
	// TODO: Free all resources
}

void ResourceManager::purge() {
	debug(1, "ResourceManager::purge()");
	for (int16 slotIndex = 0; slotIndex < kMaxResourceSlots; slotIndex++) {
		if (_slots[slotIndex].resource && _slots[slotIndex].refCount == 0) {
			delete _slots[slotIndex].resource;
			_slots[slotIndex].resource = NULL;
			_slots[slotIndex].type = -1;
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
		purge();

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
	fd.open(filename);
	fd.write(data, dataSize);
	delete[] data;
}

} // End of namespace Prisoner
