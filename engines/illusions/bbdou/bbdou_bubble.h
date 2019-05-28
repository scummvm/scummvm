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

struct BubbleStyle {
	uint32 _showSequenceId;
	uint32 _hideSequenceId;
	int16 _count;
	uint32 _progResKeywordId;
	uint32 _baseNamedPointId;
	uint32 _namedPointIds[32];
	uint32 _objectId;
	Common::Point _position;
	BubbleStyle() : _count(0) {}
};

struct BubbleIcon {
	uint32 _objectId;
	bool _enabled;
	Common::Point _position;
	uint32 _sequenceId;
};

class BbdouBubble {
public:
	BbdouBubble(IllusionsEngine_BBDOU *vm, BbdouSpecialCode *bbdou);
	~BbdouBubble();
	void init();
	void addBubbleStyle(uint32 showSequenceId, uint32 hideSequenceId, uint32 progResKeywordId,
		uint32 namedPointId, int16 count, uint32 *namedPointIds);
	void show();
	void hide();
	void selectBubbleStyle(int16 minCount, Common::Point sourcePt, Common::Point destPt, uint32 progResKeywordId);
	uint32 addBubbleIcon(uint positionIndex, uint32 sequenceId);
	void calcBubbleTrail(Common::Point &sourcePt, Common::Point &destPt);
protected:
	IllusionsEngine_BBDOU *_vm;
	BbdouSpecialCode *_bbdou;
	Common::Array<BubbleStyle> _bubbleStyles;
	BubbleStyle *_currBubbleStyle;
	BubbleStyle *_showingBubbleStyle;
	BubbleStyle *_hidingBubbleStyle;
	uint32 _trailObjectIds[32];
	Common::Point _sourcePt;
	Common::Point _destPt;
	int _bubbleObjectId1;
	int _bubbleObjectId2;
	BubbleIcon _icons[32];
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_BUBBLE_H
