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

#ifndef WATCHMAKER_GAME_H
#define WATCHMAKER_GAME_H

#include "common/random.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/fonts.h"
#include "watchmaker/game_options.h"
#include "watchmaker/ll/ll_ffile.h"
#include "watchmaker/saveload.h"
#include "watchmaker/schedule.h"
#include "watchmaker/work_dirs.h"

namespace Watchmaker {

class sdl_wrapper;
class Renderer;

struct GameVars {
private:
	int32 CurRoom = 0;

public:
	void setCurRoomId(int32 room) {
		CurRoom = room;
	}
	int32 getCurRoomId() const {
		return CurRoom;
	}
};

class MeshModifiers;
class RoomManager;
class CameraMan;

class WGame {
	bool g_bReady = false, g_bActive = false;
	//bool g_bSkipActive = false;
	const char *CharName[32] = {};
	uint32 LoadChar;
	MeshModifiers *_meshModifiers;

public:
	Common::RandomSource *_rnd;
	Renderer *_renderer;
	WorkDirs workDirs;
	GameOptions gameOptions;
	Init init;
	sdl_wrapper *sdl;
	GameVars _gameVars;
	GameRect _gameRect;
	Fonts _fonts;
	MessageSystem _messageSystem;
	RoomManager *_roomManager;
	CameraMan *_cameraMan;
	WGame();
	~WGame();

	SRoom &getCurRoom() {
		return init.Room[_gameVars.getCurRoomId()];
	}

	Common::SharedPtr<Common::SeekableReadStream> resolveFile(const char *path, bool noFastFile = false);

	void configLoaderFlags() {
		// TODO: Add back some of the configurability from the argument parsing.
		LoadChar = 3;
		LoaderFlags = T3D_STATIC_SET0;
		LoaderFlags |= T3D_OUTDOORLIGHTS;
		LoaderFlags |= T3D_PRELOADBASE;
		LoaderFlags |= T3D_STATIC_SET1;

		if (!(LoaderFlags & T3D_DEBUGMODE)) {
			LoadChar = 3;
			LoadChar |= 16777212;
		}
	}

	int StartPlayingGame(const Common::String &LoaderName_override);
	bool LoadAndSetup(const Common::String &name, uint8 lite);
	void LoadMisc();
	void UpdateAll();
	void initCharNames();
	void CleanUpAndPostQuit();

	bool CheckAndLoadMoglieSupervisoreModel(int32 c);

	void GameLoop();

	// TODO: These might belong elsewhere after some refactoring:
	void addMeshModifier(const Common::String &name, int16 com, void *p);
	void loadMeshModifiers(Common::SeekableReadStream &stream);
};

extern WGame *_vm;

} // End of namespace Watchmaker

#endif // WATCHMAKER_GAME_H
