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

#ifndef ILLUSIONS_BBDOU_BBDOU_BUBBLE_H
#define ILLUSIONS_BBDOU_BBDOU_BUBBLE_H

#include "illusions/specialcode.h"
#include "common/rect.h"

namespace Illusions {

class IllusionsEngine_BBDOU;
class BbdouSpecialCode;
class Control;

struct Item0 {
	uint32 _sequenceId1;
	uint32 _sequenceId2;
	int16 _count;
	uint32 _progResKeywordId;
	uint32 _baseNamedPointId;
	uint32 _namedPointIds[32];
	uint32 _objectId;
	Common::Point _pt;
	Item0() : _count(0) {}
};

struct Item141C {
	uint32 _objectId;
	int16 _enabled;
	Common::Point _position;
	int16 _fieldA;
	uint32 _sequenceId;
};

class BbdouBubble {
public:
	BbdouBubble(IllusionsEngine_BBDOU *vm, BbdouSpecialCode *bbdou);
	~BbdouBubble();
	void init();
	void addItem0(uint32 sequenceId1, uint32 sequenceId2, uint32 progResKeywordId,
		uint32 namedPointId, int16 count, uint32 *namedPointIds);
	void show();
	void hide();
	void setup(int16 minCount, Common::Point pt1, Common::Point pt2, uint32 progResKeywordId);
	uint32 addItem(uint positionIndex, uint32 sequenceId);
	void calcBubbles(Common::Point &pt1, Common::Point &pt2);
protected:
	IllusionsEngine_BBDOU *_vm;
	BbdouSpecialCode *_bbdou;
	Common::Array<Item0> _item0s;
	Item0 *_currItem0;
	Item0 *_prevItem0;
	Item0 *_someItem0;
	uint32 _objectIds[32];
	Common::Point _pt1;
	Common::Point _pt2;
	int _objectId1414;
	int _objectId1418;
	Item141C _items[32];
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_BUBBLE_H
