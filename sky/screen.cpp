/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "screen.h"

uint8 SkyScreen::_top16Colours[16*3] =
{
	0, 0, 0,
	38, 38, 38,
	63, 63, 63,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	54, 54, 54,
	45, 47, 49,
	32, 31, 41,
	29, 23, 37,
	23, 18, 30,
	49, 11, 11,
	39, 5, 5,
	29, 1, 1,
	63, 63, 63
};

SkyScreen::SkyScreen(OSystem *pSystem, SkyDisk *pDisk) {

	_system = pSystem;
	_skyDisk = pDisk;

	int i;
	uint8 tmpPal[1024];

	_system->init_size(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
	_gameGrid = (uint8 *)malloc(GRID_X * GRID_Y * 2);
	_currentScreen = NULL;

	//blank the first 240 colors of the palette 
	memset(tmpPal, 0, GAME_COLOURS * 4);

	//set the remaining colors
	for (i = 0; i < (VGA_COLOURS-GAME_COLOURS); i++) {
		tmpPal[4 * GAME_COLOURS + i * 4] = (_top16Colours[i * 3] << 2) + (_top16Colours[i * 3] & 3);
		tmpPal[4 * GAME_COLOURS + i * 4 + 1] = (_top16Colours[i * 3 + 1] << 2) + (_top16Colours[i * 3 + 1] & 3);
		tmpPal[4 * GAME_COLOURS + i * 4 + 2] = (_top16Colours[i * 3 + 2] << 2) + (_top16Colours[i * 3 + 2] & 3);
		tmpPal[4 * GAME_COLOURS + i * 4 + 3] = 0x00; 
	}

	//set the palette
	_system->set_palette(tmpPal, 0, VGA_COLOURS);

	_seqInfo.framesLeft = 0;
	_seqInfo.seqData = _seqInfo.seqDataPos = NULL;
	_seqInfo.running = false;
}

SkyScreen::~SkyScreen(void) {

	free(_gameGrid);
	if (_currentScreen) free(_currentScreen);
}

//set a new palette, pal is a pointer to dos vga rgb components 0..63
void SkyScreen::setPalette(uint8 *pal) {
	
	convertPalette(pal, _palette);
	_system->set_palette(_palette, 0, GAME_COLOURS);
	_system->update_screen();
}

void SkyScreen::setPalette(uint16 fileNum) {

	uint8 *tmpPal = _skyDisk->loadFile(fileNum, NULL);
	if (tmpPal) {
		setPalette(tmpPal);
		free(tmpPal);
	} else warning("SkyScreen::setPalette: can't load file nr. %d\n",fileNum);
}

void SkyScreen::showScreen(uint16 fileNum) {

	if (_currentScreen) free(_currentScreen);
	_currentScreen = _skyDisk->loadFile(fileNum, NULL);
	
	if (_currentScreen) showScreen(_currentScreen);
	else warning("SkyScreen::showScreen: can't load file nr. %d\n",fileNum);
}

void SkyScreen::showScreen(uint8 *pScreen) {

	_system->copy_rect(pScreen, 320, 0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
	_system->update_screen();
}

void SkyScreen::convertPalette(uint8 *inPal, uint8* outPal) { //convert 3 byte 0..63 rgb to 4byte 0..255 rgbx
	
	int i;

	for (i = 0; i < VGA_COLOURS; i++) {
		outPal[4 * i] = (inPal[3 * i] << 2) + (inPal[3 * i] & 3);
		outPal[4 * i + 1] = (inPal[3 * i + 1] << 2) + (inPal[3 * i + 1] & 3);
		outPal[4 * i + 2] = (inPal[3 * i + 2] << 2) + (inPal[3 * i + 2] & 3);
		outPal[4 * i + 3] = 0x00;
	}
}

//action = 0, simply fade out
//action = 1, scroll left
//action = 2, scroll right
void SkyScreen::fnFadeDown(uint8 action) {

	if (action) {
	
		//do scroll
		warning("SkyScreen::fnFadeDown: Scrolling not yet implemented!\n");

	} else {
		for (uint8 cnt = 0; cnt < 32; cnt++) {
			palette_fadedown_helper((uint32 *)_palette, GAME_COLOURS);
			_system->set_palette(_palette, 0, GAME_COLOURS);
			_system->update_screen();
			waitForTimer();			
		}
	}
}

void SkyScreen::palette_fadedown_helper(uint32 *pal, uint num) {
	byte *p = (byte *)pal;

	do {
		if (p[0] >= 8)
			p[0] -= 8;
		else
			p[0] = 0;
		if (p[1] >= 8)
			p[1] -= 8;
		else
			p[1] = 0;
		if (p[2] >= 8)
			p[2] -= 8;
		else
			p[2] = 0;
		p += sizeof(uint32);
	} while (--num);
}

void SkyScreen::paletteFadeUp(uint16 fileNr) {

	uint8 *pal = _skyDisk->loadFile(fileNr, NULL);
	if (pal) {
		paletteFadeUp(pal);
        free(pal);
	} else printf("SkyScreen::paletteFadeUp: Can't load palette #%d\n",fileNr);
}

void SkyScreen::paletteFadeUp(uint8 *pal) {

	byte tmpPal[1024];
	
	convertPalette(pal, tmpPal);

	for (uint8 cnt = 0; cnt < 32; cnt++) {
		palette_fadeup_helper((uint32 *)_palette, (uint32 *)tmpPal, GAME_COLOURS);
		_system->set_palette(_palette, 0, GAME_COLOURS);
		_system->update_screen();
		waitForTimer();
	}	
}

void SkyScreen::palette_fadeup_helper(uint32 *realPal, uint32 *desiredPal, int num) {

	byte *r = (byte *)realPal;
	byte *d = (byte *)desiredPal;

	do {
		if (r[0] < d[0]-8) r[0] += 8;
		else r[0] = d[0];

		if (r[1] < d[1]-8) r[1] += 8;
		else r[1] = d[1];

		if (r[2] < d[2]-8) r[2] += 8;
		else r[2] = d[2];
				
		r += sizeof(uint32);
		d += sizeof(uint32);
	} while (--num);
	
}

void SkyScreen::waitForTimer(void) {

	_gotTick = false;
	while (!_gotTick)
		_system->delay_msecs(10);
}

void SkyScreen::handleTimer(void) {

	_gotTick = true;
	if (_seqInfo.running) processSequence();
}

void SkyScreen::startSequence(uint16 fileNum) {

	_seqInfo.seqData = _skyDisk->loadFile(fileNum, NULL);
	_seqInfo.framesLeft = _seqInfo.seqData[0];
	_seqInfo.seqDataPos = _seqInfo.seqData + 1;
	_seqInfo.delay = SEQ_DELAY;
	_seqInfo.running = true;
}

void SkyScreen::stopSequence() {

	_seqInfo.running = false;
	waitForTimer();
	waitForTimer();
	_seqInfo.framesLeft = 0;
	free(_seqInfo.seqData);
	_seqInfo.seqData = _seqInfo.seqDataPos = NULL;
}

void SkyScreen::processSequence(void) {

	uint32 screenPos = 0;

	_seqInfo.delay--;
	if (_seqInfo.delay == 0) {
		_seqInfo.delay = SEQ_DELAY;

        uint8 nrToSkip, nrToDo, cnt;
		do {
			do {
				nrToSkip = _seqInfo.seqDataPos[0];
				_seqInfo.seqDataPos++;
				screenPos += nrToSkip;
			} while (nrToSkip == 0xFF);
			do {
				nrToDo = _seqInfo.seqDataPos[0];
				_seqInfo.seqDataPos++;
				for (cnt = 0; cnt < nrToDo; cnt++) {
					_currentScreen[screenPos] = _seqInfo.seqDataPos[0];
					_seqInfo.seqDataPos++;
					screenPos++;
				}
			} while (nrToDo == 0xFF);
		} while (screenPos < (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT));
		showScreen(_currentScreen);
		_seqInfo.framesLeft--;
	}
	if (_seqInfo.framesLeft == 0) {
		_seqInfo.running = false;
		free(_seqInfo.seqData);
		_seqInfo.seqData = _seqInfo.seqDataPos = NULL;
	}
}

bool SkyScreen::sequenceRunning(void) {

	return _seqInfo.running;
}
