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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GLK_BLORB_H
#define GLK_BLORB_H

#include "glk/glk_types.h"
#include "glk/streams.h"

namespace Glk {


/**
 * Error type
 */
typedef glui32 giblorb_err_t;

/**
 * Error codes
 */
enum giblorbError {
	giblorb_err_None        = 0,
	giblorb_err_CompileTime = 1,
	giblorb_err_Alloc       = 2,
	giblorb_err_Read        = 3,
	giblorb_err_NotAMap     = 4,
	giblorb_err_Format      = 5,
	giblorb_err_NotFound    = 6
};

/**
 * Methods for loading a chunk
 */
enum giblorbMethod {
	giblorb_method_DontLoad = 0,
	giblorb_method_Memory   = 1,
	giblorb_method_FilePos  = 2
};

enum {
	giblorb_ID_Snd       = MKTAG('S', 'n', 'd', ' '),
	giblorb_ID_Exec      = MKTAG('E', 'x', 'e', 'c'),
	giblorb_ID_Pict      = MKTAG('P', 'i', 'c', 't'),
	giblorb_ID_Copyright = MKTAG('(', 'c', ')', ' '),
	giblorb_ID_AUTH      = MKTAG('A', 'U', 'T', 'H'),
	giblorb_ID_ANNO      = MKTAG('A', 'N', 'N', 'O')
};


enum {
	giblorb_ID_MOD  = MKTAG('M', 'O', 'D', ' '),
	giblorb_ID_FORM = MKTAG('F', 'O', 'R', 'M'),
	giblorb_ID_IFRS = MKTAG('I', 'F', 'R', 'S'),
	giblorb_ID_RIdx = MKTAG('R', 'I', 'd', 'x'),
	giblorb_ID_OGG  = MKTAG('O', 'G', 'G', 'V'),

	// non-standard types
	giblorb_ID_MIDI = MKTAG('M', 'I', 'D', 'I'),
	giblorb_ID_MP3  = MKTAG('M', 'P', '3', ' '),
	giblorb_ID_WAVE = MKTAG('W', 'A', 'V', 'E')
};

/**
 * Holds the complete description of an open Blorb file.
 * This type is opaque for normal interpreter use.
 */
typedef struct giblorb_map_struct giblorb_map_t;

/**
 * giblorb_result_t: Result when you try to load a chunk.
 */
typedef struct giblorb_result_struct {
	glui32 chunknum; // The chunk number (for use in giblorb_unload_chunk(), etc.)
	union {
		void *ptr;			///< A pointer to the data (if you used giblorb_method_Memory)
		glui32 startpos;	///< The position in the file (if you used giblorb_method_FilePos)
	} data;

	glui32 length;			///< The length of the data
	glui32 chunktype;		///< The type of the chunk.
} giblorb_result_t;

typedef struct giblorb_resdesc_struct giblorb_resdesc_t;

class Blorb {
private:
	bool _libInited;
	Common::SeekableReadStream *_file;
	giblorb_map_t *_map;
private:
	/**
	 * Initializes Blorb
	 */
	giblorb_err_t giblorb_initialize();

	giblorb_err_t giblorb_initialize_map(giblorb_map_t *map);
	void giblorb_qsort(giblorb_resdesc_t **list, size_t len);
	giblorb_resdesc_t *giblorb_bsearch(giblorb_resdesc_t *sample,
		giblorb_resdesc_t **list, int len);
	int sortsplot(giblorb_resdesc_t *v1, giblorb_resdesc_t *v2);
public:
	/**
	 * Constructor
	 */
	Blorb() : _libInited(false), _file(nullptr), _map(nullptr) {}

	giblorb_err_t giblorb_set_resource_map(Common::SeekableReadStream *file);
	giblorb_map_t *giblorb_get_resource_map(void);
	bool giblorb_is_resource_map(void) const;


	giblorb_err_t giblorb_create_map(Common::SeekableReadStream *file, giblorb_map_t **newmap);
	giblorb_err_t giblorb_destroy_map(giblorb_map_t *map);

	giblorb_err_t giblorb_load_chunk_by_type(giblorb_map_t *map,
		glui32 method, giblorb_result_t *res, glui32 chunktype, glui32 count);
	giblorb_err_t giblorb_load_chunk_by_number(giblorb_map_t *map,
		glui32 method, giblorb_result_t *res, glui32 chunknum);
	giblorb_err_t giblorb_unload_chunk(giblorb_map_t *map, glui32 chunknum);

	giblorb_err_t giblorb_load_resource(giblorb_map_t *map, glui32 method,
		giblorb_result_t *res, glui32 usage, glui32 resnum);
	giblorb_err_t giblorb_count_resources(giblorb_map_t *map,
		glui32 usage, glui32 *num, glui32 *min, glui32 *max);
};

} // End of namespace Glk

#endif
