/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/file.h"
#include "common/system.h"
#include "common/events.h"
#include "common/stream.h"

#include "engines/util.h"

#include "graphics/palette.h"

#include "adl/adl.h"
#include "adl/display.h"
#include "adl/parser.h"

namespace Adl {

AdlEngine::AdlEngine(OSystem *syst, const AdlGameDescription *gd) :
		Engine(syst),
		_gameDescription(gd),
		_console(nullptr),
		_display(nullptr) {
}

AdlEngine::~AdlEngine() {
	delete _console;
	delete _display;
}

Common::Error AdlEngine::run() {
	initGraphics(560, 384, true);

	byte palette[6 * 3] = {
		0x00, 0x00, 0x00,
		0xff, 0xff, 0xff,
		0xc7, 0x34, 0xff,
		0x38, 0xcb, 0x00,
		0x00, 0x00, 0xff, // FIXME
		0xff, 0xa5, 0x00  // FIXME
	};

	g_system->getPaletteManager()->setPalette(palette, 0, 6);

	_console = new Console(this);
	_display = new Display();
	_parser = new Parser(*this, *_display);

	runGame();

	return Common::kNoError;
}

Common::String AdlEngine::readString(Common::ReadStream &stream, byte until) {
	Common::String str;

	while (1) {
		byte b = stream.readByte();

		if (stream.eos() || stream.err() || b == until)
			break;

		str += b;
	};

	return str;
}

void AdlEngine::printStrings(Common::SeekableReadStream &stream, int count) {
	while (1) {
		Common::String str = readString(stream);
		_display->printString(str);

		if (--count == 0)
			break;

		stream.seek(3, SEEK_CUR);
	};
}

AdlEngine *AdlEngine::create(GameType type, OSystem *syst, const AdlGameDescription *gd) {
	switch(type) {
	case kGameTypeHires1:
		return AdlEngine_v1__create(syst, gd);
	default:
		error("Unknown GameType");
	}
}

} // End of namespace Adl
