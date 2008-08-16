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

#ifndef CINE_ANIM_H
#define CINE_ANIM_H

#include "common/endian.h"

namespace Cine {

/**
 * Cine engine's save game formats.
 * Enumeration entries (Excluding the one used as an error)
 * are sorted according to age (i.e. top one is oldest, last one newest etc).
 *
 * ANIMSIZE_UNKNOWN:
 * - Animation data entry size is unknown (Used as an error).
 *
 * ANIMSIZE_23:
 * - Animation data entry size is 23 bytes.
 * - Used at least by 0.11.0 and 0.11.1 releases of ScummVM.
 * - Introduced in revision 21772, stopped using in revision 31444.
 *
 * ANIMSIZE_30_PTRS_BROKEN:
 * - Animation data entry size is 30 bytes.
 * - Data and mask pointers in the saved structs are always NULL.
 * - Introduced in revision 31453, stopped using in revision 32073.
 *
 * ANIMSIZE_30_PTRS_INTACT:
 * - Animation data entry size is 30 bytes.
 * - Data and mask pointers in the saved structs are intact,
 *   so you can test them for equality or inequality with NULL
 *   but don't try using them for anything else, it won't work.
 * - Introduced in revision 31444, got broken in revision 31453,
 *   got fixed in revision 32073 and used after that.
 *
 * TEMP_OS_FORMAT:
 * - Temporary Operation Stealth savegame format.
 * - NOT backward compatible and NOT to be supported in the future.
 *   This format should ONLY be used during development and abandoned
 *   later in favor of a better format!
 */
enum CineSaveGameFormat {
	ANIMSIZE_UNKNOWN,
	ANIMSIZE_23,
	ANIMSIZE_30_PTRS_BROKEN,
	ANIMSIZE_30_PTRS_INTACT,
	TEMP_OS_FORMAT
};

/** Identifier for the temporary Operation Stealth savegame format. */
static const uint32 TEMP_OS_FORMAT_ID = MKID_BE('TEMP');

/** The current version number of Operation Stealth's savegame format. */
static const uint32 CURRENT_OS_SAVE_VER = 0;

/** Chunk header used by the temporary Operation Stealth savegame format. */
struct ChunkHeader {
	uint32 id;      ///< Identifier (e.g. MKID_BE('TEMP'))
	uint32 version; ///< Version number
	uint32 size;    ///< Size of the chunk after this header in bytes
};

struct AnimHeaderStruct {
	byte field_0;
	byte field_1;
	byte field_2;
	byte field_3;
	uint16 frameWidth;
	uint16 frameHeight;
	byte field_8;
	byte field_9;
	byte field_A;
	byte field_B;
	byte field_C;
	byte field_D;
	uint16 numFrames;
	byte field_10;
	byte field_11;
	byte field_12;
	byte field_13;
	uint16 field_14;
};

struct AnimDataEntry {
	char name[9];
	byte color;
};

#define ANIM_RAW 0 // memcpy
#define ANIM_MASK 1 // convertMask
#define ANIM_SPRITE 2 // gfxConvertSpriteToRaw
#define ANIM_MASKSPRITE 3 // gfxConvertSpriteToRaw + generateMask
#define ANIM_PALSPRITE 5 // convert8BBP
#define ANIM_FULLSPRITE 8 // convert8BBP2

class AnimData {
private:
	byte *_data; ///< Image data
	byte *_mask; ///< Image mask (may be NULL)
	int16 _fileIdx; ///< Source file index in bundle
	int16 _frameIdx; ///< Frame number in animation
	char _name[10]; ///< Part filename
	int _size; ///< _data/_mask size, internal only

public:
	uint16 _width; ///< Image width (usually twice the real size)
	uint16 _height; ///< Image height
	uint16 _bpp; ///< Bit depth/type information
	uint16 _var1; ///< Something related to width
	int _realWidth; ///< Real image width in bytes

	AnimData();
	AnimData(const AnimData &src);
	~AnimData();

	AnimData &operator=(const AnimData &src);

	const byte *data() const { return _data; } ///< Image data
	const byte *mask() const { return _mask; } ///< Image mask (may be NULL)
	byte getColor(int x, int y);

	void load(byte *d, int type, uint16 w, uint16 h, int16 file, int16 frame, const char *n, byte transparent = 0);
	void clear();

	void save(Common::OutSaveFile &fHandle) const;
};

#define NUM_MAX_ANIMDATA 255

extern Common::Array<AnimData> animDataTable;

void freeAnimDataTable(void);
void freeAnimDataRange(byte startIdx, byte numIdx);
int loadResource(const char *resourceName, int16 idx = -1);
void loadResourcesFromSave(Common::SeekableReadStream &fHandle, enum CineSaveGameFormat saveGameFormat);
void generateMask(const byte *sprite, byte *mask, uint16 size, byte transparency);

} // End of namespace Cine

#endif
