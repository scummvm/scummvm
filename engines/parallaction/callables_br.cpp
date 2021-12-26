/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "parallaction/parallaction.h"

#include "common/textconsole.h"

namespace Parallaction {

void Parallaction_br::_c_null(void *) {
	// do nothing :)
}

void Parallaction_br::_c_blufade(void *) {
	warning("Parallaction_br::_c_blufade() not yet implemented");
}

void Parallaction_br::_c_resetpalette(void *) {
	warning("Parallaction_br::_c_resetpalette() not yet implemented");
}

/*
	Used in part 3 by location ULTIMAVAG to
	simulate the movement of trees and railroad.
*/
void Parallaction_br::_c_ferrcycle(void *) {
	Palette pal = _gfx->_palette;

	if (_ferrcycleMode == 1) {
		for (int i = 0; i < 16; ++i) {
			pal.setEntry(4 * i + 192, 0, 0, 0);
			pal.setEntry(4 * i + 193, 30, 12, 12);
			pal.setEntry(4 * i + 194, 28, 47, 54);
			pal.setEntry(4 * i + 195, 63, 63, 63);
		}
	} else if (_ferrcycleMode == 2) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				pal.setEntry(j + 16 * i + 192, 0, 0, 0);
				pal.setEntry(j + 16 * i + 196, 30, 12, 12);
				pal.setEntry(j + 16 * i + 200, 28, 47, 54);
				pal.setEntry(j + 16 * i + 204, 63, 63, 63);
			}
		}
	} else if (_ferrcycleMode == 0) {
		for (int i = 0; i < 16; ++i) {
			pal.setEntry(i + 192, 0, 0, 0);
			pal.setEntry(i + 208, 30, 12, 12);
			pal.setEntry(i + 224, 28, 47, 54);
			pal.setEntry(i + 240, 63, 63, 63);
		}
	}

	_gfx->setPalette(pal);

	_ferrcycleMode = (_ferrcycleMode + 1) % 3;
}

void Parallaction_br::_c_lipsinc(void *) {
	warning("Unexpected lipsinc routine call! Please notify the team");
}

void Parallaction_br::_c_albcycle(void *) {
	warning("Parallaction_br::_c_albcycle() not yet implemented");
}

void Parallaction_br::_c_password(void *) {
	warning("Parallaction_br::_c_password() not yet implemented");
}




} // namespace Parallaction
