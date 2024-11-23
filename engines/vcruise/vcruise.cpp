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

#include "engines/util.h"

#include "common/compression/gentee_installer.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/algorithm.h"
#include "common/translation.h"

#include "audio/mididrv.h"
#include "audio/mixer.h"

#include "gui/message.h"

#include "vcruise/runtime.h"
#include "vcruise/vcruise.h"

namespace VCruise {

VCruiseEngine::VCruiseEngine(OSystem *syst, const VCruiseGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
}

VCruiseEngine::~VCruiseEngine() {
}

void VCruiseEngine::handleEvents() {
	Common::Event evt;
	Common::EventManager *eventMan = _system->getEventManager();

	while (eventMan->pollEvent(evt)) {
		switch (evt.type) {
		case Common::EVENT_LBUTTONDOWN:
			_runtime->onLButtonDown(evt.mouse.x, evt.mouse.y);
			break;
		case Common::EVENT_LBUTTONUP:
			_runtime->onLButtonUp(evt.mouse.x, evt.mouse.y);
			break;
		case Common::EVENT_MOUSEMOVE:
			_runtime->onMouseMove(evt.mouse.x, evt.mouse.y);
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			_runtime->onKeymappedEvent(static_cast<VCruise::KeymappedEvent>(evt.customType));
			break;
		default:
			break;
		}
	}
}

void VCruiseEngine::staticHandleMidiTimer(void *refCon) {
	static_cast<VCruiseEngine *>(refCon)->handleMidiTimer();
}

void VCruiseEngine::handleMidiTimer() {
	_runtime->onMidiTimer();
}

Common::Error VCruiseEngine::run() {
	Common::List<Graphics::PixelFormat> pixelFormats = _system->getSupportedFormats();

#if !defined(USE_JPEG)
	if (_gameDescription->desc.flags & VCRUISE_GF_NEED_JPEG) {
		return Common::Error(Common::kUnknownError, _s("This game requires JPEG support, which was not compiled in."));
	}
#endif

#if !defined(USE_OGG) || !defined(USE_VORBIS)
	if (_gameDescription->desc.flags & VCRUISE_GF_WANT_OGG_VORBIS) {
		GUI::MessageDialog dialog(
			_("Music for this game requires Ogg Vorbis support, which was not compiled in.\n"
			  "The game will still play, but will not have any music."),
			_("OK"));
		dialog.runModal();
	}
#endif

#if !defined(USE_MAD)
	if (_gameDescription->desc.flags & VCRUISE_GF_WANT_MP3) {
		GUI::MessageDialog dialog(
			_("Music for this game requires MP3 support, which was not compiled in.\n"
			  "The game will still play, but will not have any music."),
			_("OK"));
		dialog.runModal();
	}
#endif

	Common::ScopedPtr<MidiDriver> midiDrv;
	if (_gameDescription->gameID == GID_AD2044) {
		MidiDriver::DeviceHandle midiDevHdl = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
		if (midiDevHdl) {
			midiDrv.reset(MidiDriver::createMidi(midiDevHdl));

			if (midiDrv->open() != 0)
				midiDrv.reset();
		}
	}

	if (_gameDescription->desc.flags & VCRUISE_GF_GENTEE_PACKAGE) {
		Common::File *f = new Common::File();

		if (!f->open(_gameDescription->desc.filesDescriptions[0].fileName))
			error("Couldn't open installer package '%s'", _gameDescription->desc.filesDescriptions[0].fileName);

		Common::Archive *installerPackageArchive = Common::createGenteeInstallerArchive(f, "#setuppath#\\", true);
		if (!installerPackageArchive)
			error("Couldn't load installer package '%s'", _gameDescription->desc.filesDescriptions[0].fileName);

		SearchMan.add("VCruiseInstallerPackage", installerPackageArchive);
	}

	syncSoundSettings();

	const Graphics::PixelFormat *fmt16_565 = nullptr;
	const Graphics::PixelFormat *fmt16_555 = nullptr;
	const Graphics::PixelFormat *fmt32 = nullptr;

	for (const Graphics::PixelFormat &fmt : pixelFormats) {
		if (fmt32 == nullptr && fmt.bytesPerPixel == 4 && fmt.rBits() == 8 && fmt.gBits() == 8 && fmt.bBits() == 8)
			fmt32 = &fmt;
		if (fmt16_555 == nullptr && fmt.rBits() == 5 && fmt.gBits() == 5 && fmt.bBits() == 5)
			fmt16_555 = &fmt;
		if (fmt16_565 == nullptr && fmt.rBits() == 5 && fmt.gBits() == 6 && fmt.bBits() == 5)
			fmt16_565 = &fmt;
	}

	// Figure out screen layout
	Common::Point size;

	if (_gameDescription->gameID == GID_AD2044) {
		size = Common::Point(640, 480);

		Common::Point traySize = Common::Point(640, 97);
		Common::Point menuBarSize = Common::Point(188, 102);
		Common::Point videoTL = Common::Point(20, 21);

		Common::Point videoSize = Common::Point(432, 307);

		_menuBarRect = Common::Rect(size.x - menuBarSize.x, 0, size.x, menuBarSize.y);
		_videoRect = Common::Rect(videoTL, videoTL + videoSize);
		_trayRect = Common::Rect(0, size.y - traySize.y, size.x, size.y);
		_subtitleRect = Common::Rect(_videoRect.left, _videoRect.bottom, _videoRect.right, _trayRect.top);
	} else {
		Common::Point videoSize;
		Common::Point traySize;
		Common::Point menuBarSize;

		if (_gameDescription->gameID == GID_REAH) {
			videoSize = Common::Point(608, 348);
			menuBarSize = Common::Point(640, 44);
			traySize = Common::Point(640, 88);
		} else if (_gameDescription->gameID == GID_SCHIZM) {
			videoSize = Common::Point(640, 360);
			menuBarSize = Common::Point(640, 32);
			traySize = Common::Point(640, 88);
		} else {
			error("Unknown game");
		}

		size.x = videoSize.x;
		if (menuBarSize.x > size.x)
			size.x = menuBarSize.x;
		if (traySize.x > size.x)
			size.x = traySize.x;

		size.y = videoSize.y + menuBarSize.y + traySize.y;

		Common::Point menuTL = Common::Point((size.x - menuBarSize.x) / 2, 0);
		Common::Point videoTL = Common::Point((size.x - videoSize.x) / 2, menuTL.y + menuBarSize.y);
		Common::Point trayTL = Common::Point((size.x - traySize.x) / 2, videoTL.y + videoSize.y);

		_menuBarRect = Common::Rect(menuTL.x, menuTL.y, menuTL.x + menuBarSize.x, menuTL.y + menuBarSize.y);
		_videoRect = Common::Rect(videoTL.x, videoTL.y, videoTL.x + videoSize.x, videoTL.y + videoSize.y);
		_trayRect = Common::Rect(trayTL.x, trayTL.y, trayTL.x + traySize.x, trayTL.y + traySize.y);
	}

	if (fmt32)
		initGraphics(size.x, size.y, fmt32);
	else if (fmt16_565)
		initGraphics(size.x, size.y, fmt16_565);
	else if (fmt16_555)
		initGraphics(size.x, size.y, fmt16_555);
	else
		error("Unable to find a suitable graphics format");

	_system->fillScreen(0);

	_runtime.reset(new Runtime(_system, _mixer, midiDrv.get(), _rootFSNode, _gameDescription->gameID, _gameDescription->defaultLanguage));
	_runtime->initSections(_videoRect, _menuBarRect, _trayRect, _subtitleRect, Common::Rect(640, 480), _system->getScreenFormat());

	if (midiDrv)
		midiDrv->setTimerCallback(this, VCruiseEngine::staticHandleMidiTimer);

	const char *exeName = _gameDescription->desc.filesDescriptions[0].fileName;

	if (_gameDescription->desc.flags & VCRUISE_GF_GENTEE_PACKAGE)
		exeName = "Schizm.exe";

	_runtime->loadCursors(exeName);

	if (ConfMan.getBool("vcruise_debug")) {
		_runtime->setDebugMode(true);
	}

	if (ConfMan.getBool("vcruise_fast_animations")) {
		_runtime->setFastAnimationMode(true);
	}

	if (ConfMan.getBool("vcruise_preload_sounds")) {
		_runtime->setPreloadSounds(true);
	}

	if (ConfMan.getBool("vcruise_use_4bit")) {
		_runtime->setLowQualityGraphicsMode(true);
	}

	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0) {
			(void)_runtime->bootGame(false);

			Common::Error err = loadGameState(saveSlot);
			if (err.getCode() != Common::kNoError)
				return err;
		}
	}

	// Run the game
	while (!shouldQuit()) {
		handleEvents();

		if (!_runtime->runFrame())
			break;

		_runtime->drawFrame();
		_system->delayMillis(5);
	}


	if (midiDrv)
		midiDrv->setTimerCallback(nullptr, nullptr);

	_runtime.reset();

	if (midiDrv)
		midiDrv->close();

	if (_gameDescription->desc.flags & VCRUISE_GF_GENTEE_PACKAGE)
		SearchMan.remove("VCruiseInstallerPackage");

	// Flush any settings changes made in-game
	ConfMan.flushToDisk();

	return Common::kNoError;
}

void VCruiseEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
}

bool VCruiseEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsReturnToLauncher:
	case kSupportsSavingDuringRuntime:
	case kSupportsLoadingDuringRuntime:
		return true;
	default:
		return false;
	};
}

void VCruiseEngine::syncSoundSettings() {
	// Sync the engine with the config manager
	int soundVolumeMusic = ConfMan.getInt("music_volume");
	int soundVolumeSFX = ConfMan.getInt("sfx_volume");
	int soundVolumeSpeech = ConfMan.getInt("speech_volume");

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	// We need to handle the speech mute separately here. This is because the
	// engine code should be able to rely on all speech sounds muted when the
	// user specified subtitles only mode, which results in "speech_mute" to
	// be set to "true". The global mute setting has precedence over the
	// speech mute setting though.
	bool speechMute = mute;
	if (!speechMute)
		speechMute = ConfMan.getBool("speech_mute");

	bool muteSound = ConfMan.getBool("vcruise_mute_sound");
	if (ConfMan.hasKey("vcruise_mute_sound"))
		muteSound = ConfMan.getBool("vcruise_mute_sound");

	// We don't mute music here because Schizm has a special behavior that bypasses music mute when using one
	// of the ships to transition zones.
	_mixer->muteSoundType(Audio::Mixer::kPlainSoundType, mute || muteSound);
	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, mute);
	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, mute || muteSound);
	_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, speechMute || muteSound);

	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, Audio::Mixer::kMaxMixerVolume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolumeSFX);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, soundVolumeSpeech);
}

Common::Error VCruiseEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	_runtime->saveGame(stream);

	if (stream->err())
		return Common::Error(Common::kWritingFailed);

	return Common::Error(Common::kNoError);
}

Common::Error VCruiseEngine::loadGameStream(Common::SeekableReadStream *stream) {
	LoadGameOutcome loadGameOutcome = _runtime->loadGame(stream);

	switch (loadGameOutcome) {
	case kLoadGameOutcomeSucceeded:
		return Common::Error(Common::kNoError);
	case kLoadGameOutcomeSaveDataCorrupted: {
			GUI::MessageDialog dialog(_("Failed to load save, the save data appears to be damaged."));
			dialog.runModal();
		}
		return Common::Error(Common::kReadingFailed);
	case kLoadGameOutcomeMissingVersion: {
			GUI::MessageDialog dialog(_("Failed to read version information from save file."));
			dialog.runModal();
		}
		return Common::Error(Common::kReadingFailed);
	case kLoadGameOutcomeInvalidVersion: {
			GUI::MessageDialog dialog(_("Failed to load save, the save file doesn't contain valid version information."));
			dialog.runModal();
		}
		return Common::Error(Common::kReadingFailed);
	case kLoadGameOutcomeSaveIsTooNew: {
			GUI::MessageDialog dialog(_("Saved game was created with a newer version of ScummVM. Unable to load."));
			dialog.runModal();
		}
		return Common::Error(Common::kReadingFailed);
	case kLoadGameOutcomeSaveIsTooOld: {
			GUI::MessageDialog dialog(_("Saved game was created with an earlier, incompatible version of ScummVM. Unable to load."));
			dialog.runModal();
		}
		return Common::Error(Common::kReadingFailed);
	default: {
			GUI::MessageDialog dialog(_("An unknown error occurred while attempting to load the saved game."));
			dialog.runModal();
		}
		return Common::Error(Common::kReadingFailed);
	};

	return Common::Error(Common::kNoError);
}

bool VCruiseEngine::canSaveAutosaveCurrently() {
	return _runtime->canSave(false);
}

bool VCruiseEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return _runtime->canSave(false);
}

bool VCruiseEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return _runtime->canLoad();
}

void VCruiseEngine::initializePath(const Common::FSNode &gamePath) {
	Engine::initializePath(gamePath);

	const char *gameSubPath = nullptr;

	if (_gameDescription->desc.flags & VCRUISE_GF_GENTEE_PACKAGE) {
		if (_gameDescription->gameID == GID_SCHIZM)
			gameSubPath = "Schizm";
	}

	if (gameSubPath) {
		const int kNumCasePasses = 3;

		for (int casePass = 0; casePass < kNumCasePasses; casePass++) {
			Common::String subPathStr(gameSubPath);

			if (casePass == 1)
				subPathStr.toUppercase();
			else if (casePass == 2)
				subPathStr.toLowercase();

			Common::FSNode gameSubDir = gamePath.getChild(subPathStr);
			if (gameSubDir.isDirectory()) {
				SearchMan.addDirectory("VCruiseGameDir", gameSubDir, 0, 3);
				break;
			}

			if (casePass != kNumCasePasses - 1)
				warning("Expected to find subpath '%s' in the game directory but couldn't find it", gameSubPath);
		}
	}

	_rootFSNode = gamePath;
}

bool VCruiseEngine::hasDefaultSave() {
	return hasAnySave();
}

bool VCruiseEngine::hasAnySave() {
	Common::StringArray saveFiles = getSaveFileManager()->listSavefiles(_targetName + ".*");
	return saveFiles.size() > 0;
}

Common::Error VCruiseEngine::loadMostRecentSave() {
	Common::SaveFileManager *sfm = getSaveFileManager();

	Common::StringArray saveFiles = sfm->listSavefiles(_targetName + ".*");

	uint64 highestDateTime = 0;
	uint32 highestPlayTime = 0;
	Common::String highestSaveFileName;

	for (const Common::String &saveFileName : saveFiles) {
		Common::InSaveFile *saveFile = sfm->openForLoading(saveFileName);

		if (!saveFile) {
			warning("Couldn't load save file '%s' to determine recency", saveFileName.c_str());
			continue;
		}

		ExtendedSavegameHeader header;
		bool loadedHeader = getMetaEngine()->readSavegameHeader(saveFile, &header, true);
		if (!loadedHeader) {
			warning("Couldn't parse header from '%s'", saveFileName.c_str());
			continue;
		}

		uint8 day = 0;
		uint8 month = 0;
		uint16 year = 0;

		uint8 hour = 0;
		uint8 minute = 0;

		MetaEngine::decodeSavegameDate(&header, year, month, day);
		MetaEngine::decodeSavegameTime(&header, hour, minute);

		uint64 dateTime = static_cast<uint64>(year) << 32;
		dateTime |= static_cast<uint64>(month) << 24;
		dateTime |= static_cast<uint64>(day) << 16;
		dateTime |= static_cast<uint64>(hour) << 8;
		dateTime |= minute;

		uint32 playTime = header.playtime;

		if (dateTime > highestDateTime || (dateTime == highestDateTime && playTime > highestPlayTime)) {
			highestSaveFileName = saveFileName;
			highestDateTime = dateTime;
			highestPlayTime = playTime;
		}
	}

	if (highestSaveFileName.empty()) {
		warning("Couldn't find any valid saves to load");
		return Common::Error(Common::kReadingFailed);
	}

	Common::String slotStr = highestSaveFileName.substr(_targetName.size() + 1);

	int slot = 0;
	if (sscanf(slotStr.c_str(), "%i", &slot) != 1) {
		warning("Couldn't parse save slot ID from %s", highestSaveFileName.c_str());
		return Common::Error(Common::kReadingFailed);
	}

	return loadGameState(slot);
}


} // End of namespace VCruise
