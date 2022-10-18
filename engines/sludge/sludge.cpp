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
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/random.h"

#include "sludge/cursors.h"
#include "sludge/event.h"
#include "sludge/fileset.h"
#include "sludge/fonttext.h"
#include "sludge/floor.h"
#include "sludge/graphics.h"
#include "sludge/language.h"
#include "sludge/main_loop.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/sludge.h"
#include "sludge/sound.h"
#include "sludge/speech.h"
#include "sludge/statusba.h"
#include "sludge/timing.h"

namespace Sludge {

SludgeEngine *g_sludge;

Graphics::PixelFormat *SludgeEngine::getScreenPixelFormat() const { return _pixelFormat; }
Graphics::PixelFormat *SludgeEngine::getOrigPixelFormat() const { return _origFormat; }

SludgeEngine::SludgeEngine(OSystem *syst, const SludgeGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc) {

	// register your random source
	_rnd = new Common::RandomSource("sludge");

	//DebugMan.enableDebugChannel("loading");
	//DebugMan.enableDebugChannel("builtin");

	_dumpScripts = ConfMan.getBool("dump_scripts");

	// init graphics
	_origFormat = new Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_pixelFormat = new Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);

	// Init Strings
	launchNext = "";
	loadNow = "";
	gamePath = "";

	// Init managers
	_timer = new Timer();
	_fatalMan = new FatalMsgManager();
	_peopleMan = new PeopleManager(this);
	_resMan = new ResourceManager();
	_languageMan = new LanguageManager();
	_objMan = new ObjectManager(this);
	_gfxMan = new GraphicsManager(this);
	_evtMan = new EventManager(this);
	_soundMan = new SoundManager();
	_txtMan = new TextManager();
	_cursorMan = new CursorManager(this);
	_speechMan = new SpeechManager(this);
	_regionMan = new RegionManager(this);
	_floorMan = new FloorManager(this);
	_statusBar = new StatusBarManager(this);
}

SludgeEngine::~SludgeEngine() {

	// Dispose resources
	delete _rnd;
	_rnd = nullptr;

	// Dispose pixel formats
	delete _origFormat;
	_origFormat = nullptr;
	delete _pixelFormat;
	_pixelFormat = nullptr;

	// Dispose managers
	delete _cursorMan;
	_cursorMan = nullptr;
	delete _txtMan;
	_txtMan = nullptr;
	delete _soundMan;
	_soundMan = nullptr;
	delete _evtMan;
	_evtMan = nullptr;
	delete _gfxMan;
	_gfxMan = nullptr;
	delete _objMan;
	_objMan = nullptr;
	delete _languageMan;
	_languageMan = nullptr;
	delete _resMan;
	_resMan = nullptr;
	delete _speechMan;
	_speechMan = nullptr;
	delete _regionMan;
	_regionMan = nullptr;
	delete _peopleMan;
	_peopleMan = nullptr;
	delete _floorMan;
	_floorMan = nullptr;
	delete _fatalMan;
	_fatalMan = nullptr;
	delete _statusBar;
	delete _timer;
}

Common::Error SludgeEngine::run() {
	// set global variable
	g_sludge = this;

	// debug log
	main_loop(getGameFile());

	return Common::kNoError;
}

} // End of namespace Sludge
