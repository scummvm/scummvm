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

#ifndef XEEN_RESOURCES_H
#define XEEN_RESOURCES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/file.h"
#include "graphics/surface.h"
#include "xeen/xsurface.h"

namespace Xeen {

class XeenEngine;

class Resources {
public:
	/**
	 * Instantiates the resource manager
	 */
	static void init(XeenEngine *vm);
};

/**
 * Derived file class
 */
class File : public Common::File {
public:
	File() : Common::File() {}
	File(const Common::String &filename) { openFile(filename); }
	virtual ~File() {}

	void openFile(const Common::String &filename);
};

class SpriteResource {
private:
	struct IndexEntry {
		uint16 _offset1, _offset2;
	};
	Common::Array<IndexEntry> _index;

	int32 _filesize;
	byte *_data;

	void drawOffset(XSurface &dest, uint16 offset, const Common::Point &destPos) const;
public:
	SpriteResource(const Common::String &filename);

	~SpriteResource();

	int size() const;

	void draw(XSurface &dest, int frame, const Common::Point &destPos) const;

	void draw(XSurface &dest, int frame) const;
};

} // End of namespace Xeen

#endif /* MADS_RESOURCES_H */
