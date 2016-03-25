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

class IllusionsEngine_BBDOU;
class BbdouSpecialCode;
class Control;

struct VerbState {
	int _cursorState;
	bool _verbActive[32];
	uint32 _verbId;
	bool _isBubbleVisible;
	uint32 _objectIds[2];
	int16 _index;
	bool _flag56;
	int _minPriority;
};

struct CursorData {
	int _mode;
	int _mode2;
	uint32 _verbId1;
	uint32 _progResKeywordId;
	VerbState _verbState;
	uint32 _currOverlappedObjectId;
	uint32 _overlappedObjectId;
	uint32 _sequenceId;
	uint32 _sequenceId2;
	uint32 _holdingObjectId;
	uint32 _holdingObjectId2;
	int _visibleCtr;
	uint32 _causeThreadId1;
	uint32 _causeThreadId2;
	int16 _field90;
	uint _flags;
	uint32 _currCursorTrackingSequenceId;
	int16 _idleCtr;
};

struct CursorSequence {
	uint32 _objectId;
	uint32 _sequenceId;
	CursorSequence() : _objectId(0), _sequenceId(0) {}
};

const uint kMaxCursorSequences = 100;

class BbdouCursor {
public:
	BbdouCursor(IllusionsEngine_BBDOU *vm, BbdouSpecialCode *bbdou);
	~BbdouCursor();
	void init(uint32 objectId, uint32 progResKeywordId);
	void enable(uint32 objectId);
	void disable(uint32 objectId);
	void reset(uint32 objectId);
	void addCursorSequenceId(uint32 objectId, uint32 sequenceId);
	uint32 findCursorSequenceId(uint32 objectId);
	bool updateTrackingCursor(Control *control);
	void saveInfo();
	void restoreInfo();
	void saveBeforeTrackingCursor(Control *control, uint32 sequenceId);
	void restoreAfterTrackingCursor();
	uint32 getSequenceId1(int sequenceIndex);
	uint calcTrackingFlags(Common::Point actorPos, Common::Point trackingLimits);
	uint calcTrackingCursorIndex(uint trackingFlags);
	bool getTrackingCursorSequenceId(Control *control, uint32 &outSequenceId);
public:
	IllusionsEngine_BBDOU *_vm;
	BbdouSpecialCode *_bbdou;
	Control *_control;
	CursorData _data;
	CursorSequence _cursorSequences[kMaxCursorSequences];
	void resetActiveVerbs();
	void show(Control *control);
	void hide(uint32 objectId);
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_CURSOR_H
