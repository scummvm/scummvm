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
#include "common/algorithm.h"
#include "common/stack.h"

namespace Illusions {

class IllusionsEngine;

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
public:

	IllusionsEngine *_vm;
	ActiveScenes _activeScenes;
	ScriptStack _stack;
	
	uint32 _theSceneId;
	uint32 _theThreadId;
	
};

} // End of namespace Illusions

#endif // ILLUSIONS_SCRIPTMAN_H
