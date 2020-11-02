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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SLUDGE_FONTTEXT_H
#define SLUDGE_FONTTEXT_H

#include "common/hashmap.h"
#include "common/ustr.h"

#include "sludge/sprites.h"

namespace Sludge {

struct SpriteBank;
class SpritePalette;

class TextManager {
public:
	TextManager();
	virtual ~TextManager();

	void init();
	void kill();

	int stringWidth(const Common::String &theText);
	int stringLength(const Common::String &theText);

	bool loadFont(int filenum, const Common::String &charOrder, int);
	void pasteString(const Common::String &theText, int, int, SpritePalette &);
	void pasteStringToBackdrop(const Common::String &theText, int xOff, int y);
	void burnStringToBackdrop(const Common::String &theText, int xOff, int y);
	bool isInFont(const Common::String &theText);

	// setter & getter
	void setFontSpace(int fontSpace) { _fontSpace = fontSpace; }
	int getFontHeight() const { return _fontHeight; }
	void setPasterColor(byte r, byte g, byte b) { _pastePalette.setColor(r, g, b); }

	// load & save
	void saveFont(Common::WriteStream *stream);
	void loadFont(int ssgVersion, Common::SeekableReadStream *stream);

private:
	SpriteBank _theFont;
	int _fontHeight, _numFontColours, _loadedFontNum;
	Common::U32String _fontOrder;
	int16 _fontSpace;
	SpritePalette _pastePalette;

	Common::HashMap<uint32, uint32> _fontTable;

	inline uint32 fontInTable(uint32 x) { return _fontTable[x]; }

};

} // End of namespace Sludge

#endif
