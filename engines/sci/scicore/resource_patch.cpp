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

#include "common/archive.h"
#include "common/file.h"
#include "common/debug.h"

#include "sci/scicore/resource.h"
#include "sci/sci_memory.h"

namespace Sci {

// version-agnostic patch application
void ResourceManager::processPatch(ResourceSource *source,
	const char *filename, ResourceType restype, int resnumber) {
	Common::File file;
	Resource *newrsc;
	uint32 resId = RESOURCE_HASH(restype, resnumber);
	byte patchtype, patch_data_offset;
	int fsize;

	if (resnumber == -1)
		return;
	if (!file.open(filename)) {
		perror("""__FILE__"": (""__LINE__""): failed to open");
		return;
	}
	fsize = file.size();
	if (fsize < 3) {
		debug("Patching %s failed - file too small", filename);
		return;
	}
	
	patchtype = file.readByte() & 0x7F;
	patch_data_offset = file.readByte();

	if (patchtype != restype) {
		debug("Patching %s failed - resource type mismatch", filename);
		return;
	}
	if (patch_data_offset + 2 >= fsize) {
		debug("Patching %s failed - patch starting at offset %d can't be in file of size %d",
			filename, patch_data_offset + 2, fsize);
		return;
	}
	// Prepare destination, if neccessary
	if (_resMap.contains(resId) == false) {
		newrsc = new Resource;
		_resMap.setVal(resId, newrsc);
	} else 
		newrsc = _resMap.getVal(resId);
	// Overwrite everything, because we're patching
	newrsc->id = resId;
	newrsc->number = resnumber;
	newrsc->status = SCI_STATUS_NOMALLOC;
	newrsc->type = restype;
	newrsc->source = source;
	newrsc->size = fsize - patch_data_offset - 2;
	newrsc->file_offset = 2 + patch_data_offset;
	debug("Patching %s - OK", filename);
}


void ResourceManager::readResourcePatches(ResourceSource *source) {
// Note: since some SCI1 games(KQ5 floppy, SQ4) might use SCI0 naming scheme for patch files
// this function tries to read patch file with any supported naming scheme,
// regardless of _sciVersion value

	Common::String mask, name;
	Common::ArchiveMemberList files;
	int number;
	const char *szResType;
	ResourceSource *psrcPatch;

	for (int i = kResourceTypeView; i < kResourceTypeInvalid; i ++) {
		files.clear();
		szResType = getResourceTypeName((ResourceType)i);
		// SCI0 naming - type.nnn
		mask = szResType;
		mask += ".???";
		SearchMan.listMatchingMembers(files, mask);
		// SCI1 and later naming - nnn.typ
		mask = "*.";
		mask += getResourceTypeSuffix((ResourceType)i);
		SearchMan.listMatchingMembers(files, mask);
		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); x++) {
			number = -1;
			name = (*x)->getName();
			if (isdigit(name[0])) {
				// SCI1 scheme
				number = atoi(name.c_str());
			} else {
				// SCI0 scheme
				int resname_len = strlen(szResType);
				if (scumm_strnicmp(name.c_str(), szResType, resname_len) == 0) {
					number = atoi(name.c_str() + resname_len + 1);
				}
			}
			psrcPatch = new ResourceSource;
			psrcPatch->source_type = kSourcePatch;
			psrcPatch->location_name = name;
			processPatch(psrcPatch, name.c_str(), (ResourceType)i, number);
		}
	}
}

} // End of namespace Sci
