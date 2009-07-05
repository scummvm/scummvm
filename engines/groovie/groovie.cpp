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

#include "common/config-manager.h"
#include "common/events.h"
#include "sound/mixer.h"

#include "groovie/groovie.h"
#include "groovie/music.h"
#include "groovie/roq.h"
#include "groovie/vdx.h"

namespace Groovie {

GroovieEngine::GroovieEngine(OSystem *syst, const GroovieGameDescription *gd) :
	Engine(syst), _gameDescription(gd), _debugger(NULL), _script(this, gd->version),
	_resMan(NULL), _grvCursorMan(NULL), _videoPlayer(NULL), _musicPlayer(NULL),
	_graphicsMan(NULL), _waitingForInput(false) {

	// Adding the default directories
	Common::File::addDefaultDirectory(_gameDataDir.getChild("groovie"));
	Common::File::addDefaultDirectory(_gameDataDir.getChild("media"));
	Common::File::addDefaultDirectory(_gameDataDir.getChild("system"));

	// Initialize the custom debug levels
	Common::addDebugChannel(kGroovieDebugAll, "All", "Debug everything");
	Common::addDebugChannel(kGroovieDebugVideo, "Video", "Debug video and audio playback");
	Common::addDebugChannel(kGroovieDebugResource, "Resource", "Debug resouce management");
	Common::addDebugChannel(kGroovieDebugScript, "Script", "Debug the scripts");
	Common::addDebugChannel(kGroovieDebugUnknown, "Unknown", "Report values of unknown data in files");
	Common::addDebugChannel(kGroovieDebugHotspots, "Hotspots", "Show the hotspots");
	Common::addDebugChannel(kGroovieDebugCursor, "Cursor", "Debug cursor decompression / switching");
	Common::addDebugChannel(kGroovieDebugMIDI, "MIDI", "Debug MIDI / XMIDI files");
	Common::addDebugChannel(kGroovieDebugScriptvars, "Scriptvars", "Print out any change to script variables");
	Common::addDebugChannel(kGroovieDebugCell, "Cell", "Debug the cell game (in the microscope)");
	Common::addDebugChannel(kGroovieDebugFast, "Fast", "Play videos quickly, with no sound (unstable)");
}

GroovieEngine::~GroovieEngine() {
	// Delete the remaining objects
	delete _debugger;
	delete _resMan;
	delete _grvCursorMan;
	delete _videoPlayer;
	delete _musicPlayer;
	delete _graphicsMan;
}

Common::Error GroovieEngine::run() {
	// Initialize the graphics
	initGraphics(640, 480, true);

	// Create debugger. It requires GFX to be initialized
	_debugger = new Debugger(this);
	_script.setDebugger(_debugger);

	// Create the graphics manager
	_graphicsMan = new GraphicsMan(this);

	// Create the resource and cursor managers and the video player
	switch (_gameDescription->version) {
	case kGroovieT7G:
		_resMan = new ResMan_t7g();
		_grvCursorMan = new GrvCursorMan_t7g(_system);
		_videoPlayer = new VDXPlayer(this);
		break;
	case kGroovieV2:
		_resMan = new ResMan_v2();
		_grvCursorMan = new GrvCursorMan_v2(_system);
		_videoPlayer = new ROQPlayer(this);
		break;
	}

	// Create the music player
	if (_gameDescription->desc.platform == Common::kPlatformMacintosh) {
		_musicPlayer = new MusicPlayerMac(this);
	} else {
		_musicPlayer = new MusicPlayerXMI(this, _gameDescription->version == kGroovieT7G ? "fat" : "sample");
	}

	// Load volume levels
	syncSoundSettings();

	// Get the name of the main script
	Common::String filename = _gameDescription->desc.filesDescriptions[0].fileName;
	if (_gameDescription->version == kGroovieT7G) {
		// Run The 7th Guest's demo if requested
		if (ConfMan.hasKey("demo_mode") && ConfMan.getBool("demo_mode")) {
			filename = Common::String("demo.grv");
		}
	} else if (_gameDescription->version == kGroovieV2) {
		// Open the disk index
		Common::File disk;
		if (!disk.open(filename)) {
			error("Couldn't open %s", filename.c_str());
			return Common::kNoGameDataFoundError;
		}

		// Search the entry
		bool found = false;
		int index = 0;
		while (!found && !disk.eos()) {
			Common::String line = disk.readLine();
			if (line.hasPrefix("title: ")) {
				// A new entry
				index++;
			} else if (line.hasPrefix("boot: ") && index == _gameDescription->indexEntry) {
				// It's the boot of the entry we're looking for,
				// get the script filename
				filename = line.c_str() + 6;
				found = true;
			}
		}

		// Couldn't find the entry
		if (!found) {
			error("Couldn't find entry %d in %s", _gameDescription->indexEntry, filename.c_str());
			return Common::kUnknownError;
		}
	}

	// Check the script file extension
	if (!filename.hasSuffix(".grv")) {
		error("%s isn't a valid script filename", filename.c_str());
		return Common::kUnknownError;
	}

	// Load the script
	if (!_script.loadScript(filename)) {
		error("Couldn't load the script file %s", filename.c_str());
		return Common::kUnknownError;
	}

	// Should I load a saved game?
	if (ConfMan.hasKey("save_slot")) {
		// Get the requested slot
		int slot = ConfMan.getInt("save_slot");
		_script.directGameLoad(slot);
	}

	// Check that the game files and the audio tracks aren't together run from
	// the same cd
	checkCD();

	// Game timer counter
	uint16 tmr = 0;

	// Initialize the CD
	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);

	while (!shouldQuit()) {
		// Show the debugger if required
		if (_debugger->isAttached()) {
			_debugger->onFrame();
		}

		// Handle input
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_KEYDOWN:
				// CTRL-D: Attach the debugger
				if ((ev.kbd.flags & Common::KBD_CTRL) && ev.kbd.keycode == Common::KEYCODE_d)
					_debugger->attach();

				// Send the event to the scripts
				_script.setKbdChar(ev.kbd.ascii);

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
				_script.setMouseClick();

				// Continue the script execution to handle
				// the click
				_waitingForInput = false;
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

		if (_waitingForInput) {
			// Still waiting for input, just update the mouse, game timer and then wait a bit more
			_grvCursorMan->animate();
			_system->updateScreen();
			tmr++;
			// Wait a little bit between increments.  While mouse is moving, this triggers
			// only negligably slower.
			if (tmr > 4) {
				_script.timerTick();
				tmr = 0;
			}

			_system->delayMillis(50);
		} else if (_graphicsMan->isFading()) {
			// We're waiting for a fading to end, let the CPU rest
			// for a while and continue
			_system->delayMillis(30);
		} else {
			// Everything's fine, execute another script step
			_script.step();
		}

		// Update the screen if required
		_graphicsMan->update();
	}

	return Common::kNoError;
}

bool GroovieEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime);
}

void GroovieEngine::errorString(const char *buf_input, char *buf_output, int buf_output_size) {
	//snprintf(buf_output, buf_output_size, "%s%s\n", _script.getContext().c_str(), buf_input);
	snprintf(buf_output, buf_output_size, "%s", buf_input);
}

void GroovieEngine::syncSoundSettings() {
	_musicPlayer->setUserVolume(ConfMan.getInt("music_volume"));
	// VDX videos just contain one digital audio track, which can be used for
	// both SFX or Speech, but the engine doesn't know what they contain, so
	// we have to use just one volume setting for videos.
	// We use "speech" because most users will want to change the videos
	// volume when they can't hear the speech because of the music.
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, ConfMan.getInt("speech_volume"));
}

bool GroovieEngine::canLoadGameStateCurrently() {
	// TODO: verify the engine has been initialized
	return true;
}

Common::Error GroovieEngine::loadGameState(int slot) {
	_script.directGameLoad(slot);

	// TODO: Use specific error codes
	return Common::kNoError;
}

void GroovieEngine::waitForInput() {
	_waitingForInput = true;
}

} // End of namespace Groovie
