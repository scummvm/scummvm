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

// common/config-manager is needed for the search manager
#include "common/config-manager.h"

// engines/util is needed for initGraphics()
#include "engines/util.h"
#include "immortal/immortal.h"

namespace Immortal {

ImmortalEngine *g_immortal;

ImmortalEngine::ImmortalEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst)
	, _gameDescription(gameDesc)
	, _randomSource("Immortal") {
	g_immortal = this;

	// Add the game folder to the search manager path variable
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "game");

	// Confirm that the engine was created
	debug("ImmortalEngine::ImmortalEngine");
}

ImmortalEngine::~ImmortalEngine() {
	// Confirm that the engine was destroyed
	debug("ImmortalEngine::~ImmortalEngine");
}

// --- Functions to make things a little more simple ---

uint16 ImmortalEngine::xba(uint16 ab) {
	/* XBA in 65816 swaps the low and high bits of a given word in A.
	 * This is used very frequently, so this function just makes
	 * initial translation a little more straightforward. Eventually,
	 * code should be refactored to not require this.
	 */
	return ((ab & kMaskLow) << 8) | ((ab & kMaskHigh) >> 8);
}

uint16 ImmortalEngine::rol(uint16 ab, int n) {
	/* This just replicates bit rotation, and it
	 * assumes a 16 bit unsigned int because that's all
	 * we need for the 65816.
	 */
	return (ab << n) | (ab >> (-n & 15));
}

uint16 ImmortalEngine::ror(uint16 ab, int n) {
	/* The way this works is very straightforward. We start by
	 * performing the bit shift like normal: 0001 -> 0000
	 * Then we need an easy way to apply the bit whether it fell
	 * off the end or not, so we bit shift the opposite direction
	 * for the length of the word minus the size of the shift.
	 * This way, the bit that we shifted normally, gets
	 * separately moved to the other end: 0001 -> 1000
	 * In other words, the space C uses to evaluate the second
	 * part of the expression, is simulating the register for the
	 * carry flag. To avoid branching in case of a 0, we get the
	 * shift size by using the negative of the number as an
	 * implicit subtraction and grabbing just the relevant bits.
	 */
	return (ab >> n) | (ab << (-n & 15));
}

uint16 ImmortalEngine::mult16(uint16 a, uint16 b) {
	/* We aren't using the game's multiplication function (mult16), but we do want
	 * to retain the ability to drop the second word, without doing (uint16) every time
	 */
	return (uint16)(a * b);
}
// -----------------------------------------------------

uint32 ImmortalEngine::getFeatures() const {
	// No specific features currently
	return _gameDescription->flags;
}

Common::String ImmortalEngine::getGameId() const {
	// No game ID currently
	return _gameDescription->gameId;
}

Common::ErrorCode ImmortalEngine::initDisks() {
	// Check for the boot disk
	if (SearchMan.hasFile("IMMORTAL.dsk")) {

		// Instantiate the disk as an object. The disk will then open and parse itself
		ProDOSDisk *diskBoot = new ProDOSDisk("IMMORTAL.dsk");
		if (diskBoot) {

			// With the disk successfully parsed, it can be added to the search manager
			debug("Boot disk found");
			SearchMan.add("IMMORTAL.dsk", diskBoot, 0, true);
		}
	} else {
		debug("Please insert Boot disk...");
		return Common::kPathDoesNotExist;
	}

	// Check for the gfx disk
	if (SearchMan.hasFile("IMMORTAL_GFX.dsk")) {
		ProDOSDisk *diskGFX = new ProDOSDisk("IMMORTAL_GFX.dsk");
		if (diskGFX) {
			debug("Gfx disk found");
			SearchMan.add("IMMORTAL_GFX.dsk", diskGFX, 0, true);
		}
	} else {
		debug("Please insert GFX disk...");
		return Common::kPathDoesNotExist;
	}

	// All good, return with no error
	return Common::kNoError;
}

Common::Error ImmortalEngine::run() {
	initGraphics(kResH, kResV);

	_mainSurface = new Graphics::Surface();
	_mainSurface->create(kResH, kResV, Graphics::PixelFormat::createFormatCLUT8());

	_screenBuff = new byte[kScreenSize];

	if (initDisks() != Common::kNoError) {
		debug("Some kind of disc error!");
		return Common::kPathDoesNotExist;
	}

	// Main:
	_zero = 0;
	_draw = 1;
	_usingNormal = 1;
	_penY = 7;
	_penX = 1;

	initStoryStatic();                      // Init the arrays of static story elements (done at compile time in the source)
	loadPalette();                          // We need to grab the palette from the disk first

	// This is the equivalent of Main->InitGraphics->MyClearScreen in Driver
	useNormal();                            // The first palette will be the default

	loadFont();                             // Load the font sprites
	loadWindow();                           // Load the window background
	loadSingles("Song A");                  // Music
	loadSprites();                          // Get all the sprite data into memory

	_playing = kSongNothing;
	_themePaused = 0;

	clearSprites();                         // Clear the sprites before we start
	// This is where the request play disk would happen, but that's not needed here
	logicInit();                            // Init the game logic

	_err = Common::kNoError;

	while (!shouldQuit()) {
		/* The game loop runs at 60fps, which is 16 milliseconds per frame.
		 * This loop keeps that time by getting the time in milliseconds at the start of the loop,
		 * then again at the end, and the difference between them is the remainder
		 * of the frame budget. If that remainder is within the 16 millisecond budget,
		 * then it delays ScummVM for the remainder. If it is 0 or negative, then it continues.
		 */
		int64 loopStart = g_system->getMillis();

		// Main
		Common::Event event;
		g_system->getEventManager()->pollEvent(event);

		userIO();
		noNetwork();
		pollKeys();
		logic();
		pollKeys();
		if (logicFreeze() == 0) {
			drawUniv();
			pollKeys();
			fixColors();
			copyToScreen();
			pollKeys();
		}

		if (_err != Common::kNoError) {
			debug("To err is human, to really screw up you need an Apple IIGS!");
			return Common::kUnknownError;
		}

		int64 loopEnd = 16 - (g_system->getMillis() - loopStart);
		if (loopEnd > 0) {
			//debug("remaining budget: %d", loopEnd);
			g_system->delayMillis(loopEnd);
		}
	}

	return Common::kNoError;
}

Common::Error ImmortalEngine::syncGame(Common::Serializer &s) {
	/* The Serializer has methods isLoading() and isSaving()
	 * if you need to specific steps; for example setting
	 * an array size after reading it's length, whereas
	 * for saving it would write the existing array's length
	 */
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // namespace Immortal
