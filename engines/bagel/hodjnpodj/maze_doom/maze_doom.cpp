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

#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/maze_doom/maze_doom.h"
#include "bagel/hodjnpodj/maze_doom/globals.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/views/main_menu.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

MazeDoom::MazeDoom() : MinigameView("mazedoom", "mazedoom/hnpmaze.dll") {
}

bool MazeDoom::msgOpen(const OpenMessage &msg) {
	MinigameView::msgOpen(msg);

	setupBitmaps();
	loadBackground();


	return true;
}

void MazeDoom::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);
}

void MazeDoom::setupBitmaps() {
	if (pGameParams->bPlayingHodj) {
		_upBmp = IDB_HODJ_UP_BMP;
		_downBmp = IDB_HODJ_DOWN_BMP;
		_leftBmp = IDB_HODJ_LEFT_BMP;
		_rightBmp = IDB_HODJ_RIGHT_BMP;
	} else {
		_upBmp = IDB_PODJ_UP_BMP;
		_downBmp = IDB_PODJ_DOWN_BMP;
		_leftBmp = IDB_PODJ_LEFT_BMP;
		_rightBmp = IDB_PODJ_RIGHT_BMP;
	}
}

void MazeDoom::loadBackground() {
	Image::BitmapDecoder decoder;
	Common::File f;

	if (!f.open(MAIN_SCREEN) || !decoder.loadStream(f))
		error("Could not load - %s", MAIN_SCREEN);
	loadPalette(decoder.getPalette());

	_background.copyFrom(*decoder.getSurface());
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
