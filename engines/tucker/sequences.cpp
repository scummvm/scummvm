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

void TuckerEngine::handleIntroSequence() {
	const int firstSequence = (_gameFlags & kGameFlagDemo) != 0 ? kFirstAnimationSequenceDemo : kFirstAnimationSequenceGame;
	_player = new AnimationSequencePlayer(_system, _mixer, _eventMan, &_compressedSound, firstSequence);
	_player->mainLoop();
	delete _player;
	_player = 0;
}

void TuckerEngine::handleCreditsSequence() {
	static const int _creditsSequenceTimecounts[] = { 200, 350, 650, 850, 1150, 1450, 12000 };
	static const int _creditsSequenceSpriteCounts[] = { 1, 1, 5, 0, 6, 6, 0 };
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
	_flagsTable[236] = 74;
	uint8 *imgBuf = (uint8 *)malloc(16 * 64000);
	loadSprC02_01();
	_spritesCount = _creditsSequenceSpriteCounts[num];
	for (int i = 0; i < _spritesCount; ++i) {
		memset(&_spritesTable[i], 0, sizeof(Sprite));
		_spritesTable[i].state = -1;
		_spritesTable[i].stateIndex = -1;
	}
	loadFile("credits.txt", _ptTextBuf);
	loadImage("loc74.pcx", _quadBackgroundGfxBuf, 1);
	startSpeechSound(9001, 120);
	_timerCounter2 = 0;
	_fadePaletteCounter = 0;
	do {
		if (_fadePaletteCounter < 16) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		if (counter4 + 20 > _creditsSequenceTimecounts[num]) {
			fadeInPalette();
		}
		++imgNum;
		if (imgNum == 16) {
			imgNum = 0;
		}
		if (num < 6) {
			Graphics::copyRect(_locationBackgroundGfxBuf, 640, _quadBackgroundGfxBuf, 320, 320, 200);
		} else {
			Graphics::copyRect(_locationBackgroundGfxBuf, 640, imgBuf + imgNum * 64000, 320, 320, 200);
			static const int yPosTable[] = { 48, 60, 80, 92, 140, 116 };
			for (int i = 0; i < 6; ++i) {
				drawCreditsString(5, yPosTable[i], counter2 * 6 + i);
			}
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
		_fullRedraw = true;
		++counter3;
		if (counter3 == 2) {
			counter3 = 0;
			updateSprites();
		}
		for (int i = 0; i < _spritesCount; ++i) {
			drawSprite(i);
		}
		redrawScreen(0);
		waitForTimer(3);
		_timerCounter1 = 0;
		counter4 = _timerCounter2 / 3;
		if (counter4 == _creditsSequenceTimecounts[num]) {
			_fadePaletteCounter = 0;
			_spritesCount = _creditsSequenceSpriteCounts[num];
			for (int i = 0; i < _spritesCount; ++i) {
				memset(&_spritesTable[i], 0, sizeof(Sprite));
				_spritesTable[i].state = -1;
				_spritesTable[i].stateIndex = -1;
			}
			++num;
			char filename[40];
			if (num == 6) {
				for (int i = 0; i < 16; ++i) {
					sprintf(filename, "cogs%04d.pcx", i);
					loadImage(filename, imgBuf + i * 64000, 2);
				}
			} else {
				switch (num) {
				case 1:
					strcpy(filename, "loc75.pcx");
					break;
				case 2:
					strcpy(filename, "loc76.pcx");
					break;
				case 3:
					strcpy(filename, "paper-3.pcx");
					break;
				case 4:
					strcpy(filename, "loc77.pcx");
					break;
				case 5:
					strcpy(filename, "loc78.pcx");
					break;
				}
				loadImage(filename, _quadBackgroundGfxBuf, 2);
			}
			_spritesCount = _creditsSequenceSpriteCounts[num];
			++_flagsTable[236];
		}
	} while (!_quitGame && isSpeechSoundPlaying());
	free(imgBuf);
	_locationNum = prevLocationNum;
	do {
		if (_fadePaletteCounter > 0) {
			fadeInPalette();
			--_fadePaletteCounter;
		}
		redrawScreen(0);
		waitForTimer(2);
	} while (_fadePaletteCounter > 0);
}

void TuckerEngine::handleCongratulationsSequence() {
	_timerCounter2 = 0;
	_fadePaletteCounter = 0;
	stopSounds();
	loadImage("congrat.pcx", _loadTempBuf, 1);
	Graphics::copyRect(_locationBackgroundGfxBuf, 640, _loadTempBuf, 320, 320, 200);
	_fullRedraw = true;
	redrawScreen(0);
	while (!_quitGame && _timerCounter2 < 450) {
		while (_fadePaletteCounter < 14) {
			++_fadePaletteCounter;
			fadeOutPalette();
		}
		waitForTimer(3);
	}
}

void TuckerEngine::handleNewPartSequence() {
	char filename[40];

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
		strcpy(filename, "pt1bak.pcx");
		break;
	case 2:
		strcpy(filename, "pt2bak.pcx");
		break;
	default:
		strcpy(filename, "pt3bak.pcx");
		break;
	}
	loadImage(filename, _quadBackgroundGfxBuf, 1);
	_spritesCount = 1;
	memset(&_spritesTable[0], 0, sizeof(Sprite));
	_spritesTable[0].state = -1;
	_spritesTable[0].stateIndex = -1;
	int currentLocation = _locationNum;
	_locationNum = 98;
	unloadSprA02_01();
	unloadSprC02_01();
	switch (_partNum) {
	case 1:
		strcpy(filename, "sprites/partone.spr");
		break;
	case 2:
		strcpy(filename, "sprites/parttwo.spr");
		break;
	default:
		strcpy(filename, "sprites/partthr.spr");
		break;
	}
	_sprC02Table[1] = loadFile(filename, 0);
	startSpeechSound(9000, 60);
	_fadePaletteCounter = 0;
	do {
		if (_fadePaletteCounter < 16) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		Graphics::copyRect(_locationBackgroundGfxBuf, 640, _quadBackgroundGfxBuf, 320, 320, 200);
		_fullRedraw = true;
		updateSprites();
		drawSprite(0);
		redrawScreen(0);
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
		Graphics::copyRect(_locationBackgroundGfxBuf, 640, _quadBackgroundGfxBuf, 320, 320, 200);
		_fullRedraw = true;
		updateSprites();
		drawSprite(0);
		redrawScreen(0);
		waitForTimer(3);
	} while (_fadePaletteCounter > 0);
	_locationNum = currentLocation;
}

void TuckerEngine::handleMeanwhileSequence() {
	char filename[40];
	uint8 backupPalette[256 * 3];
	memcpy(backupPalette, _currentPalette, 256 * 3);
	switch (_partNum) {
	case 1:
		strcpy(filename, "meanw01.pcx");
		break;
	case 2:
		strcpy(filename, "meanw02.pcx");
		break;
	default:
		strcpy(filename, "meanw03.pcx");
		break;
	}
	if (_flagsTable[215] == 0 && _flagsTable[231] == 1) {
		strcpy(filename, "loc80.pcx");
	}
	loadImage(filename, _quadBackgroundGfxBuf + 89600, 1);
	_fadePaletteCounter = 0;
	for (int i = 0; i < 60; ++i) {
		if (_fadePaletteCounter < 16) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		Graphics::copyRect(_locationBackgroundGfxBuf, 640, _quadBackgroundGfxBuf + 89600, 320, 320, 200);
		_fullRedraw = true;
		redrawScreen(0);
		waitForTimer(3);
		++i;
	}
	do {
		if (_fadePaletteCounter > 0) {
			fadeInPalette();
			--_fadePaletteCounter;
		}
		Graphics::copyRect(_locationBackgroundGfxBuf, 640, _quadBackgroundGfxBuf + 89600, 320, 320, 200);
		_fullRedraw = true;
		redrawScreen(0);
		waitForTimer(3);
	} while (_fadePaletteCounter > 0);
	memcpy(_currentPalette, backupPalette, 256 * 3);
	_fullRedraw = true;
}

void TuckerEngine::handleMapSequence() {
	loadImage("map2.pcx", _quadBackgroundGfxBuf + 89600, 0);
	loadImage("map1.pcx", _loadTempBuf, 1);
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
	while (!_quitGame) {
		waitForTimer(2);
		updateMouseState();
		Graphics::copyRect(_locationBackgroundGfxBuf + _scrollOffset, 640, _quadBackgroundGfxBuf + 89600, 320, 320, 200);
		_fullRedraw = true;
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
		redrawScreen(_scrollOffset);
		if (_fadePaletteCounter < 14) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		if (_leftMouseButtonPressed && textNum != 0) {
			break;
		}
	}
	while (_fadePaletteCounter > 0) {
		fadeInPalette();
		redrawScreen(_scrollOffset);
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
	const uint8 *src = _loadTempBuf + y * 320 + x;
	uint8 *dst = _quadBackgroundGfxBuf + 89600 + y * 320 + x;
	for (int i = 0; i < h; ++i) {
		memcpy(dst, src, w);
		src += 320;
		dst += 320;
	}
}

int TuckerEngine::handleSpecialObjectSelectionSequence() {
	char filename[40];
	if (_partNum == 1 && _selectedObjectNum == 6) {
		strcpy(filename, "news1.pcx");
		_flagsTable[7] = 4;
	} else if (_partNum == 3 && _selectedObjectNum == 45) {
		strcpy(filename, "profnote.pcx");
	} else if (_partNum == 1 && _selectedObjectNum == 26) {
		strcpy(filename, "photo.pcx");
	} else if (_partNum == 3 && _selectedObjectNum == 39) {
		strcpy(filename, "news2.pcx");
		_flagsTable[135] = 1;
	} else if (_currentInfoString1SourceType == 0 && _currentActionObj1Num == 259) {
		strcpy(filename, "postit.pcx");
	} else if (_currentInfoString1SourceType == 1 && _currentActionObj1Num == 91) {
		strcpy(filename, "memo.pcx");
	} else {
		return 0;
	}
	while (_fadePaletteCounter > 0) {
		fadeInPalette();
		redrawScreen(_scrollOffset);
		--_fadePaletteCounter;
	}
	_mouseClick = 1;
	loadImage(filename, _quadBackgroundGfxBuf, 1);
	_fadePaletteCounter = 0;
	while (!_quitGame) {
		waitForTimer(2);
		updateMouseState();
		Graphics::copyRect(_locationBackgroundGfxBuf + _scrollOffset, 640, _quadBackgroundGfxBuf, 320, 320, 200);
		_fullRedraw = true;
		if (_fadePaletteCounter < 14) {
			fadeOutPalette();
			++_fadePaletteCounter;
		}
		if (!_leftMouseButtonPressed && _mouseClick == 1) {
			_mouseClick = 0;
		}
		if (_partNum == 3 && _selectedObjectNum == 45) {
			for (int i = 0; i < 13; ++i) {
				const int offset = _dataTable[204 + i].yDest * 640 + _dataTable[204 + i].xDest;
				static const int itemsTable[] = { 15, 44, 25, 19, 21, 24, 12, 27, 20, 29, 35, 23, 3 };
				if (_inventoryItemsState[itemsTable[i]] > 1) {
					Graphics::decodeRLE(_locationBackgroundGfxBuf + _scrollOffset + offset, _data3GfxBuf + _dataTable[204 + i].sourceOffset, _dataTable[204 + i].xSize, _dataTable[204 + i].ySize);
				}
			}
		}
		redrawScreen(_scrollOffset);
		if (_leftMouseButtonPressed && _mouseClick != 1) {
			while (_fadePaletteCounter > 0) {
				fadeInPalette();
				redrawScreen(_scrollOffset);
				--_fadePaletteCounter;
			}
			_mouseClick = 1;
			break;
		}
	}
	loadLoc();
	return 1;
}

AnimationSequencePlayer::AnimationSequencePlayer(OSystem *system, Audio::Mixer *mixer, Common::EventManager *event, CompressedSound *sound, int num)
	: _system(system), _mixer(mixer), _event(event), _compressedSound(sound), _seqNum(num) {
	memset(_animationPalette, 0, sizeof(_animationPalette));
	_soundSeqDataCount = 0;
	_soundSeqDataIndex = 0;
	_soundSeqData = 0;
	_offscreenBuffer = (uint8 *)malloc(kScreenWidth * kScreenHeight);
	_updateScreenWidth = 0;
	_updateScreenPicture = false;
	_updateScreenOffset = 0;
	_picBufPtr = _pic2BufPtr = 0;
}

AnimationSequencePlayer::~AnimationSequencePlayer() {
	unloadAnimation();
	free(_offscreenBuffer);
}

void AnimationSequencePlayer::mainLoop() {
	static const SequenceUpdateFunc _demoSeqUpdateFuncs[] = {
		{ 13, 2, &AnimationSequencePlayer::loadIntroSeq13_14, &AnimationSequencePlayer::playIntroSeq13_14 },
		{ 15, 2, &AnimationSequencePlayer::loadIntroSeq15_16, &AnimationSequencePlayer::playIntroSeq15_16 },
		{ 27, 2, &AnimationSequencePlayer::loadIntroSeq27_28, &AnimationSequencePlayer::playIntroSeq27_28 },
		{  1, 0, 0, 0 }
	};
	static const SequenceUpdateFunc _gameSeqUpdateFuncs[] = {
		{ 17, 1, &AnimationSequencePlayer::loadIntroSeq17_18, &AnimationSequencePlayer::playIntroSeq17_18 },
		{ 19, 1, &AnimationSequencePlayer::loadIntroSeq19_20, &AnimationSequencePlayer::playIntroSeq19_20 },
		{  3, 2, &AnimationSequencePlayer::loadIntroSeq3_4,   &AnimationSequencePlayer::playIntroSeq3_4   },
		{  9, 2, &AnimationSequencePlayer::loadIntroSeq9_10,  &AnimationSequencePlayer::playIntroSeq9_10  },
		{ 21, 2, &AnimationSequencePlayer::loadIntroSeq21_22, &AnimationSequencePlayer::playIntroSeq21_22 },
		{  1, 0, 0, 0 }
	};
	switch (_seqNum) {
	case kFirstAnimationSequenceDemo:
		_updateFunc = _demoSeqUpdateFuncs;
		break;
	case kFirstAnimationSequenceGame:
		_updateFunc = _gameSeqUpdateFuncs;
		break;
	}
	_updateFuncIndex = 0;
	_changeToNextSequence = true;
	do {
		if (_changeToNextSequence) {
			_changeToNextSequence = false;
			_frameCounter = 0;
			_lastFrameTime = _system->getMillis();
			_frameTime = this->_updateFunc[_updateFuncIndex].frameTime;
			(this->*(_updateFunc[_updateFuncIndex].load))();
			if (_seqNum == 1) {
				break;
			}
		}
		(this->*(_updateFunc[_updateFuncIndex].play))();
		if (_changeToNextSequence) {
			unloadAnimation();
			++_updateFuncIndex;
			_seqNum = this->_updateFunc[_updateFuncIndex].num;
		} else {
			updateSounds();
		}
		_system->copyRectToScreen(_offscreenBuffer, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
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

Audio::AudioStream *AnimationSequencePlayer::loadSound(int index, AnimationSoundType type) {
	Audio::AudioStream *stream = _compressedSound->load(kSoundTypeIntro, index, type == kAnimationSoundTypeLoopingWAV);
	if (stream) {
		return stream;
	}
	char fileName[64];
	snprintf(fileName, sizeof(fileName), "audio/%s", _audioFileNamesTable[index]);
	Common::File f;
	if (f.open(fileName)) {
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
			uint8 *sampleData = (uint8 *)malloc(size);
			if (sampleData) {
				f.read(sampleData, size);
				flags |= Audio::Mixer::FLAG_AUTOFREE;
				stream = Audio::makeLinearInputStream(sampleData, size, rate, flags, 0, 0);
			}
		}
	}
	return stream;
}

void AnimationSequencePlayer::loadSounds(int num) {
	if (_soundSeqDataList[num].musicVolume != 0) {
		Audio::AudioStream *s;
		if ((s = loadSound(_soundSeqDataList[num].musicIndex, kAnimationSoundType8BitsRAW)) != 0) {
			_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, s, -1, scaleMixerVolume(_soundSeqDataList[num].musicVolume));
		}
	}
	_soundSeqDataIndex = 0;
	_soundSeqDataCount = _soundSeqDataList[num].soundSeqDataCount;
	_soundSeqData = _soundSeqDataList[num].soundSeqData;
}

void AnimationSequencePlayer::updateSounds() {
	Audio::AudioStream *s = 0;
	const SoundSequenceData *p = &_soundSeqData[_soundSeqDataIndex];
	while (_soundSeqDataIndex < _soundSeqDataCount && p->timestamp <= _frameCounter) {
		switch (p->opcode) {
		case 0:
			if ((s = loadSound(p->num, kAnimationSoundTypeWAV)) != 0) {
				_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_soundsHandle[p->index], s, -1, scaleMixerVolume(p->volume));
			}
			break;
		case 1:
			if ((s = loadSound(p->num, kAnimationSoundTypeLoopingWAV)) != 0) {
				_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_soundsHandle[p->index], s, -1, scaleMixerVolume(p->volume));
			}
			break;
		case 2:
			_mixer->stopHandle(_soundsHandle[p->index]);
			break;
		case 3:
			_mixer->stopHandle(_musicHandle);
			break;
		case 4:
			_mixer->stopHandle(_musicHandle);
			if ((s = loadSound(p->num, kAnimationSoundType8BitsRAW)) != 0) {
				_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, s, -1, scaleMixerVolume(p->volume));
			}
			break;
		case 5:
			if ((s = loadSound(p->num, kAnimationSoundTypeWAV)) != 0) {
				_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, s, -1, scaleMixerVolume(p->volume));
			}
			break;
		case 6:
			_mixer->stopHandle(_musicHandle);
			if ((s = loadSound(p->num, kAnimationSoundType16BitsRAW)) != 0) {
				_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, s, -1, scaleMixerVolume(p->volume));
			}
			break;
		default:
			warning("Unhandled sound opcode %d (%d,%d)", p->opcode, _frameCounter, p->timestamp);
			break;
		}
		++p;
		++_soundSeqDataIndex;
	}
}

void AnimationSequencePlayer::fadeInPalette() {
	uint8 paletteBuffer[256 * 4];
	memset(paletteBuffer, 0, sizeof(paletteBuffer));
	bool fadeColors = true;
	for (int step = 0; step < 64; ++step) {
		if (fadeColors) {
			fadeColors = false;
			for (int i = 0; i < 1024; ++i) {
				if ((i & 3) != 3 && paletteBuffer[i] < _animationPalette[i]) {
					const int color = paletteBuffer[i] + 4;
					paletteBuffer[i] = MIN<int>(color, _animationPalette[i]);
					fadeColors = true;
				}
			}
			_system->setPalette(paletteBuffer, 0, 256);
			_system->updateScreen();
		}
		_system->delayMillis(1000 / 60);
	}
}

void AnimationSequencePlayer::fadeOutPalette() {
	uint8 paletteBuffer[256 * 4];
	memcpy(paletteBuffer, _animationPalette, 1024);
	bool fadeColors = true;
	for (int step = 0; step < 64; ++step) {
		if (fadeColors) {
			fadeColors = false;
			for (int i = 0; i < 1024; ++i) {
				if ((i & 3) != 3 && paletteBuffer[i] > 0) {
					const int color = paletteBuffer[i] - 4;
					paletteBuffer[i] = MAX<int>(0, color);
					fadeColors = true;
				}
			}
			_system->setPalette(paletteBuffer, 0, 256);
			_system->updateScreen();
		}
		_system->delayMillis(1000 / 60);
	}
	_system->fillScreen(0);
}

void AnimationSequencePlayer::unloadAnimation() {
	_mixer->stopAll();
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

void AnimationSequencePlayer::getRGBPalette(int index) {
	const byte *rgbPalette = _flicPlayer[index].getPalette();
	for (int i = 0; i < 256; i++) {
		_animationPalette[i * 4 + 0] = rgbPalette[i * 3 + 0];
		_animationPalette[i * 4 + 1] = rgbPalette[i * 3 + 1];
		_animationPalette[i * 4 + 2] = rgbPalette[i * 3 + 2];
		_animationPalette[i * 4 + 3] = 0;
	}
}

void AnimationSequencePlayer::openAnimation(int index, const char *fileName) {
	if (!_flicPlayer[index].loadFile(fileName)) {
		warning("Unable to open flc animation file '%s'", fileName);
		_seqNum = 1;
		return;
	}
	_flicPlayer[index].decodeNextFrame();
	if (index == 0) {
		getRGBPalette(index);
		_flicPlayer[index].copyDirtyRectsToBuffer(_offscreenBuffer, kScreenWidth);
	}
}

bool AnimationSequencePlayer::decodeNextAnimationFrame(int index) {
	bool framesLeft = _flicPlayer[index].decodeNextFrame();
	if (_seqNum == 19) {
		_flicPlayer[index].copyFrameToBuffer(_offscreenBuffer, 0, 0, kScreenWidth);
	} else {
		_flicPlayer[index].copyDirtyRectsToBuffer(_offscreenBuffer, kScreenWidth);
		++_frameCounter;
	}
	if (index == 0) {
		if (_flicPlayer[index].paletteChanged()) {
			getRGBPalette(index);
		}
	}
	return framesLeft;
}

void AnimationSequencePlayer::loadIntroSeq17_18() {
	loadSounds(kSoundsList_Seq17_18);
	openAnimation(0, "graphics/merit.flc");
}

void AnimationSequencePlayer::playIntroSeq17_18() {
	if (!decodeNextAnimationFrame(0)) {
		_changeToNextSequence = true;
	}
}

void AnimationSequencePlayer::loadIntroSeq19_20() {
	fadeOutPalette();
	loadSounds(kSoundsList_Seq19_20);
	openAnimation(0, "graphics/budttle2.flc");
	openAnimation(1, "graphics/machine.flc");
}

void AnimationSequencePlayer::playIntroSeq19_20() {
	// The intro credits animation. This uses 2 animations: the foreground one, which
	// is the actual intro credits, and the background one, which is an animation of
	// cogs, and is being replayed when an intro credit appears
	if (_flicPlayer[0].getCurFrame() >= 116) {
		if (!_flicPlayer[1].decodeNextFrame()) {
			_flicPlayer[1].reset();
		}
	}
	bool framesLeft = decodeNextAnimationFrame(0);
	for (int i = 0; i < kScreenWidth * kScreenHeight; ++i) {
		if (_offscreenBuffer[i] == 0) {
			_offscreenBuffer[i] = _flicPlayer[1].getPixel(i);
		}
	}
	if (!framesLeft) {
		_changeToNextSequence = true;
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
	_updateScreenPicture = true;
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

void AnimationSequencePlayer::loadIntroSeq3_4() {
	displayLoadingScreen();
	loadSounds(kSoundsList_Seq3_4);
	_picBufPtr = loadPicture("graphics/house.pic");
	openAnimation(0, "graphics/intro1.flc");
	_system->copyRectToScreen(_offscreenBuffer, 320, 0, 0, kScreenWidth, kScreenHeight);
	fadeInPalette();
	_updateScreenPicture = false;
}

void AnimationSequencePlayer::playIntroSeq3_4() {
	if (!_updateScreenPicture) {
		bool framesLeft = decodeNextAnimationFrame(0);
		if (_flicPlayer[0].getCurFrame() == 706) {
			initPicPart4();
		}
		if (!framesLeft) {
			_changeToNextSequence = true;
		}
	} else {
		drawPicPart4();
	}
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
	int offset = 0;
	for (int y = 0; y < kScreenHeight; ++y) {
		for (int x = 0; x < kScreenWidth; ++x) {
			byte color = _flicPlayer[0].getPixel(offset);
			if (color == 0) {
				color = _picBufPtr[800 + y * 640 + _updateScreenWidth + x];
			}
			_offscreenBuffer[offset++] = color;
		}
	}
}

void AnimationSequencePlayer::loadIntroSeq9_10() {
	loadSounds(kSoundsList_Seq9_10);
	_pic2BufPtr = loadPicture("graphics/bits.pic");
	_picBufPtr = loadPicture("graphics/lab.pic");
	openAnimation(0, "graphics/intro2.flc");
	_updateScreenWidth = 0;
}

void AnimationSequencePlayer::playIntroSeq9_10() {
	bool framesLeft = decodeNextAnimationFrame(0);
	if (_flicPlayer[0].getCurFrame() >= 264 && _flicPlayer[0].getCurFrame() <= 295) {
		drawPic1Part10();
		_updateScreenWidth += 6;
	} else if (_flicPlayer[0].getCurFrame() == 984) {
		drawPic2Part10();
	} else if (_flicPlayer[0].getCurFrame() >= 988 && _flicPlayer[0].getCurFrame() <= 996) {
		drawPic1Part10();
		_updateScreenWidth -= 25;
		if (_updateScreenWidth < 0) {
			_updateScreenWidth = 0;
		}
	}
	if (!framesLeft) {
		_changeToNextSequence = true;
	}
}

void AnimationSequencePlayer::loadIntroSeq21_22() {
	loadSounds(kSoundsList_Seq21_20);
	openAnimation(0, "graphics/intro3.flc");
}

void AnimationSequencePlayer::playIntroSeq21_22() {
	if (!decodeNextAnimationFrame(0)) {
		_changeToNextSequence = true;
	}
}

void AnimationSequencePlayer::loadIntroSeq13_14() {
	loadSounds(kSoundsList_Seq13_14);
	openAnimation(0, "graphics/allseg02.flc");
}

void AnimationSequencePlayer::playIntroSeq13_14() {
	if (!decodeNextAnimationFrame(0)) {
		_changeToNextSequence = true;
	}
}

void AnimationSequencePlayer::loadIntroSeq15_16() {
	loadSounds(kSoundsList_Seq15_16);
	openAnimation(0, "graphics/allseg03.flc");
}

void AnimationSequencePlayer::playIntroSeq15_16() {
	if (!decodeNextAnimationFrame(0)) {
		_changeToNextSequence = true;
	}
}

void AnimationSequencePlayer::loadIntroSeq27_28() {
	loadSounds(kSoundsList_Seq27_28);
	openAnimation(0, "graphics/allseg04.flc");
}

void AnimationSequencePlayer::playIntroSeq27_28() {
	if (!decodeNextAnimationFrame(0)) {
		_changeToNextSequence = true;
	}
}

} // namespace Tucker
