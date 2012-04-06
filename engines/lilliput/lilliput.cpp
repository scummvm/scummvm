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
 */

#include "common/system.h"
#include "common/random.h"
#include "common/error.h"
#include "common/events.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/memstream.h"

#include "lilliput/lilliput.h"
#include "engines/util.h"
#include "lilliput/script.h"

namespace Lilliput {

LilliputEngine *LilliputEngine::s_Engine = 0;

LilliputEngine::LilliputEngine(OSystem *syst, const LilliputGameDescription *gd) : Engine(syst), _gameDescription(gd)
{
	_system = syst;
	DebugMan.addDebugChannel(kDebugSchedule, "Schedule", "Script Schedule debug level");
	DebugMan.addDebugChannel(kDebugEngine, "Engine", "Engine debug level");
	DebugMan.addDebugChannel(kDebugDisplay, "Display", "Display debug level");
	DebugMan.addDebugChannel(kDebugMouse, "Mouse", "Mouse debug level");
	DebugMan.addDebugChannel(kDebugParser, "Parser", "Parser debug level");
	DebugMan.addDebugChannel(kDebugFile, "File", "File IO debug level");
	DebugMan.addDebugChannel(kDebugRoute, "Route", "Route debug level");
	DebugMan.addDebugChannel(kDebugInventory, "Inventory", "Inventory debug level");
	DebugMan.addDebugChannel(kDebugObject, "Object", "Object debug level");
	DebugMan.addDebugChannel(kDebugMusic, "Music", "Music debug level");

	_console = new LilliputConsole(this);
	_rnd = 0;
	_scriptHandler = new LilliputScript(this);

	_byte1714E = 0;
	_rulesBuffer2PrevIndx = 0;
}

LilliputEngine::~LilliputEngine() {

	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _rnd;
}

GUI::Debugger *LilliputEngine::getDebugger() {
	return _console;
}

bool LilliputEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsRTL) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *LilliputEngine::getCopyrightString() const {
	return "copyright S.L.Grand, Brainware, 1991";
}

GameType LilliputEngine::getGameType() const {
	return _gameType;
}

Common::Platform LilliputEngine::getPlatform() const {
	return _platform;
}

byte *LilliputEngine::loadVGA(Common::String filename, bool loadPal) {
	Common::File f;

	if (!f.open(filename))
		error("Missing game file %s", filename.c_str());

	int remainingSize = f.size();
	if (loadPal) {
		for (int i = 0; i < 768; ++i)
			_palette[i] = f.readByte();
		remainingSize -= 768;
	}

	uint8 curByte;
	byte decodeBuffer[100000];
	int size = 0;

	for (;remainingSize > 0;) {
		curByte = f.readByte();
		--remainingSize;

		if (curByte == 0xFF)
			break;

		if (curByte & 0x80) {
			// Compressed
			int compSize = (curByte & 0x7F);
			curByte = f.readByte();
			--remainingSize;

			for (int i = 0; i < compSize; ++i) {
				decodeBuffer[size] = curByte;
				++size;
			}
		} else {
			// Not compressed
			int rawSize = (curByte & 0xF);
			for (int i = 0; i < rawSize; ++i) {
				decodeBuffer[size] = f.readByte();
				--remainingSize;
				++size;
			}
		}
	}

	f.close();

	byte *res = (byte *)malloc(sizeof(byte) * size);
	memcpy(res, decodeBuffer, size);
	return res;
}

byte *LilliputEngine::loadRaw(Common::String filename) {
	Common::File f;

	if (!f.open(filename))
		error("Missing game file %s", filename.c_str());

	int size = f.size();
	byte *res = (byte *)malloc(sizeof(byte) * size);
	for (int i = 0; i < size; ++i)
		res[i] = f.readByte();

	f.close();
	return res;
}

void LilliputEngine::loadRules() {
	static const byte _rulesXlatArray[26] = {30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38, 50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44};
	Common::File f;
	uint16 curWord;

	if (!f.open("ERULES.PRG"))
		error("Missing game file ERULES.PRG");

	_word10800_ERULES = f.readUint16LE();

	// Chunk 1
	int size = f.readUint16LE();
	_rulesChunk1 = (byte *)malloc(sizeof(byte) * size);
	for (int i = 0; i < size; ++i)
		_rulesChunk1[i] = f.readByte();

	// Chunk 2
	_word10807_ERULES = f.readSint16LE();
	assert(_word10807_ERULES <= 40);

	for (int i = _word10807_ERULES, j = 0; i != 0; i--, j++) {
		curWord = f.readUint16LE();
		if (curWord != 0xFFFF)
			curWord = (curWord << 3) + 4;
		_rulesBuffer2_1[j] = curWord;

		curWord = f.readUint16LE();
		if (curWord != 0xFFFF)
			curWord = (curWord << 3) + 4;
		_rulesBuffer2_2[j] = curWord;

		_rulesBuffer2_3[j] = (f.readUint16LE() & 0xFF);
		_rulesBuffer2_4[j] = f.readUint16LE();
		_rulesBuffer2_5[j] = f.readByte();
		_rulesBuffer2_6[j] = f.readByte();
		_rulesBuffer2_7[j] = f.readByte();
		_rulesBuffer2_8[j] = f.readByte();
		_rulesBuffer2_9[j] = f.readByte();
		_rulesBuffer2_10[j] = f.readByte();
		_rulesBuffer2_11[j] = f.readByte();
		_rulesBuffer2_12[j] = f.readByte();
		_rulesBuffer2_13[j] = f.readByte();
		_rulesBuffer2_14[j] = f.readByte();

		for (int k = 0; k < 32; k++)
			_rulesBuffer2_15[(j * 32) + k] = f.readByte();

		for (int k = 0; k < 32; k++)
			_rulesBuffer2_16[(j * 32) + k] = f.readByte();
	}

	// Chunk 3 & 4
	_rulesChunk3_size = f.readSint16LE();
	curWord = f.readSint16LE();

	_rulesChunk3 = (int *)malloc(sizeof(int) * _rulesChunk3_size);
	for (int i = 0; i < _rulesChunk3_size; ++i)
		_rulesChunk3[i] = f.readUint16LE();

	_rulesChunk4 = (byte *)malloc(sizeof(byte) * curWord);
	for (int i = 0; i < curWord; ++i)
		_rulesChunk4[i] = f.readByte();

	// Chunk 5: Scripts
	// Use byte instead of int, therefore multiply by two the size.
	// This is for changing that into a memory read stream
	_rulesScript_size = f.readUint16LE() * 2;
	_rulesScript = (byte *)malloc(sizeof(byte) * _rulesScript_size);
	for (int i = 0; i < _rulesScript_size; ++i)
		_rulesScript[i] = f.readByte();

	// Chunk 6
	_rulesChunk6_size = f.readUint16LE();
	_rulesChunk6 = (int *)malloc(sizeof(int) * _rulesChunk6_size);
	for (int i = 0; i < _rulesChunk6_size; ++i)
		_rulesChunk6[i] = f.readUint16LE();

	// Chunk 7 & 8
	_rulesChunk7_size = f.readUint16LE();
	_rulesChunk7 = (int *)malloc(sizeof(int) * _rulesChunk7_size);
	for (int i = 0; i < _rulesChunk7_size; ++i)
		_rulesChunk7[i] = f.readUint16LE();

	curWord = f.readUint16LE();
	_rulesChunk8 = (byte *)malloc(sizeof(byte) * curWord);
	for (int i = 0; i < curWord; ++i)
		_rulesChunk8[i] = f.readByte();

	// Chunk 9
	for (int i = 0; i < 60; i++)
		_rulesChunk9[i] = f.readByte();

	// Chunk 10 & 11
	_rulesChunk10_size = f.readByte();
	assert(_rulesChunk10_size <= 20);

	if (_rulesChunk10_size != 0) {
		_rulesChunk10 = (int *)malloc(sizeof(int) * _rulesChunk10_size);
		int totalSize = 0;
		for (int i = 0; i < _rulesChunk10_size; ++i) {
			_rulesChunk10[i] = totalSize;
			totalSize += f.readByte();
		}
		if (totalSize != 0) {
			_rulesChunk11 = (byte *)malloc(sizeof(byte) * totalSize);
			for (int i = 0; i < totalSize; i++)
				_rulesChunk11[i] = f.readByte();
		}
	}

	// Chunk 12
	_rulesChunk12_size = f.readUint16LE();
	assert(_rulesChunk12_size <= 40);

	for (int i = 0; i < _rulesChunk12_size; i++) {
		_rulesBuffer12_1[i] = f.readUint16LE();
		_rulesBuffer12_2[i] = f.readUint16LE();
		_rulesBuffer12_3[i] = f.readUint16LE();
		_rulesBuffer12_4[i] = f.readUint16LE();
	}

	// Chunk 13
	_word12F68_ERULES = f.readUint16LE();
	for (int i = 0 ; i < 20; i++)
		_rulesBuffer13_1[i] = f.readByte();

	for (int i = 0 ; i < 20; i++)
		_rulesBuffer13_2[i] = f.readUint16LE();

	for (int i = 0 ; i < 20; i++)
		_rulesBuffer13_3[i] = f.readUint16LE();

	for (int i = 0; i < 20; i++) {
		byte curByte = f.readByte();

		if (curByte == 0x20)
			_rulesBuffer13_4[i] = 0x39;
		else if (curByte == 0xD)
			_rulesBuffer13_4[i] = 0x1C;
		// Hack to avoid xlat out of bounds
		else if (curByte == 0xFF)
			_rulesBuffer13_4[i] = 0x21;
		// Hack to avoid xlat out of bounds
		else if (curByte == 0x00)
			_rulesBuffer13_4[i] = 0xB4;
		else {
			assert((curByte > 0x40) && (curByte <= 0x41 + 26));
			_rulesBuffer13_4[i] = _rulesXlatArray[curByte - 0x41];
		}
	}
	f.close();

	// Skipped: Load Savegame
}

Common::Error LilliputEngine::run() {
	s_Engine = this;
	initGraphics(320, 200);

	// Setup mixer
	syncSoundSettings();

	// TODO: Init Palette

	// Load files. In the original, the size was hardcoded
	_bufferIdeogram = loadVGA("IDEOGRAM.VGA", false);
	_bufferMen = loadVGA("MEN.VGA", false);
	_bufferMen2 = loadVGA("MEN2.VGA", false);
	_bufferIsoChars = loadVGA("ISOCHARS.VGA", false);
	_bufferIsoMap = loadRaw("ISOMAP.DTA");

	//TODO: Init mouse handler

	loadRules();

	//TODO: Init sound/music player
	_scriptHandler->runScript(Common::MemoryReadStream(_rulesScript, _rulesScript_size));

	//TODO: Main loop
	return Common::kNoError;
}

void LilliputEngine::initialize() {
	debugC(1, kDebugEngine, "initialize");

	_rnd = new Common::RandomSource("robin");
	_rnd->setSeed(42);                              // Kick random number generator

	for (int i = 0; i < 4; i++) {
		_arr18560[i]._field0 = 0;
		_arr18560[i]._field1 = 0;
		_arr18560[i]._field3 = 0;
		for (int j = 0; j < 8; j ++)
			_arr18560[i]._field5[j] = 0;
	}
}

void LilliputEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

//	_sound->syncVolume();
}

Common::String LilliputEngine::getSavegameFilename(int slot) {
	return _targetName + Common::String::format("-%02d.SAV", slot);
}

byte LilliputEngine::_keyboard_getch() {
	return ' ';
}

} // End of namespace Lilliput
