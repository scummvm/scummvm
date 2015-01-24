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
 */

#ifndef XEEN_SPRITES_H
#define XEEN_SPRITES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/file.h"
#include "graphics/surface.h"
#include "xeen/xsurface.h"

namespace Xeen {

class XeenEngine;

enum SpriteFlags { SPRFLAG_2000 = 0x2000, SPRFLAG_4000 = 0x4000, 
	SPRFLAG_HORIZ_FLIPPED = 0x8000, SPRFLAG_RESIZE = 0x10000 };

class SpriteResource {
private:
	struct IndexEntry {
		uint16 _offset1, _offset2;
	};
	Common::Array<IndexEntry> _index;
	int32 _filesize;
	byte *_data;

	void load(Common::SeekableReadStream &f);

	void drawOffset(XSurface &dest, uint16 offset, const Common::Point &destPos, int flags) const;
public:
	SpriteResource();
	SpriteResource(const Common::String &filename);

	virtual ~SpriteResource();

	SpriteResource &operator=(const SpriteResource &src);

	void load(const Common::String &filename);

	void load(const Common::String &filename, Common::Archive &archive);

	void clear();

	void draw(XSurface &dest, int frame, const Common::Point &destPos, 
		int flags = 0, int scale = 0) const;

	void draw(XSurface &dest, int frame) const;

	int size() const { return _index.size(); }

	bool empty() const { return _index.size() == 0; }
};

} // End of namespace Xeen

#endif /* MADS_SPRITES_H */
