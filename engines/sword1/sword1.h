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

#ifndef SWORD1_SWORD1_H
#define SWORD1_SWORD1_H

#include "engines/engine.h"
#include "common/error.h"
#include "common/keyboard.h"
#include "common/rect.h"
#include "common/util.h"
#include "common/events.h"
#include "sword1/sworddefs.h"
#include "sword1/console.h"

struct ADGameDescription;

/**
 * This is the namespace of the Sword1 engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Broken Sword: The Shadow of the Templars
 */

namespace Sword1 {

enum SWORD1Action {
	kActionNone,
	kActionPause,
	kActionQuit,
	kActionMainPanel,
	kActionEscape
};

enum ControlPanelMode {
	CP_NORMAL = 0,
	CP_DEATHSCREEN,
	CP_THEEND,
	CP_NEWGAME
};

class Screen;
class Sound;
class Logic;
class Mouse;
class ResMan;
class ObjectMan;
class Menu;
class Control;

struct SystemVars {
	bool             runningFromCd;
	uint32           currentCD;          // starts at zero, then either 1 or 2 depending on section being played
	uint32           justRestoredGame;   // see main() in sword.c & New_screen() in gtm_core.c
	uint8            controlPanelMode;   // 1 death screen version of the control panel, 2 = successful end of game, 3 = force restart
	uint8            saveGameFlag;
	int              snrStatus;
	bool             wantFade;           // when true => fade during scene change, else cut.
	bool             playSpeech;
	bool             textRunning;
	uint32           speechRunning;
	bool             speechFinished;
	bool             showText;
	int32            textNumber;
	uint8            language;
	bool             isDemo;
	bool             isSpanishDemo;
	Common::Platform platform;
	Common::Language realLanguage;
	bool             isLangRtl;
	bool             debugMode;
	bool             slowMode;
	bool             fastMode;
	bool             parallaxOn;
	bool             gamePaused;
	bool             displayDebugText;
	bool             displayDebugMouse;
	bool             displayDebugGrid;
	uint32           framesPerSecondCounter;
	uint32           gameCycle;
	bool             useWindowsAudioMode; // DOS and Windows use different implementations of the audio driver, each with their own behavior
};

class SwordEngine : public Engine {
	friend class SwordConsole;
	friend class Screen;
	friend class Control;

public:
	SwordEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~SwordEngine() override;
	static SystemVars _systemVars;
	void reinitialize();

	uint32 _features;

	int _inTimer = -1; // Is the timer running?
	int32 _vbl60HzUSecElapsed = 0; // 60 Hz counter for palette fades
	int _vblCount = 0; // How many vblCallback calls have been made?
	int _rate = DEFAULT_FRAME_TIME / 10;
	int _targetFrameTime = DEFAULT_FRAME_TIME;
	uint32 _mainLoopFrameCount = 0;
	uint32 _ticker = 0; // For the frame time shown within the debug text

	bool mouseIsActive();

	static bool isMac() { return _systemVars.platform == Common::kPlatformMacintosh; }
	static bool isPsx() { return _systemVars.platform == Common::kPlatformPSX; }
	static bool isWindows() { return _systemVars.platform == Common::kPlatformWindows ; }

	// Used by timer
	void updateTopMenu();
	void updateBottomMenu();
	void fadePaletteStep();
	void startFadePaletteDown(int speed);
	void startFadePaletteUp(int speed);
	void waitForFade();
	bool screenIsFading();
	bool fadeDirectionIsUp();
	void setMenuToTargetState();

	void showDebugInfo();

protected:
	// Engine APIs
	Common::Error init();
	Common::Error go();
	Common::Error run() override {
		Common::Error err;
		err = init();
		if (err.getCode() != Common::kNoError)
			return err;
		return go();
	}
	bool hasFeature(EngineFeature f) const override;
	void syncSoundSettings() override;

	Common::Error loadGameState(int slot) override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::String getSaveStateName(int slot) const override {
		return Common::String::format("sword1.%03d", slot);
	}
private:
	void pollInput(uint32 delay);
	void checkKeys();

	void checkCdFiles();
	void checkCd();
	void askForCd();

	void showFileErrorMsg(uint8 type, bool *fileExists);
	void flagsToBool(bool *dest, uint8 flags);

	void reinitRes(); //Reinits the resources after a GMM load

	void installTimerRoutines();
	void uninstallTimerRoutines();

	uint8 mainLoop();

	Common::Point _mouseCoord;
	uint16 _mouseState;
	Common::KeyState _keyPressed;
	Common::CustomEventType _customType;

	ResMan      *_resMan;
	ObjectMan   *_objectMan;
	Screen      *_screen;
	Mouse       *_mouse;
	Logic       *_logic;
	Sound       *_sound;
	Menu        *_menu;
	Control     *_control;
	static const uint8  _cdList[TOTAL_SECTIONS];
	static const CdFile _pcCdFileList[];
	static const CdFile _macCdFileList[];
	static const CdFile _psxCdFileList[];
};

} // End of namespace Sword1

#endif // SWORD1_SWORD1_H
