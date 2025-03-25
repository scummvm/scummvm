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

#include "bagel/hodjnpodj/metagame/views/credits.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define CREDITSFILE     ".\\CREDITS.TXT"
#define MAX_CREDITS  12

#define CR_NEXT   0x0001
#define CR_ESCAPE 0x0002

struct CreditsEntry {
	const char *_filename;
	uint _duration;
};

static const CreditsEntry CREDITS[MAX_CREDITS] = {
	{ "meta/art/credit1.bmp", 7000 },
	{ "meta/art/credit2.bmp", 15000 },
	{ "meta/art/credit3.bmp", 12000 },
	{ "meta/art/credit4.bmp", 5000 },
	{ "meta/art/credit5.bmp", 15000 },
	{ "meta/art/credit6.bmp", 13000 },
	{ "meta/art/credit7.bmp", 14000 },
	{ "meta/art/credit8.bmp", 15000 },
	{ "meta/art/credit9.bmp", 5000 },
	{ "meta/art/credit10.bmp", 7000 },
	{ "meta/art/credit11.bmp", 7000 },
	{ "meta/art/credit12.bmp", 8000 }
};

Credits::Credits() : View("Credits") {
}

bool Credits::msgOpen(const OpenMessage &msg) {
	_creditsIndex = 0;
	_nextCreditsTime = g_system->getMillis() +
		CREDITS[0]._duration;
	_background.loadBitmap(CREDITS[0]._filename);

	g_engine->startBackgroundMidi();
	return true;
}

bool Credits::msgClose(const CloseMessage &msg) {
	g_engine->stopBackgroundMidi();
	return true;
}

bool Credits::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE ||
		msg._action == KEYBIND_SELECT) {
		replaceView("TitleMenu", true);
		return true;
	}

	return false;
}

void Credits::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);
}

bool Credits::tick() {
	uint32 currTime = g_system->getMillis();

	if (currTime >= _nextCreditsTime) {
		if (++_creditsIndex < MAX_CREDITS) {
			_nextCreditsTime = currTime +
				CREDITS[_creditsIndex]._duration;
			_background.loadBitmap(CREDITS[_creditsIndex]._filename);
			redraw();

		} else {
			// Credits are done
			replaceView("TitleMenu", true);
		}
	}

	return true;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
