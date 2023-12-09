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

#include <vector>
#include <vector>
#include <filesystem>
#include <fstream>
#include "twp/twp.h"
#include "twp/detection.h"
#include "twp/console.h"
#include "twp/vm.h"
#include "twp/ggpack.h"
#include "twp/gfx.h"
#include "twp/lighting.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "common/file.h"
#include "image/png.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/opengl/system_headers.h"

namespace Twp {

TwpEngine *g_engine;

static Math::Vector2d getScreenSize(const Room &room) {
	switch (room.height) {
	case 128:
		return {320, 180};
	case 172:
		return {428, 240};
	case 256:
		return {640, 360};
	default:
		return {room.roomSize.getX(), (float)room.height};
	}
}

static bool cmpLayer(const Layer &l1, const Layer &l2) {
	return l1.zsort > l2.zsort;
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
	const int screenWidth = 1280;
	const int screenHeight = 720;
	initGraphics3d(screenWidth, screenHeight);
	_screen = new Graphics::Screen(screenWidth, screenHeight);

	XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x56, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0xAD};
	// XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x56, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0x6D};
	// XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x5B, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0x6D};
	// XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x5B, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0xAD};

	Common::File f;
	f.open("ThimbleweedPark.ggpack1");

	pack.open(&f, key);

	const SQChar *code = R"(
		function
		bounceImage() {
		local image = createObject("RaySheet", ["fstand_head1"]);
		local x = random(100, 1180);
		local y = random(100, 620);

		do {
			local steps = random(100, 150);
			local end_x = random(0, 1180);
			local end_y = random(0, 620);

			local dx = (end_x - x) / steps;
			local dy = (end_y - y) / steps;

			for (local i = 0; i < steps; i++) {
				x += dx;
				y += dy;
				objectAt(image, x, y);
				breaktime(0.01);
			}
		} while (1);
	}

	for (local i = 1; i <= 200; i++) {
		startthread(bounceImage);
	})";

	GGPackEntryReader r;
	r.open(g_engine->pack, "MainStreet.wimpy");

	Room room;
	room.load(r);

	Vm v;
	v.exec(code);

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	Math::Vector2d pos;
	Gfx gfx;
	gfx.init();

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
		for (int i = 0; i < threads.size(); i++) {
			Thread *thread = threads[i];
			if (thread->update(deltaTimeMs)) {
				// TODO: delete it
			}
		}

		// update screen
		Math::Vector2d screenSize = getScreenSize(room);
		gfx.camera(screenSize);
		gfx.clear(Color(0, 0, 0));
		gfx.use(&lighting);

		// draw room
		SpriteSheet *ss = resManager.spriteSheet(room.sheet);
		Texture *texture = resManager.texture(ss->meta.image);
		Common::sort(room.layers.begin(), room.layers.end(), cmpLayer);
		for (int i = 0; i < room.layers.size(); i++) {
			float x = 0;
			const Layer &layer = room.layers[i];
			for (int j = 0; j < layer.names.size(); j++) {
				const Common::String &name = layer.names[j];
				const SpriteSheetFrame &frame = ss->frameTable[name];
				Math::Matrix4 m;
				Math::Vector3d t1 = Math::Vector3d(x - pos.getX() * layer.parallax.getX(), -pos.getY() * layer.parallax.getY(), 0);
				Math::Vector3d t2 = Math::Vector3d(frame.spriteSourceSize.left, frame.sourceSize.getY() - frame.spriteSourceSize.height() - frame.spriteSourceSize.top, 0.0f);
				m.translate(t1+t2);
				lighting.setSpriteSheetFrame(frame, *texture);
				gfx.drawSprite(frame.frame, *texture, Color(), m);
				x += frame.frame.width();
			}
		}

		// draw entities
		gfx.use(NULL);
		for (int i = 0; i < entities.size(); i++) {
			Entity &ett = entities[i];
			Math::Matrix4 m;
			m.translate(Math::Vector3d(ett.x - pos.getX(), ett.y - pos.getY(), 0));
			gfx.drawSprite(ett.rect, *ett.texture, Color(), m);
		}
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

} // End of namespace Twp
