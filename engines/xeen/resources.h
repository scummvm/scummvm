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
	Common::Array<XSurface> _frames;
public:
	SpriteResource(const Common::String &filename);

	int size() const;

	const XSurface &getFrame(int frame);
};

} // End of namespace Xeen

#endif /* MADS_RESOURCES_H */
