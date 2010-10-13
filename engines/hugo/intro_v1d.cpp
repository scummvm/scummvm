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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/intro.h"
#include "hugo/display.h"

namespace Hugo {
intro_v1d::intro_v1d(HugoEngine &vm) : IntroHandler(vm) {
}

intro_v1d::~intro_v1d() {
}

void intro_v1d::preNewGame() {
}

void intro_v1d::introInit() {
	introTicks = 0;
}

bool intro_v1d::introPlay() {
	static int state = 0;
	byte introSize = _vm.getIntroSize();

	if (introTicks < introSize) {
		switch (state++) {
		case 0:
			_vm.screen().drawRectangle(true, 0, 0, 319, 199, _TMAGENTA);
			_vm.screen().drawRectangle(true, 10, 10, 309, 189, _TBLACK);
			break;

		case 1:
			_vm.screen().drawShape(20, 92,_TLIGHTMAGENTA,_TMAGENTA);
			_vm.screen().drawShape(250,92,_TLIGHTMAGENTA,_TMAGENTA);

			// HACK: use of TROMAN, size 10-5
			_vm.screen().loadFont(0);

			char buffer[80];
			if (_boot.registered)
				strcpy(buffer, "Registered Version");
			else
				strcpy(buffer, "Shareware Version");
			_vm.screen().writeStr(CENTER, 163, buffer, _TLIGHTMAGENTA);
			_vm.screen().writeStr(CENTER, 176, COPYRIGHT, _TLIGHTMAGENTA);

			if (scumm_stricmp(_boot.distrib, "David P. Gray")) {
				sprintf(buffer, "Distributed by %s.", _boot.distrib);
				_vm.screen().writeStr(CENTER, 75, buffer, _TMAGENTA);
			}

			// HACK: use of SCRIPT size 24-16
			_vm.screen().loadFont(2);

			strcpy(buffer, "Hugo's");
			_vm.screen().writeStr(CENTER, 20, buffer, _TMAGENTA);

			// HACK: use of TROMAN, size 30-24
			strcpy(buffer, "House of Horrors !");
			_vm.screen().writeStr(CENTER, 50, buffer, _TLIGHTMAGENTA);
			break;
		case 2:
			_vm.screen().drawRectangle(true, 82, 92, 237, 138, _TBLACK);
			// HACK: use of TROMAN, size 16-9
			_vm.screen().loadFont(2);

			strcpy(buffer, "S t a r r i n g :");
			_vm.screen().writeStr(CENTER, 95, buffer, _TMAGENTA);
			break;
		case 3:
			// HACK: use of TROMAN size 20-9
			_vm.screen().loadFont(2);

			strcpy(buffer, "Hugo !");
			_vm.screen().writeStr(CENTER, 115, buffer, _TLIGHTMAGENTA);
			break;
		case 4:
			_vm.screen().drawRectangle(true, 82, 92, 237, 138, _TBLACK);
			// HACK: use of TROMAN size 16-9
			_vm.screen().loadFont(2);

			strcpy(buffer, "P r o d u c e d  b y :");
			_vm.screen().writeStr(CENTER, 95, buffer, _TMAGENTA);
			break;
		case 5:
			// HACK: use of TROMAN size 16-9
			_vm.screen().loadFont(2);

			strcpy(buffer, "David P Gray !");
			_vm.screen().writeStr(CENTER, 115, buffer, _TLIGHTMAGENTA);
			break;
		case 6:
			_vm.screen().drawRectangle(true, 82, 92, 237, 138, _TBLACK);
			// HACK: use of TROMAN size 16-9
			_vm.screen().loadFont(2);

			strcpy(buffer, "D i r e c t e d   b y :");
			_vm.screen().writeStr(CENTER, 95, buffer, _TMAGENTA);
			break;
		case 7:
			// HACK: use of TROMAN size 16-9
			_vm.screen().loadFont(2);

			strcpy(buffer, "David P Gray !");
			_vm.screen().writeStr(CENTER, 115, buffer, _TLIGHTMAGENTA);
			break;
		case 8:
			_vm.screen().drawRectangle(true, 82, 92, 237, 138, _TBLACK);
			// HACK: use of TROMAN size 16-9
			_vm.screen().loadFont(2);

			strcpy(buffer, "M u s i c   b y :");
			_vm.screen().writeStr(CENTER, 95, buffer, _TMAGENTA);
			break;
		case 9:
			// HACK: use of TROMAN size 16-9
			_vm.screen().loadFont(2);

			strcpy(buffer, "David P Gray !");
			_vm.screen().writeStr(CENTER, 115, buffer, _TLIGHTMAGENTA);
			break;
		case 10:
			_vm.screen().drawRectangle(true, 82, 92, 237, 138, _TBLACK);
			// HACK: use of TROMAN size 20-14
			_vm.screen().loadFont(2);

			strcpy(buffer, "E n j o y !");
			_vm.screen().writeStr(CENTER, 100, buffer, _TLIGHTMAGENTA);
			break;
		}

		_vm.screen().displayBackground();
		g_system->updateScreen();
		g_system->delayMillis(1000);
	}

	return (++introTicks >= introSize);
}

} // End of namespace Hugo
