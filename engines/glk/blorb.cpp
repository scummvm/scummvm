/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/blorb.h"

namespace Glk {

#define giblorb_Inited_Magic 0xB7012BEDU

/**
 * Describes one chunk of the Blorb file.
 */
struct giblorb_chunkdesc_struct {
    glui32 type;
    glui32 len;
    glui32 startpos;	///< start of chunk header
    glui32 datpos;		///< start of data (either startpos or startpos+8)

    byte *ptr;		///< pointer to malloc'd data, if loaded
    int auxdatnum;	///< entry in the auxsound/auxpict array; -1 if none. This only applies to chunks that represent resources;
};
typedef giblorb_chunkdesc_struct giblorb_chunkdesc_t;

/**
 * Describes one resource in the Blorb file.
 */
struct giblorb_resdesc_struct {
	glui32 usage;
	glui32 resnum;
	glui32 chunknum;
};

/**
 * Holds the complete description of an open Blorb file. 
 */
struct giblorb_map_struct {
    glui32 inited; ///< holds giblorb_Inited_Magic if the map structure is valid
	Common::SeekableReadStream *file;

    uint numchunks;
    giblorb_chunkdesc_t *chunks;	///< list of chunk descriptors

    int numresources;
    giblorb_resdesc_t *resources;	///< list of resource descriptors
    giblorb_resdesc_t **ressorted;	///< list of pointers to descriptors in map->resources -- sorted by usage and resource number.
};

/*--------------------------------------------------------------------------*/

giblorb_err_t Blorb::giblorb_initialize() {
	_file = nullptr;
	_map = nullptr;
	return giblorb_err_None;
}

giblorb_err_t Blorb::giblorb_create_map(Common::SeekableReadStream *file, giblorb_map_t **newmap) {
	giblorb_err_t err;
	giblorb_map_t *map;
	glui32 readlen;
	glui32 nextpos, totallength;
	giblorb_chunkdesc_t *chunks;
	int chunks_size, numchunks;
	char buffer[16];

	*newmap = nullptr;

	if (!_libInited) {
		err = giblorb_initialize();
		if (err)
			return err;
		_libInited = true;
	}

	/* First, chew through the file and index the chunks. */
	file->seek(0);

	readlen = file->read(buffer, 12);
	if (readlen != 12)
		return giblorb_err_Read;

	if (READ_BE_INT32(buffer + 0) != giblorb_ID_FORM)
		return giblorb_err_Format;
	if (READ_BE_INT32(buffer + 8) != giblorb_ID_IFRS)
		return giblorb_err_Format;

	totallength = READ_BE_INT32(buffer + 4) + 8;
	nextpos = 12;

	chunks_size = 8;
	numchunks = 0;
	chunks = new giblorb_chunkdesc_t[chunks_size];

	while (nextpos < totallength) {
		glui32 type, len;
		int chunum;
		giblorb_chunkdesc_t *chu;

		file->seek(nextpos);

		readlen = file->read(buffer, 8);
		if (readlen != 8) {
			delete[] chunks;
			return giblorb_err_Read;
		}

		type = READ_BE_INT32(buffer + 0);
		len = READ_BE_INT32(buffer + 4);

		if (numchunks >= chunks_size) {
			chunks_size *= 2;
			chunks = new giblorb_chunkdesc_t[chunks_size];
		}

		chunum = numchunks;
		chu = &(chunks[chunum]);
		numchunks++;

		chu->type = type;
		chu->startpos = nextpos;
		if (type == giblorb_ID_FORM) {
			chu->datpos = nextpos;
			chu->len = len + 8;
		} else {
			chu->datpos = nextpos + 8;
			chu->len = len;
		}
		chu->ptr = nullptr;
		chu->auxdatnum = -1;

		nextpos = nextpos + len + 8;
		if (nextpos & 1)
			nextpos++;

		if (nextpos > totallength) {
			delete[] chunks;
			return giblorb_err_Format;
		}
	}

	// The basic IFF structure seems to be ok, and we have a list of chunks.
	// Now we allocate the map structure itself.
	map = new giblorb_map_t();
	if (!map) {
		delete[] chunks;
		return giblorb_err_Alloc;
	}

	map->inited = giblorb_Inited_Magic;
	map->file = file;
	map->chunks = chunks;
	map->numchunks = numchunks;
	map->resources = nullptr;
	map->ressorted = nullptr;
	map->numresources = 0;

	// Now we do everything else involved in loading the Blorb file,
	// such as building resource lists.
	err = giblorb_initialize_map(map);
	if (err) {
		giblorb_destroy_map(map);
		return err;
	}

	*newmap = map;
	return giblorb_err_None;
}


giblorb_err_t Blorb::giblorb_initialize_map(giblorb_map_t *map) {
	// It is important that the map structure be kept valid during this function.
	// If this returns an error, giblorb_destroy_map() will be called.
	uint ix, jx;
	giblorb_result_t chunkres;
	giblorb_err_t err;
	char *ptr;
	glui32 len;
	glui32 numres;
	int gotindex = false;

	for (ix = 0; ix<map->numchunks; ix++) {
		giblorb_chunkdesc_t *chu = &map->chunks[ix];

		switch (chu->type) {
		case giblorb_ID_RIdx:
			// Resource index chunk: build the resource list and sort it.

			if (gotindex)
				return giblorb_err_Format; // duplicate index chunk
			err = giblorb_load_chunk_by_number(map, giblorb_method_Memory, &chunkres, ix);
			if (err)
				return err;

			ptr = (char *)chunkres.data.ptr;
			len = chunkres.length;
			numres = READ_BE_INT32(ptr + 0);

			if (numres) {
				uint ix2;
				giblorb_resdesc_t *resources;
				giblorb_resdesc_t **ressorted;

				if (len != numres * 12 + 4)
					return giblorb_err_Format; // bad length field

				resources = new giblorb_resdesc_t[numres];
				ressorted = new giblorb_resdesc_t *[numres];
				if (!ressorted || !resources) {
					delete[] resources;
					delete[] ressorted;
					return giblorb_err_Alloc;
				}

				ix2 = 0;
				for (jx = 0; jx < numres; jx++) {
					giblorb_resdesc_t *res = &(resources[jx]);
					glui32 respos;

					res->usage = READ_BE_INT32(ptr + jx * 12 + 4);
					res->resnum = READ_BE_INT32(ptr + jx * 12 + 8);
					respos = READ_BE_INT32(ptr + jx * 12 + 12);

					while (ix2 < map->numchunks
						&& map->chunks[ix2].startpos < respos)
						ix2++;

					if (ix2 >= map->numchunks
						|| map->chunks[ix2].startpos != respos) {
						delete[] resources;
						delete[] ressorted;
						return giblorb_err_Format; // start pos does not match a real chunk
					}

					res->chunknum = ix2;

					ressorted[jx] = res;
				}

				// Sort a resource list (actually a list of pointers to structures in map->resources.)
				// This makes it easy to find resources by usage and resource number.
				giblorb_qsort(ressorted, numres);

				map->numresources = numres;
				map->resources = resources;
				map->ressorted = ressorted;
			}

			giblorb_unload_chunk(map, ix);
			gotindex = true;
			break;
		}
	}

	return giblorb_err_None;
}

void Blorb::giblorb_qsort(giblorb_resdesc_t **list, size_t len) {
	int ix, jx, res;
	giblorb_resdesc_t *tmpptr, *pivot;

	if (len < 6) {
		// The list is short enough for a bubble-sort.
		for (jx = len - 1; jx>0; jx--) {
			for (ix = 0; ix<jx; ix++) {
				res = sortsplot(list[ix], list[ix + 1]);
				if (res > 0) {
					tmpptr = list[ix];
					list[ix] = list[ix + 1];
					list[ix + 1] = tmpptr;
				}
			}
		}
	} else {
		// Split the list.
		pivot = list[len / 2];
		ix = 0;
		jx = len;
		for (;;) {
			while (ix < jx - 1 && sortsplot(list[ix], pivot) < 0)
				ix++;
			while (ix < jx - 1 && sortsplot(list[jx - 1], pivot) > 0)
				jx--;
			if (ix >= jx - 1)
				break;
			tmpptr = list[ix];
			list[ix] = list[jx - 1];
			list[jx - 1] = tmpptr;
		}
		ix++;
		// Sort the halves.
		giblorb_qsort(list + 0, ix);
		giblorb_qsort(list + ix, len - ix);
	}
}

giblorb_resdesc_t *Blorb::giblorb_bsearch(giblorb_resdesc_t *sample,
	giblorb_resdesc_t **list, int len) {
	int top, bot, val, res;

	bot = 0;
	top = len;

	while (bot < top) {
		val = (top + bot) / 2;
		res = sortsplot(list[val], sample);
		if (res == 0)
			return list[val];
		if (res < 0) {
			bot = val + 1;
		} else {
			top = val;
		}
	}

	return nullptr;
}

int Blorb::sortsplot(giblorb_resdesc_t *v1, giblorb_resdesc_t *v2) {
	if (v1->usage < v2->usage)
		return -1;
	if (v1->usage > v2->usage)
		return 1;
	if (v1->resnum < v2->resnum)
		return -1;
	if (v1->resnum > v2->resnum)
		return 1;
	return 0;
}

giblorb_err_t Blorb::giblorb_destroy_map(giblorb_map_t *map) {
	if (!map || !map->chunks || map->inited != giblorb_Inited_Magic)
		return giblorb_err_NotAMap;

	for (uint ix = 0; ix<map->numchunks; ix++) {
		giblorb_chunkdesc_t *chu = &(map->chunks[ix]);
		if (chu->ptr) {
			delete chu->ptr;
			chu->ptr = nullptr;
		}
	}

	if (map->chunks) {
		delete[] map->chunks;
		map->chunks = nullptr;
	}

	map->numchunks = 0;

	if (map->resources) {
		delete[] map->resources;
		map->resources = nullptr;
	}

	if (map->ressorted) {
		delete[] map->ressorted;
		map->ressorted = nullptr;
	}

	map->numresources = 0;
	map->file = nullptr;
	map->inited = 0;

	delete map;

	return giblorb_err_None;
}

giblorb_err_t Blorb::giblorb_load_chunk_by_type(giblorb_map_t *map,
		glui32 method, giblorb_result_t *res, glui32 chunktype, glui32 count) {
	uint ix;

	for (ix = 0; ix < map->numchunks; ix++) {
		if (map->chunks[ix].type == chunktype) {
			if (count == 0)
				break;
			count--;
		}
	}

	if (ix >= map->numchunks) {
		return giblorb_err_NotFound;
	}

	return giblorb_load_chunk_by_number(map, method, res, ix);
}

giblorb_err_t Blorb::giblorb_load_chunk_by_number(giblorb_map_t *map,
		glui32 method, giblorb_result_t *res, glui32 chunknum) {
	giblorb_chunkdesc_t *chu;

	if (chunknum >= map->numchunks)
		return giblorb_err_NotFound;

	chu = &(map->chunks[chunknum]);

	switch (method) {
	case giblorb_method_DontLoad:
		// do nothing
		break;

	case giblorb_method_FilePos:
		res->data.startpos = chu->datpos;
		break;

	case giblorb_method_Memory:
		if (!chu->ptr) {
			glui32 readlen;
			byte *dat = new byte[chu->len];

			if (!dat)
				return giblorb_err_Alloc;

			map->file->seek(chu->datpos);

			readlen = map->file->read(dat, chu->len);
			if (readlen != chu->len)
				return giblorb_err_Read;

			chu->ptr = dat;
		}

		res->data.ptr = chu->ptr;
		break;
	}

	res->chunknum = chunknum;
	res->length = chu->len;
	res->chunktype = chu->type;

	return giblorb_err_None;
}

giblorb_err_t Blorb::giblorb_unload_chunk(giblorb_map_t *map, glui32 chunknum) {
	giblorb_chunkdesc_t *chu;

	if (chunknum >= map->numchunks)
		return giblorb_err_NotFound;

	chu = &(map->chunks[chunknum]);

	if (chu->ptr) {
		delete chu->ptr;
		chu->ptr = nullptr;
	}

	return giblorb_err_None;
}

giblorb_err_t Blorb::giblorb_load_resource(giblorb_map_t *map, glui32 method,
		giblorb_result_t *res, glui32 usage, glui32 resnum) {
	giblorb_resdesc_t sample;
	giblorb_resdesc_t *found;

	sample.usage = usage;
	sample.resnum = resnum;

	found = giblorb_bsearch(&sample, map->ressorted, map->numresources);

	if (!found)
		return giblorb_err_NotFound;

	return giblorb_load_chunk_by_number(map, method, res, found->chunknum);
}

giblorb_err_t Blorb::giblorb_count_resources(giblorb_map_t *map,
		glui32 usage, glui32 *num, glui32 *min, glui32 *max) {
	int ix;
	int count;
	glui32 val;
	glui32 minval, maxval;

	count = 0;
	minval = 0;
	maxval = 0;

	for (ix = 0; ix<map->numresources; ix++) {
		if (map->resources[ix].usage == usage) {
			val = map->resources[ix].resnum;
			if (count == 0) {
				count++;
				minval = val;
				maxval = val;
			}
			else {
				count++;
				if (val < minval)
					minval = val;
				if (val > maxval)
					maxval = val;
			}
		}
	}

	if (num)
		*num = count;
	if (min)
		*min = minval;
	if (max)
		*max = maxval;

	return giblorb_err_None;
}

giblorb_err_t Blorb::giblorb_set_resource_map(Common::SeekableReadStream *file) {
	giblorb_err_t err;

	err = giblorb_create_map(file, &_map);
	if (err) {
		_map = nullptr;
		return err;
	}

	_file = file;
	return giblorb_err_None;
}

giblorb_map_t *Blorb::giblorb_get_resource_map(void) {
	return _map;
}

bool Blorb::giblorb_is_resource_map(void) const {
	return _map != nullptr;
}

} // End of namespace Glk
