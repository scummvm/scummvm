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

#include "sci/scicore/resource.h"
#include "sci/sci_memory.h"

namespace Sci {

void sci0_sprintf_patch_file_name(char *string, Resource *res) {
	sprintf(string, "%s.%03i", getResourceTypeName(res->type), res->number);
}

void sci1_sprintf_patch_file_name(char *string, Resource *res) {
	sprintf(string, "%d.%s", res->number, getResourceTypeSuffix(res->type));
}

// version-agnostic patch application
void ResourceManager::process_patch(ResourceSource *source,
	Common::ArchiveMember &member, ResourceType restype, int resnumber) {
	Common::File file;

	if (restype == kResourceTypeInvalid)
		return;

	printf("Patching \"%s\": ", member.getName().c_str());
	if (!file.open(member.createReadStream(), member.getName()))
		perror("""__FILE__"": (""__LINE__""): failed to open");
	else {
		uint8 filehdr[2];
		Resource *newrsc = findResourceUnsorted(_resources, _resourcesNr, restype, resnumber);
		int fsize = file.size();
		if (fsize < 3) {
			printf("File too small\n");
			return;
		}

		int patch_data_offset;

		file.read(filehdr, 2);

		patch_data_offset = filehdr[1];

		if ((filehdr[0] & 0x7f) != restype) {
			printf("Failed; resource type mismatch\n");
		} else if (patch_data_offset + 2 >= fsize) {
			printf("Failed; patch starting at offset %d can't be in file of size %d\n", filehdr[1] + 2, fsize);
		} else {
			// Adjust for file offset
			fsize -= patch_data_offset;

			// Prepare destination, if neccessary
			if (!newrsc) {
				// Completely new resource!
				_resourcesNr++;
				_resources = (Resource *)sci_realloc(_resources, _resourcesNr * sizeof(Resource));
				newrsc = (_resources - 1) + _resourcesNr;
				newrsc->alt_sources = NULL;
			}

			// Overwrite everything, because we're patching
			newrsc->size = fsize - 2;
			newrsc->id = restype << 11 | resnumber;
			newrsc->number = resnumber;
			newrsc->status = SCI_STATUS_NOMALLOC;
			newrsc->type = restype;
			newrsc->source = source;
			newrsc->file_offset = 2 + patch_data_offset;
			addAltSource(newrsc, source, 2);
			printf("OK\n");
		}
	}
}


int ResourceManager::readResourcePatchesSCI0(ResourceSource *source) {
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.???");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		ResourceType restype = kResourceTypeInvalid;
		int resnumber = -1;
		unsigned int resname_len;
		char *endptr;

		for (int i = kResourceTypeView; i < kResourceTypeInvalid; i++)
			if (scumm_strnicmp(getResourceTypeName((ResourceType)i), name.c_str(), strlen(getResourceTypeName((ResourceType)i))) == 0)
				restype = (ResourceType)i;

		if (restype != kResourceTypeInvalid) {
			resname_len = strlen(getResourceTypeName(restype));
			if (name[resname_len] != '.')
				restype = kResourceTypeInvalid;
			else {
				resnumber = strtol(name.c_str() + 1 + resname_len, &endptr, 10); // Get resource number
				if ((*endptr != '\0') || (resname_len + 1 == name.size()))
					restype = kResourceTypeInvalid;

				if ((resnumber < 0) || (resnumber > 1000))
					restype = kResourceTypeInvalid;
			}
		}

		process_patch(source, **x, restype, resnumber);
	}

	return 0;
}

int ResourceManager::readResourcePatchesSCI1(ResourceSource *source) {
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.*");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		ResourceType restype = kResourceTypeInvalid;
		int resnumber = -1;
		char *endptr;
		const char *dot = strchr(name.c_str(), '.');

		for (int i = kResourceTypeView; i < kResourceTypeInvalid; i++) {
			if (dot != NULL) {
				if (scumm_strnicmp(getResourceTypeSuffix((ResourceType)i), dot + 1, 3) == 0) {
					restype = (ResourceType)i;
				}
			}
		}

		if (restype != kResourceTypeInvalid) {
			resnumber = strtol(name.c_str(), &endptr, 10); // Get resource number

			if (endptr != dot)
				restype = kResourceTypeInvalid;

			if (*(dot + 4) != '\0')
				restype = kResourceTypeInvalid;

			if ((resnumber < 0) || (resnumber > 8192))
				restype = kResourceTypeInvalid;
		}

		process_patch(source, **x, restype, resnumber);
	}

	return 0;
}

} // End of namespace Sci
