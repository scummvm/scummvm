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
#include "bolt/detection.h"
#include "bolt/xplib/xplib.h"

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

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
		_extendedViewport = ConfMan.getBool("extended_viewport");
	}

	// Initialize paletted graphics mode
	if (!_extendedViewport) {
		initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	} else {
		initGraphics(EXTENDED_SCREEN_WIDTH, EXTENDED_SCREEN_HEIGHT);
	}

	if ((getFeatures() & ADGF_DEMO) != 0)
		_isDemo = true;

	_xp->initialize();
	boltMain();
	_xp->terminate();

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
		_boothsBoltLib = nullptr;

		if (openBOLTLib(&_boothsBoltLib, &_boothsBoltCallbacks, assetPath("booths.blt"))) {
			int16 chosenSpecId = _extendedViewport ? 0 : 1;

			if (_xp->setDisplaySpec(&_displayMode, &_displaySpecs[chosenSpecId])) {
				assert(_displayMode >= 0);
				_displayWidth = _displaySpecs[_displayMode].width;
				_displayHeight = _displaySpecs[_displayMode].height;

				// Center within 384x240 virtual coordinate space...
				_displayX = (EXTENDED_SCREEN_WIDTH - _displayWidth) / 2;
				_displayY = (EXTENDED_SCREEN_HEIGHT - _displayHeight) / 2;

				_xp->setCoordSpec(_displayX, _displayY, _displayWidth, _displayHeight);
				
				if (_displayMode == 0) {
					_rtfHandle = openRTF(assetPath("cc_og.av"));
				} else {
					_rtfHandle = openRTF(assetPath("cc_cr.av"));
				}

				if (_rtfHandle) {
					int16 boothGroupBase = _isDemo ? 0x0E00 : 0x1700;

					if (!_isDemo) {
						playAV(_rtfHandle, 0, _displayWidth, _displayHeight, _displayX, _displayY);
					}

					boothSprite = getBOLTMember(_boothsBoltLib, (_displayMode != 0) ? (boothGroupBase + 1) : (boothGroupBase + 2));

					_xp->setTransparency(false);

					displayColors(getBOLTMember(_boothsBoltLib, boothGroupBase), stFront, 0);
					displayPic(boothSprite, _displayX, _displayY, stFront);

					_xp->updateDisplay();

					displayColors(getBOLTMember(_boothsBoltLib, boothGroupBase), stBack, 0);
					displayPic(boothSprite, _displayX, _displayY, stBack);

					playAV(_rtfHandle, _isDemo ? 0 : 2, _displayWidth, _displayHeight, _displayX, _displayY);

					freeBOLTGroup(_boothsBoltLib, boothGroupBase, 1);

					if (getBOLTGroup(_boothsBoltLib, 0, 1)) {
						setCursorPict(memberAddr(_boothsBoltLib, 0));

						if (initVRam(1500)) {
							barkerTable = createBarker(2, _isDemo ? 19 : 17);
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

								if (_isDemo) {
									registerSideShow(barkerTable, &BoltEngine::displayDemoPict, 17);
									registerSideShow(barkerTable, &BoltEngine::endDemo, 18);
								}

								_lettersWon = 0;
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

	freeBOLTGroup(_boothsBoltLib, 0, 1);

	if (_boothsBoltLib != 0)
		closeBOLTLib(&_boothsBoltLib);

	if (_rtfHandle != nullptr)
		closeRTF(_rtfHandle);

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

	int16 memberId = (_displayMode != 0) ? 0x1002 : 0x1001;
	displayPic(getBOLTMember(_boothsBoltLib, memberId), _displayX, _displayY, stFront);
	displayColors(getBOLTMember(_boothsBoltLib, 0x1000), stFront, 0);

	_xp->updateDisplay();

	while (_xp->getEvent(etMouseDown, &eventData) != etMouseDown);

	return prevBooth;
}

} // End of namespace Bolt
