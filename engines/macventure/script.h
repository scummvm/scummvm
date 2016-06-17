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

#ifndef MACVENTURE_SCRIPT_H
#define MACVENTURE_SCRIPT_H

#include "macventure/container.h"
#include "macventure/world.h"

namespace MacVenture {
		
class Container;
class World;

enum ControlAction {
	kNoCommand = 0,
	kStartOrResume = 1,
	kClose = 2,
	kTick = 3,
	kActivateObject = 4,
	kMoveObject = 5,
	kConsume = 6,
	kExamine = 7,
	kGo = 8,
	kHit = 9,
	kOpen = 10,
	kOperate = 11,
	kSpeak = 12,
	kBabble = 13,
	kTargetName = 14,
	kDebugObject = 15
};

typedef uint32 ObjID;

class ScriptAsset {
public:
	ScriptAsset(ObjID id, Container *container); 
	~ScriptAsset() {}

	void reset();
	uint8 fecth(); 

private:

	void loadInstructions();

private:
	ObjID _id;
	Container *_container;

	Common::Array<uint8> _instructions;
	uint32 _ip; // Instruction pointer
};

struct EngineState {
	uint8 stack[0x80];
	uint8 sp;

	EngineState() {
		sp = 0x80;
	}
};

struct EngineFrame {
	ControlAction action;
	ObjID src;
	ObjID dest;
	int x;
	int y;
	EngineState state;
	Common::Array<ScriptAsset> scripts;
	uint32 familyIdx;

	bool haltedInFirst;
	bool haltedInFamily;
	bool haltedInSaves;
};

class ScriptEngine {
public:
	ScriptEngine(World *world);
	~ScriptEngine();

public:
	bool runControl(ControlAction action, ObjID source, ObjID destination, Common::Point delta); 
	bool resume(bool execAll); 
	void reset();

private:
	bool execFrame(bool execAll);
	bool loadScript(EngineFrame * frame, uint32 scriptID);
	bool resumeFunc(EngineFrame * frame); 
	bool runFunc(); 

private:
	World *_world;
	Common::Array<EngineFrame> _frames;
	Container *_scripts;
};

} // End of namespace MacVenture

#endif
