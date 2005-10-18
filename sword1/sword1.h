/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef BSSWORD1_H
#define BSSWORD1_H

#include "base/engine.h"
#include "common/util.h"
#include "base/gameDetector.h"
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
	bool	engineQuit;

	uint8	controlPanelMode;	// 1 death screen version of the control panel, 2 = successful end of game, 3 = force restart
	bool	forceRestart;
	bool	wantFade;			// when true => fade during scene change, else cut.
	uint8	playSpeech;
	uint8	showText;
	uint8	language;
	bool    isDemo;

	uint8	cutscenePackVersion;
};

class SwordEngine : public Engine {
	void errorString(const char *buf_input, char *buf_output);
public:
	SwordEngine(GameDetector *detector, OSystem *syst);
	virtual ~SwordEngine();
	static SystemVars _systemVars;
	void reinitialize(void);

	uint32 _features;
protected:
	int go();
	int init(GameDetector &detector);
private:
	void delay(int32 amount);

	void checkCdFiles(void);
	void checkCd(void);
	void showFileErrorMsg(uint8 type, bool *fileExists);
	void flagsToBool(bool *dest, uint8 flags);
	uint8 mainLoop(void);

	uint16 _mouseX, _mouseY, _mouseState;
	uint8 _keyPressed;

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
	static const CdFile	_cdFileList[];
};

} // End of namespace Sword1

#endif //BSSWORD1_H
