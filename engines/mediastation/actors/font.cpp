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
#include "mediastation/mediastation.h"

namespace MediaStation {

FontCharacter::FontCharacter(Chunk &chunk, uint charCode, int horizontalSpacing, int baselineOffset, const ImageInfo &header) :
	PixMapImage(chunk, header),
	_charCode(charCode),
	_horizontalSpacing(horizontalSpacing),
	_baselineOffset(baselineOffset) {
}

FontActor::~FontActor() {
	unregisterWithStreamManager();
	for (auto it = _characters.begin(); it != _characters.end(); ++it) {
		delete it->_value;
	}
	_characters.clear();
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
	// This is always 16-bit because there are some special char codes above 0xFF,
	// such as the cursor and arrow keys.
	uint charCode = chunk.readTypedUint16();
	int16 horizontalSpacing = static_cast<int16>(chunk.readTypedUint16());
	int16 baselineOffset = static_cast<int16>(chunk.readTypedUint16());
	ImageInfo header(chunk);
	FontCharacter *glyph = new FontCharacter(chunk, charCode, horizontalSpacing, baselineOffset, header);
	if (_characters.getValOrDefault(charCode) != nullptr) {
		warning("[%s] %s: Glyph for char code 0x%x already exists", debugName(), __func__, charCode);
	}
	_characters.setVal(charCode, glyph);
	_totalHeightOfAllChars += glyph->height();
	_totalWidthOfAllChars += glyph->width();

	// Track the maximum ascent across all glyphs.
	// The ascent is either the specified baseline offset, or the full glyph height if not specified.
	_maxAscent = MAX(_maxAscent, glyph->ascent());

	// Calculate descent (distance from baseline to bottom of glyph)
	int16 charDescent = glyph->height() - glyph->ascent();
	_maxDescent = MAX(_maxDescent, charDescent);
}

void FontActor::loadIsComplete() {
	if (_characters.size() > 0) {
		_averageCharWidth = _totalWidthOfAllChars / _characters.size();
		_averageCharHeight = _totalHeightOfAllChars / _characters.size();
	}
	Actor::loadIsComplete();
}

} // End of namespace MediaStation
