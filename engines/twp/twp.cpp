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

#include "common/scummsys.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/file.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/opengl/system_headers.h"
#include "twp/twp.h"
#include "twp/detection.h"
#include "twp/console.h"
#include "twp/vm.h"
#include "twp/ggpack.h"
#include "twp/gfx.h"
#include "twp/lighting.h"
#include "twp/font.h"
#include "twp/thread.h"

namespace Twp {

#define SCREEN_WIDTH	1280
#define SCREEN_HEIGHT	720

TwpEngine *g_engine;

static bool cmpLayer(const Layer *l1, const Layer *l2) {
	return l1->_zsort > l2->_zsort;
}

TwpEngine::TwpEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst),
	  _gameDescription(gameDesc),
	  _randomSource("Twp") {
	g_engine = this;
}

TwpEngine::~TwpEngine() {
	delete _screen;
}

uint32 TwpEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String TwpEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error TwpEngine::run() {
	initGraphics3d(SCREEN_WIDTH, SCREEN_HEIGHT);
	_screen = new Graphics::Screen(SCREEN_WIDTH, SCREEN_HEIGHT);

	XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x56, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0xAD};
	// XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x56, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0x6D};
	// XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x5B, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0x6D};
	// XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x5B, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0xAD};

	Common::File f;
	f.open("ThimbleweedPark.ggpack1");

	_pack.open(&f, key);

	GGPackEntryReader r;
	r.open(g_engine->_pack, "MainStreet.wimpy");

	Room room;
	room.load(r);

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	Math::Vector2d pos;
	_gfx.init();

	Lighting lighting;
	// Simple event handling loop
	Common::Event e;
	while (!shouldQuit()) {
		const uint deltaTimeMs = 10;
		const int dx = 4;
		const int dy = 4;
		while (g_system->getEventManager()->pollEvent(e)) {
			switch (e.type) {
			case Common::EVENT_KEYDOWN:
				switch (e.kbd.keycode) {
				case Common::KEYCODE_LEFT:
					pos.setX(pos.getX() - dx);
					break;
				case Common::KEYCODE_RIGHT:
					pos.setX(pos.getX() + dx);
					break;
				case Common::KEYCODE_UP:
					pos.setY(pos.getY() + dy);
					break;
				case Common::KEYCODE_DOWN:
					pos.setY(pos.getY() - dy);
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		// update threads
		for (int i = 0; i < _threads.size(); i++) {
			Thread *thread = _threads[i];
			if (thread->update(deltaTimeMs)) {
				// TODO: delete it
			}
		}

		// update screen
		Math::Vector2d screenSize = room.getScreenSize();
		_gfx.camera(screenSize);
		_gfx.clear(Color(0, 0, 0));
		_gfx.use(&lighting);

		// draw room
		SpriteSheet *ss = _resManager.spriteSheet(room._sheet);
		Texture *texture = _resManager.texture(ss->meta.image);
		Common::sort(room._layers.begin(), room._layers.end(), cmpLayer);
		for (int i = 0; i < room._layers.size(); i++) {
			float x = 0;
			const Layer* layer = room._layers[i];
			for (int j = 0; j < layer->_names.size(); j++) {
				const Common::String &name = layer->_names[j];
				const SpriteSheetFrame &frame = ss->frameTable[name];
				Math::Matrix4 m;
				Math::Vector3d t1 = Math::Vector3d(x - pos.getX() * layer->_parallax.getX(), -pos.getY() * layer->_parallax.getY(), 0);
				Math::Vector3d t2 = Math::Vector3d(frame.spriteSourceSize.left, frame.sourceSize.getY() - frame.spriteSourceSize.height() - frame.spriteSourceSize.top, 0.0f);
				m.translate(t1+t2);
				lighting.setSpriteSheetFrame(frame, *texture);
				_gfx.drawSprite(frame.frame, *texture, Color(), m);
				x += frame.frame.width();
			}
		}

		// TODO: entities
		_gfx.use(NULL);
		// for (int i = 0; i < objects.size(); i++) {
		// 	Object &obj = *objects[i];
		// 	Math::Matrix4 m;
		// 	m.translate(Math::Vector3d(obj.x - pos.getX(), obj.y - pos.getY(), 0));
		// 	_gfx.drawSprite(obj.rect, *obj.texture, Color(), m);
		// }
		g_system->updateScreen();

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		g_system->delayMillis(deltaTimeMs);
	}

	return Common::kNoError;
}

Common::Error TwpEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

Math::Vector2d TwpEngine::roomToScreen(Math::Vector2d pos) {
	Math::Vector2d screenSize = _room->getScreenSize();
	return Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT) * (pos - _gfx.cameraPos()) / screenSize;
}

Math::Vector2d TwpEngine::screenToRoom(Math::Vector2d pos) {
  Math::Vector2d screenSize = _room->getScreenSize();
  return (pos * screenSize) / Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT) + _gfx.cameraPos();
}

} // End of namespace Twp
