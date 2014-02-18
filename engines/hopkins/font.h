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

#ifndef HOPKINS_FONT_H
#define HOPKINS_FONT_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/str.h"

namespace Hopkins {

#define MAX_TEXT 11

class HopkinsEngine;

struct TxtItem {
	bool _textOnFl;
	Common::String _filename;
	Common::Point _pos;
	int _messageId;
	int _lineCount;
	Common::String _lines[10];
	int _textType;
	int _length;
	byte *_textBlock;
	int16 _width;
	int16 _height;
	bool _textLoadedFl;
	int _color;
};

struct TxtItemList {
	bool _enabledFl;
	Common::Point _pos;
	int16 _width;
	int16 _height;
};

class FontManager {
private:
	HopkinsEngine *_vm;

	void setTextColor(int idx, byte colByte);

	int _textSortArray[21];
	Common::String _oldName;
	Common::String _indexName;
	int _index[4048];
	byte *_tempText;
	byte *_zoneText;
	int _boxWidth;

	void loadZoneText();
public:
	byte *_font;
	int _fontFixedWidth;
	int _fontFixedHeight;
	TxtItem _text[12];
	TxtItemList _textList[12];

	FontManager(HopkinsEngine *vm);
	~FontManager();
	void clearAll();
	void initData();

	void showText(int idx);
	void hideText(int idx);
	void initTextBuffers(int idx, int messageId, const Common::String &filename, int xp, int yp, int textType, int length, int color);
	void displayText(int xp, int yp, const Common::String &message, int col);
	void displayTextVesa(int xp, int yp, const Common::String &message, int col);
	void renderTextDisplay(int xp, int yp, const Common::String &msg, int col);
	void setOptimalColor(int idx1, int idx2, int idx3, int idx4);
	void box(int idx, int messageId, const Common::String &filename, int xp, int yp);
};

} // End of namespace Hopkins

#endif /* HOPKINS_FONT_H */
