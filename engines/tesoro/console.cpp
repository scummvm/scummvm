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

#include "tesoro/console.h"
#include "tesoro/core/animbank.h"

#include "graphics/managed_surface.h"

namespace Tesoro {

Console::Console() : GUI::Debugger() {
	registerCmd("entries",   WRAP_METHOD(Console, entries));
	registerCmd("entry",   WRAP_METHOD(Console, entry));
	registerCmd("animbank", WRAP_METHOD(Console, animbank));
	registerCmd("showimage", WRAP_METHOD(Console, showimage));
}

Console::~Console() {
}

bool Console::entries(int argc, const char **argv) {
	debugPrintf("Entries: %d\n", g_engine->_archive.index().count());
	return true;
}

bool Console::entry(int argc, const char **argv) {
	debugPrintf("Entry %s\n", argv[0]);
	g_engine->_archive.findEntry(argv[0]);
	return true;
}

bool Console::animbank(int argc, const char **argv) {
	debugPrintf("AnimBank\n");


	Common::String name(argv[1]);
	name.toUppercase();	// the archive's names are upper case
	const uint32 offset = g_engine->_archive.findEntry(name.c_str());
	if (!offset) {
		debugPrintf("No entry called %s\n", name.c_str());
		return true;
	}

	// The stream belongs to the archive: do not delete it.
	Common::SeekableReadStream *dat = g_engine->_archive.seekEntry(offset);

	AnimBank bank;
	if (!bank.load(*dat, offset, AnimBank::kLoadNow, false)) {
		debugPrintf("Could not load %s\n", name.c_str());
		return true;
	}

	debugPrintf("%s: %u sequences, %u images\n", name.c_str(), bank.sequenceCount(), bank.imageCount());
	for (uint s = 0; s < bank.sequenceCount() && s < 10; ++s) {
		const AnimSequence &q = bank.sequence(s);
		debugPrintf("  seq %u: image %d at %d,%d, %u ms, type %u\n", s, q.imageIndex, q.imageX, q.imageY, q.duration, q.type);
	}
	for (uint i = 0; i < bank.imageCount(); ++i) {
		const AnimImage &img = bank.image(i);
		if (!img.isLoaded()) {
			debugPrintf("  image %u: not loaded\n", i);
			continue;
		}
		const byte *d = img.data();
		debugPrintf("  image %u: type %u, %u x %u, %u bytes of pixels\n", i, img.type(), READ_LE_UINT16(d + 8), READ_LE_UINT16(d + 10), img.pixelBytes());
	}
	return true;
}

// Throwaway: draws one image unscaled with its top left corner at (x, y).
static void drawImage(Graphics::ManagedSurface &dst, int x, int y, const AnimImage &image) {
	const byte *blob = image.data();
	if (!blob)
		return;
	const uint height = READ_LE_UINT16(blob + 10);
	const uint type = READ_LE_UINT16(blob + 12);
	if (type == 2)	// shadow: intensities, not colours
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
			px += READ_LE_UINT16(item + 4);		// empty pixels before the run
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

bool Console::showimage(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: %s NAME.ANM IMAGE_NUMBER\n", argv[0]);
		return true;
	}
	Common::String name(argv[1]);
	name.toUppercase();
	const uint32 offset = g_engine->_archive.findEntry(name.c_str());
	if (!offset) {
		debugPrintf("No entry called %s\n", name.c_str());
		return true;
	}
	AnimBank bank;
	if (!bank.load(*g_engine->_archive.seekEntry(offset), offset, AnimBank::kLoadNow, false)) {
		debugPrintf("Could not load %s\n", name.c_str());
		return true;
	}
	const uint index = atoi(argv[2]);
	if (index >= bank.imageCount()) {
		debugPrintf("%s has %u images\n", name.c_str(), bank.imageCount());
		return true;
	}

	g_engine->_screen->fillRect(Common::Rect(0, 0, g_engine->_screen->w, g_engine->_screen->h), 0x4208);	// dark grey, so the shape shows
	drawImage(*g_engine->_screen, 20, 20, bank.image(index));
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
	return false;	// false closes the console, so the picture is visible
}

} // End of namespace Tesoro
