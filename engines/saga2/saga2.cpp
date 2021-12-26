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

#include "saga2/actor.h"
#include "saga2/audio.h"
#include "saga2/band.h"
#include "saga2/beegee.h"
#include "saga2/calender.h"
#include "saga2/contain.h"
#include "saga2/detection.h"
#include "saga2/dispnode.h"
#include "saga2/gdraw.h"
#include "saga2/imagcach.h"
#include "saga2/mouseimg.h"
#include "saga2/motion.h"
#include "saga2/music.h"
#include "saga2/panel.h"
#include "saga2/spelshow.h"
#include "saga2/tilemode.h"
#include "saga2/vpal.h"

namespace Saga2 {

void main_saga2();

Saga2Engine *g_vm;

Saga2Engine::Saga2Engine(OSystem *syst, const SAGA2GameDescription *desc)
	: Engine(syst), _gameDescription(desc) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("saga2");

	g_vm = this;

	_console = nullptr;
	_renderer = nullptr;
	_audio = nullptr;
	_pal = nullptr;
	_act = nullptr;
	_calender = nullptr;
	_tmm = nullptr;
	_cnm = nullptr;

	_bandList = nullptr;
	_mouseInfo = nullptr;
	_smkDecoder = nullptr;
	_videoX = _videoY = 0;
	_loadedWeapons = 0;

	_gameRunning = true;
	_autoAggression = true;
	_autoWeapon = true;
	_showNight = true;
	_speechText = true;
	_speechVoice = true;

	_showPosition = false;
	_showStats = false;
	_showStatusMsg = false;
	_teleportOnClick = false;
	_teleportOnMap = false;

	_indivControlsFlag = false;
	_userControlsSetup = false;
	_fadeDepth = 1;
	_currentMapNum = 0;

	SearchMan.addSubDirectoryMatching(gameDataDir, "res");
	SearchMan.addSubDirectoryMatching(gameDataDir, "dos/drivers"); // For Miles Sound files
	SearchMan.addSubDirectoryMatching(gameDataDir, "drivers");
	SearchMan.addSubDirectoryMatching(gameDataDir, "video");	// FTA2 movies
	SearchMan.addSubDirectoryMatching(gameDataDir, "smack"); // Dino movies

	_loadedWeapons = 0;

	_imageCache = new CImageCache;
	_mTaskList = new MotionTaskList;
	_bandList = new BandList();
	_mainDisplayList = new DisplayNodeList;
	_activeSpells = new SpellDisplayList(kMaxActiveSpells);
	_pointer = new gMousePointer(_mainPort);
	_activeRegionList = new ActiveRegion[kPlayerActors];
	_toolBase = new gToolBase;
	_properties = new Properties;
	_aTaskList = new TileActivityTaskList;
	_grandMasterFTA = new Deejay;

	_edpList = nullptr;
	_sdpList = nullptr;
	_tileImageBanks = nullptr;
	_stackList = nullptr;
	_taskList = nullptr;
	_frate = nullptr;
	_lrate = nullptr;
}

Saga2Engine::~Saga2Engine() {
	debug("Saga2Engine::~Saga2Engine");

	freeExeResources();

	// Dispose your resources here
	delete _rnd;
	delete _renderer;
	delete _pal;
	delete _act;
	delete _calender;
	delete _tmm;
	delete _cnm;

	delete _imageCache;
	delete _mTaskList;
	delete _bandList;
	delete _mainDisplayList;
	delete _activeSpells;
	delete _pointer;
	delete[] _activeRegionList;
	delete _toolBase;
	delete _properties;
	delete _aTaskList;
	delete _grandMasterFTA;
}

Common::Error Saga2Engine::run() {
	// Initialize graphics using following:
	initGraphics(640, 480);

	_console = new Console(this);
	setDebugger(_console);

	_renderer = new Renderer();

	_pal = new PaletteManager;
	_act = new ActorManager;
	_calender = new CalenderTime;
	_tmm = new TileModeManager;
	_cnm = new ContainerManager;

	readConfig();

	if (getGameId() == GID_FTA2)
		loadExeResources();
	// TODO: Load EXE resources for Dino

	main_saga2();

	return Common::kNoError;
}

bool Saga2Engine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsSubtitleOptions);
}

int Saga2Engine::getGameId() const {
	return _gameDescription->gameId;
}

const ADGameFileDescription *Saga2Engine::getFilesDescriptions() const {
	return _gameDescription->desc.filesDescriptions;
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

Common::String Saga2Engine::getSavegameFile(int slot) {
	return getMetaEngine()->getSavegameFile(slot, _targetName.c_str());
}

Common::Error Saga2Engine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	pauseTimer();

	Common::OutSaveFile *outS = getSaveFileManager()->openForSaving(getSavegameFile(slot), false);
	if (!outS)
		return Common::kCreatingFileFailed;

	saveGame(outS, desc);

	outS->write("SCVM", 4);
	CHUNK_BEGIN;
	uint32 pos = outS->pos() + 4;

	_renderer->saveBackBuffer(kBeforeTakingThumbnail);

	if (_renderer->hasSavedBackBuffer(kBeforeOpeningMenu))
		_renderer->popSavedBackBuffer(kBeforeOpeningMenu);

	getMetaEngine()->appendExtendedSaveToStream(out, g_vm->getTotalPlayTime() / 1000, desc, isAutosave, pos);

	_renderer->popSavedBackBuffer(kBeforeTakingThumbnail);
	CHUNK_END;

	outS->finalize();

	delete outS;

	resumeTimer();

	return Common::kNoError;
}

Common::Error Saga2Engine::loadGameState(int slot) {
	loadGame(slot);

	return Common::kNoError;
}

void Saga2Engine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_speechText = true;

	if (ConfMan.hasKey("subtitles"))
		_speechText = ConfMan.getBool("subtitles");

	_speechVoice = true;

	if (ConfMan.hasKey("speech_mute"))
		_speechVoice = !ConfMan.getBool("speech_mute");

	if (_audio)
		_audio->_music->syncSoundSettings();
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
	{ nullptr,						0,			0 }
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

	syncSoundSettings();
}

void Saga2Engine::saveConfig() {
	ConfMan.flushToDisk();
}

} // End of namespace Saga2
