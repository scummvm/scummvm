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

class SwordScreen;
class SwordSound;
class SwordLogic;
class SwordMouse;
class ResMan;
class MemMan;
class ObjectMan;
class SwordMenu;
class SwordMusic;

struct SystemVars {
	// todo: move these to a better place
	uint32	currentCD;			// starts at zero, then either 1 or 2 depending on section being played
	uint32	justRestoredGame;	// see main() in sword.c & New_screen() in gtm_core.c
	uint32	gamePaused;			// 1 when paused
	uint32	rate;				// game rate  => what's this for?
	//uint32	textNumber;		// holds last text_no displayed => I think it was only for debugging
	//uint32	endOfQ;				// next available slot in sound fx queue
	//uint8	debug;				// toggles tdebug file

	//uint8	saveGameFlag;		// controls save game loop							0=off 1=save game 2=restore game 3=restart 4=quit to dos
	uint8	deathScreenFlag;	// 1 death screen version of the control panel, 2 = successful end of game, 3 = force restart
	bool	forceRestart;
	uint8	playSpeech;
	uint8   showText;
	//uint8   snrStatus;
			// ^=> move into SwordControl... or whatever it will be.
	//uint8	displayText;		// toggles debug text display						on "t"
	//uint8	displayGrid;		// toggles debug grid display						on "g"
	//uint8	displayMouse;		// toggles debug mouse display 					on "m"
	//uint8	framesPerSecond;	// toggles one frame pre second mode		on "1"
	//uint8	writingPCXs;		// writing a PCX every frame						on "f"
	//int16	parallaxOn;			I think we don't need this.
    uint8	language;
	//int32	currentMusic;
	//uint32	gameCycle;
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

	void mainLoop(void);
	void fnCheckCd(uint32 newScreen);
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
};
