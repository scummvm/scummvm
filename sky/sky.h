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

class SkyState : public Engine {
	void errorString(const char *buf_input, char *buf_output);
protected:
	byte _game;
	uint32 _gameVersion;
	bool _isCDVersion;
	bool _isDemo;
	byte _key_pressed;

	uint32 _tseqFrames;
	byte *_tseqData;
	uint32 _tseqCounter;

	void *_itemList[300];
	uint8 _textBuffer[1024];
	uint32	_dtLineWidth;	//width of line in pixels
	uint32	_dtLines;	//no of lines to do
	uint32	_dtLineSize;	//size of one line in bytes
	uint8	*_dtData;	//address of textdata
	uint32	_dtLetters;	//no of chars in message
	uint8	*_dtText;	//pointer to text
	uint32	_dtCharSpacing;	//character seperation adjustment
	uint32	_dtWidth;	//width of chars in last line (for editing (?))
	uint32	_dtCentre;	//set for centre text
	uint8	*_mouseTextData;	//space for the mouse text

	struct charSet {
		uint8 *addr;
		uint32 charHeight;
		uint32 thirdVal;
	} _mainCharacterSet, _linkCharacterSet, _controlCharacterSet;	
	
	uint32	_curCharSet;
	uint32	_characterSet;
	uint32	_charHeight;
	uint8	*_preAfterTableArea;
	
	uint16 _debugMode;
	uint16 _debugLevel;
	uint16 _language;
	uint _mouse_x, _mouse_y;
	uint _mouse_x_old, _mouse_y_old;
	bool _mouse_pos_changed;
	uint _left_button_down;

	uint8 _palette[1024];

	int _numScreenUpdates;

	uint32 _lastLoadedFileSize;
	
	Timer *_timer;
	//int _timer_id;

	FILE *_dump_file;

	int _number_of_savegames;

	int _sdl_mouse_x, _sdl_mouse_y;

	SkySound *_sound;
	
	byte *_workScreen;
	byte *_backScreen;
	byte *_tempPal;
	byte *_workPalette;
	byte *_halfPalette;
	byte *_scrollAddr;
	
	byte *_gameGrid;
	byte *_gameGrids;
	
public:
	SkyState(GameDetector *detector, OSystem *syst);
	virtual ~SkyState();

protected:
	void delay(uint amount);
	void pollMouseXY();
	void go();
	void convertPalette(uint8 *inpal, uint8* outpal);

	void determineGameVersion(uint32 dnrEntries);
	void setupVersionSpecifics(uint32 version);
	void initialise();
	void initTimer();
	void initialiseDisk();
	void initialiseScreen();
	void initialiseGrids();
	void initItemList();
	void initialiseText();
	void fnSetFont(uint32 fontNr);
	void getText(uint32 textNr);
	char (SkyState::*getTextChar)(uint8 *, uint8 *, uint8 *&);
	char getTextChar_v00267(uint8 *inputValue, uint8 *shiftBits, uint8 *&inputStream);
	char getTextChar_v00288(uint8 *inputValue, uint8 *shiftBits, uint8 *&inputStream);
	char getTextChar_v00368(uint8 *inputValue, uint8 *shiftBits, uint8 *&inputStream);
	bool getTBit(uint8 *inputValue, uint8 *shiftBits, byte *&inputStream);
	void setPalette(uint8 *pal);
	void fnFadeDown(uint8 action);
	void palette_fadedown_helper(uint32 *pal, uint num);
	void paletteFadeUp(uint8 *pal);
	void palette_fadeup_helper(uint32 *realPal, uint32 *desiredPal, int num);
	uint8 *loadFile(uint16 fileNr, uint8 *dest);
	uint8 *getFileInfo(uint16 fileNr);
	void dumpFile(uint16 fileNr);
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
