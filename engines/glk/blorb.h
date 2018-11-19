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
#include "common/archive.h"
#include "common/array.h"

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

/**
 * Blorb file manager
 */
class Blorb : public Common::Archive {
private:
	Common::File _file;
	InterpreterType _interpType;
	giblorb_map_t *_map;
private:
	/**
	 * Parses the Blorb file index to load in a list of the chunks
	 */
	giblorb_err_t create_map();

	giblorb_err_t initialize_map();
	void qsort(giblorb_resdesc_t **list, size_t len);
	giblorb_resdesc_t *bsearch(giblorb_resdesc_t *sample, giblorb_resdesc_t **list, int len);
	int sortsplot(giblorb_resdesc_t *v1, giblorb_resdesc_t *v2);
public:
	/**
	 * Constructor
	 */
	Blorb(const Common::String &filename, InterpreterType interpType);

	/**
	 * Destructor
	 */
	~Blorb();

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	virtual bool hasFile(const Common::String &name) const override;

	/**
	 * Add all members of the Archive matching the specified pattern to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of members added to list
	 */
	virtual int listMatchingMembers(Common::ArchiveMemberList &list, const Common::String &pattern) const override;

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of names added to list
	 */
	virtual int listMembers(Common::ArchiveMemberList &list) const override;

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
public:
	/**
	 * Get a pointer to the Blorb's resource map
	 */
	giblorb_map_t *get_resource_map() const { return _map; }

	giblorb_err_t load_chunk_by_type(glui32 method, giblorb_result_t *res, glui32 chunktype, glui32 count);
	giblorb_err_t load_chunk_by_number(glui32 method, giblorb_result_t *res, glui32 chunknum);
	giblorb_err_t unload_chunk(glui32 chunknum);

	giblorb_err_t load_resource(glui32 method, giblorb_result_t *res, glui32 usage, glui32 resnum);
	giblorb_err_t count_resources(glui32 usage, glui32 *num, glui32 *min, glui32 *max);
};

} // End of namespace Glk

#endif
