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

#include "mediastation/debugchannels.h"
#include "mediastation/assets/font.h"

namespace MediaStation {

FontGlyph::FontGlyph(Chunk &chunk, uint asciiCode, uint unk1, uint unk2, BitmapHeader *header) : Bitmap(chunk, header) {
    _asciiCode = asciiCode;
    _unk1 = unk1;
    _unk2 = unk2;
}

Font::~Font() {
    _glyphs.clear();
}

Operand Font::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
    error("Font::callMethod(): Font does not have any callable methods");
}

void Font::readChunk(Chunk &chunk) {
	debugC(5, kDebugLoading, "Font::readChunk(): Reading font glyph (@0x%llx)", static_cast<long long int>(chunk.pos()));
    uint asciiCode = Datum(chunk).u.i;
    int unk1 = Datum(chunk).u.i;
    int unk2 = Datum(chunk).u.i;
	BitmapHeader *header = new BitmapHeader(chunk);
	FontGlyph *glyph = new FontGlyph(chunk, asciiCode, unk1, unk2, header);
    if (_glyphs.getValOrDefault(asciiCode) != nullptr) {
        error("Font::readChunk(): Glyph for ASCII code 0x%x already exists", asciiCode);
    }
	_glyphs.setVal(asciiCode, glyph);
}

} // End of namespace MediaStation
