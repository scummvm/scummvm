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

#include "bolt/bolt.h"
#include "graphics/framelimiter.h"
#include "bolt/detection.h"
#include "bolt/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

#include "bolt/xplib/xplib.h"

namespace Bolt {

BoltEngine *g_engine;

BoltEngine::BoltEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Bolt") {
	g_engine = this;
	_xp = new XpLib(this);
}

BoltEngine::~BoltEngine() {
	delete _xp;
}

uint32 BoltEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String BoltEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error BoltEngine::run() {
	ConfMan.registerDefault("extended_viewport", false);
	if (ConfMan.hasKey("extended_viewport", _targetName)) {
		g_extendedViewport = ConfMan.getBool("extended_viewport");
	}

	// Initialize paletted graphics mode
	if (g_extendedViewport) {
		initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	} else {
		initGraphics(EXTENDED_SCREEN_WIDTH, EXTENDED_SCREEN_HEIGHT);
	}

	// Set the engine's debugger console
	setDebugger(new Console());

	//
	//// If a savegame was selected from the launcher, load it
	//int saveSlot = ConfMan.getInt("save_slot");
	//if (saveSlot != -1)
	//	(void)loadGameState(saveSlot);
	//
	//// Draw a series of boxes on screen as a sample
	//for (int i = 0; i < 100; ++i)
	//	_screen->frameRect(Common::Rect(i, i, 320 - i, 200 - i), i);
	//_screen->update();
	//
	//// Simple event handling loop
	//byte pal[256 * 3] = { 0 };
	//Common::Event e;
	//int offset = 0;
	//
	//Graphics::FrameLimiter limiter(g_system, 60);
	//while (!shouldQuit()) {
	//	while (g_system->getEventManager()->pollEvent(e)) {
	//	}
	//
	//	// Cycle through a simple palette
	//	++offset;
	//	for (int i = 0; i < 256; ++i)
	//		pal[i * 3 + 1] = (i + offset) % 256;
	//	g_system->getPaletteManager()->setPalette(pal, 0, 256);
	//	// Delay for a bit. All events loops should have a delay
	//	// to prevent the system being unduly loaded
	//	limiter.delayBeforeSwap();
	//	_screen->update();
	//	limiter.startFrame();
	//}

	_xp->initialize();
	boltMain();

	return Common::kNoError;
}

Common::Error BoltEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

void BoltEngine::boltMain() {
	void *testAlloc;
	BarkerTable *barkerTable;
	void *boothSprite;

	// g_callbackPtr = DS:0xCB; 

	testAlloc = _xp->allocMem(0x100000);
	if (!testAlloc)
		return;

	_xp->freeMem(testAlloc);

	_xp->randomize();

	if (!allocResourceIndex())
		goto cleanup;

	g_boothsBoltLib = 0;

	if (!openBOLTLib(&g_boothsBoltLib, &g_boothsBoltIndex, AssetPath("booths.blt")))
		goto cleanup;

	if (!_xp->chooseDisplaySpec(&g_displayMode, 2, g_displaySpecs))
		goto cleanup;

	g_displayWidth = g_displaySpecs[g_displayMode].width;
	g_displayHeight = g_displaySpecs[g_displayMode].height;

	// Center within 384x240 virtual coordinate space...
	g_displayX = (384 - g_displayWidth) / 2;
	g_displayY = (240 - g_displayHeight) / 2;

	_xp->setCoordSpec(g_displayX, g_displayY, g_displayWidth, g_displayHeight);

	if (g_displayMode != 0)
		g_rtfHandle = openRTF(AssetPath("booths.pal"));
	else
		g_rtfHandle = openRTF(AssetPath("booths4.pal"));

	if (g_rtfHandle == nullptr)
		goto cleanup;

	playAV(g_rtfHandle, 0, g_displayWidth, g_displayHeight, g_displayX, g_displayY);

	boothSprite = getBOLTMember(g_boothsBoltLib, (g_displayMode != 0) ? 0x1701 : 0x1702);

	_xp->setTransparency(false);

	displayColors(getBOLTMember(g_boothsBoltLib, 0x1700), 0, 0);
	displayPic(boothSprite, g_displayX, g_displayY, 0);

	_xp->updateDisplay();

	displayColors(getBOLTMember(g_boothsBoltLib, 0x1700), 1, 0);
	displayPic(boothSprite, g_displayX, g_displayY, 1);

	playAV(g_rtfHandle, 2, g_displayWidth, g_displayHeight, g_displayX, g_displayY);

	freeBOLTGroup(g_boothsBoltLib, 0x1700, 1);

	if (!getBOLTGroup(g_boothsBoltLib, 0, 1))
		goto cleanup;

	setCursorPict(memberAddr(g_boothsBoltLib, 0));

	if (!initVRam(1500))
		goto cleanup;

	barkerTable = createBarker(17, 2);
	if (barkerTable == nullptr)
		goto cleanup;

	// Register booth handlers...
	registerSideShow(barkerTable, &BoltEngine::hucksBooth, 3);
	registerSideShow(barkerTable, &BoltEngine::fredsBooth, 4);
	registerSideShow(barkerTable, &BoltEngine::scoobysBooth, 5);
	registerSideShow(barkerTable, &BoltEngine::yogisBooth, 6);
	registerSideShow(barkerTable, &BoltEngine::georgesBooth, 7);
	registerSideShow(barkerTable, &BoltEngine::topCatsBooth, 8);
	registerSideShow(barkerTable, &BoltEngine::mainEntrance, 9);
	registerSideShow(barkerTable, &BoltEngine::huckGame, 10);
	registerSideShow(barkerTable, &BoltEngine::fredGame, 11);
	registerSideShow(barkerTable, &BoltEngine::scoobyGame, 12);
	registerSideShow(barkerTable, &BoltEngine::yogiGame, 13);
	registerSideShow(barkerTable, &BoltEngine::georgeGame, 14);
	registerSideShow(barkerTable, &BoltEngine::topCatGame, 15);
	registerSideShow(barkerTable, &BoltEngine::winALetter, 16);

	g_lettersWon = 0;
	_xp->setScreenSaverTimer(1800);

	// The barker function runs the main loop, starting at mainEntrance()...
	if (!checkError())
		barker(barkerTable, 9);

	freeBarker(barkerTable);

cleanup:
	freeBOLTGroup(g_boothsBoltLib, 0, 1);

	if (g_boothsBoltLib != 0)
		closeBOLTLib(&g_boothsBoltLib);

	if (g_rtfHandle != nullptr)
		closeRTF(g_rtfHandle);

	freeVRam();
	freeResourceIndex();
}

BarkerTable *BoltEngine::createBarker(int16 minIndex, int16 maxIndex) {
	return nullptr;
}

void BoltEngine::freeBarker(BarkerTable *table) {
}

void BoltEngine::registerSideShow(BarkerTable *table, SideShowHandler handler, short boothId) {
}

void BoltEngine::barker(BarkerTable *table, int16 startBooth) {
}

bool BoltEngine::checkError() {
	return false;
}

} // End of namespace Bolt
