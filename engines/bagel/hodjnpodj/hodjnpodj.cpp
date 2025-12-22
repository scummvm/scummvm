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
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/console.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/demo/app.h"
#include "bagel/hodjnpodj/metagame/frame/app.h"
#include "bagel/hodjnpodj/metagame/frame/hodjpodj.h"
#include "bagel/hodjnpodj/metagame/gtl/init.h"
#include "bagel/hodjnpodj/metagame/saves/savegame.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {

HodjNPodjEngine *g_engine;

HodjNPodjEngine::HodjNPodjEngine(OSystem *syst, const ADGameDescription *gameDesc) :
	BagelEngine(syst, gameDesc) {
	g_engine = this;
}

HodjNPodjEngine::~HodjNPodjEngine() {
	_boardgameThumbnail.free();
}

Common::Error HodjNPodjEngine::run() {
	initGraphics(GAME_WIDTH, GAME_HEIGHT);

	// Set the engine's debugger console
	setDebugger(new Console());

	_midi = new MusicPlayer();

	// Run the game
	BagelMetaEngine::setKeybindingMode(KBMODE_NORMAL);

	if (getGameId() == "hodjnpodj" && isDemo()) {
		Metagame::Demo::CTheApp app;
		app.InitApplication();
		app.InitInstance();
		app.setKeybinder(KeybindToKeycode);
		app.Run();

	} else {
		Metagame::Frame::CTheApp app;

		if (getGameId() == "mazeodoom") {
			app.setStartupMinigame("mazedoom_demo");
		} else if (getGameId() == "artparts") {
			app.setStartupMinigame("artparts_demo");
		} else if (ConfMan.getBool("metagame")) {
			Metagame::Frame::InitBFCInfo(&_bfcMgr);
			Metagame::Gtl::RunMeta(nullptr, &_bfcMgr, false);

			return Common::kNoError;
		} else {
			Common::String minigame = ConfMan.get("minigame");
			if (!minigame.empty())
				app.setStartupMinigame(minigame);
		}
		app.InitApplication();
		app.InitInstance();
		app.setKeybinder(KeybindToKeycode);

		app.Run();
	}

	return Common::kNoError;
}

Graphics::Screen *HodjNPodjEngine::getScreen() const {
	return AfxGetApp()->getScreen();
}

Common::Error HodjNPodjEngine::syncGame(Common::Serializer &s) {
	Metagame::Saves::SAVEGAME_INFO savegameInfo;
	if (s.isSaving())
		Metagame::Saves::ConvertToSGI(&_bfcMgr, &savegameInfo);

	savegameInfo.sync(s);

	if (s.isLoading()) {
		if (!ValidateSGInfo(&savegameInfo))
			return Common::kReadingFailed;

		if (ConvertFromSGI(&_bfcMgr, &savegameInfo) != ERR_NONE)
			return Common::kReadingFailed;
	}

	return Common::kNoError;
}

} // namespace HodjNPodj
} // namespace Bagel
