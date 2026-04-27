/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EEM_RESOURCE_H
#define EEM_RESOURCE_H

#include "common/array.h"
#include "common/file.h"
#include "common/path.h"
#include "common/scummsys.h"

#include "graphics/managed_surface.h"

namespace EEM {

/**
 * Index entry for a .DBD/.DBX archive pair (10 bytes on disk).
 *
 * Mirrors the original `dbi` struct read by _InitGraphicsSystem @ 172b:0145
 * in 10-byte chunks until EOF. Each entry locates one resource blob in the
 * companion .DBD container.
 */
struct DBEntry {
	uint32 offset;     ///< Byte offset of the entry in the .DBD file.
	uint16 compressed; ///< Non-zero if the payload is PKWARE DCL ("Implode") packed.
	uint32 size;       ///< Total size of the entry on disk (including 14-byte header).
};

/**
 * 8-bit indexed picture decoded from a .DBD entry.
 *
 * The original engine's PicData is a 16-byte struct; we keep the descriptive
 * fields here and let `Graphics::ManagedSurface` own the pixel data so the
 * rest of the engine can blit/scale/clip with the standard API.
 */
struct Picture {
	uint16 flags     = 0; ///< +0  high byte = sub-mode used by some sprites
	uint16 rowoff    = 0; ///< +6  row offset (used by some clipped sprites)
	uint16 miscflags = 0; ///< +8  high byte = transparent-mask flag
	uint16 compsize  = 0; ///< +10 packed payload size on disk
	Graphics::ManagedSurface surface;
};

/// Multi-frame animation as stored in ANI.DBD — a sequence of Pictures.
typedef Common::Array<Picture> Animation;

/**
 * Reader for a .DBD + .DBX archive pair.
 *
 * The original engine has five such pairs: PICS, SITES, ANI, BALLOON, BUTTON.
 * Each .DBX is parsed once into an in-memory `_index`; reads of individual
 * entries seek into the .DBD on demand and (when flagged) decompress with
 * `Common::decompressDCL`.
 */
class DBDArchive {
public:
	DBDArchive();
	~DBDArchive();

	/**
	 * Open both halves of an archive. @p dbdName / @p dbxName are looked up
	 * via SearchMan, so case is normalized for us. Returns false if either
	 * file is missing or the index is malformed.
	 */
	bool open(const Common::Path &dbdName, const Common::Path &dbxName);
	void close();

	/** Number of entries in the index. */
	uint32 size() const { return _index.size(); }

	/**
	 * Load entry @p num (0-based index), decompressing if needed.
	 * Returns true on success. Mirrors _GetFromDB @ 172b:105d.
	 */
	bool loadEntry(uint num, Picture &out);

	/**
	 * Convenience wrapper that mirrors the engine's 1-based picture API:
	 * `_GetPicture(num)` calls `_GetFromDB(..., num - 1)`. Use this when
	 * porting code that references picture IDs by their original number.
	 */
	bool getPicture(uint num, Picture &out) { return loadEntry(num - 1, out); }

	/**
	 * Load a multi-frame animation entry. Mirrors _GetAnimation @ 172b:163a:
	 * read u16 frameCount, then for each frame read a 12-byte header and
	 * decompress the payload. Used for ANI.DBD entries.
	 */
	bool loadAnimation(uint num, Animation &out);

private:
	Common::File _dbd;
	Common::Array<DBEntry> _index;
};

} // End of namespace EEM

#endif
