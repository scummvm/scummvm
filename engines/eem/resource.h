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

namespace Common {
class SeekableReadStream;
}

namespace EEM {

struct DBEntry {
	uint32 offset;     ///< Byte offset in the .DBD file.
	uint16 compressed; ///< Non-zero = PKWARE DCL ("Implode") packed payload.
	uint32 size;       ///< Total entry size on disk (incl. 14-byte header).
};

/// 8-bit indexed picture decoded from a .DBD entry. Original PicData is
/// 16 bytes; pixels live in Graphics::ManagedSurface.
struct Picture {
	uint16 flags     = 0; ///< +0  high byte: sub-mode
	uint16 rowoff    = 0; ///< +6  row offset (clipped sprites)
	uint16 miscflags = 0; ///< +8  high byte: transparent-mask flag
	uint32 compsize  = 0; ///< Packed payload size on disk.
	Graphics::ManagedSurface surface;
};

/// Multi-frame animation from ANI.DBD.
typedef Common::Array<Picture> Animation;

/// .DBD + .DBX archive pair (PICS, SITES, ANI, BALLOON, BUTTON).
class DBDArchive {
public:
	DBDArchive();
	~DBDArchive();

	/// Open both halves; returns false if either file is missing/malformed.
	bool open(const Common::Path &dbdName, const Common::Path &dbxName, bool bigEndian = false);
	void close();

	uint32 size() const { return _index.size(); }

	/// Load entry num (0-based), decompressing if needed.
	/// _GetFromDB @ 172b:105d.
	bool loadEntry(uint num, Picture &out);

	/// 1-based wrapper matching the engine's _GetPicture(num) -> _GetFromDB(num - 1).
	bool getPicture(uint num, Picture &out) { return loadEntry(num - 1, out); }

	/// Multi-frame entry. _GetAnimation @ 172b:163a: u16 frameCount,
	/// then per frame a 12-byte header + payload.
	bool loadAnimation(uint num, Animation &out);

private:
	Common::File _dbd;
	Common::Array<DBEntry> _index;
	bool _bigEndian = false;
};

Common::SeekableReadStream *openMacResource(const Common::Path &path,
											uint32 typeId,
											uint16 resourceId);

} // End of namespace EEM

#endif
