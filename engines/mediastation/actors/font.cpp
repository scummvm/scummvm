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
#include "mediastation/actors/font.h"

namespace MediaStation {

FontGlyph::FontGlyph(Chunk &chunk, uint asciiCode, uint unk1, uint unk2, BitmapHeader *header) : Bitmap(chunk, header) {
	_asciiCode = asciiCode;
	_unk1 = unk1;
	_unk2 = unk2;
}

FontActor::~FontActor() {
	unregisterWithStreamManager();
	for (auto it = _glyphs.begin(); it != _glyphs.end(); ++it) {
		delete it->_value;
	}
	_glyphs.clear();
}

void FontActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderChannelIdent:
		_channelIdent = chunk.readTypedChannelIdent();
		registerWithStreamManager();
		break;

	default:
		Actor::readParameter(chunk, paramType);
	}
}

void FontActor::readChunk(Chunk &chunk) {
	debugC(5, kDebugLoading, "FontActor::readChunk(): Reading font glyph (@0x%llx)", static_cast<long long int>(chunk.pos()));
	uint asciiCode = chunk.readTypedUint16();
	int unk1 = chunk.readTypedUint16();
	int unk2 = chunk.readTypedUint16();
	BitmapHeader *header = new BitmapHeader(chunk);
	FontGlyph *glyph = new FontGlyph(chunk, asciiCode, unk1, unk2, header);
	if (_glyphs.getValOrDefault(asciiCode) != nullptr) {
		error("%s: Glyph for ASCII code 0x%x already exists", __func__, asciiCode);
	}
	_glyphs.setVal(asciiCode, glyph);
}

} // End of namespace MediaStation
