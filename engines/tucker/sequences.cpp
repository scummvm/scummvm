/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * $URL$
 * $Id$
 *
 */

#include "tucker/tucker.h"
#include "tucker/graphics.h"

namespace Tucker {

void TuckerEngine::handleCreditsSequence() {
	static const int _creditsSequenceData1[] = { 200, 350, 650, 850, 1150, 1450, 12000 };
	static const int _creditsSequenceData2[] = { 1, 1, 5, 0, 6, 6, 0 };
	int counter4 = 0;
	int counter3 = 0;
	int num = 0;
	int imgNum = 0;
	int prevLocationNum = _locationNum;
	int counter2 = 0;
	int counter1 = 0;
	loadCharset2();
	stopSounds();
	_locationNum = 74;
	uint8 *imgBuf = (uint8 *)malloc(16 * 64000);
	loadSprC02_01();
	_spritesCount = _creditsSequenceData2[num];
	for (int i = 0; i < _spritesCount; ++i) {
		memset(&_spritesTable[i], 0, sizeof(Sprite));
		_spritesTable[i].state = -1;
		_spritesTable[i].stateIndex = -1;
	}
	strcpy(_fileToLoad, "credits.txt");
	loadFile(_ptTextBuf);
	strcpy(_fileToLoad, "loc74.pcx");
	loadImage(_quadBackgroundGfxBuf, 1);
	startSpeechSound(9001, 120);
	_timerCounter2 = 0;
	_fadePaletteCounter = 0;
	do {
		if (_fadePaletteCounter < 16) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		if (counter4 + 20 > _creditsSequenceData1[num]) {
			fadeInPalette();
		}
		++imgNum;
		if (imgNum == 16) {
			imgNum = 0;
		}
		if (num < 6) {
			Graphics::copyTo640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf, 320, 320, 200);
		} else {
			if (getLastKeyCode() > 0) {
				return;
			}
			Graphics::copyTo640(_locationBackgroundGfxBuf, imgBuf + imgNum * 64000, 320, 320, 200);
			drawString2(5, 48, counter2 * 6);
			drawString2(5, 60, counter2 * 6 + 1);
			drawString2(5, 80, counter2 * 6 + 2);
			drawString2(5, 92, counter2 * 6 + 3);
			drawString2(5, 140, counter2 * 6 + 4);
			drawString2(5, 116, counter2 * 6 + 5);
			++counter1;
			if (counter1 < 20) {
				fadePaletteColor(191, kFadePaletteStep);
			} else if (counter1 > 106) {
				fadePaletteColor(191, -kFadePaletteStep);
			}
			if (counter1 > 116) {
				counter1 = 0;
			}
			++counter2;
			if (counter2 > 17) {
				counter2 = 0;
			}
		}
		++counter3;
		if (counter3 == 2) {
			counter3 = 0;
			updateSprites();
		}
		for (int i = 0; i < _spritesCount; ++i) {
			drawSprite(i);
			isSpeechSoundPlaying();
		}
		copyToVGA(_locationBackgroundGfxBuf);
		waitForTimer(3);
		isSpeechSoundPlaying();
		_timerCounter1 = 0;
		counter4 = _timerCounter2 / 3;
		if (counter4 == _creditsSequenceData1[num]) {
			_fadePaletteCounter = 0;
			_spritesCount = _creditsSequenceData2[num];
			for (int i = 0; i < _spritesCount; ++i) {
				memset(&_spritesTable[i], 0, sizeof(Sprite));
				_spritesTable[i].state = -1;
				_spritesTable[i].stateIndex = -1;
			}
			++num;
			switch (num) {
			case 1:
				strcpy(_fileToLoad, "loc75.pcx");
				break;
			case 2:
				strcpy(_fileToLoad, "loc76.pcx");
				break;
			case 3:
				strcpy(_fileToLoad, "paper-3.pcx");
				break;
			case 4:
				strcpy(_fileToLoad, "loc77.pcx");
				break;
			case 5:
				strcpy(_fileToLoad, "loc78.pcx");
				break;
			}
			if (num == 6) {
				for (int i = 0; i < 16; ++i) {
					sprintf(_fileToLoad, "cogs%04d.pcx", i);
					loadImage(imgBuf + i * 64000, 2);
				}
			} else {
				loadImage(_quadBackgroundGfxBuf, 2);
			}
			_spritesCount = _creditsSequenceData2[num];
			++_flagsTable[236];
		}
	} while (isSpeechSoundPlaying());
	free(imgBuf);
	_locationNum = prevLocationNum;
	do {
		if (_fadePaletteCounter > 0) {
			fadeInPalette();
			--_fadePaletteCounter;
		}
		copyToVGA(_locationBackgroundGfxBuf);
		waitForTimer(2);
	} while (_fadePaletteCounter > 0);
}

void TuckerEngine::handleCongratulationsSequence() {
	_timerCounter2 = 0;
	_fadePaletteCounter = 0;
	stopSounds();
	strcpy(_fileToLoad, "congrat.pcx");
	loadImage(_loadTempBuf, 1);
	Graphics::copyTo640(_locationBackgroundGfxBuf, _loadTempBuf, 320, 320, 200);
	copyToVGA(_locationBackgroundGfxBuf);
	while (_timerCounter2 < 450) {
		while (_fadePaletteCounter < 14) {
			++_fadePaletteCounter;
			fadeOutPalette();
		}
		waitForTimer(3);
		updateTimer();
	}
}

void TuckerEngine::handleNewPartSequence() {
	stopSounds();
	if (_flagsTable[219] == 1) {
		_flagsTable[219] = 0;
		for (int i = 0; i < 50; ++i) {
			_inventoryItemsState[i] = 0;
		}
		_inventoryObjectsOffset = 0;
		_inventoryObjectsCount = 0;
		addObjectToInventory(30);
		if (_partNum == 1 || _partNum == 3) {
			addObjectToInventory(1);
			addObjectToInventory(0);
		}
		_redrawPanelItemsCounter = 0;
	}
	_scrollOffset = 0;
	switch (_partNum) {
	case 1:
		strcpy(_fileToLoad, "pt1bak.pcx");
		break;
	case 2:
		strcpy(_fileToLoad, "pt2bak.pcx");
		break;
	default:
		strcpy(_fileToLoad, "pt3bak.pcx");
		break;
	}
	loadImage(_quadBackgroundGfxBuf, 1);
	switch (_partNum) {
	case 1:
		strcpy(_fileToLoad, "sprites/partone.spr");
		break;
	case 2:
		strcpy(_fileToLoad, "sprites/parttwo.spr");
		break;
	default:
		strcpy(_fileToLoad, "sprites/partthr.spr");
		break;
	}
	_spritesCount = 1;
	memset(&_spritesTable[0], 0, sizeof(Sprite));
	_spritesTable[0].state = -1;
	_spritesTable[0].stateIndex = -1;
	int currentLocation = _locationNum;
	_locationNum = 98;
	for (int i = 1; i < kSprA02TableSize; ++i) {
		free(_sprA02Table[i]);
		_sprA02Table[i] = 0;
	}
	for (int i = 1; i < kSprC02TableSize; ++i) {
		free(_sprC02Table[i]);
		_sprC02Table[i] = 0;
	}
	_sprC02Table[1] = loadFile();
	startSpeechSound(9000, 60);
	_fadePaletteCounter = 0;
	do {
		if (_fadePaletteCounter < 16) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		Graphics::copyTo640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf, 320, 320, 200);
		updateSprites();
		drawSprite(0);
		copyToVGA(_locationBackgroundGfxBuf);
		waitForTimer(3);
		if (getLastKeyCode() > 0) {
			stopSounds();
		}
	} while (isSpeechSoundPlaying());
	do {
		if (_fadePaletteCounter > 0) {
			fadeInPalette();
			--_fadePaletteCounter;
		}
		Graphics::copyTo640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf, 320, 320, 200);
		updateSprites();
		drawSprite(0);
		copyToVGA(_locationBackgroundGfxBuf);
		waitForTimer(3);
	} while (_fadePaletteCounter > 0);
	_locationNum = currentLocation;
}

void TuckerEngine::handleMeanwhileSequence() {
	backupPalette();
	switch (_partNum) {
	case 1:
		strcpy(_fileToLoad, "meanw01.pcx");
		break;
	case 2:
		strcpy(_fileToLoad, "meanw02.pcx");
		break;
	default:
		strcpy(_fileToLoad, "meanw03.pcx");
		break;
	}
	if (_flagsTable[215] == 0 && _flagsTable[231] == 1) {
		strcpy(_fileToLoad, "loc80.pcx");
	}
	loadImage(_quadBackgroundGfxBuf + 89600, 1);
	_fadePaletteCounter = 0;
	for (int i = 0; i < 60; ++i) {
		if (_fadePaletteCounter < 16) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		Graphics::copyTo640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf + 89600, 320, 320, 200);
		copyToVGA(_locationBackgroundGfxBuf);
		waitForTimer(3);
		++i;
	}
	do {
		if (_fadePaletteCounter > 0) {
			fadeInPalette();
			--_fadePaletteCounter;
		}
		Graphics::copyTo640(_locationBackgroundGfxBuf, _quadBackgroundGfxBuf + 89600, 320, 320, 200);
		copyToVGA(_locationBackgroundGfxBuf);
		waitForTimer(3);
	} while (_fadePaletteCounter > 0);
	restorePalette();
}

void TuckerEngine::handleMapSequence() {
	strcpy(_fileToLoad, "map2.pcx");
	loadImage(_quadBackgroundGfxBuf + 89600, 0);
	strcpy(_fileToLoad, "map1.pcx");
	loadImage(_loadTempBuf, 1);
	_selectedObject.locationObject_locationNum = 0;
	if (_flagsTable[7] > 0) {
		copyMapRect(0, 0, 140, 86);
	}
	if (_flagsTable[7] > 1) {
		copyMapRect(0, 60, 122, 120);
	}
	if (_flagsTable[7] > 2) {
		copyMapRect(122, 114, 97, 86);
	}
	if (_flagsTable[7] == 4) {
		copyMapRect(140, 0, 88, 125);
	}
	if (_flagsTable[120] == 1) {
		copyMapRect(220, 0, 100, 180);
	}
	_fadePaletteCounter = 0;
	int xPos, yPos, textNum = 0;
	do {
		waitForTimer(2);
		updateMouseState();
		Graphics::copyTo640(_locationBackgroundGfxBuf + _scrollOffset, _quadBackgroundGfxBuf + 89600, 320, 320, 200);
		if (_flagsTable[7] > 0 && _mousePosX > 30 && _mousePosX < 86 && _mousePosY > 36 && _mousePosY < 86) {
			textNum = 13;
			_nextLocationNum = (_partNum == 1) ? 3 : 65;
			xPos = 620;
			yPos = 130;
		} else if (_flagsTable[7] > 1 && _mousePosX > 60 && _mousePosX < 120 && _mousePosY > 120 && _mousePosY < 170) {
			textNum = 14;
			_nextLocationNum = (_partNum == 1) ? 9 : 66;
			xPos = 344;
			yPos = 120;
		} else if (_flagsTable[7] > 2 && _mousePosX > 160 && _mousePosX < 210 && _mousePosY > 110 && _mousePosY < 160) {
			textNum = 15;
			_nextLocationNum = (_partNum == 1) ? 16 : 61;
			xPos = 590;
			yPos = 130;
		} else if ((_flagsTable[7] == 4 || _flagsTable[7] == 6) && _mousePosX > 150 && _mousePosX < 200 && _mousePosY > 20 && _mousePosY < 70) {
			textNum = 16;
			_nextLocationNum = (_partNum == 1) ? 20 : 68;
			xPos = 20;
			yPos = 130;
		} else if (_flagsTable[120] == 1 && _mousePosX > 240 && _mousePosX < 290 && _mousePosY > 35 && _mousePosY < 90) {
			textNum = 17;
			_nextLocationNum = (_partNum == 1) ? 19 : 62;
			xPos = 20;
			yPos = 124;
		} else if (_mousePosX > 135 && _mousePosX < 185 && _mousePosY > 170 && _mousePosY < 200) {
			textNum = 18;
			_nextLocationNum = _locationNum;
			if (!_noPositionChangeAfterMap) {
				xPos = _xPosCurrent;
				yPos = _yPosCurrent;
			} else if (_locationNum == 3 ||_locationNum == 65) {
				xPos = 620;
				yPos = 130;
			} else if (_locationNum == 9 ||_locationNum == 66) {
				xPos = 344;
				yPos = 120;
			} else if (_locationNum == 16 ||_locationNum == 61) {
				xPos = 590;
				yPos = 130;
			} else if (_locationNum == 20 ||_locationNum == 68) {
				xPos = 20;
				yPos = 130;
			} else {
				xPos = 20;
				yPos = 124;
			}
		}
		if (textNum > 0) {
			drawSpeechText(_scrollOffset + _mousePosX + 8, _mousePosY - 10, _infoBarBuf, textNum, 96);
		}
		copyToVGA(_locationBackgroundGfxBuf + _scrollOffset);
		if (_fadePaletteCounter < 14) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
	} while (!_leftMouseButtonPressed || textNum == 0);
	while (_fadePaletteCounter > 0) {
		fadeInPalette();
		copyToVGA(_locationBackgroundGfxBuf + _scrollOffset);
		--_fadePaletteCounter;
	}
	_mouseClick = 1;
	if (_nextLocationNum == 9 && _noPositionChangeAfterMap) {
		_backgroundSpriteCurrentAnimation = 2;
		_backgroundSpriteCurrentFrame = 0;
		setCursorType(2);
	} else if (_nextLocationNum == 66 && _noPositionChangeAfterMap) {
		_backgroundSpriteCurrentAnimation = 1;
		_backgroundSpriteCurrentFrame = 0;
		setCursorType(2);
	}
	_noPositionChangeAfterMap = false;
	_xPosCurrent = xPos;
	_yPosCurrent = yPos;
}

void TuckerEngine::copyMapRect(int x, int y, int w, int h) {
	const uint8 *src = _quadBackgroundGfxBuf + 89600 + y * 320 + x;
	uint8 *dst = _loadTempBuf + y * 320 + x;
	for (int i = 0; i < h; ++i) {
		memcpy(dst, src, w);
		src += 320;
		dst += 320;
	}
}

int TuckerEngine::handleSpecialObjectSelectionSequence() {
	if (_partNum == 1 && _selectedObjectNum == 6) {
		strcpy(_fileToLoad, "news1.pcx");
		_flagsTable[7] = 1;
	} else if (_partNum == 3 && _selectedObjectNum == 45) {
		strcpy(_fileToLoad, "profnote.pcx");
	} else if (_partNum == 1 && _selectedObjectNum == 26) {
		strcpy(_fileToLoad, "photo.pcx");
	} else if (_partNum == 3 && _selectedObjectNum == 39) {
		strcpy(_fileToLoad, "news2.pcx");
		_flagsTable[135] = 1;
	} else if (_currentInfoString1SourceType == 0 && _currentActionObj1Num == 259) {
		strcpy(_fileToLoad, "postit.pcx");
	} else if (_currentInfoString1SourceType == 1 && _currentActionObj1Num == 91) {
		strcpy(_fileToLoad, "memo.pcx");
	} else {
		return 0;
	}
	while (_fadePaletteCounter > 0) {
		fadeInPalette();
		copyToVGA(_locationBackgroundGfxBuf + _scrollOffset);
		--_fadePaletteCounter;
	}
	_mouseClick = 1;
	loadImage(_quadBackgroundGfxBuf, 1);
	_fadePaletteCounter = 0;
	while (1) {
		waitForTimer(2);
		updateMouseState();
		Graphics::copyTo640(_locationBackgroundGfxBuf + _scrollOffset, _quadBackgroundGfxBuf, 320, 320, 200);
		if (_fadePaletteCounter < 14) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		if (!_leftMouseButtonPressed && _mouseClick == 1) {
			_mouseClick = 0;
		}
		if (_partNum == 3 && _selectedObjectNum == 45) {
			for (int i = 0; i < 13; ++i) {
				const int offset = _dataTable[204 + i].yDest * 640 + _dataTable[i].xDest;
				static const int itemsTable[] = { 15, 44, 25, 19, 21, 24, 12, 27, 20, 29, 35, 23, 3 };
				if (_inventoryItemsState[itemsTable[i]] > 1) {
					Graphics::decodeRLE(_locationBackgroundGfxBuf + _scrollOffset + offset, _data3GfxBuf + _dataTable[204 + i].sourceOffset, _dataTable[204 + i].xSize, _dataTable[204 + i].ySize);
				}
			}
		}
		copyToVGA(_locationBackgroundGfxBuf + _scrollOffset);
		if (_leftMouseButtonPressed && _mouseClick != 1) {
			while (_fadePaletteCounter > 0) {
				fadeInPalette();
				copyToVGA(_locationBackgroundGfxBuf + _scrollOffset);
				--_fadePaletteCounter;
			}
			_mouseClick = 1;
			break;
		}
	}
	loadLoc();
	return 1;
}

} // namespace Tucker
