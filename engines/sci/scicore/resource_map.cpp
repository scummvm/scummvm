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

#include "sci/sci_memory.h"
#include "sci/scicore/resource.h"
#include "sci/tools.h"

#include "common/util.h"
#include "common/file.h"

namespace Sci {

#define RESOURCE_MAP_FILENAME "resource.map"

#define SCI0_RESMAP_ENTRIES_SIZE 6
#define SCI1_RESMAP_ENTRIES_SIZE 6
#define SCI11_RESMAP_ENTRIES_SIZE 5

// Resource type encoding
#define SCI0_B1_RESTYPE_MASK  0xf8
#define SCI0_B1_RESTYPE_SHIFT 3
#define SCI0_B3_RESFILE_MASK  0xfc
#define SCI0_B3_RESFILE_SHIFT 2
#define SCI01V_B3_RESFILE_MASK  0xf0
#define SCI01V_B3_RESFILE_SHIFT 4

#define SCI0_RESID_GET_TYPE(bytes) \
	(((bytes)[1] & SCI0_B1_RESTYPE_MASK) >> SCI0_B1_RESTYPE_SHIFT)
#define SCI0_RESID_GET_NUMBER(bytes) \
	((((bytes)[1] & ~SCI0_B1_RESTYPE_MASK) << 8) | ((bytes)[0]))

#define SCI0_RESFILE_GET_FILE(bytes) \
	(((bytes)[3] & SCI0_B3_RESFILE_MASK) >> SCI0_B3_RESFILE_SHIFT)
#define SCI0_RESFILE_GET_OFFSET(bytes) \
	((((bytes)[3] & ~SCI0_B3_RESFILE_MASK) << 24) \
		| (((bytes)[2]) << 16) \
		| (((bytes)[1]) << 8) \
		| (((bytes)[0]) << 0))

#define SCI01V_RESFILE_GET_FILE(bytes) \
	(((bytes)[3] & SCI01V_B3_RESFILE_MASK) >> SCI01V_B3_RESFILE_SHIFT)
#define SCI01V_RESFILE_GET_OFFSET(bytes) \
	((((bytes)[3] & ~SCI01V_B3_RESFILE_MASK) << 24) \
	| (((bytes)[2]) << 16) \
	| (((bytes)[1]) << 8) \
	| (((bytes)[0]) << 0))

#define SCI1_B5_RESFILE_MASK 0xf0
#define SCI1_B5_RESFILE_SHIFT 4

#define SCI1_RESFILE_GET_FILE(bytes) \
	(((bytes)[5] & SCI1_B5_RESFILE_MASK) >> SCI1_B5_RESFILE_SHIFT)

#define SCI1_RESFILE_GET_OFFSET(bytes) \
	((((bytes)[5] & ~SCI1_B5_RESFILE_MASK) << 24) \
		| (((bytes)[4]) << 16) \
		| (((bytes)[3]) << 8) \
		| (((bytes)[2]) << 0))

#define SCI1_RESFILE_GET_NUMBER(bytes) \
	((((bytes)[1]) << 8) \
		| (((bytes)[0]) << 0))

#define SCI11_RESFILE_GET_OFFSET(bytes) \
	((((bytes)[4]) << 17) \
		| (((bytes)[3]) << 9) \
		| (((bytes)[2]) << 1))

int ResourceManager::detectOddSCI01(Common::File &file) {
	byte buf[6];
	int files_ok = 1;
	int fsize, resource_nr, read_ok;
	char filename[14];

	fsize = file.size();
	if (fsize < 0) {
		perror("Error occured while trying to get filesize of resource.map");
		return SCI_ERROR_RESMAP_NOT_FOUND;
	}

	resource_nr = fsize / SCI0_RESMAP_ENTRIES_SIZE;

	while (resource_nr-- > 1) {
		read_ok = file.read(&buf, SCI0_RESMAP_ENTRIES_SIZE);

		if (read_ok) {
			sprintf(filename, "resource.%03i", SCI0_RESFILE_GET_FILE(buf + 2));
			if (!Common::File::exists(filename)) {
				files_ok = 0;
				break;
			}
		}
	}

	file.seek(0, SEEK_SET);

	return files_ok;
}

int ResourceManager::resReadEntry(ResourceSource *map, byte *buf, Resource *res, int sci_version) {
	res->id = READ_LE_UINT16(buf);//buf[0] | (buf[1] << 8);
	res->type = (ResourceType)SCI0_RESID_GET_TYPE(buf);
	res->number = SCI0_RESID_GET_NUMBER(buf);
	res->status = SCI_STATUS_NOMALLOC;

	if (sci_version == SCI_VERSION_01_VGA_ODD) {
		res->source = getVolume(map, SCI01V_RESFILE_GET_FILE(buf + 2));
		res->file_offset = SCI01V_RESFILE_GET_OFFSET(buf + 2);

#if 0
		if (res->type < 0 || res->type > sci1_last_resource)
			return 1;
#endif
	} else {
		res->source = getVolume(map, SCI0_RESFILE_GET_FILE(buf + 2));
		res->file_offset = SCI0_RESFILE_GET_OFFSET(buf + 2);

#if 0
		if (res->type < 0 || res->type > sci0_last_resource)
			return 1;
#endif
	}

#if 0
	fprintf(stderr, "Read [%04x] %6d.%s\tresource.%03d, %08x\n",
	        res->id, res->number,
	        sci_resource_type_suffixes[res->type],
	        res->file, res->file_offset);
#endif

	if (res->source == NULL)
		return 1;

	return 0;
}

ResourceType ResourceManager::resTypeSCI1(int ofs, int *types, ResourceType lastrt) {
	ResourceType last = kResourceTypeInvalid;

	for (int i = 0; i <= sci1_last_resource; i++)
		if (types[i]) {
			if (types[i] > ofs)
				return last;
			last = (ResourceType)i;
		}

	return lastrt;
}

int ResourceManager::parseHeaderSCI1(Common::ReadStream &stream, int *types, ResourceType *lastrt) {
	unsigned char rtype;
	unsigned char offset[2];
	int read_ok;
	int size = 0;

	do {
		read_ok = stream.read(&rtype, 1);
		if (!read_ok)
			break;
		read_ok = stream.read(&offset, 2);
		if (read_ok < 2)
			read_ok = 0;
		if (rtype != 0xff) {
			types[rtype&0x7f] = (offset[1] << 8) | (offset[0]);
			*lastrt = (ResourceType)(rtype & 0x7f);
		}
		size += 3;
	} while (read_ok && (rtype != 0xFF));

	if (!read_ok)
		return 0;

	return size;
}



int ResourceManager::readResourceMapSCI0(ResourceSource *map, int *sci_version) {
	int fsize;
	Common::File file;
	Resource *res, res1;
	int resources_total_read = 0;
	bool bAdded = false;
	byte buf[SCI0_RESMAP_ENTRIES_SIZE];

	if (!file.open(map->location_name))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	file.read(&buf, 4);

	/* Theory: An SCI1 map file begins with an index that allows us to seek quickly
	   to a particular resource type. The entries are three bytes long; one byte
	   resource type, two bytes start position and so on.
	   The below code therefore tests for three things:

	   Is the first resource type 'view'?
	   Do those entries start at an offset that is an exact multiple of the
	   index entry size?
	   Is the second resource type 'pic'?

	   This requires that a given game has both views and pics,
	   a safe assumption usually, except in message.map and room-specific
	   (audio) map files, neither of which SCI0 has.

	   */

	if ((buf[0] == 0x80) && (buf[1] % 3 == 0) && (buf[3] == 0x81)) {
		return SCI_ERROR_INVALID_RESMAP_ENTRY;
	}

	file.seek(0, SEEK_SET);

	switch (detectOddSCI01(file)) {
	case 0 : // Odd SCI01
		if (*sci_version == SCI_VERSION_AUTODETECT)
			*sci_version = SCI_VERSION_01_VGA_ODD;
		break;
	case 1 : // SCI0 or normal SCI01
		if (*sci_version == SCI_VERSION_AUTODETECT)
			*sci_version = SCI_VERSION_0;
		break;
	default : // Neither, or error occurred
		return SCI_ERROR_RESMAP_NOT_FOUND;
	}

	fsize = file.size();
	if (fsize < 0) {
		perror("Error occured while trying to get filesize of resource.map");
		return SCI_ERROR_RESMAP_NOT_FOUND;
	}

	int resource_nr = fsize / SCI0_RESMAP_ENTRIES_SIZE;

	do {
		int read_ok = file.read(&buf, SCI0_RESMAP_ENTRIES_SIZE);

		if (read_ok != SCI0_RESMAP_ENTRIES_SIZE) {
			sciprintf("Error while reading %s: ", map->location_name.c_str());
			perror("");
			break;
		}
		if(buf[5] == 0xff)
			break;

		if (resReadEntry(map, buf, &res1, *sci_version))
			return SCI_ERROR_RESMAP_NOT_FOUND;
		uint32 resId = RESOURCE_HASH(res1.type, res1.number);
		// adding a new resource
		if (_resMap.contains(resId) == false) {
			res = new Resource;
			res->id = res1.id;
			res->file_offset = res1.file_offset;
			res->number = res1.number;
			res->type = res1.type;
			res->source = res1.source;
			_resMap.setVal(resId, res);
			bAdded = true;
		}

		if (++resources_total_read >= resource_nr) {
			warning("After %d entries, resource.map is not terminated", resources_total_read);
			break;
		}
	} while (!file.eos());
	if (!bAdded)
		return SCI_ERROR_RESMAP_NOT_FOUND;

	file.close();
	return 0;
}

#define TEST fprintf(stderr, "OK in line %d\n", __LINE__);

int ResourceManager::isSCI10or11(int *types) {
	int this_restype = 0;
	int next_restype = 1;

	while (next_restype <= kResourceTypeHeap) {
		int could_be_10 = 0;
		int could_be_11 = 0;

		while (types[this_restype] == 0) {
			this_restype++;
			next_restype++;
		}

		while (types[next_restype] == 0)
			next_restype++;

		could_be_10 = ((types[next_restype] - types[this_restype]) % SCI1_RESMAP_ENTRIES_SIZE) == 0;
		could_be_11 = ((types[next_restype] - types[this_restype]) % SCI11_RESMAP_ENTRIES_SIZE) == 0;

		if (could_be_10 && !could_be_11)
			return SCI_VERSION_1;
		if (could_be_11 && !could_be_10)
			return SCI_VERSION_1_1;

		this_restype++;
		next_restype++;
	}

	return SCI_VERSION_AUTODETECT;
}

int ResourceManager::readResourceMapSCI1(ResourceSource *map, ResourceSource *vol, int *sci_version) {
	int fsize;
	Common::File file;
	int resource_nr;
	int ofs, header_size;
	int *types = (int *)sci_malloc(sizeof(int) * (sci1_last_resource + 1));
	int i;
	byte buf[SCI1_RESMAP_ENTRIES_SIZE];
	ResourceType lastrt;
	int entrysize;
	int entry_size_selector;

	if (!file.open(map->location_name))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	memset(types, 0, sizeof(int) * (sci1_last_resource + 1));

	if (!(parseHeaderSCI1(file, types, &lastrt))) {
		return SCI_ERROR_INVALID_RESMAP_ENTRY;
	}

	entry_size_selector = isSCI10or11(types);
	if (*sci_version == SCI_VERSION_AUTODETECT)
		*sci_version = entry_size_selector;

	if (*sci_version == SCI_VERSION_AUTODETECT) { // That didn't help
		sciprintf("Unable to detect resource map version\n");
		return SCI_ERROR_NO_RESOURCE_FILES_FOUND;
	}

	entrysize = entry_size_selector == SCI_VERSION_1_1 ? SCI11_RESMAP_ENTRIES_SIZE : SCI1_RESMAP_ENTRIES_SIZE;

	fsize = file.size();
	if (fsize < 0) {
		perror("Error occured while trying to get filesize of resource.map");
		return SCI_ERROR_RESMAP_NOT_FOUND;
	}

	resource_nr = (fsize - types[0]) / entrysize;
	i = 0;
	while (types[i] == 0)
		i++;
	header_size = ofs = types[i];

	file.seek(ofs, SEEK_SET);

	for (i = 0; i < resource_nr; i++) {
		Resource *res;
		int number;
		ResourceType type;
		uint32 resId;

		file.read(&buf, entrysize);
		type = resTypeSCI1(ofs, types, lastrt);
		number = SCI1_RESFILE_GET_NUMBER(buf);
		resId = RESOURCE_HASH(type, number);
		// adding new resource only if it does not exist
		if (_resMap.contains(resId) == false) {
			res = new Resource;
			_resMap.setVal(resId, res);
			res->type = type;
			res->number = number;
			res->id = res->number | (res->type << 16);
		// only 1st source would be used when loading resource
			if (entry_size_selector < SCI_VERSION_1_1) {
				res->source = getVolume(map, SCI1_RESFILE_GET_FILE(buf));
				res->file_offset = SCI1_RESFILE_GET_OFFSET(buf);
			} else {
				res->source = vol;
				res->file_offset = SCI11_RESFILE_GET_OFFSET(buf);
			};
		}

		ofs += entrysize;
	}

	free(types);

	return 0;
}

} // End of namespace Sci
