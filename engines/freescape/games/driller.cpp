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
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/random.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

enum {
	kDrillerCGAPalettePinkBlue = 0,
	kDrillerCGAPaletteRedGreen = 1,
};

static const struct CGAPalettteEntry {
	int areaId;
	int palette;
} rawCGAPaletteTable[] {
	{1, kDrillerCGAPaletteRedGreen},
	{2, kDrillerCGAPalettePinkBlue},
	{3, kDrillerCGAPaletteRedGreen},
	{4, kDrillerCGAPalettePinkBlue},
	{5, kDrillerCGAPaletteRedGreen},
	{6, kDrillerCGAPalettePinkBlue},
	{7, kDrillerCGAPaletteRedGreen},
	{8, kDrillerCGAPalettePinkBlue},
	{9, kDrillerCGAPaletteRedGreen},
	{10, kDrillerCGAPalettePinkBlue},
	{11, kDrillerCGAPaletteRedGreen},
	{12, kDrillerCGAPalettePinkBlue},

	{14, kDrillerCGAPalettePinkBlue},

	{16, kDrillerCGAPalettePinkBlue},

	{19, kDrillerCGAPaletteRedGreen},
	{20, kDrillerCGAPalettePinkBlue},
	{21, kDrillerCGAPaletteRedGreen},
	{22, kDrillerCGAPalettePinkBlue},
	{23, kDrillerCGAPaletteRedGreen},

	{28, kDrillerCGAPalettePinkBlue},

	{32, kDrillerCGAPalettePinkBlue},
	{127, kDrillerCGAPaletteRedGreen},
	{0, 0}   // This marks the end
};

byte kDrillerCGAPalettePinkBlueData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0xaa, 0xaa},
	{0xaa, 0x00, 0xaa},
	{0xaa, 0xaa, 0xaa},
};

byte kDrillerCGAPaletteRedGreenData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0xaa, 0x00},
	{0xaa, 0x00, 0x00},
	{0xaa, 0x55, 0x00},
};

enum {
	kDrillerNoRig = 0,
	kDrillerRigInPlace = 1,
	kDrillerRigOutOfPlace = 2,
	kDrillerRigNoGas = 3
};

DrillerEngine::DrillerEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {

	if (!Common::parseBool(ConfMan.get("automatic_drilling"), _useAutomaticDrilling))
		error("Failed to parse bool from automatic_drilling option");

	if (isDOS()) {
		if (_renderMode == Common::kRenderEGA)
			_viewArea = Common::Rect(40, 16, 280, 117);
		else if (_renderMode == Common::kRenderCGA)
			_viewArea = Common::Rect(36, 16, 284, 117);
		else
			error("Invalid or unknown render mode");
	}
	else if (isAmiga() || isAtariST())
		_viewArea = Common::Rect(36, 16, 284, 118);
	else if (isSpectrum())
		_viewArea = Common::Rect(58, 20, 266, 124);
	else if (isCPC())
		_viewArea = Common::Rect(36, 19, 284, 120);
	else if (isC64())
		_viewArea = Common::Rect(32, 16, 288, 119);

	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);
	_playerHeights.push_back(80);
	_playerHeights.push_back(112);

	_angleRotations.push_back(5);
	_angleRotations.push_back(10);
	_angleRotations.push_back(15);
	_angleRotations.push_back(30);
	_angleRotations.push_back(45);
	_angleRotations.push_back(90);

	_playerHeight = _playerHeights[_playerHeightNumber];
	_playerWidth = 12;
	_playerDepth = 32;

	_initialTankEnergy = 48;
	_initialTankShield = 50;
	_initialJetEnergy = 29;
	_initialJetShield = 34;

	Math::Vector3d drillBaseOrigin = Math::Vector3d(0, 0, 0);
	Math::Vector3d drillBaseSize = Math::Vector3d(3, 2, 3);
	_drillBase = new GeometricObject(kCubeType, 0, 0, drillBaseOrigin, drillBaseSize, nullptr, nullptr, FCLInstructionVector(), "");
	assert(!_drillBase->isDestroyed() && !_drillBase->isInvisible());
}

DrillerEngine::~DrillerEngine() {
	delete _drillBase;
}

void DrillerEngine::titleScreen() {
	if (isDOS() && isDemo()) // Demo will not show any title screen
		return;

	if (isAmiga() || isAtariST()) // These releases has their own screens
		return;

	if (_title) {
		drawTitle();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(3000);
	}
}
void DrillerEngine::borderScreen() {
	if (isDOS() && isDemo()) // Demo will not show the border
		return;

	if (isAmiga() || isAtariST()) // These releases has their own screens
		return;

	if (_border) {
		drawBorder();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(3000);
	}
}

void DrillerEngine::gotoArea(uint16 areaID, int entranceID) {
	int prevAreaID = _currentArea ? _currentArea->getAreaID(): -1;
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	if (!_areaMap.contains(areaID)) {
		assert(isDOS() && isDemo());
		// Not included in the demo, abort area change
		return;
	}
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	if (entranceID > 0 || areaID == 127) {
		traverseEntrance(entranceID);
	} else if (entranceID == 0) {
		int newPos = -1;
		// FIX: The next check will abort changing the current another
		// area if the player is too close to the corners
		if ((_position.z() < 100 && _position.x() > 3900) ||
			(_position.z() > 3900 && _position.x() < 100) ||
			(_position.z() < 100 && _position.x()  < 100) ||
			(_position.z() > 3900 && _position.x() > 3900)) {
				assert(prevAreaID > 0);
				_currentArea = _areaMap[prevAreaID];
				return;
		}
		if (_position.z() < 200 || _position.z() >= 3800) {
			if (_position.z() < 200)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(2, newPos);
		} else if(_position.x() < 200 || _position.x() >= 3800)  {
			if (_position.x() < 200)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(0, newPos);
		} else
			error("Invalid movement across areas");
		assert(newPos != -1);
		_sensors = _currentArea->getSensors();
	}
	_lastPosition = _position;
	_gameStateVars[0x1f] = 0;

	if (areaID == _startArea && entranceID == _startEntrance) {
		_yaw = 280;
		_pitch = 0;
	} else if (areaID == 127) {
		assert(entranceID == 0);
		_yaw = 90;
		_pitch = 335;
		_flyMode = true; // Avoid falling
		// Show the number of completed areas
		_areaMap[127]->_name.replace(0, 3, Common::String::format("%4d", _gameStateVars[32]));
	}

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	clearTemporalMessages();
	playSound(5, false);
	// Ignore sky/ground fields
	_gfx->_keyColor = 0;
	_gfx->setColorRemaps(&_currentArea->_colorRemaps);

	swapPalette(areaID);
	_currentArea->_skyColor = 0;
	_currentArea->_usualBackgroundColor = 0;

	resetInput();
}

void DrillerEngine::loadGlobalObjects(Common::SeekableReadStream *file, int offset) {
	assert(!_areaMap.contains(255));
	ObjectMap *globalObjectsByID = new ObjectMap;
	file->seek(offset);
	for (int i = 0; i < 8; i++) {
		if (isDOS() && isDemo()) // The DOS demo has a few missing objects
			if (i == 5)
				break;

		Object *gobj = load8bitObject(file);
		assert(gobj);
		assert(!globalObjectsByID->contains(gobj->getObjectID()));
		debugC(1, kFreescapeDebugParser, "Adding global object: %d", gobj->getObjectID());
		(*globalObjectsByID)[gobj->getObjectID()] = gobj;
	}

	_areaMap[255] = new Area(255, 0, globalObjectsByID, nullptr);
}

void DrillerEngine::loadAssets() {
	if (isDemo())
		loadAssetsDemo();
	else
		loadAssetsFullGame();

	// Start playing music, if any, in any supported format
	playMusic("Matt Gray - The Best Of Reformation - 07 Driller Theme");
}

void DrillerEngine::loadAssetsDemo() {
	Common::File file;
	if (isAmiga()) {
		file.open("lift.neo");
		if (!file.isOpen())
			error("Failed to open 'lift.neo' file");

		_title = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("console.neo");
		if (!file.isOpen())
			error("Failed to open 'console.neo' file");

		_border = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("demo.cmd");
		if (!file.isOpen())
			error("Failed to open 'demo.cmd' file");

		loadDemoData(&file, 0, 0x1000);

		file.close();
		file.open("data");
		if (!file.isOpen())
			error("Failed to open 'data' file");

		load8bitBinary(&file, 0x442, 16);
		loadPalettes(&file, 0x0);

		file.close();
		file.open("driller");
		if (!file.isOpen())
			error("Failed to open 'driller' file");

		loadFonts(&file, 0xa30);
		loadMessagesFixedSize(&file, 0x3960, 14, 20);
		loadGlobalObjects(&file, 0x3716);

		file.close();
		file.open("soundfx");
		if (!file.isOpen())
			error("Failed to open 'soundfx' executable for Amiga");

		loadSoundsFx(&file, 0, 25);
	} else if (isAtariST()) {
		file.open("lift.neo");
		if (!file.isOpen())
			error("Failed to open 'lift.neo' file");

		_title = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("console.neo");
		if (!file.isOpen())
			error("Failed to open 'console.neo' file");

		_border = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("demo.cmd");
		if (!file.isOpen())
			error("Failed to open 'demo.cmd' file");

		loadDemoData(&file, 0, 0x1000);

		file.close();
		file.open("data");

		if (!file.isOpen())
			error("Failed to open 'data' file");

		load8bitBinary(&file, 0x442, 16);
		loadPalettes(&file, 0x0);

		file.close();
		file.open("x.prg");
		if (!file.isOpen())
			error("Failed to open 'x.prg' file");

		loadFonts(&file, 0x7bc);
		loadMessagesFixedSize(&file, 0x3b90, 14, 20);
		loadGlobalObjects(&file, 0x3946);

		file.close();
		file.open("soundfx");
		if (!file.isOpen())
			error("Failed to open 'soundfx' executable for AtariST demo");

		loadSoundsFx(&file, 0, 25);
	} else if (isDOS()) {
		_renderMode = Common::kRenderCGA; // DOS demos is CGA only
		_viewArea = Common::Rect(36, 16, 284, 117); // correct view area
		_gfx->_renderMode = _renderMode;
		loadBundledImages();
		file.open("d2");
		if (!file.isOpen())
			error("Failed to open 'd2' file");

		loadFonts(&file, 0x4eb0);
		loadMessagesFixedSize(&file, 0x636, 14, 20);
		load8bitBinary(&file, 0x55b0, 4);
		loadGlobalObjects(&file, 0x8c);

		// Fixed for a corrupted area names in the demo data
		_areaMap[2]->_name = "LAPIS LAZULI";
		_areaMap[3]->_name = "EMERALD";
		_areaMap[8]->_name = "TOPAZ";
		file.close();
	} else
		error("Unsupported demo for Driller");

	_demoMode = !_disableDemoMode;
	_angleRotationIndex = 0;
}


byte *parseEDSK(const Common::String filename, int &size) {
	debugC(1, kFreescapeDebugParser, "Trying to parse edsk file: %s", filename.c_str());
	Common::File file;
	file.open(filename);
	if (!file.isOpen())
		error("Failed to open %s", filename.c_str());

	int totalSize = file.size();
	byte *edskBuffer = (byte *)malloc(totalSize);
	file.read(edskBuffer, totalSize);
	file.close();

	// We don't know the final size, but we allocate enough
	byte *memBuffer = (byte *)malloc(totalSize);

	byte nsides = edskBuffer[49];
	assert(nsides == 1);
	int ntracks = 0;
	int i = 256;
	int j = 0;
	while (i + 1 < totalSize) {
		byte ssize = edskBuffer[i + 0x14];
		debug("i: %x ssize: %d, number: %d", i, ssize, edskBuffer[i + 0x10]);
		assert(ssize == 3 || edskBuffer[i + 0x0] == 'T');
		assert(ssize == 3 || edskBuffer[i + 0x1] == 'r');
		assert(ssize == 3 || edskBuffer[i + 0x2] == 'a');
		//assert(ssize == 3 || ntracks == edskBuffer[i + 0x10]);
		int start = i + 0x100;
		debugC(1, kFreescapeDebugParser, "sector size: %d", ssize);
		if (ssize == 2) {
			i = i + 9 * 512 + 256;
		} else if (ssize == 5) {
			i = i + 8 * 512 + 256;
		} else if (ssize == 0) {
			i = totalSize - 1;
		} else if (ssize == 3) {
			break; // Not sure
		} else {
			error("ssize: %d", ssize);
		}
		int osize = i - start;
		debugC(1, kFreescapeDebugParser, "copying track %d start: %x size: %x, dest: %x", ntracks, start, osize, j);
		memcpy(memBuffer + j, edskBuffer + start, osize);
		j = j + osize;
		ntracks++;
	}
	size = j;

	if (0) { // Useful to debug where exactly each object is located in memory once it is parsed
		i = 0;
		while(i < j) {
			debugN("%05x: ", i);
			for (int k = 0; k <= 16; k++) {
				debugN("%02x ", memBuffer[i]);
				i++;
			}
			debugN("\n");
		}
	}
	free(edskBuffer);
	return memBuffer;
}


void DrillerEngine::loadAssetsFullGame() {
	Common::File file;
	if (isAmiga()) {
		if (_variant & GF_AMIGA_RETAIL) {
			file.open("driller");

			if (!file.isOpen())
				error("Failed to open 'driller' executable for Amiga");

			_border = loadAndConvertNeoImage(&file, 0x137f4);
			byte *palette = (byte *)malloc(16 * 3);
			for (int i = 0; i < 16; i++) { // gray scale palette
				palette[i * 3 + 0] = i * (255 / 16);
				palette[i * 3 + 1] = i * (255 / 16);
				palette[i * 3 + 2] = i * (255 / 16);
			}
			_title = loadAndConvertNeoImage(&file, 0x10, palette);

			loadFonts(&file, 0x8940);
			loadMessagesFixedSize(&file, 0xc66e, 14, 20);
			loadGlobalObjects(&file, 0xbd62);
			load8bitBinary(&file, 0x29c16, 16);
			loadPalettes(&file, 0x297d4);
			loadSoundsFx(&file, 0x30e80, 25);
		} else if (_variant & GF_AMIGA_BUDGET) {
			file.open("lift.neo");
			if (!file.isOpen())
				error("Failed to open 'lift.neo' file");

			_title = loadAndConvertNeoImage(&file, 0);

			file.close();
			file.open("console.neo");
			if (!file.isOpen())
				error("Failed to open 'console.neo' file");

			_border = loadAndConvertNeoImage(&file, 0);

			file.close();
			file.open("driller");
			if (!file.isOpen())
				error("Failed to open 'driller' executable for Amiga");

			loadFonts(&file, 0xa62);
			loadMessagesFixedSize(&file, 0x499a, 14, 20);
			loadGlobalObjects(&file, 0x4098);
			load8bitBinary(&file, 0x21a3e, 16);
			loadPalettes(&file, 0x215fc);

			file.close();
			file.open("soundfx");
			if (!file.isOpen())
				error("Failed to open 'soundfx' executable for Amiga");

			loadSoundsFx(&file, 0, 25);
		}
		else
			error("Invalid or unknown Amiga release");
	} else if (isAtariST()) {
		file.open("x.prg");

		if (!file.isOpen())
			error("Failed to open 'x.prg' executable for AtariST");

		_border = loadAndConvertNeoImage(&file, 0x1371a);
		byte *palette = (byte *)malloc(16 * 3);
		for (int i = 0; i < 16; i++) { // gray scale palette
			palette[i * 3 + 0] = i * (255 / 16);
			palette[i * 3 + 1] = i * (255 / 16);
			palette[i * 3 + 2] = i * (255 / 16);
		}
		_title = loadAndConvertNeoImage(&file, 0x10, palette);

		loadFonts(&file, 0x8a32);
		loadMessagesFixedSize(&file, 0xc5d8, 14, 20);
		loadGlobalObjects(&file, 0xbccc);
		load8bitBinary(&file, 0x29b3c, 16);
		loadPalettes(&file, 0x296fa);
		loadSoundsFx(&file, 0x30da6, 25);
	} else if (isSpectrum()) {
		loadBundledImages();
		file.open("driller.zx.extracted");

		if (!file.isOpen())
			error("Failed to open driller.zx.extracted");

		if (_variant & GF_ZX_DISC)
			loadMessagesFixedSize(&file, 0x2164, 14, 20);
		else
			loadMessagesFixedSize(&file, 0x20e4, 14, 20);

		if (_variant & GF_ZX_RETAIL)
			loadFonts(&file, 0x62ca);
		else if (_variant & GF_ZX_BUDGET)
			loadFonts(&file, 0x5aa8);
		else if (_variant & GF_ZX_DISC)
			loadFonts(&file, 0x63f0);

		if (_variant & GF_ZX_DISC)
			loadGlobalObjects(&file, 0x1d13);
		else
			loadGlobalObjects(&file, 0x1c93);

		if (_variant & GF_ZX_RETAIL)
			load8bitBinary(&file, 0x642c, 4);
		else if (_variant & GF_ZX_BUDGET)
			load8bitBinary(&file, 0x5c0a, 4);
		else if (_variant & GF_ZX_DISC)
			load8bitBinary(&file, 0x6552, 4);

		else
			error("Unknown ZX spectrum variant");
	} else if (isCPC()) {
		loadBundledImages();
		byte *memBuffer;
		int memSize = 0;
		if (_variant & GF_CPC_VIRTUALWORLDS) {
			memBuffer = parseEDSK("virtualworlds.A.cpc.edsk", memSize);

			// Deofuscation / loader code
			for (int j = 0; j < 0x200; j++) {
				memBuffer[0x14000 + j] = memBuffer[0x14200 + j];
				memBuffer[0x14200 + j] = memBuffer[0x13400 + j];
				memBuffer[0x14400 + j] = memBuffer[0x13800 + j];
				memBuffer[0x14600 + j] = memBuffer[0x13c00 + j];
			}

			for (int j = 0; j < 0x200; j++) {
				memBuffer[0x13c00 + j] = memBuffer[0x13a00 + j];
				memBuffer[0x13a00 + j] = memBuffer[0x13600 + j];
				memBuffer[0x13800 + j] = memBuffer[0x13200 + j];
				memBuffer[0x13600 + j] = memBuffer[0x12e00 + j];
				memBuffer[0x12e00 + j] = memBuffer[0x13000 + j];
				memBuffer[0x13000 + j] = memBuffer[0x12200 + j];
				memBuffer[0x13200 + j] = memBuffer[0x12600 + j];
				memBuffer[0x13400 + j] = memBuffer[0x12a00 + j];
			}

			for (int i = 6; i >= 0; i--) {
				//debug("copying 0x200 bytes to %x from %x", 0x12000 + 0x200*i, 0x11400 + 0x400*i);
				for (int j = 0; j < 0x200; j++) {
					memBuffer[0x12000 + 0x200*i + j] = memBuffer[0x11400 + 0x400*i + j];
				}
			}

			for (int j = 0; j < 0x200; j++) {
				memBuffer[0x11c00 + j] = memBuffer[0x11e00 + j];
				memBuffer[0x11e00 + j] = memBuffer[0x11000 + j];
			}
		} else
			memBuffer = parseEDSK("driller.cpc.edsk", memSize);
		assert(memSize > 0);
		Common::SeekableReadStream *stream = new Common::MemoryReadStream((const byte*)memBuffer, memSize);

		if (_variant & GF_CPC_RETAIL) {
			loadMessagesFixedSize(stream, 0xb0f7, 14, 20);
			loadFonts(stream, 0xeb14);
			load8bitBinary(stream, 0xec76, 4);
			loadGlobalObjects(stream, 0xacb2);
		} else if (_variant & GF_CPC_RETAIL2) {
			loadMessagesFixedSize(stream, 0xb0f7 - 0x3fab, 14, 20);
			loadFonts(stream, 0xeb14 - 0x3fab);
			load8bitBinary(stream, 0xaccb, 4);
			loadGlobalObjects(stream, 0xacb2 - 0x3fab);
		} else if (_variant & _variant & GF_CPC_VIRTUALWORLDS) {
			load8bitBinary(stream, 0x11acb, 4);
		} else if (_variant & GF_CPC_BUDGET) {
			loadMessagesFixedSize(stream, 0x9ef7, 14, 20);
			loadFonts(stream, 0xd914);
			load8bitBinary(stream, 0xda76, 4);
			loadGlobalObjects(stream, 0x9ab2);
		} else
			error("Unknown Amstrad CPC variant");
	} else if (isC64()) {
		if (_targetName.hasPrefix("spacestationoblivion")) {
			loadBundledImages();
			file.open("spacestationoblivion.c64.extracted");
			loadMessagesFixedSize(&file, 0x167a, 14, 20);
			//loadFonts(&file, 0xae54);
			load8bitBinary(&file, 0x8e02, 4);
			loadGlobalObjects(&file, 0x1855);
		} else if (_targetName.hasPrefix("driller")) {
			file.open("driller.c64.extracted");
			loadMessagesFixedSize(&file, 0x167a - 0x400, 14, 20);
			//loadFonts(&file, 0xae54);
			load8bitBinary(&file, 0x8e02 - 0x400, 4);
			loadGlobalObjects(&file, 0x1855 - 0x400);
		}
	} else if (_renderMode == Common::kRenderEGA) {
		loadBundledImages();
		file.open("DRILLE.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLE.EXE");

		loadMessagesFixedSize(&file, 0x4135, 14, 20);
		loadFonts(&file, 0x99dd);
		loadGlobalObjects(&file, 0x3b42);
		load8bitBinary(&file, 0x9b40, 16);

	} else if (_renderMode == Common::kRenderCGA) {
		loadBundledImages();
		_title->free();
		delete _title;
		file.open("CGATITLE.RL");
		_title = load8bitTitleImage(&file, 4, 0x1b3);

		file.close();
		file.open("DRILLC.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLC.EXE");

		loadFonts(&file, 0x07a4a);
		loadMessagesFixedSize(&file, 0x2585, 14, 20);
		load8bitBinary(&file, 0x7bb0, 4);
		loadGlobalObjects(&file, 0x1fa2);
		_border = load8bitBinImage(&file, 4, 0x210);
	} else
		error("Invalid or unsupported render mode %s for Driller", Common::getRenderModeDescription(_renderMode));

	/*
	We are going to inject a small script in the
	last area to force the game to end:
	IF COLLIDED? THEN
	IF VAR!=? (v32, 18) THEN END ENDIF
	GOTO (127, 0)
	*/

	FCLInstructionVector instructions;
	Common::Array<uint8> conditionArray;

	conditionArray.push_back(0xb);
	conditionArray.push_back(0x20);
	conditionArray.push_back(0x12);
	conditionArray.push_back(0x12);
	conditionArray.push_back(0x7f);
	conditionArray.push_back(0x0);

	Common::String conditionSource = detokenise8bitCondition(conditionArray, instructions);
	debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
	_areaMap[18]->_conditions.push_back(instructions);
	_areaMap[18]->_conditionSources.push_back(conditionSource);
}

void DrillerEngine::processBorder() {
	FreescapeEngine::processBorder();
	if (isDOS() && _renderMode == Common::kRenderCGA) { // Replace some colors for the CGA borders
		uint32 color1 = _border->format.ARGBToColor(0xFF, 0xAA, 0x00, 0xAA);
		uint32 color2 = _border->format.ARGBToColor(0xFF, 0xAA, 0x55, 0x00);

		uint32 colorA = _border->format.ARGBToColor(0xFF, 0x00, 0xAA, 0xAA);
		uint32 colorB = _border->format.ARGBToColor(0xFF, 0x00, 0xAA, 0x00);

		uint32 colorX = _border->format.ARGBToColor(0xFF, 0xAA, 0xAA, 0xAA);
		uint32 colorY = _border->format.ARGBToColor(0xFF, 0xAA, 0x00, 0x00);

		Graphics::Surface *borderRedGreen = new Graphics::Surface();
		borderRedGreen->create(1, 1, _border->format);
		borderRedGreen->copyFrom(*_border);

		for (int i = 0; i < _border->w; i++) {
			for (int j = 0; j < _border->h; j++) {
				if (borderRedGreen->getPixel(i, j) == color1)
					borderRedGreen->setPixel(i, j, color2);
				else if (borderRedGreen->getPixel(i, j) == colorA)
					borderRedGreen->setPixel(i, j, colorB);
				else if (borderRedGreen->getPixel(i, j) == colorX)
					borderRedGreen->setPixel(i, j, colorY);

			}
		}
		Texture *borderTextureRedGreen = _gfx->createTexture(borderRedGreen);

		const CGAPalettteEntry *entry = rawCGAPaletteTable;
		while (entry->areaId) {

			if (entry->palette == kDrillerCGAPaletteRedGreen) {
				_borderCGAByArea[entry->areaId] = borderTextureRedGreen;
				_paletteCGAByArea[entry->areaId] = (byte *)kDrillerCGAPaletteRedGreenData;
			} else if (entry->palette == kDrillerCGAPalettePinkBlue) {
				_borderCGAByArea[entry->areaId] = _borderTexture;
				_paletteCGAByArea[entry->areaId] = (byte *)kDrillerCGAPalettePinkBlueData;
			} else
				error("Invalid CGA palette to use");
			entry++;
		}
	}
}

void DrillerEngine::drawUI() {
	Graphics::Surface *surface = nullptr;
	if (_border) { // This can be removed when all the borders are loaded
		uint32 gray = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0xA0, 0xA0, 0xA0);
		surface = new Graphics::Surface();
		surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
		surface->fillRect(_fullscreenViewArea, gray);
		drawCrossair(surface);
	} else
		return;

	if (isDOS())
		drawDOSUI(surface);
	else if (isC64())
		drawC64UI(surface);
	else if (isSpectrum())
		drawZXUI(surface);
	else if (isCPC())
		drawCPCUI(surface);
	else if (isAmiga() || isAtariST())
		drawAmigaAtariSTUI(surface);

	if (!_uiTexture)
		_uiTexture = _gfx->createTexture(surface);
	else
		_uiTexture->update(surface);

	_gfx->setViewport(_fullscreenViewArea);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _uiTexture);
	_gfx->setViewport(_viewArea);

	surface->free();
	delete surface;
}

void DrillerEngine::drawDOSUI(Graphics::Surface *surface) {
	uint32 color = _renderMode == Common::kRenderCGA ? 1 : 14;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 196, 185, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 150, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 150, 153, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 150, 161, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 57, 161, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 57, 161, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 46, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), _renderMode == Common::kRenderCGA ? 44 : 46, 153, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 238, 129, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d", hours), 208, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", minutes), 230, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 8, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 190, 177, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 191, 177, front, back, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (energy >= 0) {
		Common::Rect backBar(20, 185, 88 - energy, 191);
		surface->fillRect(backBar, back);
		Common::Rect energyBar(87 - energy, 185, 88, 191);
		surface->fillRect(energyBar, front);
	}

	if (shield >= 0) {
		Common::Rect backBar(20, 177, 88 - shield, 183);
		surface->fillRect(backBar, back);

		Common::Rect shieldBar(87 - shield, 177, 88, 183);
		surface->fillRect(shieldBar, front);
	}
}

void DrillerEngine::drawCPCUI(Graphics::Surface *surface) {
	uint32 color = 1;
	uint8 r, g, b;

	_gfx->selectColorFromFourColorPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 0;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 200, 188, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 149, 148, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 149, 156, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 149, 164, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 54, 164, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 54, 164, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 46, 148, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 44, 156, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 239, 132, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d", hours), 209, 11, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", minutes), 232, 11, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 11, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 191, 180, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (_messagesList.size() > 0) {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 191, 180, front, back, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (energy >= 0) {
		Common::Rect backBar(25, 187, 89 - energy, 194);
		surface->fillRect(backBar, back);
		Common::Rect energyBar(88 - energy, 187, 88, 194);
		surface->fillRect(energyBar, front);
	}

	if (shield >= 0) {
		Common::Rect backBar(25, 180, 89 - shield, 186);
		surface->fillRect(backBar, back);

		Common::Rect shieldBar(88 - shield, 180, 88, 186);
		surface->fillRect(shieldBar, front);
	}
}

void DrillerEngine::drawC64UI(Graphics::Surface *surface) {
	uint32 color = 1;
	uint8 r, g, b;

	_gfx->selectColorFromFourColorPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 0;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 200, 188, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 149, 148, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 149, 156, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 149, 164, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 54, 164, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 54, 164, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 46, 148, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 44, 156, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 240, 128, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d", hours), 209, 11, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", minutes), 232, 11, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 11, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 191, 180, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 191, 180, front, back, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (energy >= 0) {
		Common::Rect backBar(25, 187, 89 - energy, 194);
		surface->fillRect(backBar, back);
		Common::Rect energyBar(88 - energy, 187, 88, 194);
		surface->fillRect(energyBar, front);
	}

	if (shield >= 0) {
		Common::Rect backBar(25, 180, 89 - shield, 186);
		surface->fillRect(backBar, back);

		Common::Rect shieldBar(88 - shield, 180, 88, 186);
		surface->fillRect(shieldBar, front);
	}
}

void DrillerEngine::drawZXUI(Graphics::Surface *surface) {
	uint32 color = 5;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 176, 188, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 152, 149, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 152, 157, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 152, 165, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 74, 165, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 74, 165, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 64, 149, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 65, 157, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 217, 133, white, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d", hours), 187, 12, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", minutes), 209, 12, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 232, 12, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 169, 181, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 169, 181, front, back, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (energy >= 0) {
		Common::Rect backBar(45, 188, 109 - energy, 194);
		surface->fillRect(backBar, back);
		Common::Rect energyBar(108 - energy, 188, 108, 194);
		surface->fillRect(energyBar, front);
	}

	if (shield >= 0) {
		Common::Rect backBar(45, 181, 109 - shield, 187);
		surface->fillRect(backBar, back);

		Common::Rect shieldBar(108 - shield, 181, 108, 187);
		surface->fillRect(shieldBar, front);
	}
}

void DrillerEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 brownish = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x9E, 0x80, 0x20);
	uint32 brown = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x7E, 0x60, 0x19);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);

	int score = _gameStateVars[k8bitVariableScore];
	Common::String coords;

	if (!isDemo()) { // It seems demos will not include the complete font?
		drawStringInSurface("x", 37, 18, white, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.x()));
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 18, white, transparent, surface, 112);

		drawStringInSurface("y", 37, 26, white, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.z())); // Coords y and z are swapped!
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 26, white, transparent, surface, 112);

		drawStringInSurface("z", 37, 34, white, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.y())); // Coords y and z are swapped!
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 34, white, transparent, surface, 112);
	}

	drawStringInSurface(_currentArea->_name, 188, 185, yellow, black, surface);
	drawStringInSurface(Common::String::format("%07d", score), 240, 129, yellow, black, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d:", hours), 208, 7, yellow, black, surface);
	drawStringInSurface(Common::String::format("%02d:", minutes), 230, 7, yellow, black, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 7, yellow, black, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 188, 177, black, yellow, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 188, 177, yellow, black, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (shield >= 0) {
		Common::Rect shieldBar;
		shieldBar = Common::Rect(11, 178, 76 - (k8bitMaxShield - shield), 184);
		surface->fillRect(shieldBar, brown);

		shieldBar = Common::Rect(11, 179, 76 - (k8bitMaxShield - shield), 183);
		surface->fillRect(shieldBar, brownish);

		shieldBar = Common::Rect(11, 180, 76 - (k8bitMaxShield - shield), 182);
		surface->fillRect(shieldBar, yellow);
	}

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(11, 186, 75 - (k8bitMaxEnergy - energy), 192);
		surface->fillRect(energyBar, brown);

		energyBar = Common::Rect(11, 187, 75 - (k8bitMaxEnergy - energy), 191);
		surface->fillRect(energyBar, brownish);

		energyBar = Common::Rect(11, 188, 75 - (k8bitMaxEnergy - energy), 190);
		surface->fillRect(energyBar, yellow);
	}
}

void DrillerEngine::drawInfoMenu() {
	_savedScreen = _gfx->getScreenshot();

	uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, color);

	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	surface->fillRect(_viewArea, black);

	switch (_renderMode) {
		case Common::kRenderCGA:
			color = 1;
			break;
		case Common::kRenderZX:
			color = 6;
			break;
		default:
			color = 14;
	}
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	drawStringInSurface(Common::String::format("%10s : %s", "sector", _currentArea->_name.c_str()), 69, 25, front, black, surface);
	Common::String rigStatus;
	Common::String gasFound;
	Common::String perTapped;
	Common::String gasTapped;

	switch (_drillStatusByArea[_currentArea->getAreaID()]) {
		case kDrillerNoRig:
			rigStatus = "Unpositioned";
			gasFound = "-";
			perTapped = "-";
			gasTapped = "-";
			break;
		case kDrillerRigInPlace:
		case kDrillerRigOutOfPlace:
			rigStatus = "Positioned";
			gasFound = Common::String::format("%d CFT", _drillMaxScoreByArea[_currentArea->getAreaID()]);
			perTapped = Common::String::format("%d %%", _drillSuccessByArea[_currentArea->getAreaID()]);
			gasTapped = Common::String::format("%d", uint32(_drillSuccessByArea[_currentArea->getAreaID()] * _drillMaxScoreByArea[_currentArea->getAreaID()]) / 100);
			break;
		case kDrillerRigNoGas:
			rigStatus = "Positioned";
			gasFound = "none";
			perTapped = "none";
			gasTapped = "zero";
			break;
		default:
			error("Invalid drill status");
			break;
	}

	drawStringInSurface(Common::String::format("%10s : %s", "rig status", rigStatus.c_str()), 69, 33, front, black, surface);
	drawStringInSurface(Common::String::format("%10s : %s", "gas found", gasFound.c_str()), 69, 41, front, black, surface);
	drawStringInSurface(Common::String::format("%10s : %s", "% tapped", perTapped.c_str()), 69, 49, front, black, surface);
	drawStringInSurface(Common::String::format("%10s : %s", "gas tapped", gasTapped.c_str()), 69, 57, front, black, surface);

	drawStringInSurface(Common::String::format("%13s : %d", "total sectors", 18), 84, 73, front, black, surface);
	drawStringInSurface(Common::String::format("%13s : %d", "safe sectors", _gameStateVars[32]), 84, 81, front, black, surface);

	if (isDOS() || isCPC()) {
		drawStringInSurface("l-load s-save esc-terminate", 53, 97, front, black, surface);
		drawStringInSurface("t-toggle sound on/off", 76, 105, front, black, surface);
	} else if (isSpectrum()) {
		drawStringInSurface("l-load s-save 1-abort", 76, 97, front, black, surface);
		drawStringInSurface("any other key-continue", 76, 105, front, black, surface);
	}

	_uiTexture->update(surface);
	_gfx->setViewport(_fullscreenViewArea);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _uiTexture);
	_gfx->setViewport(_viewArea);

	_gfx->flipBuffer();
	g_system->updateScreen();

	Common::Event event;
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (g_system->getEventManager()->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_l) {
					_gfx->setViewport(_fullscreenViewArea);
					loadGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (event.kbd.keycode == Common::KEYCODE_s) {
					_gfx->setViewport(_fullscreenViewArea);
					saveGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (isDOS() && event.kbd.keycode == Common::KEYCODE_t) {
					// TODO
				} else if ((isDOS() || isCPC()) && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_forceEndGame = true;
					cont = false;
				} else if (isSpectrum() && event.kbd.keycode == Common::KEYCODE_1) {
					_forceEndGame = true;
					cont = false;
				} else
					cont = false;
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				// TODO: properly refresh screen
				break;

			default:
				break;
			}
		}
		g_system->delayMillis(10);
	}

	_savedScreen->free();
	delete _savedScreen;
	surface->free();
	delete surface;
}

Math::Vector3d getProjectionToPlane(const Math::Vector3d &vect, const Math::Vector3d normal) {
	assert (normal.length() == 1);
	// Formula: return p - n * (n . p)
	Math::Vector3d result = vect;
	result -= normal * normal.dotProduct(vect);
	return result;
}

void DrillerEngine::pressedKey(const int keycode) {
	if (keycode == Common::KEYCODE_d) {
		if (isDOS() && isDemo()) // No support for drilling here yet
			return;
		clearTemporalMessages();
		Common::Point gasPocket = _currentArea->_gasPocketPosition;
		uint32 gasPocketRadius = _currentArea->_gasPocketRadius;
		if (gasPocketRadius == 0) {
			insertTemporaryMessage(_messagesList[2], _countdown - 2);
			return;
		}

		if (_flyMode) {
			insertTemporaryMessage(_messagesList[8], _countdown - 2);
			return;
		}

		if (drillDeployed(_currentArea)) {
			insertTemporaryMessage(_messagesList[12], _countdown - 2);
			return;
		}

		if (_gameStateVars[k8bitVariableEnergy] < 5) {
			insertTemporaryMessage(_messagesList[7], _countdown - 2);
			return;
		}

		Math::Vector3d drill = drillPosition();
		debugC(1, kFreescapeDebugMove, "Current position at %f %f %f", _position.x(), _position.y(), _position.z());
		debugC(1, kFreescapeDebugMove, "Trying to adding drill at %f %f %f", drill.x(), drill.y(), drill.z());
		debugC(1, kFreescapeDebugMove, "with pitch: %f and yaw %f", _pitch, _yaw);

		if (!checkDrill(drill)) {
			insertTemporaryMessage(_messagesList[4], _countdown - 2);
			return;
		}

		_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergy] - 5;
		const Math::Vector3d gasPocket3D(gasPocket.x, drill.y(), gasPocket.y);
		float distanceToPocket = (gasPocket3D - drill).length();
		float success = _useAutomaticDrilling ? 100.0 : 100.0 * (1.0 - distanceToPocket / _currentArea->_gasPocketRadius);
		insertTemporaryMessage(_messagesList[3], _countdown - 2);
		addDrill(drill, success > 0);
		if (success <= 0) {
			insertTemporaryMessage(_messagesList[9], _countdown - 4);
			_drillStatusByArea[_currentArea->getAreaID()] = kDrillerRigNoGas;
			return;
		}
		Common::String maxScoreMessage = _messagesList[5];
		int maxScore = _drillMaxScoreByArea[_currentArea->getAreaID()];
		maxScoreMessage.replace(2, 6, Common::String::format("%d", maxScore));
		insertTemporaryMessage(maxScoreMessage, _countdown - 4);
		Common::String successMessage = _messagesList[6];
		successMessage.replace(0, 4, Common::String::format("%d", int(success)));
		while (successMessage.size() < 14)
			successMessage += " ";
		insertTemporaryMessage(successMessage, _countdown - 6);
		_drillSuccessByArea[_currentArea->getAreaID()] = uint32(success);
		_gameStateVars[k8bitVariableScore] += uint32(maxScore * uint32(success)) / 100;

		if (success >= 50.0) {
			_drillStatusByArea[_currentArea->getAreaID()] = kDrillerRigInPlace;
			_gameStateVars[32]++;
		} else
			_drillStatusByArea[_currentArea->getAreaID()] = kDrillerRigOutOfPlace;
	} else if (keycode == Common::KEYCODE_c) {
		if (isDOS() && isDemo()) // No support for drilling here yet
			return;
		uint32 gasPocketRadius = _currentArea->_gasPocketRadius;
		clearTemporalMessages();
		if (gasPocketRadius == 0) {
			insertTemporaryMessage(_messagesList[2], _countdown - 2);
			return;
		}

		if (_flyMode) {
			insertTemporaryMessage(_messagesList[8], _countdown - 2);
			return;
		}

		if (!drillDeployed(_currentArea)) {
			insertTemporaryMessage(_messagesList[13], _countdown - 2);
			return;
		}

		if (_gameStateVars[k8bitVariableEnergy] < 5) {
			insertTemporaryMessage(_messagesList[7], _countdown - 2);
			return;
		}

		_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergy] - 5;

		uint16 areaID = _currentArea->getAreaID();
		if (_drillStatusByArea[areaID] > 0) {
			if (_drillStatusByArea[areaID] == kDrillerRigInPlace)
				_gameStateVars[32]--;
			_drillStatusByArea[areaID] = kDrillerNoRig;
		}
		removeDrill(_currentArea);
		insertTemporaryMessage(_messagesList[10], _countdown - 2);
		int maxScore = _drillMaxScoreByArea[_currentArea->getAreaID()];
		uint32 success = _drillSuccessByArea[_currentArea->getAreaID()];
		uint32 scoreToRemove = uint32(maxScore * success) / 100;
		assert(scoreToRemove <= uint32(_gameStateVars[k8bitVariableScore]));
		_gameStateVars[k8bitVariableScore] -= scoreToRemove;
	}
}

Math::Vector3d DrillerEngine::drillPosition() {
	Math::Vector3d position = _position;
	position.setValue(1, position.y() - _playerHeight);
	position = position + 300 * getProjectionToPlane(_cameraFront, Math::Vector3d(0, 1, 0));

	Object *obj = (GeometricObject *)_areaMap[255]->objectWithID(255); // Drill base
	assert(obj);
	position.setValue(0, position.x() - 128);
	position.setValue(2, position.z() - 128);
	return position;
}

bool DrillerEngine::drillDeployed(Area *area) {
	return (area->objectWithID(255) != nullptr);
}

bool DrillerEngine::checkDrill(const Math::Vector3d position) {
	GeometricObject *obj = nullptr;
	Math::Vector3d origin = position;

	int16 id;
	int heightLastObject;

	origin.setValue(0, origin.x() + 128);
	origin.setValue(1, origin.y() - 5);
	origin.setValue(2, origin.z() + 128);

	_drillBase->setOrigin(origin);
	if (_currentArea->checkCollisions(_drillBase->_boundingBox).empty())
		return false;

	origin.setValue(0, origin.x() - 128);
	origin.setValue(2, origin.z() - 128);

	id = 255;
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	origin.setValue(1, origin.y() + 6);
	obj->setOrigin(origin);

	// This bounding box is too large and can result in the drill to float next to a wall
	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	origin.setValue(1, origin.y() + 15);
	obj->setOrigin(origin);

	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	origin.setValue(1, origin.y() - 10);
	heightLastObject = obj->getSize().y();
	delete obj;

	id = 254;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	// Set position for object
	origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() - obj->getSize().z() / 5);

	obj = (GeometricObject *)obj->duplicate();
	obj->setOrigin(origin);
	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	// Undo offset
	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);
	delete obj;

	id = 253;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();

	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);

	obj->setOrigin(origin);
	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	// Undo offset
	// origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	// origin.setValue(2, origin.z() - obj->getSize().z() / 5);
	delete obj;
	return true;
}


void DrillerEngine::addSkanner(Area *area) {
	debug("area: %d", area->getAreaID());
	GeometricObject *obj = nullptr;
	int16 id;

	id = 248;
	// If first object is already added, do not re-add any
	if (area->objectWithID(id) != nullptr)
		return;

	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->makeInvisible();
	area->addObject(obj);

	id = 249;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->makeInvisible();
	area->addObject(obj);

	id = 250;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->makeInvisible();
	area->addObject(obj);
}

void DrillerEngine::addDrill(const Math::Vector3d position, bool gasFound) {
	// int drillObjectIDs[8] = {255, 254, 253, 252, 251, 250, 248, 247};
	GeometricObject *obj = nullptr;
	Math::Vector3d origin = position;

	int16 id;
	int heightLastObject;

	id = 255;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->setOrigin(origin);
	// offset.setValue(1, offset.y() + obj->getSize().y());
	obj->makeVisible();
	_currentArea->addObject(obj);

	heightLastObject = obj->getSize().y();

	id = 254;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	// Set position for object
	origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() - obj->getSize().z() / 5);

	obj = (GeometricObject *)obj->duplicate();
	obj->setOrigin(origin);
	obj->makeVisible();
	_currentArea->addObject(obj);

	// Undo offset
	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);

	id = 253;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();

	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);

	obj->setOrigin(origin);
	obj->makeVisible();
	_currentArea->addObject(obj);

	// Undo offset
	// origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	// origin.setValue(2, origin.z() - obj->getSize().z() / 5);

	if (gasFound) {
		id = 252;
		debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
		obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
		assert(obj);
		obj = (GeometricObject *)obj->duplicate();
		origin.setValue(0, origin.x() + obj->getSize().x());
		origin.setValue(1, origin.y() + heightLastObject);
		origin.setValue(2, origin.z() + obj->getSize().z());
		obj->setOrigin(origin);
		assert(obj);
		obj->makeVisible();
		_currentArea->addObject(obj);
		heightLastObject = obj->getSize().y();

		id = 251;
		debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
		obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
		assert(obj);
		obj = (GeometricObject *)obj->duplicate();
		origin.setValue(1, origin.y() + heightLastObject);
		obj->setOrigin(origin);
		assert(obj);
		obj->makeVisible();
		_currentArea->addObject(obj);
	}
}

void DrillerEngine::removeDrill(Area *area) {
	for (int16 id = 251; id < 256; id++) {
		if (id > 252)
			assert(area->objectWithID(id));

		if (area->objectWithID(id))
			area->removeObject(id);
	}
}

void DrillerEngine::initGameState() {
	_flyMode = false;
	_noClipMode = false;
	_shootingFrames = 0;
	_underFireFrames = 0;
	_yaw = 0;
	_pitch = 0;

	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (auto &it : _areaMap) {
		it._value->resetArea();
		_gameStateBits[it._key] = 0;
		if (_drillStatusByArea[it._key] != kDrillerNoRig)
			removeDrill(it._value);
		_drillStatusByArea[it._key] = kDrillerNoRig;
		if (it._key != 255) {
			addSkanner(it._value);
			_drillMaxScoreByArea[it._key] = (10 + _rnd->getRandomNumber(89)) * 1000;
		}
		_drillSuccessByArea[it._key] = 0;
	}

	_gameStateVars[k8bitVariableEnergy] = _initialTankEnergy;
	_gameStateVars[k8bitVariableShield] = _initialTankShield;

	_gameStateVars[k8bitVariableEnergyDrillerTank] = _initialTankEnergy;
	_gameStateVars[k8bitVariableShieldDrillerTank] = _initialTankShield;

	_gameStateVars[k8bitVariableEnergyDrillerJet] = _initialJetEnergy;
	_gameStateVars[k8bitVariableShieldDrillerJet] = _initialJetShield;

	_playerHeightNumber = 1;
	_playerHeight = _playerHeights[_playerHeightNumber];
	removeTimers();
	startCountdown(_initialCountdown);
	_lastMinute = 0;
	_demoIndex = 0;
	_demoEvents.clear();
}

bool DrillerEngine::checkIfGameEnded() {
	if (isDemo())
		return (_demoData[_demoIndex + 1] == 0x5f);

	if (_countdown <= 0) {
		insertTemporaryMessage(_messagesList[14], _countdown - 2);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		gotoArea(127, 0);
	}

	if (_gameStateVars[k8bitVariableShield] == 0) {
		insertTemporaryMessage(_messagesList[15], _countdown - 2);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		gotoArea(127, 0);
	}

	if (_gameStateVars[k8bitVariableEnergy] == 0) {
		insertTemporaryMessage(_messagesList[16], _countdown - 2);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		gotoArea(127, 0);
	}

	if (_hasFallen) {
		_hasFallen = false;
		playSound(14, false);
		insertTemporaryMessage(_messagesList[17], _countdown - 4);
		drawBackground();
		drawBorder();
		drawUI();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(1000);
		gotoArea(127, 0);
	}

	if (_forceEndGame) {
		_forceEndGame = false;
		insertTemporaryMessage(_messagesList[18], _countdown - 2);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		gotoArea(127, 0);
	}

	if (_currentArea->getAreaID() == 127) {
		if (_gameStateVars[32] == 18) { // All areas are complete
			insertTemporaryMessage(_messagesList[19], _countdown - 2);
			_gameStateVars[32] = 0;  // Avoid repeating the message
		}
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(5000);
		return true;
	}
	return false;
}

Common::Error DrillerEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	for (auto &it : _areaMap) { // All but skip area 255
		if (it._key == 255)
			continue;
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(_drillStatusByArea[it._key]);
		stream->writeUint32LE(_drillMaxScoreByArea[it._key]);
		stream->writeUint32LE(_drillSuccessByArea[it._key]);
	}

	return Common::kNoError;
}

Common::Error DrillerEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	for (uint i = 0; i < _areaMap.size() - 1; i++) { // All except area 255
		uint16 key = stream->readUint16LE();
		assert(key != 255);
		assert(_areaMap.contains(key));
		_drillStatusByArea[key] = stream->readUint32LE();
		if (_drillStatusByArea[key] == kDrillerNoRig)
			if (drillDeployed(_areaMap[key]))
				removeDrill(_areaMap[key]);

		_drillMaxScoreByArea[key] = stream->readUint32LE();
		_drillSuccessByArea[key] = stream->readUint32LE();
	}

	return Common::kNoError;
}

} // End of namespace Freescape
