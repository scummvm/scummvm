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

#include "mediastation/actor.h"
#include "mediastation/bitmap.h"
#include "mediastation/datafile.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

class FontCharacter : public PixMapImage {
public:
	FontCharacter(Chunk &chunk, uint charCode, int horizontalSpacing, int baselineOffset, const ImageInfo &header);

	// Returns the ascent (baseline position). Falls back to full height if baseline offset is not specified.
	int16 ascent() const { return (_baselineOffset != 0) ? _baselineOffset : height(); }
	uint _charCode = 0;
	int16 _horizontalSpacing = 0; // Additional horizontal spacing added after the glyph width to get total advance

private:
	int16 _baselineOffset = 0;    // Baseline position within the glyph bitmap (ascent - distance from top edge to baseline)
};

class FontActor : public Actor, public ChannelClient {
public:
	FontActor() : Actor(kActorTypeFont) {};
	~FontActor();

	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual void readChunk(Chunk &chunk) override;
	virtual void loadIsComplete() override;

	FontCharacter *lookupCharacter(uint charCode) { return _characters.getValOrDefault(charCode); }

	int16 _totalWidthOfAllChars = 0;
	int16 _totalHeightOfAllChars = 0;
	int16 _averageCharWidth = 0;
	int16 _averageCharHeight = 0;
	int16 _maxAscent = 0;  // Maximum ascent (distance from top to baseline) across all glyphs.
	int16 _maxDescent = 0; // Maximum descent (distance from baseline to bottom) across all glyphs.

private:
	Common::HashMap<uint, FontCharacter *> _characters;
};

} // End of namespace MediaStation

#endif
