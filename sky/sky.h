/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#ifndef SKYMAIN_H
#define SKYMAIN_H

#include "stdafx.h"
#include "base/engine.h"

class GameDetector;

namespace Sky {

struct SystemVars {
	uint32 systemFlags;
	uint32 gameVersion;
	uint32 mouseFlag;
	uint16 language;
	uint32 currentPalette;
	uint16 gameSpeed;
	uint16 currentMusic;
	bool pastIntro;
	bool quitting;
};

struct Compact;
class Sound;
class Disk;
class Text;
class Logic;
class Mouse;
class Screen;
class Control;
class MusicBase;
class Intro;

class SkyEngine : public Engine {
	void errorString(const char *buf_input, char *buf_output);
protected:
	byte _key_pressed;
	bool _quickLaunch; // set when starting with -x
	bool _floppyIntro;

	uint16 _debugMode;

	int _numScreenUpdates;

	FILE *_dump_file;

	int _number_of_savegames;

	int _sdl_mouse_x, _sdl_mouse_y;

	Sound *_skySound;
	Disk *_skyDisk;
	Text *_skyText;
	Logic *_skyLogic;
	Mouse *_skyMouse;
	Screen *_skyScreen;
	Control *_skyControl;

	MusicBase *_skyMusic;
	Intro *_skyIntro;
	
public:
	SkyEngine(GameDetector *detector, OSystem *syst);
	virtual ~SkyEngine();

	static bool isDemo(void);
	static bool isCDVersion(void);

	static Compact *fetchCompact(uint32 a);
	static void **fetchItem(uint32 num);
	
	static void **_itemList[300];

	static SystemVars _systemVars;

protected:
	byte _fastMode;

	void logic_engine();
	void delay(uint amount);
	void go();
	void doCheat(uint8 num);
	void handleKey(void);

	uint32 _lastSaveTime;

	Text *getText();
	void initialise();
	void initItemList();

	void initVirgin();
	static void timerHandler(void *ptr);
	void gotTimerTick();
	void loadFixedItems();
	void loadBase0();
	
	static int CDECL game_thread_proc(void *param);
};

} // End of namespace Sky

#endif
