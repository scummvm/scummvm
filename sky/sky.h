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

#ifndef SKYMAIN_H
#define SKYMAIN_H

#include <stdio.h>
#include "common/engine.h"
#include "common/util.h"
#include "common/timer.h"
#include "sound/mixer.h"
#include "sky/sound.h"
#include "sky/text.h"
#include "sky/disk.h"
#include "sky/struc.h"
#include "sky/skymusic.h"
#include "sky/grid.h"

class SkyLogic;
class SkyGrid;

class SkyState : public Engine {
	void errorString(const char *buf_input, char *buf_output);
protected:
	byte _game;
	uint32 _gameVersion;
	byte _key_pressed;

	uint32 _tseqFrames;
	byte *_tseqData;
	uint32 _tseqCounter;

	uint16 _debugMode;
	uint16 _debugLevel;
	uint16 _language;
	uint _mouse_x, _mouse_y;
	uint _mouse_x_old, _mouse_y_old;
	bool _mouse_pos_changed;
	uint _left_button_down;

	uint8 _palette[1024];

	int _numScreenUpdates;

	Timer *_timer;
	//int _timer_id;

	FILE *_dump_file;

	int _number_of_savegames;

	int _sdl_mouse_x, _sdl_mouse_y;

	SkySound *_sound;
	SkyDisk *_skyDisk;
	SkyText *_skyText;
	SkyGrid *_grid;
	SkyLogic *_skyLogic;
	SkyMusic *_music;
	
	byte *_workScreen;
	byte *_backScreen;
	byte *_tempPal;
	byte *_workPalette;
	byte *_halfPalette;
	byte *_scrollAddr;
	
public:
	SkyState(GameDetector *detector, OSystem *syst);
	virtual ~SkyState();

	static bool isDemo(uint32 version);
	static bool isCDVersion(uint32 version);

	static Compact *fetchCompact(uint32 a);
	
	static void **_itemList[300];

protected:
	void logic_engine();
	void delay(uint amount);
	void pollMouseXY();
	void go();
	void convertPalette(uint8 *inpal, uint8* outpal);

	SkyText *getSkyText();
	void initialise();
	void initTimer();
	void initialiseScreen();
	void initialiseGrids();
	void initItemList();
	void setPalette(uint8 *pal);
	void fnFadeDown(uint8 action);
	void palette_fadedown_helper(uint32 *pal, uint num);
	void paletteFadeUp(uint8 *pal);
	void palette_fadeup_helper(uint32 *realPal, uint32 *desiredPal, int num);
	void initVirgin();
	void intro();
	void doCDIntro();
	void showScreen();
	void startTimerSequence(byte *sequence);
	static void timerHandler(void *ptr);
	void doTimerSequence();
	
	static int CDECL game_thread_proc(void *param);

	void shutdown();

	RandomSource _rnd;
};

#endif
