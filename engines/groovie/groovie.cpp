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

#include "groovie/groovie.h"
#include "groovie/cursor.h"
#include "groovie/detection.h"
#include "groovie/graphics.h"
#include "groovie/script.h"
#include "groovie/music.h"
#include "groovie/resource.h"
#include "groovie/video/vdx.h"

#ifdef ENABLE_GROOVIE2
#include "groovie/video/roq.h"
#endif

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/stuffit.h"
#include "common/textconsole.h"

#include "backends/audiocd/audiocd.h"
#include "engines/util.h"
#include "graphics/fontman.h"
#include "audio/mixer.h"

namespace Groovie {

const int GroovieEngine::AUTOSAVE_SLOT = MAX_SAVES - 1;

GroovieEngine::GroovieEngine(OSystem *syst, const GroovieGameDescription *gd) :
	Engine(syst), _gameDescription(gd), _script(nullptr),
	_resMan(nullptr), _grvCursorMan(nullptr), _videoPlayer(nullptr), _musicPlayer(nullptr),
	_graphicsMan(nullptr), _macResFork(nullptr), _waitingForInput(false), _font(nullptr),
	_spookyMode(false) {

	// Adding the default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "groovie");
	SearchMan.addSubDirectoryMatching(gameDataDir, "media");
	SearchMan.addSubDirectoryMatching(gameDataDir, "system");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MIDI");

	_modeSpeed = kGroovieSpeedNormal;
	if (ConfMan.hasKey("fast_movie_speed") && ConfMan.getBool("fast_movie_speed"))
		_modeSpeed = kGroovieSpeedFast;
}

GroovieEngine::~GroovieEngine() {
	// Delete the remaining objects
	delete _resMan;
	delete _grvCursorMan;
	delete _videoPlayer;
	delete _musicPlayer;
	delete _graphicsMan;
	delete _script;
	delete _macResFork;
}

Common::Error GroovieEngine::run() {
	if (_gameDescription->version == kGroovieT11H && getPlatform() == Common::kPlatformMacintosh) {
		// Load the Mac installer with the lowest priority (in case the user has installed
		// the game and has the MIDI folder present; faster to just load them)
		Common::Archive *archive = Common::createStuffItArchive("The 11th Hour Installer");

		if (archive)
			SearchMan.add("The 11th Hour Installer", archive);
	}

	// TODO: remove this default logging when we're done testing?
	DebugMan.enableDebugChannel(kDebugScript);
	DebugMan.enableDebugChannel(kDebugScriptvars);
	DebugMan.enableDebugChannel(kDebugLogic);
	DebugMan.enableDebugChannel(kDebugVideo);
	if (gDebugLevel < 0)
		gDebugLevel = 0;

	_script = new Script(this, _gameDescription->version);

	// Initialize the graphics
	switch (_gameDescription->version) {
	case kGroovieT11H:
	case kGroovieCDY:
	case kGroovieUHP:
	case kGroovieTLC: {
		// Request the mode with the highest precision available
		Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
		initGraphics(640, 480, &format);

		if (_system->getScreenFormat() != format)
			return Common::kUnsupportedColorMode;

		// Save the enabled mode
		_pixelFormat = format;
		break;
	}
	case kGroovieT7G:
		initGraphics(640, 480);
		_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
		break;

	default:
		error("GROOVIE: Unknown Game version. groovie.cpp:run()");
	}

	// Create debugger. It requires GFX to be initialized
	Debugger *debugger = new Debugger(this);
	setDebugger(debugger);
	_script->setDebugger(debugger);

	// Create the graphics manager
	_graphicsMan = new GraphicsMan(this);

	// Create the resource and cursor managers and the video player
	// Prepare the font too
	switch (_gameDescription->version) {
	case kGroovieT7G:
		if (getPlatform() == Common::kPlatformMacintosh) {
			_macResFork = new Common::MacResManager();
			if (!_macResFork->open(_gameDescription->desc.filesDescriptions[0].fileName))
				error("Could not open %s as a resource fork", _gameDescription->desc.filesDescriptions[0].fileName);
			// The Macintosh release used system fonts. We use GUI fonts.
			_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		} else {
			Common::File fontfile;
			if (!fontfile.open("sphinx.fnt")) {
				error("Couldn't open sphinx.fnt");
				return Common::kNoGameDataFoundError;
			} else if (!_sphinxFont.load(fontfile)) {
				error("Error loading sphinx.fnt");
				return Common::kUnknownError;
			}
			fontfile.close();
			_font = &_sphinxFont;
		}

		_resMan = new ResMan_t7g(_macResFork);
		_grvCursorMan = new GrvCursorMan_t7g(_system, _macResFork);
		_videoPlayer = new VDXPlayer(this);
		break;

	case kGroovieT11H:
	case kGroovieCDY:
	case kGroovieUHP:
	case kGroovieTLC:
		_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		_resMan = new ResMan_v2();
		_grvCursorMan = new GrvCursorMan_v2(_system);
#ifdef ENABLE_GROOVIE2
		_videoPlayer = new ROQPlayer(this);
		_soundQueue.setVM(this);
#endif
		break;

	default:
		error("GROOVIE: Unknown Game version. groovie.cpp:run()");
	}


	switch (_gameDescription->version) {
	case kGroovieT7G:
		// Detect ScummVM Music Enhancement Project presence (T7G only)
		if (Common::File::exists("gu16.ogg")) {
			// Load player for external files
			_musicPlayer = new MusicPlayerIOS(this);
			break;
		}
		// else, fall through
	case kGroovieT11H:
		// Create the music player
		switch (getPlatform()) {
		case Common::kPlatformMacintosh:
			if (_gameDescription->version == kGroovieT7G)
				_musicPlayer = new MusicPlayerMac_t7g(this);
			else
				_musicPlayer = new MusicPlayerMac_v2(this);
			break;
		case Common::kPlatformIOS:
			_musicPlayer = new MusicPlayerIOS(this);
			break;
		default:
			_musicPlayer = new MusicPlayerXMI(this, _gameDescription->version == kGroovieT7G ? "fat" : "sample");
			break;
		}
		break;

	case kGroovieCDY:
	case kGroovieUHP:
		_musicPlayer = new MusicPlayerClan(this);
		break;

	case kGroovieTLC:
		_musicPlayer = new MusicPlayerTlc(this);
		break;
	}

	// Load volume levels
	syncSoundSettings();

	// Get the name of the main script
	Common::String filename;
	if (_gameDescription->version == kGroovieT7G) {
		filename = "script.grv";
		// Run The 7th Guest's demo if requested
		if (ConfMan.hasKey("demo_mode") && ConfMan.getBool("demo_mode"))
			filename = "demo.grv";
	} else {
		filename = _gameDescription->desc.filesDescriptions[0].fileName;
	}

	// Check the script file extension
	if (!filename.hasSuffixIgnoreCase(".grv")) {
		error("%s isn't a valid script filename", filename.c_str());
		return Common::kUnknownError;
	}

	// Load the script
	if (!_script->loadScript(filename)) {
		error("Couldn't load the script file %s", filename.c_str());
		return Common::kUnknownError;
	}

	// Should I load a saved game?
	if (ConfMan.hasKey("save_slot")) {
		// Get the requested slot
		int slot = ConfMan.getInt("save_slot");
		_script->directGameLoad(slot);
	}

	// Game timer counter
	int tmr = 0;

	// Check that the game files and the audio tracks aren't together run from
	// the same cd
	if (getPlatform() != Common::kPlatformIOS && _gameDescription->version == kGroovieT7G) {
		if (!existExtractedCDAudioFiles()
		    && !isDataAndCDAudioReadFromSameCD()) {
			warnMissingExtractedCDAudio();
		}
		_system->getAudioCDManager()->open();
	}

	while (!shouldQuit()) {
		// Handle input
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_KEYDOWN:
				// Send the event to the scripts
				_script->setKbdChar(ev.kbd.ascii);

				// Continue the script execution to handle the key
				_waitingForInput = false;
				break;

			case Common::EVENT_MAINMENU:
				// Closing the GMM
			case Common::EVENT_MOUSEMOVE:
				// Continue the script execution, the mouse pointer
				// may fall inside a different hotspot now
				_waitingForInput = false;
				break;

			case Common::EVENT_LBUTTONDOWN:
				// Send the event to the scripts
				_script->setMouseClick(1);

				// Continue the script execution to handle
				// the click
				_waitingForInput = false;
				break;

			case Common::EVENT_RBUTTONDOWN:
				// Send the event to the scripts (to skip the video)
				_script->setMouseClick(2);
				break;

			case Common::EVENT_QUIT:
				quitGame();
				break;

			default:
				break;
			}
		}

		// The event loop may have triggered the quit status. In this case,
		// stop the execution.
		if (shouldQuit()) {
			continue;
		}

		if (_graphicsMan->isFading()) {
			// We're waiting for a fading to end, let the CPU rest
			// for a while and continue
			_system->delayMillis(10);
		} else {
			if (_waitingForInput) {
				// Still waiting for input, just update the mouse, game timer and then wait a bit more
				_grvCursorMan->animate();
				_system->updateScreen();

				// Wait a little bit between increments.  While mouse is moving, this triggers
				// only negligably slower.
				if (tmr >= 500) {
					_script->timerTick();
					tmr = 0;
				}

				_system->delayMillis(10);
				tmr += 10;

				// the script doesn't unset _waitingForInput
				// so we unset it here in order to let the script run as many steps as it needs to
				// this makes the game more responsive
				_waitingForInput = false;
			}

			// Everything's fine, execute another script step
			_script->step();
		}

		// Update the screen if required
		_graphicsMan->update();
		_soundQueue.tick();
	}

	return Common::kNoError;
}

void GroovieEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (_musicPlayer)
		_musicPlayer->pause(pause);
}

Common::Platform GroovieEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

EngineVersion GroovieEngine::getEngineVersion() const {
	return _gameDescription->version;
}

bool GroovieEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsLoadingDuringRuntime);
}

bool GroovieEngine::canLaunchLoad() const {
	if (_gameDescription->desc.guiOptions == nullptr)
		return false;
	return strstr(_gameDescription->desc.guiOptions, GUIO_NOLAUNCHLOAD) != nullptr;
}

bool GroovieEngine::isDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

void GroovieEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	bool mute = ConfMan.getBool("mute");

	// Set the music volume
	_musicPlayer->setUserVolume(mute ? 0 : ConfMan.getInt("music_volume"));

	// Videos just contain one digital audio track, which can be used for
	// both SFX or Speech, but the engine doesn't know what they contain, so
	// we have to use just one volume setting for videos.
	// We use "speech" because most users will want to change the videos
	// volume when they can't hear the speech because of the music.
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType,
		mute ? 0 : ConfMan.getInt("speech_volume"));
}

bool GroovieEngine::canLoadGameStateCurrently() {
	// TODO: verify the engine has been initialized
	if (isDemo())
		return false;
	if (_script)
		return true;
	else
		return false;
}

bool GroovieEngine::canSaveGameStateCurrently() {
	// TODO: verify the engine has been initialized
	if (isDemo())
		return false;
	if (_script)
		return _script->canDirectSave();
	else
		return false;
}

Common::Error GroovieEngine::loadGameState(int slot) {
	_script->directGameLoad(slot);

	// TODO: Use specific error codes
	return Common::kNoError;
}

Common::Error GroovieEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	_script->directGameSave(slot,desc);

	// TODO: Use specific error codes
	return Common::kNoError;
}

int GroovieEngine::getAutosaveSlot() const {
	return AUTOSAVE_SLOT;
}

void GroovieEngine::waitForInput() {
	_waitingForInput = true;
}

SoundEffectQueue::SoundEffectQueue() {
	_vm = nullptr;
	_player = nullptr;
	_file = nullptr;
}

void SoundEffectQueue::setVM(GroovieEngine *vm) {
	_vm = vm;
#ifdef ENABLE_GROOVIE2
	_player = new ROQSoundPlayer(vm);
#endif
}

void SoundEffectQueue::queue(Common::SeekableReadStream *soundfile, uint32 loops) {
	if (_queue.size() > 20) {
		stopAll();
	}
	_queue.push({soundfile, loops});
	for (uint32 i = 1; i < loops; i++) {
		_queue.push({soundfile, loops});
	}
	tick();
}

void SoundEffectQueue::tick() {
#ifdef ENABLE_GROOVIE2
	if (_file && !_player->playFrame()) {
		_vm->_script->setBitFlag(0, true);
		return;
	}
	if (_queue.size() == 0) {
		deleteFile();
		return;
	}

	SoundQueueEntry entry = _queue.front();
	if (entry._loops != 0 || _queue.size() > 1) {
		_queue.pop();
	}
	if (_file != entry._file) {
		deleteFile();
	}
	_file = entry._file;

	_vm->_script->setBitFlag(0, true);
	_file->seek(0);
	_player->load(_file, 0);
	_player->playFrame();
	if (_player->isFastForwarding()) {
		stopAll();
	}
#endif
}

void SoundEffectQueue::deleteFile() {
	if (_file) {
		delete _file;
		_file = nullptr;
		_vm->_script->setBitFlag(0, false);
	}
}

void SoundEffectQueue::stopAll() {
	if (_file && _player) {
		_player->stopAudioStream();
	}
	_queue.clear();
	deleteFile();
}

} // End of namespace Groovie
