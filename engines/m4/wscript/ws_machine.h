
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

#ifndef M4_WSCRIPT_WS_MACHINE_H
#define M4_WSCRIPT_WS_MACHINE_H

#include "m4/m4_types.h"
#include "m4/gui/gui.h"

namespace M4 {

// A message request
struct msgRequest {
	msgRequest *nextMsg = nullptr;
	uint32 msgHash = 0;
	frac16 msgValue = 0;
	int32 pcOffset = 0;
	int32 pcCount = 0;
};

struct machine;
typedef void (*MessageCB)(frac16 myMessage, struct machine *sender);

//timebase request structure.
struct onTimeReq {
	onTimeReq *next = nullptr;
	int32 myTime = 0;
	struct machine *myXM = nullptr;
	int32 pcOffset = 0;
	int32 pcCount = 0;
};

// rails algorithm struct
struct railNode {
	uint8 nodeID = 0;
	int32 x = 0, y = 0;
	railNode *shortPath = nullptr;
	int32 pathWeight = 0;
};

struct CCB {
	uint32 flags = 0;
	M4sprite *source = nullptr;
	M4Rect *currLocation = nullptr;
	M4Rect *newLocation = nullptr;
	M4Rect *maxArea = nullptr;
	int32 scaleX = 0;
	int32 scaleY = 0;
	int32 layer = 0;
	uint32 *streamSSHeader = nullptr;
	uint32 *streamSpriteSource = nullptr;
	void *myStream = nullptr;
	char *seriesName = nullptr;
};

#define JSR_STACK_MAX	8
struct Anim8 {
	machine *myMachine = nullptr;	// Pointer back to myMachine
	int32 eosReqOffset = 0;			// The machine PC offset to be executed at the EOS
	int32 eosReqCount = 0;
	Anim8 *next = nullptr;			// The linked list used for execution order
	Anim8 *prev = nullptr;
	int32 myLayer = 0;
	Anim8 *infront = nullptr;		// The linked list used for layering
	Anim8 *behind = nullptr;
	Anim8 *myParent = nullptr;		// The parent anim8
	int32 sequHash = 0;				// The current sequence Hash = 0;
	Handle sequHandle = nullptr;	// The sequence Handle
	int32 pcOffset = 0;				// The offset into the sequence of the current PC
	CCB *myCCB = nullptr;
	int32 dataHash = 0;				// The array of data
	Handle dataHandle = nullptr;
	int32 dataOffset = 0;
	int32 startTime = 0;
	int32 switchTime = 0;
	frac16 transTime = 0;
	int32 flags = 0;
	frac16 start_s = 0;
	frac16 start_r = 0;
	frac16 start_x = 0;
	frac16 start_y = 0;
	int32 numLocalVars = 0;
	frac16 *myRegs = nullptr;
	bool active = false;
	int32 returnStackIndex = 0;
	uint32 returnHashes[JSR_STACK_MAX] = { 0 };
	int32 returnOffsets[JSR_STACK_MAX] = { 0 };
};

struct machine {
	machine *next = nullptr;
	machine *prev = nullptr;
	uint32 myHash = 0;
	uint32 machID = 0;
	char *machName = nullptr;
	Handle machHandle = 0;
	int32 machInstrOffset = 0;
	int32 stateTableOffset = 0;
	int32 curState = 0;
	int32 numOfStates = 0;
	uint32 recurseLevel = 0;
	Anim8 *myAnim8 = nullptr;
	Anim8 *parentAnim8 = nullptr;
	int32 dataHash = 0;
	Handle dataHandle = 0;
	int32 dataOffset = 0;
	int32 targetCount = 0;
	struct machine *msgReplyXM = nullptr;
	MessageCB CintrMsg;
	msgRequest *myMsgs = nullptr;
	msgRequest *myPersistentMsgs = nullptr;
	msgRequest *usedPersistentMsgs = nullptr;
	railNode *walkPath = nullptr;
};

struct globalMsgReq {
	struct globalMsgReq *next = nullptr;
	ulong msgHash = 0;
	frac16 msgValue = 0;
	ulong machHash = 0;
	machine *sendM = nullptr;
	int32 msgCount = 0;
};

struct WSMachine_Globals {
	int32 _pauseTime = 0;
	int32 _oldTime = 0;
	bool _enginesPaused = false;

	int32 *_dataFormats = nullptr;
	uint32	_machineIDCount = 0;
	machine *_firstMachine = nullptr;

	machine *_nextXM = nullptr;
	globalMsgReq *_myGlobalMessages = nullptr;

	// Used for processing pCodes
	frac16 *_ws_globals = nullptr;
};

extern bool ws_Initialize(frac16 *theGlobals);
extern void ws_Shutdown();
extern void TerminateMachinesByHash(int32 machHash);

} // End of namespace M4

#endif
