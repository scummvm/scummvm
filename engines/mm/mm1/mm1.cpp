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
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/file.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/console.h"
#include "mm/mm1/gfx/gfx.h"
#include "mm/xeen/files.h"

namespace MM {
namespace MM1 {

MM1Engine *g_engine = nullptr;

MM1Engine::MM1Engine(OSystem *syst, const MightAndMagicGameDescription *gameDesc)
		: Engine(syst), Events(gameDesc->features & GF_ENHANCED),
		_gameDescription(gameDesc), _randomSource("MM1") {
	g_engine = this;
}

MM1Engine::~MM1Engine() {
	g_engine = nullptr;
}

Common::Error MM1Engine::run() {
	// Initialize graphics mode
	initGraphics(320, 200);

	if (isEnhanced()) {
		if (!setupEnhanced())
			return Common::kNoError;
	} else {
		setupNormal();
	}

	// Setup console
	setDebugger(new Console());

	// Load globals
	if (!_globals.load())
		return Common::kNoError;

	runGame();
	return Common::kNoError;
}

bool MM1Engine::isEnhanced() const {
	return (_gameDescription->features & GF_ENHANCED) != 0;
}

void MM1Engine::setupNormal() {
	Gfx::GFX::setEgaPalette(0);
}

bool MM1Engine::setupEnhanced() {
	if (!Common::File::exists("dark.cc")) {
		GUIErrorMessage(
			"In order to run in Enhanced mode,  please copy dark.cc "
			"from a copy of World of Xeen\n"
			"or Dark Side of Xeen to your Might and Magic 1 game folder"
		);

		return false;
	}

	// Add the Dark Side dark.cc archive
	::MM::Xeen::CCArchive *darkCC = new ::MM::Xeen::CCArchive(
		"dark.cc", "dark", true);
	SearchMan.add("dark", darkCC);

	// Load the palette
	Common::File f;
	if (!f.open("dark.pal"))
		error("Could not load palette");

	byte pal[PALETTE_SIZE];
	for (int i = 0; i < PALETTE_SIZE; ++i)
		pal[i] = f.readByte() << 2;
	g_system->getPaletteManager()->setPalette(pal, 0, PALETTE_COUNT);

	return true;
}

} // End of namespace Xeen
} // End of namespace MM
