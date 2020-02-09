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
#include "common/debug-channels.h"
#include "common/error.h"

#include "sludge/cursors.h"
#include "sludge/event.h"
#include "sludge/fonttext.h"
#include "sludge/floor.h"
#include "sludge/graphics.h"
#include "sludge/main_loop.h"
#include "sludge/newfatal.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/sludge.h"
#include "sludge/sound.h"
#include "sludge/speech.h"

namespace Sludge {

SludgeEngine *g_sludge;

Graphics::PixelFormat *SludgeEngine::getScreenPixelFormat() const { return _pixelFormat; }
Graphics::PixelFormat *SludgeEngine::getOrigPixelFormat() const { return _origFormat; }

SludgeEngine::SludgeEngine(OSystem *syst, const SludgeGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc) {

	// register your random source
	_rnd = new Common::RandomSource("sludge");

	// Add debug channels
	DebugMan.addDebugChannel(kSludgeDebugFatal, "Script", "Script debug level");
	DebugMan.addDebugChannel(kSludgeDebugDataLoad, "Data Load", "Data loading debug level");
	DebugMan.addDebugChannel(kSludgeDebugStackMachine, "Stack Machine", "Stack Machine debug level");
	DebugMan.addDebugChannel(kSludgeDebugBuiltin, "Built-in", "Built-in debug level");
	DebugMan.addDebugChannel(kSludgeDebugGraphics, "Graphics", "Graphics debug level");
	DebugMan.addDebugChannel(kSludgeDebugZBuffer, "ZBuffer", "ZBuffer debug level");
	DebugMan.addDebugChannel(kSludgeDebugSound, "Sound", "Sound debug level");

	DebugMan.enableDebugChannel("Data Load");
	DebugMan.enableDebugChannel("Built-in");

	// init graphics
	_origFormat = new Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_pixelFormat = new Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);

	// Init Strings
	launchMe = "";
	launchNext = "";
	loadNow = "";
	gamePath = "";

	// Init managers
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
}

SludgeEngine::~SludgeEngine() {

	// Dispose resources
	delete _rnd;
	_rnd = nullptr;

	// Remove debug levels
	DebugMan.clearAllDebugChannels();

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
}

Common::Error SludgeEngine::run() {
	// set global variable
	g_sludge = this;

	// create console
	setDebugger(new SludgeConsole(this));

	// debug log
	main_loop(getGameFile());

	return Common::kNoError;
}

} // End of namespace Sludge

