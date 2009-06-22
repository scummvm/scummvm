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

#include "kyra/kyra_v1.h"
#include "kyra/kyra_hof.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"
#include "kyra/text_hof.h"
#include "kyra/timer.h"
#include "kyra/resource.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_HoF::seq_playSequences(int startSeq, int endSeq) {
	seq_init();

	bool allowSkip = (!(_flags.isDemo && !_flags.isTalkie) && (startSeq == kSequenceTitle)) ? false : true;

	if (endSeq == -1)
		endSeq = startSeq;

	assert(startSeq >= 0 && endSeq < kSequenceArraySize && startSeq <= endSeq);

	_sound->setSoundList(&_soundData[(startSeq > kSequenceZanfaun) ? kMusicFinale : kMusicIntro]);
	_sound->loadSoundFile(0);

	_screen->_charWidth = (_flags.gameID == GI_LOL) ? 0 : -2;

	memset(_activeWSA, 0, sizeof(ActiveWSA) * 8);
	for (int i = 0; i < 8; ++i)
		_activeWSA[i].flags = -1;

	memset(_activeText, 0, sizeof(ActiveText) * 10);
	seq_resetAllTextEntries();

	_screen->hideMouse();
	int oldPage = _screen->setCurPage(2);

	for (int i = 0; i < 4; ++i)
		_screen->getPalette(i).clear();

	_screen->clearPage(10);
	_screen->clearPage(12);

	_seqSubframePlaying = false;

	_seqWsaCurrentFrame = 0;
	_seqTextColor[0] = _seqTextColor[1] = 0;
	_seqEndTime = 0;
	_menuChoice = 0;

	for (int seqNum = startSeq; seqNum <= endSeq && !((skipFlag() && allowSkip) || shouldQuit() || (_abortIntroFlag && allowSkip) || _menuChoice); seqNum++) {
		_screen->clearPage(0);
		_screen->clearPage(8);
		_screen->copyPalette(1, 0);
		_seqFrameCounter = 0;
		_seqStartTime = _system->getMillis();

		allowSkip = (!(_flags.isDemo && !_flags.isTalkie) && (seqNum == kSequenceTitle)) ? false : true;

		Sequence cseq = _sequences->seq[seqNum];
		SeqProc cb = _callbackS[seqNum];

		if (cseq.flags & 2) {
			_screen->loadBitmap(cseq.cpsFile, 2, 2, &_screen->getPalette(0));
			_screen->setScreenPalette(_screen->getPalette(0));
		} else {
			_screen->setCurPage(2);
			_screen->clearPage(2);
			_screen->loadPalette("goldfont.col", _screen->getPalette(0));
		}

		if (cb && !(_flags.isDemo && !_flags.isTalkie))
			(this->*cb)(0, 0, 0, -1);

		if (cseq.flags & 1) {
			_seqWsa->close();
			_seqWsa->open(cseq.wsaFile, 0, &_screen->getPalette(0));
			_screen->setScreenPalette(_screen->getPalette(0));
			_seqWsa->displayFrame(0, 2, cseq.xPos, cseq.yPos, 0, 0, 0);
		}

		if (cseq.flags & 4) {
			int cp = _screen->setCurPage(2);
			Screen::FontId cf =	_screen->setFont(Screen::FID_GOLDFONT_FNT);
			if (cseq.stringIndex1 != -1) {
				int sX = (320 - _screen->getTextWidth(_sequenceStrings[cseq.stringIndex1])) / 2;
				_screen->printText(_sequenceStrings[cseq.stringIndex1], sX, 100 - _screen->getFontHeight(), 1, 0);
			}
			if (cseq.stringIndex2 != -1) {
				int sX = (320 - _screen->getTextWidth(_sequenceStrings[cseq.stringIndex2])) / 2;
				_screen->printText(_sequenceStrings[cseq.stringIndex2], sX, 100, 1, 0);
			}
			_screen->setFont(cf);
			_screen->setCurPage(cp);
		}

		_screen->copyPage(2, 12);
		_screen->copyPage(0, 2);
		_screen->copyPage(2, 10);
		_screen->copyPage(12, 2);

		seq_sequenceCommand(cseq.startupCommand);

		if (!((skipFlag() && allowSkip) || shouldQuit() || (_abortIntroFlag && allowSkip) || _menuChoice)) {
			_screen->copyPage(2, 0);
			_screen->updateScreen();
		}

		if (cseq.flags & 1) {
			int w2 = _seqWsa->width();
			int h2 = _seqWsa->height();
			int x = cseq.xPos;
			int y = cseq.yPos;

			_seqFrameDelay = cseq.frameDelay;

			if (_seqWsa) {
				if (x < 0) {
					x = 0;
					w2 = 0;
				}

				if (y < 0) {
					y = 0;
					h2 = 0;
				}

				if (cseq.xPos + _seqWsa->width() > 319)
					_seqWsa->setWidth(320 - cseq.xPos);

				if (cseq.yPos + _seqWsa->height() > 199)
					_seqWsa->setHeight(199 - cseq.yPos);
			}
			uint8 dir = (cseq.startFrame > cseq.numFrames) ? 0 : 1;
			_seqWsaCurrentFrame = cseq.startFrame;

			bool loop = true;
			while (loop && !((skipFlag() && allowSkip) || shouldQuit() || (_abortIntroFlag && allowSkip) || _menuChoice)) {
				_seqEndTime = _system->getMillis() + _seqFrameDelay * _tickLength;

				if (_seqWsa || !cb)
					_screen->copyPage(12, 2);

				if (cb) {
					int f = _seqWsaCurrentFrame % _seqWsa->frames();
					(this->*cb)(_seqWsa, cseq.xPos, cseq.yPos, f);
				}

				if (_seqWsa) {
					int f = _seqWsaCurrentFrame % _seqWsa->frames();
					_seqWsa->displayFrame(f, 2, cseq.xPos, cseq.yPos, 0, 0, 0);
				}

				_screen->copyPage(2, 12);

				seq_processWSAs();
				seq_processText();

				if ((_seqWsa || !cb) && !((skipFlag() && allowSkip) || shouldQuit() || (_abortIntroFlag && allowSkip) || _menuChoice)) {
					_screen->copyPage(2, 0);
					_screen->updateScreen();
				}

				bool loop2 = true;
				while (loop2 && !((skipFlag() && allowSkip) || shouldQuit() || (_abortIntroFlag && allowSkip) || _menuChoice)) {
					if (_seqWsa) {
						seq_processText();
						if (!((skipFlag() && allowSkip) || shouldQuit() || (_abortIntroFlag && allowSkip) || _menuChoice)) {
							_screen->copyPage(2, 0);
							_screen->updateScreen();
						}

						uint32 now = _system->getMillis();
						if (now >= _seqEndTime) {
							loop2 = false;
						} else {
							uint32 tdiff = _seqEndTime - now;
							uint32 dly = tdiff < _tickLength ? tdiff : _tickLength;
							delay(dly);
						}
					} else {
						loop = loop2 = false;
					}
				}

				if (loop) {
					if (dir == 1) {
						if (++_seqWsaCurrentFrame >= cseq.numFrames)
							loop = false;
					} else {
						if (--_seqWsaCurrentFrame < cseq.numFrames)
							loop = false;
					}
				}
			}
			_seqWsa->close();
		} else {
			_seqFrameDelay = cseq.frameDelay;
			_seqEndTime = _system->getMillis() + _seqFrameDelay * _tickLength;
			while (!((skipFlag() && allowSkip) || shouldQuit() || (_abortIntroFlag && allowSkip) || _menuChoice)) {
				_seqSubFrameStartTime = _system->getMillis();
				seq_processWSAs();
				if (cb)
					(this->*cb)(0, 0, 0, 0);

				seq_processText();

				_screen->copyPage(2, 0);
				_screen->updateScreen();
				_screen->copyPage(12, 2);

				uint32 now = _system->getMillis();
				if (now >= _seqEndTime && !_seqSubframePlaying)
					break;

				uint32 tdiff = _seqEndTime - _seqSubFrameStartTime;
				int32 dly = _tickLength - (now - _seqSubFrameStartTime);
				if (dly > 0)
					delay(MIN<uint32>(dly, tdiff));
			}
		}

		if (cb && !(_flags.isDemo && !_flags.isTalkie))
			(this->*cb)(0, 0, 0, -2);

		uint32 ct = seq_activeTextsTimeLeft();
		uint32 dl = cseq.duration * _tickLength;
		if (dl < ct)
			dl = ct;
		_seqEndTime = _system->getMillis() + dl;

		while (!((skipFlag() && allowSkip) || shouldQuit() || (_abortIntroFlag && allowSkip) || _menuChoice)) {
			_seqSubFrameStartTime = _system->getMillis();
			seq_processWSAs();

			_screen->copyPage(2, 0);
			_screen->updateScreen();
			_screen->copyPage(12, 2);

			uint32 now = _system->getMillis();
			if (now >= _seqEndTime && !_seqSubframePlaying) {
				break;
			}

			uint32 tdiff = _seqEndTime - _seqSubFrameStartTime;
			int32 dly = _tickLength - (now - _seqSubFrameStartTime);
			if (dly > 0)
				delay(MIN<uint32>(dly, tdiff));
		}

		seq_sequenceCommand(cseq.finalCommand);
		seq_resetAllTextEntries();

		if (_abortIntroFlag || skipFlag()) {
			_sound->haltTrack();
			_sound->voiceStop();
		}

		if (!_flags.isDemo || _flags.isTalkie) {
			if ((seqNum != kSequenceTitle && seqNum < kSequenceZanfaun &&
			(_abortIntroFlag || skipFlag())) || seqNum == kSequenceZanfaun) {
				_abortIntroFlag = false;
				_eventList.clear();
				seqNum = kSequenceWestwood;
			} else if (seqNum < kSequenceFrash && (_abortIntroFlag || skipFlag())) {
				_abortIntroFlag = false;
				_eventList.clear();
				seqNum = kSequenceFirates;
			}
		} else if (seqNum == endSeq && !(_abortIntroFlag || skipFlag())) {
			seqNum = 0;
		}

		if (_menuChoice) {
			_abortIntroFlag = false;
			_eventList.clear();

			if (_menuChoice == 2) {
				seqNum = kSequenceTitle;
				_menuChoice = 0;
			}
		}
	}

	if (_flags.isDemo && !_flags.isTalkie) {
		_eventList.clear();
		_screen->fadeToBlack();
	}

	if (!_menuChoice)
		delay(1200);

	_screen->setCurPage(oldPage);
	_screen->showMouse();

	for (int i = 0; i < 8; i++)
		seq_unloadWSA(i);

	_seqWsa->close();

	_screen->_charWidth = 0;

	seq_uninit();
}

int KyraEngine_HoF::seq_introWestwood(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == -2) {
		if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)
			delay(300 * _tickLength);
	} else if (!frm) {
		_sound->playTrack(2);
	}

	return 0;
}

int KyraEngine_HoF::seq_introTitle(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 1) {
		_sound->playTrack(3);
	} else if (frm == 25) {
		int cp = _screen->setCurPage(0);
		_screen->showMouse();
		_system->updateScreen();
		_menuChoice = _menu->handle(11) + 1;
		_seqEndTime = 0;
		_seqSubframePlaying = false;
		if (_menuChoice == 4)
			quitGame();

		_screen->hideMouse();
		_screen->setCurPage(cp);
	}

	return 0;
}

int KyraEngine_HoF::seq_introOverview(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	uint8 *tmpPal = _screen->getPalette(3).getData() + 0x101;
	memset(tmpPal, 0, 256);
	_seqSubFrameEndTimeInternal = 0;
	uint32 now = 0;

	switch (_seqFrameCounter) {
	case 0:
		_seqSubframePlaying = true;
		_sound->playTrack(4);
		_seqSubFrameEndTimeInternal = _system->getMillis() + 60 * _tickLength;

		_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0).getData() + 3, 255) & 0xff;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColorMap[1] = _seqTextColor[0] = _screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0).getData() + 3, 255) & 0xff;

		_screen->setTextColorMap(_seqTextColorMap);

		now = _system->getMillis();
		if (_seqSubFrameEndTimeInternal > now)
			delay(_seqSubFrameEndTimeInternal - now);
		break;

	case 1:
		_screen->generateGrayOverlay(_screen->getPalette(0).getData(), _screen->getPalette(3).getData(), 0x40, 0, 0, 0, 0x100, true);
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
		_screen->applyOverlay(0, 0, 320, 200, 2, _screen->getPalette(3).getData());
		_screen->copyPage(2, 12);
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
	case 434:
		if (!((_seqFrameCounter == 354 && (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) || (_seqFrameCounter == 434 && _flags.platform == Common::kPlatformPC)))
			break;

		seq_resetActiveWSA(0);
		seq_loadNestedSequence(0, kSequenceDragon);
		break;

	case 400:
	case 540:
		if (!((_seqFrameCounter == 400 && (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) || (_seqFrameCounter == 540 && _flags.platform == Common::kPlatformPC)))
			break;

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

int KyraEngine_HoF::seq_introLibrary(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (_seqFrameCounter) {
	case 0:
		_seqSubframePlaying = true;
		_sound->playTrack(5);

		_screen->generateGrayOverlay(_screen->getPalette(0).getData(), _screen->getPalette(3).getData(), 0x24, 0, 0, 0, 0x100, false);
		_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0).getData() + 3, 255) & 0xff;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColorMap[1] = _seqTextColor[0] = _screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0).getData() + 3, 255) & 0xff;

		_screen->setTextColorMap(_seqTextColorMap);
		break;

	case 1:
		seq_loadNestedSequence(0, kSequenceLibrary3);
		seq_playTalkText(4);
		break;

	case 100:
		seq_waitForTextsTimeout();

		_screen->copyPage(12, 2);
		_screen->applyOverlay(0, 0, 320, 200, 2, _screen->getPalette(3).getData());
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->updateScreen();
		_screen->copyPage(2, 12);

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
		_screen->applyOverlay(0, 0, 320, 200, 2, _screen->getPalette(3).getData());
		_screen->copyPage(2, 12);
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->updateScreen();

		seq_loadNestedSequence(0, kSequenceMarco);
		seq_playTalkText(6);
		break;

	case 480:
	case 660:
		if (!((_seqFrameCounter == 480 && (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) || (_seqFrameCounter == 660 && _flags.platform == Common::kPlatformPC)))
			break;

		_screen->copyPage(2, 12);
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


int KyraEngine_HoF::seq_introHand(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (_seqFrameCounter) {
	case 0:
		_seqSubframePlaying = true;
		_sound->playTrack(6);

		_screen->generateGrayOverlay(_screen->getPalette(0).getData(), _screen->getPalette(3).getData(), 0x24, 0, 0, 0, 0x100, false);
		_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0).getData() + 3, 255) & 0xff;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColorMap[1] = _seqTextColor[0] = _screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0).getData() + 3, 255) & 0xff;

		_screen->setTextColorMap(_seqTextColorMap);
		break;

	case 1:
		seq_loadNestedSequence(0, kSequenceHand1a);
		seq_loadNestedSequence(1, kSequenceHand1b);
		seq_loadNestedSequence(2, kSequenceHand1c);
		seq_playTalkText(7);
		break;

	case 201:
		seq_waitForTextsTimeout();
		_screen->applyOverlay(0, 0, 320, 200, 2, _screen->getPalette(3).getData());
		_screen->copyPage(2, 12);
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->updateScreen();
		seq_resetActiveWSA(0);
		seq_resetActiveWSA(1);
		seq_resetActiveWSA(2);
		seq_loadNestedSequence(0, kSequenceHand2);
		seq_playTalkText(8);
		break;

	case 260:
	case 395:
		if (!((_seqFrameCounter == 260 && (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) || (_seqFrameCounter == 395 && _flags.platform == Common::kPlatformPC)))
			break;

		seq_waitForTextsTimeout();
		seq_resetActiveWSA(0);
		seq_loadNestedSequence(1, kSequenceHand3);
		seq_playTalkText(9);
		break;

	case 365:
	case 500:
		if (!((_seqFrameCounter == 365 && (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) || (_seqFrameCounter == 500 && _flags.platform == Common::kPlatformPC)))
			break;

		seq_waitForTextsTimeout();
		seq_resetActiveWSA(1);
		seq_loadNestedSequence(0, kSequenceHand4);
		break;

	case 405:
	case 540:
		if (!((_seqFrameCounter == 405 && (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) || (_seqFrameCounter == 540 && _flags.platform == Common::kPlatformPC)))
			break;

		seq_playTalkText(10);
		break;

	case 484:
	case 630:
		if (!((_seqFrameCounter == 484 && (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)) || (_seqFrameCounter == 630 && _flags.platform == Common::kPlatformPC)))
			break;

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

int KyraEngine_HoF::seq_introPoint(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == -2) {
		seq_waitForTextsTimeout();
		_seqEndTime = 0;
	}

	switch (_seqFrameCounter) {
	case -2:
		seq_waitForTextsTimeout();
		break;

	case 0:
		_sound->playTrack(7);

		_seqTextColor[1] = 0xf7;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColorMap[1] = _seqTextColor[0] = _screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0).getData() + 3, 255) & 0xff;
		_screen->setTextColorMap(_seqTextColorMap);
		_screen->generateGrayOverlay(_screen->getPalette(0).getData(), _screen->getPalette(3).getData(), 0x24, 0, 0, 0, 0x100, false);
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

int KyraEngine_HoF::seq_introZanfaun(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == -2) {
		seq_waitForTextsTimeout();
		_seqEndTime = 0;
		return 0;
	}

	switch (_seqFrameCounter) {
	case 0:
		_sound->playTrack(8);

		_seqTextColor[1] = 0xfd;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColorMap[1] = _seqTextColor[0] = _screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0).getData() + 3, 255) & 0xff;
		_screen->setTextColorMap(_seqTextColorMap);
		break;

	case 1:
		if (_flags.isTalkie) {
			seq_playWsaSyncDialogue(21, 13, -1, 140, 70, 160, wsaObj, 0, 8, x, y);
		} else {
			seq_setTextEntry(21, 140, 70, 200, 160);
			_seqFrameDelay = 200;
		}
		break;

	case 2:
	case 11:
	case 21:
		if (!_flags.isTalkie)
			_seqFrameDelay = 12;
		break;

	case 9:
		if (_flags.isTalkie)
			seq_playWsaSyncDialogue(13, 14, -1, 140, (_flags.lang == Common::FR_FRA
				|| _flags.lang == Common::DE_DEU) ? 50 : 70, 160, wsaObj, 9, 15, x, y);
		break;

	case 10:
		if (!_flags.isTalkie) {
			seq_waitForTextsTimeout();
			seq_setTextEntry(13, 140, 50, _sequenceStringsDuration[13], 160);
			_seqFrameDelay = 300;
		}
		break;

	case 16:
		if (_flags.isTalkie)
			seq_playWsaSyncDialogue(18, 15, -1, 140, (_flags.lang == Common::FR_FRA) ? 50 :
				(_flags.lang == Common::DE_DEU ? 40 : 70), 160, wsaObj, 10, 16, x, y);
		break;

	case 17:
		if (_flags.isTalkie)
			_seqFrameDelay = 12;
		break;

	case 20:
		if (!_flags.isTalkie) {
			seq_waitForTextsTimeout();
			seq_setTextEntry(18, 160, 50, _sequenceStringsDuration[18], 160);
			_seqFrameDelay = 200;
		}
		break;

	case 26:
		seq_waitForTextsTimeout();
		break;

	case 46:
		if (_flags.isTalkie) {
			seq_playWsaSyncDialogue(16, 16, -1, 200, 50, 120, wsaObj, 46, 46, x, y);
		} else {
			seq_waitForTextsTimeout();
			seq_setTextEntry(16, 200, 50, _sequenceStringsDuration[16], 120);
		}

		_seqEndTime = _system->getMillis() + 120 * _tickLength;
		break;

	default:
		break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_HoF::seq_introOver1(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 2)
		seq_waitForTextsTimeout();
	else if (frm == 3)
		seq_playTalkText(12);
	return frm;
}


int KyraEngine_HoF::seq_introOver2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 1)
		seq_playTalkText(12);
	return frm;
}

int KyraEngine_HoF::seq_introForest(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 11)
		seq_waitForTextsTimeout();
	else if (frm == 12)
		seq_playTalkText(2);

	return frm;
}

int KyraEngine_HoF::seq_introDragon(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 11)
		seq_waitForTextsTimeout();
	else if (frm == 3)
		seq_playTalkText(3);
	return frm;
}

int KyraEngine_HoF::seq_introDarm(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	//NULLSUB (at least in fm-towns version)
	return frm;
}

int KyraEngine_HoF::seq_introLibrary2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	//NULLSUB (at least in fm-towns version)
	return frm;
}

int KyraEngine_HoF::seq_introMarco(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 36) {
		seq_waitForTextsTimeout();
		_seqEndTime = 0;
	}
	return frm;
}

int KyraEngine_HoF::seq_introHand1a(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	//NULLSUB (at least in fm-towns version)
	return frm;
}

int KyraEngine_HoF::seq_introHand1b(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 15)
		frm = 12;
	return frm;
}

int KyraEngine_HoF::seq_introHand1c(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 8)
		frm = 4;
	return frm;
}

int KyraEngine_HoF::seq_introHand2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	//NULLSUB (at least in fm-towns version)
	return frm;
}

int KyraEngine_HoF::seq_introHand3(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	//NULLSUB (at least in fm-towns version)
	return frm;
}

int KyraEngine_HoF::seq_finaleFunters(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	_seqSubFrameEndTimeInternal = 0;
	int chatX = 0;
	int chatY = 0;
	int chatW = 0;
	int chatFirstFrame = 0;
	int chatLastFrame = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		seq_sequenceCommand(9);
		break;

	case 0:
		_sound->playTrack(3);

		_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0).getData() + 3, 255) & 0xff;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColor[0] = _seqTextColorMap[1] = 0xff;
		_screen->setTextColorMap(_seqTextColorMap);

		_seqSubFrameEndTimeInternal = _system->getMillis() + 480 * _tickLength;
		seq_printCreditsString(81, 240, 70, _seqTextColorMap, 252);
		seq_printCreditsString(82, 240, 90, _seqTextColorMap, _seqTextColor[0]);
		_screen->copyPage(2, 12);
		seq_playTalkText(_flags.isTalkie ? 28 : 24);
		delay(_seqSubFrameEndTimeInternal - _system->getMillis());
		_seqTextColor[0] = 1;

		if (_flags.isTalkie) {
			chatY = (_flags.lang == Common::FR_FRA) ? 70 : 78;
			chatFirstFrame = 9;
			chatLastFrame = 15;
			voiceIndex = 34;
		} else {
			chatY = (_flags.lang == Common::FR_FRA) ? 78 : 70;
			chatFirstFrame = 0;
			chatLastFrame = 8;
		}
		chatX = (_flags.lang == Common::FR_FRA) ? 84 : 88;
		chatW = 100;

		seq_playWsaSyncDialogue(22, voiceIndex, 187, chatX, chatY, chatW, wsaObj, chatFirstFrame, chatLastFrame, x, y);
		break;

	case 9:
	case 16:
		if (!((frm == 9 && !_flags.isTalkie) || (frm == 16 && _flags.isTalkie)))
			break;

		_seqFrameDelay = 12;

		if (_flags.lang == Common::FR_FRA) {
			chatX = 80;
			chatW = 112;
		} else {
			chatX = (_flags.lang == Common::DE_DEU) ? 84 : 96;
			chatW = 100;
		}

		if (_flags.isTalkie) {
			chatFirstFrame = 0;
			chatLastFrame = 8;
			voiceIndex = 35;
		} else {
			chatFirstFrame = 9;
			chatLastFrame = 15;
		}
		chatY = 70;

		seq_playWsaSyncDialogue(23, voiceIndex, 137, chatX, chatY, chatW, wsaObj, chatFirstFrame, chatLastFrame, x, y);
		if (_flags.isTalkie)
			_seqWsaCurrentFrame = 17;
		break;

	default:
		break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_HoF::seq_finaleFerb(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	_seqSubFrameEndTimeInternal = 0;
	int chatX = 0;
	int chatY = 0;
	int chatW = 0;
	int chatFirstFrame = 0;
	int chatLastFrame = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		seq_sequenceCommand(9);
		_seqSubFrameEndTimeInternal = _system->getMillis() + 480 * _tickLength;
		seq_printCreditsString(34, 240, _flags.isTalkie ? 60 : 40, _seqTextColorMap, 252);
		seq_printCreditsString(35, 240, _flags.isTalkie ? 70 : 50, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(36, 240, _flags.isTalkie ? 90 : 70, _seqTextColorMap, 252);
		seq_printCreditsString(37, 240, _flags.isTalkie ? 100 : 90, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(38, 240, _flags.isTalkie ? 120 : 110, _seqTextColorMap, 252);
		seq_printCreditsString(39, 240, _flags.isTalkie ? 130 : 120, _seqTextColorMap, _seqTextColor[0]);
		if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)
			seq_printCreditsString(103, 240, 130, _seqTextColorMap, _seqTextColor[0]);
		delay(_seqSubFrameEndTimeInternal - _system->getMillis());
		_seqEndTime = 0;
		break;

	case 0:
		_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0).getData() + 3, 255) & 0xff;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColor[0] = _seqTextColorMap[1] = 255;
		_screen->setTextColorMap(_seqTextColorMap);
		break;

	case 5:
		if (!_flags.isTalkie)
			seq_playTalkText(18);
		_seqFrameDelay = 16;

		if (_flags.isTalkie) {
			chatFirstFrame = 5;
			chatLastFrame = 8;
			voiceIndex = 22;
		} else {
			chatLastFrame = 14;
		}
		chatX = 116;
		chatY = 90;
		chatW = 60;

		seq_playWsaSyncDialogue(24, voiceIndex, 149, chatX, chatY, chatW, wsaObj, chatFirstFrame, chatLastFrame, x, y);
		break;

	case 11:
		if (_flags.isTalkie)
			seq_playWsaSyncDialogue(24, 22, 149, 116, 90, 60, wsaObj, 11, 14, x, y);
		break;

	case 16:
		seq_playTalkText(_flags.isTalkie ? 23 : 19);
		_seqFrameDelay = _flags.isTalkie ? 20 : 16;

		if (_flags.lang == Common::FR_FRA) {
			chatY = 48;
			chatW = 88;
		} else {
			chatY = 60;
			chatW = 100;
		}
		chatX = 60;

		if (_flags.isTalkie)
			voiceIndex = 36;

		seq_playWsaSyncDialogue(25, voiceIndex, 143, chatX, chatY, chatW, wsaObj, 16, 25, x, y);
		_seqFrameDelay = 16;
		break;

	default:
		break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_HoF::seq_finaleFish(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	_seqSubFrameEndTimeInternal = 0;
	int chatX = 0;
	int chatY = 0;
	int chatW = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		seq_sequenceCommand(9);
		_seqSubFrameEndTimeInternal = _system->getMillis() + 480 * _tickLength;

		seq_printCreditsString(40, 240, _flags.isTalkie ? 55 : 40, _seqTextColorMap, 252);
		seq_printCreditsString(41, 240, _flags.isTalkie ? 65 : 50, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(42, 240, _flags.isTalkie ? 75 : 60, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(43, 240, _flags.isTalkie ? 95 : 80, _seqTextColorMap, 252);
		seq_printCreditsString(44, 240, _flags.isTalkie ? 105 : 90, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(93, 240, _flags.isTalkie ? 125 : 110, _seqTextColorMap, 252);
		seq_printCreditsString(94, 240, _flags.isTalkie ? 135 : 120, _seqTextColorMap, _seqTextColor[0]);
		delay(_seqSubFrameEndTimeInternal - _system->getMillis());
		_seqEndTime = 0;
		break;

	case 0:
		_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0).getData() + 3, 255) & 0xff;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColor[0] = _seqTextColorMap[1] = 0xff;
		_screen->setTextColorMap(_seqTextColorMap);
		break;

	case 4:
		chatX = 94;
		chatY = 42;
		chatW = 100;
		if (_flags.isTalkie)
			voiceIndex = 37;
		seq_playWsaSyncDialogue(26, voiceIndex, 149, chatX, chatY, chatW, wsaObj, 3, 12, x, y);
		break;

	case 14:
		seq_playTalkText(_flags.isTalkie ? 19 : 15);
		break;

	case 23:
		seq_playTalkText(_flags.isTalkie ? 20 : 16);
		break;

	case 29:
		chatX = (_flags.lang == Common::DE_DEU) ? 82 : ((_flags.lang == Common::FR_FRA) ? 92 : 88);
		chatY = 40;
		chatW = 100;

		if (_flags.isTalkie) {
			if (_flags.lang == Common::DE_DEU)
				chatY = 35;
			voiceIndex = 38;
		}

		seq_playWsaSyncDialogue(27, voiceIndex, 187, chatX, chatY, chatW, wsaObj, 28, 34, x, y);
		break;

	case 45:
		seq_playTalkText(_flags.isTalkie ? 21 : 17);
		break;

	case 50:
		seq_playTalkText(_flags.isTalkie ? 29 : 25);
		break;

	default:
		break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_HoF::seq_finaleFheep(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	_seqSubFrameEndTimeInternal = 0;
	int chatX = 0;
	int chatY = 0;
	int chatW = 0;
	int chatFirstFrame = 0;
	int chatLastFrame = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		_screen->copyPage(12, 2);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		seq_sequenceCommand(9);
		_seqSubFrameEndTimeInternal = _system->getMillis() + 480 * _tickLength;
		seq_printCreditsString(49, 240, 20, _seqTextColorMap, 252);
		seq_printCreditsString(50, 240, 30, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(51, 240, 40, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(52, 240, 50, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(53, 240, 60, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(54, 240, 70, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(55, 240, 80, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(56, 240, 90, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(57, 240, 100, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(58, 240, 110, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(60, 240, 120, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(61, 240, 130, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(62, 240, 140, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(63, 240, 150, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(64, 240, 160, _seqTextColorMap, _seqTextColor[0]);

		delay(_seqSubFrameEndTimeInternal - _system->getMillis());
		_seqEndTime = 0;
		break;

	case 0:
		_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0).getData() + 3, 255) & 0xff;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColor[0] = _seqTextColorMap[1] = 0xff;
		_screen->setTextColorMap(_seqTextColorMap);
		break;

	case 2:
		seq_playTalkText(_flags.isTalkie ? 25 : 21);

		if (_flags.lang == Common::FR_FRA) {
			chatX = 92;
			chatY = 72;
		} else {
			chatX = (_flags.lang == Common::DE_DEU) ? 90 : 98;
			chatY = 84;
		}

		if (_flags.isTalkie) {
			chatFirstFrame = 8;
			chatLastFrame = 9;
			voiceIndex = 39;
		} else {
			chatFirstFrame = 2;
			chatLastFrame = -8;
		}
		chatW = 100;

		seq_playWsaSyncDialogue(28, voiceIndex, -1, chatX, chatY, chatW, wsaObj, chatFirstFrame, chatLastFrame, x, y);
		if (_flags.isTalkie)
			_seqWsaCurrentFrame = 4;
		break;

	case 9:
		seq_playTalkText(_flags.isTalkie ? 24 : 20);
		_seqFrameDelay = 100;
		break;

	default:
		break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_HoF::seq_finaleFarmer(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	_seqSubFrameEndTimeInternal = 0;
	int chatX = 0;
	int chatY = 0;
	int chatW = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		_screen->copyPage(12, 2);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		seq_sequenceCommand(9);
		_seqSubFrameEndTimeInternal = _system->getMillis() + 480 * _tickLength;
		seq_printCreditsString(45, 240, 40, _seqTextColorMap, 252);
		seq_printCreditsString(46, 240, 50, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(47, 240, 60, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(83, 240, 80, _seqTextColorMap, 252);
		seq_printCreditsString(48, 240, 90, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(65, 240, 110, _seqTextColorMap, 252);
		seq_printCreditsString(66, 240, 120, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(67, 240, 130, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(68, 240, 140, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(69, 240, 150, _seqTextColorMap, _seqTextColor[0]);
		if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)
			seq_printCreditsString(104, 240, 160, _seqTextColorMap, _seqTextColor[0]);
		delay(_seqSubFrameEndTimeInternal - _system->getMillis());
		_seqEndTime = 0;
		break;

	case 0:
		_seqTextColor[1] = 1 + (_screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0).getData() + 3, 254) & 0xff);
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColorMap[1] = _seqTextColor[0] = 1 + (_screen->findLeastDifferentColor(_seqTextColorPresets + 3, _screen->getPalette(0).getData() + 3, 254) & 0xff);
		_screen->setTextColorMap(_seqTextColorMap);
		seq_playTalkText(_flags.isTalkie ? 30 : 26);
		break;

	case 6:
		if (_flags.isTalkie)
			seq_playTalkText(18);
		break;

	case 12:
		if (!_flags.isTalkie)
			seq_playTalkText(14);

		chatX = 90;
		chatY = 30;
		chatW = 100;

		if (_flags.isTalkie) {
			if (_flags.lang == Common::FR_FRA || _flags.lang == Common::DE_DEU) {
				chatX = 75;
				chatY = 25;
			}
			voiceIndex = 40;
		}

		seq_playWsaSyncDialogue(29, voiceIndex, 150, chatX, chatY, chatW, wsaObj, 12, -21, x, y);
		break;

	default:
		break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_HoF::seq_finaleFuards(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	_seqSubFrameEndTimeInternal = 0;
	int chatX = 0;
	int chatY = 0;
	int chatW = 0;
	int chatFirstFrame = 0;
	int chatLastFrame = 0;
	int textCol = 0;

	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		seq_sequenceCommand(9);
		_seqSubFrameEndTimeInternal = _system->getMillis() + 480 * _tickLength;
		seq_printCreditsString(70, 240, 20, _seqTextColorMap, 252);
		seq_printCreditsString(71, 240, 30, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(72, 240, 40, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(73, 240, 50, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(74, 240, 60, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(75, 240, 70, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(101, 240, 80, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(102, 240, 90, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(87, 240, 100, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(88, 240, 110, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(89, 240, 120, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(90, 240, 130, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(91, 240, 140, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(92, 240, 150, _seqTextColorMap, _seqTextColor[0]);
		delay(_seqSubFrameEndTimeInternal - _system->getMillis());
		_seqEndTime = 0;
		break;

	case 0:
		for (int i = 0; i < 0x300; i++)
			_screen->getPalette(0)[i] &= 0x3f;
		_seqTextColor[1] = 0xCf;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColor[0] = _seqTextColorMap[1] = 0xfe;

		_screen->setTextColorMap(_seqTextColorMap);
		break;

	case 6:
		_seqFrameDelay = 20;

		if (_flags.isTalkie) {
			chatX = 82;
			textCol = 143;
			chatFirstFrame = 16;
			chatLastFrame = 21;
			voiceIndex = 41;
		} else {
			chatX = 62;
			textCol = 137;
			chatFirstFrame = 9;
			chatLastFrame = 13;
		}
		chatY = (_flags.lang == Common::FR_FRA || _flags.lang == Common::DE_DEU) ? 88 :100;
		chatW = 80;

		seq_playWsaSyncDialogue(30, voiceIndex, 137, chatX, chatY, chatW, wsaObj, chatFirstFrame, chatLastFrame, x, y);
		if (_flags.isTalkie)
			_seqWsaCurrentFrame = 8;
		break;

	case 9:
	case 16:
		if (_flags.isTalkie) {
			if (frm == 16)
				break;
			chatX = 64;
			textCol = 137;
			chatFirstFrame = 9;
			chatLastFrame = 13;
			voiceIndex = 42;
		} else {
			if (frm == 9)
				break;
			chatX = 80;
			textCol = 143;
			chatFirstFrame = 16;
			chatLastFrame = 21;
		}
		chatY = 100;
		chatW = 100;

		seq_playWsaSyncDialogue(31, voiceIndex, 143, chatX, chatY, chatW, wsaObj, chatFirstFrame, chatLastFrame, x, y);
		if (_flags.isTalkie)
			_seqWsaCurrentFrame = 21;
		break;

	default:
		break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_HoF::seq_finaleFirates(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	_seqSubFrameEndTimeInternal = 0;
	int chatX = 0;
	int chatY = 0;
	int chatW = 0;
	uint16 voiceIndex = 0;

	switch (frm) {
	case -2:
		_screen->copyPage(12, 2);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		seq_sequenceCommand(9);
		_seqSubFrameEndTimeInternal = _system->getMillis() + 480 * _tickLength;
		seq_printCreditsString(76, 240, 40, _seqTextColorMap, 252);
		seq_printCreditsString(77, 240, 50, _seqTextColorMap, 252);
		seq_printCreditsString(78, 240, 60, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(79, 240, 70, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(80, 240, 80, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(84, 240, 100, _seqTextColorMap, 252);
		seq_printCreditsString(85, 240, 110, _seqTextColorMap, _seqTextColor[0]);
		seq_printCreditsString(99, 240, 130, _seqTextColorMap, 252);
		seq_printCreditsString(100, 240, 140, _seqTextColorMap, _seqTextColor[0]);
		delay(_seqSubFrameEndTimeInternal - _system->getMillis());
		_seqEndTime = 0;
		break;

	case 0:
		_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0).getData() + 3, 255) & 0xff;
		memset(_seqTextColorMap, _seqTextColor[1], 16);
		_seqTextColor[0] = _seqTextColorMap[1] = 0xff;
		_screen->setTextColorMap(_seqTextColorMap);
		break;

	case 6:
		seq_playTalkText(_flags.isTalkie ? 31 : 27);
		break;

	case 14:
	case 15:
		if (!((frm == 15 && !_flags.isTalkie) || (frm == 14 && _flags.isTalkie)))
			break;

		seq_playTalkText(_flags.isTalkie ? 31 : 27);

		if (_flags.lang == Common::DE_DEU) {
			chatX = 82;
			chatY = 84;
			chatW = 140;
		} else {
			chatX = 74;
			chatY = (_flags.lang == Common::FR_FRA) ? 96: 108;
			chatW = 80;
		}

		if (_flags.isTalkie)
			voiceIndex = 43;

		seq_playWsaSyncDialogue(32, voiceIndex, 137, chatX, chatY, chatW, wsaObj, 14, 16, x, y);
		break;

	case 28:
		seq_playTalkText(_flags.isTalkie ? 32 : 28);
		break;

	case 29:
		seq_playTalkText(_flags.isTalkie ? 33 : 29);
		break;

	case 31:
		if (_flags.isTalkie)
			voiceIndex = 44;

		chatX = 90;
		chatY = (_flags.lang == Common::DE_DEU) ? 60 : 76;
		chatW = 80;

		seq_playWsaSyncDialogue(33, voiceIndex, 143, chatX, chatY, chatW, wsaObj, 31, 34, x, y);
		break;

	case 35:
		_seqFrameDelay = 300;
		break;

	default:
		break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_HoF::seq_finaleFrash(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	int tmp = 0;

	switch (frm) {
	case -2:
		_screen->setCurPage(2);
		_screen->clearCurPage();
		_screen->copyPage(2, 12);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		_seqFrameCounter = 0;
		seq_loadNestedSequence(0, kSequenceFiggle);
		break;

	case -1:
		if (_flags.isTalkie)
			 seq_finaleActorScreen();
		_seqSpecialFlag = _flags.isTalkie;
		break;

	case 0:
		if (_seqFrameCounter == 1) {
			_sound->playTrack(4);
			_seqTextColor[1] = _screen->findLeastDifferentColor(_seqTextColorPresets, _screen->getPalette(0).getData() + 3, 255) & 0xff;
			memset(_seqTextColorMap, _seqTextColor[1], 16);
			_seqTextColor[0] = _seqTextColorMap[1] = 0xff;
			_screen->setTextColorMap(_seqTextColorMap);
		}
		_seqFrameDelay = 10;
		break;

	case 1:
		if (_seqFrameCounter < 20 && _seqSpecialFlag) {
			_seqWsaCurrentFrame = 0;
		} else {
			_seqFrameDelay = _flags.isTalkie ? 500 : (300 + _rnd.getRandomNumberRng(1, 300));
			seq_playTalkText(_flags.isTalkie ? 26 : 22);
			if (_seqSpecialFlag) {
				_seqFrameCounter = 3;
				_seqSpecialFlag = false;
			}
		}
		break;

	case 2:
		_seqFrameDelay = 20;
		break;

	case 3:
		seq_playTalkText(_flags.isTalkie ? 27 : 23);
		_seqFrameDelay = _flags.isTalkie ? 500 : (300 + _rnd.getRandomNumberRng(1, 300));
		break;

	case 4:
		_seqFrameDelay = 10;
		break;

	case 5:
		seq_playTalkText(_flags.isTalkie ? 27 : 23);
		tmp = _seqFrameCounter / 6;
		if (tmp == 2)
			_seqFrameDelay = _flags.isTalkie ? 7 : (1 + _rnd.getRandomNumberRng(1, 10));
		else if (tmp < 2)
			_seqFrameDelay = _flags.isTalkie ? 500 : (300 + _rnd.getRandomNumberRng(1, 300));
		break;

	case 6:
		_seqFrameDelay = 10;
		tmp = _seqFrameCounter / 6;
		if (tmp == 2)
			_seqWsaCurrentFrame = 4;
		else if (tmp < 2)
			_seqWsaCurrentFrame = 0;
		break;

	case 7:
		_seqFrameCounter = 0;
		_seqFrameDelay = 5;
		seq_playTalkText(_flags.isTalkie ? 26 : 22);
		break;

	case 11:
		if (_seqFrameCounter < 8)
			_seqWsaCurrentFrame = 8;
		break;

	default:
		break;
	}

	_seqFrameCounter++;
	return 0;
}

void KyraEngine_HoF::seq_finaleActorScreen() {
	static const uint8 colormap[] = {0, 0, 102, 102, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const ScreenDim d = { 0x00, 0x0C, 0x28, 0xB4, 0xFF, 0x00, 0x00, 0x00 };

	_screen->loadBitmap("finale.cps", 3, 3, &_screen->getPalette(0));
	_screen->setFont(Screen::FID_GOLDFONT_FNT);

	int talkieCreditsSize, talkieCreditsSpecialSize;
	const uint8 *talkieCredits = _staticres->loadRawData(k2SeqplayCredits, talkieCreditsSize);
	const char * const *talkieCreditsSpecial = _staticres->loadStrings(k2SeqplayCreditsSpecial, talkieCreditsSpecialSize);

	_sound->setSoundList(&_soundData[kMusicIngame]);
	_sound->loadSoundFile(3);
	_sound->playTrack(3);

	_screen->setTextColorMap(colormap);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
	_screen->updateScreen();
	_screen->fadeFromBlack();

	_screen->_charWidth = -2;
	uint8 *dataPtr = new uint8[0xafd];
	memcpy(dataPtr, talkieCredits, talkieCreditsSize);
	_staticres->unloadId(k2SeqplayCredits);

	seq_displayScrollText(dataPtr, &d, 2, 6, 5, 1, Screen::FID_GOLDFONT_FNT, Screen::FID_GOLDFONT_FNT, 0, talkieCreditsSpecial);
	delay(120);

	delete[] dataPtr;
	_staticres->unloadId(k2SeqplayCreditsSpecial);
	_sound->setSoundList(&_soundData[kMusicFinale]);
	_sound->loadSoundFile(0);
}

int KyraEngine_HoF::seq_finaleFiggle(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (_seqFrameCounter == 10)
		_seqEndTime = 0;
	if (_seqFrameCounter == 10 || _seqFrameCounter == 5 || _seqFrameCounter == 7)
		seq_playTalkText(_flags.isTalkie ? 45 : 30);

	_seqFrameCounter++;
	return frm;
}

int KyraEngine_HoF::seq_demoVirgin(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (!frm)
		delay(50 * _tickLength);
	return 0;
}

int KyraEngine_HoF::seq_demoWestwood(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (!frm)
		_sound->playTrack(2);
	return 0;
}
int KyraEngine_HoF::seq_demoTitle(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (!frm) {
		_sound->playTrack(3);
	} else if (frm == 25) {
		delay(60 * _tickLength);
		_seqEndTime = 0;
		seq_sequenceCommand(0);
	}
	return 0;
}

int KyraEngine_HoF::seq_demoHill(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (!frm) {
		_sound->playTrack(4);
	} else if (frm == 25) {
		seq_loadNestedSequence(0, kSequenceDemoWater);
		_seqFrameDelay--;
	} else if (frm > 25 && frm < 50) {
		if (_seqFrameDelay > 3)
			_seqFrameDelay--;
	} else if (frm == 95) {
		_seqFrameDelay = 70;
	} else if (frm == 96) {
		_seqFrameDelay = 7;
	} else if (frm == 129) {
		seq_resetActiveWSA(0);
	}

	return 0;
}

int KyraEngine_HoF::seq_demoOuthome(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (frm) {
	case 12:
		seq_playTalkText(4);
		break;

	case 32:
		seq_playTalkText(7);
		break;

	case 36:
		seq_playTalkText(10);
		break;

	case 57:
		seq_playTalkText(9);
		break;

	case 80:
	case 96:
	case 149:
		_seqFrameDelay = 70;
		break;

	case 81:
	case 97:
		_seqFrameDelay = 5;
		break;

	case 110:
		seq_playTalkText(5);
		break;

	case 137:
		seq_playTalkText(6);
		break;
	}

	return 0;
}

int KyraEngine_HoF::seq_demoWharf(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (!_seqFrameCounter)
		seq_loadNestedSequence(0, kSequenceDemoWharf2);

	switch (frm) {
	case 0:
		seq_playTalkText(11);
		break;

	case 5:
		if ((_seqFrameCounter / 8) <= 2 || _activeWSA[0].flags != -1)
			_seqWsaCurrentFrame = 0;
		else
			seq_resetActiveWSA(0);
		break;

	case 6:
		seq_resetActiveWSA(0);
		break;

	case 8:
	case 10:
		seq_playTalkText(2);
		break;

	case 13:
		seq_playTalkText(7);
		break;

	case 16:
		seq_playTalkText(12);
		break;

	default:
		break;
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_HoF::seq_demoDinob(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 0) {
		if (!(_seqFrameCounter/8)) {
			seq_loadNestedSequence(0, kSequenceDemoDinob2);
			_seqWsaCurrentFrame = 0;
		}
	} else if (frm == 3) {
		if (_activeWSA[0].flags != -1) {
			_seqWsaCurrentFrame = 0;
		} else {
			seq_resetActiveWSA(0);
			_screen->copyPage(2, 12);
		}
	} else if (frm == 4) {
		seq_resetActiveWSA(0);
	}

	_seqFrameCounter++;
	return 0;
}

int KyraEngine_HoF::seq_demoFisher(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (((_system->getMillis() - _seqStartTime) / (5 * _tickLength)) > 0) {
		_seqStartTime = _system->getMillis();
		if (!_seqFrameCounter) {
			seq_loadNestedSequence(0, kSequenceDemoBail);
			seq_loadNestedSequence(1, kSequenceDemoDig);
		}

		if (_seqScrollTextCounter >= 0x18f && !_seqFrameCounter)
			return 0;

		if (!_seqFrameCounter) {
			_screen->loadBitmap("adtext.cps", 4, 4, 0);
			_screen->loadBitmap("adtext2.cps", 6, 6, 0);
			_screen->copyPageMemory(6, 0, 4, 64000, 1024);
			_screen->copyPageMemory(6, 1023, 6, 0, 64000);
			_seqScrollTextCounter = 0;
		}

		seq_scrollPage(24, 144);
		_seqFrameCounter++;
		if (_seqFrameCounter < 0x256 || _seqFrameCounter > 0x31c) {
			if (_seqFrameCounter < 0x174 || _seqFrameCounter > 0x1d7) {
				if (_seqFrameCounter < 0x84 || _seqFrameCounter > 0xe7) {
					_seqScrollTextCounter++;
				}
			}
		}

		if (_seqFrameCounter > 0x31e) {
			seq_resetActiveWSA(0);
			seq_resetActiveWSA(1);
			_seqEndTime = 0;
			_screen->copyPage(2, 12);
		}

	} else {
		seq_scrollPage(24, 144);
	}
	return 0;
}

int KyraEngine_HoF::seq_demoWharf2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 69)
		_seqWsaCurrentFrame = 8;

	return frm;
}

int KyraEngine_HoF::seq_demoDinob2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (frm) {
	case 19:
		seq_playTalkText(13);
		break;

	case 54:
		seq_playTalkText(15);
		break;

	case 61:
		seq_playTalkText(16);
		break;

	case 69:
		seq_playTalkText(14);
		break;

	case 77:
		seq_playTalkText(13);
		break;

	case 79:
		_seqWsaCurrentFrame = 4;
		break;
	}

	return frm;
}

int KyraEngine_HoF::seq_demoWater(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (frm == 1)
		seq_playTalkText(11);
	return frm;
}

int KyraEngine_HoF::seq_demoBail(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	return frm;
}

int KyraEngine_HoF::seq_demoDig(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	return frm;
}

#ifdef ENABLE_LOL
int KyraEngine_HoF::seq_lolDemoScene1(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	Palette &tmpPal = _screen->getPalette(2);

	if (!(_seqFrameCounter % 100)) {
		if (_seqFrameCounter == 0) {
			_sound->haltTrack();
			_sound->playTrack(6);
		}
		tmpPal.copy(_screen->getPalette(0));

		for (int i = 3; i < 0x300; i++) {
			tmpPal[i] = ((int)tmpPal[i] * 120) / 64;
			if (tmpPal[i] > 0x3f)
				tmpPal[i] = 0x3f;
		}

		seq_playTalkText(_rnd.getRandomBit());
		_screen->setScreenPalette(tmpPal);
		_screen->updateScreen();
		delay(8);
	} else {
		_screen->setScreenPalette(_screen->getPalette(0));
		_screen->updateScreen();
		if (_seqFrameCounter == 40)
			seq_playTalkText(3);
	}

	_seqFrameCounter++;
	return frm;
}

int KyraEngine_HoF::seq_lolDemoScene2(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (_seqFrameCounter - 17) {
	case 0:
		_seqFrameDelay = 8;
		break;
	case 3:
	case 6:
	case 9:
		seq_playTalkText(8);
		break;
	case 15:
		seq_playTalkText(9);
		break;
	case 18:
		seq_playTalkText(2);
		break;
	default:
		break;
	}
	_seqFrameCounter++;
	return frm;
}

int KyraEngine_HoF::seq_lolDemoScene3(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (_seqFrameCounter == 1)
		seq_playTalkText(6);
	else if (frm == 26)
		seq_playTalkText(7);

	_seqFrameCounter++;
	return frm;
}

int KyraEngine_HoF::seq_lolDemoScene4(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (_seqFrameCounter) {
	case 11:
	case 14:
	case 17:
	case 20:
		seq_playTalkText(8);
		break;
	case 22:
		seq_playTalkText(11);
		break;
	case 24:
		seq_playTalkText(8);
		break;
	case 30:
		seq_playTalkText(15);
		break;
	case 34:
		seq_playTalkText(14);
		break;
	case 38:
		seq_playTalkText(13);
		break;
	case 42:
		seq_playTalkText(12);
		break;
	default:
		break;
	}

	_seqFrameCounter++;
	return frm;
}

int KyraEngine_HoF::seq_lolDemoScene5(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	switch (_seqFrameCounter++) {
	case 0:
	case 4:
	case 6:
	case 8:
	case 10:
	case 14:
	case 16:
	case 18:
	case 20:
	case 22:
	case 24:
	case 26:
	case 28:
	case 30:
		seq_playTalkText(15);
		break;
	case 32:
		seq_playTalkText(16);
		break;
	case 42:
		seq_playTalkText(6);
		break;
	default:
		break;
	}
	return frm;
}

int KyraEngine_HoF::seq_lolDemoText5(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	if (_seqFrameCounter++ == 100)
		seq_playTalkText(5);
	return frm;
}

int KyraEngine_HoF::seq_lolDemoScene6(WSAMovie_v2 *wsaObj, int x, int y, int frm) {
	while (_seqScrollTextCounter < 0x122) {
		_seqEndTime = _system->getMillis() + 6 * _tickLength;
		if (!_seqFrameCounter) {
			_screen->loadBitmap("adtext.cps", 4, 4, 0);
			_screen->loadBitmap("adtext2.cps", 6, 6, 0);
			_screen->copyPageMemory(6, 0, 4, 64000, 1024);
			_screen->copyPageMemory(6, 1023, 6, 0, 64000);
			_seqScrollTextCounter = 0;
		}

		if (_seqFrameCounter % 175) {
			_screen->setScreenPalette(_screen->getPalette(0));
		} else {
			Palette &tmpPal = _screen->getPalette(2);
			tmpPal.copy(_screen->getPalette(0));

			for (int i = 3; i < 0x300; i++) {
				tmpPal[i] = ((int)tmpPal[i] * 120) / 64;
				if (tmpPal[i] > 0x3f)
					tmpPal[i] = 0x3f;
			}

			seq_playTalkText(_rnd.getRandomBit());
			_screen->setScreenPalette(tmpPal);
			_screen->updateScreen();
			delay(8);
		}

		if (_seqFrameCounter == 40 || _seqFrameCounter == 80 || _seqFrameCounter == 150 || _seqFrameCounter == 300)
			seq_playTalkText(3);

		_screen->copyPage(12, 2);
		seq_scrollPage(70, 130);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		_seqFrameCounter++;
		if (_seqFrameCounter < 128 || _seqFrameCounter > 207)
			_seqScrollTextCounter++;
		delayUntil(_seqEndTime);
	}
	_screen->copyPage(2, 12);

	return 0;
}
#endif // ENABLE_LOL

uint32 KyraEngine_HoF::seq_activeTextsTimeLeft() {
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

void KyraEngine_HoF::seq_processWSAs() {
	for (int i = 0; i < 8; i++) {
		if (_activeWSA[i].flags != -1) {
			if (seq_processNextSubFrame(i))
				seq_resetActiveWSA(i);
		}
	}
}

void KyraEngine_HoF::seq_processText() {
	Screen::FontId curFont = _screen->setFont(Screen::FID_GOLDFONT_FNT);
	int curPage = _screen->setCurPage(2);
	char outputStr[70];

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

char *KyraEngine_HoF::seq_preprocessString(const char *srcStr, int width) {
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

void KyraEngine_HoF::seq_sequenceCommand(int command) {
	for (int i = 0; i < 8; i++)
		seq_resetActiveWSA(i);

	switch (command) {
	case 0:
		_screen->fadeToBlack(36);
		_screen->getPalette(0).clear();
		_screen->getPalette(1).clear();
		break;

	case 1:
		seq_playTalkText(_rnd.getRandomBit());

		memset(_screen->getPalette(0).getData(), 0x3F, 0x300);
		_screen->fadePalette(_screen->getPalette(0), 16);

		_screen->copyPalette(1, 0);
		break;

	case 3:
		_screen->copyPage(2, 0);
		_screen->fadePalette(_screen->getPalette(0), 16);
		_screen->copyPalette(1, 0);
		break;

	case 4:
		_screen->copyPage(2, 0);
		_screen->fadePalette(_screen->getPalette(0), 36);
		_screen->copyPalette(1, 0);
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
		_screen->fadeToBlack(16);
		_screen->getPalette(0).clear();
		_screen->getPalette(1).clear();

		delay(120 * _tickLength);
		break;

	case 9: {
		Palette &pal = _screen->getPalette(0);
		for (int i = 0; i < 256; i++) {
			int pv = (pal[3 * i] + pal[3 * i + 1] + pal[3 * i + 2]) / 3;
			pal[3 * i] = pal[3 * i + 1] = pal[3 * i + 2] = pv & 0xff;
		}

		//int a = 0x100;
		//int d = (0x800 << 5) - 0x100;
		//pal[3 * i] = pal[3 * i + 1] = pal[3 * i + 2] = 0x3f;

		_screen->fadePalette(pal, 64);
		_screen->copyPalette(1, 0);
		} break;

	default:
		break;
	}
}

void KyraEngine_HoF::seq_cmpFadeFrame(const char *cmpFile) {
	_screen->copyPage(10, 2);
	_screen->copyPage(4, 10);
	_screen->clearPage(6);
	_screen->loadBitmap(cmpFile, 6, 6, 0);
	_screen->copyPage(12, 4);

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
	_screen->copyPage(10, 4);
}

void KyraEngine_HoF::seq_playTalkText(uint8 chatNum) {
	assert(chatNum < _sequenceSoundListSize);

	if (chatNum < 12 && !_flags.isDemo && textEnabled())
		seq_setTextEntry(chatNum, 160, 168, _sequenceStringsDuration[chatNum], 160);

	_sound->voicePlay(_sequenceSoundList[chatNum], &_speechHandle);
}

void KyraEngine_HoF::seq_waitForTextsTimeout() {
	uint32 longest = seq_activeTextsTimeLeft() + _system->getMillis();
	uint32 now = _system->getMillis();

	if (textEnabled()) {
		if (longest > now)
			delay(longest - now);
	} else if (speechEnabled()) {
		while (snd_voiceIsPlaying())
			delay(_tickLength);
	}

	seq_resetAllTextEntries();
}

void KyraEngine_HoF::seq_resetAllTextEntries() {
	for (int i = 0; i < 10; i++)
		_activeText[i].duration = -1;
}

int KyraEngine_HoF::seq_setTextEntry(uint16 strIndex, uint16 posX, uint16 posY, int duration, uint16 width) {
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

void KyraEngine_HoF::seq_loadNestedSequence(int wsaNum, int seqNum) {
	if (_activeWSA[wsaNum].flags != -1)
		return;

	NestedSequence s = _sequences->seqn[seqNum];

	if (!_activeWSA[wsaNum].movie) {
		_activeWSA[wsaNum].movie = new WSAMovie_v2(this);
		assert(_activeWSA[wsaNum].movie);
	}

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
	_activeWSA[wsaNum].callback = _callbackN[seqNum];
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

void KyraEngine_HoF::seq_nestedSequenceFrame(int command, int wsaNum) {
	int xa = 0, ya = 0;
	command--;
	if (!_activeWSA[wsaNum].movie || skipFlag() || shouldQuit() || _abortIntroFlag)
		return;

	switch (command) {
	case 0:
		xa = -_activeWSA[wsaNum].movie->xAdd();
		ya = -_activeWSA[wsaNum].movie->yAdd();
		_activeWSA[wsaNum].movie->displayFrame(0, 8, xa, ya, 0, 0, 0);
		seq_animatedSubFrame(8, 2, 7, 8, _activeWSA[wsaNum].movie->xAdd(), _activeWSA[wsaNum].movie->yAdd(),
							_activeWSA[wsaNum].movie->width(), _activeWSA[wsaNum].movie->height(), 1, 2);
		break;

	case 1:
		xa = -_activeWSA[wsaNum].movie->xAdd();
		ya = -_activeWSA[wsaNum].movie->yAdd();
		_activeWSA[wsaNum].movie->displayFrame(0, 8, xa, ya, 0, 0, 0);
		seq_animatedSubFrame(8, 2, 7, 8, _activeWSA[wsaNum].movie->xAdd(), _activeWSA[wsaNum].movie->yAdd(),
							_activeWSA[wsaNum].movie->width(), _activeWSA[wsaNum].movie->height(), 1, 1);
		break;

	case 2:
		seq_waitForTextsTimeout();
		xa = -_activeWSA[wsaNum].movie->xAdd();
		ya = -_activeWSA[wsaNum].movie->yAdd();
		_activeWSA[wsaNum].movie->displayFrame(0x15, 8, xa, ya, 0, 0, 0);
		seq_animatedSubFrame(8, 2, 7, 8, _activeWSA[wsaNum].movie->xAdd(), _activeWSA[wsaNum].movie->yAdd(),
							_activeWSA[wsaNum].movie->width(), _activeWSA[wsaNum].movie->height(), 0, 2);
		break;

	case 3:
		_screen->copyPage(2, 10);
		_activeWSA[wsaNum].movie->displayFrame(0, 2, 0, 0, 0, 0, 0);
		_screen->copyPage(2, 12);
		seq_cmpFadeFrame("scene2.cmp");
		break;

	case 4:
		_screen->copyPage(2, 10);
		_activeWSA[wsaNum].movie->displayFrame(0, 2, 0, 0, 0, 0, 0);
		_screen->copyPage(2, 12);
		seq_cmpFadeFrame("scene3.cmp");
		break;

	default:
		break;
	}
}

void KyraEngine_HoF::seq_animatedSubFrame(int srcPage, int dstPage, int delaytime, int steps,
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

			_screen->copyPage(12, dstPage);
			delayUntil(endtime);
		}

		_screen->wsaFrameAnimationStep(0, 0, x, y, w, h, w, h, srcPage, dstPage, 0);
		_screen->copyPage(dstPage, 6);
		_screen->copyPage(dstPage, 0);
		_screen->updateScreen();
	} else {
		_screen->copyPage(12, dstPage);
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

			_screen->copyPage(12, dstPage);
			delayUntil(endtime);
		}
	}
}

void KyraEngine_HoF::seq_resetActiveWSA(int wsaNum) {
	if (_activeWSA[wsaNum].flags == -1)
		return;

	_activeWSA[wsaNum].flags = -1;
	seq_nestedSequenceFrame(_activeWSA[wsaNum].finalCommand, wsaNum);
	_activeWSA[wsaNum].movie->close();
}

void KyraEngine_HoF::seq_unloadWSA(int wsaNum) {
	if (_activeWSA[wsaNum].movie) {
		_activeWSA[wsaNum].movie->close();
		delete _activeWSA[wsaNum].movie;
		_activeWSA[wsaNum].movie = 0;
	}
}

bool KyraEngine_HoF::seq_processNextSubFrame(int wsaNum) {
	uint32 currentFrame = _activeWSA[wsaNum].currentFrame;
	uint32 currentTime = _system->getMillis();

	if (_activeWSA[wsaNum].callback && currentFrame != _activeWSA[wsaNum].lastFrame) {
		_activeWSA[wsaNum].lastFrame = currentFrame;
		currentFrame = (this->*_activeWSA[wsaNum].callback)(_activeWSA[wsaNum].movie, _activeWSA[wsaNum].x, _activeWSA[wsaNum].y, currentFrame);
	}

	if (_activeWSA[wsaNum].movie) {
		if (_activeWSA[wsaNum].flags & 0x20) {
			_activeWSA[wsaNum].movie->displayFrame(_activeWSA[wsaNum].control[currentFrame].index, 2, _activeWSA[wsaNum].x, _activeWSA[wsaNum].y, 0x4000, 0, 0);
			_activeWSA[wsaNum].frameDelay = _activeWSA[wsaNum].control[currentFrame].delay;
		} else {
			_activeWSA[wsaNum].movie->displayFrame(currentFrame % _activeWSA[wsaNum].movie->frames(), 2, _activeWSA[wsaNum].x, _activeWSA[wsaNum].y, 0x4000, 0, 0);
		}
	}

	if (_activeWSA[wsaNum].flags & 0x10) {
		currentFrame = (currentTime - _activeWSA[wsaNum].nextFrame) / (_activeWSA[wsaNum].frameDelay * _tickLength);
	} else {
		if (((int32)(currentTime - _activeWSA[wsaNum].nextFrame) / (int32)(_activeWSA[wsaNum].frameDelay * _tickLength)) > 0) {
			currentFrame++;
			_activeWSA[wsaNum].nextFrame = currentTime;
		}
	}

	bool res = false;

	if (currentFrame >= _activeWSA[wsaNum].endFrame) {
		int sw = ((_activeWSA[wsaNum].flags & 0x1e) - 2);
		switch (sw) {
		case 0:
			res = true;
			currentFrame = _activeWSA[wsaNum].endFrame;
			_screen->copyPage(2, 12);
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
		}
	}

	_activeWSA[wsaNum].currentFrame = currentFrame & 0xffff;
	return res;
}

void KyraEngine_HoF::seq_printCreditsString(uint16 strIndex, int x, int y, const uint8 *colorMap, uint8 textcolor) {
	uint8 colormap[16];
	if (skipFlag() || shouldQuit() || _abortIntroFlag || _menuChoice)
		return;

	memset(&_screen->getPalette(0)[0x2fa], 0x3f, 6);
	_screen->getPalette(0)[0x2f6] = 0x3f;
	_screen->getPalette(0)[0x2f5] = 0x20;
	_screen->getPalette(0)[0x2f4] = 0x30;
	colormap[0] = colorMap[0];
	colormap[1] = 0xfd;
	memcpy(&colormap[2], &colorMap[2], 14);
	uint8 seqTextColor0 = _seqTextColor[0];

	_seqTextColor[0] = 0xfd;
	_screen->setTextColorMap(colormap);
	seq_resetAllTextEntries();
	seq_setTextEntry(strIndex, x, y, 0x80, 0x78);
	seq_processText();
	_screen->copyPage(2, 0);
	_screen->updateScreen();
	_screen->getPalette(0)[0x2f7] = _screen->getPalette(0)[textcolor * 3];
	_screen->getPalette(0)[0x2f8] = _screen->getPalette(0)[textcolor * 3 + 1];
	_screen->getPalette(0)[0x2f9] = _screen->getPalette(0)[textcolor * 3 + 2];
	_screen->fadePalette(_screen->getPalette(0), 0x18);

	_seqTextColor[0] = textcolor;
	_screen->setTextColorMap(colorMap);
	seq_resetAllTextEntries();
	seq_setTextEntry(strIndex, x, y, 0x80, 0x78);
	seq_processText();
	_screen->copyPage(2, 0);
	_screen->updateScreen();
	_screen->getPalette(0)[0x2f7] = _screen->getPalette(0)[0x2f8] = _screen->getPalette(0)[0x2f9] = 0;
	_screen->fadePalette(_screen->getPalette(0), 1);
	_screen->copyPage(2, 12);
	seq_resetAllTextEntries();

	_seqTextColor[0] = seqTextColor0;
}

void KyraEngine_HoF::seq_playWsaSyncDialogue(uint16 strIndex, uint16 vocIndex, int textColor, int x, int y, int width, WSAMovie_v2 *wsa, int firstframe, int lastframe, int wsaXpos, int wsaYpos) {
	int dur = int(strlen(_sequenceStrings[strIndex])) * (_flags.isTalkie ? 7 : 15);
	int entry = textEnabled() ? seq_setTextEntry(strIndex, x, y, dur, width) : strIndex;
	_activeText[entry].textcolor = textColor;
	_seqWsaChatTimeout = _system->getMillis() + dur * _tickLength;
	int curframe = firstframe;

	if (vocIndex && speechEnabled()) {
		while (_sound->voiceIsPlaying() && !skipFlag())
			delay(4);
		seq_playTalkText(vocIndex);
	}

	while (_system->getMillis() < _seqWsaChatTimeout && !(_abortIntroFlag || skipFlag())) {
		if (lastframe < 0) {
			int t = ABS(lastframe);
			if (t < curframe)
				curframe = t;
		}

		if (ABS(lastframe) < curframe)
			curframe = firstframe;

		_seqWsaChatFrameTimeout = _seqEndTime = _system->getMillis() + _seqFrameDelay * _tickLength;
		if (wsa)
			wsa->displayFrame(curframe % wsa->frames(), 2, wsaXpos, wsaYpos, 0, 0, 0);

		_screen->copyPage(2, 12);

		seq_processText();

		uint32 tm = _system->getMillis();
		if (_seqWsaChatFrameTimeout > tm && _seqWsaChatTimeout > tm)
			delay(MIN(_seqWsaChatFrameTimeout - tm, _seqWsaChatTimeout - tm));

		if (speechEnabled() && !textEnabled() && !snd_voiceIsPlaying())
			break;

		_screen->copyPage(2, 0);
		_screen->updateScreen();
		curframe++;
	}

	if (_abortIntroFlag || skipFlag())
		_sound->voiceStop();

	if (ABS(lastframe) < curframe)
		curframe = ABS(lastframe);

	if (curframe == firstframe)
		curframe++;

	_seqWsaCurrentFrame = curframe;
}

void KyraEngine_HoF::seq_displayScrollText(uint8 *data, const ScreenDim *d, int tempPage1, int tempPage2, int speed,
	int step, Screen::FontId fid1, Screen::FontId fid2, const uint8 *shapeData, const char *const *specialData) {
	if (!data)
		return;

	static const char mark[] = { 5, 13, 0 };

	_screen->clearPage(tempPage1);
	_screen->clearPage(tempPage2);
	_screen->copyRegion(d->sx << 3, d->sy, d->sx << 3, d->sy, d->w << 3, d->h, 0, tempPage1);

	struct ScrollTextData {
		int16	x;
		int16	y;
		uint8	*text;
		byte	unk1;
		byte	height;
		byte	adjust;

		ScrollTextData() {
			x = 0;      // 0  11
			y = 0;		// 2  13
			text = 0;	// 4  15
			unk1 = 0;   // 8  19
			height = 0; // 9  20
			adjust = 0; // 10 21
		}
	};

	ScrollTextData *textData = new ScrollTextData[36];
	uint8 *ptr = data;

	bool loop = true;
	int cnt = 0;

	while (loop) {
		_seqSubFrameEndTimeInternal = _system->getMillis() + speed * _tickLength;

		while (cnt < 35 && *ptr) {
			uint16 cH;

			if (cnt)
				cH = textData[cnt].y + textData[cnt].height + (textData[cnt].height >> 3);
			else
				cH = d->h;

			char *str = (char *)ptr;

			ptr = (uint8*)strpbrk(str, mark);
			if (!ptr)
				ptr = (uint8*)strchr(str, 0);

			textData[cnt + 1].unk1 = *ptr;
			*ptr = 0;
			if (textData[cnt + 1].unk1)
				ptr++;

			if (*str == 3 || *str == 4)
				textData[cnt + 1].adjust = *str++;
			else
				textData[cnt + 1].adjust = 0;

			_screen->setFont(fid1);

			if (*str == 1) {
				_screen->setFont(fid2);
				str++;
			} else if (*str == 2) {
				str++;
			}

			textData[cnt + 1].height = _screen->getFontHeight();

			switch (textData[cnt + 1].adjust) {
			case 3:
				textData[cnt + 1].x = 157 - _screen->getTextWidth(str);
				break;
			case 4:
				textData[cnt + 1].x = 161;
				break;
			default:
				textData[cnt + 1].x = (((d->w << 3) - _screen->getTextWidth(str)) >> 1) + 1;
			}

			if (textData[cnt].unk1 == 5)
				cH -= (textData[cnt].height + (textData[cnt].height >> 3));

			textData[cnt + 1].y = cH;
			textData[cnt + 1].text = (uint8*)str;
			cnt++;
		}

		_screen->copyRegion(d->sx << 3, d->sy, d->sx << 3, d->sy, d->w << 3, d->h, tempPage1, tempPage2);

		int cnt2 = 0;
		bool palCycle = 0;

		while (cnt2 < cnt) {
			const char *str = (const char *)textData[cnt2 + 1].text;
			const char *str2 = str;
			int16 cW = textData[cnt2 + 1].x - 10;
			int16 cH = textData[cnt2 + 1].y;
			int x = (d->sx << 3) + cW;
			int y = d->sy + cH;
			int col1 = 255;

			if (cH < d->h) {
				_screen->setCurPage(tempPage2);
				_screen->setFont(fid1);
				if (textData[cnt2 + 1].height != _screen->getFontHeight())
					_screen->setFont(fid2);

				if (specialData) {
					if (!strcmp(str, specialData[0])) {
						col1 = 112;
						char cChar[2] = " ";
						while (*str2) {
							cChar[0] = *str2;
							_screen->printText(cChar, x, y, col1++, 0);
							x += _screen->getCharWidth(*str2++);
						}
						palCycle = true;
					} else if (!strcmp(str, specialData[1])) {
						col1 = 133;
						char cChar[2] = " ";
						while (*str2) {
							cChar[0] = *str2;
							_screen->printText(cChar, x, y, col1--, 0);
							x += _screen->getCharWidth(*str2++);
						}
						palCycle = true;
					} else {
						_screen->printText(str, x, y, col1, 0);
					}
				} else {
					_screen->printText(str, x, y, col1, 0);
				}
				_screen->setCurPage(0);
			}

			textData[cnt2 + 1].y -= step;
			cnt2++;
		}

		_screen->copyRegion(d->sx << 3, d->sy, d->sx << 3, d->sy, d->w << 3, d->h, tempPage2, 0);
		_screen->updateScreen();

		if (textData[1].y < -10) {
			textData[1].text += strlen((char *)textData[1].text);
			textData[1].text[0] = textData[1].unk1;
			cnt--;
			memcpy(&textData[1], &textData[2], cnt * sizeof(ScrollTextData));
		}

		if (palCycle) {
			for (int col = 133; col > 112; col--)
				_screen->getPalette(0).copy(_screen->getPalette(0), col - 1, 1, col);
			_screen->getPalette(0).copy(_screen->getPalette(0), 133, 1, 112);
			_screen->setScreenPalette(_screen->getPalette(0));
		}

		delayUntil(_seqSubFrameEndTimeInternal);

		if ((cnt < 36) && ((d->sy + d->h) > (textData[cnt].y + textData[cnt].height)) && !skipFlag()) {
			resetSkipFlag();
			delay(_tickLength * 500);
			cnt = 0;
		}

		if (!cnt || skipFlag())
			loop = false;
	}

	_sound->beginFadeOut();
	_screen->fadeToBlack();

	_abortIntroFlag= false;
	resetSkipFlag();

	delete[] textData;
}

void KyraEngine_HoF::seq_scrollPage(int bottom, int top) {
	int dstY, dstH, srcH;

	static const ScreenDim d = { 0x00, 0x00, 0x28, 0x320, 0xFF, 0xFE, 0x00, 0x00 };

	if (_seqScrollTextCounter - (top - 1) < 0) {
		dstY = top - _seqScrollTextCounter;
		dstH = _seqScrollTextCounter;
		srcH = 0;
	} else {
		dstY = 0;
		srcH = _seqScrollTextCounter - top;
		dstH = (400 - srcH <= top) ? 400 - srcH : top;
	}

	if (dstH > 0) {
		if (_demoAnimData) {
			for (int i = 0; i < 4; i++) {
				const ItemAnimData_v1 *def = &_demoAnimData[i];
				ActiveItemAnim *a = &_activeItemAnim[i];

				_screen->fillRect(12, def->y - 8, 28, def->y + 8, 0, 4);
				_screen->drawShape(4, getShapePtr(def->itemIndex + def->frames[a->currentFrame]), 12, def->y - 8, 0, 0);
				if (_seqFrameCounter % 2 == 0)
					a->currentFrame = ++a->currentFrame % 20;
			}
		}
		_screen->copyRegionEx(4, 0, srcH, 2, 2, dstY + bottom, 320, dstH, &d);
	}
}

void KyraEngine_HoF::seq_showStarcraftLogo() {
	WSAMovie_v2 *ci = new WSAMovie_v2(this);
	assert(ci);
	_screen->clearPage(2);
	_res->loadPakFile("INTROGEN.PAK");
	int endframe = ci->open("ci.wsa", 0, &_screen->getPalette(0));
	_res->unloadPakFile("INTROGEN.PAK");
	if (!ci->opened()) {
		delete ci;
		return;
	}
	_screen->hideMouse();
	ci->displayFrame(0, 2, 0, 0, 0, 0, 0);
	_screen->copyPage(2, 0);
	_screen->fadeFromBlack();
	for (int i = 1; i < endframe; i++) {
		_seqEndTime = _system->getMillis() + 50;
		if (skipFlag())
			break;
		ci->displayFrame(i, 2, 0, 0, 0, 0, 0);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		delay(_seqEndTime - _system->getMillis());
	}
	if (!skipFlag()) {
		_seqEndTime = _system->getMillis() + 50;
		ci->displayFrame(0, 2, 0, 0, 0, 0, 0);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		delay(_seqEndTime - _system->getMillis());
	}
	_screen->fadeToBlack();
	_screen->showMouse();

	_eventList.clear();
	delete ci;
}

void KyraEngine_HoF::seq_init() {
	_seqProcessedString = new char[200];
	_seqWsa = new WSAMovie_v2(this);
	_activeWSA = new ActiveWSA[8];
	_activeText = new ActiveText[10];

	_res->unloadAllPakFiles();
	_res->loadPakFile(StaticResource::staticDataFilename());
	_res->loadFileList(_sequencePakList, _sequencePakListSize);

	if (_flags.platform == Common::kPlatformPC98)
		_sound->loadSoundFile("sound.dat");

	int numShp = -1;

	if (_flags.gameID == GI_LOL)
		return;

	if (_flags.isDemo && !_flags.isTalkie) {
		_demoAnimData = _staticres->loadShapeAnimData_v1(k2SeqplayShapeAnimData, _itemAnimDataSize);
		uint8 *shp = _res->fileData("icons.shp", 0);
		uint32 outsize = READ_LE_UINT16(shp + 4);
		_animShapeFiledata = new uint8[outsize];
		Screen::decodeFrame4(shp + 10, _animShapeFiledata, outsize);
		delete[] shp;

		do {
			numShp++;
			addShapeToPool(_screen->getPtrToShape(_animShapeFiledata, numShp), numShp);
		} while (getShapePtr(numShp));
	} else {
		MainMenu::StaticData data = {
			{ _sequenceStrings[97], _sequenceStrings[96], _sequenceStrings[95], _sequenceStrings[98], 0 },
			{ 0x01, 0x04, 0x0C, 0x04, 0x00, 0xd7, 0xd6 },
			{ 0xd8, 0xda, 0xd9, 0xd8 },
			Screen::FID_8_FNT, 240
		};
		_menu = new MainMenu(this);
		_menu->init(data, MainMenu::Animation());
	}
}

void KyraEngine_HoF::seq_uninit() {
	delete[] _seqProcessedString;
	_seqProcessedString = NULL;

	delete[] _activeWSA;
	_activeWSA = NULL;

	delete[] _activeText;
	_activeText = NULL;

	delete _seqWsa;
	_seqWsa = NULL;

	delete[] _animShapeFiledata;
	_animShapeFiledata = 0;

	if (_flags.isDemo && !_flags.isTalkie)
		_staticres->unloadId(k2SeqplayShapeAnimData);

	delete _menu;
	_menu = 0;
}

#pragma mark -
#pragma mark - Ingame sequences
#pragma mark -

void KyraEngine_HoF::seq_makeBookOrCauldronAppear(int type) {
	_screen->hideMouse();
	showMessage(0, 0xCF);

	if (type == 1)
		seq_makeBookAppear();
	else if (type == 2)
		loadInvWsa("CAULDRON.WSA", 1, 6, 0, -2, -2, 1);

	_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _screenBuffer);
	_screen->loadBitmap("_PLAYALL.CPS", 3, 3, 0);

	static const uint8 bookCauldronRects[] = {
		0x46, 0x90, 0x7F, 0x2B,	// unknown rect (maybe unused?)
		0xCE, 0x90, 0x2C, 0x2C,	// book rect
		0xFA, 0x90, 0x46, 0x2C	// cauldron rect
	};

	int x = bookCauldronRects[type*4+0];
	int y = bookCauldronRects[type*4+1];
	int w = bookCauldronRects[type*4+2];
	int h = bookCauldronRects[type*4+3];
	_screen->copyRegion(x, y, x, y, w, h, 2, 0, Screen::CR_NO_P_CHECK);

	_screen->copyBlockToPage(2, 0, 0, 320, 200, _screenBuffer);

	if (type == 2) {
		int32 countdown = _rnd.getRandomNumberRng(45, 80);
		_timer->setCountdown(2, countdown * 60);
	}

	_screen->showMouse();
}

void KyraEngine_HoF::seq_makeBookAppear() {
	_screen->hideMouse();

	displayInvWsaLastFrame();

	showMessage(0, 0xCF);

	loadInvWsa("BOOK2.WSA", 0, 4, 2, -1, -1, 0);

	uint8 *rect = new uint8[_screen->getRectSize(_invWsa.w, _invWsa.h)];
	assert(rect);

	_screen->copyRegionToBuffer(_invWsa.page, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, rect);

	_invWsa.running = false;
	snd_playSoundEffect(0xAF);

	while (true) {
		_invWsa.timer = _system->getMillis() + _invWsa.delay * _tickLength;

		_screen->copyBlockToPage(_invWsa.page, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, rect);

		_invWsa.wsa->displayFrame(_invWsa.curFrame, _invWsa.page, 0, 0, 0x4000, 0, 0);

		if (_invWsa.page)
			_screen->copyRegion(_invWsa.x, _invWsa.y, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, _invWsa.page, 0, Screen::CR_NO_P_CHECK);

		++_invWsa.curFrame;

		if (_invWsa.curFrame >= _invWsa.lastFrame && !shouldQuit())
			break;

		switch (_invWsa.curFrame) {
		case 39:
			snd_playSoundEffect(0xCA);
			break;

		case 50:
			snd_playSoundEffect(0x6A);
			break;

		case 72:
			snd_playSoundEffect(0xCB);
			break;

		case 85:
			snd_playSoundEffect(0x38);
			break;

		default:
			break;
		}

		do {
			update();
		} while (_invWsa.timer > _system->getMillis() && !skipFlag());
	}

	closeInvWsa();
	delete[] rect;
	_invWsa.running = false;

	_screen->showMouse();
}

} // end of namespace Kyra


