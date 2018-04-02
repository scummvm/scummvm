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

#include "common/scummsys.h"
#include "mutationofjb/script.h"

namespace Common {
class String;
}

namespace MutationOfJB {

class Command;
class MutationOfJBEngine;
class GameData;
class Script;
class Room;
class Door;
class Static;
class Bitmap;

class Game {
public:
	Game(MutationOfJBEngine *vm);
	GameData &getGameData();
	Room &getRoom();
	Script *getGlobalScript() const;
	Script *getLocalScript() const;

	void changeScene(uint8 sceneId, bool partB);
	Script *changeSceneDelayScript(uint8 sceneId, bool partB);

	bool startActionSection(ActionInfo::Action action, const Common::String &entity1Name, const Common::String &entity2Name = Common::String());

	bool isCurrentSceneMap() const;

	void update();

private:
	bool loadGameData(bool partB);
	void runActiveCommand();
	void startCommand(Command *cmd);
	Script *changeSceneLoadScript(uint8 sceneId, bool partB);

	MutationOfJBEngine *_vm;

	GameData *_gameData;
	Script *_globalScript;
	Script *_localScript;
	Script *_delayedLocalScript;
	Room *_room;

	ScriptExecutionContext _scriptExecCtx;
};

}

#endif
