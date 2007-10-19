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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra.h"
#include "kyra/kyra_v2.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"
#include "kyra/text_v2.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_v2::seq_playSequences(int startSeq, int endSeq) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_playSequences(%i, %i)", startSeq, endSeq);
	seq_init();

	bool allowSkip = (startSeq == kSequenceTitle) ? false : true;

	if (endSeq == -1)
		endSeq = startSeq;
	
	assert(startSeq >= 0 && endSeq < kSequenceArraySize && startSeq <= endSeq);

	_screen->_charWidth = -2;

	memset(_activeWSA, 0, sizeof(ActiveWSA) * 8);
	for (int i = 0; i < 8; i++)
		_activeWSA[i].flags = -1;
				

	memset(_activeText, 0, sizeof(ActiveText) * 10);
	seq_resetAllTextEntries();

	_screen->hideMouse();
	int oldPage = _screen->setCurPage(2);

	for (int i = 0; i < 4; i++)
		memset(_screen->getPalette(i), 0, 0x300);

	memset(_pageBuffer1, 0, 0xfa00);
	memset(_pageBuffer2, 0, 0xfa00);

	_seqSubframePlaying = false;
	 
	int seqWsaCurrentFrame = 0;
	_seqTextColor[0] = _seqTextColor[1] = 0;	
	_seqEndTime = 0;
	_menuChoice = 0;

	for (int seqNum = startSeq; seqNum <= endSeq && !((_skipFlag && allowSkip) || _quitFlag || (_abortIntroFlag && allowSkip) || _menuChoice); seqNum++) {
		_screen->clearPage(0);
		_screen->clearPage(8);
		memcpy(_screen->getPalette(1), _screen->getPalette(0), 0x300);				
		_seqFrameCounter = 0;

		allowSkip = (seqNum == 2) ? false : true;
		
		if (_sequences[seqNum].flags & 2) {
			_screen->loadBitmap(_sequences[seqNum].cpsFile, 2, 2, _screen->getPalette(0));
		} else {
			_screen->setCurPage(2);
			_screen->clearPage(2);
			_screen->loadPalette("goldfont.col", _screen->getPalette(0));
		}

		if (_sequences[seqNum].callback)
			(this->*_sequences[seqNum].callback)(0, 0, 0, -1);

		if (_sequences[seqNum].flags & 1) {
			if (_seqWsa->opened())
				_seqWsa->close();
			_seqWsa->open(_sequences[seqNum].wsaFile, 0, _screen->getPalette(0));
			_seqWsa->setX(_sequences[seqNum].xPos);
			_seqWsa->setY(_sequences[seqNum].yPos);
			_seqWsa->setDrawPage(2);
			_seqWsa->displayFrame(0, 0);
		}

		if (_sequences[seqNum].flags & 4) {
			int cp = _screen->setCurPage(2);
			Screen::FontId cf =	_screen->setFont(Screen::FID_GOLDFONT_FNT);
			int sX = (320 - _screen->getTextWidth(_sequenceStrings[_sequences[seqNum].stringIndex1])) / 2;			
			_screen->printText(_sequenceStrings[_sequences[seqNum].stringIndex1], sX, 100 - _screen->getFontHeight(), 1, 0);
			sX = (320 - _screen->getTextWidth(_sequenceStrings[_sequences[seqNum].stringIndex2])) / 2;			
			_screen->printText(_sequenceStrings[_sequences[seqNum].stringIndex2], sX, 100, 1, 0);

			_screen->setFont(cf);
			_screen->setCurPage(cp);
		}

		_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);
		_screen->copyPage(0, 2);
		_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer1);
		_screen->copyBlockToPage(2, 0, 0, 320, 200, _pageBuffer2);
		
		_screen->copyPage(2, 6);
		
		seq_sequenceCommand(_sequences[seqNum].startupCommand);
		
		if (!((_skipFlag && allowSkip) || _quitFlag || (_abortIntroFlag && allowSkip) || _menuChoice)) {
			_screen->copyPage(2, 0);
			_screen->updateScreen();
		}		

		if (_sequences[seqNum].flags & 1) {
			int w2 = _seqWsa->width();
			int h2 = _seqWsa->height();
			int x = _sequences[seqNum].xPos;
			int y = _sequences[seqNum].yPos;

			_seqFrameDelay = _sequences[seqNum].frameDelay;

			if (_seqWsa) {
				if (x < 0) {
					x = 0;
					w2 = 0;
				}

				if (y < 0) {
					y = 0;
					h2 = 0;
				}

				if (_sequences[seqNum].xPos + _seqWsa->width() > 0x13F)
					_seqWsa->setWidth(0x140 - _sequences[seqNum].xPos);

				if (_sequences[seqNum].yPos + _seqWsa->height() > 0xC7)
					_seqWsa->setHeight(0xC7 - _sequences[seqNum].yPos);
			}
			uint8 dir = (_sequences[seqNum].startFrame > _sequences[seqNum].numFrames) ? 0 : 1;
			seqWsaCurrentFrame = _sequences[seqNum].startFrame;

			bool loop = true;
			while (loop && !((_skipFlag && allowSkip) || _quitFlag || (_abortIntroFlag && allowSkip) || _menuChoice)) {
				_seqEndTime = _system->getMillis() + _seqFrameDelay * _tickLength;

				if (_seqWsa || !_sequences[seqNum].callback)
					_screen->copyBlockToPage(2, 0, 0, 320, 200, _pageBuffer2);

				if (_sequences[seqNum].callback) {
					int f = seqWsaCurrentFrame % _seqWsa->frames();
					(this->*_sequences[seqNum].callback)(_seqWsa, _sequences[seqNum].xPos, _sequences[seqNum].yPos, f);
				}

				if (_seqWsa) {
					int f = seqWsaCurrentFrame % _seqWsa->frames();
					_seqWsa->setX(_sequences[seqNum].xPos);
					_seqWsa->setY(_sequences[seqNum].yPos);
					_seqWsa->setDrawPage(2);
					_seqWsa->displayFrame(f, 0);
				}

				_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);

				seq_processWSAs();		
				seq_processText();
			
				if ((_seqWsa || !_sequences[seqNum].callback) && !((_skipFlag && allowSkip) || _quitFlag || (_abortIntroFlag && allowSkip) || _menuChoice)) {
					_screen->copyPage(2, 0);
					_screen->copyPage(2, 6);
					_screen->updateScreen();
				}
				
				bool loop2 = true;
				while (loop2 && !((_skipFlag && allowSkip) || _quitFlag || (_abortIntroFlag && allowSkip) || _menuChoice)) {
					if (_seqWsa) {
						seq_processText();
						if (!((_skipFlag && allowSkip) || _quitFlag || (_abortIntroFlag && allowSkip) || _menuChoice)) {
							_screen->copyPage(2, 0);
							_screen->copyPage(2, 6);
							_screen->updateScreen();
						}


						uint32 now = _system->getMillis();
						if (now >= _seqEndTime) {
							loop2 = false;
						} else {
							uint32 tdiff = _seqEndTime - now;
							uint32 dly = tdiff < _tickLength ? tdiff : _tickLength;
							delay(dly);
							_seqEndTime -= dly;
						}
					} else {
						loop = loop2 = false;
					}
				}

				if (loop) {
					if (dir == 1) {
						if (++seqWsaCurrentFrame >= _sequences[seqNum].numFrames)
							loop = false;
					} else {
						if (--seqWsaCurrentFrame < _sequences[seqNum].numFrames)
							loop = false;
					}
				}
			}
			_seqWsa->close();


		} else {
			_seqFrameDelay = _sequences[seqNum].frameDelay;
			_seqEndTime = _system->getMillis() + _seqFrameDelay * _tickLength;
			while (!((_skipFlag && allowSkip) || _quitFlag || (_abortIntroFlag && allowSkip) || _menuChoice)) {
				uint32 starttime = _system->getMillis();
				seq_processWSAs();		
				if (_sequences[seqNum].callback)
					(this->*_sequences[seqNum].callback)(0, 0, 0, 0);
				
				seq_processText();
	
				_screen->copyPage(2, 6);		
				_screen->copyPage(2, 0);
				_screen->updateScreen();
				_screen->copyBlockToPage(2, 0, 0, 320, 200, _pageBuffer2);

				uint32 now = _system->getMillis();
				if (now >= _seqEndTime && !_seqSubframePlaying)
					break;
				
				uint32 tdiff = _seqEndTime - starttime;
				int32 dly = _tickLength - (now - starttime);
				if (dly > 0)
					delay(MIN<uint32>(dly, tdiff));
			}
		}

		if (_sequences[seqNum].callback)
			(this->*_sequences[seqNum].callback)(0, 0, 0, -2);

		uint32 ct = seq_activeTextsTimeLeft();
		uint32 dl = _sequences[seqNum].duration * _tickLength;
		if (dl < ct)
			dl = ct;
		_seqEndTime = _system->getMillis() + dl;
		
		while (!((_skipFlag && allowSkip) || _quitFlag || (_abortIntroFlag && allowSkip) || _menuChoice)) {
			uint32 starttime = _system->getMillis();
			seq_processWSAs();

			_screen->copyPage(2, 6);		
			_screen->copyPage(2, 0);
			_screen->updateScreen();
			_screen->copyBlockToPage(2, 0, 0, 320, 200, _pageBuffer2);
			
			uint32 now = _system->getMillis();
			if (now >= _seqEndTime && !_seqSubframePlaying) {
				break;
			} else {
				uint32 tdiff = _seqEndTime - starttime;
				delay(MIN<uint32>(tdiff, _tickLength));
			}
		}
		
		seq_sequenceCommand(_sequences[seqNum].finalCommand);
		seq_resetAllTextEntries();

		if ((seqNum != kSequenceTitle && seqNum < kSequenceZanfaun &&
			(_abortIntroFlag || _skipFlag)) || seqNum == kSequenceZanfaun) {
			_abortIntroFlag = _skipFlag = false;
			seqNum = kSequenceWestwood;
		}
		
		if (_menuChoice) {
			_abortIntroFlag = _skipFlag = false;
			if (_menuChoice == 2)
				_menuChoice = 0;
		}
	}

	_screen->setCurPage(oldPage);
	_screen->showMouse();

	for (int i = 0; i < 8; i++)
		seq_unloadWSA(i);

	if (_seqWsa->opened())
		_seqWsa->close();

	_screen->_charWidth = 0;

	seq_uninit();
}

int KyraEngine_v2::seq_introWestwood(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introWestwood(%p, %i, %i, %i)", (const void*)wsaObj, x, y, frm);
	
	if (frm == -2)
		delay(300 * _tickLength);
	else if (!frm)
		_sound->playTrack(2);
	
	return 0;
}

int KyraEngine_v2::seq_introTitle(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introTitle(%p, %i, %i, %i)", (const void*)wsaObj, x, y, frm);
	
	if (frm == 1) {
		_sound->playTrack(3);
	} else if (frm == 0x19) {
		int cp = _screen->setCurPage(0);
		_screen->showMouse();
		_system->updateScreen();
		_menuChoice = gui_handleMainMenu() + 1;
		_seqEndTime = 0;
		_seqSubframePlaying = false;
		if (_menuChoice == 4)
			quitGame();

		_screen->hideMouse();
		_screen->setCurPage(cp);
	}

	return 0;
}

int KyraEngine_v2::seq_introOverview(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introOverview(%p, %i, %i, %i)", (const void*)wsaObj, x, y, frm);
	
	uint8 * tmpPal = &(_screen->getPalette(3)[0x101]);
	memset(tmpPal, 0, 256);
	uint8 txtColorMap[16];
	uint32 endtime = 0, now = 0;

	switch (_seqFrameCounter) {
		case 0:
			_seqSubframePlaying = true;
			_sound->playTrack(4);
			endtime = _system->getMillis() + 60 * _tickLength;
			
			_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0) + 3, 255) & 0xff;
			memset(txtColorMap, _seqTextColor[1], 16);
			txtColorMap[1] = _seqTextColor[0] = _screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0) + 3, 255) & 0xff;

			_screen->setTextColorMap(txtColorMap);
		
			now = _system->getMillis();
			if (endtime > now)
				delay(endtime - now);
			break;

		case 1:
			_screen->generateGrayOverlay(_screen->getPalette(0), _screen->getPalette(3), 0x40, 0, 0, 0, 0x100, true);
			for (int i = 0; i < 256; i++)
				tmpPal[_screen->getPalette(3)[i]] = 1;

			for (int i = 0; i < 256; i++) {
				int v = (tmpPal[i] == 1) ? i : _screen->getPalette(3)[i];
				v *= 3;
				_screen->getPalette(2)[3 * i] = _screen->getPalette(0)[v];
				_screen->getPalette(2)[3 * i + 1] = _screen->getPalette(0)[v + 1];
				_screen->getPalette(2)[3 * i + 2] = _screen->getPalette(0)[v + 2];
			}
			break;

		case 40:
			seq_loadNestedSequence(0, kSequenceOver1);
			break;

		case 60:
			seq_loadNestedSequence(1, kSequenceOver2);
			break;

		case 120:
			seq_playTalkText(0);
			break;

		case 200:
			seq_waitForTextsTimeout();
			_screen->fadePalette(_screen->getPalette(2), 64);
			break;

		case 201:
			_screen->setScreenPalette(_screen->getPalette(2));
			_screen->updateScreen();
			_screen->applyGrayOverlay(0, 0, 320, 200, 2, _screen->getPalette(3));
			_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
			_screen->setScreenPalette(_screen->getPalette(0));
			_screen->updateScreen();
			seq_resetActiveWSA(0);
			seq_resetActiveWSA(1);
			break;

		case 282:
			seq_loadNestedSequence(0, kSequenceForest);
			seq_playTalkText(1);
			break;

		case 354:
			seq_resetActiveWSA(0);
			seq_loadNestedSequence(0, kSequenceDragon);
			break;

		case 400:
			seq_waitForTextsTimeout();
			seq_resetActiveWSA(0);
			_seqEndTime = 0;
			_seqSubframePlaying = false;
			break;

		default:
			break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_v2::seq_introLibrary(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introLibrary(%p, %i, %i, %i)", (const void*)wsaObj, x, y, frm);
	uint8 txtColorMap[16];

	switch (_seqFrameCounter) {
		case 0:
			_seqSubframePlaying = true;
			_sound->playTrack(5);
			
			_screen->generateGrayOverlay(_screen->getPalette(0), _screen->getPalette(3), 0x24, 0, 0, 0, 0x100, false);
			_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0) + 3, 255) & 0xff;
			memset(txtColorMap, _seqTextColor[1], 16);
			txtColorMap[1] = _seqTextColor[0] = _screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0) + 3, 255) & 0xff;

			_screen->setTextColorMap(txtColorMap);
			break;

		case 1:
			seq_loadNestedSequence(0, kSequenceLibrary3);
			seq_playTalkText(4);
			break;

		case 100:
			seq_waitForTextsTimeout();

			_screen->copyBlockToPage(2, 0, 0, 320, 200, _pageBuffer2);
			_screen->applyGrayOverlay(0, 0, 320, 200, 2, _screen->getPalette(3));
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
			_screen->updateScreen();
			_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);
			
			seq_resetActiveWSA(0);
			seq_loadNestedSequence(0, kSequenceDarm);

			break;

		case 104:
			seq_playTalkText(5);
			break;

		case 240:
			seq_waitForTextsTimeout();
			seq_resetActiveWSA(0);
			seq_loadNestedSequence(0, kSequenceLibrary2);
			break;

		case 340:
			seq_resetActiveWSA(0);
			_screen->applyGrayOverlay(0, 0, 320, 200, 2, _screen->getPalette(3));
			_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
			_screen->updateScreen();

			seq_loadNestedSequence(0, kSequenceMarco);
			seq_playTalkText(6);
			break;

		case 480:
			_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);
			seq_waitForTextsTimeout();
			seq_resetActiveWSA(0);
			_seqEndTime = 0;
			_seqSubframePlaying = false;
			break;

		default:
			break;
	}

	_seqFrameCounter++;
	return 0;
}


int KyraEngine_v2::seq_introHand(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introHand(%p, %i, %i, %i)", (const void*)wsaObj, x, y, frm);
	uint8 txtColorMap[16];

	switch (_seqFrameCounter) {
		case 0:
			_seqSubframePlaying = true;
			_sound->playTrack(6);
			
			_screen->generateGrayOverlay(_screen->getPalette(0), _screen->getPalette(3), 0x24, 0, 0, 0, 0x100, false);
			_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0) + 3, 255) & 0xff;
			memset(txtColorMap, _seqTextColor[1], 16);
			txtColorMap[1] = _seqTextColor[0] = _screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0) + 3, 255) & 0xff;

			_screen->setTextColorMap(txtColorMap);
			break;

		case 1:
			seq_loadNestedSequence(0, kSequenceHand1a);
			seq_loadNestedSequence(1, kSequenceHand1b);
			seq_loadNestedSequence(2, kSequenceHand1c);
			seq_playTalkText(7);
			break;

		case 201:
			seq_waitForTextsTimeout();
			_screen->applyGrayOverlay(0, 0, 320, 200, 2, _screen->getPalette(3));
			_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
			_screen->updateScreen();
			seq_resetActiveWSA(0);
			seq_resetActiveWSA(1);
			seq_resetActiveWSA(2);
			seq_loadNestedSequence(0, kSequenceHand2);
			seq_playTalkText(8);
			break;

		case 260:
			seq_waitForTextsTimeout();
			seq_resetActiveWSA(0);
			seq_loadNestedSequence(1, kSequenceHand3);
			seq_playTalkText(9);
			break;

		case 365:
			seq_waitForTextsTimeout();
			seq_resetActiveWSA(1);
			seq_loadNestedSequence(0, kSequenceHand4);
			break;

		case 405:
			seq_playTalkText(10);
			break;

		case 484:
			seq_waitForTextsTimeout();
			seq_resetActiveWSA(0);
			_seqEndTime = 0;
			_seqSubframePlaying = false;
			break;

		default:
			break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_v2::seq_introPoint(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	if (frm == -2) {
		seq_waitForTextsTimeout();
		_seqEndTime = 0;
	}

	uint8 txtColorMap[16];

	switch (_seqFrameCounter) {
		case -2:
			seq_waitForTextsTimeout();
			break;

		case 0:
			_sound->playTrack(7);			
			
			_seqTextColor[1] = 0xf7;
			memset(txtColorMap, _seqTextColor[1], 16);
			txtColorMap[1] = _seqTextColor[0] = _screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0) + 3, 255) & 0xff;
			_screen->setTextColorMap(txtColorMap);
			_screen->generateGrayOverlay(_screen->getPalette(0), _screen->getPalette(3), 0x24, 0, 0, 0, 0x100, false);
			break;

		case 1:
			seq_playTalkText(11);
			break;

		default:
			break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_v2::seq_introZanfaun(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	if (frm == -2) {
		seq_waitForTextsTimeout();
		_seqEndTime = 0;
		return 0;
	}

	uint8 txtColorMap[16];

	switch (_seqFrameCounter) {
		case 0:
			_sound->playTrack(8);			
			
			_seqTextColor[1] = 0xfd;
			memset(txtColorMap, _seqTextColor[1], 16);
			txtColorMap[1] = _seqTextColor[0] = _screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0) + 3, 255) & 0xff;
			_screen->setTextColorMap(txtColorMap);
			break;

		case 1:
			seq_setTextEntry(21, 140, 70, 20, 160);
			if (_flags.isTalkie)
				_sound->voicePlay(_sequenceSoundList[13]);
			_seqFrameDelay = 200;
			break;

		case 2:
		case 11:
		case 21:
			_seqFrameDelay = 12;
			break;

		case 10:
			seq_waitForTextsTimeout();
			seq_setTextEntry(13, 140, 50, _sequenceStringsDuration[13], 160);
			if (_flags.isTalkie)
				_sound->voicePlay(_sequenceSoundList[14]);
			_seqFrameDelay = 300;
			break;

		case 20:
			seq_setTextEntry(18, 160, 50, _sequenceStringsDuration[17], 160);
			if (_flags.isTalkie)
				_sound->voicePlay(_sequenceSoundList[15]);
			_seqFrameDelay = 200;
		break;

		case 19:
		case 26:
			seq_waitForTextsTimeout();
			break;

		case 46:
			seq_waitForTextsTimeout();
			seq_setTextEntry(16, 200, 50, _sequenceStringsDuration[16], 120);
			if (_flags.isTalkie)
				_sound->voicePlay(_sequenceSoundList[16]);
			_seqEndTime = _system->getMillis() + 120 * _tickLength;
			break;

		default:
			break;
	}	
	
	_seqFrameCounter++;
	return 0;
}

int KyraEngine_v2::seq_introOver1(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	if (frm == 2)
		seq_waitForTextsTimeout();
	else if (frm == 3)
		seq_playTalkText(12);
	return frm;
}


int KyraEngine_v2::seq_introOver2(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	if (frm == 1)
		seq_playTalkText(12);
	return frm;
}

int KyraEngine_v2::seq_introForest(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	if (frm == 11)
		seq_waitForTextsTimeout();
	else if (frm == 12)
		seq_playTalkText(2);
	return frm;
}

int KyraEngine_v2::seq_introDragon(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	if (frm == 11)
		seq_waitForTextsTimeout();
	else if (frm == 3)
		seq_playTalkText(3);
	return frm;
}

int KyraEngine_v2::seq_introDarm(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	//NULLSUB (at least in fm-towns version)
	return frm;
}

int KyraEngine_v2::seq_introLibrary2(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	//NULLSUB (at least in fm-towns version)
	return frm;
}

int KyraEngine_v2::seq_introMarco(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	if (frm == 36) {
		seq_waitForTextsTimeout();
		_seqEndTime = 0;
	}
	return frm;
}

int KyraEngine_v2::seq_introHand1a(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	//NULLSUB (at least in fm-towns version)
	return frm;
}

int KyraEngine_v2::seq_introHand1b(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	if (frm == 15)
		frm = 12;
	return frm;
}

int KyraEngine_v2::seq_introHand1c(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	if (frm == 8)
		frm = 4;
	return frm;
}

int KyraEngine_v2::seq_introHand2(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	//NULLSUB (at least in fm-towns version)
	return frm;
}

int KyraEngine_v2::seq_introHand3(WSAMovieV2 *wsaObj, int x, int y, int frm) {
	//NULLSUB (at least in fm-towns version)
	return frm;
}

uint32 KyraEngine_v2::seq_activeTextsTimeLeft() {
	uint32 res = 0;

	for (int i = 0; i < 10; i++) {
		uint32 chatend = (_activeText[i].duration + _activeText[i].startTime);
		uint32 curtime = _system->getMillis();
		if (_activeText[i].duration != -1 && chatend > curtime) {
			chatend -= curtime;
			if (res < chatend)
				res = chatend;
		}
	}

	return res;
}

void KyraEngine_v2::seq_processWSAs() {
	for (int i = 0; i <  8; i++) {
		if (_activeWSA[i].flags != -1) {
			if (seq_processNextSubFrame(i))
				seq_resetActiveWSA(i);
		}
	}
}

void KyraEngine_v2::seq_processText() {
	Screen::FontId curFont = _screen->setFont(Screen::FID_GOLDFONT_FNT);
	int curPage = _screen->setCurPage(2);
	char outputStr[60];

	for (int i = 0; i < 10; i++) {
		if (_activeText[i].startTime + _activeText[i].duration > _system->getMillis() && _activeText[i].duration != -1) {

			char *srcStr = seq_preprocessString(_sequenceStrings[_activeText[i].strIndex], _activeText[i].width);
			int yPos = _activeText[i].y;

			while (*srcStr) {
				uint32 linePos = 0;
				for (; *srcStr; linePos++) {
					if (*srcStr == 0x0d) // Carriage return
						break;
					outputStr[linePos] = *srcStr;
					srcStr++;
				}
                outputStr[linePos] = 0;
				if (*srcStr == 0x0d)
					srcStr++;

				uint8 textColor = (_activeText[i].textcolor >= 0) ? _activeText[i].textcolor : _seqTextColor[0];
				_screen->printText(outputStr, _activeText[i].x - (_screen->getTextWidth(outputStr) / 2), yPos, textColor, 0);
				yPos += 10;
			}
		} else {
			_activeText[i].duration = -1;
		}
	}

	_screen->setCurPage(curPage);
	_screen->setFont(curFont);
}

char *KyraEngine_v2::seq_preprocessString(const char *srcStr, int width) {
	char *dstStr = _seqProcessedString;
	int lineStart = 0;
	int linePos = 0;
	
	while (*srcStr) {
		while (*srcStr && *srcStr != 0x20) // Space
			dstStr[lineStart + linePos++] = *srcStr++;
		dstStr[lineStart + linePos] = 0;

		int len = _screen->getTextWidth(&dstStr[lineStart]);
		if (width >= len && *srcStr) {
			dstStr[lineStart + linePos++] = *srcStr++;			
		} else {
			dstStr[lineStart + linePos] = 0x0d; // Carriage return
			lineStart += linePos + 1;
			linePos = 0;
			if (*srcStr)
				srcStr++;
		}
	}
	dstStr[lineStart + linePos] = 0;
	
	return strlen(_seqProcessedString) ? dstStr : 0;
}

void KyraEngine_v2::seq_sequenceCommand(int command) {
	uint8 pal[768];
    
	for (int i = 0; i < 8; i++)
		seq_resetActiveWSA(i);
	
	switch (command) {
		case 0:
			memset(pal, 0, 0x300);
			_screen->fadePalette(pal, 16);
			memcpy (_screen->getPalette(0), pal, 0x300);
			memcpy (_screen->getPalette(1), pal, 0x300);
			break;

		case 1:
			memset(pal, 0x3F, 0x300);
			//////////TODO
			//////////Unused anyway (at least by fm-towns intro/outro)

			_screen->fadePalette(pal, 16);
			memcpy (_screen->getPalette(0), pal, 0x300);
			memcpy (_screen->getPalette(1), pal, 0x300);
			break;

		case 3:
			_screen->copyPage(2, 0);
			_screen->fadePalette(_screen->getPalette(0), 16);
			memcpy (_screen->getPalette(1), _screen->getPalette(0), 0x300);
			break;

		case 4:
			_screen->copyPage(2, 0);
			_screen->fadePalette(_screen->getPalette(0), 36);
			memcpy (_screen->getPalette(1), _screen->getPalette(0), 0x300);
			break;

		case 5:
			_screen->copyPage(2, 0);
			break;
	
		case 6:
			// UNUSED
			// seq_loadBLD("library.bld");
			break;

		case 7:
			// UNUSED
			// seq_loadBLD("marco.bld");
			break;
	
		case 8:
			memset(pal, 0, 0x300);
			_screen->fadePalette(pal, 16);
			memcpy (_screen->getPalette(0), pal, 0x300);
			memcpy (_screen->getPalette(1), pal, 0x300);

			delay(120 * _tickLength);
			break;
	
		case 9:
			for (int i = 0; i < 0x100; i++) {
				int pv = (_screen->getPalette(0)[3 * i] + _screen->getPalette(0)[3 * i + 1] + _screen->getPalette(0)[3 * i + 2]) / 3;
				pal[3 * i] = pal[3 * i + 1] = pal[3 * i + 2] = pv & 0xff;
			}

			//int a = 0x100;
			//int d = (0x800 << 5) - 0x100;
			//pal[3 * i] = pal[3 * i + 1] = pal[3 * i + 2] = 0x3f;

			_screen->fadePalette(pal, 64);
			memcpy (_screen->getPalette(0), pal, 0x300);
			memcpy (_screen->getPalette(1), pal, 0x300);
			break;

		default:
			break;
	}
}

void KyraEngine_v2::seq_cmpFadeFrame(const char * cmpFile) {
	_screen->copyBlockToPage(2, 0, 0, 320, 200, _pageBuffer1);
	_screen->copyRegionToBuffer(4, 0, 0, 320, 200, _pageBuffer1);
	_screen->clearPage(6);
	_screen->loadBitmap(cmpFile, 6, 6, 0);
	_screen->copyBlockToPage(4, 0, 0, 320, 200, _pageBuffer2);

	for (int i = 0; i < 3; i++) {
		uint32 endtime = _system->getMillis() + 4 * _tickLength;
		_screen->cmpFadeFrameStep(4, 320, 200, 0, 0, 2, 320, 200, 0, 0, 320, 200, 6);
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->updateScreen();
        delayUntil(endtime);
	}

	_screen->copyPage(4, 0);
	_screen->updateScreen();
	_screen->copyPage(4, 2);
	_screen->copyPage(4, 6);
	_screen->copyBlockToPage(4, 0, 0, 320, 200, _pageBuffer1);
}

void KyraEngine_v2::seq_playTalkText(uint8 chatNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_playIntroChat(%i)", chatNum);
	
	assert(chatNum < _sequenceSoundListSize);

	if (chatNum < 12)
		seq_setTextEntry(chatNum, 160, 168, _sequenceStringsDuration[chatNum], 160);

	_sound->voicePlay(_sequenceSoundList[chatNum]);
}

void KyraEngine_v2::seq_waitForTextsTimeout() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_waitForTextsTimeout()");
	
	uint32 longest = seq_activeTextsTimeLeft() + _system->getMillis();
	uint32 now = _system->getMillis();
	if (longest > now)
		delay(longest - now);

	seq_resetAllTextEntries();
}

void KyraEngine_v2::seq_resetAllTextEntries() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_resetAllTextEntries()");
	for (int i = 0; i < 10; i++)
		_activeText[i].duration = -1;
}

int KyraEngine_v2::seq_setTextEntry(uint16 strIndex, uint16 posX, uint16 posY, int duration, uint16 width) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_setTextEntry(%i, %i, %i, %i, %i)", strIndex, posX, posY, duration, width);
	
	for (int i = 0; i < 10; i++) {
		if (_activeText[i].duration != -1) {
			if (i < 9)
				continue;
			else
				return -1;
		}

		_activeText[i].strIndex = strIndex;
		_activeText[i].x = posX;
		_activeText[i].y = posY;
		_activeText[i].duration = duration * _tickLength;
		_activeText[i].width = width;
		_activeText[i].startTime = _system->getMillis();
		_activeText[i].textcolor = -1;		

		return i;
	}
	return -1;
}

void KyraEngine_v2::seq_loadNestedSequence(int wsaNum, int seqNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_loadNestedSequence(%i, %i)", wsaNum, seqNum);

	if (_activeWSA[wsaNum].flags != -1)
		return;
	
	NestedSequence s = _nSequences[seqNum];
	
	if (!_activeWSA[wsaNum].movie) {
		_activeWSA[wsaNum].movie = new WSAMovieV2(this);	
		assert(_activeWSA[wsaNum].movie);
	}

	if (_activeWSA[wsaNum].movie->opened())
		_activeWSA[wsaNum].movie->close();

	_activeWSA[wsaNum].movie->open(s.wsaFile, 0, 0);

	if (!_activeWSA[wsaNum].movie->opened()) {
		delete _activeWSA[wsaNum].movie;
		_activeWSA[wsaNum].movie = 0;
		return;
	}

	_activeWSA[wsaNum].endFrame = s.endFrame;
	_activeWSA[wsaNum].startFrame = _activeWSA[wsaNum].currentFrame = s.startframe;
	_activeWSA[wsaNum].frameDelay = s.frameDelay;
	_activeWSA[wsaNum].movie->setX(0);
	_activeWSA[wsaNum].movie->setY(0);	
	_activeWSA[wsaNum].movie->setDrawPage(_screen->_curPage);
	_activeWSA[wsaNum].callback = s.callback;	
	_activeWSA[wsaNum].control = s.wsaControl;

	_activeWSA[wsaNum].flags = s.flags | 1;
	_activeWSA[wsaNum].x = s.x;
	_activeWSA[wsaNum].y = s.y;
	_activeWSA[wsaNum].startupCommand = s.startupCommand;
	_activeWSA[wsaNum].finalCommand = s.finalCommand;
	_activeWSA[wsaNum].lastFrame = 0xffff;
	
	seq_nestedSequenceFrame(s.startupCommand, wsaNum);

	if (!s.startupCommand)
		seq_processNextSubFrame(wsaNum);

	_activeWSA[wsaNum].nextFrame = _system->getMillis();
}

void KyraEngine_v2::seq_nestedSequenceFrame(int command, int wsaNum) {
	int xa = 0, ya = 0;
	command--;
	if (!_activeWSA[wsaNum].movie)
		return;

	switch (command) {
		case 0:
			_activeWSA[wsaNum].movie->setDrawPage(8);
			xa = -_activeWSA[wsaNum].movie->xAdd();
			ya = -_activeWSA[wsaNum].movie->yAdd();
			_activeWSA[wsaNum].movie->setX(xa);
			_activeWSA[wsaNum].movie->setY(ya);
			_activeWSA[wsaNum].movie->displayFrame(0, 0);
			_activeWSA[wsaNum].movie->setX(0);
			_activeWSA[wsaNum].movie->setY(0);
			seq_animatedSubFrame(8, 2, 7, 8, _activeWSA[wsaNum].movie->xAdd(), _activeWSA[wsaNum].movie->yAdd(),
								_activeWSA[wsaNum].movie->width(), _activeWSA[wsaNum].movie->height(), 1, 2);
			break;

		case 1:
			_activeWSA[wsaNum].movie->setDrawPage(8);
			xa = -_activeWSA[wsaNum].movie->xAdd();
			ya = -_activeWSA[wsaNum].movie->yAdd();
			_activeWSA[wsaNum].movie->setX(xa);
			_activeWSA[wsaNum].movie->setY(ya);
			_activeWSA[wsaNum].movie->displayFrame(0, 0);
			_activeWSA[wsaNum].movie->setX(0);
			_activeWSA[wsaNum].movie->setY(0);
			seq_animatedSubFrame(8, 2, 7, 8, _activeWSA[wsaNum].movie->xAdd(), _activeWSA[wsaNum].movie->yAdd(),
								_activeWSA[wsaNum].movie->width(), _activeWSA[wsaNum].movie->height(), 1, 1);
			break;	

		case 2:
			seq_waitForTextsTimeout();
			_activeWSA[wsaNum].movie->setDrawPage(8);
			xa = -_activeWSA[wsaNum].movie->xAdd();
			ya = -_activeWSA[wsaNum].movie->yAdd();
			_activeWSA[wsaNum].movie->setX(xa);
			_activeWSA[wsaNum].movie->setY(ya);
			_activeWSA[wsaNum].movie->displayFrame(0x15, 0);
			_activeWSA[wsaNum].movie->setX(0);
			_activeWSA[wsaNum].movie->setY(0);
			seq_animatedSubFrame(8, 2, 7, 8, _activeWSA[wsaNum].movie->xAdd(), _activeWSA[wsaNum].movie->yAdd(),
						     	_activeWSA[wsaNum].movie->width(), _activeWSA[wsaNum].movie->height(), 0, 2);
			break;

		case 3:
			_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer1);
			_activeWSA[wsaNum].movie->setDrawPage(2);
			_activeWSA[wsaNum].movie->setX(0);
			_activeWSA[wsaNum].movie->setY(0);
			_activeWSA[wsaNum].movie->displayFrame(0, 0);
			_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);
			seq_cmpFadeFrame("scene2.cmp");
			break;

		case 4:
			_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer1);
			_activeWSA[wsaNum].movie->setDrawPage(2);
			_activeWSA[wsaNum].movie->setX(0);
			_activeWSA[wsaNum].movie->setY(0);
			_activeWSA[wsaNum].movie->displayFrame(0, 0);
			_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);
			seq_cmpFadeFrame("scene3.cmp");
			break;

		default:
			break;
	}
}

void KyraEngine_v2::seq_animatedSubFrame(int srcPage, int dstPage, int delaytime, int steps, 
                                         int x, int y, int w, int h, int openClose, int directionFlags) {
		
	if (openClose) {
		for (int i = 1; i < steps; i++) {
			uint32 endtime = _system->getMillis() + delaytime * _tickLength;

			int w2 = (((w * 256) / steps) * i) / 256;
			int h2 = (((h * 256) / steps) * i) / 256;

			int ym = (directionFlags & 2) ? (h - h2) : 0;
			int xm = (directionFlags & 1) ? (w - w2) : 0;

			_screen->wsaFrameAnimationStep(0, 0, x + xm, y + ym, w, h, w2, h2, srcPage, dstPage, 0);

			_screen->copyPage(dstPage, 6);
			_screen->copyPage(dstPage, 0);
			_screen->updateScreen();

			_screen->copyBlockToPage(dstPage, 0, 0, 320, 200, _pageBuffer2);
			delayUntil(endtime);
		}

		_screen->wsaFrameAnimationStep(0, 0, x, y, w, h, w, h, srcPage, dstPage, 0);
		_screen->copyPage(dstPage, 6);
		_screen->copyPage(dstPage, 0);
		_screen->updateScreen();
	} else {
		_screen->copyBlockToPage(dstPage, 0, 0, 320, 200, _pageBuffer2);
		for (int i = steps; i; i--) {
			uint32 endtime = _system->getMillis() + delaytime * _tickLength;

			int w2 = (((w * 256) / steps) * i) / 256;
			int h2 = (((h * 256) / steps) * i) / 256;

			int ym = (directionFlags & 2) ? (h - h2) : 0;
			int xm = (directionFlags & 1) ? (w - w2) : 0;

			_screen->wsaFrameAnimationStep(0, 0, x + xm, y + ym, w, h, w2, h2, srcPage, dstPage, 0);

			_screen->copyPage(dstPage, 6);
			_screen->copyPage(dstPage, 0);
			_screen->updateScreen();

			_screen->copyBlockToPage(dstPage, 0, 0, 320, 200, _pageBuffer2);
			delayUntil(endtime);
		}
	}
}

void KyraEngine_v2::seq_resetActiveWSA(int wsaNum) {
	if (_activeWSA[wsaNum].flags == -1)
		return;

	_activeWSA[wsaNum].flags = -1;
	seq_nestedSequenceFrame(_activeWSA[wsaNum].finalCommand, wsaNum);
	_activeWSA[wsaNum].movie->close();
}

void KyraEngine_v2::seq_unloadWSA(int wsaNum) {
	if (_activeWSA[wsaNum].movie) {
		_activeWSA[wsaNum].movie->close();
		delete _activeWSA[wsaNum].movie;
		_activeWSA[wsaNum].movie = 0;
	}
}

bool KyraEngine_v2::seq_processNextSubFrame(int wsaNum) {
	uint32 currentFrame = _activeWSA[wsaNum].currentFrame;
	uint32 currentTime = _system->getMillis();
	
	if (_activeWSA[wsaNum].callback && currentFrame != _activeWSA[wsaNum].lastFrame) {
		_activeWSA[wsaNum].lastFrame = currentFrame;
		currentFrame = (this->*_activeWSA[wsaNum].callback)(_activeWSA[wsaNum].movie, _activeWSA[wsaNum].x, _activeWSA[wsaNum].y, currentFrame);		
	}

	if (_activeWSA[wsaNum].movie) {
		_activeWSA[wsaNum].movie->setDrawPage(2);
		_activeWSA[wsaNum].movie->setX(_activeWSA[wsaNum].x);
		_activeWSA[wsaNum].movie->setY(_activeWSA[wsaNum].y);
		
		if (_activeWSA[wsaNum].flags & 0x20) {
			_activeWSA[wsaNum].movie->displayFrame(_activeWSA[wsaNum].control[currentFrame].frameIndex, 0x4000);
			_activeWSA[wsaNum].frameDelay = _activeWSA[wsaNum].control[currentFrame].frameDelay;
		} else {
			_activeWSA[wsaNum].movie->displayFrame(currentFrame % _activeWSA[wsaNum].movie->frames(), 0x4000);
		}
	}

	if (_activeWSA[wsaNum].flags & 0x10) {
		currentFrame = (currentTime - _activeWSA[wsaNum].nextFrame) / (_activeWSA[wsaNum].frameDelay * _tickLength);
	} else {
		if (((int32)(currentTime - _activeWSA[wsaNum].nextFrame) / (int32)(_activeWSA[wsaNum].frameDelay * _tickLength)) > 0) {
			currentFrame++;
			_activeWSA[wsaNum].nextFrame += (_activeWSA[wsaNum].frameDelay * _tickLength);
		}
	}
	
	bool res = false;

	if (currentFrame >= _activeWSA[wsaNum].endFrame) {
		int sw = ((_activeWSA[wsaNum].flags & 0x1e) - 2);
		switch (sw) {
			case 0:
				res = true;
				currentFrame = _activeWSA[wsaNum].endFrame;
				_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);
				break;

			case 6:
			case 8:
				currentFrame = _activeWSA[wsaNum].endFrame - 1;
				break;

			case 2:
			case 10:
				currentFrame = _activeWSA[wsaNum].startFrame;
				break;

			default:
				currentFrame = _activeWSA[wsaNum].endFrame - 1;
				res = true;
				break;
		}
	}

	_activeWSA[wsaNum].currentFrame = currentFrame & 0xffff;
	return res;
}

void KyraEngine_v2::seq_init() {
	_pageBuffer1 = new uint8[64000];
	_pageBuffer2 = new uint8[64000];
	_seqProcessedString = new char[200];
	_seqWsa = new WSAMovieV2(this);
	_activeWSA = new ActiveWSA[8];
	_activeText = new ActiveText[10];
}

void KyraEngine_v2::seq_uninit() {
	delete [] _pageBuffer1;
	_pageBuffer1 = NULL;
	
	delete [] _pageBuffer2;
	_pageBuffer2 = NULL;

	delete [] _seqProcessedString;
	_seqProcessedString = NULL;
	
	delete [] _activeWSA;
	_activeWSA = NULL;
	
	delete [] _activeText;
	_activeText = NULL;
	
	delete _seqWsa;
	_seqWsa = NULL;
}

// static res
// TODO: move to staticres.cpp

const Sequence KyraEngine_v2::_sequences[] = {
	// flags, wsaFile, cpsFile, startupCommand, finalCommand, stringIndex1, stringIndex2,
	// startFrame, numFrames, frameDelay, xPos, yPos, callback, duration
	{ 2, 0, "virgin.cps",   4, 0, -1, -1, 0, 1,  100,  0, 0, 0,                                 30 },
	{ 1, "westwood.wsa", 0, 4, 0, -1, -1, 0, 18, 12,   0, 0, &KyraEngine_v2::seq_introWestwood, 10 },
	{ 1, "title.wsa", 0,    4, 0, -1, -1, 0, 26, 12,   0, 0, &KyraEngine_v2::seq_introTitle,    10 },
	{ 2, 0, "over.cps",     4, 0, -1, -1, 0, 1,  3600, 0, 0, &KyraEngine_v2::seq_introOverview, 30 },
	{ 2, 0, "library.cps",  4, 0, -1, -1, 0, 1,  3600, 0, 0, &KyraEngine_v2::seq_introLibrary,  30 },
	{ 2, 0, "hand.cps",     4, 0, -1, -1, 0, 1,  3600, 0, 0, &KyraEngine_v2::seq_introHand,     90 },
	{ 1, "point.wsa", 0,    4, 8, -1, -1, 0, 38, 7,    0, 0, &KyraEngine_v2::seq_introPoint,    200 },
	{ 1, "zanfaun.wsa", 0,  4, 0, -1, -1, 0, 51, 16,   0, 0, &KyraEngine_v2::seq_introZanfaun,  240 },
};

const NestedSequence KyraEngine_v2::_nSequences[] = {
	// flags, wsaFile, startframe, endFrame, frameDelay, callback, x, y, wsaControl, startupCommand, finalCommand, unk1;
	{ 0x0C, "figgle.wsa",  0, 3,   60, /*&KyraEngine_v2::seq_finaleFiggle*/0, 0, 0, 0,              0, 0, 0 },

	{ 8,    "over1.wsa",   0, 10,  10, &KyraEngine_v2::seq_introOver1,    0, 0, 0,                  0, 0, 0 },
	{ 8,    "over2.wsa",   0, 11,  9,  &KyraEngine_v2::seq_introOver2,    0, 0, 0,                  0, 0, 0 },
	{ 8,    "forest.wsa",  0, 22,  6,  &KyraEngine_v2::seq_introForest,   0, 0, 0,                  1, 3, 0 },
	{ 8,    "dragon.wsa",  0, 11,  6,  &KyraEngine_v2::seq_introDragon,   0, 0, 0,                  2, 0, 0 },
	{ 2,    "darm.wsa",    0, 19,  9,  &KyraEngine_v2::seq_introDarm,     0, 0, 0,                  4, 0, 0 },
	{ 2,    "library.wsa", 0, 33,  9,  &KyraEngine_v2::seq_introLibrary2, 0, 0, 0,                  4, 0, 0 },
	{ 0x2A, "library.wsa", 0, 18,  9,  &KyraEngine_v2::seq_introLibrary2, 0, 0, _wsaControlLibrary, 0, 0, 0 },
	{ 0x0A, "marco.wsa",   0, 37,  9,  &KyraEngine_v2::seq_introMarco,    0, 0, 0,                  4, 0, 0 },
	{ 2,    "hand1a.wsa",  0, 34,  9,  &KyraEngine_v2::seq_introHand1a,   0, 0, 0,                  0, 0, 0 },
	{ 0x2A, "hand1b.wsa",  0, 16,  9,  &KyraEngine_v2::seq_introHand1b,   0, 0, _wsaControlHand1b,  0, 0, 0 },
	{ 0x2A, "hand1c.wsa",  0, 9,   9,  &KyraEngine_v2::seq_introHand1c,   0, 0, _wsaControlHand1c,  0, 0, 0 },
	{ 0x2C, "hand2.wsa",   0, 2,   9,  &KyraEngine_v2::seq_introHand2,    0, 0, _wsaControlHand2,   5, 0, 0 },
	{ 0x2C, "hand3.wsa",   0, 4,   9,  &KyraEngine_v2::seq_introHand3,    0, 0, _wsaControlHand3,   5, 0, 0 },
	{ 0x2C, "hand4.wsa",   0, 8,   9,  0,                                 0, 0, _wsaControlHand4,   5, 0, 0 }
};


const SequenceControl KyraEngine_v2::_wsaControlLibrary[] = {
	{0x00, 0x0A}, {0x01, 0x0A}, {0x02, 0x0A}, {0x03, 0x0A}, {0x04, 0x0A}, {0x05, 0x0A},
	{0x06, 0x0A}, {0x07, 0x0A}, {0x08, 0x0A}, {0x09, 0x0A}, {0x08, 0x0A}, {0x07, 0x0A},
	{0x06, 0x0A}, {0x05, 0x28}, {0x04, 0x0A}, {0x03, 0x0A}, {0x02, 0x0A}, {0x01, 0x0A}
};

const SequenceControl KyraEngine_v2::_wsaControlHand1b[] = {
	{0x00, 0x06}, {0x01, 0x06}, {0x02, 0x06}, {0x03, 0x06}, {0x04, 0x06}, {0x05, 0x06},
	{0x06, 0x06}, {0x07, 0x06}, {0x08, 0x06}, {0x09, 0x06}, {0x0A, 0x06}, {0x0B, 0x06},
	{0x0B, 0x0C}, {0x0C, 0x0C}, {0x0D, 0x0C}, {0x0C, 0x0C}, {0x0B, 0x0C}
};

const SequenceControl KyraEngine_v2::_wsaControlHand1c[] = {
	{0x00, 0x06}, {0x01, 0x06}, {0x02, 0x06}, {0x03, 0x06}, {0x04, 0x06}, {0x03, 0x06},
	{0x04, 0x06}, {0x05, 0x40}, {0x05, 0x06}
};
 
const SequenceControl KyraEngine_v2::_wsaControlHand2[] = {
	{0x00, 0x06}, {0x01, 0x06}, {0x00, 0x06}, {0x01, 0x06}, {0x00, 0x06}, {0x01, 0x06},
	{0x00, 0x06}, {0x01, 0x06}, {0x00, 0x06}, {0x01, 0x06}, {0x00, 0x06}, {0x01, 0x06},
	{0x00, 0x06}, {0x01, 0x06}, {0x00, 0x06}, {0x01, 0x06}	
};

const SequenceControl KyraEngine_v2::_wsaControlHand3[] = {
	{0x00, 0x06}, {0x01, 0x06}, {0x02, 0x06}, {0x01, 0x06}, {0x00, 0x01}
};

const SequenceControl KyraEngine_v2::_wsaControlHand4[] = {
	{0x00, 0x06}, {0x01, 0x06}, {0x02, 0x06}, {0x03, 0x06}, {0x04, 0x06},
	{0x03, 0x06}, {0x02, 0x06}, {0x01, 0x06}
};

} // end of namespace Kyra

