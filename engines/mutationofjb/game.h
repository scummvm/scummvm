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

#ifndef MUTATIONOFJB_GAME_H
#define MUTATIONOFJB_GAME_H

#include "mutationofjb/assets.h"
#include "mutationofjb/gamescreen.h"
#include "mutationofjb/tasks/taskmanager.h"

#include "common/language.h"
#include "common/ptr.h"
#include "common/random.h"
#include "common/scummsys.h"

namespace Common {
class String;
}

namespace MutationOfJB {

class Command;
class MutationOfJBEngine;
class Script;
class Room;
class SayTask;
struct GameData;
struct Door;
struct Static;
struct Bitmap;

class Game {
public:
	Game(MutationOfJBEngine *vm);
	MutationOfJBEngine &getEngine();

	Common::RandomSource &getRandomSource();
	GameData &getGameData();
	Room &getRoom();
	Script *getGlobalScript() const;
	Script *getLocalScript() const;

	void changeScene(uint8 sceneId, bool partB);
	Script *changeSceneDelayScript(uint8 sceneId, bool partB, bool runDelayedScriptStartup = false);

	bool startActionSection(ActionInfo::Action action, const Common::String &entity1Name, const Common::String &entity2Name = Common::String());

	bool isCurrentSceneMap() const;

	void update();

	GameScreen &getGameScreen();

	static uint8 colorFromString(const char *colorStr);

	TaskManager &getTaskManager();
	Assets &getAssets();

	Graphics::Screen &getScreen();

	TaskPtr getActiveSayTask() const;
	void setActiveSayTask(const TaskPtr &sayTask);

	bool loadSaveAllowed() const;

	Common::Language getLanguage() const;

	void switchToPartB();

private:
	bool loadGameData(bool partB);
	void runActiveCommand();
	void startCommand(Command *cmd);
	Script *changeSceneLoadScript(uint8 sceneId, bool partB);

	MutationOfJBEngine *_vm;
	Common::RandomSource _randomSource;

	GameData *_gameData;
	Script *_globalScript;
	Script *_localScript;
	Script *_delayedLocalScript;
	bool _runDelayedScriptStartup;
	Room *_room;
	GameScreen _gui;

	ScriptExecutionContext _scriptExecCtx;

	TaskManager _taskManager;
	Assets _assets;
	TaskPtr _activeSayTask;
};

}

#endif
