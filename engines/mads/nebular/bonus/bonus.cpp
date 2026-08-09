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
 */

#include "mads/nebular/bonus/bonus.h"

#include "common/array.h"
#include "common/file.h"
#include "common/func.h"
#include "common/path.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "engines/util.h"
#include "graphics/pixelformat.h"
#include "graphics/screen.h"
#include "gui/message.h"
#include "mads/animview/animview.h"
#include "mads/core/env.h"
#include "mads/core/game.h"
#include "mads/core/screen.h"
#include "mads/nebular/bonus/bonus_exe_data.h"
#include "mads/nebular/bonus/bonus_text_ui.h"
#include "mads/nebular/console.h"
#include "mads/nebular/sound/sound.h"

namespace MADS {
namespace RexNebular {

namespace {

struct BonusTrack {
	byte section;
	byte command;
	// BONUS uses one cross-driver command table. For ISOUND, false entries
	// either have no sequence or point directly at the native 00 00 terminator;
	// do not substitute a nearby gameplay sound without executable evidence.
	bool pcSpeakerAvailable;
};

static const BonusTrack kBonusTracks[] = {
	{ 1, 12, false }, { 1, 10, false }, { 9, 49, false }, { 2, 9, false },
	{ 2, 15, false }, { 2, 19, true }, { 3, 11, false }, { 3, 10, false },
	{ 3, 16, false }, { 3, 17, false }, { 4, 12, false }, { 5, 29, false },
	{ 6, 24, false }, { 7, 24, false }, { 7, 27, true }, { 7, 9, false }
};

static void showError(const Common::String &message) {
	GUI::MessageDialog dialog(message);
	dialog.runModal();
}

static bool containsIgnoreCase(const Common::Array<Common::String> &strings,
		const Common::String &value) {
	for (uint i = 0; i < strings.size(); ++i) {
		if (strings[i].equalsIgnoreCase(value))
			return true;
	}
	return false;
}

static bool reportMissingFiles(const char *action,
		const Common::Array<Common::String> &missingFiles) {
	if (missingFiles.empty())
		return true;

	Common::String names;
	for (uint i = 0; i < missingFiles.size(); ++i) {
		Common::String displayName = missingFiles[i];
		displayName.toUppercase();
		warning("MADS Bonus: %s requires missing file %s",
				action, displayName.c_str());
		if (!names.empty())
			names += '\n';
		names += displayName;
	}

	Common::String message = Common::String::format(
			"The Rex Nebular Bonus Disk cannot %s because the following "
			"file%s missing:\n\n%s",
			action, missingFiles.size() == 1 ? " is" : "s are", names.c_str());
	showError(message);
	return false;
}

static const char *skipResourceFlags(const char *line) {
	while (*line == '/' || *line == '-') {
		const char *space = strchr(line, ' ');
		if (!space)
			return line + strlen(line);
		line = space;
		while (*line == ' ')
			++line;
	}
	return line;
}

static bool checkPresentationFiles(const char *resourceName,
		const char *action) {
	Common::Array<Common::String> missingFiles;
	Common::File resource;
	if (!resource.open(resourceName)) {
		missingFiles.push_back(resourceName);
		return reportMissingFiles(action, missingFiles);
	}

	while (!resource.eos() && !resource.err()) {
		Common::String line = resource.readLine();
		line.trim();
		if (line.empty())
			continue;

		const char *animationName = skipResourceFlags(line.c_str());
		if (!*animationName)
			continue;

		Common::String lookupName(animationName);
		if (!strchr(lookupName.c_str(), '.'))
			lookupName += ".aa";
		lookupName = Common::String("*") + lookupName;

		char dataFilename[80] = { 0 };
		if (!env_get_path(dataFilename, lookupName.c_str())) {
			if (!containsIgnoreCase(missingFiles, animationName))
				missingFiles.push_back(animationName);
			continue;
		}
		if (!Common::File::exists(dataFilename) &&
				!containsIgnoreCase(missingFiles, dataFilename))
			missingFiles.push_back(dataFilename);
	}

	return reportMissingFiles(action, missingFiles);
}

class BonusApplication {
public:
	explicit BonusApplication(Sound::RexSoundManager &soundManager) :
			_ui(_text), _soundManager(soundManager),
			_mainSelection(0), _musicSelection(0),
			_section3ExtraCommand(false) {
	}

	bool init(Common::String &errorMessage) {
		if (!_text.load("bonus.exe", errorMessage))
			return false;
		return _ui.init(errorMessage);
	}

	void run() {
		while (!g_engine->shouldQuit()) {
			switch (_ui.runMainMenu(_mainSelection)) {
			case BonusTextUI::kDeathScenes:
				if (checkPresentationFiles("death.res", "show the death scenes"))
					AnimView::animview_main("@death");
				break;
			case BonusTextUI::kEvolution:
				if (checkPresentationFiles("evolve.res", "show the evolution sequence"))
					AnimView::animview_main("@evolve");
				break;
			case BonusTextUI::kSets:
				if (checkPresentationFiles("sets.res", "show the Rex Nebular sets"))
					AnimView::animview_main("@sets");
				break;
			case BonusTextUI::kMusic:
				runMusicMenu();
				break;
			case BonusTextUI::kCoolStuff:
				showBonusText();
				break;
			case BonusTextUI::kExit:
				_ui.showGoodbye();
				g_engine->quitGame();
				return;
			case BonusTextUI::kAbort:
				return;
			}
		}
	}

private:
	BonusExeData _text;
	BonusTextUI _ui;
	Sound::RexSoundManager &_soundManager;
	int _mainSelection;
	int _musicSelection;
	bool _section3ExtraCommand;

	void runMusicMenu() {
		bool enabled[ARRAYSIZE(kBonusTracks) + 1];
		const bool pcSpeaker = _soundManager.usesPCSpeaker();
		for (uint index = 0; index < ARRAYSIZE(kBonusTracks); ++index)
			enabled[index] = !pcSpeaker || kBonusTracks[index].pcSpeakerAvailable;
		enabled[ARRAYSIZE(kBonusTracks)] = true;

		while (!g_engine->shouldQuit()) {
			const int selected = _ui.runMusicMenu(_musicSelection,
					pcSpeaker ? enabled : nullptr);
			if (selected < 0 || selected == (int)ARRAYSIZE(kBonusTracks))
				return;
			playTrack(selected);
		}
	}

	void playTrack(int index) {
		if (index < 0 || index >= (int)ARRAYSIZE(kBonusTracks))
			return;

		const BonusTrack &track = kBonusTracks[index];
		if (_soundManager.usesPCSpeaker() &&
				!track.pcSpeakerAvailable)
			return;

		// Present the card before loading the driver. This keeps the final
		// AnimView frame from leaking through while a driver is starting.
		_ui.prepareNowPlaying(_text.musicTitles[index]);
		_soundManager.init(track.section);

		// ISOUND initializes with a one-tick null sequence. The original host
		// services that sequence before dispatching a menu selection; wait on
		// the driver's own completion status instead of racing its priority.
		const uint32 startupTime = g_system->getMillis();
		while (_soundManager.isDriverActive() &&
				!g_engine->shouldQuit() &&
				g_system->getMillis() - startupTime < 1000)
			g_system->delayMillis(1);

		_soundManager.command(track.command, 127);

		// These follow the control flow surrounding the native track table in
		// BONUS.EXE; they are player commands, not additional track entries.
		if (track.section == 2 && track.command == 9) {
			_soundManager.command(17);
		} else if (track.section == 3 && track.command == 16) {
			_section3ExtraCommand = !_section3ExtraCommand;
			if (!_section3ExtraCommand)
				_soundManager.command(16);
		} else if (track.section == 7 && track.command == 9 &&
				!_soundManager.isDriverActive()) {
			_soundManager.command(25, 127);
		}

		Common::Functor0Mem<bool, BonusApplication> isPlaying(
				this, &BonusApplication::isDriverActive);
		_ui.waitForNowPlaying(_text.musicTitles[index], isPlaying);
		if (_soundManager.isLoaded())
			_soundManager.closeDriver();
	}

	bool isDriverActive() {
		return _soundManager.isDriverActive();
	}

	void showBonusText() {
		if (_ui.showBonusText(Common::Path(_text.bonusTextFilename)) ||
				g_engine->shouldQuit())
			return;

		Common::Array<Common::String> missingFiles;
		missingFiles.push_back(_text.bonusTextFilename);
		reportMissingFiles("show the Cool Stuff list", missingFiles);
	}
};

} // namespace

BonusEngine::BonusEngine(OSystem *syst, const MADSGameDescription *gameDesc) :
		RexNebularEngine(syst, gameDesc) {
}

BonusEngine::~BonusEngine() {
	_presentation.free();
}

Common::Error BonusEngine::run() {
	initGraphics(640, 400);
	applyGameSettings();

	_screen = new Graphics::Screen(320, 200);
	scr_live.data = (byte *)_screen->getPixels();
	_presentation.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());

	setDebugger(new Console());

	if (!Common::File::exists("bonus.exe")) {
		warning("MADS Bonus: required file BONUS.EXE is missing");
		return Common::Error(Common::kNoGameDataFoundError,
				"The Rex Nebular Bonus Disk requires BONUS.EXE");
	}

	if (!Common::File::exists("section9.hag"))
		warning("SECTION9.HAG not found. The Bonus Disk requires data from the full game.");

	readConfigFile();
	env_search_mode = ENV_SEARCH_CONCAT_FILES;
	art_hags_are_on_hd = true;

	Sound::RexSoundManager *soundManager =
			new Sound::RexSoundManager(_mixer, _soundFlag, false);
	_soundManager = soundManager;
	soundManager->validate();

	BonusApplication application(*soundManager);
	Common::String errorMessage;
	if (!application.init(errorMessage))
		return Common::Error(Common::kNoGameDataFoundError, errorMessage);

	application.run();
	return Common::kNoError;
}

bool BonusEngine::hasFeature(EngineFeature feature) const {
	return feature == kSupportsReturnToLauncher;
}

Common::Point BonusEngine::screenToGame(const Common::Point &point) const {
	return Common::Point(CLIP<int>(point.x / 2, 0, 319),
			CLIP<int>(point.y / 2, 0, 199));
}

Common::Point BonusEngine::gameToScreen(const Common::Point &point) const {
	return Common::Point(point.x * 2, point.y * 2);
}

void BonusEngine::presentScreen(int shakeOffset) {
	const int shift = shakeOffset > 0 ? shakeOffset % 320 : 0;
	for (int y = 0; y < 200; ++y) {
		const byte *source = (const byte *)_screen->getBasePtr(0, y);
		byte *line1 = (byte *)_presentation.getBasePtr(0, y * 2);
		byte *line2 = (byte *)_presentation.getBasePtr(0, y * 2 + 1);

		for (int x = 0; x < 320; ++x) {
			const byte color = source[(x + shift) % 320];
			line1[x * 2] = color;
			line1[x * 2 + 1] = color;
		}
		memcpy(line2, line1, 640);
	}

	g_system->copyRectToScreen(_presentation.getPixels(), _presentation.pitch,
			0, 0, 640, 400);
	g_system->updateScreen();
}

} // namespace RexNebular
} // namespace MADS
