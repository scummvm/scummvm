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
#include "kyra/text.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_v2::seq_playSequences(int startSeq, int endSeq) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_playSequences(%i, %i)", startSeq, endSeq);

	_skipFlag = false;

	if (endSeq == -1)
		endSeq = startSeq;
	
	static const Sequence sequences[] = { 
		// type, filename, callback, framedelay, duration, numframes, timeOut, fadeOut
		{2, "virgin.cps",   0,                                 100, 0,   1,  true,  true},
		{1, "westwood.wsa", &KyraEngine_v2::seq_introWestwood, 6,   160, 18, true,  true},
		{1, "title.wsa",    &KyraEngine_v2::seq_introTitle,    6,   10,  26, false, false},
		{2, "over.cps",     &KyraEngine_v2::seq_introOverview, 16,  30,  1,  false, true},
		{2, "library.cps",  &KyraEngine_v2::seq_introLibrary,  16,  30,  1,  false, true},
		{2, "hand.cps",     &KyraEngine_v2::seq_introHand,     16,  90,  1,  false, true}
	};

	assert(startSeq >= 0 && endSeq < ARRAYSIZE(sequences) && startSeq <= endSeq);

	_activeWSA = new ActiveWSA[8];
	assert(_activeWSA);	
	memset(_activeWSA, 0, sizeof(ActiveWSA) * 8);

	_activeChat = new ActiveChat[10];
	assert(_activeChat);	
	memset(_activeChat, 0, sizeof(ActiveChat) * 10);

	seq_resetAllChatEntries();

	_screen->hideMouse();
	int oldPage = _screen->setCurPage(2);
	
	uint8 pal[768];
	memset(pal, 0, sizeof(pal));
	
	for (int i = startSeq; i <= endSeq && !_skipFlag; i++) {
		uint32 seqDelay = 0;
		int seqNum = 0;

		_screen->setScreenPalette(pal);
		_screen->clearPage(0);

		if (sequences[i].type == 2) {
			_screen->loadBitmap(sequences[i].filename, 2, 2, _screen->_currentPalette);
			_screen->updateScreen();
			seqDelay = sequences[i].frameDelay * _tickLength;
		} else if(sequences[i].type == 1) {
			seq_loadWSA(0, sequences[i].filename, sequences[i].frameDelay);
			seqDelay = sequences[i].duration * _tickLength;
		}

		if (sequences[i].callback)
			(*this.*sequences[i].callback)(seqNum++);

		seq_playWSAs();
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		_screen->fadeFromBlack(40);

		seqDelay += _system->getMillis();
		bool mayEndLoop = sequences[i].timeOut;
		
		while (!_quitFlag && !_skipFlag) {
			uint32 startTime = _system->getMillis();
			
			if (sequences[i].callback) {
				int newTime = (*this.*sequences[i].callback)(seqNum++);
				if (newTime != -1) {
					seqDelay = newTime * _tickLength + _system->getMillis();
					mayEndLoop = true;
				}
			}
		
			seq_playWSAs();
			_screen->copyPage(2, 0);
			seq_showChats();
			_screen->updateScreen();
			
			uint32 currTime = _system->getMillis();
			if (seqDelay <= currTime && mayEndLoop) {
				break;
			} else {
				uint32 loopTime = currTime - startTime;
				delay(loopTime < _tickLength ? loopTime : _tickLength);
			}
		}

		if (sequences[i].fadeOut)
			_screen->fadeToBlack(40);
		
		if (sequences[i].type == 1)
			seq_unloadWSA(0);
		
		_screen->clearPage(2);
		
	}
	_screen->setCurPage(oldPage);
	_screen->showMouse();

	for (int i = 0; i < 8; i++)
		seq_unloadWSA(i);
	delete[] _activeWSA;
	delete[] _activeChat;
}

int KyraEngine_v2::seq_introHand(int seqNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introHand(%i)", seqNum);
	static const SequenceControl hand1bWSAControl[] = {
		{0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 6}, {7, 6},
		{8, 6}, {9, 6}, {10, 6}, {11, 6}, {11, 12}, {12, 12}, {13, 12}, 
		{12, 12}, {11, 12}, {-1, -1} };
	
	static const SequenceControl hand1cWSAControl[] = {
		{0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {3, 6},
		{4, 6}, {5, 64}, {5, 6}, {-1, -1} };
	
	static const SequenceControl hand2WSAControl[] = {
		{0, 6}, {1, 6}, {0, 6}, {1, 6}, {0, 6}, {1, 6},
		{0, 6}, {1, 6}, {0, 6}, {1, 6}, {0, 6}, {1, 6},
		{0, 6}, {1, 6}, {0, 6}, {1, 6}, {-1, -1} };

	static const SequenceControl hand3WSAControl[] = {
		{0, 6}, {1, 6}, {2, 6}, {1, 6},
		{0, 6}, {-1, -1} };

	static const SequenceControl hand4WSAControl[] = {
		{0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6},
		{3, 6}, {2, 6}, {1, 6}, {-1, -1} };
	
	switch (seqNum) {
		case 0:
			_sound->playTrack(6);
			seq_playIntroChat(7);
			//palette stuff
			break;
		case 1:
			// XXX: these show as garbage. New frame encode?
			seq_loadWSA(1, "hand1a.wsa", 9);
			seq_loadWSA(2, "hand1b.wsa", 9, 0, hand1bWSAControl);
			seq_loadWSA(3, "hand1c.wsa", 9, 0, hand1cWSAControl);
			break;
		case 0xc9:
			// palette stuff
			seq_loadWSA(4, "hand2.wsa", 9, 0, hand2WSAControl);
			seq_playIntroChat(8);
			break;
		case 0x18b:
			seq_waitForChatsToFinish();
			seq_loadWSA(5, "hand3.wsa", 9, 0, hand3WSAControl);
			break;
		case 0x1f4:
			seq_waitForChatsToFinish();
			seq_loadWSA(6, "hand4.wsa", 9, 0, hand4WSAControl);
			break;
		case 0x21c:
			seq_playIntroChat(10);
			break;
		case 0x276:
			seq_waitForChatsToFinish();
			seq_unloadWSA(1);
			seq_unloadWSA(2);	
			seq_unloadWSA(3);
			seq_unloadWSA(4);
			seq_unloadWSA(5);
			seq_unloadWSA(6);						
			return 0;
	}
	
	return -1;
}

int KyraEngine_v2::seq_introLibrary(int seqNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introLibrary(%i)", seqNum);
	
	static const SequenceControl libraryWSAControl[] = {
		{0, 10}, {1, 10}, {2, 10}, {3, 10}, {4, 10}, {5, 10}, {6, 10}, {7, 10},
		{8, 10}, {9, 10}, {8, 10}, {7, 10}, {6, 10}, {5, 40}, {4, 10}, {3, 10},
		{2, 10}, {1, 10}, {-1, -1} };
	
	switch (seqNum) {
		case 0:
			_sound->playTrack(5);
			seq_playIntroChat(4);
			//XXX: palette stuff
			break;
		case 1:
			seq_loadWSA(1, "library.wsa", 9, 0, libraryWSAControl);
			break;
		case 0x64:
			seq_waitForChatsToFinish();
			// unk1 = 7;
			// palette/screen stuff
			seq_loadWSA(2, "darm.wsa", 9);
			break;
		case 0x68:
			seq_playIntroChat(5);
			break;
		case 0xF0:
			seq_waitForChatsToFinish();
			seq_loadWSA(3, "library.wsa", 9);
			break;
		case 0x154:
			// palette stuff
			seq_loadWSA(4, "marco.wsa", 9);
			seq_playIntroChat(6);
			break;
		case 0x294:
			seq_waitForChatsToFinish();
			seq_unloadWSA(1);
			seq_unloadWSA(2);
			seq_unloadWSA(3);
			seq_unloadWSA(4);
			return 0;
		default:
			break;
	}

	return -1;
}

int KyraEngine_v2::seq_introOverview(int seqNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introOverview(%i)", seqNum);
	
	switch (seqNum) {
		case 0:
			_sound->playTrack(4);
			break;
		case 40:
			seq_loadWSA(1, "over1.wsa", 10, &KyraEngine_v2::seq_introOverviewOver1);
			break;
		case 60:
			seq_loadWSA(2, "over2.wsa", 9);
			break;
		case 120:
			seq_playIntroChat(0);
			break;
		case 200:
			seq_waitForChatsToFinish();
			// XXX: fade to grey
			break;
		case 201:
			// XXX
			break;
		case 282:
			seq_loadWSA(3, "forest.wsa", 6,  &KyraEngine_v2::seq_introOverviewForest);
			seq_playIntroChat(1);
			break;
		case 434:
			seq_loadWSA(4, "dragon.wsa", 6,  &KyraEngine_v2::seq_introOverviewDragon);
			break;
		case 540:
			seq_waitForChatsToFinish();
			seq_unloadWSA(1);
			seq_unloadWSA(2);
			seq_unloadWSA(3);
			seq_unloadWSA(4);
			return 0;
			break;	
	}

	return -1;
}

void KyraEngine_v2::seq_introOverviewOver1(int currentFrame) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introOverviewOver1(%i)", currentFrame);
	
	if (currentFrame == 2)
		seq_waitForChatsToFinish();
	else if(currentFrame == 3)
		seq_playIntroChat(12);
}

void KyraEngine_v2::seq_introOverviewForest(int currentFrame) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introOverviewForest(%i)", currentFrame);
	
	if (currentFrame == 11) {
		seq_waitForChatsToFinish();
	} else if(currentFrame == 12) {
		delay(25);
		seq_playIntroChat(2);
	}
}

void KyraEngine_v2::seq_introOverviewDragon(int currentFrame) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introOverviewDragon(%i)", currentFrame);
	
	if (currentFrame == 3)
		seq_playIntroChat(3);
	else if(currentFrame == 11)
		seq_waitForChatsToFinish();
}

int KyraEngine_v2::seq_introTitle(int seqNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introtitle(%i)", seqNum);
	
	if (seqNum == 1) {
		_sound->playTrack(3);
	} else if (seqNum == 25) {
		// XXX: handle menu
		return 200;
	}

	return -1;
}

int KyraEngine_v2::seq_introWestwood(int seqNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_introWestwood(%i)", seqNum);
	
	if (seqNum == 0)
		_sound->playTrack(2);

	return -1;
}

void KyraEngine_v2::seq_playIntroChat(uint8 chatNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_playIntroChat(%i)", chatNum);
	
	assert(chatNum < _introSoundListSize);
	
	if (chatNum < 12)
		seq_setChatEntry(chatNum, 160, 168, _introStringsDuration[chatNum], 160);
	_sound->voicePlay(_introSoundList[chatNum]);
}

void KyraEngine_v2::seq_waitForChatsToFinish() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_waitForChatsToFinish()");
	
	uint32 longest = 0;
	
	for (int i = 0; i < 10; i++) {
		if (_activeChat[i].duration != -1) {
			uint32 currChatTime = _activeChat[i].duration + _activeChat[i].startTime;
			if ( currChatTime > longest)
				longest = currChatTime;
		}
	}

	uint32 now = _system->getMillis();
	if (longest > now)
		delay(longest - now);
}

void KyraEngine_v2::seq_resetAllChatEntries() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_resetAllChatEntries()");
	
	for (int i = 0; i < 10; i++)
		_activeChat[i].duration = -1;
}

void KyraEngine_v2::seq_setChatEntry(uint16 strIndex, uint16 posX, uint16 posY, int duration, uint16 unk1) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_setChatEntry(%i, %i, %i, %i, %i)", strIndex, posX, posY, duration, unk1);
	
	for (int i = 0; i < 10; i++) {
		if (_activeChat[i].duration != -1)
			continue;

		_activeChat[i].strIndex = strIndex;
		_activeChat[i].x = posX;
		_activeChat[i].y = posY;
		_activeChat[i].duration = duration * _tickLength;
		_activeChat[i].field_8 = unk1;
		_activeChat[i].startTime = _system->getMillis();

		return;
	}
}

void KyraEngine_v2::seq_showChats() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_showChats()");
	
	uint32 now = _system->getMillis();

	for (int i = 0; i < 10; i++) {
		if (_activeChat[i].duration != -1) {
			if ((_activeChat[i].startTime + (uint32)_activeChat[i].duration) > now) {
				assert(_activeChat[i].strIndex < _introStringsSize);
				
				_text->printIntroTextMessage(_introStrings[_activeChat[i].strIndex], _activeChat[i].x, _activeChat[i].y + 12,
					0xfe, 150 /*_activeChat[i].field_8*/, 0x0, 0, Screen::FID_GOLDFONT_FNT);
			} else
				_activeChat[i].duration = -1;
		}
	}
}

void KyraEngine_v2::seq_playWSAs() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_playWSAs()");
	
	uint32 currTime = _system->getMillis();

	for (int i = 0; i < 8; i++) {
			int currentFrame, frameDelay;

			if (_activeWSA[i].control) {
				int8 nextFrame = _activeWSA[i].control[_activeWSA[i].currentFrame].frameIndex;
				if (nextFrame == -1)
					continue;
				
				currentFrame = nextFrame;
				frameDelay = _activeWSA[i].control[_activeWSA[i].currentFrame].frameDelay;				
			} else {
				if (_activeWSA[i].currentFrame >= _activeWSA[i].endFrame)
					continue;				

				currentFrame = _activeWSA[i].currentFrame;
				frameDelay = _activeWSA[i].frameDelay;
			}

			_activeWSA[i].movie->displayFrame(currentFrame);

			if (_activeWSA[i].movie && currTime >= _activeWSA[i].nextFrame) {
				if (_activeWSA[i].callback != 0)
					(*this.*_activeWSA[i].callback)(currentFrame);
				_activeWSA[i].currentFrame++;
				_activeWSA[i].nextFrame = currTime + frameDelay * _tickLength;
			}
	}
}

void KyraEngine_v2::seq_loadWSA(int wsaNum, const char *filename, int frameDelay,
								void (KyraEngine_v2::*callback)(int), const SequenceControl *control) {

	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_loadWSA(%i, %s, %i, %i)", wsaNum, filename, frameDelay, callback ? true : false);
	
	_activeWSA[wsaNum].movie = new WSAMovieV2(this);
	assert(_activeWSA[wsaNum].movie);
	_activeWSA[wsaNum].endFrame = _activeWSA[wsaNum].movie->open(filename, 0, _screen->_currentPalette);
	assert(_activeWSA[wsaNum].movie->opened());
	_activeWSA[wsaNum].currentFrame = 0;
	_activeWSA[wsaNum].frameDelay = frameDelay;
	_activeWSA[wsaNum].nextFrame = _system->getMillis();
	_activeWSA[wsaNum].movie->setX(0);
	_activeWSA[wsaNum].movie->setY(0);	
	_activeWSA[wsaNum].movie->setDrawPage(_screen->_curPage);
	_activeWSA[wsaNum].callback = callback;	
	_activeWSA[wsaNum].control = control;
}

void KyraEngine_v2::seq_unloadWSA(int wsaNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_unloadWSA(%i)", wsaNum);
	
	if (_activeWSA[wsaNum].movie) {
		_activeWSA[wsaNum].movie->close();
		delete _activeWSA[wsaNum].movie;
		_activeWSA[wsaNum].movie = 0;
	}
}

} // end of namespace Kyra

