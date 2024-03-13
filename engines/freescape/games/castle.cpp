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

#include "common/file.h"
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

CastleEngine::CastleEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);
	_playerHeight = _playerHeights[_playerHeightNumber];

	_playerWidth = 8;
	_playerDepth = 8;
	_stepUpDistance = 32;
	_option = nullptr;
}

CastleEngine::~CastleEngine() {
	if (_option) {
		_option->free();
		delete _option;
	}
}

byte kFreescapeCastleFont[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1c, 0x1c, 0x1c, 0x18, 0x18, 0x00, 0x18, 0x18,
	0x66, 0x66, 0x44, 0x22, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x00,
	0x10, 0x54, 0x38, 0xfe, 0x38, 0x54, 0x10, 0x00,
	0x3c, 0x42, 0x9d, 0xb1, 0xb1, 0x9d, 0x42, 0x3c,
	0x78, 0xcc, 0xcc, 0x78, 0xdb, 0xcf, 0xce, 0x7b,
	0x30, 0x30, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x20, 0x40, 0x40, 0x40, 0x40, 0x20, 0x10,
	0x10, 0x08, 0x04, 0x04, 0x04, 0x04, 0x08, 0x10,
	0x10, 0x54, 0x38, 0xfe, 0x38, 0x54, 0x10, 0x00,
	0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x08, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18,
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
	0x18, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x18,
	0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x9e, 0x61, 0x01, 0x7e, 0xe0, 0xc6, 0xe3, 0xfe,
	0xee, 0x73, 0x03, 0x3e, 0x03, 0x01, 0x7f, 0xe6,
	0x0e, 0x1c, 0x38, 0x71, 0xfd, 0xe6, 0x0c, 0x0c,
	0xfd, 0x86, 0x80, 0x7e, 0x07, 0x63, 0xc7, 0x7c,
	0x3d, 0x66, 0xc0, 0xf0, 0xfc, 0xc6, 0x66, 0x3c,
	0xb3, 0x4e, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x3c,
	0x7c, 0xc6, 0xc6, 0x7c, 0xc6, 0xc2, 0xfe, 0x4c,
	0x3c, 0x4e, 0xc6, 0xc6, 0x4e, 0x36, 0x46, 0x3c,
	0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x08, 0x10,
	0x03, 0x0c, 0x30, 0xc0, 0x30, 0x0c, 0x03, 0x00,
	0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00,
	0xc0, 0x30, 0x0c, 0x03, 0x0c, 0x30, 0xc0, 0x00,
	0x7c, 0xc6, 0x06, 0x0c, 0x30, 0x30, 0x00, 0x30,
	0x00, 0x08, 0x0c, 0xfe, 0xff, 0xfe, 0x0c, 0x08,
	0x1e, 0x1c, 0x1e, 0x66, 0xbe, 0x26, 0x43, 0xe3,
	0xee, 0x73, 0x23, 0x3e, 0x23, 0x21, 0x7f, 0xe6,
	0x39, 0x6e, 0xc6, 0xc0, 0xc0, 0xc2, 0x63, 0x3e,
	0xec, 0x72, 0x23, 0x23, 0x23, 0x23, 0x72, 0xec,
	0xce, 0x7f, 0x61, 0x6c, 0x78, 0x61, 0x7f, 0xce,
	0xce, 0x7f, 0x61, 0x6c, 0x78, 0x60, 0x60, 0xf0,
	0x3d, 0x66, 0xc0, 0xc1, 0xce, 0xc6, 0x66, 0x3c,
	0xe7, 0x66, 0x66, 0x6e, 0x76, 0x66, 0x66, 0xe7,
	0x66, 0x3c, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x66,
	0x33, 0x1e, 0x0c, 0x8c, 0x4c, 0xcc, 0xdc, 0x78,
	0xf2, 0x67, 0x64, 0x68, 0x7e, 0x66, 0x66, 0xf3,
	0xd8, 0x70, 0x60, 0x60, 0x66, 0x61, 0xf3, 0x7e,
	0xc3, 0x66, 0x6e, 0x76, 0x56, 0x46, 0x46, 0xef,
	0x87, 0x62, 0x72, 0x7a, 0x5e, 0x4e, 0x46, 0xe1,
	0x18, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x18,
	0xec, 0x72, 0x63, 0x63, 0x72, 0x6c, 0x60, 0xf0,
	0x3c, 0x66, 0xc3, 0xc3, 0x66, 0x3c, 0x31, 0x1e,
	0xec, 0x72, 0x63, 0x63, 0x76, 0x6c, 0x66, 0xf1,
	0x79, 0x86, 0x80, 0x7e, 0x07, 0x63, 0xc7, 0x7c,
	0x01, 0x7f, 0xfe, 0x98, 0x58, 0x18, 0x18, 0x3c,
	0xf7, 0x62, 0x62, 0x62, 0x62, 0x62, 0xf2, 0x3c,
	0xf3, 0x61, 0x72, 0x72, 0x32, 0x32, 0x1c, 0x3e,
	0xc3, 0x62, 0x62, 0x6a, 0x6e, 0x76, 0x66, 0xc3,
	0xf3, 0x72, 0x3c, 0x38, 0x1c, 0x3c, 0x4e, 0xcf,
	0xe3, 0x72, 0x34, 0x38, 0x18, 0x18, 0x18, 0x3c,
	0x7f, 0x87, 0x0e, 0x1c, 0x38, 0x71, 0xfd, 0xe6,
};

Common::SeekableReadStream *CastleEngine::decryptFile(const Common::Path &filename) {
	Common::File file;
	file.open(filename);
	if (!file.isOpen())
		error("Failed to open %s", filename.toString().c_str());

	int size = file.size();
	byte *encryptedBuffer = (byte *)malloc(size);
	file.read(encryptedBuffer, size);
	file.close();

	int seed = 24;
	for (int i = 0; i < size; i++) {
		encryptedBuffer[i] ^= seed;
		seed = (seed + 1) & 0xff;
	}

	return (new Common::MemoryReadStream(encryptedBuffer, size));
}

extern byte kEGADefaultPalette[16][3];
extern Common::MemoryReadStream *unpackEXE(Common::File &ms);

void CastleEngine::loadAssetsDOSFullGame() {
	Common::File file;
	Common::SeekableReadStream *stream = nullptr;

	if (_renderMode == Common::kRenderEGA) {
		_viewArea = Common::Rect(40, 33, 280, 152);

		file.open("CME.EXE");
		stream = unpackEXE(file);
		if (stream) {
			loadSpeakerFxDOS(stream, 0x636d + 0x200, 0x63ed + 0x200);
		}

		delete stream;
		file.close();

		file.open("CMLE.DAT");
		_title = load8bitBinImage(&file, 0x0);
		_title->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		file.open("CMOE.DAT");
		_option = load8bitBinImage(&file, 0x0);
		_option->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		file.open("CME.DAT");
		_border = load8bitBinImage(&file, 0x0);
		_border->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		switch (_language) {
			case Common::ES_ESP:
				stream = decryptFile("CMLS");
				break;
			case Common::FR_FRA:
				stream = decryptFile("CMLF");
				break;
			case Common::DE_DEU:
				stream = decryptFile("CMLG");
				break;
			case Common::EN_ANY:
				stream = decryptFile("CMLE");
				break;
			default:
				error("Invalid or unsupported language: %x", _language);
		}

		loadFonts(kFreescapeCastleFont, 59);
		loadMessagesVariableSize(stream, 0x11, 164);
		delete stream;

		stream = decryptFile("CMEDF");
		load8bitBinary(stream, 0, 16);
		for (auto &it : _areaMap)
			it._value->addStructure(_areaMap[255]);

		_areaMap[2]->addFloor();
		delete stream;
	} else
		error("Not implemented yet");
	// CPC
	// file = gameDir.createReadStreamForMember("cm.bin");
	// if (file == nullptr)
	//	error("Failed to open cm.bin");
	// load8bitBinary(file, 0x791a, 16);
}

void CastleEngine::loadAssetsDOSDemo() {
	Common::File file;
	Common::SeekableReadStream *stream = nullptr;

	if (_renderMode == Common::kRenderEGA) {
		_viewArea = Common::Rect(40, 33, 280, 152);

		file.open("CMLE.DAT");
		_title = load8bitBinImage(&file, 0x0);
		_title->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		file.open("CMOE.DAT");
		_option = load8bitBinImage(&file, 0x0);
		_option->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		file.open("CME.DAT");
		_border = load8bitBinImage(&file, 0x0);
		_border->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		stream = decryptFile("CMLD"); // Only english

		loadFonts(kFreescapeCastleFont, 59);
		loadMessagesVariableSize(stream, 0x11, 164);
		delete stream;

		stream = decryptFile("CDEDF");
		load8bitBinary(stream, 0, 16);
		for (auto &it : _areaMap)
			it._value->addStructure(_areaMap[255]);

		_areaMap[2]->addFloor();
		delete stream;
	} else
		error("Not implemented yet");
}

void CastleEngine::loadAssetsAmigaDemo() {
	Common::File file;
	file.open("x");
	if (!file.isOpen())
		error("Failed to open 'x' file");

	loadMessagesVariableSize(&file, 0x8bb2, 164);
	load8bitBinary(&file, 0x162a6, 16);
	loadPalettes(&file, 0x151a6);

	file.seek(0x2be96); // Area 255
	_areaMap[255] = load8bitArea(&file, 16);
	file.close();


	_areaMap[2]->_groundColor = 1;
	for (auto &it : _areaMap)
		it._value->addStructure(_areaMap[255]);

	_areaMap[2]->addFloor();
}


void CastleEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	if (entranceID > 0)
		traverseEntrance(entranceID);

	_lastPosition = _position;

	if (_currentArea->_skyColor > 0 && _currentArea->_skyColor != 255) {
		_gfx->_keyColor = 0;
	} else
		_gfx->_keyColor = 255;

	_lastPosition = _position;
	_gameStateVars[0x1f] = 0;

	if (areaID == _startArea && entranceID == _startEntrance) {
		_yaw = 310;
		_pitch = 0;
		playSound(9, false);
	} else {
		playSound(5, false);
	}

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	clearTemporalMessages();
	// Ignore sky/ground fields
	_gfx->_keyColor = 0;
	_gfx->clearColorPairArray();

	_gfx->_colorPair[_currentArea->_underFireBackgroundColor] = _currentArea->_extraColor[0];
	_gfx->_colorPair[_currentArea->_usualBackgroundColor] = _currentArea->_extraColor[1];
	_gfx->_colorPair[_currentArea->_paperColor] = _currentArea->_extraColor[2];
	_gfx->_colorPair[_currentArea->_inkColor] = _currentArea->_extraColor[3];

	swapPalette(areaID);
	resetInput();
}

void CastleEngine::drawDOSUI(Graphics::Surface *surface) {
	uint32 color = 10;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 0;

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::Rect backRect(97, 181, 232, 190);
	surface->fillRect(backRect, back);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 97, 182, front, back, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else
		drawStringInSurface(_currentArea->_name, 97, 182, front, back, surface);
}

void CastleEngine::initGameState() {
	FreescapeEngine::initGameState();
	_playerHeightNumber = 1;
	_playerHeight = _playerHeights[_playerHeightNumber];
}

void CastleEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source;
	_currentAreaMessages.clear();
	if (index > 127) {
		index = _messagesList.size() - (index - 254) - 2;
		// TODO
		//drawFullscreenMessage(_messagesList[index]);
		return;
	}
	debugC(1, kFreescapeDebugCode, "Printing message %d: \"%s\"", index, _messagesList[index].c_str());
	insertTemporaryMessage(_messagesList[index], _countdown - 3);
}


Common::Error CastleEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	return Common::kNoError;
}

Common::Error CastleEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	return Common::kNoError;
}

} // End of namespace Freescape
