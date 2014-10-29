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

#ifndef ILLUSIONS_BBDOU_BBDOU_CURSOR_H
#define ILLUSIONS_BBDOU_BBDOU_CURSOR_H

#include "illusions/specialcode.h"

namespace Illusions {

class IllusionsEngine;
class BbdouSpecialCode;
class Control;
struct Item10;

struct Item10 {
	int _field0;
	int16 _verbActive[32];
	uint32 _verbId;
	int16 _playSound48;
	//field_4A dw
	uint32 _objectIds[2];
	int16 _index;
	int16 _flag56;
	int _field58;
};

struct CursorData {
	int _mode;
	int _mode2;
	uint32 _verbId1;
	uint32 _progResKeywordId;
	Item10 _item10;
	uint32 _currOverlappedObjectId;
	uint32 _overlappedObjectId;
	uint32 _sequenceId;
	uint32 _sequenceId2;
	uint32 _holdingObjectId;
	uint32 _holdingObjectId2;
	int _visibleCtr;
	//field_86 dw
	uint32 _causeThreadId1;
	uint32 _causeThreadId2;
	int16 _field90;
	//field_92 dw
	uint _flags;
	uint32 _sequenceId98;
	int16 _idleCtr;
	//field_9E db
	//field_9F db
};

struct CursorSequence {
	uint32 _objectId;
	uint32 _sequenceId;
	CursorSequence() : _objectId(0), _sequenceId(0) {}
};

struct Struct8b {
	uint32 _objectId;
	int _value;
	Struct8b() : _objectId(0), _value(0) {}
};

const uint kMaxCursorSequences = 100;

class BbdouCursor {
public:
	BbdouCursor(IllusionsEngine *vm, BbdouSpecialCode *bbdou);
	~BbdouCursor();
	void init(uint32 objectId, uint32 progResKeywordId);
	void enable(uint32 objectId);
	void disable(uint32 objectId);
	void addCursorSequence(uint32 objectId, uint32 sequenceId);
	uint32 findCursorSequenceId(uint32 objectId);
	void setStruct8bsValue(uint32 objectId, int value);
	int findStruct8bsValue(uint32 objectId);
	void saveInfo();
	void restoreInfo();
	void restoreAfterTrackingCursor();
	uint32 getSequenceId1(int sequenceIndex);
public:
	IllusionsEngine *_vm;
	BbdouSpecialCode *_bbdou;
	Control *_control;
	CursorData _data;
	CursorSequence _cursorSequences[kMaxCursorSequences];
	Struct8b _cursorStruct8bs[512];
	void clearCursorDataField14();
	void show(Control *control);
	void hide(uint32 objectId);
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_CURSOR_H
