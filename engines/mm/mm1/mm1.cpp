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
	if (!_globals.load(isEnhanced()))
		return Common::kNoError;

	runGame();
	return Common::kNoError;
}

bool MM1Engine::isEnhanced() const {
	return (_gameDescription->features & GF_ENHANCED) != 0;
}

void MM1Engine::setupNormal() {
	byte pal[12];
	pal[0] = pal[1] = pal[2] = 0;
	pal[3] = 168; pal[4] = 84; pal[5] = 0;
	pal[6] = 252; pal[7] = 252; pal[8] = 84;
	pal[9] = pal[10] = pal[11] = 255;

	g_system->getPaletteManager()->setPalette(pal, 0, 3);
	g_system->getPaletteManager()->setPalette(&pal[9], 255, 1);
}

bool MM1Engine::setupEnhanced() {
	if (!Common::File::exists("xeen.cc")) {// || !Common::File::exists("dark.cc")) {
		GUIErrorMessage(
			"In order to run in Enhanced mode,  please copy xeen.cc "
			"and dark.cc from a copy of World of Xeen\n"
			"or Dark Side of Xeen to your Might and Magic 1 game folder"
		);

		return false;
	}

	// Add the Xeen cc archives
	::MM::Xeen::CCArchive *xeenCC = new ::MM::Xeen::CCArchive(
		"xeen.cc", "xeen", true);
	SearchMan.add("xeen", xeenCC);
	/*
	::MM::Xeen::CCArchive *darkCC = new ::MM::Xeen::CCArchive(
		"dark.cc", "dark", true);
	SearchMan.add("dark", darkCC);
	*/
	// Load the palette
	Common::File f;
	if (!f.open("mm4.pal"))
		error("Could not load palette");

	byte pal[PALETTE_SIZE];
	for (int i = 0; i < PALETTE_SIZE; ++i)
		pal[i] = f.readByte() << 2;
	g_system->getPaletteManager()->setPalette(pal, 0, PALETTE_COUNT);

	return true;
}

bool MM1Engine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

} // namespace MM1
} // namespace MM
