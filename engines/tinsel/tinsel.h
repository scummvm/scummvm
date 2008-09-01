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
 * $URL$
 * $Id$
 *
 */

#ifndef TINSEL_H
#define TINSEL_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/util.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "engines/engine.h"
#include "tinsel/debugger.h"
#include "tinsel/graphics.h"
#include "tinsel/sound.h"

namespace Tinsel {

class MusicPlayer;
class Scheduler;
class SoundManager;

enum TinselGameID {
	GID_DW1 = 0,
	GID_DW2 = 1
};

enum TinselGameFeatures {
	GF_DEMO = 1 << 0,
	GF_CD = 1 << 1,
	GF_FLOPPY = 1 << 2,
	GF_SCNFILES = 1 << 3,

	// The GF_USE_?FLAGS values specify how many country flags are displayed
	// in the subtitles options dialog.
	// None of these defined -> 1 language, in ENGLISH.TXT
	GF_USE_3FLAGS = 1 << 4,	// French, German, Spanish
	GF_USE_4FLAGS = 1 << 5,	// French, German, Spanish, Italian
	GF_USE_5FLAGS = 1 << 6	// All 5 flags
};

enum TinselEngineVersion {
	TINSEL_V0 = 0,	// Used in the DW1 demo only
	TINSEL_V1 = 1
};

struct TinselGameDescription;

enum TinselKeyDirection {
	MSK_LEFT = 1, MSK_RIGHT = 2, MSK_UP = 4, MSK_DOWN = 8,
	MSK_DIRECTION = MSK_LEFT | MSK_RIGHT | MSK_UP | MSK_DOWN
};

typedef bool (*KEYFPTR)(const Common::KeyState &);

class TinselEngine : public Engine {
	int _gameId;
	Common::KeyState _keyPressed;
	Common::RandomSource _random;
	Graphics::Surface _screenSurface;
	Common::Point _mousePos;
	uint8 _dosPlayerDir;
	Console *_console;
	Scheduler *_scheduler;

protected:

	int init();
	int go();

public:
	TinselEngine(OSystem *syst, const TinselGameDescription *gameDesc);
	virtual ~TinselEngine();
	int getGameId() {
		return _gameId;
	}

	const TinselGameDescription *_gameDescription;
	uint32 getGameID() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;

	MidiDriver *_driver;
	SoundManager *_sound;
	MusicPlayer *_music;

	KEYFPTR _keyHandler;
private:
	//MusicPlayer *_music;
	int _musicVolume;

	void NextGameCycle(void);
	void CreateConstProcesses(void);
	void RestartGame(void);
	void RestartDrivers(void);
	void ChopDrivers(void);
	void ProcessKeyEvent(const Common::Event &event);
	bool pollEvent();

public:
	const Common::String getTargetName() const { return _targetName; }
	Common::String getSavegamePattern() const;
	Common::String getSavegameFilename(int16 saveNum) const;
	Common::SaveFileManager *getSaveFileMan() { return _saveFileMan; }
	Graphics::Surface &screen() { return _screenSurface; }

	Common::Point getMousePosition() const { return _mousePos; }
	void setMousePosition(const Common::Point &pt) {
		g_system->warpMouse(pt.x, pt.y);
		_mousePos = pt;
	}
	void divertKeyInput(KEYFPTR fptr) { _keyHandler = fptr; }
	int getRandomNumber(int maxNumber) { return _random.getRandomNumber(maxNumber); }
	uint8 getKeyDirection() const { return _dosPlayerDir; }
};

// Global reference to the TinselEngine object
extern TinselEngine *_vm;

} // End of namespace Tinsel

#endif /* TINSEL_H */
