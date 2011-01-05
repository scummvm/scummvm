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
intro_v1d::intro_v1d(HugoEngine *vm) : IntroHandler(vm) {
}

intro_v1d::~intro_v1d() {
}

void intro_v1d::preNewGame() {
}

void intro_v1d::introInit() {
	introTicks = 0;
	surf.w = 320;
	surf.h = 200;
	surf.pixels = _vm->_screen->getFrontBuffer();
	surf.pitch = 320;
	surf.bytesPerPixel = 1;
}

bool intro_v1d::introPlay() {
	static int state = 0;
	byte introSize = _vm->getIntroSize();

	if (introTicks < introSize) {
		switch (state++) {
		case 0:
			_vm->_screen->drawRectangle(true, 0, 0, 319, 199, _TMAGENTA);
			_vm->_screen->drawRectangle(true, 10, 10, 309, 189, _TBLACK);
			break;

		case 1:
			_vm->_screen->drawShape(20, 92,_TLIGHTMAGENTA,_TMAGENTA);
			_vm->_screen->drawShape(250,92,_TLIGHTMAGENTA,_TMAGENTA);

			// TROMAN, size 10-5
			if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 8)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 8");

			char buffer[80];
			if (_boot.registered)
				strcpy(buffer, "Registered Version");
			else
				strcpy(buffer, "Shareware Version");

			font.drawString(&surf, buffer, 0, 163, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			font.drawString(&surf, COPYRIGHT, 0, 176, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);

			if (scumm_stricmp(_boot.distrib, "David P. Gray")) {
				sprintf(buffer, "Distributed by %s.", _boot.distrib);
				font.drawString(&surf, buffer, 0, 75, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			}

			// SCRIPT, size 24-16
			strcpy(buffer, "Hugo's");

			if (font.loadFromFON("SCRIPT.FON", Graphics::WinFontDirEntry("Script", 16))) {
				font.drawString(&surf, buffer, 0, 20, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			} else {
				// Workaround: SCRIPT.FON doesn't load properly at the moment
				_vm->_screen->loadFont(2);
				_vm->_screen->writeStr(CENTER, 20, buffer, _TMAGENTA);
			}

			// TROMAN, size 30-24
			if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 24)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 24");

			strcpy(buffer, "House of Horrors !");
			font.drawString(&surf, buffer, 0, 50, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 2:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 10)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 10");

			strcpy(buffer, "S t a r r i n g :");
			font.drawString(&surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 3:
			// TROMAN, size 20-9
			strcpy(buffer, "Hugo !");
			font.drawString(&surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 4:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			strcpy(buffer, "P r o d u c e d  b y :");
			font.drawString(&surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 5:
			// TROMAN size 16-9
			strcpy(buffer, "David P Gray !");
			font.drawString(&surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 6:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			strcpy(buffer, "D i r e c t e d   b y :");
			font.drawString(&surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 7:
			// TROMAN, size 16-9
			strcpy(buffer, "David P Gray !");
			font.drawString(&surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 8:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			strcpy(buffer, "M u s i c   b y :");
			font.drawString(&surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 9:
			// TROMAN, size 16-9
			strcpy(buffer, "David P Gray !");
			font.drawString(&surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 10:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 20-14
			if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 14)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 14");

			strcpy(buffer, "E n j o y !");
			font.drawString(&surf, buffer, 0, 100, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		}

		_vm->_screen->displayBackground();
		g_system->updateScreen();
		g_system->delayMillis(1000);
	}

	return (++introTicks >= introSize);
}

} // End of namespace Hugo
