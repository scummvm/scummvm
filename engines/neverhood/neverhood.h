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

#ifndef NEVERHOOD_NEVERHOOD_H
#define NEVERHOOD_NEVERHOOD_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"
#include "engines/engine.h"
#include "gui/debugger.h"
#include "neverhood/console.h"
#include "neverhood/messages.h"

namespace Neverhood {

enum NeverhoodGameFeatures {
};

struct NeverhoodGameDescription;

class GameModule;
class GameVars;
class ResourceMan;
class Screen;
class SoundMan;
class AudioResourceMan;
class StaticData;
struct NPoint;

struct GameState {
	int sceneNum;
	int which;
};

class NeverhoodEngine : public ::Engine {
protected:

	Common::Error run();
	void mainLoop();

public:
	NeverhoodEngine(OSystem *syst, const NeverhoodGameDescription *gameDesc);
	virtual ~NeverhoodEngine();

	// Detection related functions
	const NeverhoodGameDescription *_gameDescription;
	const char *getGameId() const;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	Common::Language getLanguage() const;
	bool hasFeature(EngineFeature f) const;
	bool isDemo() const;
	bool applyResourceFixes() const;
	Common::String getTargetName() { return _targetName; };

	Common::RandomSource *_rnd;

	int16 _mouseX, _mouseY;
	uint16 _buttonState;

	GameState _gameState;
	GameVars *_gameVars;
	Screen *_screen;
	ResourceMan *_res;
	GameModule *_gameModule;
	StaticData *_staticData;
	Console *_console;
	GUI::Debugger *getDebugger() { return _console; }

	SoundMan *_soundMan;
	AudioResourceMan *_audioResourceMan;

public:

	/* Save/load */

	enum kReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;
		uint32 saveDate;
		uint32 saveTime;
		uint32 playTime;
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }

	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &description);
	Common::Error removeGameState(int slot);
	bool savegame(const char *filename, const char *description);
	bool loadgame(const char *filename);
	const char *getSavegameFilename(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	WARN_UNUSED_RESULT static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail = true);

	GameState& gameState() { return _gameState; }
	GameModule *gameModule() { return _gameModule; }
	int16 getMouseX() const { return _mouseX; }
	int16 getMouseY() const { return _mouseY; }
	NPoint getMousePos();

	void toggleSoundUpdate(bool state) { _updateSound = state; }
	void toggleMusic(bool state) { _enableMusic = state; }
	bool musicIsEnabled() { return _enableMusic; }

private:
	bool _updateSound;
	bool _enableMusic;

};

} // End of namespace Neverhood

#endif /* NEVERHOOD_NEVERHOOD_H */
