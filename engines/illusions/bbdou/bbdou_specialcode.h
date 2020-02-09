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

class IllusionsEngine_BBDOU;
class BbdouBubble;
class BbdouCredits;
class BbdouCursor;
class BbdouFoodCtl;
class BbdouInventory;
struct CursorData;
struct VerbState;

typedef Common::Functor1<OpCall&, void> SpecialCodeFunction;

class BbdouSpecialCode;

struct Struct10 {
	uint32 _verbId;
	uint32 _sequenceId1;
	uint32 _sequenceId2;
	uint32 _sequenceId3;
};

class CauseThread_BBDOU : public Thread {
public:
	CauseThread_BBDOU(IllusionsEngine_BBDOU *vm, uint32 threadId, uint32 callingThreadId,
		BbdouSpecialCode *bbdou, uint32 cursorObjectId, uint32 sceneId,
		uint32 verbId, uint32 objectId2, uint32 objectId);
	void onNotify() override;
	void onTerminated() override;
public:
	BbdouSpecialCode *_bbdou;
	uint32 _cursorObjectId;
	uint32 _sceneId;
	uint32 _verbId;
	uint32 _objectId2;
	uint32 _objectId;
};

struct RadarMicrophoneZone {
	int16 _x;
	uint32 _threadId;
};

class RadarMicrophoneThread : public Thread {
public:
	RadarMicrophoneThread(IllusionsEngine_BBDOU *vm, uint32 threadId,
		uint32 callingThreadId, uint32 cursorObjectId);
	int onUpdate() override;
	void addZone(uint32 threadId);
	void initZones();
public:
	uint32 _cursorObjectId;
	uint _zonesCount;
	uint _currZoneIndex;
	RadarMicrophoneZone _zones[8];
};

struct ShooterStatus {
	int gridX;
	bool flag;
};

struct ObjectInteractMode {
	uint32 _objectId;
	int _interactMode;
	ObjectInteractMode() : _objectId(0), _interactMode(0) {}
};

class ObjectInteractModeMap {
public:
	ObjectInteractModeMap();
	void setObjectInteractMode(uint32 objectId, int value);
	int getObjectInteractMode(uint32 objectId);
protected:
	ObjectInteractMode _objectVerbs[512];
};

class BbdouSpecialCode : public SpecialCode {
public:
	BbdouSpecialCode(IllusionsEngine_BBDOU *vm);
	~BbdouSpecialCode() override;
	void init() override;
	void run(uint32 specialCodeId, OpCall &opCall) override;
	void resetBeforeResumeSavegame() override;
public:
	typedef Common::HashMap<uint32, SpecialCodeFunction*> Map;
	typedef Map::iterator MapIterator;
	IllusionsEngine_BBDOU *_vm;
	Map _map;
	BbdouCursor *_cursor;
	BbdouBubble *_bubble;
	BbdouInventory *_inventory;

	BbdouCredits *_credits;

	// Salad
	uint _saladCount;
	uint32 _saladObjectIds[12];

	// Shooter
	ShooterStatus _shooterStatus[2];
	uint _shooterObjectIdIndex;

	BbdouFoodCtl *_foodCtl;

	ObjectInteractModeMap _objectInteractModeMap;

	// Special code interface functions
	void spcInitCursor(OpCall &opCall);
	void spcEnableCursor(OpCall &opCall);
	void spcDisableCursor(OpCall &opCall);
	void spcAddCursorSequence(OpCall &opCall);
	void spcCursorStartHoldingObjectId(OpCall &opCall);
	void spcCursorStopHoldingObjectId(OpCall &opCall);
	void spcSetCursorState(OpCall &opCall);
	void spcInitBubble(OpCall &opCall);
	void spcSetupBubble(OpCall &opCall);
	void spcSetObjectInteractMode(OpCall &opCall);
	void spcInitInventory(OpCall &opCall);
	void spcClearInventory(OpCall &opCall);
	void spcRegisterInventoryBag(OpCall &opCall);
	void spcRegisterInventorySlot(OpCall &opCall);
	void spcRegisterInventoryItem(OpCall &opCall);
	void spcOpenInventory(OpCall &opCall);
	void spcAddInventoryItem(OpCall &opCall);
	void spcRemoveInventoryItem(OpCall &opCall);
	void spcHasInventoryItem(OpCall &opCall);
	void spcCloseInventory(OpCall &opCall);
	void spcInitConversation(OpCall &opCall);
	void spcClearConversation(OpCall &opCall);
	void spcClearBlockCounter(OpCall &opCall);
	void spcResetCursor(OpCall &opCall);
	void spcSetCursorField90(OpCall &opCall);
	void spcFoodCtl(OpCall &opCall);
	void spcTestFoodCtl(OpCall &opCall);
	void spcInitMenu(OpCall &opCall);
	void spcIsCursorHoldingObjectId(OpCall &opCall);
	void spcInitRadarMicrophone(OpCall &opCall);
	void spcCreditsCtl(OpCall &opCall);
	void spcSaladCtl(OpCall &opCall);
	void spcRunCause(OpCall &opCall);

	void playSoundEffect(int soundIndex);
	void hideVerbBubble(uint32 objectId, VerbState *verbState);
	void startHoldingObjectId(uint32 objectId1, uint32 holdingObjectId, bool doPlaySound);
	void stopHoldingObjectId(uint32 objectId1, bool doPlaySound);
	bool isHoldingObjectId(uint32 objectId);

protected:
	// Internal functions
	bool testInteractModeRange(int value);
	void setCursorControlRoutine(uint32 objectId, int num);
	Common::Point getBackgroundCursorPos(Common::Point cursorPos);
	void showBubble(uint32 objectId, uint32 overlappedObjectId, uint32 holdingObjectId,
		VerbState *verbState, uint32 progResKeywordId);
	bool findVerbId(VerbState *verbState, uint32 currOverlappedObjectId, int always0, uint32 &outVerbId);
	void cursorInteractControlRoutine(Control *cursorControl, uint32 deltaTime);
	void cursorCrosshairControlRoutine(Control *cursorControl, uint32 deltaTime);
	bool testVerbId(uint32 verbId, uint32 holdingObjectId, uint32 overlappedObjectId);
	bool getCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId,
		uint32 &outVerbId, uint32 &outObjectId2, uint32 &outObjectId);
	bool runCause(Control *cursorControl, CursorData &cursorData,
		uint32 verbId, uint32 objectId2, uint32 objectId, int soundIndex);
	uint32 startCauseThread(uint32 cursorObjectId, uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId);
	// Salad
	void initSalad();
	void addSalad(uint32 sequenceId);
	// Shooter
	bool getShooterCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId,
		uint32 &outSceneId, uint32 &outVerbId, uint32 &outObjectId2, uint32 &outObjectId);
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_SPECIALCODE_H
