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
#include "common/translation.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/console.h"
#include "mm/mm1/gfx/gfx.h"
#include "mm/mm1/views/game.h"
#include "mm/mm1/views_enh/game.h"
#include "mm/shared/xeen/cc_archive.h"

namespace MM {
namespace MM1 {

#define SAVEGAME_VERSION 1

MM1Engine *g_engine = nullptr;

MM1Engine::MM1Engine(OSystem *syst, const MightAndMagicGameDescription *gameDesc)
		: MMEngine(syst, gameDesc), Events(gameDesc->features & GF_ENHANCED) {
	g_engine = this;
}

MM1Engine::~MM1Engine() {
	g_engine = nullptr;
	delete _sound;
}

Common::Error MM1Engine::run() {
	if (_gameDescription->features & GF_GFX_PACK) {
		GUIErrorMessage(_("You cannot run the game directly from the Graphics Overhaul Mod. "
			"Instead, it will automatically be available if you detect the original game "
			"and select Enhanced mode."));
		return Common::kNoError;
	}

	// Initialize graphics mode
	initGraphics(320, 200);

	// Setup mixer
	_sound = new Sound(_mixer);
	syncSoundSettings();

	if (isEnhanced()) {
		if (!setupEnhanced())
			return Common::kNoError;
	} else {
		setupNormal();
	}

	// Setup console
	setDebugger(new Console());
	if (gDebugLevel > 0)
		// TODO: Remove flag once everything is tested
		g_globals->_encountersOn = false;

	// Load globals
	if (!_globals.load(isEnhanced()))
		return Common::kNoError;

	runGame();
	return Common::kNoError;
}

void MM1Engine::syncSoundSettings() {
	Engine::syncSoundSettings();

	if (_sound)
		_sound->updateSoundSettings();
}

bool MM1Engine::isEnhanced() const {
	return (_gameDescription->features & GF_ENHANCED) != 0;
}

void MM1Engine::setupNormal() {
	Gfx::GFX::setEgaPalette();
}

bool MM1Engine::setupEnhanced() {
	if (!Common::File::exists("xeen.cc")) {
		GUIErrorMessage(_(
			"In order to run in Enhanced mode,  please copy xeen.cc "
			"from a copy of World of Xeen\n"
			"or Clouds of Xeen to your Might and Magic 1 game folder"
		));

		return false;
	}

	// Add the Xeen cc archives
	Shared::Xeen::CCArchive *xeenCC = new Shared::Xeen::CCArchive(
		"xeen.cc", "xeen", true);
	SearchMan.add("xeen", xeenCC);

	// Load the palette
	Common::File f;
	if (!f.open("mm4.pal"))
		error("Could not load palette");

	// Load the Xeen palette
	byte pal[PALETTE_SIZE];
	for (int i = 0; i < PALETTE_SIZE; ++i)
		pal[i] = f.readByte() << 2;
	g_system->getPaletteManager()->setPalette(pal, 0, PALETTE_COUNT);
	Gfx::GFX::findPalette(pal);

	// Show the mouse cursor
	g_events->loadCursors();
	g_events->setCursor(0);
	g_events->showCursor();

	return true;
}

bool MM1Engine::canSaveGameStateCurrently(Common::U32String *msg) {
	if (!g_events)
		return false;

	UIElement *view = g_events->focusedView();
	return dynamic_cast<Views::Game *>(view) != nullptr ||
		dynamic_cast<ViewsEnh::Game *>(view) != nullptr;
}

bool MM1Engine::canLoadGameStateCurrently(Common::U32String *msg) {
	if (!g_events)
		return false;

	// Loading savegames can be done in any view, since we can
	// just remove them all and add the game view
	return true;
}

Common::Error MM1Engine::synchronizeSave(Common::Serializer &s) {
	// Get/set the version
	byte version = SAVEGAME_VERSION;
	s.syncAsByte(version);
	if (version > SAVEGAME_VERSION)
		return Common::kReadingFailed;
	s.setVersion(version);

	// If we're loading a savegame, switch to the game view
	if (s.isLoading()) {
		g_events->replaceView("Game", true);
	}

	// Sync globals
	g_globals->synchronize(s);

	return Common::kNoError;
}

} // namespace MM1
} // namespace MM
