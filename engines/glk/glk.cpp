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
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/language.h"
#include "engines/util.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "glk/glk.h"
#include "glk/blorb.h"
#include "glk/conf.h"
#include "glk/debugger.h"
#include "glk/events.h"
#include "glk/picture.h"
#include "glk/screen.h"
#include "glk/selection.h"
#include "glk/sound.h"
#include "glk/speech.h"
#include "glk/streams.h"
#include "glk/windows.h"

namespace Glk {

GlkEngine *g_vm;

GlkEngine::GlkEngine(OSystem *syst, const GlkGameDescription &gameDesc) :
		_gameDescription(gameDesc), Engine(syst), _random("Glk"), _quitFlag(false), _blorb(nullptr),
		_clipboard(nullptr), _conf(nullptr),_events(nullptr), _pictures(nullptr), _screen(nullptr),
		_selection(nullptr), _sounds(nullptr), _streams(nullptr), _windows(nullptr),
		_copySelect(false), _terminated(false), _pcSpeaker(nullptr), _loadSaveSlot(-1),
		gli_register_obj(nullptr), gli_unregister_obj(nullptr), gli_register_arr(nullptr),
		gli_unregister_arr(nullptr) {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugCore, "core", "Core engine debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");
	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound and Music handling");
	DebugMan.addDebugChannel(kDebugSpeech, "speech", "Text to Speech handling");

	g_vm = this;
}

GlkEngine::~GlkEngine() {
	delete _blorb;
	delete _clipboard;
	delete _events;
	delete _pcSpeaker;
	delete _pictures;
	delete _screen;
	delete _selection;
	delete _sounds;
	delete _streams;
	delete _windows;
	delete _conf;

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

void GlkEngine::initialize() {
	createConfiguration();
	_conf->load();
	//_conf->flush();

	initGraphicsMode();
	createDebugger();

	_screen = createScreen();
	_screen->initialize();
	_clipboard = new Clipboard();
	_events = new Events();
	_pcSpeaker = new PCSpeaker(_mixer);
	_pictures = new Pictures();
	_selection = new Selection();
	_sounds = new Sounds();
	_streams = new Streams();
	_windows = new Windows(_screen);

	// Setup mixer
	syncSoundSettings();
}

Screen *GlkEngine::createScreen() {
	return new Screen();
}

void GlkEngine::initGraphicsMode() {
	initGraphics(_conf->_width, _conf->_height, &_conf->_screenFormat);
}

void GlkEngine::createDebugger() {
	setDebugger(new Debugger());
}

void GlkEngine::createConfiguration() {
	_conf = new Conf(getInterpreterType());
}

Common::Error GlkEngine::run() {
	// Open up the game file
	Common::String filename = getFilename();
	if (!Common::File::exists(filename))
		return Common::kNoGameDataFoundError;

	if (Blorb::isBlorb(filename)) {
		// Blorb archive
		_blorb = new Blorb(filename, getInterpreterType());
		SearchMan.add("blorb", _blorb, 99, false);

		if (!_gameFile.open("game", *_blorb))
			return Common::kNoGameDataFoundError;
	} else {
		// Check for a secondary blorb file with the same filename
		Common::StringArray blorbFilenames;
		Blorb::getBlorbFilenames(filename, blorbFilenames, getInterpreterType(), getGameID());

		for (uint idx = 0; idx < blorbFilenames.size(); ++idx) {
			if (Common::File::exists(blorbFilenames[idx])) {
				_blorb = new Blorb(blorbFilenames[idx], getInterpreterType());
				SearchMan.add("blorb", _blorb, 99, false);
				break;
			}
		}

		// Open up the game file
		if (!_gameFile.open(filename))
			return Common::kNoGameDataFoundError;
	}

	// Perform initialization
	initialize();

	// Play the game
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	runGame();
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	return Common::kNoError;
}

bool GlkEngine::canLoadGameStateCurrently() {
	// Only allow savegames by default when sub-engines are waiting for a line
	Window *win = _windows->getFocusWindow();
	return win && (win->_lineRequest || win->_lineRequestUni);
}

bool GlkEngine::canSaveGameStateCurrently() {
	// Only allow savegames by default when sub-engines are waiting for a line
	Window *win = _windows->getFocusWindow();
	return win && (win->_lineRequest || win->_lineRequestUni);
}

Common::Error GlkEngine::loadGame() {
	frefid_t ref = _streams->createByPrompt(fileusage_BinaryMode | fileusage_SavedGame,
		filemode_Read, 0);
	if (ref == nullptr)
		return Common::kReadingFailed;

	int slotNumber = ref->_slotNumber;
	_streams->deleteRef(ref);

	return loadGameState(slotNumber);
}

Common::Error GlkEngine::saveGame() {
	frefid_t ref = _streams->createByPrompt(fileusage_BinaryMode | fileusage_SavedGame,
		filemode_Write, 0);
	if (ref == nullptr)
		return Common::kWritingFailed;

	int slot = ref->_slotNumber;
	Common::String desc = ref->_description;
	_streams->deleteRef(ref);

	return saveGameState(slot, desc);
}

Common::Error GlkEngine::loadGameState(int slot) {
	FileReference ref(slot, "", fileusage_SavedGame | fileusage_TextMode);

	strid_t file = _streams->openFileStream(&ref, filemode_Read);
	if (file == nullptr)
		return Common::kReadingFailed;

	Common::ErrorCode errCode = Common::kNoError;
	QuetzalReader r;
	if (r.open(*file, ID_IFSF))
		// Load in the savegame chunks
		errCode = loadGameChunks(r).getCode();

	file->close();
	return errCode;
}

Common::Error GlkEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::String msg;
	FileReference ref(slot, desc, fileusage_BinaryMode | fileusage_SavedGame);

	strid_t file = _streams->openFileStream(&ref, filemode_Write);
	if (file == nullptr)
		return Common::kWritingFailed;

	// Write out savegame chunks
	QuetzalWriter w;
	Common::ErrorCode errCode = saveGameChunks(w).getCode();

	if (errCode == Common::kNoError) {
		w.save(*file, desc);
	}

	file->close();
	return errCode;
}

Common::Error GlkEngine::loadGameChunks(QuetzalReader &quetzal) {
	// First scan for a SCVM chunk. It has information of the game the save is for,
	// so if present we can validate the save is for this game
	for (QuetzalReader::Iterator it = quetzal.begin(); it != quetzal.end(); ++it) {
		if ((*it)._id == ID_SCVM) {
			// Skip over date/time & playtime
			Common::SeekableReadStream *rs = it.getStream();
			rs->skip(14);

			uint32 interpType = rs->readUint32BE();
			Common::String langCode = QuetzalReader::readString(rs);
			Common::String md5 = QuetzalReader::readString(rs);
			delete rs;

			if (interpType != QuetzalBase::getInterpreterTag(getInterpreterType()) ||
				parseLanguage(langCode) != getLanguage() || md5 != getGameMD5())
				return Common::kReadingFailed;
		}
	}

	// Scan for an uncompressed memory chunk
	for (QuetzalReader::Iterator it = quetzal.begin(); it != quetzal.end(); ++it) {
		if ((*it)._id == ID_UMem) {
			Common::SeekableReadStream *rs = it.getStream();
			Common::Error err = readSaveData(rs);
			delete rs;

			return err;
		}
	}

	// Couldn't find any data chunk, so reading failed
	return Common::kReadingFailed;
}

Common::Error GlkEngine::saveGameChunks(QuetzalWriter &quetzal) {
	// Add the uncompressed memory chunk with the game's save data
	Common::WriteStream &ws = quetzal.add(ID_UMem);
	return writeGameData(&ws);
}

void GlkEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	int volume = ConfMan.getBool("sfx_mute") ? 0 : CLIP(ConfMan.getInt("sfx_volume"), 0, 255);
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, volume);

	SpeechManager::syncSoundSettings();
}

void GlkEngine::beep() {
	_pcSpeaker->speakerOn(50, 50);
}

void GlkEngine::switchToWhiteOnBlack() {
	const uint WHITE = _conf->parseColor("ffffff");
	const uint BLACK = _conf->parseColor("000000");

	_conf->_wMarginX = 0;
	_conf->_wMarginY = 0;
	_conf->_tMarginY = 4;
	_conf->_propInfo._caretColor = WHITE;
	_conf->_monoInfo._caretColor = WHITE;

	_conf->_windowColor = _conf->_windowSave = 0;
	WindowStyle &ws1 = _conf->_tStyles[style_Normal];
	ws1.bg = BLACK;
	ws1.fg = WHITE;

	WindowStyle &ws2 = _conf->_tStyles[style_Input];
	ws2.bg = BLACK;
	ws2.fg = WHITE;

	WindowStyle &ws3 = _conf->_gStyles[style_Normal];
	ws3.bg = BLACK;
	ws3.fg = WHITE;
}

} // End of namespace Glk
