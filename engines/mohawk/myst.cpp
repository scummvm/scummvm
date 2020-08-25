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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/gui_options.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/textconsole.h"

#include "engines/dialogs.h"
#include "gui/saveload.h"

#include "mohawk/cursors.h"
#include "mohawk/dialogs.h"
#include "mohawk/myst.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_card.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_scripts.h"
#include "mohawk/myst_sound.h"
#include "mohawk/myst_state.h"
#include "mohawk/resource.h"
#include "mohawk/resource_cache.h"
#include "mohawk/video.h"

// The stacks
#include "mohawk/myst_stacks/channelwood.h"
#include "mohawk/myst_stacks/credits.h"
#include "mohawk/myst_stacks/demo.h"
#include "mohawk/myst_stacks/dni.h"
#include "mohawk/myst_stacks/intro.h"
#include "mohawk/myst_stacks/makingof.h"
#include "mohawk/myst_stacks/mechanical.h"
#include "mohawk/myst_stacks/menu.h"
#include "mohawk/myst_stacks/myst.h"
#include "mohawk/myst_stacks/preview.h"
#include "mohawk/myst_stacks/selenitic.h"
#include "mohawk/myst_stacks/slides.h"
#include "mohawk/myst_stacks/stoneship.h"

// Common files for detection & engines
#include "mohawk/myst_metaengine.h"

namespace Mohawk {

MohawkEngine_Myst::MohawkEngine_Myst(OSystem *syst, const MohawkGameDescription *gamedesc) :
		MohawkEngine(syst, gamedesc) {
	DebugMan.addDebugChannel(kDebugVariable, "Variable", "Track Variable Accesses");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
	DebugMan.addDebugChannel(kDebugView, "View", "Track Card File (VIEW) Parsing");
	DebugMan.addDebugChannel(kDebugHint, "Hint", "Track Cursor Hints (HINT) Parsing");
	DebugMan.addDebugChannel(kDebugResource, "Resource", "Track Resource (RLST) Parsing");
	DebugMan.addDebugChannel(kDebugINIT, "Init", "Track Card Init Script (INIT) Parsing");
	DebugMan.addDebugChannel(kDebugEXIT, "Exit", "Track Card Exit Script (EXIT) Parsing");
	DebugMan.addDebugChannel(kDebugScript, "Script", "Track Script Execution");
	DebugMan.addDebugChannel(kDebugHelp, "Help", "Track Help File (HELP) Parsing");
	DebugMan.addDebugChannel(kDebugCache, "Cache", "Track Resource Cache Accesses");

	_currentCursor = 0;
	_mainCursor = kDefaultMystCursor;
	_showResourceRects = false;
	_scheduledAction = kMystActionNone;
	_currentLanguage = Common::UNK_LANG;
	_currentLanguage = getLanguage();

	_sound = nullptr;
	_video = nullptr;
	_gfx = nullptr;
	_gameState = nullptr;
	_rnd = nullptr;

	_mouseClicked = false;
	_mouseMoved = false;
	_escapePressed = false;
	_waitingOnBlockingOperation = false;

	// We have a custom GMM subclass to show the credits when quitting
	// and to support the drop page and other actions in the options dialog.
	assert(!_mainMenuDialog);
	_mainMenuDialog = new MystMenuDialog(this);
}

MohawkEngine_Myst::~MohawkEngine_Myst() {
	DebugMan.clearAllDebugChannels();

	delete _gfx;
	delete _video;
	delete _sound;
	delete _gameState;
	delete _rnd;
}

// Uses cached data objects in preference to disk access
Common::SeekableReadStream *MohawkEngine_Myst::getResource(uint32 tag, uint16 id) {
	Common::SeekableReadStream *ret = _cache.search(tag, id);

	if (ret)
		return ret;

	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id)) {
			ret = _mhk[i]->getResource(tag, id);
			_cache.add(tag, id, ret);
			return ret;
		}

	error("Could not find a \'%s\' resource with ID %04x", tag2str(tag), id);
}

Common::Array<uint16> MohawkEngine_Myst::getResourceIDList(uint32 type) const {
	Common::Array<uint16> ids;

	for (uint i = 0; i < _mhk.size(); i++) {
		ids.push_back(_mhk[i]->getResourceIDList(type));
	}

	return ids;
}

void MohawkEngine_Myst::cachePreload(uint32 tag, uint16 id) {
	if (!_cache.enabled)
		return;

	for (uint32 i = 0; i < _mhk.size(); i++) {
		// Check for MJMP in Myst ME
		if (isGameVariant(GF_ME) && tag == ID_MSND && _mhk[i]->hasResource(ID_MJMP, id)) {
			Common::SeekableReadStream *tempData = _mhk[i]->getResource(ID_MJMP, id);
			uint16 msndId = tempData->readUint16LE();
			delete tempData;

			// We've found where the real MSND data is, so go get that
			tempData = _mhk[i]->getResource(tag, msndId);
			_cache.add(tag, id, tempData);
			delete tempData;
			return;
		}

		if (_mhk[i]->hasResource(tag, id)) {
			Common::SeekableReadStream *tempData = _mhk[i]->getResource(tag, id);
			_cache.add(tag, id, tempData);
			delete tempData;
			return;
		}
	}

	debugC(kDebugCache, "cachePreload: Could not find a \'%s\' resource with ID %04x", tag2str(tag), id);
}

static const char *mystFiles[] = {
	"channel",
	"credits",
	"demo",
	"dunny",
	"intro",
	"making",
	"mechan",
	"myst",
	"selen",
	"slides",
	"sneak",
	"stone",
	"menu"
};

// Myst Hardcoded Movie Paths
// Mechanical Stack Movie "sstairs" referenced in executable, but not used?

// NOTE: cl1wg1.mov etc. found in the root directory in versions of Myst
// Original are duplicates of those in qtw/myst directory and thus not necessary.
// However, this *is* a problem for Myst ME Mac. Right now it will use the qtw/myst
// video, but this is most likely going to fail for the standalone Mac version.

// The following movies are not referenced in RLST or hardcoded into the executables.
// It is likely they are unused:
// qtw/mech/lwrgear2.mov + lwrgears.mov:	I have no idea what these are; perhaps replaced by an animated image in-game?
// qtw/myst/gar4wbf1.mov:	gar4wbf2.mov has two butterflies instead of one
// qtw/myst/libelev.mov:	libup.mov is basically the same with sound

Common::String MohawkEngine_Myst::wrapMovieFilename(const Common::String &movieName, uint16 stack) {
	Common::String prefix;

	switch (stack) {
	case kIntroStack:
		prefix = "intro/";
		break;
	case kChannelwoodStack:
		// The Windmill videos like to hide in a different folder
		if (movieName.contains("wmill"))
			prefix = "channel2/";
		else
			prefix = "channel/";
		break;
	case kDniStack:
		prefix = "dunny/";
		break;
	case kMechanicalStack:
		prefix = "mech/";
		break;
	case kMystStack:
		prefix = "myst/";
		break;
	case kSeleniticStack:
		prefix = "selen/";
		break;
	case kStoneshipStack:
		prefix = "stone/";
		break;
	default:
		// Masterpiece Edition Only Movies
		break;
	}

	return Common::String("qtw/") + prefix + movieName + ".mov";
}

Common::String MohawkEngine_Myst::selectLocalizedMovieFilename(const Common::String &movieName) {
	Common::Language language = getLanguage();
	const MystLanguage *languageDesc = getLanguageDesc(language);

	if (!languageDesc) {
		return movieName;
	}

	Common::String localizedMovieName = Common::String::format("%s/%s", languageDesc->archiveSuffix, movieName.c_str());
	if (!SearchMan.hasFile(localizedMovieName)) {
		return movieName;
	}

	return localizedMovieName;
}

VideoEntryPtr MohawkEngine_Myst::playMovie(const Common::String &name, MystStack stack) {
	Common::String filename = wrapMovieFilename(name, stack);
	filename = selectLocalizedMovieFilename(filename);
	VideoEntryPtr video = _video->playMovie(filename, Audio::Mixer::kSFXSoundType);

	if (!video) {
		error("Failed to open the '%s' movie", filename.c_str());
	}

	return video;
}

VideoEntryPtr MohawkEngine_Myst::playMovieFullscreen(const Common::String &name, MystStack stack) {
	_gfx->clearScreen();

	VideoEntryPtr video = playMovie(name, stack);
	video->center();
	return video;
}


VideoEntryPtr MohawkEngine_Myst::findVideo(const Common::String &name, MystStack stack) {
	Common::String filename = wrapMovieFilename(name, stack);
	filename = selectLocalizedMovieFilename(filename);
	return _video->findVideo(filename);
}

void MohawkEngine_Myst::playMovieBlocking(const Common::String &name, MystStack stack, uint16 x, uint16 y) {
	Common::String filename = wrapMovieFilename(name, stack);
	filename = selectLocalizedMovieFilename(filename);
	VideoEntryPtr video = _video->playMovie(filename, Audio::Mixer::kSFXSoundType);
	if (!video) {
		error("Failed to open the '%s' movie", filename.c_str());
	}

	video->moveTo(x, y);

	waitUntilMovieEnds(video);
}

void MohawkEngine_Myst::playFlybyMovie(MystStack stack) {
	static const uint16 kMasterpieceOnly = 0xFFFF;

	// Play Flyby Entry Movie on Masterpiece Edition.
	const char *flyby = nullptr;
	bool looping = true;

	switch (stack) {
		case kSeleniticStack:
			flyby = "selenitic flyby";
			break;
		case kStoneshipStack:
			flyby = "stoneship flyby";
			break;
			// Myst Flyby Movie not used in Original Masterpiece Edition Engine
			// We play it when first arriving on Myst, and if the user has chosen so.
		case kMystStack:
			if (ConfMan.getBool("playmystflyby")) {
				flyby = "myst flyby";
				looping = false;
			}
			break;
		case kMechanicalStack:
			flyby = "mech age flyby";
			break;
		case kChannelwoodStack:
			flyby = "channelwood flyby";
			break;
		default:
			break;
	}

	if (!flyby) {
		return;
	}

	_gfx->clearScreen();

	Common::String filename = wrapMovieFilename(flyby, kMasterpieceOnly);
	VideoEntryPtr video = _video->playMovie(filename, Audio::Mixer::kSFXSoundType);
	if (!video) {
		error("Failed to open the '%s' movie", filename.c_str());
	}

	video->center();
	playSkippableMovie(video, looping);
}

void MohawkEngine_Myst::playSkippableMovie(const VideoEntryPtr &video, bool looping) {
	_waitingOnBlockingOperation = true;

	video->setLooping(true);

	_cursor->setCursor(_mainCursor);

	while ((looping || !video->endOfVideo()) && !shouldQuit()) {
		doFrame();

		// Allow skipping
		if (_escapePressed) {
			_escapePressed = false;
			break;
		}

		if (_mouseClicked) {
			_mouseClicked = false;
			break;
		}
	}

	_cursor->setCursor(0);

	// Ensure it's removed
	_video->removeEntry(video);
	_waitingOnBlockingOperation = false;
}

void MohawkEngine_Myst::waitUntilMovieEnds(const VideoEntryPtr &video) {
	if (!video)
		return;

	_waitingOnBlockingOperation = true;

	// Sanity check
	if (video->isLooping())
		error("Called waitUntilMovieEnds() on a looping video");

	while (!video->endOfVideo() && !shouldQuit()) {
		doFrame();

		// Allow skipping
		if (_escapePressed) {
			_escapePressed = false;
			break;
		}
	}

	// Ensure it's removed
	_video->removeEntry(video);
	_waitingOnBlockingOperation = false;
}

void MohawkEngine_Myst::playSoundBlocking(uint16 id) {
	_waitingOnBlockingOperation = true;
	_sound->playEffect(id);

	while (_sound->isEffectPlaying() && !shouldQuit()) {
		doFrame();
	}
	_waitingOnBlockingOperation = false;
}

Common::Error MohawkEngine_Myst::run() {
	MohawkEngine::run();

	if (!_mixer->isReady()) {
		return Common::kAudioDeviceInitFailed;
	}

	_gfx = new MystGraphics(this);
	_video = new VideoManager(this);
	_sound = new MystSound(this);
	setDebugger(new MystConsole(this));
	_gameState = new MystGameState(this, _saveFileMan);
	_cursor = new MystCursorManager(this);
	_rnd = new Common::RandomSource("myst");

	// Cursor is visible by default
	_cursor->showCursor();

	// Load game from launcher/command line if requested
	if (ConfMan.hasKey("save_slot") && hasGameSaveSupport()) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (!_gameState->load(saveSlot))
			error("Failed to load save game from slot %i", saveSlot);
	} else {
		// Start us on the first stack.
		if (getGameType() == GType_MAKINGOF)
			changeToStack(kMakingOfStack, 1, 0, 0);
		else if (isGameVariant(GF_DEMO))
			changeToStack(kDemoStack, 2000, 0, 0);
		else if (isGameVariant(GF_25TH))
			changeToStack(kMenuStack, 1, 0, 0);
		else
			changeToStack(kIntroStack, 1, 0, 0);
	}

	while (!shouldQuit()) {
		doFrame();
	}

	return Common::kNoError;
}

const MystLanguage *MohawkEngine_Myst::getLanguageDesc(Common::Language language) {
	const MystLanguage *languages = MohawkMetaEngine_Myst::listLanguages();

	while (languages->language != Common::UNK_LANG) {
		if (languages->language == language) {
			return languages;
		}

		languages++;
	}

	return nullptr;
}

Common::Language MohawkEngine_Myst::getLanguage() const {
	Common::Language language = MohawkEngine::getLanguage();

	if (language == Common::UNK_LANG) {
		language = _currentLanguage;
	}

	// The language can be changed at run time in the 25th anniversary edition
	if (language == Common::UNK_LANG) {
		language = Common::parseLanguage(ConfMan.get("language"));
	}

	if (language == Common::UNK_LANG) {
		language = Common::EN_ANY;
	}

	return language;
}

void MohawkEngine_Myst::loadStackArchives(MystStack stackId) {
	closeAllArchives();

	Common::Language language = getLanguage();
	const MystLanguage *languageDesc = getLanguageDesc(language);

	if (languageDesc) {
		loadArchive(mystFiles[stackId], languageDesc->archiveSuffix, false);
	}

	loadArchive(mystFiles[stackId], nullptr, true);

	if (isGameVariant(GF_ME)) {
		if (languageDesc) {
			loadArchive("help", languageDesc->archiveSuffix, false);
		}

		loadArchive("help", nullptr, true);
	}

	if (isGameVariant(GF_25TH)) {
		loadArchive("menu", nullptr, true);
	}
}

void MohawkEngine_Myst::loadArchive(const char *archiveName, const char *language, bool mandatory) {
	Common::String filename;
	if (language) {
		filename = Common::String::format("%s_%s.dat", archiveName, language);
	} else {
		filename = Common::String::format("%s.dat", archiveName);
	}

	Archive *archive = new MohawkArchive();
	if (!archive->openFile(filename)) {
		delete archive;
		if (mandatory) {
			error("Could not open %s", filename.c_str());
		} else {
			return;
		}
	}

	_mhk.push_back(archive);
}

void MohawkEngine_Myst::applyGameSettings() {
	// Allow changing the language when in the main menu when the game has not yet been started.
	// It's not possible to reliably change the language once the game is started as the current
	//  view cannot be reconstructed using the save / stack state.
	if (isGameVariant(GF_25TH) && !isGameStarted()) {
		_currentLanguage = Common::parseLanguage(ConfMan.get("language"));
		_gfx->loadMenuFont();
		changeToStack(_stack->getStackId(), _card->getId(), 0, 0);
	}
}

Common::KeymapArray MohawkEngine_Myst::initKeymaps(const char *target) {
	using namespace Common;

	String guiOptions = ConfMan.get("guioptions", target);
	bool isME = checkGameGUIOption(GAMEOPTION_ME, guiOptions);
	bool is25th = checkGameGUIOption(GAMEOPTION_25TH, guiOptions);
	bool isDemo = checkGameGUIOption(GAMEOPTION_DEMO, guiOptions);

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "myst", "Myst");

	Action *act;

	act = new Action(kStandardActionOpenMainMenu, _("Open main menu"));
	act->setCustomEngineActionEvent(kMystActionOpenMainMenu);
	act->addDefaultInputMapping("JOY_X");
	if (is25th) {
		act->addDefaultInputMapping("ESCAPE");
	} else if (isDemo) {
		// TODO: Check the original keybinding for the demo version menu
	} else {
		act->addDefaultInputMapping("F5");
	}
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionSkip, _("Skip"));
	act->setCustomEngineActionEvent(kMystActionSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionInteract, _("Interact"));
	act->setCustomEngineActionEvent(kMystActionInteract);
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionLoad, _("Load game state"));
	act->setCustomEngineActionEvent(kMystActionLoadGameState);
	act->addDefaultInputMapping("C+o");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionSave, _("Save game state"));
	act->setCustomEngineActionEvent(kMystActionSaveGameState);
	act->addDefaultInputMapping("C+s");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionOpenSettings, _("Show options menu"));
	act->setCustomEngineActionEvent(kMystActionOpenOptionsDialog);
	if (is25th || isDemo) {
		act->addDefaultInputMapping("F5");
	}
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionPause, _("Pause"));
	act->setCustomEngineActionEvent(kMystActionPause);
	act->addDefaultInputMapping("SPACE");
	engineKeyMap->addAction(act);

	act = new Action("DRPP", _("Drop page"));
	act->setCustomEngineActionEvent(kMystActionDropPage);
	act->addDefaultInputMapping("A+d");
	engineKeyMap->addAction(act);

	if (isME) {
		act = new Action("SMAP", _("Show map"));
		act->setCustomEngineActionEvent(kMystActionShowMap);
		act->addDefaultInputMapping("A+F8");
		engineKeyMap->addAction(act);
	}

	return Keymap::arrayOf(engineKeyMap);
}

void MohawkEngine_Myst::doFrame() {
	// Update any background videos
	_video->updateMovies();
	if (isInteractive()) {
		_waitingOnBlockingOperation = true;
		_stack->runPersistentScripts();
		_waitingOnBlockingOperation = false;
	}

	Common::Event event;
	while (_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_mouseMoved = true;
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			doAction((MystEventAction)event.customType);
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			switch ((MystEventAction)event.customType) {
			case kMystActionInteract:
				_mouseClicked = false;
				break;
			case kMystActionSkip:
				_escapePressed = false;
				break;
			default:
				break;
			}
			break;
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			// Attempt to autosave before exiting
			saveAutosaveIfEnabled();
			break;
		default:
			break;
		}
	}

	if (isInteractive()) {
		Common::Point mousePos = _system->getEventManager()->getMousePos();

		// Keep a reference to the card so it is not freed if a script switches to another card
		MystCardPtr card = _card;
		card->updateActiveResource(mousePos);
		card->updateResourcesForInput(mousePos, _mouseClicked, _mouseMoved);

		refreshCursor();

		_mouseMoved = false;
	}

	_system->updateScreen();

	// Cut down on CPU usage
	_system->delayMillis(10);
}

bool MohawkEngine_Myst::canDoAction(MystEventAction action) {
	bool inMenu = (_stack->getStackId() == kMenuStack) && _prevStack;
	bool actionsAllowed = inMenu || isInteractive();

	const MystScriptParserPtr &stack = inMenu ? _prevStack : _stack;

	switch (action) {
	case kMystActionDropPage:
		return actionsAllowed && _gameState->_globals.heldPage != kNoPage;
	case kMystActionShowMap:
		return actionsAllowed && stack->getMap();
	case kMystActionOpenMainMenu:
		assert(isGameVariant(GF_DEMO));
		return actionsAllowed && stack->getStackId() != kDemoStack;
	default:
		// Not implemented yet
		error("canDoAction(): Not implemented");
	}
}

void MohawkEngine_Myst::doAction(MystEventAction action) {
	switch (action) {
	case kMystActionInteract:
		_mouseClicked = true;
		break;
	case kMystActionPause:
		pauseGame();
		break;
	case kMystActionOpenOptionsDialog:
		runOptionsDialog();
		break;
	case kMystActionOpenMainMenu:
		if (_stack->getStackId() == kCreditsStack) {
			// Don't allow going to the menu while the credits play
			break;
		}

		if (isGameVariant(GF_DEMO)) {
			if (_stack->getStackId() != kDemoStack && isInteractive()) {
				changeToStack(kDemoStack, 2002, 0, 0);
			}
			break;
		}

		if (isGameVariant(GF_25TH) && isInteractive()) {
			if (_stack->getStackId() == kMenuStack) {
				// If the menu is active and a game is loaded, go back to the game
				if (_prevStack) {
					resumeFromMainMenu();
				}
			} else {
				// If the game is interactive, open the main menu
				goToMainMenu();
			}
			break;
		}

		if (!isGameVariant(GF_25TH)) {
			openMainMenuDialog();
		}

		break;
	case kMystActionSkip:
		if (!isInteractive()) {
			// Try to skip the currently playing video
			_escapePressed = true;
		}
		break;
	case kMystActionLoadGameState:
		if (canLoadGameStateCurrently()) {
			loadGameDialog();
		}
		break;
	case kMystActionSaveGameState:
		if (canSaveGameStateCurrently()) {
			saveGameDialog();
		}
		break;
	case kMystActionDropPage:
		if (_gameState->_globals.heldPage != kNoPage && isInteractive()) {
			dropPage();
		}
		break;
	case kMystActionShowMap:
		if (_stack->getMap() && isInteractive()) {
			_stack->showMap();
		}
		break;
	case kMystActionNone:
	default:
		break;
	}
}

void MohawkEngine_Myst::scheduleAction(MystEventAction action) {
	_scheduledAction = action;
}

void MohawkEngine_Myst::runOptionsDialog() {
	GUI::ConfigDialog dlg;
	int result = runDialog(dlg);
	if (result > 0) {
		syncSoundSettings();
		applyGameSettings();
	}

	if (result > kMystActionNone && result <= kMystActionLast) {
		if (_prevStack) {
			resumeFromMainMenu();
		}

		doAction(static_cast<MystEventAction>(result));
	}
}

void MohawkEngine_Myst::runCredits() {
	if (isInteractive() && getGameType() != GType_MAKINGOF) {
		_cursor->hideCursor();
		changeToStack(kCreditsStack, 10000, 0, 0);
	} else {
		// Showing the credits in the middle of a script is not possible
		// because it unloads the previous age, removing data needed by the
		// rest of the script. Instead we just quit without showing the credits.
		quitGame();
	}
}

bool MohawkEngine_Myst::wait(uint32 duration, bool skippable) {
	_waitingOnBlockingOperation = true;
	uint32 end = getTotalPlayTime() + duration;

	do {
		doFrame();

		if (_escapePressed && skippable) {
			_escapePressed = false;
			return true; // Return true if skipped
		}
	} while (getTotalPlayTime() < end && !shouldQuit());

	_waitingOnBlockingOperation = false;
	return false;
}

void MohawkEngine_Myst::pauseEngineIntern(bool pause) {
	MohawkEngine::pauseEngineIntern(pause);

	if (pause) {
		_video->pauseVideos();
	} else {
		_video->resumeVideos();

		// We may have missed events while paused
		_mouseClicked = (_eventMan->getButtonState() & 1) != 0;

		if (_scheduledAction != kMystActionNone) {
			doAction(_scheduledAction);
		}
	}

	_scheduledAction = kMystActionNone;
}

void MohawkEngine_Myst::changeToStack(MystStack stackId, uint16 card, uint16 linkSrcSound, uint16 linkDstSound) {
	debug(2, "changeToStack(%d)", stackId);

	// Fill screen with black and empty cursor
	_cursor->setCursor(0);
	_currentCursor = 0;

	_sound->stopEffect();
	_video->stopVideos();

	// In Myst ME, play a fullscreen flyby movie, except when loading saves.
	// Also play a flyby when first linking to Myst.
	if (isGameVariant(GF_ME)
			&& ((_stack && _stack->getStackId() == kMystStack) || (stackId == kMystStack && card == 4134))) {
		playFlybyMovie(stackId);
	}

	_sound->stopBackground();

	_gfx->clearScreen();

	if (linkSrcSound)
		playSoundBlocking(linkSrcSound);

	if (_card) {
		_card->leave();
		_card.reset();
	}

	switch (stackId) {
	case kChannelwoodStack:
		_gameState->_globals.currentAge = kChannelwood;
		_stack = MystScriptParserPtr(new MystStacks::Channelwood(this));
		break;
	case kCreditsStack:
		_stack = MystScriptParserPtr(new MystStacks::Credits(this));
		break;
	case kDemoStack:
		_gameState->_globals.currentAge = kSelenitic;
		_stack = MystScriptParserPtr(new MystStacks::Demo(this));
		break;
	case kDniStack:
		_gameState->_globals.currentAge = kDni;
		_stack = MystScriptParserPtr(new MystStacks::Dni(this));
		break;
	case kIntroStack:
		_stack = MystScriptParserPtr(new MystStacks::Intro(this));
		break;
	case kMakingOfStack:
		_stack = MystScriptParserPtr(new MystStacks::MakingOf(this));
		break;
	case kMechanicalStack:
		_gameState->_globals.currentAge = kMechanical;
		_stack = MystScriptParserPtr(new MystStacks::Mechanical(this));
		break;
	case kMenuStack:
		_stack = MystScriptParserPtr(new MystStacks::Menu(this));
		break;
	case kMystStack:
		_gameState->_globals.currentAge = kMystLibrary;
		_stack = MystScriptParserPtr(new MystStacks::Myst(this));
		break;
	case kDemoPreviewStack:
		_stack = MystScriptParserPtr(new MystStacks::Preview(this));
		break;
	case kSeleniticStack:
		_gameState->_globals.currentAge = kSelenitic;
		_stack = MystScriptParserPtr(new MystStacks::Selenitic(this));
		break;
	case kDemoSlidesStack:
		_gameState->_globals.currentAge = kStoneship;
		_stack = MystScriptParserPtr(new MystStacks::Slides(this));
		break;
	case kStoneshipStack:
		_gameState->_globals.currentAge = kStoneship;
		_stack = MystScriptParserPtr(new MystStacks::Stoneship(this));
		break;
	default:
		error("Unknown Myst stack %d", stackId);
	}

	loadStackArchives(stackId);

	// Clear the resource cache and the image cache
	_cache.clear();
	_gfx->clearCache();

	changeToCard(card, kTransitionCopy);

	if (linkDstSound)
		playSoundBlocking(linkDstSound);
}

void MohawkEngine_Myst::changeToCard(uint16 card, TransitionType transition) {
	debug(2, "changeToCard(%d)", card);

	_stack->disablePersistentScripts();

	_video->stopVideos();

	// Clear the resource cache and image cache
	_cache.clear();
	_gfx->clearCache();

	_mouseClicked = false;
	_mouseMoved = false;
	_escapePressed = false;

	if (_card) {
		_card->leave();
	}

	_card = MystCardPtr(new MystCard(this, card));
	_card->enter();

	// The demo resets the cursor at each card change except when in the library
	if (isGameVariant(GF_DEMO)
			&& _gameState->_globals.currentAge != kMystLibrary) {
		_cursor->setDefaultCursor();
	}

	// Make sure the screen is updated
	if (transition != kNoTransition) {
		if (ConfMan.getBool("transition_mode")) {
			_gfx->runTransition(transition, Common::Rect(544, 333), 10, 0);
		} else {
			_gfx->copyBackBufferToScreen(Common::Rect(544, 333));
		}
	}

	// Debug: Show resource rects
	if (_showResourceRects)
		_card->drawResourceRects();
}

void MohawkEngine_Myst::setMainCursor(uint16 cursor) {
	_currentCursor = _mainCursor = cursor;
	_cursor->setCursor(_currentCursor);
}

void MohawkEngine_Myst::refreshCursor() {
	int16 cursor = _card->getActiveResourceCursor();
	if (cursor == -1) {
		cursor = _mainCursor;
	}

	if (cursor != _currentCursor) {
		_currentCursor = cursor;
		_cursor->setCursor(cursor);
	}
}

void MohawkEngine_Myst::redrawResource(MystAreaImageSwitch *resource, bool update) {
	resource->drawConditionalDataToScreen(_stack->getVar(resource->getImageSwitchVar()), update);
}

MystArea *MohawkEngine_Myst::loadResource(Common::SeekableReadStream *rlstStream, MystArea *parent) {
	MystArea *resource = nullptr;
	ResourceType type = static_cast<ResourceType>(rlstStream->readUint16LE());

	debugC(kDebugResource, "\tType: %d", type);
	debugC(kDebugResource, "\tSub_Record: %d", (parent == nullptr) ? 0 : 1);

	switch (type) {
	case kMystAreaAction:
		resource =  new MystAreaAction(this, type, rlstStream, parent);
		break;
	case kMystAreaVideo:
		resource =  new MystAreaVideo(this, type, rlstStream, parent);
		break;
	case kMystAreaActionSwitch:
		resource =  new MystAreaActionSwitch(this, type, rlstStream, parent);
		break;
	case kMystAreaImageSwitch:
		resource =  new MystAreaImageSwitch(this, type, rlstStream, parent);
		break;
	case kMystAreaSlider:
		resource =  new MystAreaSlider(this, type, rlstStream, parent);
		break;
	case kMystAreaDrag:
		resource =  new MystAreaDrag(this, type, rlstStream, parent);
		break;
	case kMystVideoInfo:
		resource =  new MystVideoInfo(this, type, rlstStream, parent);
		break;
	case kMystAreaHover:
		resource =  new MystAreaHover(this, type, rlstStream, parent);
		break;
	default:
		resource = new MystArea(this, type, rlstStream, parent);
		break;
	}

	return resource;
}

Common::Error MohawkEngine_Myst::loadGameState(int slot) {
	if (_gameState->load(slot))
		return Common::kNoError;

	return Common::kUnknownError;
}

Common::Error MohawkEngine_Myst::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	const Graphics::Surface *thumbnail = nullptr;
	if (_stack->getStackId() == kMenuStack) {
		thumbnail = _gfx->getThumbnailForMainMenu();
	}

	return _gameState->save(slot, desc, thumbnail, isAutosave) ? Common::kNoError : Common::kUnknownError;
}

bool MohawkEngine_Myst::hasGameSaveSupport() const {
	return !isGameVariant(GF_DEMO) && getGameType() != GType_MAKINGOF;
}

bool MohawkEngine_Myst::isInteractive() const {
	return !_stack->isScriptRunning() && !_waitingOnBlockingOperation;
}

bool MohawkEngine_Myst::canLoadGameStateCurrently() {
	bool isInMenu = _stack->getStackId() == kMenuStack;

	if (!isInMenu) {
		if (!isInteractive()) {
			return false;
		}

		if (_card->isDraggingResource()) {
			return false;
		}
	}

	if (!hasGameSaveSupport()) {
		// No loading in the demo/makingof
		return false;
	}

	return true;
}

bool MohawkEngine_Myst::canSaveGameStateCurrently() {
	if (!canLoadGameStateCurrently()) {
		return false;
	}

	// There's a limited number of stacks the game can save in
	switch (_stack->getStackId()) {
	case kChannelwoodStack:
	case kDniStack:
	case kMechanicalStack:
	case kMystStack:
	case kSeleniticStack:
	case kStoneshipStack:
		return true;
	case kMenuStack:
		return _prevStack;
	default:
		return false;
	}
}

void MohawkEngine_Myst::dropPage() {
	HeldPage page = _gameState->_globals.heldPage;
	bool whitePage = page == kWhitePage;
	bool bluePage = page - 1 < 6;
	bool redPage = page - 7 < 6;

	// Play drop page sound
	_sound->playEffect(800);

	// Drop page
	_gameState->_globals.heldPage = kNoPage;

	// Redraw page area
	if (whitePage && _gameState->_globals.currentAge == kMystLibrary) {
		_stack->toggleVar(41);
		_card->redrawArea(41);
	} else if (bluePage) {
		if (page == kBlueFirePlacePage) {
			if (_gameState->_globals.currentAge == kMystLibrary)
				_card->redrawArea(24);
		} else {
			_card->redrawArea(103);
		}
	} else if (redPage) {
		if (page == kRedFirePlacePage) {
			if (_gameState->_globals.currentAge == kMystLibrary)
				_card->redrawArea(25);
		} else if (page == kRedStoneshipPage) {
			if (_gameState->_globals.currentAge == kStoneship)
				_card->redrawArea(35);
		} else {
			_card->redrawArea(102);
		}
	}

	setMainCursor(kDefaultMystCursor);
	refreshCursor();
}

MystSoundBlock MohawkEngine_Myst::readSoundBlock(Common::ReadStream *stream) const {
	MystSoundBlock soundBlock;
	soundBlock.sound = stream->readSint16LE();
	debugCN(kDebugView, "Sound Control: %d = ", soundBlock.sound);

	if (soundBlock.sound > 0) {
		debugC(kDebugView, "Play new Sound, change volume");
		debugC(kDebugView, "\tSound: %d", soundBlock.sound);
		soundBlock.soundVolume = stream->readUint16LE();
		debugC(kDebugView, "\tVolume: %d", soundBlock.soundVolume);
	} else if (soundBlock.sound == kMystSoundActionContinue) {
		debugC(kDebugView, "Continue current sound");
	} else if (soundBlock.sound == kMystSoundActionChangeVolume) {
		debugC(kDebugView, "Continue current sound, change volume");
		soundBlock.soundVolume = stream->readUint16LE();
		debugC(kDebugView, "\tVolume: %d", soundBlock.soundVolume);
	} else if (soundBlock.sound == kMystSoundActionStop) {
		debugC(kDebugView, "Stop sound");
	} else if (soundBlock.sound == kMystSoundActionConditional) {
		debugC(kDebugView, "Conditional sound list");
		soundBlock.soundVar = stream->readUint16LE();
		debugC(kDebugView, "\tVar: %d", soundBlock.soundVar);
		uint16 soundCount = stream->readUint16LE();
		debugC(kDebugView, "\tCount: %d", soundCount);

		for (uint16 i = 0; i < soundCount; i++) {
			MystSoundBlock::SoundItem sound;

			sound.action = stream->readSint16LE();
			debugC(kDebugView, "\t\tCondition %d: Action %d", i, sound.action);
			if (sound.action == kMystSoundActionChangeVolume || sound.action >= 0) {
				sound.volume = stream->readUint16LE();
				debugC(kDebugView, "\t\tCondition %d: Volume %d", i, sound.volume);
			}

			soundBlock.soundList.push_back(sound);
		}
	} else {
		error("Unknown sound control value '%d' in card '%d'", soundBlock.sound, _card->getId());
	}

	return soundBlock;
}

void MohawkEngine_Myst::applySoundBlock(const MystSoundBlock &block) {
	int16 soundAction = 0;
	uint16 soundActionVolume = 0;

	if (block.sound == kMystSoundActionConditional) {
		uint16 soundVarValue = _stack->getVar(block.soundVar);
		if (soundVarValue >= block.soundList.size())
			warning("Conditional sound variable outside range");
		else {
			soundAction = block.soundList[soundVarValue].action;
			soundActionVolume = block.soundList[soundVarValue].volume;
		}
	} else {
		soundAction = block.sound;
		soundActionVolume = block.soundVolume;
	}

	if (soundAction == kMystSoundActionContinue)
		debug(2, "Continuing with current sound");
	else if (soundAction == kMystSoundActionChangeVolume) {
		debug(2, "Continuing with current sound, changing volume");
		_sound->changeBackgroundVolume(soundActionVolume);
	} else if (soundAction == kMystSoundActionStop) {
		debug(2, "Stopping sound");
		_sound->stopBackground();
	} else if (soundAction > 0) {
		debug(2, "Playing new sound %d", soundAction);
		_sound->playBackground(soundAction, soundActionVolume);
	} else {
		error("Unknown sound action %d", soundAction);
	}
}

void MohawkEngine_Myst::goToMainMenu() {
	_waitingOnBlockingOperation = false;

	_prevCard = _card;
	_prevStack = _stack;
	_gfx->saveStateForMainMenu();

	MystStacks::Menu *menu = new MystStacks::Menu(this);
	menu->setInGame(true);
	menu->setCanSave(canSaveGameStateCurrently());

	_stack = MystScriptParserPtr(menu);
	_card.reset();

	// Clear the resource cache and the image cache
	_cache.clear();
	_gfx->clearCache();

	_card = MystCardPtr(new MystCard(this, 1000));
	_card->enter();

	_gfx->copyBackBufferToScreen(Common::Rect(544, 333));
}

bool MohawkEngine_Myst::isGameStarted() const {
	return _prevStack || (_stack->getStackId() != kMenuStack);
}

void MohawkEngine_Myst::resumeFromMainMenu() {
	assert(_prevStack);

	_card->leave();
	_card.reset();

	_stack = _prevStack;
	_prevStack.reset();


	// Clear the resource cache and image cache
	_cache.clear();
	_gfx->clearCache();

	_mouseClicked = false;
	_mouseMoved = false;
	_escapePressed = false;
	_card = _prevCard;

	_prevCard.reset();
}

} // End of namespace Mohawk
