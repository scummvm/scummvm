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

#include "darkseed/darkseed.h"
#include "anm.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "darkseed/console.h"
#include "darkseed/detection.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "img.h"
#include "nsp.h"
#include "pal.h"
#include "pic.h"
#include "room.h"
#include "titlefont.h"

namespace Darkseed {

DarkseedEngine *g_engine;

DarkseedEngine::DarkseedEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Darkseed") {
	g_engine = this;
}

DarkseedEngine::~DarkseedEngine() {
	delete _screen;
}

uint32 DarkseedEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String DarkseedEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error DarkseedEngine::run() {
	initGraphics(640, 350);
	_screen = new Graphics::Screen();
	_tosText = new TosText();
	_tosText->load();
	Img left00Img;
	left00Img.load("art/left00.img");
	Img left01Img;
	left01Img.load("art/left01.img");

	Anm lettersAnm;
	lettersAnm.load("art/letters.anm");
	Img letterD;
	lettersAnm.getImg(6, letterD);
	Img letterD1;
	lettersAnm.getImg(7, letterD1);

	// Set the engine's debugger console
	setDebugger(new Console(_tosText));

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Draw a series of boxes on screen as a sample
//	for (int i = 0; i < 100; ++i)
//		_screen->frameRect(Common::Rect(i, i, 320 - i, 200 - i), i);
//	Pal::load("art/ship.pal");
//	_screen->copyRectToSurface(left00Img.getPixels().data(), left00Img.getWidth(), left00Img.getX(), left00Img.getY(), left00Img.getWidth(), left00Img.getHeight());
//	_screen->copyRectToSurface(left01Img.getPixels().data(), left01Img.getWidth(), left01Img.getX(), left01Img.getY(), left01Img.getWidth(), left01Img.getHeight());

	Pal housePalette;
	housePalette.load("art/house.pal");
	TitleFont titleFont;
	titleFont.displayString(0x44, 0xa0, "DEVELOPING NEW WAYS TO AMAZE");
	Img house;
//	house.load("art/bdoll0.img");
//		_screen->copyRectToSurface(house.getPixels().data(), house.getWidth(), house.getX(), house.getY(), house.getWidth(), house.getHeight());
//	_screen->copyRectToSurfaceWithKey(letterD.getPixels().data(), letterD.getWidth(), 24, 24, letterD.getWidth(), letterD.getHeight(), 0);
//	_screen->copyRectToSurfaceWithKey(letterD1.getPixels().data(), letterD1.getWidth(), 24+1, 24, letterD1.getWidth(), letterD1.getHeight(), 0);

	_frame.load("cframe.pic");
	_frame.draw();

	_baseSprites.load("cbase.nsp");

	_cursor.updatePosition(0x140,0xaf);
	_cursor.setCursorType(Pointer);

//	Pic room;
//	room.load("bed1a.pic");
//	_screen->copyRectToSurface(room.getPixels().data(), room.getWidth(), 0x45, 0x28, room.getWidth(), room.getHeight());

//	Pal roomPal;
//	roomPal.load("room0.pal");

	Nsp playerNsp;
	playerNsp.load("cbase.nsp"); //"cplayer.nsp");
//	const Sprite &s = playerNsp.getSpriteAt(11);
//
//	_screen->copyRectToSurfaceWithKey(s.pixels.data(), s.width, 0x45 + 220, 0x28 + 40, s.width, s.height, 0xf);

	_room = new Room(2);
//	_room->draw();
//	_cursor.draw();
//	_screen->update();
	// Simple event handling loop
//	Common::Event e;
	int offset = 0;
	int sIdx = 0;

	gameloop();

//	while (!shouldQuit()) {
//		while (g_system->getEventManager()->pollEvent(e)) {
//			if(e.type == Common::EVENT_KEYDOWN) {
//				sIdx++;
//				if (sIdx >= 96) {
//					sIdx = 0;
//				}
//				const Sprite &s = playerNsp.getSpriteAt(sIdx);
//				_room->draw();
//				_cursor.draw();
////				_screen->copyRectToSurface(room.getPixels().data(), room.getWidth(), 0x45, 0x28, room.getWidth(), room.getHeight());
//				_screen->copyRectToSurfaceWithKey(s.pixels.data(), s.width, 0x45 + 220, 0x28 + 40, s.width, s.height, 0xf);
//				_screen->makeAllDirty();
//			}
//		}
////		_screen->copyRectToSurface(room.getPixels().data(), room.getWidth(), 0x45, 0x28, room.getWidth(), room.getHeight());
//		// Cycle through a simple palette
////		++offset;
////		for (int i = 0; i < 256; ++i)
////			pal[i * 3 + 1] = (i + offset) % 256;
////		g_system->getPaletteManager()->setPalette(pal, 0, 256);
//		_screen->update();
//
//		// Delay for a bit. All events loops should have a delay
//		// to prevent the system being unduly loaded
//		g_system->delayMillis(10);
//	}

	delete _room;

	return Common::kNoError;
}

Common::Error DarkseedEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}
void DarkseedEngine::fadeOut() {
}

void DarkseedEngine::fadeIn() {

}

void DarkseedEngine::fadeInner(int startValue, int endValue, int increment) {

}

void DarkseedEngine::gameloop() {
	while (!shouldQuit()) {
		updateEvents();
		handleInput();
		_room->update();
		_frame.draw();
		_room->draw();
		_cursor.draw();
		_screen->makeAllDirty();
		_screen->update();
		wait();
	}
}

void DarkseedEngine::updateEvents() {
	Common::Event event;
	_isRightMouseClicked = false;
	_isLeftMouseClicked = false;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE: _cursor.updatePosition(event.mouse.x, event.mouse.y); break;
		case Common::EVENT_RBUTTONDOWN: _isRightMouseClicked = true; break;
		case Common::EVENT_RBUTTONUP: _isRightMouseClicked = false; break;
		case Common::EVENT_LBUTTONDOWN: _isLeftMouseClicked = true; break;
		case Common::EVENT_LBUTTONUP: _isLeftMouseClicked = false; break;
		default: break;
		}
	}
}

void DarkseedEngine::wait() {
	g_system->delayMillis(10);
}

void DarkseedEngine::handleInput() {
	if (_isRightMouseClicked) {
		if (_actionMode == LookAction) {
			_actionMode = PointerAction;
		} else if (_actionMode == PointerAction) {
			_actionMode = HandAction;
		} else if (_actionMode == HandAction) {
			_actionMode = LookAction;
		}
		_cursor.setCursorType((CursorType)_actionMode);
	} else if (_isLeftMouseClicked) {
		// TODO do actions here.
		handlePointerAction();
	}
}

void DarkseedEngine::handlePointerAction() {
	if (_cursor.getCursorType() == ConnectorEntrance) {
		int newRoomNumber = _room->getExitRoomNumberAtPoint(_cursor.getX(), _cursor.getY());
		if (newRoomNumber >= 0) {
			changeToRoom(newRoomNumber);
		}
	}
}
void DarkseedEngine::changeToRoom(int newRoomNumber) {
	delete _room;
	_room = new Room(newRoomNumber);
}

} // End of namespace Darkseed
