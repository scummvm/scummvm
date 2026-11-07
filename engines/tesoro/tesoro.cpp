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

#include "tesoro/tesoro.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "tesoro/console.h"
#include "tesoro/detection.h"

#include "graphics/managed_surface.h"

namespace Tesoro {

TesoroEngine *g_engine;

TesoroEngine::TesoroEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																			   _gameDescription(gameDesc), _clock(syst->getMillis()), _randomSource("Tesoro") {
	g_engine = this;
}

TesoroEngine::~TesoroEngine() {
	delete _screen;
}

uint32 TesoroEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String TesoroEngine::getGameId() const {
	return _gameDescription->gameId;
}

// Throwaway: draws one image unscaled with its top left corner at (x, y).
static void drawImage(Graphics::ManagedSurface &dst, int x, int y, const AnimImage &image) {
	const byte *blob = image.data();
	if (!blob)
		return;
	const uint height = READ_LE_UINT16(blob + 10);
	const uint type = READ_LE_UINT16(blob + 12);
	if (type == 2) // shadow: intensities, not colours
		return;

	for (uint row = 0; row < height; ++row) {
		const int py = y + (int)row;
		if (py < 0 || py >= dst.h)
			continue;
		const byte *entry = blob + 0x10 + 8 * row;
		const byte *item = blob + READ_LE_UINT32(entry);
		uint items = READ_LE_UINT16(entry + 4);
		int px = x;
		while (items--) {
			px += READ_LE_UINT16(item + 4); // empty pixels before the run
			const uint length = READ_LE_UINT16(item + 6);
			const uint32 first = READ_LE_UINT32(item);
			for (uint k = 0; k < length; ++k) {
				const int cx = px + (int)k;
				if (cx < 0 || cx >= dst.w)
					continue;
				const uint16 colour = type == 1 ? READ_LE_UINT16(blob + first + 2 * k) : (uint16)first;
				*(uint16 *)dst.getBasePtr(cx, py) = colour;
			}
			px += length;
			item += 8;
		}
	}
}

Common::Error TesoroEngine::run() {
	// Initialize 320x200 paletted graphics mode
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(640, 480, &format);
	_screen = new Graphics::Screen(640, 480, format);

	if (!_archive.open("TESORO.DAT", "TESORO.TRE")) {
		GUIErrorMessage("Could not open TESORO.DAT and TESORO.TRE");
		return Common::kReadingFailed;
	}

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Simple event handling loop
	Common::Event e;

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}

		if (_animPlayer.bank()) {
			_animPlayer.advance(gameTime());
			const AnimBank &bank = *_animPlayer.bank();
			// get the current step
			const AnimStep &step = bank.step(_animPlayer.current());

			_screen->fillRect(Common::Rect(0, 0, 640, 480), 0x4208); // clear to grey
			if (step.hasImage())
				drawImage(*_screen, 300 + step.imageX, 300 - step.imageY, bank.image(step.imageIndex));
			_screen->markAllDirty(); // writes through getBasePtr don't mark the screen dirty
		}

		limiter.delayBeforeSwap();
		_screen->update();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error TesoroEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

bool TesoroEngine::showImage(const char *name, int index) {

	const uint32 offset = g_engine->_archive.findEntry(name);
	if (!offset) {
		debug("No entry called %s\n", name);
		return false;
	}
	AnimBank bank;
	if (!bank.load(*g_engine->_archive.seekEntry(offset), offset, AnimBank::kLoadNow, false)) {
		debug("Could not load %s\n", name);
		return false;
	}
	if (index >= bank.imageCount()) {
		debug("%s has %u images\n", name, bank.imageCount());
		return false;
	}

	_screen->fillRect(Common::Rect(0, 0, g_engine->_screen->w, g_engine->_screen->h), 0x4208); // dark grey, so the shape shows
	drawImage(*g_engine->_screen, 20, 20, bank.image(index));
	_screen->markAllDirty();
	_screen->update();
	return true;
}

bool TesoroEngine::playAnim(const char *name) {
	Common::String upper(name);
	upper.toUppercase(); // archive names are upper case
	const uint32 offset = _archive.findEntry(upper.c_str());
	if (!offset) {
		debug("No entry called %s", upper.c_str());
		return false;
	}
	if (!_animPlayer.load(*_archive.seekEntry(offset), offset, AnimBank::kLoadNow, false)) {
		debug("Could not load %s", upper.c_str());
		return false;
	}
	// All steps up to the first one without an image, looping, with the cycle length totalled.
	_animPlayer.setRange(0, -1, true, true, gameTime());
	return true;
}

Common::String TesoroEngine::dumpRow(const char *name, uint index, uint imageRow, int x) {
	const uint32 offset = _archive.findEntry(name);
	if (!offset)
		return Common::String::format("No entry called %s\n", name);
	AnimBank bank;
	if (!bank.load(*_archive.seekEntry(offset), offset, AnimBank::kLoadNow, false))
		return Common::String::format("Could not load %s\n", name);
	if (index >= bank.imageCount())
		return Common::String::format("The bank has %u images\n", bank.imageCount());

	const AnimImage &image = bank.image(index);
	const byte *blob = image.data();
	if (!blob)
		return "That image is not loaded\n";
	const int y = 20;
	const uint height = READ_LE_UINT16(blob + 10);
	const uint type = READ_LE_UINT16(blob + 12);
	if (type != 1 && type != 3)
		return Common::String::format("Image %u is type %u (shadow): it is not in the row list\n", index, type);
	if (imageRow >= height || y + (int)imageRow >= RenderQueue::kRows)
		return Common::String::format("The image has %u rows\n", height);

	_queue.reset();
	_queue.addSprite(x, y, 5, image, 0);

	Common::String out = Common::String::format("image %u: %u x %u, type %u, drawn at (%d, %d); image row %u is screen row %d\n",
		index, READ_LE_UINT16(blob + 8), height, type, x, y, imageRow, y + (int)imageRow);

		const RenderQueue::Row &row = _queue.row(y + imageRow);
	const byte *entry = blob + 0x10 + 8 * imageRow;
	const byte *item = blob + READ_LE_UINT32(entry);
	uint items = READ_LE_UINT16(entry + 4);
	int n = row.head();	// the queue's span, walked in step with the image's runs
	bool same = true;
	int px = x;

	out += "each run of the image, then the span the queue holds for it:\n";
	while (items--) {
		const uint gap = READ_LE_UINT16(item + 4), length = READ_LE_UINT16(item + 6);
		const uint32 first = READ_LE_UINT32(item);
		px += gap;
		const int x0 = px, x1 = px + (int)length - 1;
		const long source = type == 1 ? (long)first : (long)(first & 0xffff);
		out += Common::String::format("  run  gap %u, length %u -> columns %d..%d, %ld\n", gap, length, x0, x1, source);
		if (n) {
			const RenderQueue::Span &s = row.span(n);
			const long spanSource = type == 1 ? (long)((const byte *)s.pixels - blob) : (long)s.colour;
			out += Common::String::format("  span columns %d..%d, depth %u, %ld\n", s.x0, s.x1, s.depth, spanSource);
			same = same && s.x0 == x0 && s.x1 == x1 && spanSource == source;
			n = s.next;
		} else {
			out += "  span (none)\n";
			same = false;
		}
		px += length;
		item += 8;
	}
	if (n)
		same = false;	// the queue holds more spans than the image has runs
	out += same ? "MATCH\n" : "DIFFERENT\n";
	return out;
}

} // End of namespace Tesoro
