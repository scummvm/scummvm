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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/scummsys.h"
 
#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"

#include "graphics/font.h"
 
#include "draci/draci.h"
#include "draci/barchive.h"
#include "draci/gpldisasm.h"
#include "draci/font.h"

namespace Draci {

DraciEngine::DraciEngine(OSystem *syst, const ADGameDescription *gameDesc) 
 : Engine(syst) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().
 
	// Do not initialize graphics here
 
	// However this is the place to specify all default directories
	//Common::File::addDefaultDirectory(_gameDataPath + "sound/");
 
	// Here is the right place to set up the engine specific debug levels
	Common::addDebugChannel(kDraciGeneralDebugLevel, "general", "Draci general debug level");
	Common::addDebugChannel(kDraciBytecodeDebugLevel, "bytecode", "GPL bytecode instructions");
	Common::addDebugChannel(kDraciArchiverDebugLevel, "archiver", "BAR archiver debug info");
 
	// Don't forget to register your random source
	syst->getEventManager()->registerRandomSource(_rnd, "draci");
}

int DraciEngine::init() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);

	// Basic archive test
	debugC(2, kDraciGeneralDebugLevel, "Running archive tests...");	
	Common::String path("INIT.DFW");	
	BArchive ar(path);
	BAFile *f;
	debugC(3, kDraciGeneralDebugLevel, "Number of file streams in archive: %d", ar.size());	
	f = ar[0];	
	debugC(3, kDraciGeneralDebugLevel, "First 10 bytes of file %d: ", 0);
	for (unsigned int i = 0; i < 10; ++i) {
		debugC(3, kDraciGeneralDebugLevel, "0x%02x%c", f->_data[i], (i < 9) ? ' ' : '\n');
	}

	// Read in GPL script for the first game location	
	debugC(2, kDraciBytecodeDebugLevel, "Disassembling GPL script " 
		"for the first game location...");		
	Common::String path2("MIST.DFW");	
	ar.closeArchive();
	ar.openArchive(path2);
	f = ar[3];

	// Disassemble GPL script for the first location
	gpldisasm(f->_data, f->_length);

	return 0;
}

int DraciEngine::go() {
	debugC(1, kDraciGeneralDebugLevel, "DraciEngine::go()");
 
	// Read in a sample palette
	byte *palette = new byte[4 * 256];

	debugC(2, kDraciGeneralDebugLevel, "Running graphics/animation test...");

	Common::String path("PALETY.DFW");	
	BArchive ar(path);
	BAFile *f;

	ar.closeArchive();
	ar.openArchive(path);
	f = ar[0];
	Common::MemoryReadStream paletteReader(f->_data, f->_length);
	
	for (unsigned int i = 0; i < 256; ++i) {
		palette[i * 4] = paletteReader.readByte();
		palette[i * 4 + 1] = paletteReader.readByte();
		palette[i * 4 + 2] = paletteReader.readByte();
		palette[i * 4 + 3] = 0;
	}

	// Shift the palette one bit to the left to make it brighter
	for (unsigned int i = 0; i < 4 * 256; ++i) {
		palette[i] <<= 2;
	} 

	_system->setPalette(palette, 0, 256);
	
	// Fill screen with white
	_system->fillScreen(255);

	// Draw big string
	path = "Big.fon";	
	DraciFont font(path);
	Common::String testString = "Testing, testing, read all about it!";
	Graphics::Surface *surf = _system->lockScreen();
	font.drawString(surf, testString, 
		(320 - font.getStringWidth(testString, 1)) / 2, 130, 1);

	// Draw small string
	path = "Small.fon";
	font.setFont(path);
	testString = "I'm smaller than the font above me";
	font.drawString(surf, testString, 
		(320 - font.getStringWidth(testString, 1)) / 2, 150, 1);
	_system->unlockScreen();

	_system->updateScreen();

	// Draw and animate the dragon
	path = "OBR_AN.DFW";
	ar.closeArchive();
	ar.openArchive(path);

	for (unsigned int t = 0; t < 25; ++t) {
		debugC(5, kDraciGeneralDebugLevel, "Drawing frame %d...", t);

		// Load frame to memory
		f = ar[t];
		Common::MemoryReadStream reader(f->_data, f->_length);
	
		// Read in frame width and height
		uint16 w = reader.readUint16LE();
		uint16 h = reader.readUint16LE();
		
		// Allocate frame memory
		byte *scr = new byte[w * h];

		// Draw frame
		for (uint16 i = 0; i < w; ++i) {
			for (uint16 j = 0; j < h; ++j) {
				scr[j * w + i] = reader.readByte();
			}
		}
		_system->copyRectToScreen(scr, w, (320 - w) / 2, 60, w, h);
		_system->updateScreen();
		_system->delayMillis(100);

		debugC(5, kDraciGeneralDebugLevel, "Finished frame %d", t);	

		// Free frame memory
		delete[] scr;
	}	

	getchar();

	return 0;
}

DraciEngine::~DraciEngine() {
	// Dispose your resources here
 
	// Remove all of our debug levels here
	Common::clearAllDebugChannels();
}

Common::Error DraciEngine::run() {
	init();
	go();
	return Common::kNoError;
}

} // End of namespace Draci
