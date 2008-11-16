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

#include "common/system.h"

#include "sound/audiostream.h"
#include "sound/wave.h"

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
			if (_inputKeys[kInputKeyEscape]) {
				_inputKeys[kInputKeyEscape] = false;
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
		}
		copyToVGA(_locationBackgroundGfxBuf);
		waitForTimer(3);
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
	unloadSprA02_01();
	unloadSprC02_01();
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
		if (_inputKeys[kInputKeyEscape]) {
			_inputKeys[kInputKeyEscape] = false;
			break;
		}
	} while (isSpeechSoundPlaying());
	stopSpeechSound();
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
	uint8 backupPalette[256 * 3];
	memcpy(backupPalette, _currentPalette, 256 * 3);
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
	memcpy(_currentPalette, backupPalette, 256 * 3);
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
	int xPos = 0, yPos = 0, textNum = 0;
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
			} else if (_locationNum == 3 || _locationNum == 65) {
				xPos = 620;
				yPos = 130;
			} else if (_locationNum == 9 || _locationNum == 66) {
				xPos = 344;
				yPos = 120;
			} else if (_locationNum == 16 || _locationNum == 61) {
				xPos = 590;
				yPos = 130;
			} else if (_locationNum == 20 || _locationNum == 68) {
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

AnimationSequencePlayer::AnimationSequencePlayer(OSystem *system, Audio::Mixer *mixer, Common::EventManager *event, int num)
	: _system(system), _mixer(mixer), _event(event), _seqNum(num), _currentSeqNum(0) {
	_newSeq = false;
	memset(_animationPalette, 0, sizeof(_animationPalette));
	memset(_paletteBuffer, 0, sizeof(_paletteBuffer));
	memset(_flicPlayer, 0, sizeof(_flicPlayer));
	_soundsListSeqData = 0;
	_soundsList1 = 0;
	_soundsList1Count = 0;
	_soundsList2 = 0;
	_soundsList2Count = 0;
	_musicVolume = 0;
	_offscreenBuffer = (uint8 *)malloc(kScreenWidth * kScreenHeight);
	_updateScreenWidth = 0;
	_updateScreenPicture = 0;
	_updateScreenOffset = 0;
	_frameCounter = 0;
	_frameTime = 1;
	_lastFrameTime = 0;
	_picBufPtr = _pic2BufPtr = 0;
}

AnimationSequencePlayer::~AnimationSequencePlayer() {
	unloadAnimation();
	free(_offscreenBuffer);
}

void AnimationSequencePlayer::mainLoop() {
	do {
		if (_seqNum != _currentSeqNum) {
			unloadAnimation();
			_currentSeqNum = _seqNum;
			_newSeq = true;
			_frameCounter = 0;
			_lastFrameTime = _system->getMillis();
		}
		switch (_seqNum) {
		case 17:
			introSeq17_18();
			break;
		case 19:
			introSeq19_20();
			break;
		case 3:
			introSeq3_4();
			break;
		case 9:
			introSeq9_10();
			break;
		case 21:
			introSeq21_22();
			break;
		case 13:
			introSeq13_14();
			break;
		case 15:
			introSeq15_16();
			break;
		case 27:
			introSeq27_28();
			break;
		}
		_system->copyRectToScreen(_offscreenBuffer, 320, 0, 0, kScreenWidth, kScreenHeight);
		_system->setPalette(_animationPalette, 0, 256);
		_system->updateScreen();
		syncTime();
	} while (_seqNum != 1);
}

void AnimationSequencePlayer::syncTime() {
	uint32 end = _lastFrameTime + kSequenceFrameTime * _frameTime;
	do {
		Common::Event ev;
		while (_event->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_KEYDOWN:
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_seqNum = 1;
				}
				break;
			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				_seqNum = 1;
				break;
			default:
				break;
			}
		}
		_system->delayMillis(10);
		_lastFrameTime = _system->getMillis();
	} while (_lastFrameTime <= end);
}

Audio::AudioStream *AnimationSequencePlayer::loadSoundFileAsStream(const char *name, AnimationSoundType type) {
	Audio::AudioStream *stream = 0;
	char fileName[64];
	snprintf(fileName, sizeof(fileName), "audio/%s", name);
	Common::File f;
	if (f.open(fileName)) {
		uint8 *sampleData = 0;
		int size = 0, rate = 0;
		uint8 flags = 0;
		switch (type) {
		case kAnimationSoundType8BitsRAW:
		case kAnimationSoundType16BitsRAW:
			size = f.size();
			rate = 22050;
			flags = Audio::Mixer::FLAG_UNSIGNED;
			if (type == kAnimationSoundType16BitsRAW) {
				flags = Audio::Mixer::FLAG_LITTLE_ENDIAN | Audio::Mixer::FLAG_16BITS;
			}
			break;
		case kAnimationSoundTypeWAV:
		case kAnimationSoundTypeLoopingWAV:
			Audio::loadWAVFromStream(f, size, rate, flags);
			if (type == kAnimationSoundTypeLoopingWAV) {
				flags |= Audio::Mixer::FLAG_LOOP;
			}
			break;
		}
		if (size != 0) {
			sampleData = (uint8 *)malloc(size);
			if (sampleData) {
				f.read(sampleData, size);
				flags |= Audio::Mixer::FLAG_AUTOFREE;
				stream = Audio::makeLinearInputStream(sampleData, size, rate, flags, 0, 0);
			}
		}
	}
	return stream;
}

void AnimationSequencePlayer::loadSounds(int type, int num) {
	int index = 0;
	switch (type) {
	case 1:
		switch (num) {
		case 0:
			index = 1;
			_soundsList1Count = 14;
			_soundsList1 = _soundFilesList1;
			_soundsList2Count = 10;
			_soundsList2 = _soundFilesList2;
			_soundsListSeqData = _soundSeqData2;
			break;
		case 1:
			index = 1;
			_soundsList1Count = 14;
			_soundsList1 = _soundFilesList3;
			_soundsList2Count = 5;
			_soundsList2 = _soundFilesList4;
			_soundsListSeqData = _soundSeqData3;
			break;
		case 2:
			index = 1;
			_soundsList1Count = 14;
			_soundsList1 = _soundFilesList5;
			_soundsList2Count = 9;
			_soundsList2 = _soundFilesList6;
			_soundsListSeqData = _soundSeqData4;
			break;
		}
		_musicVolume = 0;
		break;
	case 3:
		switch (num) {
		case 0:
			index = 3;
			_soundsList1Count = 13;
			_soundsList1 = _soundFilesList7;
			_soundsList2Count = 0;
			_soundsListSeqData = _soundSeqData5;
			break;
		case 1:
			index = 6;
			_soundsList1Count = 14;
			_soundsList1 = _soundFilesList8;
			_soundsList2Count = 4;
			_soundsList2 = _soundFilesList9;
			_soundsListSeqData = _soundSeqData6;
			break;
		case 2:
			index = 7;
			_soundsList1Count = 13;
			_soundsList1 = _soundFilesList10;
			_soundsList2Count = 9;
			_soundsList2 = _soundFilesList11;
			_soundsListSeqData = _soundSeqData7;
			break;
		case 3:
			index = 10;
			_soundsList1Count = 11;
			_soundsList1 = _soundFilesList12;
			_soundsList2Count = 0;
			_soundsListSeqData = _soundSeqData8;
			break;
		}
		_musicVolume = 80;
		break;
	case 4:
		switch (num) {
		case 0:
			index = 4;
			_soundsList1Count = 6;
			_soundsList1 = _soundFilesList13;
			_soundsList2Count = 0;
			_soundsListSeqData = _soundSeqData9;
			break;
		case 1:
			index = 9;
			_soundsList1Count = 10;
			_soundsList1 = _soundFilesList14;
			_soundsList2Count = 0;
			_soundsListSeqData = _soundSeqData10;
			break;
		}
		_musicVolume = 80;
		break;
	case 5:
		index = 5;
		_soundsList1Count = 0;
		_soundsList2Count = 0;
		_soundsListSeqData = _soundSeqData1;
		_musicVolume = 100;
		break;
	case 9:
		index = 8;
		_soundsList1Count = 0;
		_soundsList2Count = 0;
		_soundsListSeqData = _soundSeqData1;
		_musicVolume = 100;
		break;
	case 10:
		index = 0;
		_soundsList1Count = 4;
		_soundsList1 = _soundFilesList15;
		_soundsList2Count = 0;
		_soundsListSeqData = _soundSeqData11;
		_musicVolume = 100;
		break;
	default:
		index = 0;
		_soundsList1Count = 0;
		_soundsList2Count = 0;
		_soundsListSeqData = _soundSeqData1;
		_musicVolume = 100;
		break;
	}
	if (_musicVolume != 0) {
		Audio::AudioStream *s;
		if ((s = loadSoundFileAsStream(_musicFileNamesTable[index], (type == 5) ? kAnimationSoundType16BitsRAW : kAnimationSoundType8BitsRAW)) != 0) {
			_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, s, -1, _musicVolume * Audio::Mixer::kMaxChannelVolume / 100);
		}
	}
}

void AnimationSequencePlayer::updateSounds() {
	Audio::AudioStream *s = 0;
	int index;
	const int *p = _soundsListSeqData;
	while (p[0] != -1 && p[0] <= _frameCounter) {
		switch (p[2]) {
		case 0:
			if ((index = p[1]) < _soundsList1Count) {
				if ((s = loadSoundFileAsStream(_soundsList1[index], kAnimationSoundTypeWAV)) != 0) {
					_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_soundsHandle[index], s, -1, p[3] * Audio::Mixer::kMaxChannelVolume / 100);
				}
			}
			break;
		case 1:
			if ((index = p[1]) < _soundsList1Count) {
				if ((s = loadSoundFileAsStream(_soundsList1[index], kAnimationSoundTypeLoopingWAV)) != 0) {
					_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_soundsHandle[index], s, -1, p[3] * Audio::Mixer::kMaxChannelVolume / 100);
				}
			}
			break;
		case 2:
			if ((index = p[1]) < _soundsList1Count) {
				_mixer->stopHandle(_soundsHandle[index]);
			}
			break;
		case 3:
			_mixer->stopHandle(_musicHandle);
			break;
		case 4:
			_mixer->stopHandle(_musicHandle);
			index = p[1];
			if ((s = loadSoundFileAsStream(_musicFileNamesTable[index], kAnimationSoundType8BitsRAW)) != 0) {
				_musicVolume = p[3];
				_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, s, -1, _musicVolume * Audio::Mixer::kMaxChannelVolume / 100);
			}
			break;
		case 5:
			if ((index = p[1]) < _soundsList2Count) {
				if ((s = loadSoundFileAsStream(_soundsList2[index], kAnimationSoundTypeWAV)) != 0) {
					_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, s, -1, p[3] * Audio::Mixer::kMaxChannelVolume / 100);
				}
			}
			break;
		case 6:
			_mixer->stopHandle(_musicHandle);
			index = p[1];
			if ((s = loadSoundFileAsStream(_musicFileNamesTable[index], kAnimationSoundType16BitsRAW)) != 0) {
				_musicVolume = p[3];
				_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, s, -1, _musicVolume * Audio::Mixer::kMaxChannelVolume / 100);
			}
			break;
		default:
			warning("Unhandled sound opcode %d (%d,%d)", p[2], _frameCounter, p[0]);
			break;
		}
		p += 4;
	}
	_soundsListSeqData = p;
}

void AnimationSequencePlayer::fadeInPalette() {
	memset(_paletteBuffer, 0, sizeof(_paletteBuffer));
	bool fadeColors = true;
	for (int step = 0; step < 64; ++step) {
		if (fadeColors) {
			fadeColors = false;
			for (int i = 0; i < 1024; ++i) {
				if ((i & 3) != 3 && _paletteBuffer[i] < _animationPalette[i]) {
					const int color = _paletteBuffer[i] + 4;
					_paletteBuffer[i] = MIN<int>(color, _animationPalette[i]);
					fadeColors = true;
				}
			}
			_system->setPalette(_paletteBuffer, 0, 256);
			_system->updateScreen();
		}
		_system->delayMillis(1000 / 60);
	}
}

void AnimationSequencePlayer::fadeOutPalette() {
	memcpy(_paletteBuffer, _animationPalette, 1024);
	bool fadeColors = true;
	for (int step = 0; step < 64; ++step) {
		if (fadeColors) {
			fadeColors = false;
			for (int i = 0; i < 1024; ++i) {
				if ((i & 3) != 3 && _paletteBuffer[i] > 0) {
					const int color = _paletteBuffer[i] - 4;
					_paletteBuffer[i] = MAX<int>(0, color);
					fadeColors = true;
				}
			}
			_system->setPalette(_paletteBuffer, 0, 256);
			_system->updateScreen();
		}
		_system->delayMillis(1000 / 60);
	}
	_system->clearScreen();
}

void AnimationSequencePlayer::unloadAnimation() {
	_mixer->stopAll();
	for (int i = 0; i < ARRAYSIZE(_flicPlayer); ++i) {
		delete _flicPlayer[i];
		_flicPlayer[i] = 0;
	}
	free(_picBufPtr);
	_picBufPtr = 0;
	free(_pic2BufPtr);
	_pic2BufPtr = 0;
}

uint8 *AnimationSequencePlayer::loadPicture(const char *fileName) {
	uint8 *p = 0;
	Common::File f;
	if (f.open(fileName)) {
		const int sz = f.size();
		p = (uint8 *)malloc(sz);
		if (p) {
			f.read(p, sz);
		}
	}
	return p;
}

void AnimationSequencePlayer::openAnimation(int index, const char *fileName) {
	_flicPlayer[index] = new ::Graphics::FlicPlayer(fileName);
	_flicPlayer[index]->decodeFrame();
	if (index == 0) {
		memcpy(_animationPalette, _flicPlayer[index]->getPalette(), 1024);
		memcpy(_offscreenBuffer, _flicPlayer[index]->getOffscreen(), kScreenWidth * kScreenHeight);
	}
}

void AnimationSequencePlayer::decodeNextAnimationFrame(int index) {
	_flicPlayer[index]->decodeFrame();
	memcpy(_offscreenBuffer, _flicPlayer[index]->getOffscreen(), kScreenWidth * kScreenHeight);
	if (index == 0) {
		if (_flicPlayer[index]->isPaletteDirty()) {
			memcpy(_animationPalette, _flicPlayer[index]->getPalette(), 1024);
		}
	}
	if (_seqNum != 19) {
		++_frameCounter;
	}
}

void AnimationSequencePlayer::introSeq17_18() {
	if (_newSeq) {
		loadSounds(9, 0);
		openAnimation(0, "graphics/merit.flc");
		_frameTime = 1;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0]->getCurFrame() == _flicPlayer[0]->getFrameCount()) {
		_seqNum = 19;
	}
	updateSounds();
}

void AnimationSequencePlayer::introSeq19_20() {
	if (_newSeq) {
		fadeOutPalette();
		loadSounds(10, 1);
		openAnimation(0, "graphics/budttle2.flc");
		openAnimation(1, "graphics/machine.flc");
		_frameTime = 1;
		_newSeq = false;
	}
	if (_flicPlayer[0]->getCurFrame() >= 116) {
		_flicPlayer[1]->decodeFrame();
		if (_flicPlayer[1]->getCurFrame() == _flicPlayer[1]->getFrameCount()) {
			_flicPlayer[1]->reset();
		}
	}
	_flicPlayer[0]->decodeFrame();
	const uint8 *t = _flicPlayer[1]->getOffscreen();
	for (int i = 0; i < 64000; ++i) {
		const uint8 color = _flicPlayer[0]->getOffscreen()[i];
		_offscreenBuffer[i] = color ? color : t[i];
	}
	updateSounds();
	if (_flicPlayer[0]->getCurFrame() == _flicPlayer[0]->getFrameCount()) {
		_seqNum = 3;
	}
}

void AnimationSequencePlayer::displayLoadingScreen() {
	Common::File f;
	if (f.open("graphics/loading.pic")) {
		fadeOutPalette();
		f.seek(32);
		for (int i = 0; i < 1024; i += 4) {
			f.read(_animationPalette + i, 3);
		}
		f.read(_offscreenBuffer, 64000);
		_system->copyRectToScreen(_offscreenBuffer, 320, 0, 0, kScreenWidth, kScreenHeight);
		fadeInPalette();
	}
}

void AnimationSequencePlayer::initPicPart4() {
	_updateScreenWidth = 320;
	_updateScreenPicture = 1;
	_updateScreenOffset = 0;
}

void AnimationSequencePlayer::drawPicPart4() {
	static const uint8 offsetsTable[77] = {
		1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4,
		5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3,
		3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1
	};
	_updateScreenWidth = _updateScreenWidth - offsetsTable[_updateScreenOffset];
	++_updateScreenOffset;
	for (int y = 0; y < 200; ++y) {
		memcpy(_offscreenBuffer + y * 320, _picBufPtr + 800 + y * 640 + _updateScreenWidth, 320);
	}
	if (_updateScreenWidth == 0) {
		_updateScreenPicture = false;
	}
}

void AnimationSequencePlayer::introSeq3_4() {
	if (_newSeq) {
		displayLoadingScreen();
		loadSounds(1, 0);
		_picBufPtr = loadPicture("graphics/house.pic");
		openAnimation(0, "graphics/intro1.flc");
		_system->copyRectToScreen(_offscreenBuffer, 320, 0, 0, kScreenWidth, kScreenHeight);
		fadeInPalette();
		_updateScreenPicture = false;
		_frameTime = 2;
		_newSeq = false;
	}
	if (!_updateScreenPicture) {
		decodeNextAnimationFrame(0);
		if (_flicPlayer[0]->getCurFrame() == 706) {
			initPicPart4();
		}
		if (_flicPlayer[0]->getCurFrame() == _flicPlayer[0]->getFrameCount()) {
			_seqNum = 9;
		}
	} else {
		drawPicPart4();
	}
	updateSounds();
}

void AnimationSequencePlayer::drawPic2Part10() {
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 64; ++x) {
			const uint8 color = _pic2BufPtr[y * 64 + x];
			if (color != 0) {
				_picBufPtr[89417 + y * 640 + x] = color;
			}
		}
	}
	for (int y = 0; y < 80; ++y) {
		for (int x = 0; x < 48; ++x) {
			const uint8 color = _pic2BufPtr[1024 + y * 48 + x];
			if (color != 0) {
				_picBufPtr[63939 + y * 640 + x] = color;
			}
		}
	}
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 80; ++x) {
			const uint8 color = _pic2BufPtr[7424 + y * 80 + x];
			if (color != 0) {
				_picBufPtr[33067 + y * 640 + x] = color;
			}
		}
	}
}

void AnimationSequencePlayer::drawPic1Part10() {
	for (int y = 0; y < 200; ++y) {
		memcpy(_offscreenBuffer + y * 320, _picBufPtr + 800 + y * 640 + _updateScreenWidth, 320);
	}
	for (int i = 0; i < 64000; ++i) {
		const uint8 color = _flicPlayer[0]->getOffscreen()[i];
		if (color) {
			_offscreenBuffer[i] = color;
		}
	}
}

void AnimationSequencePlayer::introSeq9_10() {
	if (_newSeq) {
		loadSounds(1, 1);
		_pic2BufPtr = loadPicture("graphics/bits.pic");
		_picBufPtr = loadPicture("graphics/lab.pic");
		openAnimation(0, "graphics/intro2.flc");
		_updateScreenWidth = 0;
		_frameTime = 2;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0]->getCurFrame() == 984) {
		drawPic2Part10();
	}
	if (_flicPlayer[0]->getCurFrame() >= 264 && _flicPlayer[0]->getCurFrame() <= 295) {
		drawPic1Part10();
		_updateScreenWidth += 6;
	} else if (_flicPlayer[0]->getCurFrame() >= 988 && _flicPlayer[0]->getCurFrame() <= 996) {
		drawPic1Part10();
		_updateScreenWidth -= 25;
		if (_updateScreenWidth < 0) {
			_updateScreenWidth = 0;
		}
	}
	if (_flicPlayer[0]->getCurFrame() == _flicPlayer[0]->getFrameCount()) {
		_seqNum = 21;
	}
	updateSounds();
}

void AnimationSequencePlayer::introSeq21_22() {
	if (_newSeq) {
		loadSounds(1, 2);
		openAnimation(0, "graphics/intro3.flc");
		_frameTime = 2;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0]->getCurFrame() == _flicPlayer[0]->getFrameCount()) {
		_seqNum = 1;
	}
	updateSounds();
}

void AnimationSequencePlayer::introSeq13_14() {
	if (_newSeq) {
		loadSounds(3, 1);
		openAnimation(0, "graphics/allseg02.flc");
		_frameTime = 2;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0]->getCurFrame() == _flicPlayer[0]->getFrameCount()) {
		_seqNum = 15;
	}
	updateSounds();
}

void AnimationSequencePlayer::introSeq15_16() {
	if (_newSeq) {
		loadSounds(3, 2);
		openAnimation(0, "graphics/allseg03.flc");
		_frameTime = 2;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0]->getCurFrame() == _flicPlayer[0]->getFrameCount()) {
		_seqNum = 27;
	}
	updateSounds();
}

void AnimationSequencePlayer::introSeq27_28() {
	if (_newSeq) {
		loadSounds(3, 3);
		openAnimation(0, "graphics/allseg04.flc");
		_frameTime = 2;
		_newSeq = false;
	}
	decodeNextAnimationFrame(0);
	if (_flicPlayer[0]->getCurFrame() == _flicPlayer[0]->getFrameCount()) {
		_seqNum = 1;
	}
	updateSounds();
}

} // namespace Tucker
