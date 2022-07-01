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

#ifndef IMMORTAL_IMMORTAL_H
#define IMMORTAL_IMMORTAL_H

#include "audio/mixer.h"

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "common/platform.h"

#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "immortal/detection.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Immortal {

enum BitMask16 : uint16 {
    kMaskLow   = 0x00FF,
    kMaskHigh  = 0xFF00,
    kMaskLast  = 0xF000,
    kMaskFirst = 0x000F,
    kMaskHLow  = 0x0F00,
    kMaskLHigh = 0x00F0,
    kMaskNeg   = 0x8000,
};

enum BitMask8 : uint8 {
    kMaskASCII = 0x7F,                                  // the non-extended ASCII table uses 7 bits, this makes a couple of things easier
    kMask8High = 0xF0,
    kMask8Low  = 0x0F,
};


enum ColourMask : uint16 {
	kMaskRed   = 0x0F00,
	kMaskGreen = 0x00F0,
	kMaskBlue  = 0x000F
};

struct ImmortalGameDescription;

// Forward declaration because we will need the Disk class
class ProDosDisk;

class ImmortalEngine : public Engine {
private:
	Common::RandomSource _randomSource;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	ImmortalEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~ImmortalEngine() override;

	/* Terrible functions because C doesn't like
	 * bit manipulation enough
	 */
	uint16 xba(uint16 ab);								// This just replicates the XBA command from the 65816, because flipping the byte order is somehow not a common library function???
	uint16 rol(uint16 ab, int n);						// Rotate bits left by n
	uint16 ror(uint16 ab, int n);						// Rotate bits right by n
	uint16 mult16(uint16 a, uint16 b);					// Just avoids using (uint16) everywhere, and is slightly closer to the original

	const ADGameDescription *_gameDescription;

	/* 'global' members
	 */
			bool _draw;									// Whether the screen should draw this frame
			bool _dim;									// Whether the palette is dim
			bool _usingNormal;							// Whether the palette is using normal
			 int _zero;									// No idea what this is yet
	Common::File _window;								// Bitmap of the window around the game
	Common::File _font;									// The gfx for font sprites? Why is this not done with tilemaps...

	Common::HashMap<Common::String, Common::File> _sprites;	// Dictionary containing all sprites

	/* Screen related members
	 */
	Graphics::Surface *_mainSurface;
				 byte *_screenBuff;						// The final buffer that will transfer to the screen
			 const int _resH = 320;
			 const int _resV = 200;
			 const int _screenSize = (_resH * _resV) * 2; // The size of the screen buffer is 320x200

	/* Palette related members
	 */
	const int kPaletteOffset = 21205;					// This is the byte position of the palette data in the disk
	uint16 _palDefault[16];
	uint16 _palWhite[16];
	uint16 _palBlack[16];
	uint16 _palDim[16];
	  byte _palRGB[48];									// Palette that ScummVM actually uses, which is an RGB conversion of the original
	   int _dontResetColors = 0;						// Not sure yet

	/* Functions from Kernal.gs and Driver.gs
	 */

	// Palette functions
	void loadPalette();									// Get the static palette data from the disk
	void setColors(uint16 pal[]);						// Applies the current palette to the ScummVM surface palette
	void fixColors();									// Determine whether the screen should be dim or normal
	void useNormal();
	void useDim();
	void useBlack();
	void useWhite();
	void pump();										// Alternates between white and black with delays in between (flashes screen)
	void fadePal(uint16 pal[], int count, uint16 target[]); // Fades the palette except the frame
	void fade(uint16 pal[], int dir, int delay);		// Calls fadePal() by a given delay each iteration
	void fadeOut(int j);								// Calls Fade with a delay of j jiffies and direction 1
	void fadeIn(int j);									// || and direction 0

	void delay(int j);									// Delay engine by j jiffies
	void delay4(int j);									// || /4
	void delay8(int j);									// || /8

	Common::ErrorCode main();							// Main game loop
	void loadWindow();									// Gets the window.bm file
	void loadFont();									// Gets the font.spr file, and centers the sprite
	Common::SeekableReadStream *loadIFF(Common::String fileName); // Loads a file and uncompresses if it is compressed
	void loadSingles(Common::String songName);			// Loads and then parse the maze song
	void loadSprites();									// Loads all the sprite files and centers their sprites (in spritelist, but called from kernal)
	void clearSprites();								// Clears all sprites before drawing the current frame
	//void Misc::textPrint(int num);
	void logicInit();
	void userIO();										// Get input
	void pollKeys();									// Buffer input
	void noNetwork();									// Setup input mirrors
	void keyTraps();									// Seems to be debug only
	void logic();										// Keeps time, handles win and lose conditions, then general logic
	void restartLogic();								// This is the actual logic init
	int logicFreeze();									// Overcomplicated way to check if game over or level over
	void drawUniv();									// Draw the background, add the sprites, determine draw order, draw the sprites
	
	void copyToScreen();								// If draw is 0, just check input, otherwise also copy the screen buffer to the scummvm surface and update screen
	void clearScreen();									// Draws a black rectangle on the screen buffer but only inside the frame

	/* General engine functions
	 */
	Common::ErrorCode initDisks();						// Opens and parses IMMORTAL.dsk and IMMORTAL_GFX.dsk
	uint32 getFeatures() const;							// Returns the game description flags
	Common::String getGameId() const;					// Returns the game Id

	/* Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveGameStateCurrently() override {
		return true;
	}

	/* Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	/* Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}

	Common::Error loadGameStream(Common::SeekableReadStream *stream) {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	} */
};

extern ImmortalEngine *g_engine;
#define SHOULD_QUIT ::Immortal::g_engine->shouldQuit()

} // namespace Immortal

#endif
