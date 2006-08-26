/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
#include "kyra/kyra2.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_v2::seq_playSequences(int startSeq, int endSeq) {
	if (endSeq == -1)
		endSeq = startSeq;
	
	assert(startSeq >= 0 && endSeq < 4 && startSeq <= endSeq);

	static const Sequence sequences[] = { 
		// type, filename, callback, framedelay, duration, numframes, fadeOut, timeOut
		{2, "virgin.cps",   0,                                 100, 0,   1,  true,  true},
		{1, "westwood.wsa", &KyraEngine_v2::seq_introWestwood, 6,   160, 18, true,  true},
		{1, "title.wsa",    &KyraEngine_v2::seq_introTitle,    6,   10,  26, false,  false},
		{2, "over.cps",     &KyraEngine_v2::seq_introOverview, 16,  30,  1,  false, true}
	};

	_activeWSA = new ActiveWSA[8];
	assert(_activeWSA);	
	memset(_activeWSA, 0, sizeof(ActiveWSA) * 8);

	_screen->hideMouse();

	uint8 pal[768];
	memset(pal, 0, sizeof(pal));
	_screen->setScreenPalette(pal);
	
	for (int i = startSeq; i <= endSeq; i++) {
		uint32 seqDelay = 0;
		int seqNum = 0;

		_screen->clearPage(0);

		if (sequences[i].type == 2) {
			_screen->loadBitmap(sequences[i].filename, 0, 0, _screen->_currentPalette);
			_screen->updateScreen();
			seqDelay = sequences[i].frameDelay * _tickLength;
		} else if(sequences[i].type == 1) {
			seq_loadWSA(0, sequences[i].filename, sequences[i].frameDelay);
			seqDelay = sequences[i].duration * _tickLength;
		}

		if (sequences[i].callback)
			(*this.*sequences[i].callback)(seqNum++);

		seq_playWSAs();
		_screen->updateScreen();
		_screen->fadeFromBlack(40);

		seqDelay += _system->getMillis();
		bool mayEndLoop = sequences[i].timeOut;
		
		while(1) {
			uint32 startTime = _system->getMillis();
			
			if (sequences[i].callback) {
				int newTime = (*this.*sequences[i].callback)(seqNum++);
				if (newTime != -1) {
					seqDelay = newTime * _tickLength + _system->getMillis();
					mayEndLoop = true;
				}
			}
		
			seq_playWSAs();
			_screen->updateScreen();
			
			uint32 currTime = _system->getMillis();
			if (seqDelay <= currTime && mayEndLoop)
				break;
			else {
				uint32 loopTime = currTime - startTime;
				delay(loopTime > _tickLength ? loopTime : _tickLength);
			}
		}

		if (sequences[i].fadeOut)
			_screen->fadeToBlack(40);
		
		if (sequences[i].type == 1)
			seq_unloadWSA(0);
	}
	_screen->showMouse();
	delete[] _activeWSA;
}

int KyraEngine_v2::seq_introOverview(int seqNum) {
	switch (seqNum) {
		case 0:
			_sound->playTrack(4);
			break;
		case 40:
			seq_loadWSA(1, "over1.wsa", 10);
			break;
		case 60:
			seq_loadWSA(2, "over2.wsa", 9);
			break;
		case 282:
			seq_loadWSA(3, "forest.wsa", 6);
			break;
		case 434:
			seq_loadWSA(4, "dragon.wsa", 6);
			break;
		case 540:
			seq_unloadWSA(1);
			seq_unloadWSA(2);
			seq_unloadWSA(3);
			seq_unloadWSA(4);
			return 0;
			break;	
	}

	return -1;
}

int KyraEngine_v2::seq_introTitle(int seqNum) {
	if (seqNum == 1)
		_sound->playTrack(3);
	else if (seqNum == 25) {
		// XXX: handle menu
		return 200;
	}

	return -1;
}

int KyraEngine_v2::seq_introWestwood(int seqNum) {
	if (seqNum == 0)
		_sound->playTrack(2);

	return -1;
}

void KyraEngine_v2::seq_playWSAs() {
	uint32 currTime = _system->getMillis();

	for (int i = 0; i < 8; i++) {
		if (_activeWSA[i].movie && currTime >= _activeWSA[i].nextFrame && _activeWSA[i].currentFrame < _activeWSA[i].endFrame) {
			_activeWSA[i].movie->displayFrame(_activeWSA[i].currentFrame++);
			_activeWSA[i].nextFrame = currTime + _activeWSA[i].frameDelay * _tickLength;
		}
	}
}

void KyraEngine_v2::seq_loadWSA(int wsaNum, const char *filename, int frameDelay) {
	_activeWSA[wsaNum].movie = new WSAMovieV2(this);
	assert(_activeWSA[wsaNum].movie);
	_activeWSA[wsaNum].endFrame = _activeWSA[wsaNum].movie->open(filename, 0, _screen->_currentPalette);
	assert(_activeWSA[wsaNum].movie->opened());
	_activeWSA[wsaNum].currentFrame = 0;
	_activeWSA[wsaNum].frameDelay = frameDelay;
	_activeWSA[wsaNum].nextFrame = _system->getMillis();
	_activeWSA[wsaNum].movie->setX(0);
	_activeWSA[wsaNum].movie->setY(0);	
	_activeWSA[wsaNum].movie->setDrawPage(0);	
}

void KyraEngine_v2::seq_unloadWSA(int wsaNum) {
	assert(_activeWSA[wsaNum].movie);
	_activeWSA[wsaNum].movie->close();
	delete _activeWSA[wsaNum].movie;
}

} // end of namespace Kyra
