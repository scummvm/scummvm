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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

#include "olga/console.h"
#include "olga/olga.h"

namespace Olga {

OlgaEngine *g_engine;

OlgaEngine::OlgaEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Olga") {
	g_engine = this;
}

OlgaEngine::~OlgaEngine() {
	delete _screen;
}

uint32 OlgaEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String OlgaEngine::getGameId() const {
	return _gameDescription->gameId;
}

void OlgaEngine::waitOrKey(uint32 ms) {
	uint32 end = g_system->getMillis() + ms;
	while (!shouldQuit() && g_system->getMillis() < end) {
		Common::Event e;
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN || e.type == Common::EVENT_LBUTTONDOWN)
				return;
		}
		_screen->update();
		g_system->delayMillis(10);
	}
}

bool OlgaEngine::showPicture(const Common::Path &name) {
	LbmFile lbm;
	if (!lbm.load(name) || !lbm.hasPalette())
		return false;
	g_system->getPaletteManager()->setPalette(lbm.palette(), 0, 256);
	_screen->clear(0);
	_screen->blitFrom(lbm.image(1).surface, Common::Point(0, 0));
	_screen->update();
	return true;
}

void OlgaEngine::drawPanel() {
	// Responses_1e0d (5427:1e0d): message and sentence lines black, a white
	// line at y 156, the panel grey (7) with the divider at x 172 / 173
	_screen->fillRect(Common::Rect(0, 0, 320, kRoomTop), 0);
	_screen->fillRect(Common::Rect(0, kSentenceTop, 320, kPanelTop), 0);
	_screen->hLine(0, kPanelTop, 319, 15);
	_screen->fillRect(Common::Rect(0, kPanelTop + 1, 320, 200), 7);
	_screen->vLine(172, kPanelTop + 1, 199, 8);
	_screen->vLine(173, kPanelTop + 1, 199, 15);
}

bool OlgaEngine::showRoom(const Common::String &name) {
	LbmFile lbm;
	if (!lbm.load(Common::Path(name + ".LBM")) || !lbm.hasPalette())
		return false;
	_room = lbm;
	_roomName = name;
	// Scene_17a4: the room's whole CMAP, then image 1 (the background)
	g_system->getPaletteManager()->setPalette(_room.palette(), 0, 256);
	_screen->clear(0);
	_screen->blitFrom(_room.image(1).surface, Common::Point(0, kRoomTop));
	drawPanel();
	_screen->update();
	return true;
}

Common::Error OlgaEngine::run() {
	initGraphics(320, 200);
	_screen = new Graphics::Screen();
	setDebugger(new Console());

	// Main_Body: two title pictures of 6 s each (1000:198b, 1000:1994)
	if (!showPicture("OLGA.LBM"))
		return Common::kReadingFailed;
	waitOrKey(6000);
	if (!shouldQuit() && showPicture("REKLAME.LBM"))
		waitOrKey(6000);

	// Main_NewGame: location 9 = GANG5 (notes/rooms.md)
	if (!shouldQuit() && !showRoom("GANG5"))
		return Common::kReadingFailed;

	while (!shouldQuit()) {
		uint32 start = g_system->getMillis();
		Common::Event e;
		while (g_system->getEventManager()->pollEvent(e)) {
		}
		_screen->update();
		uint32 used = g_system->getMillis() - start;
		if (used < kFrameMs)
			g_system->delayMillis(kFrameMs - used);
	}

	return Common::kNoError;
}

} // End of namespace Olga
