/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "kyra/kyra.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {
KyraEngine_v3::KyraEngine_v3(OSystem *system) : KyraEngine(system) {
	_soundDigital = 0;
	_musicSoundChannel = -1;
	_menuAudioFile = "TITLE1.AUD";
}

KyraEngine_v3::~KyraEngine_v3() {
}

Movie *KyraEngine_v3::createWSAMovie() {
	return new WSAMovieV3(this);
}

int KyraEngine_v3::init() {
	KyraEngine::init();
	
	_soundDigital = new SoundDigital(this, _mixer);
	assert(_soundDigital);
	assert(_soundDigital->init());
	
	return 0;
}

int KyraEngine_v3::go() {
	_screen->_curPage = 0;
	_screen->clearPage(0);
	
	byte *pal = new byte[768];
	assert(pal);
	memset(pal, 0, sizeof(byte)*768);
	
	Movie *logo = createWSAMovie();
	logo->open("REVENGE.WSA", 1, pal);
	assert(logo->opened());
	
	pal[0] = pal[1] = pal[2] = 0;
	
	_screen->setScreenPalette(pal);
	
	// XXX
	playMenuAudioFile();
	
	logo->setX(0); logo->setY(0);
	logo->setDrawPage(0);

	for (int i = 0; i < 29; ++i) {
		uint32 nextRun = _system->getMillis() + 3 * _tickLength;
		logo->displayFrame(i);
		_screen->updateScreen();
		delayUntil(nextRun);
	}

	while (1) {
		for (int i = 29; i < 64; ++i) {
			uint32 nextRun = _system->getMillis() + 3 * _tickLength;
			logo->displayFrame(i);
			_screen->updateScreen();
			delayUntil(nextRun);
		}
	
		for (int i = 64; i > 29; --i) {
			uint32 nextRun = _system->getMillis() + 3 * _tickLength;
			logo->displayFrame(i);
			_screen->updateScreen();
			delayUntil(nextRun);
		}
	}
	
	delete [] pal;

	return 0;
}

void KyraEngine_v3::playMenuAudioFile() {
	Common::File *handle = new Common::File();
	uint32 temp = 0;
	_res->fileHandle(_menuAudioFile, &temp, *handle);
	if (handle->isOpen()) {
		_musicSoundChannel = _soundDigital->playSound(handle, true, -1);
	}
}
}
