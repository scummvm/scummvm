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

#ifndef ACCESS_BUBBLE_BOX_H
#define ACCESS_BUBBLE_BOX_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str-array.h"
#include "common/stream.h"
#include "common/types.h"
#include "graphics/surface.h"
#include "access/data.h"

namespace Access {

class AccessEngine;

enum BoxType {
	TYPE_0 = 0,
	TYPE_1 = 1,
	TYPE_2 = 2,
	TYPE_3 = 3,
	kBoxTypeFileDialog = 4,
	kTextBoxNoctPlain = 1,
	kTextBoxNoctCaption = 2,
	kTextBoxNoctCenter = 4,
};

class BubbleBox : public Manager {
private:
	int _startItem, _startBox;
	int _charCol, _rowOff;
	Common::Point _fileStart;
	Common::Point _fileOff;
	int _boxStartX, _boxStartY;
	int _boxEndX, _boxEndY;
	int _bIconStartX, _bIconStartY;
	int _boxPStartX, _boxPStartY;

	void displayBoxData();
	void drawSelectBox();
	/**
	 * Prints a text bubble and it's contents
	 */
	void printBubble_v1(const Common::String &msg);
	void printBubble_v2(const Common::String &msg);
	void printBubble_v3(const Common::String &msg);

public:
	BoxType _type;
	Common::Rect _bounds;
	Common::StringArray _nameIndex;
	Common::String _bubbleTitle; // the active default "script" title
	Common::String _bubbleDisplStr; // the title that will be drawn on the next bubble
	Common::String _tempList[60];
	int _tempListIdx[60];
	int _btnId1;
	int _btnX1;
	int _btnId2;
	int _btnX2;
	int _btnId3;
	int _btnX3;
	Common::Rect _btnUpPos;
	Common::Rect _btnDownPos;
	Common::Array<Common::Rect> _bubbles;
public:
	BubbleBox(AccessEngine *vm, Access::BoxType type, int x, int y, int w, int h, int val1, int val2, int val3, int val4, const char *title);

	void load(Common::SeekableReadStream *stream);

	void clearBubbles();

	void placeBubble(const Common::String &msg);
	void placeBubble1(const Common::String &msg);

	/**
	 * Calculate the size of a bubble needed to hold a given string
	 */
	void calcBubble(const Common::String &msg);

	/**
	 * Noctropolis version of bubble calculation
	 */
	//void calcBubble_v3(const Common::String &msg);

	/**
	 * Prints a text bubble and it's contents
	 */
	void printBubble(const Common::String &msg);

	/*
	 * Draws the background for a text bubble
	 * @param index		Index of bounds in _bubbles array
	 */
	void drawBubble(int index);

	void doBox_v2(int item, int box);
	void doBox_v3(int item, int box);

	int doBox_v1(int item, int box, int &btnSelected);
	void getList(const char *const data[], const byte *flags);
	void setCursorPos(int posX, int posY);
	void printString(Common::String msg);
};

} // End of namespace Access

#endif /* ACCESS_BUBBLE_BOX_H */
