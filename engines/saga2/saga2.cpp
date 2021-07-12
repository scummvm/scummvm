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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/util.h"

#include "saga2/saga2.h"
#include "saga2/fta.h"

#include "saga2/band.h"
#include "saga2/contain.h"
#include "saga2/gdraw.h"
#include "saga2/imagcach.h"
#include "saga2/mouseimg.h"
#include "saga2/motion.h"

namespace Saga2 {

void main_saga2();

Saga2Engine *g_vm;

Saga2Engine::Saga2Engine(OSystem *syst)
	: Engine(syst) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("saga2");

	g_vm = this;

	_bandList = nullptr;
	_mouseInfo = nullptr;
	_smkDecoder = nullptr;
	_videoX = _videoY = 0;
	_loadedWeapons = 0;

	_autoAggression = true;
	_autoWeapon = true;
	_showNight = true;
	_speechText = true;

	SearchMan.addSubDirectoryMatching(gameDataDir, "res");
	SearchMan.addSubDirectoryMatching(gameDataDir, "dos/drivers"); // For Miles Sound files
	SearchMan.addSubDirectoryMatching(gameDataDir, "drivers");

	_loadedWeapons = 0;

	_imageCache = new CImageCache;
	_mTaskList = new MotionTaskList;
	_bandList = new BandList();

	_edpList = nullptr;
	_sdpList = nullptr;
	_containerList = nullptr;
}

Saga2Engine::~Saga2Engine() {
	debug("Saga2Engine::~Saga2Engine");

	freeExeResources();

	// Dispose your resources here
	delete _rnd;
	delete _imageCache;
	delete _mTaskList;
	delete _bandList;
}

Common::Error Saga2Engine::run() {
	// Initialize graphics using following:
	initGraphics(640, 480);

	_containerList = new ContainerList;

	readConfig();

	loadExeResources();

	main_saga2();

	return Common::kNoError;
}

bool Saga2Engine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error Saga2Engine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	syncGameStream(s);
	return Common::kNoError;
}

Common::Error Saga2Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	syncGameStream(s);
	return Common::kNoError;
}

void Saga2Engine::syncGameStream(Common::Serializer &s) {
	// Use methods of Serializer to save/load fields
	int dummy = 0;
	s.syncAsUint16LE(dummy);
}

int32 clamp(int32 a, int32 val, int32 c) {
	if (val < a) return a;
	if (val > c) return c;
	return val;
}

gFont Onyx10Font;
gFont Plate18Font;
gFont Helv11Font;
gFont Amber13Font;
gFont ThinFix8Font;
gFont Script10Font;

uint32 loadingWindowWidth = 640;
uint32 loadingWindowHeight = 480;

uint8 *loadingWindowPalette;
uint8 *loadingWindowData;

uint8 *ColorMapRanges;

uint8 *closeBx1ImageData;
uint8 *closeBx2ImageData;
uint8 *usePtrImageData;
uint8 *xPointerImageData;
uint8 *arrowImageData;
uint8 *grabPtrImageData;
uint8 *attakPtrImageData;
uint8 *centerActorIndicatorImageData;
uint8 *pgUpImageData;
uint8 *pgDownImageData;
uint8 *pgLeftImageData;
uint8 *pgRightImageData;
uint8 *autoWalkImageData;
uint8 *gaugeImageData;

static void loadFont(Common::File &file, gFont *font, uint32 offset) {
	file.seek(offset);

	font->height = file.readUint16LE();
	font->baseLine = file.readUint16LE();
	font->rowMod = file.readUint16LE();

	for (int i = 0; i < 256; i++)
		font->charXOffset[i] = file.readUint16LE();

	file.read(font->charWidth, 256);
	file.read(font->charKern, 256);
	file.read(font->charSpace, 256);

	uint size = font->height * font->rowMod;

	font->fontdata = (byte *)malloc(size);
	file.read(font->fontdata, size);
}

struct dataChunks {
	uint8 **ptr;
	uint32 offset;
	uint32 size;
} chunks[] = {
	{ (uint8 **)&Onyx10Font,	0x004F7258, 0 },
	{ (uint8 **)&Plate18Font,	0x004F7EE0, 0 },
	{ (uint8 **)&Helv11Font,	0x004F9F30, 0 },
	{ (uint8 **)&Amber13Font,	0x004FAC60, 0 },
	{ (uint8 **)&ThinFix8Font,	0x004FC210, 0 },
	{ (uint8 **)&Script10Font,	0x004FCD18, 0 },
	{ &loadingWindowPalette,	0x004A2600, 1024 },
	{ &loadingWindowData,		0x004A2A00, 307200 },
	{ &ColorMapRanges,			0x004EDC20, 1584 },
	{ &closeBx1ImageData,		0x004EE2B8, 144 },
	{ &closeBx2ImageData,		0x004EE348, 144 },
	{ &usePtrImageData,			0x004EE3D8, 232 },
	{ &xPointerImageData,		0x004EE4C0, 232 },
	{ &arrowImageData,			0x004EE5A8, 192 },
	{ &grabPtrImageData,		0x004EE668, 208 },
	{ &attakPtrImageData,		0x004EE738, 536 },
	{ &centerActorIndicatorImageData,0x004EE950, 96 },
	{ &pgUpImageData,			0x004EE9B0, 256 },
	{ &pgDownImageData,			0x004EEAB0, 256 },
	{ &pgLeftImageData,			0x004EEBB0, 256 },
	{ &pgRightImageData,		0x004EECB0, 256 },
	{ &autoWalkImageData,		0x004EEDB0, 228 },
	{ &gaugeImageData,			0x004EF257, 241 },
	{ NULL,						0,			0 }
};

void Saga2Engine::loadExeResources() {
	Common::File exe;
	const uint32 offset = 0x4F6D90 - 0xF4990;

	if (!(exe.open("win/fta2win.exe") || exe.open("fta2win.exe")))
		error("FTA2WIN.EXE file is missing");

	if (exe.size() != 1093120)
		error("Incorrect FTA2WIN.EXE file size. Expected is 1093120");

	for (int i = 0; chunks[i].ptr; i++) {
		if (chunks[i].size == 0) { // Font
			loadFont(exe, (gFont *)chunks[i].ptr, chunks[i].offset - offset);
		} else {
			*chunks[i].ptr = (uint8 *)malloc(chunks[i].size);
			exe.seek(chunks[i].offset - offset);
			exe.read(*chunks[i].ptr, chunks[i].size);
		}
	}

	initCursors();

	exe.close();
}

void Saga2Engine::freeExeResources() {
	for (int i = 0; chunks[i].ptr; i++)
		if (chunks[i].size == 0) // Font
			free(((gFont *)chunks[i].ptr)->fontdata);
		else
			free(*chunks[i].ptr);

	freeCursors();
}

void Saga2Engine::readConfig() {
	_autoWeapon = true;

	if (ConfMan.hasKey("auto_weapon"))
		_autoWeapon = ConfMan.getBool("auto_weapon");

	_autoAggression = true;

	if (ConfMan.hasKey("auto_aggression"))
		_autoAggression = ConfMan.getBool("auto_aggression");

	_showNight = true;

	if (ConfMan.hasKey("show_night"))
		_showNight = ConfMan.getBool("show_night");

	_speechText = true;

	if (ConfMan.hasKey("subtitles"))
		_speechText = ConfMan.getBool("subtitles");

	syncSoundSettings();
}

void Saga2Engine::saveConfig() {
	ConfMan.flushToDisk();
}

} // End of namespace Saga2
