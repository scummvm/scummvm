/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <stdio.h>
#include "base/engine.h"
#include "common/util.h"
#include "sound/mixer.h"
#include "base/gameDetector.h"
#include "sworddefs.h"

class SwordScreen;
class SwordSound;
class SwordLogic;
class SwordMouse;
class ResMan;
class MemMan;
class ObjectMan;
class SwordMenu;
class SwordMusic;
class SwordControl;

struct SystemVars {
	// todo: move these to a better place
	uint32	currentCD;			// starts at zero, then either 1 or 2 depending on section being played
	uint32	justRestoredGame;	// see main() in sword.c & New_screen() in gtm_core.c
	uint32	gamePaused;			// 1 when paused
	uint32	rate;				// game rate  => what's this for?

	uint8	deathScreenFlag;	// 1 death screen version of the control panel, 2 = successful end of game, 3 = force restart
	bool	forceRestart;
	bool	wantFade;			// when true => fade during scene change, else cut.
	uint8	playSpeech;
	uint8   showText;
    uint8	language;
};

class SwordEngine : public Engine {
	void errorString(const char *buf_input, char *buf_output);
public:
	SwordEngine(GameDetector *detector, OSystem *syst);
	virtual ~SwordEngine();
	static SystemVars _systemVars;
	void reinitialize(void);
protected:
	void go();
private:
	void delay(uint amount);
	void initialize(void);

	void checkCd(void);
	uint8 mainLoop(void);
	void startPositions(int32 startNumber);

	uint16 _mouseX, _mouseY, _mouseState;
	uint8 _keyPressed;

	GameDetector *_detector;
	OSystem		*_system;

	MemMan		*_memMan;
	ResMan		*_resMan;
	ObjectMan	*_objectMan;
	SwordScreen	*_screen;
	SwordMouse	*_mouse;
	SwordLogic	*_logic;
	SwordSound	*_sound;
	SwordMenu	*_menu;
	SwordMusic  *_music;
	SwordControl *_control;
	static const uint8 _cdList[TOTAL_SECTIONS];
};
