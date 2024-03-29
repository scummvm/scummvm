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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef GOB_DECFILE_H
#define GOB_DECFILE_H

#include "common/system.h"

namespace Common {
	class String;
	class SeekableReadStreamEndian;
}

namespace Gob {

class GobEngine;
class Surface;
class CMPFile;

/** A DEC file, describing a "decal" (background).
 *
 *  Used in hardcoded "actiony" parts of gob games.
 *  The principle is similar to a Static in Scenery (see scenery.cpp), but
 *  instead of referencing indices in the sprites array, DECs reference sprites
 *  directly by filename.
 */
class DECFile {
public:
	DECFile(GobEngine *vm, const Common::String &fileName,
	        uint16 width, uint16 height, uint8 bpp = 1);
	~DECFile();

	/** Draw the background, including all default layer parts. */
	void draw(Surface &dest) const;

	/** Explicitly draw the backdrop. */
	void drawBackdrop(Surface &dest) const;

	/** Explicitly draw a layer part. */
	void drawLayer(Surface &dest, uint16 layer, uint16 part,
	               uint16 x, uint16 y, int32 transp = -1) const;

private:
	struct Part {
		uint8 layer;
		uint8 part;

		uint16 x;
		uint16 y;
		bool transp;
	};

	typedef Common::Array<CMPFile *> LayerArray;
	typedef Common::Array<Part>      PartArray;

	GobEngine *_vm;

	uint16 _width;
	uint16 _height;
	uint8  _bpp;

	byte _hasPadding;

	CMPFile *_backdrop;

	LayerArray _layers;
	PartArray  _parts;


	void load(Common::SeekableReadStreamEndian &dec, const Common::String &fileName);

	void loadBackdrop(Common::SeekableReadStreamEndian &dec);

	CMPFile *loadLayer(Common::SeekableReadStreamEndian &dec);

	void loadParts(Common::SeekableReadStreamEndian &dec);
	void loadPart(Part &part, Common::SeekableReadStreamEndian &dec);
};

} // End of namespace Gob

#endif // GOB_DECFILE_H
