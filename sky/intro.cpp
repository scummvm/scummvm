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

#define FREE_IF_NOT_0(ptr)	if (ptr != NULL) { free (ptr); ptr = 0; }
#define REMOVE_INTRO	commandPtr = (uint32 *)zeroCommands; \
			FREE_IF_NOT_0(_introTextSpace) \
			FREE_IF_NOT_0(_introTextSave) \
            _mixer->stopAll();
#define CHECK_ESC if (_key_pressed == 27) { _skyScreen->stopSequence(); REMOVE_INTRO return false; }
#define WAIT_SEQUENCE while (_skyScreen->sequenceRunning()) { checkCommands(commandPtr); delay(50); CHECK_ESC }

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
	
	_skyScreen->setPalette(60111);
	_skyScreen->showScreen(60110);
}

void SkyState::escDelay(uint32 pDelay) {

	pDelay /= 500;
	while (pDelay) {
		delay(500);
		if (_key_pressed == 27) pDelay = 0;
		else pDelay--;
	}
}

bool SkyState::intro(void) {

	uint32 *commandPtr = (uint32 *)zeroCommands;

	if (!isCDVersion())
		_floppyIntro = true;

	_skyDisk->prefetchFile(60112); // revolution screen
	_skyDisk->prefetchFile(60113); // revolution palette

	_skyMusic->loadSection(0);
	_skySound->loadSection(0);
	
	escDelay(3000); //keep virgin screen up for 3 seconds
	CHECK_ESC
	
	if (_floppyIntro)
		_skyMusic->startMusic(1);
	
	escDelay(3000); //and another 3 seconds.
	CHECK_ESC
	
	_skyScreen->fnFadeDown(0); //remove virgin screen
	
	_skyScreen->showScreen(60112);
	_skyScreen->paletteFadeUp(60113);

	//while rev is up, load gibbons screen
	_skyDisk->prefetchFile(60114); // gibbo screen
	_skyDisk->prefetchFile(60115); // gibbo palette

	_introTextSpace = (uint8 *)calloc(10000, 1);
	_introTextSave = (uint8 *)calloc(10000, 1);

	_skyText->getText(77);
	
	escDelay(8000); // keep revolution up for 8 seconds
	CHECK_ESC
	
	_skyScreen->fnFadeDown(0);
	_skyScreen->showScreen(60114);
	_skyScreen->paletteFadeUp(60115);


	if (!_floppyIntro) {
		return doCDIntro();
	} else {
		_skyDisk->prefetchFile(FN_A_PAL);
		_skyDisk->prefetchFile(FN_1A_LOG);
		_skyDisk->prefetchFile(FN_1A);

		//keep gibbo up for 2 seconds
		escDelay(2000);
		CHECK_ESC
		_skyScreen->fnFadeDown(0);

		_skyScreen->showScreen(FN_1A_LOG);
		_skyScreen->paletteFadeUp(FN_A_PAL);
		_skyScreen->startSequence(FN_1A);	
		_skyDisk->prefetchFile(FN_1B);
		WAIT_SEQUENCE;

		_skyScreen->startSequence(FN_1B);
		_skyDisk->prefetchFile(FN_1C);
		WAIT_SEQUENCE;

		_skyScreen->startSequence(FN_1C);
		_skyDisk->prefetchFile(FN_1D);
		WAIT_SEQUENCE;

		_skyScreen->startSequence(FN_1D);
		_skyDisk->prefetchFile(FN_1E);
		WAIT_SEQUENCE;

		_skyScreen->startSequence(FN_1E);

		uint8 *vgaData, *diffData, *vgaPointer, *diffPointer, *scrollData;
		uint8 *currScreenPos;
		vgaData = _skyDisk->loadFile(60100, NULL);
		vgaPointer = vgaData;
		diffData = _skyDisk->loadFile(60101, NULL);
		diffPointer = diffData + 2;
		uint16 noFrames = READ_LE_UINT16(diffData);

		_skyDisk->prefetchFile(FN_4A);
		
		WAIT_SEQUENCE;		

		
		//set up the scrolling intro
		scrollData = (byte *)malloc(GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT * 2);
		//clear the base
		memset(scrollData, 0, GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT);	
		memcpy(scrollData + (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT), _skyScreen->giveCurrent(), GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT); 
		currScreenPos = scrollData + (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT);

		byte scrollByte; 
		for (uint16 frameCounter = 1; frameCounter < noFrames; frameCounter++) {
			scrollByte = *diffPointer++;
			if (scrollByte)
				currScreenPos -= scrollByte * GAME_SCREEN_WIDTH;

			delay(40); 
			if (_key_pressed == 27) {
				REMOVE_INTRO;
				return false;
			}
			
			//non-scrolling frame update
			introFrame(&diffPointer, &vgaPointer, currScreenPos);
			_skyScreen->showScreen(currScreenPos);
		}
		memcpy(_skyScreen->giveCurrent(), currScreenPos, GAME_SCREEN_HEIGHT * GAME_SCREEN_WIDTH);
		_skyScreen->startSequence(FN_4A);
		free(scrollData);
		free(vgaData);
		free(diffData);
		commandPtr = (uint32 *)anim4aCommands;
		_skyDisk->prefetchFile(FN_4B);
		_skyDisk->prefetchFile(FN_4B_LOG);
		WAIT_SEQUENCE;
 		
		_skyScreen->showScreen(FN_4B_LOG);
		commandPtr = (uint32 *)cockpitCommands;
		_skyScreen->startSequence(FN_4B);
		checkCommands(commandPtr);
		checkCommands(commandPtr);
		_skyDisk->prefetchFile(FN_4C);
		WAIT_SEQUENCE; //4b

		_skyScreen->showScreen(FN_4C_LOG);
		_skyScreen->startSequence(FN_4C);
		commandPtr = (uint32 *)anim4cCommands;
		WAIT_SEQUENCE; //4c
		
		_skyDisk->prefetchFile(FN_5_PAL);
		_skyDisk->prefetchFile(FN_5_LOG);
		_skyDisk->prefetchFile(FN_5);
		
		_skyScreen->fnFadeDown(0);
		_skyScreen->showScreen(FN_5_LOG);
		_skyScreen->paletteFadeUp(FN_5_PAL);
		
		_skyScreen->startSequence(FN_5);

		commandPtr = (uint32 *)anim5Commands;
		
		WAIT_SEQUENCE;
		_skyDisk->prefetchFile(FN_6_PAL);
		_skyDisk->prefetchFile(FN_6_LOG);
		_skyDisk->prefetchFile(FN_6A);

		_skyScreen->fnFadeDown(0);
		_skyScreen->showScreen(FN_6_LOG);

		_skyMusic->startMusic(2);
		_skyScreen->paletteFadeUp(FN_6_PAL);

		_skyScreen->startSequence(FN_6A);
		_skyDisk->prefetchFile(FN_6B);
		
		commandPtr = (uint32 *)anim6aCommands;
		WAIT_SEQUENCE; //6a
		
		_skyScreen->startSequence(FN_6B);
		commandPtr = (uint32 *)anim6bCommands;
		WAIT_SEQUENCE; //6b
	}
	return true;
}

void SkyState::introFrame(uint8 **diffPtr, uint8 **vgaPtr, uint8 *screenData) {

	uint32 scrPos = 0;
	uint8 nrToSkip, nrToDo;

	do {
		do {
			nrToSkip = **diffPtr;
			(*diffPtr)++;
			scrPos += nrToSkip;
		} while (nrToSkip == 255);

		do {
			nrToDo = **diffPtr;
			(*diffPtr)++;
			memcpy(screenData + scrPos, *vgaPtr, nrToDo);
			scrPos += nrToDo;
			*vgaPtr += nrToDo;
		} while (nrToDo == 255);

	} while (scrPos < GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT);
}

void SkyState::checkCommands(uint32 *&cmdPtr) {

	//check for sequence commands
	assert (cmdPtr != NULL);
	
	uint32 afterFrame = *(cmdPtr);
	
	if (afterFrame >= _skyScreen->seqFramesLeft()) { 

		//do a command
		uint32 command = *(cmdPtr + 1);
		(this->*commandRoutines[command])(cmdPtr); 
	} 
}

void SkyState::prepareText(uint32 *&cmdPtr) {
	
	uint32 textNum = *(cmdPtr + 2);
	_skyText->getText(textNum);
	_skyText->displayText(_introTextSpace, true, INTRO_TEXT_WIDTH, 255);
	cmdPtr += 3;  
}

#define _workScreen _skyScreen->giveCurrent()
void SkyState::showIntroText(uint32 *&cmdPtr) {
	
	uint32 xPos = *(cmdPtr + 2); 
	uint32 yPos = *(cmdPtr + 3); 
	uint32 startPos = (yPos * FULL_SCREEN_WIDTH) + xPos;
	byte *destBuf = _introTextSpace;
	byte *saveBuf = _introTextSave;
	uint16 width = ((struct dataFileHeader *)destBuf)->s_width;
	uint16 height = ((struct dataFileHeader *)destBuf)->s_height;
	
	*(uint32 *)saveBuf = TO_LE_32(startPos); 
	*(uint32 *)(saveBuf + 4) = TO_LE_32(height);
	*(uint32 *)(saveBuf + 8) = TO_LE_32(width);
	saveBuf += 12;
		
	//save current screen contents
	byte *savePtr = (byte *)_workScreen + startPos;

	uint i, j;

	for (i = 0; i < height; i++) {
		memcpy(saveBuf, savePtr, width);
		saveBuf += width;
		savePtr += GAME_SCREEN_WIDTH;
	}

	//now print the text

	byte *textBuf = _introTextSpace + sizeof(struct dataFileHeader);
	byte *curPos = (byte *)_workScreen + startPos;

	for (i = 0; i < height; i++) {

		byte *prevPos = curPos;

		for (j = 0; j < width; j++) {

			uint8 pixel = *textBuf++;
			if (pixel) 
				*curPos = pixel;

			curPos++;
		}

		curPos = prevPos + GAME_SCREEN_WIDTH;
	}

	cmdPtr += 4; 
	_system->copy_rect(_workScreen + startPos, GAME_SCREEN_WIDTH, xPos, yPos, width, height);
	_system->update_screen();
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
	_system->copy_rect(_workScreen + startPos, GAME_SCREEN_WIDTH, startPos % GAME_SCREEN_WIDTH, startPos / GAME_SCREEN_WIDTH, width, height);
	_system->update_screen();
}
#undef _workScreen

void SkyState::introFx(uint32 *&cmdPtr) {

	_mixer->stopAll();
	_skySound->playSound((uint16)cmdPtr[2], (uint16)cmdPtr[3], 0);
	cmdPtr += 4;
}

void SkyState::introVol(uint32 *&cmdPtr) {

	// HACK: for some reason, the mixer will only stop playing 
	// looping sounds if you do it using SoundMixer::stopAll();
	_mixer->stopAll();
	_skySound->playSound(1, (uint16)(cmdPtr[2] & 0x7F), 0);
	cmdPtr += 3;
}
