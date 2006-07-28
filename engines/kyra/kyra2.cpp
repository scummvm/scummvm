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
#include "kyra/kyra2.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"

#include "common/system.h"

namespace Kyra {

KyraEngine_v2::KyraEngine_v2(OSystem *system)
	: KyraEngine(system) {
}

KyraEngine_v2::~KyraEngine_v2() {
}

int KyraEngine_v2::go() {	
	uint8 pal[768];
	
	WSAMovieV2 *title = new WSAMovieV2(this);
	title->open("title.WSA", 0, pal);
	assert(title->opened());
	
	_screen->setScreenPalette(pal);
	title->setX(0); title->setY(0);
	title->setDrawPage(0);
	for (int i = 0; i < 26; ++i) {
		uint32 nextRun = _system->getMillis() + 6 * _tickLength;
		title->displayFrame(i);
		_screen->updateScreen();
		delayUntil(nextRun);
	}

	delete title;
	
	waitForEvent();
	return 0;
}

} // end of namespace Kyra
