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
#include "bagel/hodjnpodj/metagame/views/minigames.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define	BACKGROUND_BMP		"meta/art/mlscroll.bmp"
#define FONT_SIZE 12

Minigames::Minigames() : View("Minigames") {
}

bool Minigames::msgOpen(const OpenMessage &msg) {
	g_events->showCursor(true);

	Common::File f;
	Image::BitmapDecoder decoder;
	if (!f.open(BACKGROUND_BMP) || !decoder.loadStream(f))
		error("Error loading - %s", BACKGROUND_BMP);

	const Graphics::Surface *surf = decoder.getSurface();
	_background.copyFrom(surf);
	_background.setTransparentColor(255);

	blackScreen();

	Common::Rect r(0, 0, surf->w, surf->h);
	r.moveTo((GAME_WIDTH - surf->w) / 2, (GAME_HEIGHT - surf->h) / 2);
	setBounds(r);

	return View::msgOpen(msg);
}

bool Minigames::msgClose(const CloseMessage &msg) {
	return View::msgClose(msg);
}

bool Minigames::msgGame(const GameMessage &msg) {
	
	return false;
}

void Minigames::draw() {
	GfxSurface s = getSurface();
	s.setFontSize(FONT_SIZE);

	s.clear();
	s.blitFrom(_background);
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
