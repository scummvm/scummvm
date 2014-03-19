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

#ifndef ILLUSIONS_SCRIPTMAN_H
#define ILLUSIONS_SCRIPTMAN_H

#include "illusions/scriptresource.h"
#include "illusions/thread.h"
#include "common/algorithm.h"
#include "common/stack.h"

namespace Illusions {

class IllusionsEngine;
class ScriptOpcodes;

struct ActiveScene {
	uint32 _sceneId;
	int _pauseCtr;
};

class ActiveScenes {
public:
	ActiveScenes();
	void clear();
	void push(uint32 sceneId);
	void pop();
	void pauseActiveScene();
	void unpauseActiveScene();
	int getActiveScenesCount();
	void getActiveSceneInfo(uint index, uint32 *sceneId, int *pauseCtr);
	uint32 getCurrentScene();
	bool isSceneActive(uint32 sceneId);
protected:
	Common::FixedStack<ActiveScene, 16> _stack;
};

class ScriptStack {
public:
	ScriptStack();
	void clear();
	void push(int16 value);
	int16 pop();
	int16 peek();
protected:
	int _stackPos;
	int16 _stack[256];
};

class ScriptMan {
public:
	ScriptMan(IllusionsEngine *vm);
	~ScriptMan();
	void setSceneIdThreadId(uint32 theSceneId, uint32 theThreadId);
	void startScriptThread(uint32 threadId, uint32 callingThreadId,
		uint32 value8, uint32 valueC, uint32 value10);
	void startAnonScriptThread(int32 threadId, uint32 callingThreadId,
		uint32 value8, uint32 valueC, uint32 value10);
	uint32 startTempScriptThread(byte *scriptCodeIp, uint32 callingThreadId,
		uint32 value8, uint32 valueC, uint32 value10);
	uint32 startAbortableTimerThread(uint32 duration, uint32 threadId);
	uint32 startTimerThread(uint32 duration, uint32 threadId);
	void setCurrFontId(uint32 fontId);
	bool enterScene(uint32 sceneId, uint32 threadId);
public:

	IllusionsEngine *_vm;
	ScriptResource *_scriptResource;

	ActiveScenes _activeScenes;
	ScriptStack _stack;
	
	int _pauseCtr;
	
	uint32 _theSceneId;
	uint32 _theThreadId;
	bool _doScriptThreadInit;
	uint32 _nextTempThreadId;
	
	uint32 _fontId;
	
	ThreadList *_threads;
	ScriptOpcodes *_scriptOpcodes;
	
	void newScriptThread(uint32 threadId, uint32 callingThreadId, uint notifyFlags,
		byte *scriptCodeIp, uint32 value8, uint32 valueC, uint32 value10);
	uint32 newTimerThread(uint32 duration, uint32 callingThreadId, bool isAbortable);
	uint32 newTempThreadId();

};

} // End of namespace Illusions

#endif // ILLUSIONS_SCRIPTMAN_H
