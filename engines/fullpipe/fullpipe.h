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

#ifndef FULLPIPE_H
#define FULLPIPE_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/system.h"

#include "audio/mixer.h"

#include "graphics/surface.h"

#include "engines/engine.h"

struct ADGameDescription;

namespace Fullpipe {

enum FullpipeGameFeatures {
};

class CGameLoader;
class GameProject;
class CGameVar;
class CInventory2;
class Scene;
class NGIArchive;

class FullpipeEngine : public ::Engine {
protected:

	Common::Error run();

public:
	FullpipeEngine(OSystem *syst, const ADGameDescription *gameDesc);
	virtual ~FullpipeEngine();

	// Detection related functions
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	Common::Platform getPlatform() const;
	bool hasFeature(EngineFeature f) const;

	Common::RandomSource *_rnd;

	int _mouseX, _mouseY;
	Common::KeyCode _keyState;
	uint16 _buttonState;

	void updateEvents();

	Graphics::Surface _backgroundSurface;

	CGameLoader *_gameLoader;
	GameProject *_gameProject;
	bool loadGam(const char *fname);

	int _gameProjectVersion;
	int _pictureScale;
	int _scrollSpeed;
	int _currSoundListCount;
	bool _soundEnabled;
	bool _flgSoundList;

	bool _needQuit;

	void initObjectStates();
	void setLevelStates();
	void setSwallowedEggsState();

	CGameVar *_swallowedEgg1;
	CGameVar *_swallowedEgg2;
	CGameVar *_swallowedEgg3;

	CInventory2 *_inventory;

	void setObjectState(const char *name, int state);
	int getObjectEnumState(const char *name, const char *state);

	Scene *accessScene(int sceneId);

	NGIArchive *_currArchive;

public:

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }

};

extern FullpipeEngine *g_fullpipe;

} // End of namespace Fullpipe

#endif /* FULLPIPE_H */
