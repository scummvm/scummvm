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

#include "stdafx.h"
#include <string.h>
#include "common/scummsys.h"
#include "sky/skydefs.h"
#include "sky/sky.h"

#define WAIT_SEQUENCE	while (_tseqFrames != 0) { checkCommands(commandPtr); delay(50); }

#define INTRO_TEXT_WIDTH    128

#define FN_A_PAL	60080
#define FN_1A_LOG	60081
#define FN_1A	60082
#define FN_1B	60083
#define FN_1C	60084
#define FN_1D	60085
#define FN_1E	60086
#define FN_4A	60087
#define FN_4B_LOG	60088
#define FN_4B	60089
#define FN_4C_LOG	60090
#define FN_4C	60091
#define FN_5_PAL	60092
#define FN_5_LOG	60093
#define FN_5	60094
#define FN_6_PAL	60095
#define FN_6_LOG	60096
#define FN_6A	60097
#define FN_6B	60098

#define IC_PREPARE_TEXT 0
#define IC_SHOW_TEXT    1
#define IC_REMOVE_TEXT  2
#define IC_MAKE_SOUND   3
#define IC_FX_VOLUME    4

uint32 cockpitCommands[] = {

	1000, //do straight away
	IC_PREPARE_TEXT,
	77,
	220,
	IC_SHOW_TEXT, //radar detects jamming signal
	20,
	160,
	105,
	IC_REMOVE_TEXT,
	105,
	IC_PREPARE_TEXT,
	81,
	105,
	IC_SHOW_TEXT, //well switch to override you fool
	170,
	86,
	35,
	IC_REMOVE_TEXT,
	35,
	IC_PREPARE_TEXT,
	477,
	35,
	IC_SHOW_TEXT,
	30,
	160,
	3,
	IC_REMOVE_TEXT,
	0
};

uint32 zeroCommands[] = { 0 };

uint32 anim5Commands[] = {

	31,
	IC_MAKE_SOUND,
	2,
	127,
	0
};

uint32 anim4aCommands[] = {

	136,
	IC_MAKE_SOUND,
	1,
	70,
	90,
	IC_FX_VOLUME,
	80,
	50,
	IC_FX_VOLUME,
	90,
	5,
	IC_FX_VOLUME,
	100,
	0
};

uint32 anim4cCommands[] = {

	1000,
	IC_FX_VOLUME,
	100,
	25,
	IC_FX_VOLUME,
	110,
	15,
	IC_FX_VOLUME,
	120,
	4,
	IC_FX_VOLUME,
	127,
	0
};

uint32 anim6aCommands[] = {

	1000,
	IC_PREPARE_TEXT,
	478,
	13,
	IC_SHOW_TEXT,
	175,
	155,
	0
};

uint32 anim6bCommands[] = {

	131,
	IC_REMOVE_TEXT,
	131,
	IC_PREPARE_TEXT,
	479,
	74,
	IC_SHOW_TEXT,
	175,
	155,
	45,
	IC_REMOVE_TEXT,
	45,
	IC_PREPARE_TEXT,
	162,
	44,
	IC_SHOW_TEXT,
	175,
	155,
	4,
	IC_REMOVE_TEXT,
	0
};

typedef void (SkyState::*commandRoutinesProc)(uint32 *&cmdPtr);
static const commandRoutinesProc commandRoutines[] = {
	&SkyState::prepareText,
	&SkyState::showIntroText,
	&SkyState::removeText,
	&SkyState::introFx,
	&SkyState::introVol,
};

void SkyState::initVirgin() {
	
	_tempPal = _skyDisk->loadFile(60111, NULL);
	if (_tempPal != NULL)
		setPalette(_tempPal);

	_workScreen = _skyDisk->loadFile(60110, NULL);

	if (_workScreen != NULL)
		showScreen();

	// free the memory that was malloc'ed indirectly via load_file
	free(_workScreen);
	free(_tempPal);
}

void SkyState::intro(void) {

	uint8 *seq1aData, *seq1bData, *seq1cData, *seq1dData, *seq1eData;
	uint8 *seq4aData, *seq4bData, *seq4cData;
	uint8 *seq5Data;
	uint8 *seq6aData, *seq6bData;
	uint32 *commandPtr = (uint32 *)zeroCommands;

	_workScreen = _skyDisk->loadFile(60112, NULL); //while virgin screen is up, load rev screen
	_tempPal = _skyDisk->loadFile(60113, NULL);

	_music->loadSectionMusic(0);
	
	delay(3000); //keep virgin screen up for 3 seconds

	if (!isCDVersion(_gameVersion))
		_music->startMusic(1);
	
	delay(3000); //and another 3 seconds.
	fnFadeDown(0); //remove virgin screen
	showScreen();
	paletteFadeUp(_tempPal);
	free (_tempPal);
	free (_workScreen);
	
	//while rev is up, load gibbons screen
	_workScreen = _skyDisk->loadFile(60114, NULL);
	_tempPal = _skyDisk->loadFile(60115, NULL);

	_introTextSpace = (uint8 *)calloc(10000, 1);
	_introTextSave = (uint8 *)calloc(10000, 1);

	_skyText->getText(77, _language);
	
	delay(8000); // keep revolution up for 8 seconds

	fnFadeDown(0);
	showScreen();
	paletteFadeUp(_tempPal);

	free (_tempPal);
	free (_workScreen);

	if (isCDVersion(_gameVersion)) {
		doCDIntro();
	} else {

		_tempPal = _skyDisk->loadFile(FN_A_PAL, NULL);
		_workScreen = _skyDisk->loadFile(FN_1A_LOG, NULL);
		seq1aData = _skyDisk->loadFile(FN_1A, NULL);

		//keep gibbo up for 2 seconds
		delay(2000);
		fnFadeDown(0);
		showScreen();
		paletteFadeUp(_tempPal);

		startTimerSequence(seq1aData);
		free(_tempPal);
		
		seq1bData = _skyDisk->loadFile(FN_1B, NULL);
		seq1cData = _skyDisk->loadFile(FN_1C, NULL);

		WAIT_SEQUENCE;
	
		startTimerSequence(seq1bData);
		WAIT_SEQUENCE;
		startTimerSequence(seq1cData);
		WAIT_SEQUENCE;

		free(seq1aData);
		free(seq1bData);
		seq1dData = _skyDisk->loadFile(FN_1D, NULL);
		seq1eData = _skyDisk->loadFile(FN_1E, NULL);

		startTimerSequence(seq1dData);
		WAIT_SEQUENCE;
		startTimerSequence(seq1eData);

		free(seq1cData);
		free(seq1dData);	

		_vgaData = _skyDisk->loadFile(60100, NULL);
		_vgaPointer = _vgaData;
		_diffData = _skyDisk->loadFile(60101, NULL);
	
		_noFrames = READ_LE_UINT16(_diffData);
		_diffPointer = _diffData + 2;

		seq4aData = _skyDisk->loadFile(FN_4A, NULL);
		
		WAIT_SEQUENCE;		

		//set up the scrolling intro
		_workBase = (byte *)malloc(GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT * 2);

		//clear the base
		memset(_workBase, 0, GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT);	

		WAIT_SEQUENCE;	//1e

		memcpy(_workBase + (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT), _workScreen, GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT); 
		free(_workScreen); 
		_workScreen = _workBase + (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT);
		_workScreenEnd = _workScreen + (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT);	

		_frameCounter = 1;
		free(seq1eData);

		byte scrollByte; 

		while (_frameCounter < _noFrames) {
	
			scrollByte = *_diffPointer++;
			if (scrollByte) {
				_workScreen -= scrollByte * GAME_SCREEN_WIDTH;
				_workScreenEnd -= scrollByte * GAME_SCREEN_WIDTH;
			}

			delay(40); 

			//non-scrolling frame update
			introFrame();

		}

		startTimerSequence(seq4aData);
		free(_vgaData);
		free(_diffData);
		WAIT_SEQUENCE;
 		
		free(_workBase);
		_workScreen = _skyDisk->loadFile(FN_4B_LOG, NULL);
		seq4bData = _skyDisk->loadFile(FN_4B, NULL);	

		showScreen();

		commandPtr = (uint32 *)anim4aCommands; 
		WAIT_SEQUENCE; 

		commandPtr = (uint32 *)cockpitCommands;
		startTimerSequence(seq4bData); 

		checkCommands(commandPtr);
		checkCommands(commandPtr);

		WAIT_SEQUENCE; //4b
		free (_workScreen);
		free (seq4aData);

		_workScreen = _skyDisk->loadFile(FN_4C_LOG, NULL);
		seq4cData = _skyDisk->loadFile(FN_4C, NULL);

		showScreen();
		startTimerSequence(seq4cData);
		free(seq4bData);

		commandPtr = (uint32 *)anim4cCommands;
		WAIT_SEQUENCE; //4c
		free (_workScreen);
		
		_tempPal = _skyDisk->loadFile(FN_5_PAL, NULL);
		_workScreen = _skyDisk->loadFile(FN_5_LOG, NULL);
		seq5Data = _skyDisk->loadFile(FN_5, NULL);
		
		fnFadeDown(0);
		showScreen();
		paletteFadeUp(_tempPal);
		
		startTimerSequence(seq5Data);
		free (_tempPal);
		free (seq4cData);

		commandPtr = (uint32 *)anim5Commands;
		
		WAIT_SEQUENCE;
		free (_workScreen);
		_tempPal = _skyDisk->loadFile(FN_6_PAL, NULL);
		_workScreen = _skyDisk->loadFile(FN_6_LOG, NULL);
		seq6aData = _skyDisk->loadFile(FN_6A, NULL);

		fnFadeDown(0);
		showScreen();

		_music->startMusic(2);
		paletteFadeUp(_tempPal);

		startTimerSequence(seq6aData);
		seq6bData = _skyDisk->loadFile(FN_6B, NULL);
		
		commandPtr = (uint32 *)anim6aCommands;
		WAIT_SEQUENCE; //6a
		
		free (seq5Data);
		free (_tempPal);
		
		startTimerSequence(seq6bData);
		free (seq6aData);
		commandPtr = (uint32 *)anim6bCommands;
		WAIT_SEQUENCE; //6b
		free (seq6bData);

	}

}

void SkyState::showScreen(void) {
	
	_system->copy_rect(_workScreen, 320, 0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
	_system->update_screen();
}

void SkyState::introFrame(void) {

	_frameCounter++;
	byte *diffPtr = _diffPointer;
	byte *vgaPtr = _vgaPointer;
	byte *scrPtr = _workScreen;
	byte count;

	do {
		do {
			count = *diffPtr++;
			scrPtr += count;
		} while (count == 255);

		do {
			count = *diffPtr++;
			memcpy(scrPtr, vgaPtr, count);
			scrPtr += count;
			vgaPtr += count;
		} while (*(diffPtr - 1) == 255);

	} while (scrPtr < _workScreenEnd);

	_diffPointer = diffPtr;
	_vgaPointer = vgaPtr;

	showScreen();

}

void SkyState::checkCommands(uint32 *&cmdPtr) {

	//check for sequence commands
	assert (cmdPtr != NULL);
	
	uint32 afterFrame = READ_LE_UINT32(cmdPtr);
	
	if (afterFrame >= _tseqFrames) { 

		//do a command
		uint32 command = READ_LE_UINT32(cmdPtr + 1);
		(this->*commandRoutines[command])(cmdPtr); 
	} 
}

void SkyState::prepareText(uint32 *&cmdPtr) {
	
	uint32 textNum = READ_LE_UINT32(cmdPtr + 2);
	_skyText->getText(textNum, _language);
	_skyText->displayText(_introTextSpace, true, INTRO_TEXT_WIDTH, 255);
	cmdPtr += 3;  
}

void SkyState::showIntroText(uint32 *&cmdPtr) {
	
	uint32 xPos = READ_LE_UINT32(cmdPtr + 2); 
	uint32 yPos = READ_LE_UINT32(cmdPtr + 3); 
	uint32 startPos = (yPos * FULL_SCREEN_WIDTH) + xPos;
	byte *destBuf = _introTextSpace;
	byte *saveBuf = _introTextSave;
	uint32 width = FROM_LE_32(((struct dataFileHeader *)destBuf)->s_width);	
	uint32 height = FROM_LE_32(((struct dataFileHeader *)destBuf)->s_height);	
	
	*(uint32 *)saveBuf = TO_LE_32(startPos); 
	*(uint32 *)(saveBuf + 4) = TO_LE_32(height);
	*(uint32 *)(saveBuf + 8) = TO_LE_32(width);
	saveBuf += 12;
		
	//save current screen contents
	byte *savePtr = (byte *)_workScreen + startPos;

	for (unsigned int i = 0; i < height; i++) {
		memcpy(saveBuf, savePtr, width);
		saveBuf += width;
		savePtr += GAME_SCREEN_WIDTH;
	}

	//now print the text

	byte *textBuf = _introTextSpace + sizeof(struct dataFileHeader);
	byte *curPos = (byte *)_workScreen + startPos;

	for (unsigned int i = 0; i < height; i++) {

		byte *prevPos = curPos;

		for (unsigned int j = 0; j < width; j++) {

			uint8 pixel = *textBuf++;
			if (pixel) 
				*curPos = pixel;

			curPos++;
		}

		curPos = prevPos + GAME_SCREEN_WIDTH;
	}

	cmdPtr += 4; 

}

void SkyState::removeText(uint32 *&cmdPtr) {
		
	byte *saveBuf = _introTextSave;
	uint32 startPos = READ_LE_UINT32(saveBuf);
	uint32 height = READ_LE_UINT32(saveBuf + 4);
	uint32 width = READ_LE_UINT32(saveBuf + 8);
	byte *dest = _workScreen + startPos;
	saveBuf += 12;
	
	for (unsigned int i = 0; i < height; i++) {
		memcpy(dest, saveBuf, width);
		saveBuf += width;
		dest += FULL_SCREEN_WIDTH;
	}

	cmdPtr += 2;
}

void SkyState::introFx(uint32 *&cmdPtr) {

	warning("introFx() not implemented yet");
	cmdPtr += 4;
}

void SkyState::introVol(uint32 *&cmdPtr) {

	warning("introVol() not implemented yet");
	cmdPtr += 3;
}
