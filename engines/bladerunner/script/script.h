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

protected:

	void Game_Flag_Set(int flag);
	void Game_Flag_Reset(int flag);
	bool Game_Flag_Query(int flag);

	int Global_Variable_Set(int, int);
	int Global_Variable_Reset(int);
	int Global_Variable_Query(int);
	int Global_Variable_Increment(int, int);
	int Global_Variable_Decrement(int, int);

	void Sound_Play(int id, int volume, int panFrom, int panTo, int priority);

	void Scene_Loop_Set_Default(int);
	void Scene_Loop_Start_Special(int, int, int);

	void Outtake_Play(int id, int noLocalization = false, int container = -1);
	void Ambient_Sounds_Add_Sound(int id, int time1, int time2, int volume1, int volume2, int pan1begin, int pan1end, int pan2begin, int pan2end, int priority, int unk);
	// Ambient_Sounds_Remove_Sound
	// Ambient_Sounds_Add_Speech_Sound
	// Ambient_Sounds_Remove_Speech_Sound
	// Ambient_Sounds_Play_Sound
	// Ambient_Sounds_Play_Speech_Sound
	void Ambient_Sounds_Remove_All_Non_Looping_Sounds(int time);
	void Ambient_Sounds_Add_Looping_Sound(int id, int volume, int pan, int fadeInTime);
	// Ambient_Sounds_Adjust_Looping_Sound
	// Ambient_Sounds_Remove_Looping_Sound
	void Ambient_Sounds_Remove_All_Looping_Sounds(int time);
	void Setup_Scene_Information(float actorX, float actorY, float actorZ, int actorFacing);

	void Scene_Exit_Add_2D_Exit(int, int, int, int, int, int);
	void Scene_2D_Region_Add(int, int, int, int, int);

	void I_Sez(const char *str);
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
