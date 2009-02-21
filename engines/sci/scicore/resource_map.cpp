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

#include "sci/include/sci_memory.h"
#include "sci/include/sciresource.h"
#include "sci/include/resource.h"

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

static int detect_odd_sci01(Common::File &file) {
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
			Common::File temp;
			
			// FIXME: Maybe better to use File::exists here?
			if (!temp.open(filename)) {
				files_ok = 0;
				break;
			}
		}
	}

	file.seek(0, SEEK_SET);

	return files_ok;
}

static int sci_res_read_entry(ResourceManager *mgr, ResourceSource *map,
                   byte *buf, resource_t *res, int sci_version) {
	res->id = buf[0] | (buf[1] << 8);
	res->type = SCI0_RESID_GET_TYPE(buf);
	res->number = SCI0_RESID_GET_NUMBER(buf);
	res->status = SCI_STATUS_NOMALLOC;

	if (sci_version == SCI_VERSION_01_VGA_ODD) {
		res->source = scir_get_volume(mgr, map, SCI01V_RESFILE_GET_FILE(buf + 2));
		res->file_offset = SCI01V_RESFILE_GET_OFFSET(buf + 2);

#if 0
		if (res->type < 0 || res->type > sci1_last_resource)
			return 1;
#endif
	} else {
		res->source = scir_get_volume(mgr, map, SCI0_RESFILE_GET_FILE(buf + 2));
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

inline int sci1_res_type(int ofs, int *types, int lastrt) {
	int i, last = -1;

	for (i = 0; i <= sci1_last_resource;i++)
		if (types[i]) {
			if (types[i] > ofs)
				return last;
			last = i;
		}

	return lastrt;
}

int sci1_parse_header(Common::ReadStream &stream, int *types, int *lastrt) {
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
			*lastrt = rtype & 0x7f;
		}
		size += 3;
	} while (read_ok && (rtype != 0xFF));

	if (!read_ok)
		return 0;

	return size;
}



int sci0_read_resource_map(ResourceManager *mgr, ResourceSource *map, resource_t **resource_p, int *resource_nr_p, int *sci_version) {
	int fsize;
	Common::File file;
	resource_t *resources;
	int resource_nr;
	int resource_index = 0;
	int resources_total_read = 0;
	int next_entry;
	int max_resfile_nr = 0;

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

	switch (detect_odd_sci01(file)) {
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

	resource_nr = fsize / SCI0_RESMAP_ENTRIES_SIZE;

	resources = (resource_t *)sci_calloc(resource_nr, sizeof(resource_t));
	// Sets valid default values for most entries

	do {
		int read_ok = file.read(&buf, SCI0_RESMAP_ENTRIES_SIZE);
		next_entry = 1;

		if (read_ok < 0) {
			sciprintf("Error while reading %s: ", map->location_name.c_str());
			perror("");
			next_entry = 0;
		} else if (read_ok != SCI0_RESMAP_ENTRIES_SIZE) {
			next_entry = 0;
		} else if (buf[5] == 0xff) // Most significant offset byte
			next_entry = 0;

		if (next_entry) {
			int fresh = 1;
			int addto = resource_index;
			int i;

			if (sci_res_read_entry(mgr, map, buf, resources + resource_index, *sci_version)) {
				free(resources);
				return SCI_ERROR_RESMAP_NOT_FOUND;
			}

			for (i = 0; i < resource_index; i++)
				if (resources[resource_index].id == resources[i].id) {
					addto = i;
					fresh = 0;
				}

			_scir_add_altsource(resources + addto, resources[resource_index].source, resources[resource_index].file_offset);

			if (fresh)
				++resource_index;

			if (++resources_total_read >= resource_nr) {
				sciprintf("Warning: After %d entries, resource.map is not terminated", resource_index);
				next_entry = 0;
			}

		}

	} while (next_entry);

	file.close();

	if (!resource_index) {
		sciprintf("resource.map was empty!\n");
		_scir_free_resources(resources, resource_nr);
		return SCI_ERROR_RESMAP_NOT_FOUND;
	}

	if (max_resfile_nr > 999) {
		_scir_free_resources(resources, resource_nr);
		return SCI_ERROR_INVALID_RESMAP_ENTRY;
	} else {
#if 0
		// Check disabled, Mac SQ3 thinks it has resource.004 but doesn't need it -- CR
		// Check whether the highest resfile used exists
		char filename_buf[14];
		sprintf(filename_buf, "resource.%03d", max_resfile_nr);

		if (!file.open(filename_buf) {
			_scir_free_resources(resources, resource_nr);
			sciprintf("'%s' requested by resource.map, but not found\n", filename_buf);
			return SCI_ERROR_INVALID_RESMAP_ENTRY;
		}
#endif
	}

	if (resource_index < resource_nr)
		resources = (resource_t *)sci_realloc(resources, sizeof(resource_t) * resource_index);

	*resource_p = resources;
	*resource_nr_p = resource_index;

	return 0;
}

#define TEST fprintf(stderr, "OK in line %d\n", __LINE__);

static int sci10_or_11(int *types) {
	int this_restype = 0;
	int next_restype = 1;

	while (next_restype <= sci_heap) {
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

int sci1_read_resource_map(ResourceManager *mgr, ResourceSource *map, ResourceSource *vol,
                       resource_t **resource_p, int *resource_nr_p, int *sci_version) {
	int fsize;
	Common::File file;
	resource_t *resources, *resource_start;
	int resource_nr;
	int resource_index = 0;
	int ofs, header_size;
	int *types = (int *)sci_malloc(sizeof(int) * (sci1_last_resource + 1));
	int i;
	byte buf[SCI1_RESMAP_ENTRIES_SIZE];
	int lastrt;
	int entrysize;
	int entry_size_selector;

	if (!file.open(map->location_name))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	memset(types, 0, sizeof(int) * (sci1_last_resource + 1));

	if (!(sci1_parse_header(file, types, &lastrt))) {
		return SCI_ERROR_INVALID_RESMAP_ENTRY;
	}

	entry_size_selector = sci10_or_11(types);
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
	resource_start = resources = (resource_t*)sci_realloc(mgr->_resources, (mgr->_resourcesNr + resource_nr) * sizeof(resource_t));
	resources += mgr->_resourcesNr;

	i = 0;
	while (types[i] == 0)
		i++;
	header_size = ofs = types[i];

	file.seek(ofs, SEEK_SET);

	for (i = 0; i < resource_nr; i++) {
		int read_ok = file.read(&buf, entrysize);
		int j;
		resource_t *res;
		int addto = resource_index;
		int fresh = 1;

		if (read_ok < entrysize) {
#if 0
			if (!file.eof()) {
				sciprintf("Error while reading %s: ", map->location_name.c_str());
				perror("");
			} else
				read_ok = 1;
			break;
#endif
		}

		res = &(resources[resource_index]);
		res->type = sci1_res_type(ofs, types, lastrt);
		res->number = SCI1_RESFILE_GET_NUMBER(buf);
		res->status = SCI_STATUS_NOMALLOC;

		if (entry_size_selector < SCI_VERSION_1_1) {
			res->source = scir_get_volume(mgr, map, SCI1_RESFILE_GET_FILE(buf));
			res->file_offset = SCI1_RESFILE_GET_OFFSET(buf);
		} else {
			res->source = vol;
			res->file_offset = SCI11_RESFILE_GET_OFFSET(buf);
		};

		res->id = res->number | (res->type << 16);

		for (j = 0; i < resource_index; i++)
			if (resources[resource_index].id == resources[i].id) {
				addto = i;
				fresh = 0;
			}

#if 0
		fprintf(stderr, "Read [%04x] %6d.%s\tresource.%03d, %08x ==> %d\n",
		        res->id, res->number,
		        sci_resource_type_suffixes[res->type],
		        res->file, res->file_offset, addto);
#endif

		_scir_add_altsource(resources + addto, resources[resource_index].source, resources[resource_index].file_offset);

		if (fresh)
			++resource_index;

		ofs += entrysize;
	}

	free(types);

	*resource_p = resource_start;
	*resource_nr_p += resource_index;
	return 0;

}

#ifdef TEST_RESOURCE_MAP
int main(int argc, char **argv) {
	int resource_nr;
	resource_t *resources;
	int notok = sci0_read_resource_map(".", &resources, &resource_nr);

	if (notok) {
		fprintf(stderr, "Failed: Error code %d\n", notok);
		return 1;
	}

	if (resources) {
		int i;

		printf("Found %d resources:\n", resource_nr);

		for (i = 0; i < resource_nr; i++) {
			resource_t *res = resources + i;

			printf("#%04d:\tRESOURCE.%03d:%8d\t%s.%03d\n", i, res->file, res->file_offset,
					sci_resource_types[res->type], res->number);
		}
	} else
		fprintf(stderr, "Found no resources.\n");

	return 0;
}
#endif

} // End of namespace Sci
