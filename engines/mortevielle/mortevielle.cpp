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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "engines/util.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mort.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

MortevielleEngine *g_vm;

MortevielleEngine::MortevielleEngine(OSystem *system, const ADGameDescription *gameDesc):
		Engine(system), _gameDescription(gameDesc) {
	g_vm = this;
}

MortevielleEngine::~MortevielleEngine() {
}

bool MortevielleEngine::hasFeature(EngineFeature f) const {
	return false;
}

Common::ErrorCode MortevielleEngine::initialise() {
	// Initialise graphics mode
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT, true);

	// Set up an intermediate screen surface
	_screenSurface.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	// Set the screen mode
	gd = ega;
	res = 2;

	// Load the mort.dat resource
	return loadMortDat();
}

/**
 * Loads the contents of the Mort.dat data file
 */
Common::ErrorCode MortevielleEngine::loadMortDat() {
	Common::File f;

	// Open the mort.dat file
	if (!f.open(MORT_DAT)) {
		GUIErrorMessage("Could not locate Mort.dat file");
		return Common::kReadingFailed;
	}

	// Validate the data file header
	char fileId[4];
	f.read(fileId, 4);
	if (strncmp(fileId, "MORT", 4) != 0) {
		GUIErrorMessage("The located mort.dat data file is invalid");
		return Common::kReadingFailed;
	}

	// Check the version
	if (f.readByte() < MORT_DAT_REQUIRED_VERSION) {
		GUIErrorMessage("The located mort.dat data file is too a version");
		return Common::kReadingFailed;
	}
	f.readByte();		// Minor version

	// Loop to load resources from the data file
	while (f.pos() < f.size()) {
		// Get the Id and size of the next resource
		char dataType[4];
		int dataSize;
		f.read(dataType, 4);
		dataSize = f.readUint16LE();

		if (!strncmp(dataType, "FONT", 4)) {
			// Font resource
			assert(dataSize == (FONT_NUM_CHARS * FONT_HEIGHT));
			f.read(_fontData, FONT_NUM_CHARS * FONT_HEIGHT);
		} else {
			// Unknown section
			f.skip(dataSize);
		}
	}

	f.close();
	return Common::kNoError;
}

/*-------------------------------------------------------------------------*/

/**
 * Update the physical screen
 */
void MortevielleEngine::updateScreen() {
	g_system->copyRectToScreen((const byte *)_screenSurface.getBasePtr(0, 0), 
		SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_system->updateScreen();
}
/**
 * Draws a character at the specified co-ordinates
 * @remarks		Because the ScummVM surface is using a double height 640x200 surface to 
 *		simulate the original 640x200 surface, all Y values have to be doubled
 */
void MortevielleEngine::writeCharacter(const Common::Point &pt, unsigned char ch, 
		int palIndex, Graphics::Surface *surface) {
	if (surface == NULL)
		surface = &_screenSurface;

	// Get the start of the character to use
	assert((ch >= ' ') && (ch <= (unsigned char)(32 + FONT_NUM_CHARS)));
	const byte *charData = &_fontData[((int)ch - 32) * FONT_HEIGHT];

	// Loop through decoding each character's data
	for (int yp = 0; yp < FONT_HEIGHT; ++yp) {
		byte *lineP = (byte *)surface->getBasePtr(pt.x, (pt.y + yp) * 2);
		byte byteVal = *charData++;

		for (int xp = 0; xp < 8; ++xp, ++lineP, byteVal <<= 1) {
			if (byteVal & 0x80) {
				*lineP = palIndex;
				*(lineP + SCREEN_WIDTH) = palIndex;
			}
		}
	}
}

/**
 * Sets a single pixel at the specified co-ordinates
 * @remarks		Because the ScummVM surface is using a double height 640x200 surface to 
 *		simulate the original 640x200 surface, all Y values have to be doubled
 */
void MortevielleEngine::setPixel(const Common::Point &pt, int palIndex, 
		Graphics::Surface *surface) {
	if (surface == NULL)
		surface = &_screenSurface;

	byte *destP = (byte *)surface->getBasePtr(pt.x, pt.y * 2);
	*destP = palIndex;
	*(destP + SCREEN_WIDTH) = palIndex;
}


/*-------------------------------------------------------------------------*/

Common::Error MortevielleEngine::run() {
	// Initialise the game
	Common::ErrorCode err = initialise();
	if (err != Common::kNoError)
		return err;

	// TODO: Remove once palette loading is correctly done
	for (int idx = 0; idx < 16; ++idx) {
		uint32 c = 0x111111 * idx;
		g_system->getPaletteManager()->setPalette((const byte *)&c, idx, 1);
	}

	// Dispatch to the game's main routine
	const char *argv[] = { "" };
	mortevielle_main(1, argv);

	return Common::kNoError;
}

} // End of namespace Mortevielle
