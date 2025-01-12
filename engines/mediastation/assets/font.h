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

#ifndef MEDIASTATION_FONT_H
#define MEDIASTATION_FONT_H

#include "mediastation/asset.h"
#include "mediastation/assetheader.h"
#include "mediastation/bitmap.h"
#include "mediastation/chunk.h"
#include "mediastation/mediascript/operand.h"
#include "mediastation/mediascript/builtins.h"

namespace MediaStation {

class FontGlyph : public Bitmap {
public:
	FontGlyph(Chunk &chunk, uint asciiCode, uint unk1, uint unk2, BitmapHeader *header);
    uint _asciiCode = 0;

private:
    int _unk1 = 0;
    int _unk2 = 0;
};

class Font : public Asset {
public:
	Font(AssetHeader *header) : Asset(header) {};
	~Font();

	virtual Operand callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) override;

	virtual void readChunk(Chunk &chunk) override;

private:
	Common::HashMap<uint, FontGlyph *> _glyphs;

	// Method implementations.
    // No methods are implemented as of now.
};

} // End of namespace MediaStation

#endif