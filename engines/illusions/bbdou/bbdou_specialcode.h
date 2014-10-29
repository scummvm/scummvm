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

#ifndef ILLUSIONS_BBDOU_BBDOU_SPECIALCODE_H
#define ILLUSIONS_BBDOU_BBDOU_SPECIALCODE_H

#include "illusions/specialcode.h"
#include "illusions/thread.h"
#include "common/hashmap.h"

namespace Illusions {

class IllusionsEngine;
class BbdouBubble;
class BbdouCursor;
class BbdouInventory;
struct CursorData;
struct Item10;

typedef Common::Functor1<OpCall&, void> SpecialCodeFunction;

class BbdouSpecialCode;

struct Struct10 {
	uint32 _verbId;
	uint32 _sequenceId1;
	uint32 _sequenceId2;
	uint32 _sequenceId3;
};

class CauseThread : public Thread {
public:
	CauseThread(IllusionsEngine *vm, uint32 threadId, uint32 callingThreadId,
		BbdouSpecialCode *bbdou, uint32 cursorObjectId, uint32 sceneId,
		uint32 verbId, uint32 objectId2, uint32 objectId);
	virtual void onNotify();
	virtual void onTerminated();
public:
	BbdouSpecialCode *_bbdou;
	uint32 _cursorObjectId;
	uint32 _sceneId;
	uint32 _verbId;
	uint32 _objectId2;
	uint32 _objectId;
};

class BbdouSpecialCode : public SpecialCode {
public:
	BbdouSpecialCode(IllusionsEngine *vm);
	virtual ~BbdouSpecialCode();
	virtual void init();
	virtual void run(uint32 specialCodeId, OpCall &opCall);
public:
	typedef Common::HashMap<uint32, SpecialCodeFunction*> Map;
	typedef Map::iterator MapIterator;
	Map _map;
	BbdouCursor *_cursor;
	BbdouBubble *_bubble;
	BbdouInventory *_inventory;
	// Special code interface functions
	void spcInitCursor(OpCall &opCall);
	void spcEnableCursor(OpCall &opCall);
	void spcDisableCursor(OpCall &opCall);
	void spcAddCursorSequence(OpCall &opCall);
	void spcCursorStartHoldingObjectId(OpCall &opCall);
	void spcCursorStopHoldingObjectId(OpCall &opCall);
	void spcInitBubble(OpCall &opCall);
	void spcSetupBubble(OpCall &opCall);
	void spcSetObjectInteractMode(OpCall &opCall);
	void spcRegisterInventoryBag(OpCall &opCall);
	void spcRegisterInventorySlot(OpCall &opCall);
	void spcRegisterInventoryItem(OpCall &opCall);
	void spcOpenInventory(OpCall &opCall);
	void spcAddInventoryItem(OpCall &opCall);
	void spcRemoveInventoryItem(OpCall &opCall);
	void spcHasInventoryItem(OpCall &opCall);
	void spcCloseInventory(OpCall &opCall);

	void playSoundEffect(int soundIndex);
	void resetItem10(uint32 objectId, Item10 *item10);
	void startHoldingObjectId(uint32 objectId1, uint32 holdingObjectId, bool doPlaySound);
	void stopHoldingObjectId(uint32 objectId1, bool doPlaySound);

protected:
	// Internal functions
	bool testValueRange(int value);
	void setCursorControlRoutine(uint32 objectId, int num);
	Common::Point getBackgroundCursorPos(Common::Point cursorPos);
	void showBubble(uint32 objectId, uint32 overlappedObjectId, uint32 holdingObjectId,
		Item10 *item10, uint32 progResKeywordId);
	bool findVerbId(Item10 *item10, uint32 currOverlappedObjectId, int always0, uint32 &outVerbId);
	void cursorInteractControlRoutine(Control *cursorControl, uint32 deltaTime);
	void cursorControlRoutine2(Control *cursorControl, uint32 deltaTime);
	bool testVerbId(uint32 verbId, uint32 holdingObjectId, uint32 overlappedObjectId);
	bool getCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId,
		uint32 &outVerbId, uint32 &outObjectId2, uint32 &outObjectId);
	bool runCause(Control *cursorControl, CursorData &cursorData,
		uint32 verbId, uint32 objectId2, uint32 objectId, int soundIndex);
	uint32 startCauseThread(uint32 cursorObjectId, uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId);
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_SPECIALCODE_H
