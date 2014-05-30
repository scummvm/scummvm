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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_SCRIPT_H
#define BLADERUNNER_SCRIPT_H

#include "common/str.h"

namespace BladeRunner {

class BladeRunnerEngine;
class ScriptBase;

class Script {
public:
	BladeRunnerEngine *_vm;
	int                _inScriptCounter;
	ScriptBase        *_currentScript;

	Script(BladeRunnerEngine *vm)
		: _vm(vm),
		  _inScriptCounter(0),
		  _currentScript(nullptr)
	{}
	~Script();

	bool open(const Common::String &name);

	void InitializeScene();
	void SceneLoaded();
	void SceneFrameAdvanced(int frame);
};

class ScriptBase {
protected:
	BladeRunnerEngine *_vm;

	ScriptBase(BladeRunnerEngine *vm)
		: _vm(vm)
	{}

public:
	virtual ~ScriptBase()
	{}

	virtual void InitializeScene() = 0;
	virtual void SceneLoaded() = 0;
	virtual void SceneFrameAdvanced(int frame) = 0;
};

#define DECLARE_SCRIPT(name) \
class Script##name : public ScriptBase { \
public: \
	Script##name(BladeRunnerEngine *vm) \
		: ScriptBase(vm) \
	{} \
	void InitializeScene(); \
	void SceneLoaded(); \
	void SceneFrameAdvanced(int frame); \
};

DECLARE_SCRIPT(RC01)

#undef DECLARE_SCRIPT

} // End of namespace BladeRunner

#endif
