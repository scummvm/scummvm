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

#include "mohawk/livingbooks.h"
#include "mohawk/file.h"

namespace Mohawk {

MohawkEngine_LivingBooks::MohawkEngine_LivingBooks(OSystem *syst, const MohawkGameDescription *gamedesc) : MohawkEngine(syst, gamedesc) {
	_needsUpdate = false;
	_screenWidth = _screenHeight = 0;
}

MohawkEngine_LivingBooks::~MohawkEngine_LivingBooks() {
	delete _console;
	delete _gfx;
	_bookInfoFile.clear();
}

Common::Error MohawkEngine_LivingBooks::run() {
	MohawkEngine::run();

	_console = new LivingBooksConsole(this);
	_gfx = new LBGraphics(this);

	// Load the book info from the detected file
	loadBookInfo(getBookInfoFileName());

	if (!_title.empty()) // Some games don't have the title stored
		debug("Starting Living Books Title \'%s\'", _title.c_str());
	if (!_copyright.empty())
		debug("Copyright: %s", _copyright.c_str());

	if (!_screenWidth || !_screenHeight)
		error("Could not find xRes/yRes variables");

	debug("Setting screen size to %dx%d", _screenWidth, _screenHeight);

	// TODO: Eventually move this to a LivingBooksGraphics class or similar
	initGraphics(_screenWidth, _screenHeight, true, NULL);

	loadIntro();

	debug(1, "Stack Version: %d", getResourceVersion());
	
	_gfx->setPalette(1000);
	loadSHP(1000);
	loadANI(1000);

	// Code to Load Sounds For Debugging...
	//for (byte i = 0; i < 30; i++)
	//	_sound->playSound(1000+i);

	Common::Event event;
	while (!shouldQuit()) {
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				break;
			case Common::EVENT_LBUTTONUP:
				break;
			case Common::EVENT_LBUTTONDOWN:
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_d:
					if (event.kbd.flags & Common::KBD_CTRL) {
						_console->attach();
						_console->onFrame();
					}
					break;
				case Common::KEYCODE_SPACE:
					pauseGame();
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		if (_needsUpdate) {
			_system->updateScreen();
			_needsUpdate = false;
		}

		// Cut down on CPU usage
		_system->delayMillis(10);
	}

	return Common::kNoError;
}

void MohawkEngine_LivingBooks::loadBookInfo(Common::String filename) {
	if (!_bookInfoFile.loadFromFile(filename))
		error("Could not open %s as a config file", filename.c_str());

	_title = getStringFromConfig("BookInfo", "title");
	_copyright = getStringFromConfig("BookInfo", "copyright");

	_numPages = getIntFromConfig("BookInfo", "nPages");
	_numLanguages = getIntFromConfig("BookInfo", "nLanguages");
	_screenWidth = getIntFromConfig("BookInfo", "xRes");
	_screenHeight = getIntFromConfig("BookInfo", "yRes");
	// nColors is here too, but it's always 256 anyway...
	
	// The later Living Books games add some more options:
	//     - fNeedPalette                (always true?)
	//     - fUse254ColorPalette         (always true?)
	//     - nKBRequired                 (4096, RAM requirement?)
	//     - fDebugWindow                (always 0?)
}

void MohawkEngine_LivingBooks::loadIntro() {
	Common::String filename;

	// We get to try for a few different names! Yay!
	filename = getFileNameFromConfig("Intro", "Page1");

	// Some store with .r, not sure why.
	if (filename.empty())
		filename = getFileNameFromConfig("Intro", "Page1.r");

	if (!filename.empty() && Common::File::exists(filename)) {
		MohawkFile *introFile = createMohawkFile();
		introFile->open(filename);
		_mhk.push_back(introFile);
	}

	filename = getFileNameFromConfig("Intro", "Page2");
	
	if (filename.empty())
		filename = getFileNameFromConfig("Intro", "Page2.r");
	
	if (!filename.empty() && Common::File::exists(filename)) {
		MohawkFile *coverFile = createMohawkFile();
		coverFile->open(filename);
		_mhk.push_back(coverFile);
	}
}

// Only 1 VSRN resource per stack, Id 1000
uint16 MohawkEngine_LivingBooks::getResourceVersion() {
	Common::SeekableReadStream *versionStream = getRawData(ID_VRSN, 1000);

	if (versionStream->size() != 2)
		warning("Version Record size mismatch");

	uint16 version = versionStream->readUint16BE();

	delete versionStream;
	return version;
}

// Multiple SHP# resource per stack.. Optional per Card?
// This record appears to be a list structure of BMAP resource Ids..
void MohawkEngine_LivingBooks::loadSHP(uint16 resourceId) {
	Common::SeekableSubReadStreamEndian *shpStream = wrapStreamEndian(ID_SHP, resourceId);

	if (shpStream->size() < 6)
		warning("SHP Record size too short");

	if (shpStream->readUint16() != 3)
		warning("SHP Record u0 not 3");

	if (shpStream->readUint16() != 0)
		warning("SHP Record u1 not 0");

	uint16 idCount = shpStream->readUint16();
	debug(1, "SHP: idCount: %d", idCount);

	if (shpStream->size() != (idCount * 2) + 6)
		warning("SHP Record size mismatch");

	uint16 *idValues = new uint16[idCount];
	for (uint16 i = 0; i < idCount; i++) {
		idValues[i] = shpStream->readUint16();
		debug(1, "SHP: BMAP Resource Id %d: %d", i, idValues[i]);
	}

	delete[] idValues;
	delete shpStream;
}

// Multiple ANI resource per stack.. Optional per Card?
void MohawkEngine_LivingBooks::loadANI(uint16 resourceId) {
	Common::SeekableSubReadStreamEndian *aniStream = wrapStreamEndian(ID_ANI, resourceId);

	if (aniStream->size() != 30)
		warning("ANI Record size mismatch");

	if (aniStream->readUint16() != 1)
		warning("ANI Record u0 not 0"); // Version?

	uint16 u1 = aniStream->readUint16();
	debug(1, "ANI u1: %d", u1);

	uint16 u2 = aniStream->readUint16();
	debug(1, "ANI u2: %d", u2);

	Common::Rect u3;
	u3.right = aniStream->readUint16();
	u3.bottom = aniStream->readUint16();
	u3.left = aniStream->readUint16();
	u3.top = aniStream->readUint16();
	debug(1, "ANI u3: (%d, %d), (%d, %d)", u3.left, u3.top, u3.right, u3.bottom);

	Common::Rect u4;
	u4.right = aniStream->readUint16();
	u4.bottom = aniStream->readUint16();
	u4.left = aniStream->readUint16();
	u4.top = aniStream->readUint16();
	debug(1, "ANI u4: (%d, %d), (%d, %d)", u4.left, u4.top, u4.right, u4.bottom);

	// BMAP Id?
	uint16 u4ResourceId = aniStream->readUint16();
	debug(1, "ANI u4ResourceId: %d", u4ResourceId);

	// Following 3 unknowns also resourceIds in Unused?
	uint16 u5 = aniStream->readUint16();
	debug(1, "ANI u5: %d", u5);
	if (u5 != 0)
		warning("ANI u5 non-zero");

	uint16 u6 = aniStream->readUint16();
	debug(1, "ANI u6: %d", u6);
	if (u6 != 0)
		warning("ANI u6 non-zero");

	uint16 u7 = aniStream->readUint16();
	debug(1, "ANI u7: %d", u7);
	if (u7 != 0)
		warning("ANI u7 non-zero");

	delete aniStream;
}

Common::SeekableSubReadStreamEndian *MohawkEngine_LivingBooks::wrapStreamEndian(uint32 tag, uint16 id) {
	Common::SeekableReadStream *dataStream = getRawData(tag, id);
	return new Common::SeekableSubReadStreamEndian(dataStream, 0, dataStream->size(), isBigEndian(), Common::DisposeAfterUse::YES);
}

Common::String MohawkEngine_LivingBooks::getStringFromConfig(Common::String section, Common::String key) {
	Common::String x = Common::String::emptyString;
	_bookInfoFile.getKey(key, section, x);
	return removeQuotesFromString(x);
}

int MohawkEngine_LivingBooks::getIntFromConfig(Common::String section, Common::String key) {
	return atoi(getStringFromConfig(section, key).c_str());
}

Common::String MohawkEngine_LivingBooks::getFileNameFromConfig(Common::String section, Common::String key) {
	Common::String x = getStringFromConfig(section, key);
	return (getPlatform() == Common::kPlatformMacintosh) ? convertMacFileName(x) : convertWinFileName(x);
}

Common::String MohawkEngine_LivingBooks::removeQuotesFromString(Common::String string) {
	// The last char isn't necessarily a quote, the line could have "fade" in it,
	// most likely representing to fade to that page. Hopefully it really is that
	// obvious :P

	// Some versions wrap in quotations, some don't...
	for (uint32 i = 0; i < string.size(); i++) {
		if (string[i] == '\"') {
			string.deleteChar(i);
			i--;
		}
	}

	return string;
}

Common::String MohawkEngine_LivingBooks::convertMacFileName(Common::String string) {
	Common::String filename;

	for (uint32 i = 1; i < string.size(); i++) { // First character should be ignored (another colon)
		if (string[i] == ':')
			filename += '/';
		else
			filename += string[i];
	}

	return filename;
}

Common::String MohawkEngine_LivingBooks::convertWinFileName(Common::String string) {
	Common::String filename;

	for (uint32 i = 0; i < string.size(); i++) {
		if (string[i] == '\\')
			filename += '/';
		else
			filename += string[i];
	}

	return filename;
}

} // End of namespace Mohawk
