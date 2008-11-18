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

#ifndef SWORD1_H
#define SWORD1_H

#include "engines/engine.h"
#include "common/events.h"
#include "common/util.h"
#include "sword1/sworddefs.h"

namespace Sword1 {

enum {
	GF_DEMO = 1 << 0
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
class Music;
class Control;

struct SystemVars {
	bool	runningFromCd;
	uint32	currentCD;			// starts at zero, then either 1 or 2 depending on section being played
	uint32	justRestoredGame;	// see main() in sword.c & New_screen() in gtm_core.c

	uint8	controlPanelMode;	// 1 death screen version of the control panel, 2 = successful end of game, 3 = force restart
	bool	forceRestart;
	bool	wantFade;			// when true => fade during scene change, else cut.
	uint8	playSpeech;
	uint8	showText;
	uint8	language;
	bool    isDemo;
	bool    isMac;

	uint8	cutscenePackVersion;
};

class SwordEngine : public Engine {
public:
	SwordEngine(OSystem *syst);
	virtual ~SwordEngine();
	static SystemVars _systemVars;
	void reinitialize(void);

	uint32 _features;

	bool mouseIsActive();

protected:
	// Engine APIs
	virtual Common::Error init();
	virtual Common::Error go();
	virtual bool hasFeature(EngineFeature f) const;
	virtual void syncSoundSettings();
	// FIXME: Loading a game through the GMM crashes the game
#if 0
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const char *desc);
	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();
#endif

private:
	void delay(int32 amount);

	void checkCdFiles(void);
	void checkCd(void);
	void showFileErrorMsg(uint8 type, bool *fileExists);
	void flagsToBool(bool *dest, uint8 flags);
	uint8 mainLoop(void);

	Common::Point _mouseCoord;
	uint16 _mouseState;
	Common::KeyState _keyPressed;

	ResMan		*_resMan;
	ObjectMan	*_objectMan;
	Screen		*_screen;
	Mouse		*_mouse;
	Logic		*_logic;
	Sound		*_sound;
	Menu		*_menu;
	Music		*_music;
	Control		*_control;
	static const uint8  _cdList[TOTAL_SECTIONS];
	static const CdFile	_pcCdFileList[];
	static const CdFile	_macCdFileList[];
};

} // End of namespace Sword1

#endif //BSSWORD1_H
