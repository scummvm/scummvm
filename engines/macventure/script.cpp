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

#include "macventure/macventure.h"
#include "macventure/script.h"
#include "macventure/world.h"
#include "macventure/container.h"

namespace MacVenture {	
	
ScriptEngine::ScriptEngine(World * world) {
	_world = world;
	_scripts = new Container("Shadowgate II/Shadow Filter");
}

ScriptEngine::~ScriptEngine() {
	if (_scripts)
		delete _scripts;
}

bool ScriptEngine::runControl(ControlAction action, ObjID source, ObjID destination, Common::Point delta) {
	//debug(7, "SCRIPT: Running control %d from obj %d into obj %d, at delta (%d, %d)", 
	//	action, source, destination, delta.x, delta.y);

	EngineFrame frame;
	frame.action = action;
	frame.src = source;
	frame.dest = destination;
	frame.x = delta.x;
	frame.y = delta.y;
	frame.haltedInSaves = false;
	frame.haltedInFirst = false;
	frame.haltedInFamily = false;
	_frames.push_back(frame);
	debug(7, "SCRIPT: Stored frame %d, action: %d src: %d dest: %d point: (%d, %d)",
		_frames.size() - 1, frame.action, frame.src, frame.dest, frame.x, frame.y);

	return resume(true);
}

bool ScriptEngine::resume(bool execAll) {	
	debug(7, "SCRIPT: Resume");
	while (_frames.size()) {
		bool fail = execFrame(execAll);
		if (fail) return true;
	}
	return false;	
}

void ScriptEngine::reset() {
	_frames.clear();
}

bool ScriptEngine::execFrame(bool execAll) {
	bool doFirst = execAll;
	bool doFamily = false;
	bool fail;

	EngineFrame *frame = &_frames.front();

	// Do first dispatch script (script 0)
	if (frame->haltedInFirst || doFirst) { // We were stuck or it's the first time
		frame->haltedInFirst = false;
		if (doFirst) { fail = loadScript(frame, 0); }
		else { fail = resumeFunc(frame); }
		if (fail) {
			frame->haltedInFirst = true;
			return true;
		}
		doFamily = true;
		frame->familyIdx = 0;
	}

	// Do scripts in the family of player (ObjID 1)
	if (frame->haltedInFamily || doFamily) { // We have to do the family or we were stuck here
		frame->haltedInFamily = false;
		Common::Array<ObjID> family = _world->getFamily(_world->getObjAttr(1, kAttrParentObject), false);
		uint32 i = frame->familyIdx;
		for (; i < family.size(); i++) {
			if (doFamily) { fail = loadScript(frame, family[i]); }
			else { fail = resumeFunc(frame); }
			if (fail) { // We are stuck, so we don't shift the frame
				frame->haltedInFamily = true;
				frame->familyIdx = i;
				return true;
			}
			doFamily = true;
		}
	}

	//Handle saves
	/*

	uint highest;
	uint high;
	if (frame->haltedInSaves) {
	frame->haltedInSaves = false;
	}


	do {
	highest = 0;
	for (uint i = 0; i < frame->haltedInSaves.size)
	}
	*/

	_frames.remove_at(0);
	return false;
}

bool ScriptEngine::loadScript(EngineFrame * frame, uint32 scriptID) {
	frame->scripts.push_back(ScriptAsset(scriptID, _scripts));
	return false;
}

bool ScriptEngine::resumeFunc(EngineFrame * frame) {
	bool fail = runFunc();
	if (fail) return fail;
	frame->scripts.remove_at(0);
	if (frame->scripts.size())
		return resumeFunc(frame);
	return false;
}

bool ScriptEngine::runFunc() {
	debug(7, "SCRIPT: I'm running the function");
	return false;
}

ScriptAsset::ScriptAsset(ObjID id, Container * container) {	
	_id = id;
	_container = container;
	_ip = 0x0;	
}

void ScriptAsset::reset() {
	_ip = 0x0;
}

uint8 ScriptAsset::fecth() {
	uint8 ins = _instructions[_ip];
	_ip++;
	return ins;
}

void ScriptAsset::loadInstructions() {
	uint32 amount = _container->getItemByteSize(_id);
	Common::SeekableReadStream *res = _container->getItem(_id);
	for (uint i = 0; i < amount; i++) {
		_instructions.push_back(res->readByte());
	}
	debug(7, "SCRIPT: Load %d instructions for script %d", amount, _id);
}

} // End of namespace MacVenture