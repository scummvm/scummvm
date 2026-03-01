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

	initCallbacks();
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
	if (!g_extendedViewport) {
		initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	} else {
		initGraphics(EXTENDED_SCREEN_WIDTH, EXTENDED_SCREEN_HEIGHT);
	}

	if ((getFeatures() & ADGF_DEMO) != 0)
		g_isDemo = true;

	// Set the engine's debugger console
	setDebugger(new Console());

	_xp->initialize();
	boltMain();
	_xp->terminate();

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
	byte *testAlloc;
	BarkerTable *barkerTable;
	byte *boothSprite;

	testAlloc = (byte *)_xp->allocMem(0x100000);
	if (!testAlloc)
		return;

	_xp->freeMem(testAlloc);

	_xp->randomize();

	if (allocResourceIndex()) {
		g_boothsBoltLib = nullptr;

		if (openBOLTLib(&g_boothsBoltLib, &g_boothsBoltCallbacks, assetPath("booths.blt"))) {
			int16 chosenSpecId = g_extendedViewport ? 0 : 1;

			if (_xp->setDisplaySpec(&g_displayMode, &g_displaySpecs[chosenSpecId])) {
				assert(g_displayMode >= 0);
				g_displayWidth = g_displaySpecs[g_displayMode].width;
				g_displayHeight = g_displaySpecs[g_displayMode].height;

				// Center within 384x240 virtual coordinate space...
				g_displayX = (EXTENDED_SCREEN_WIDTH - g_displayWidth) / 2;
				g_displayY = (EXTENDED_SCREEN_HEIGHT - g_displayHeight) / 2;

				_xp->setCoordSpec(g_displayX, g_displayY, g_displayWidth, g_displayHeight);
				
				if (g_displayMode == 0) {
					g_rtfHandle = openRTF(assetPath("cc_og.av"));
				} else {
					g_rtfHandle = openRTF(assetPath("cc_cr.av"));
				}

				if (g_rtfHandle) {
					int16 boothGroupBase = g_isDemo ? 0x0E00 : 0x1700;

					if (!g_isDemo) {
						playAV(g_rtfHandle, 0, g_displayWidth, g_displayHeight, g_displayX, g_displayY);
					}

					boothSprite = getBOLTMember(g_boothsBoltLib, (g_displayMode != 0) ? (boothGroupBase + 1) : (boothGroupBase + 2));

					_xp->setTransparency(false);

					displayColors(getBOLTMember(g_boothsBoltLib, boothGroupBase), stFront, 0);
					displayPic(boothSprite, g_displayX, g_displayY, stFront);

					_xp->updateDisplay();

					displayColors(getBOLTMember(g_boothsBoltLib, boothGroupBase), stBack, 0);
					displayPic(boothSprite, g_displayX, g_displayY, stBack);

					playAV(g_rtfHandle, g_isDemo ? 0 : 2, g_displayWidth, g_displayHeight, g_displayX, g_displayY);

					freeBOLTGroup(g_boothsBoltLib, boothGroupBase, 1);

					if (getBOLTGroup(g_boothsBoltLib, 0, 1)) {
						setCursorPict(memberAddr(g_boothsBoltLib, 0));

						if (initVRam(1500)) {
							barkerTable = createBarker(2, g_isDemo ? 19 : 17);
							if (barkerTable) {
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

								if (g_isDemo) {
									registerSideShow(barkerTable, &BoltEngine::displayDemoPict, 17);
									registerSideShow(barkerTable, &BoltEngine::endDemo, 18);
								}

								g_lettersWon = 0;
								_xp->setScreenSaverTimer(1800);

								// The barker function runs the main loop, starting at mainEntrance()...
								if (!checkError())
									barker(barkerTable, 9);

								freeBarker(barkerTable);
							}
						}
					}
				}
			}
		}
	}

	freeBOLTGroup(g_boothsBoltLib, 0, 1);

	if (g_boothsBoltLib != 0)
		closeBOLTLib(&g_boothsBoltLib);

	if (g_rtfHandle != nullptr)
		closeRTF(g_rtfHandle);

	freeVRam();
	freeResourceIndex();
}

void BoltEngine::setCursorPict(byte *sprite) {
	byte cursorBitmap[32]; // 16x16
	byte *srcPtr = getResolvedPtr(sprite, 0x12);

	// Convert 8bpp pixel data to 1bpp monochrome bitmap
	for (int i = 0; i < 32; i++) {
		cursorBitmap[i] = 0x00;
		int16 mask = 0x80;
		while (mask != 0x00) {
			if (*srcPtr++ != 0x00)
				cursorBitmap[i] |= (byte)mask;

			mask >>= 1;
		}
	}

	_xp->setCursorImage(cursorBitmap, 7, 7);
}

int16 BoltEngine::displayDemoPict(int16 prevBooth) {
	uint32 eventData;

	while (_xp->getEvent(etMouseDown, &eventData) == etMouseDown);

	while (_xp->getEvent(etTimer, &eventData) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);

	int16 memberId = (g_displayMode != 0) ? 0x1002 : 0x1001;
	displayPic(getBOLTMember(g_boothsBoltLib, memberId), g_displayX, g_displayY, stFront);
	displayColors(getBOLTMember(g_boothsBoltLib, 0x1000), stFront, 0);

	_xp->updateDisplay();

	while (_xp->getEvent(etMouseDown, &eventData) != etMouseDown);

	return prevBooth;
}

} // End of namespace Bolt
