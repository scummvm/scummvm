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
	_backScreen = (uint8 *)malloc(GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT);
	forceRefresh();

	_currentScreen = NULL;
	_scrollScreen = NULL;

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
	_currentPalette = 0;

	_seqInfo.framesLeft = 0;
	_seqInfo.seqData = _seqInfo.seqDataPos = NULL;
	_seqInfo.running = false;
}

SkyScreen::~SkyScreen(void) {

	free(_gameGrid);
	if (_currentScreen) free(_currentScreen);
	if (_backScreen) free(_backScreen);
}

void SkyScreen::clearScreen(void) {
	 
	memset(_currentScreen, 0, FULL_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);
	_system->copy_rect(_currentScreen, GAME_SCREEN_WIDTH, 0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
	_system->update_screen();
}

//set a new palette, pal is a pointer to dos vga rgb components 0..63
void SkyScreen::setPalette(uint8 *pal) {
	
	convertPalette(pal, _palette);
	_system->set_palette(_palette, 0, GAME_COLOURS);
	_system->update_screen();
}

void SkyScreen::setPaletteEndian(uint8 *pal) {

#ifdef SCUMM_BIG_ENDIAN
	uint8 endPalette[256 * 3];
	for (uint16 cnt = 0; cnt < 256 * 3; cnt++)
		endPalette[cnt] = pal[cnt ^ 1];
	convertPalette(endPalette, _palette);
#else
	convertPalette(pal, _palette);
#endif
	_system->set_palette(_palette, 0, GAME_COLOURS);
	_system->update_screen();
}

void SkyScreen::halvePalette(void) {

	uint8 halfPalette[1024];
	for (uint8 cnt = 0; cnt < GAME_COLOURS; cnt++) {
		halfPalette[(cnt << 2) | 0] = _palette[(cnt << 2) | 0] >> 1;
		halfPalette[(cnt << 2) | 1] = _palette[(cnt << 2) | 1] >> 1;
		halfPalette[(cnt << 2) | 2] = _palette[(cnt << 2) | 2] >> 1;
		halfPalette[(cnt << 2) | 3] = 0;
	}
	_system->set_palette(halfPalette, 0, GAME_COLOURS);
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

void SkyScreen::recreate(void) {

	// check the game grid for changed blocks
	if (!SkyLogic::_scriptVariables[LAYER_0_ID]) return ;
	uint8 *gridPos = _gameGrid;
	uint8 *screenData = (uint8 *)SkyState::fetchItem(SkyLogic::_scriptVariables[LAYER_0_ID]);
	if (!screenData) {
		error("SkyScreen::recreate():\nSkyState::fetchItem(SkyLogic::_scriptVariables[LAYER_0_ID](%X)) returned NULL",SkyLogic::_scriptVariables[LAYER_0_ID]);
	}
	uint8 *screenPos = _backScreen;

	for (uint8 cnty = 0; cnty < GRID_Y; cnty++) {
		for (uint8 cntx = 0; cntx < GRID_X; cntx++) {
			if (gridPos[0] & 0x80) {
				gridPos[0] &= 0x7F; // reset recreate flag
				gridPos[0] |= 1;    // set bit for flip routine
				uint8 *savedScreenY = screenPos;
				for (uint8 gridCntY = 0; gridCntY < GRID_H; gridCntY++) {
					memcpy(screenPos, screenData, GRID_W);
					screenPos += GAME_SCREEN_WIDTH;
					screenData += GRID_W;
				}
				screenPos = savedScreenY + GRID_W;
			} else {
				screenPos += GRID_W;
				screenData += GRID_W * GRID_H;
			}
			gridPos++;
		}
		screenPos += (GRID_H - 1) * GAME_SCREEN_WIDTH;
	}
}

void SkyScreen::flip(void) {

	SkyState::_systemVars.mouseFlag |= MF_NO_UPDATE;
	// drawMouseToBackScreen();
	uint8 *screenPos = _currentScreen;
	uint8 *backPos = _backScreen;
	uint32 copyX, copyWidth;
	copyX = copyWidth = 0;
	for (uint8 cnty = 0; cnty < GRID_Y; cnty++) {
		for (uint8 cntx = 0; cntx < GRID_X; cntx++) {
			if (_gameGrid[cnty * GRID_X + cntx] & 1) {
				_gameGrid[cnty * GRID_X + cntx] &= ~1;
				if (!copyWidth) copyX = cntx * GRID_W;
				copyWidth += GRID_W;
				uint8 *copySrc = backPos;
				uint8 *copyDest = screenPos;
				for (uint8 gridLineCnt = 0; gridLineCnt < GRID_H; gridLineCnt++) {
					memcpy(copyDest, copySrc, GRID_W);
					copySrc += GAME_SCREEN_WIDTH;
					copyDest += GAME_SCREEN_WIDTH;
				}
			} else if (copyWidth) {
				_system->copy_rect(_currentScreen + cnty * GRID_H * GAME_SCREEN_WIDTH + copyX, GAME_SCREEN_WIDTH, copyX, cnty * GRID_H, copyWidth, GRID_H);
				copyWidth = 0;
			}
			backPos += GRID_W;
			screenPos += GRID_W;
		}
		if (copyWidth) {
			_system->copy_rect(_currentScreen + cnty * GRID_H * GAME_SCREEN_WIDTH + copyX, GAME_SCREEN_WIDTH, copyX, cnty * GRID_H, copyWidth, GRID_H);
			copyWidth = 0;
		}
		screenPos += (GRID_H - 1) * GAME_SCREEN_WIDTH;
		backPos += (GRID_H - 1) * GAME_SCREEN_WIDTH;
	}
	SkyState::_systemVars.mouseFlag &= ~MF_NO_UPDATE;
	// _skyMouse->restoreDataToBackScreen();
}

void SkyScreen::fnDrawScreen(uint32 palette, uint32 scroll) {

	// set up the new screen
	fnFadeDown(scroll);
	forceRefresh();
	recreate();
	spriteEngine();
	flip();
	fnFadeUp(palette, scroll);
}

void SkyScreen::fnFadeDown(uint32 scroll) {

	if (scroll && (!(SkyState::_systemVars.systemFlags & SF_NO_SCROLL))) {
		// scrolling is performed by fnFadeUp. It's just prepared here
		_scrollScreen = _currentScreen;
		_currentScreen = (uint8 *)malloc(FULL_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);
		// the game will draw the new room into _currentScreen which
		// will be scrolled into the visible screen by fnFadeUp
		// fnFadeUp also frees the _scrollScreen
	} else {
		for (uint8 cnt = 0; cnt < 32; cnt++) {
			palette_fadedown_helper((uint32 *)_palette, GAME_COLOURS);
			_system->set_palette(_palette, 0, GAME_COLOURS);
			_system->update_screen();
			_system->delay_msecs(20);
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

	for (uint8 cnt = 1; cnt <= 32; cnt++) {
		for (uint8 colCnt = 0; colCnt < GAME_COLOURS; colCnt++) {
			_palette[(colCnt << 2) | 0] = (tmpPal[(colCnt << 2) | 0] * cnt) >> 5;
			_palette[(colCnt << 2) | 1] = (tmpPal[(colCnt << 2) | 1] * cnt) >> 5;
			_palette[(colCnt << 2) | 2] = (tmpPal[(colCnt << 2) | 2] * cnt) >> 5;
		}
		_system->set_palette(_palette, 0, GAME_COLOURS);
		_system->update_screen();
		_system->delay_msecs(20);
	}	
}

void SkyScreen::fnFadeUp(uint32 palNum, uint32 scroll) {

	//_currentScreen points to new screen,
	//_scrollScreen points to graphic showing old room
	if ((scroll != 123) && (scroll != 321)) {
		scroll = 0;
	}

	if ((scroll == 0) || (SkyState::_systemVars.systemFlags & SF_NO_SCROLL)) {
		uint8 *palette = (uint8 *)SkyState::fetchCompact(palNum);
		if (palette == NULL)
			error("SkyScreen::fnFadeUp: can't fetch compact %X.\n", palNum);
#ifdef SCUMM_BIG_ENDIAN
		byte tmpPal[256 * 3];
		for (uint16 cnt = 0; cnt < 256*3; cnt++)
			tmpPal[cnt] = palette[cnt ^ 1];
		paletteFadeUp(tmpPal);
#else
		paletteFadeUp(palette);
#endif
	} else if (scroll == 123) {
		// scroll left (going right)
		if (!_currentScreen) error("SkyScreen::fnFadeUp[Scroll L]: _currentScreen is NULL!\n");
		if (!_scrollScreen) error("SkyScreen::fnFadeUp[Scroll L]: _scrollScreen is NULL!\n");
		uint8 *scrNewPtr, *scrOldPtr;
		for (uint8 scrollCnt = 0; scrollCnt < (GAME_SCREEN_WIDTH / SCROLL_JUMP) - 1; scrollCnt++) {
			scrNewPtr = _currentScreen + scrollCnt * SCROLL_JUMP;
			scrOldPtr = _scrollScreen;
			for (uint8 lineCnt = 0; lineCnt < GAME_SCREEN_HEIGHT; lineCnt++) {
				memmove(scrOldPtr, scrOldPtr + SCROLL_JUMP, GAME_SCREEN_WIDTH - SCROLL_JUMP);
				memcpy(scrOldPtr + GAME_SCREEN_WIDTH - SCROLL_JUMP, scrNewPtr, SCROLL_JUMP);
				scrNewPtr += GAME_SCREEN_WIDTH;
				scrOldPtr += GAME_SCREEN_WIDTH;
			}
			showScreen(_scrollScreen);
			waitForTimer();
		}
		showScreen(_currentScreen);
		free(_scrollScreen);
	} else if (scroll == 321) {
		// scroll right (going left)
		if (!_currentScreen) error("SkyScreen::fnFadeUp[Scroll R]: _currentScreen is NULL!\n");
		if (!_scrollScreen) error("SkyScreen::fnFadeUp[Scroll R]: _scrollScreen is NULL!\n");
		uint8 *scrNewPtr, *scrOldPtr;
		for (uint8 scrollCnt = 0; scrollCnt < (GAME_SCREEN_WIDTH / SCROLL_JUMP) - 1; scrollCnt++) {
			scrNewPtr = _currentScreen + GAME_SCREEN_WIDTH - (scrollCnt + 1) * SCROLL_JUMP;
			scrOldPtr = _scrollScreen;
			for (uint8 lineCnt = 0; lineCnt < GAME_SCREEN_HEIGHT; lineCnt++) {
				memmove(scrOldPtr + SCROLL_JUMP, scrOldPtr, GAME_SCREEN_WIDTH - SCROLL_JUMP);
				memcpy(scrOldPtr, scrNewPtr, SCROLL_JUMP);
				scrNewPtr += GAME_SCREEN_WIDTH;
				scrOldPtr += GAME_SCREEN_WIDTH;
			}
			showScreen(_scrollScreen);
			waitForTimer();
		}
		showScreen(_currentScreen);
		free(_scrollScreen);
	}
}

void SkyScreen::waitForTimer(void) {

	_gotTick = false;
	while (!_gotTick) {
		OSystem::Event event;

		_system->delay_msecs(10);
		while (_system->poll_event(&event));
	}
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
	_seqInfo.runningItem = false;
}

void SkyScreen::startSequenceItem(uint16 itemNum) {

	_seqInfo.seqData = (uint8 *)SkyState::fetchItem(itemNum);
	_seqInfo.framesLeft = _seqInfo.seqData[0] - 1;
	_seqInfo.seqDataPos = _seqInfo.seqData + 1;
	_seqInfo.delay = SEQ_DELAY;
	_seqInfo.running = true;
	_seqInfo.runningItem = true;
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
		memset(_seqGrid, 0, 12 * 20);

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

				uint8 gridSta = (uint8)((screenPos / (GAME_SCREEN_WIDTH * 16))*20 + ((screenPos % GAME_SCREEN_WIDTH) >> 4));
				uint8 gridEnd = (uint8)(((screenPos+nrToDo) / (GAME_SCREEN_WIDTH * 16))*20 + (((screenPos+nrToDo) % GAME_SCREEN_WIDTH) >> 4));
				if (gridEnd >= gridSta)
					for (cnt = gridSta; cnt <= gridEnd; cnt++)
						_seqGrid[cnt] = 1;
				else {
					for (cnt = gridSta; cnt < (gridSta / 20 + 1) * 20; cnt++)
						_seqGrid[cnt] = 1;
					for (cnt = (gridEnd / 20) * 20; cnt <= gridEnd; cnt++)
						_seqGrid[cnt] = 1;
				}

				for (cnt = 0; cnt < nrToDo; cnt++) {
					_currentScreen[screenPos] = _seqInfo.seqDataPos[0];
					_seqInfo.seqDataPos++;
					screenPos++;
				}
			} while (nrToDo == 0xFF);
		} while (screenPos < (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT));
		uint8 *gridPtr = _seqGrid; uint8 *scrPtr = _currentScreen; uint8 *rectPtr = NULL;
		uint8 rectWid = 0, rectX = 0, rectY = 0;
		for (uint8 cnty = 0; cnty < 12; cnty++) {
			for (uint8 cntx = 0; cntx < 20; cntx++) {
				if (*gridPtr) {
					if (!rectWid) {
						rectX = cntx;
						rectY = cnty;
						rectPtr = scrPtr;
					}
					rectWid++;
				} else if (rectWid) {
					_system->copy_rect(rectPtr, GAME_SCREEN_WIDTH, rectX << 4, rectY << 4, rectWid << 4, 16);
					rectWid = 0;
				}
				scrPtr += 16;
				gridPtr++;
			}
			if (rectWid) {
				_system->copy_rect(rectPtr, GAME_SCREEN_WIDTH, rectX << 4, rectY << 4, rectWid << 4, 16);
				rectWid = 0;
			}
			scrPtr += 15 * GAME_SCREEN_WIDTH;
		}
		_system->update_screen();
		_seqInfo.framesLeft--;
	}
	if (_seqInfo.framesLeft == 0) {
		_seqInfo.running = false;
		if (!_seqInfo.runningItem) free(_seqInfo.seqData);
		_seqInfo.seqData = _seqInfo.seqDataPos = NULL;
	}
}

//- sprites.asm routines

void SkyScreen::spriteEngine(void) {

	doSprites(BACK);
	sortSprites();
	doSprites(FORE);	
}

void SkyScreen::sortSprites(void) {

	StSortList sortList[30];
	uint32 currDrawList = DRAW_LIST_NO;
	uint32 loadDrawList;

	bool nextDrawList = false;
	while (SkyLogic::_scriptVariables[currDrawList]) {
		// big_sort_loop
		uint32 spriteCnt = 0;
		loadDrawList = SkyLogic::_scriptVariables[currDrawList];
		currDrawList++;

		do { // a_new_draw_list:
			uint16 *drawListData = (uint16 *)SkyState::fetchCompact(loadDrawList);
			nextDrawList = false;
			while ((!nextDrawList) && (drawListData[0])) {
				if (drawListData[0] == 0xFFFF) {
					loadDrawList = drawListData[1];
					nextDrawList = true;
				} else {
					// process_this_id:
					Compact *spriteComp = SkyState::fetchCompact(drawListData[0]);
					if ((spriteComp->status & 4) && // is it sortable playfield?(!?!)
						(spriteComp->screen == SkyLogic::_scriptVariables[SCREEN])) { // on current screen
							dataFileHeader *spriteData = 
								(dataFileHeader *)SkyState::fetchItem(spriteComp->frame >> 6);
							if (!spriteData) {
								printf("Missing file %d!\n",spriteComp->frame >> 6);
								//getchar();
								spriteComp->status = 0;
							} else {
								sortList[spriteCnt].yCood = spriteComp->ycood + spriteData->s_offset_y + spriteData->s_height;
								sortList[spriteCnt].compact = spriteComp;
								sortList[spriteCnt].sprite = spriteData;
								spriteCnt++;
							}
					}
					drawListData++;
				}
			} 
		} while (nextDrawList);
		// made_list:
		if (spriteCnt > 1) { // bubble sort
			for (uint32 cnt1 = 0; cnt1 < spriteCnt - 1; cnt1++)
				for (uint32 cnt2 = cnt1 + 1; cnt2 < spriteCnt; cnt2++)
					if (sortList[cnt1].yCood > sortList[cnt2].yCood) {
						StSortList tmp;
						tmp.yCood = sortList[cnt1].yCood;
						tmp.sprite = sortList[cnt1].sprite;
						tmp.compact = sortList[cnt1].compact;
						sortList[cnt1].yCood = sortList[cnt2].yCood;
						sortList[cnt1].sprite = sortList[cnt2].sprite;
						sortList[cnt1].compact = sortList[cnt2].compact;
						sortList[cnt2].yCood = tmp.yCood;
						sortList[cnt2].sprite = tmp.sprite;
						sortList[cnt2].compact = tmp.compact;
					}
		}
		for (uint32 cnt = 0; cnt < spriteCnt; cnt++) {
			drawSprite((uint8 *)sortList[cnt].sprite, sortList[cnt].compact);
			if (sortList[cnt].compact->status & 8) vectorToGame(0x81);
			else vectorToGame(1);
			if (!(sortList[cnt].compact->status & 0x200)) verticalMask();
		}
	}
}

void SkyScreen::doSprites(uint8 layer) {

	uint16 drawListNum = DRAW_LIST_NO;
	uint32 idNum;
	uint16* drawList;
	while (SkyLogic::_scriptVariables[drawListNum]) { // std sp loop
		idNum = SkyLogic::_scriptVariables[drawListNum];
		drawListNum++;

		drawList = (uint16 *)SkyState::fetchCompact(idNum);
		while(drawList[0]) {
			// new_draw_list:
			while ((drawList[0] != 0) && (drawList[0] != 0xFFFF)) {
				// back_loop:
				// not_new_list
				Compact *spriteData = SkyState::fetchCompact(drawList[0]);
				drawList++;
				if ((spriteData->status & (1 << layer)) && 
						(spriteData->screen == SkyLogic::_scriptVariables[SCREEN])) {
					uint8 *toBeDrawn = (uint8 *)SkyState::fetchItem(spriteData->frame >> 6);
					if (!toBeDrawn) {
						printf("Spritedata %d not loaded!\n",spriteData->frame >> 6);
						spriteData->status = 0;
					} else {
						drawSprite(toBeDrawn, spriteData);
						if (layer == BACK) verticalMask();
						if (spriteData->status & 8) vectorToGame(0x81);
						else vectorToGame(1);
					}
				}
			}
			while (drawList[0] == 0xFFFF)
				drawList = (uint16 *)SkyState::fetchCompact(drawList[1]);
		}
	}
}

void SkyScreen::drawSprite(uint8 *spriteInfo, Compact *sprCompact) {

	if (spriteInfo == NULL) {
		warning("SkyScreen::drawSprite Can't draw sprite. Data %d was not loaded.\n", sprCompact->frame >> 6);
		sprCompact->status = 0;
		return ;
	}
	dataFileHeader *sprDataFile = (dataFileHeader *)spriteInfo;
	_sprWidth = sprDataFile->s_width;
	_sprHeight = sprDataFile->s_height;
	_maskX1 = _maskX2 = 0;
	uint8 *spriteData = spriteInfo + (sprCompact->frame & 0x3F) * sprDataFile->s_sp_size;
	spriteData += sizeof(dataFileHeader);
	int32 spriteY = sprCompact->ycood + sprDataFile->s_offset_y - TOP_LEFT_Y;
	if (spriteY < 0) {
		spriteY = -spriteY;
		if (_sprHeight <= (uint32)spriteY) {
			_sprWidth = 0;
			return ;
		}
		_sprHeight -= spriteY;
		spriteData += sprDataFile->s_width * spriteY;
		spriteY = 0;
	} else {
		int32 botClip = GAME_SCREEN_HEIGHT - sprDataFile->s_height - spriteY;
		if (botClip < 0) {
			botClip = -botClip;
			if (_sprHeight <= (uint32)botClip) {
				_sprWidth = 0;
				return ;
			}
			_sprHeight -= botClip;
		}
	}
	_sprY = (uint32)spriteY;
	int32 spriteX = sprCompact->xcood + sprDataFile->s_offset_x - TOP_LEFT_X;
	if (spriteX < 0) {
		spriteX = -spriteX;
		if (_sprWidth <= (uint32)spriteX) {
			_sprWidth = 0;
			return ;
		}
		_sprWidth -= spriteX;
		_maskX1 = spriteX;
		spriteX = 0;
	} else {
		int32 rightClip = GAME_SCREEN_WIDTH - (sprDataFile->s_width + spriteX);
		if (rightClip < 0) {
			rightClip = (-rightClip) + 1;
			if (_sprWidth <= (uint32)rightClip) {
				_sprWidth = 0;
				return ;
			}
			_sprWidth -= rightClip;
			_maskX2 = rightClip;
		}
	}
	_sprX = (uint32)spriteX;
	uint8 *screenPtr = _backScreen + _sprY * GAME_SCREEN_WIDTH + _sprX;
	if ((_sprHeight > 192) || (_sprY > 192)) {
		_sprWidth = 0;
		return;
	}
	if ((_sprX + _sprWidth > 320) || (_sprY + _sprHeight > 192)) {
		warning("SkyScreen::drawSprite fatal error: got x = %d, y = %d, w = %d, h = %d\n",_sprX, _sprY, _sprWidth, _sprHeight);
		_sprWidth = 0;
		return ;
	}
	
	for (uint16 cnty = 0; cnty < _sprHeight; cnty++) {
		for (uint16 cntx = 0; cntx < _sprWidth; cntx++)
			if (spriteData[cntx + _maskX1]) screenPtr[cntx] = spriteData[cntx + _maskX1];
		spriteData += _sprWidth + _maskX2 + _maskX1;
		screenPtr += GAME_SCREEN_WIDTH;
	}
	// Convert the sprite coordinate/size values to blocks for vertical mask and/or vector to game
	_sprWidth += _sprX + GRID_W-1;
	_sprHeight += _sprY + GRID_H-1;
	
	_sprX >>= GRID_W_SHIFT;
	_sprWidth >>= GRID_W_SHIFT;
	_sprY >>= GRID_H_SHIFT;
	_sprHeight >>= GRID_H_SHIFT;

	_sprWidth -= _sprX;
	_sprHeight -= _sprY;
}

void SkyScreen::vectorToGame(uint8 gridVal) {

	if (_sprWidth == 0) return ;
	uint8 *trgGrid = _gameGrid + _sprY * GRID_X +_sprX;
	for (uint32 cnty = 0; cnty < _sprHeight; cnty++) {
		for (uint32 cntx = 0; cntx < _sprWidth; cntx++)
			trgGrid[cntx] |= gridVal;
		trgGrid += GRID_X;
	}
}

void SkyScreen::vertMaskSub(uint16 *grid, uint32 gridOfs, uint8 *screenPtr, uint32 layerId) {

	for (uint32 cntx = 0; cntx < _sprHeight; cntx++) { // start_x | block_loop
		if (grid[gridOfs]) {
			if (!(FROM_LE_16(grid[gridOfs]) & 0x8000)) {
				uint32 gridVal = FROM_LE_16(grid[gridOfs]) - 1;
				gridVal *= GRID_W * GRID_H;
				uint8 *dataSrc = (uint8 *)SkyState::fetchItem(SkyLogic::_scriptVariables[layerId]) + gridVal;
				uint8 *dataTrg = screenPtr;
				for (uint32 grdCntY = 0; grdCntY < GRID_H; grdCntY++) {
					for (uint32 grdCntX = 0; grdCntX < GRID_W; grdCntX++)
						if (dataSrc[grdCntX]) dataTrg[grdCntX] = dataSrc[grdCntX];
					dataSrc += GRID_W;
					dataTrg += GAME_SCREEN_WIDTH;
				}
			} // dummy_end:
			screenPtr -= GRID_H * GAME_SCREEN_WIDTH;
			gridOfs -= GRID_X;
		} else return;
	} // next_x
}

void SkyScreen::verticalMask(void) {

	if (_sprWidth == 0) return ;
	uint32 startGridOfs = (_sprY + _sprHeight - 1) * GRID_X + _sprX;
	uint8 *startScreenPtr = (_sprY + _sprHeight - 1) * GRID_H * GAME_SCREEN_WIDTH + _sprX * GRID_W + _backScreen;

	for (uint32 layerCnt = LAYER_1_ID; layerCnt <= LAYER_3_ID; layerCnt++) {
		uint32 gridOfs = startGridOfs;
		uint8 *screenPtr = startScreenPtr;
		for (uint32 widCnt = 0; widCnt < _sprWidth; widCnt++) { // x_loop
			uint32 nLayerCnt = layerCnt;
			while (SkyLogic::_scriptVariables[nLayerCnt + 3]) {
				uint16 *scrGrid;
				scrGrid = (uint16 *)SkyState::fetchItem(SkyLogic::_scriptVariables[layerCnt + 3]);
				if (scrGrid[gridOfs]) {
					vertMaskSub(scrGrid, gridOfs, screenPtr, layerCnt);
					break;
				} else nLayerCnt++;
			}
			// next_x:
			screenPtr += GRID_W;
			gridOfs++;
		}
	}
}

void SkyScreen::paintBox(uint16 x, uint16 y) {

	uint8 *screenPos = _currentScreen + y * GAME_SCREEN_WIDTH + x;
	memset(screenPos, 255, 8);
	for (uint8 cnt = 1; cnt < 8; cnt++) {
		*(screenPos + cnt * GAME_SCREEN_WIDTH) = 255;
		*(screenPos + cnt * GAME_SCREEN_WIDTH + 7) = 255;
	}
	memset(screenPos + 7 * GAME_SCREEN_WIDTH, 255, 7);
}

void SkyScreen::showGrid(uint8 *gridBuf) {

	uint32 gridData = 0;
	uint8 bitsLeft = 0;
	for (uint16 cnty = 0; cnty < GAME_SCREEN_HEIGHT >> 3; cnty++) { 
		for (uint16 cntx = 0; cntx < GAME_SCREEN_WIDTH >> 3; cntx++) {
			if (!bitsLeft) {
				bitsLeft = 32;
				gridData = *(uint32 *)gridBuf;
				gridBuf += 4;
			}
			if (gridData & 0x80000000)
				paintBox(cntx << 3, cnty << 3);
			bitsLeft--;
			gridData <<= 1;
		}
	}
	_system->copy_rect(_currentScreen, GAME_SCREEN_WIDTH, 0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

}

