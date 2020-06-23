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

#ifndef GLK_ZCODE_MEM
#define GLK_ZCODE_MEM

#include "glk/zcode/frotz_types.h"
#include "glk/zcode/config.h"

namespace Glk {
namespace ZCode {

#define SET_WORD(addr,v)  zmp[addr] = hi(v); zmp[addr+1] = lo(v)
#define LOW_WORD(addr,v)  v = READ_BE_UINT16(&zmp[addr])
#define HIGH_WORD(addr,v) v = READ_BE_UINT16(&zmp[addr])
#define HIGH_LONG(addr,v) v = READ_BE_UINT32(&zmp[addr])
#define SET_BYTE(addr,v)   zmp[addr] = v
#define LOW_BYTE(addr,v)   v = zmp[addr]

typedef uint offset_t;

/**
 * Stores undo information
 */
struct undo_struct {
	undo_struct *next;
	undo_struct *prev;
	offset_t pc;
	long diff_size;
	zword frame_count;
	zword stack_size;
	zword frame_offset;
	// undo diff and stack data follow
};
typedef undo_struct undo_t;

/**
 * Handles the memory, header, and user options
 */
class Mem : public Header, public virtual UserOptions {
protected:
	Common::SeekableReadStream *story_fp;
	uint story_size;
	byte *pcp;
	byte *zmp;

	undo_t *first_undo, *last_undo, *curr_undo;
	zbyte *undo_mem, *prev_zmp, *undo_diff;
	int undo_count;
	int reserve_mem;
private:
	/**
	 * Handles setting the story file, parsing it if it's a Blorb file
	 */
	void initializeStoryFile();

	/**
	 * Handles loading the game header
	 */
	void loadGameHeader();

	/**
	 * Initializes memory and loads the story data
	 */
	void loadMemory();

	/**
	 * Setup undo data
	 */
	void initializeUndo();
protected:
	/**
	 * Read a value from the header extension (former mouse table).
	 */
	zword get_header_extension(int entry);

	/**
	 * Set an entry in the header extension (former mouse table).
	 */
	void set_header_extension(int entry, zword val);

	/**
	 * Set all header fields which hold information about the interpreter.
	 */
	void restart_header();

	/**
	 * Write a byte value to the dynamic Z-machine memory.
	 */
	void storeb(zword addr, zbyte value);

	/**
	 * Write a word value to the dynamic Z-machine memory.
	 */
	void storew(zword addr, zword value);

	/**
	 * Free count undo blocks from the beginning of the undo list
	 */
	void free_undo(int count);

	/**
	 * Generates a runtime error
	 */
	virtual void runtimeError(ErrorCode errNum) = 0;

	/**
	 * Called when the flags are changed
	 */
	virtual void flagsChanged(zbyte value) = 0;

	/**
	 * Close the story file and deallocate memory.
	 */
	void reset_memory();

	/**
	 * Set diff to a Quetzal-like difference between a and b,
	 * copying a to b as we go.  It is assumed that diff points to a
	 * buffer which is large enough to hold the diff.
	 * mem_size is the number of bytes to compare.
	 * Returns the number of bytes copied to diff.
	 *
	 */
	long mem_diff(zbyte *a, zbyte *b, zword mem_size, zbyte *diff);

	/**
	 * Applies a quetzal-like diff to dest
	 */
	void mem_undiff(zbyte *diff, long diff_length, zbyte *dest);
public:
	/**
	 * Constructor
	 */
	Mem();

	/**
	 * Destructor
	 */
	virtual ~Mem() {}

	/**
	 * Initialize
	 */
	void initialize();
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
