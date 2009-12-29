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

#include "mohawk/graphics.h"
#include "mohawk/myst.h"
#include "mohawk/myst_scripts.h"
#include "mohawk/myst_saveload.h"

namespace Mohawk {

MohawkEngine_Myst::MohawkEngine_Myst(OSystem *syst, const MohawkGameDescription *gamedesc) : MohawkEngine(syst, gamedesc) {
	Common::addDebugChannel(kDebugVariable, "Variable", "Track Variable Accesses");
	Common::addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
	Common::addDebugChannel(kDebugView, "View", "Track Card File (VIEW) Parsing");
	Common::addDebugChannel(kDebugHint, "Hint", "Track Cursor Hints (HINT) Parsing");
	Common::addDebugChannel(kDebugResource, "Resource", "Track Resource (RLST) Parsing");
	Common::addDebugChannel(kDebugINIT, "Init", "Track Card Init Script (INIT) Parsing");
	Common::addDebugChannel(kDebugEXIT, "Exit", "Track Card Exit Script (EXIT) Parsing");
	Common::addDebugChannel(kDebugScript, "Script", "Track Script Execution");
	Common::addDebugChannel(kDebugHelp, "Help", "Track Help File (HELP) Parsing");

	_zipMode = false;
	_transitionsEnabled = false;

	// Engine tweaks
	// Disabling this makes engine behaviour as per
	// original, including bugs, missing bits etc. :)
	_tweaksEnabled = true;

	_currentCursor = _mainCursor = kDefaultMystCursor;
	_showResourceRects = false;
	_curCard = 0;
	_needsUpdate = false;
	_curResource = -1;
	
	_cursorHintCount = 0;
	_cursorHints = NULL;
	
	_view.conditionalImageCount = 0;
	_view.conditionalImages = NULL;
	_view.soundList = NULL;
	_view.soundListVolume = NULL;
	_view.scriptResCount = 0;
	_view.scriptResources = NULL;

	_scriptParser->disableInitOpcodes();

	if ((getFeatures() & GF_ME) && getPlatform() == Common::kPlatformMacintosh)
		SearchMan.addSubDirectoryMatching(_gameDataDir, "CD Data");
}

MohawkEngine_Myst::~MohawkEngine_Myst() {
	delete _gfx;
	delete _console;
	delete _scriptParser;
	delete _varStore;
	delete _saveLoad;
	delete _loadDialog;
	delete _optionsDialog;
	delete[] _view.conditionalImages;
	delete[] _view.scriptResources;
	delete[] _cursorHints;
	_resources.clear();
}

static const char *mystFiles[] = {
	"channel.dat",
	"credits.dat",
	"demo.dat",
	"dunny.dat",
	"intro.dat",
	"making.dat",
	"mechan.dat",
	"myst.dat",
	"selen.dat",
	"slides.dat",
	"sneak.dat",
	"stone.dat"
};

// Myst Hardcoded Movie Paths
// Mechanical Stack Movie "sstairs" referenced in executable, but not used?

// NOTE: cl1wg1.mov etc. found in the root directory in versions of Myst
// Original are duplicates of those in /qtw/myst directory and thus not necessary.

// The following movies are not referenced in RLST or hardcoded into the executables.
// It is likely they are unused:
// qtw/mech/lwrgear2.mov + lwrgears.mov:	I have no idea what these are; perhaps replaced by an animated image in-game?
// qtw/myst/gar4wbf1.mov:	gar4wbf2.mov has two butterflies instead of one
// qtw/myst/libelev.mov:	libup.mov is basically the same with sound

Common::String MohawkEngine_Myst::wrapMovieFilename(Common::String movieName, uint16 stack) {
	const char* prefix;
	
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
			prefix = ""; // Masterpiece Edition Only Movies
			break;
	}

	if ((getFeatures() & GF_ME) && getPlatform() == Common::kPlatformMacintosh)
		return Common::String("CD Data/m/") + movieName + ".mov";

	return Common::String("qtw/") + prefix + movieName + ".mov";
}

Common::Error MohawkEngine_Myst::run() {
	MohawkEngine::run();

	_gfx = new MystGraphics(this);
	_console = new MystConsole(this);
	_varStore = new MystVar(this);
	_saveLoad = new MystSaveLoad(this, _saveFileMan);
	_scriptParser = new MystScriptParser(this);
	_loadDialog = new GUI::SaveLoadChooser("Load Game:", "Load");
	_loadDialog->setSaveMode(false);
	_optionsDialog = new MystOptionsDialog(this);

	// Start us on the first stack.
	if (getGameType() == GType_MAKINGOF)
		changeToStack(kMakingOfStack);
	else if (getFeatures() & GF_DEMO)
		changeToStack(kDemoStack);
	else
		changeToStack(kIntroStack);

	if (getGameType() == GType_MAKINGOF)
		changeToCard(1);
	else {
		if ((getFeatures() & GF_ME) && getPlatform() == Common::kPlatformMacintosh) {
			_video->playMovieCentered(wrapMovieFilename("mattel", kIntroStack));
			_video->playMovieCentered(wrapMovieFilename("presto", kIntroStack));
		} else
			_video->playMovieCentered(wrapMovieFilename("broder", kIntroStack));

		_video->playMovieCentered(wrapMovieFilename("cyanlogo", kIntroStack));

		if (!(getFeatures() & GF_DEMO)) { // The demo doesn't have the intro video
			if ((getFeatures() & GF_ME) && getPlatform() == Common::kPlatformMacintosh)
				// intro.mov uses Sorenson, introc uses Cinepak. Otherwise, they're the same.
				_video->playMovieCentered(wrapMovieFilename("introc", kIntroStack));
			else
				_video->playMovieCentered(wrapMovieFilename("intro", kIntroStack));
		}

		if (shouldQuit())
			return Common::kNoError;

		if (getFeatures() & GF_DEMO)
			changeToCard(2001);
		else {
			// It should be card 1 for the full game eventually too, but it's not working
			// there at the moment. Card 2 is the card with the book on the ground.
			changeToCard(2);
		}
	}

	// Load game from launcher/command line if requested
	if (ConfMan.hasKey("save_slot") && !(getFeatures() & GF_DEMO)) {
		uint32 gameToLoad = ConfMan.getInt("save_slot");
		Common::StringList savedGamesList = _saveLoad->generateSaveGameList();
		if (gameToLoad > savedGamesList.size())
			error ("Could not find saved game");
		_saveLoad->loadGame(savedGamesList[gameToLoad]);
		// HACK: The save_slot variable is saved to the disk! We don't want this!
		ConfMan.removeKey("save_slot", ConfMan.getActiveDomainName());
		ConfMan.flushToDisk();
	}

	// Load Help System (Masterpiece Edition Only)
	if (getFeatures() & GF_ME) {
		MohawkFile *mhk = new MohawkFile();
		mhk->open("help.dat");
		_mhk.push_back(mhk);
	}

	// Test Load Function...
	loadHelp(10000);

	// Set the cursor
	_gfx->changeCursor(_currentCursor);
	_gfx->showCursor();

	Common::Event event;
	while (!shouldQuit()) {
		// Update any background videos
		_needsUpdate = _video->updateBackgroundMovies();
		_scriptParser->runPersistentOpcodes();

		// Run animations...
		for (uint16 i = 0; i < _resources.size(); i++)
			_resources[i]->handleAnimation();

		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					_mousePos = event.mouse;
					_needsUpdate = true;
					checkCurrentResource();
					break;
				case Common::EVENT_LBUTTONUP:
					if (_curResource >= 0) {
						debug(2, "Sending mouse up event to resource %d\n", _curResource);
						_resources[_curResource]->handleMouseUp();
					}

					for (uint16 i = 0; i < _resources.size(); i++)
						if (_resources[i]->isEnabled())
							_resources[i]->drawDataToScreen();
					break;
				case Common::EVENT_LBUTTONDOWN:
					if (_curResource >= 0) {
						debug(2, "Sending mouse up event to resource %d\n", _curResource);
						_resources[_curResource]->handleMouseDown();
					}
					break;	
				case Common::EVENT_KEYDOWN:
					switch (event.kbd.keycode) {
						case Common::KEYCODE_d:
							if (event.kbd.flags & Common::KBD_CTRL) {
								_console->attach();
								_console->onFrame();
							}
							break;
						case Common::KEYCODE_SPACE:
							pauseGame();
							break;
						case Common::KEYCODE_F4:
							_showResourceRects = !_showResourceRects;
							if (_showResourceRects)
								drawResourceRects();
							break;
						case Common::KEYCODE_F5:
							runDialog(*_optionsDialog);
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		}

		if (_needsUpdate) {
			_system->updateScreen();
			_needsUpdate = false;
		}

		// Cut down on CPU usage
		_system->delayMillis(10);
	}

	return Common::kNoError;
}

void MohawkEngine_Myst::changeToStack(uint16 stack) {
	debug(2, "changeToStack(%d)", stack);

	_curStack = stack;

	// If the array is empty, add a new one. Otherwise, delete the first
	// entry which is the stack file (the second, if there, is the help file).
	if (_mhk.empty())
		_mhk.push_back(new MohawkFile());
	else {
		delete _mhk[0];
		_mhk[0] = new MohawkFile();
	}

	_mhk[0]->open(mystFiles[_curStack]);

	if (getPlatform() == Common::kPlatformMacintosh)
		_gfx->loadExternalPictureFile(_curStack);
	
	_runExitScript = false;
}

void MohawkEngine_Myst::changeToCard(uint16 card) {
	debug(2, "changeToCard(%d)", card);

	_scriptParser->disableInitOpcodes();
	
	_video->stopVideos();

	// Run exit script from last card (if present)
	if (_runExitScript)
		runExitScript();

	_runExitScript = true;
	
	unloadCard();

	_curCard = card;

	// Load a bunch of stuff
	loadCard();
	loadResources();
	loadCursorHints();

	// Handle images
	if (_view.conditionalImageCount != 0) {
		for (uint16 i = 0; i < _view.conditionalImageCount; i++) {
			if (_varStore->getVar(_view.conditionalImages[i].var) < _view.conditionalImages[i].numStates)
				_gfx->copyImageToScreen(_view.conditionalImages[i].values[_varStore->getVar(_view.conditionalImages[i].var)], Common::Rect(0, 0, 544, 333));
			else
				warning("Conditional image %d variable %d: %d exceeds maximum state of %d", i, _view.conditionalImages[i].var, _varStore->getVar(_view.conditionalImages[i].var), _view.conditionalImages[i].numStates-1);
		}
	} else if (_view.mainImage != 0)
		_gfx->copyImageToScreen(_view.mainImage, Common::Rect(0, 0, 544, 333));

	// Handle sound
	int16 soundAction = 0;
	uint16 soundActionVolume = 0;

	if (_view.sound == kMystSoundActionConditional) {
		uint16 soundVarValue = _varStore->getVar(_view.soundVar);
		if (soundVarValue >= _view.soundCount)
			warning("Conditional sound variable outside range");
		else {
			soundAction = _view.soundList[soundVarValue];
			soundActionVolume = _view.soundListVolume[soundVarValue];
		}
	} else {
		soundAction = _view.sound;
		soundActionVolume = _view.soundVolume;
	}
	
	// NOTE: Mixer only has 8-bit channel volume granularity, 
	// Myst uses 16-bit? Or is part of this balance?
	soundActionVolume = (byte)(soundActionVolume / 255);
	
	if (soundAction == kMystSoundActionContinue)
		debug(2, "Continuing with current sound");
	else if (soundAction == kMystSoundActionChangeVolume) {
		debug(2, "Continuing with current sound, changing volume");
		// TODO: Implement Volume Control..
	} else if (soundAction == kMystSoundActionStop) {
		debug(2, "Stopping sound");
		_sound->stopSound();
	} else if (soundAction > 0) {
		debug(2, "Playing new sound %d", soundAction);
		_sound->stopSound();
		// TODO: Need to keep sound handle and add function to change volume of
		// looped running sound for kMystSoundActionChangeVolume type
		_sound->playSound(soundAction, true, soundActionVolume);
	} else {
		error("Unknown sound action %d", soundAction);
	}

	// TODO: Handle Script Resources

	// Run the entrance script (if present)
	runInitScript();
	
	// Make sure we have the right cursor showing
	_curResource = -1;
	checkCurrentResource();

	// Debug: Show resource rects
	if (_showResourceRects)
		drawResourceRects();
}

void MohawkEngine_Myst::drawResourceRects() {
	for (uint16 i = 0; i < _resources.size(); i++) {
		_resources[i]->getRect().debugPrint(0);
		if (_resources[i]->getRect().isValidRect())
			_gfx->drawRect(_resources[i]->getRect(), _resources[i]->isEnabled());
	}

	_system->updateScreen();
}

void MohawkEngine_Myst::checkCurrentResource() {
	// See what resource we're over
	bool foundResource = false;

	for (uint16 i = 0; i < _resources.size(); i++)
		if (_resources[i]->isEnabled() && _resources[i]->contains(_system->getEventManager()->getMousePos())) {
			if (_curResource != i) {
				if (_curResource != -1)
					_resources[_curResource]->handleMouseLeave();
				_resources[i]->handleMouseEnter();
			}

			_curResource = i;
			foundResource = true;
			break;
		}

	// Set the resource to none if we're not over any
	if (!foundResource)
		_curResource = -1;

	checkCursorHints();
}

void MohawkEngine_Myst::loadCard() {
	debugC(kDebugView, "Loading Card View:");

	Common::SeekableReadStream *viewStream = getRawData(ID_VIEW, _curCard);

	// Card Flags
	_view.flags = viewStream->readUint16LE();
	debugC(kDebugView, "Flags: 0x%04X", _view.flags);

	// The Image Block (Reminiscent of Riven PLST resources)
	_view.conditionalImageCount = viewStream->readUint16LE();
	debugC(kDebugView, "Conditional Image Count: %d", _view.conditionalImageCount);
	if (_view.conditionalImageCount != 0) {
		_view.conditionalImages = new MystCondition[_view.conditionalImageCount];
		for (uint16 i = 0; i < _view.conditionalImageCount; i++) {
			debugC(kDebugView, "\tImage %d:", i);
			_view.conditionalImages[i].var = viewStream->readUint16LE();
			debugC(kDebugView, "\t\tVar: %d", _view.conditionalImages[i].var);
			_view.conditionalImages[i].numStates = viewStream->readUint16LE();
			debugC(kDebugView, "\t\tNumber of States: %d", _view.conditionalImages[i].numStates);
			_view.conditionalImages[i].values = new uint16[_view.conditionalImages[i].numStates];
			for (uint16 j = 0; j < _view.conditionalImages[i].numStates; j++) {
				_view.conditionalImages[i].values[j] = viewStream->readUint16LE();
				debugC(kDebugView, "\t\tState %d -> Value %d", j, _view.conditionalImages[i].values[j]);
			}
		}
		_view.mainImage = 0;
	} else {
		_view.mainImage = viewStream->readUint16LE();
		debugC(kDebugView, "Main Image: %d", _view.mainImage);
	}

	// The Sound Block (Reminiscent of Riven SLST resources)
	_view.sound = viewStream->readSint16LE();
	debugCN(kDebugView, "Sound Control: %d = ", _view.sound);
	if (_view.sound > 0) {
		debugC(kDebugView, "Play new Sound, change volume");
		debugC(kDebugView, "\tSound: %d", _view.sound);
		_view.soundVolume = viewStream->readUint16LE();
		debugC(kDebugView, "\tVolume: %d", _view.soundVolume);
	} else if (_view.sound == kMystSoundActionContinue)
		debugC(kDebugView, "Continue current sound");
	else if (_view.sound == kMystSoundActionChangeVolume) {
		debugC(kDebugView, "Continue current sound, change volume");
		_view.soundVolume = viewStream->readUint16LE();
		debugC(kDebugView, "\tVolume: %d", _view.soundVolume);
	} else if (_view.sound == kMystSoundActionStop) {
		debugC(kDebugView, "Stop sound");
	} else if (_view.sound == kMystSoundActionConditional) {
		debugC(kDebugView, "Conditional sound list");
		_view.soundVar = viewStream->readUint16LE();
		debugC(kDebugView, "\tVar: %d", _view.soundVar);
		_view.soundCount = viewStream->readUint16LE();
		debugC(kDebugView, "\tCount: %d", _view.soundCount);
		_view.soundList = new int16[_view.soundCount];
		_view.soundListVolume = new uint16[_view.soundCount];

		for (uint16 i = 0; i < _view.soundCount; i++) {
			_view.soundList[i] = viewStream->readSint16LE();
			debugC(kDebugView, "\t\tCondition %d: Action %d", i, _view.soundList[i]);
			if (_view.soundList[i] == kMystSoundActionChangeVolume || _view.soundList[i] >= 0) {
				_view.soundListVolume[i] = viewStream->readUint16LE();
				debugC(kDebugView, "\t\tCondition %d: Volume %d", i, _view.soundListVolume[i]);
			}
		}
	} else {
		debugC(kDebugView, "Unknown");
		warning("Unknown sound control value in card");
	}

	// Resources that scripts can call upon
	_view.scriptResCount = viewStream->readUint16LE();
	debugC(kDebugView, "Script Resource Count: %d", _view.scriptResCount);
	if (_view.scriptResCount != 0) {
		_view.scriptResources = new MystView::ScriptResource[_view.scriptResCount];
		for (uint16 i = 0; i < _view.scriptResCount; i++) {
			debugC(kDebugView, "\tResource %d:", i);
			_view.scriptResources[i].type = viewStream->readUint16LE();
			debugC(kDebugView, "\t\t Type: %d", _view.scriptResources[i].type);

			switch (_view.scriptResources[i].type) {
				case 1:
					debugC(kDebugView, "\t\t\t\t= Image");
					break;
				case 2:
					debugC(kDebugView, "\t\t\t\t= Sound");
					break;
				case 3:
					debugC(kDebugView, "\t\t\t\t= Resource List");
					break;
				default:
					debugC(kDebugView, "\t\t\t\t= Unknown");
					break;
			}

			if (_view.scriptResources[i].type == 3) {
				_view.scriptResources[i].var = viewStream->readUint16LE();
				debugC(kDebugView, "\t\t Var: %d", _view.scriptResources[i].var);
				_view.scriptResources[i].count = viewStream->readUint16LE();
				debugC(kDebugView, "\t\t Resource List Count: %d", _view.scriptResources[i].count);
				_view.scriptResources[i].u0 = viewStream->readUint16LE();
				debugC(kDebugView, "\t\t u0: %d", _view.scriptResources[i].u0);
				_view.scriptResources[i].resource_list = new int16[_view.scriptResources[i].count];

				for (uint16 j = 0; j < _view.scriptResources[i].count; j++) {
					_view.scriptResources[i].resource_list[j] = viewStream->readSint16LE();
					debugC(kDebugView, "\t\t Resource List %d: %d", j, _view.scriptResources[i].resource_list[j]);
				}
			} else {
				_view.scriptResources[i].resource_list = NULL;
				_view.scriptResources[i].id = viewStream->readUint16LE();
				debugC(kDebugView, "\t\t Id: %d", _view.scriptResources[i].id);
			}
		}
	}

	// Identifiers for other resources. 0 if non existent. There is always an RLST.
	_view.rlst = viewStream->readUint16LE();
	if (!_view.rlst)
		error("RLST Index missing");

	_view.hint = viewStream->readUint16LE();
	_view.init = viewStream->readUint16LE();
	_view.exit = viewStream->readUint16LE();

	delete viewStream;
}

void MohawkEngine_Myst::unloadCard() {
	for (uint16 i = 0; i < _view.conditionalImageCount; i++)
		delete[] _view.conditionalImages[i].values;

	delete[] _view.conditionalImages;
	_view.conditionalImageCount = 0;
	_view.conditionalImages = NULL;

	delete[] _view.soundList;
	_view.soundList = NULL;
	delete[] _view.soundListVolume;
	_view.soundListVolume = NULL;

	for (uint16 i = 0; i < _view.scriptResCount; i++)
		delete[] _view.scriptResources[i].resource_list;

	delete[] _view.scriptResources;
	_view.scriptResources = NULL;
	_view.scriptResCount = 0;
}

void MohawkEngine_Myst::runInitScript() {
	if (!_view.init) {
		debugC(kDebugINIT, "No INIT Present");
		return;
	}

	debugC(kDebugINIT, "Running INIT script");

	Common::SeekableReadStream *initStream = getRawData(ID_INIT, _view.init);

	uint16 scriptCount = initStream->readUint16LE();

	debugC(kDebugINIT, "\tOpcode Count: %d", scriptCount);

	MystScriptEntry *scripts = new MystScriptEntry[scriptCount];

	for (uint16 i = 0; i < scriptCount; i++) {
		// TODO: u0 is likely variable reference for boolean to
		// determine whether or not to execute opcode
		uint16 u0 = initStream->readUint16LE();
		scripts[i].opcode = initStream->readUint16LE();
		// If variable indicates not to execute opcode, rewrite to NOP
		//if (!_varStore->getVar(u0))
		//	scripts[i].opcode = 0xFFFF;
		scripts[i].var = initStream->readUint16LE();
		scripts[i].numValues = initStream->readUint16LE();
		scripts[i].values = new uint16[scripts[i].numValues];

		debugC(kDebugINIT, "\tu0: %d", u0);
		debugC(kDebugINIT, "\tOpcode %d: %s", i, _scriptParser->getOpcodeDesc(scripts[i].opcode));
		debugC(kDebugINIT, "\t\tUses Variable %d", scripts[i].var);
		debugC(kDebugINIT, "\t\tHas %d Arguments:", scripts[i].numValues);

		for (uint16 j = 0; j < scripts[i].numValues; j++) {
			scripts[i].values[j] = initStream->readUint16LE();
			debugC(kDebugINIT, "\t\tArgument %d: %d", j, scripts[i].values[j]);
		}
	}

	delete initStream;

	_scriptParser->runScript(scriptCount, scripts);

	for (uint16 i = 0; i < scriptCount; i++)
		delete[] scripts[i].values;
	delete[] scripts;
}

void MohawkEngine_Myst::runExitScript() {
	if (!_view.exit) {
		debugC(kDebugEXIT, "No EXIT Present");
		return;
	}

	debugC(kDebugEXIT, "Running EXIT script");

	Common::SeekableReadStream *exitStream = getRawData(ID_EXIT, _view.exit);

	uint16 scriptCount = exitStream->readUint16LE();

	debugC(kDebugEXIT, "\tOpcode Count: %d", scriptCount);

	MystScriptEntry *scripts = new MystScriptEntry[scriptCount];

	for (uint16 i = 0; i < scriptCount; i++) {
		// TODO: u0 is likely variable reference for boolean to
		// to determine whether or not to execute opcode (i.e. door
		// close noises only when door is open).
		uint16 u0 = exitStream->readUint16LE();
		scripts[i].opcode = exitStream->readUint16LE();
		// If variable indicates not to execute opcode, rewrite to NOP
		//if (!_varStore->getVar(u0))
		//	scripts[i].opcode = 0xFFFF;
		scripts[i].var = exitStream->readUint16LE();
		scripts[i].numValues = exitStream->readUint16LE();
		scripts[i].values = new uint16[scripts[i].numValues];

		debugC(kDebugEXIT, "\tu0: %d", u0);
		debugC(kDebugEXIT, "\tOpcode %d: %s", i, _scriptParser->getOpcodeDesc(scripts[i].opcode));
		debugC(kDebugEXIT, "\t\tUses Variable %d", scripts[i].var);
		debugC(kDebugEXIT, "\t\tHas %d Arguments:", scripts[i].numValues);

		for (uint16 j = 0; j < scripts[i].numValues; j++) {
			scripts[i].values[j] = exitStream->readUint16LE();
			debugC(kDebugEXIT, "\t\tArgument %d: %d", j, scripts[i].values[j]);
		}

		uint16 u1 = exitStream->readUint16LE();
		if (u1 != 1)
			warning("Myst EXIT u1 not 1");
	}

	delete exitStream;

	_scriptParser->runScript(scriptCount, scripts);
	
	for (uint16 i = 0; i < scriptCount; i++)
		delete[] scripts[i].values;
	delete[] scripts;
}

void MohawkEngine_Myst::loadHelp(uint16 id) {
	// The original version did not have the help system
	if (!(getFeatures() & GF_ME))
		return;
	
	// TODO: Help File contains 5 cards i.e. VIEW, RLST, etc.
	//       in addition to HELP resources.
	//       These are Ids 9930 to 9934
	//       Need to deal with loading and displaying these..
	//       Current engine structure only supports display of
	//       card from primary stack MHK

	debugC(kDebugHelp, "Loading Help System Data");

	Common::SeekableReadStream *helpStream = getRawData(ID_HELP, id);

	uint16 count = helpStream->readUint16LE();
	uint16 *u0 = new uint16[count];
	Common::String helpText = Common::String::emptyString;
	
	debugC(kDebugHelp, "\tcount: %d", count);
	
	for (uint16 i = 0; i < count; i++) {
		u0[i] = helpStream->readUint16LE();
		debugC(kDebugHelp, "\tu0[%d]: %d", i, u0[i]);
	}
	
	// TODO: Previous values i.e. u0[0] to u0[count - 2]
	// appear to be resource ids in the help.dat file..
	if (u0[count - 1] != count)
		warning("loadHelp(): last u0 value is not equal to count");

	do {
		helpText += helpStream->readByte();
	} while (helpText.lastChar() != 0);
	helpText.deleteLastChar();

	debugC(kDebugHelp, "\thelpText: \"%s\"", helpText.c_str());

	delete[] u0;
}

void MohawkEngine_Myst::loadCursorHints() {
	for (uint16 i = 0; i < _cursorHintCount; i++)
		delete[] _cursorHints[i].variableHint.values;
	_cursorHintCount = 0;
	delete[] _cursorHints;
	_cursorHints = NULL;

	if (!_view.hint) {
		debugC(kDebugHint, "No HINT Present");
		return;
	}

	debugC(kDebugHint, "Loading Cursor Hints:");

	Common::SeekableReadStream *hintStream = getRawData(ID_HINT, _curCard);
	_cursorHintCount = hintStream->readUint16LE();
	debugC(kDebugHint, "Cursor Hint Count: %d", _cursorHintCount);
	_cursorHints = new MystCursorHint[_cursorHintCount];

	for (uint16 i = 0; i < _cursorHintCount; i++) {
		debugC(kDebugHint, "Cursor Hint %d:", i);
		_cursorHints[i].id = hintStream->readUint16LE();
		debugC(kDebugHint, "\tId: %d", _cursorHints[i].id);
		_cursorHints[i].cursor = hintStream->readSint16LE();
		debugC(kDebugHint, "\tCursor: %d", _cursorHints[i].cursor);

		if (_cursorHints[i].cursor == -1) {
			debugC(kDebugHint, "\tConditional Cursor Hints:");
			_cursorHints[i].variableHint.var = hintStream->readUint16LE();
			debugC(kDebugHint, "\tVar: %d", _cursorHints[i].variableHint.var);
			_cursorHints[i].variableHint.numStates = hintStream->readUint16LE();
			debugC(kDebugHint, "\tNumber of States: %d", _cursorHints[i].variableHint.numStates);
			_cursorHints[i].variableHint.values = new uint16[_cursorHints[i].variableHint.numStates];
			for (uint16 j = 0; j < _cursorHints[i].variableHint.numStates; j++) {
				_cursorHints[i].variableHint.values[j] = hintStream->readUint16LE();
				debugC(kDebugHint, "\t\t State %d: Cursor %d", j, _cursorHints[i].variableHint.values[j]);
			}
		} else {
			_cursorHints[i].variableHint.var = 0;
			_cursorHints[i].variableHint.numStates = 0;
			_cursorHints[i].variableHint.values = NULL;
		}
	}

	delete hintStream;
}

void MohawkEngine_Myst::setMainCursor(uint16 cursor) {
	_currentCursor = _mainCursor = cursor;
	_gfx->changeCursor(_currentCursor);
}

void MohawkEngine_Myst::checkCursorHints() {
	if (!_view.hint)
		return;

	// Check all the cursor hints to see if we're in a hotspot that contains a hint.
	for (uint16 i = 0; i < _cursorHintCount; i++)
		if (_cursorHints[i].id == _curResource && _resources[_cursorHints[i].id]->isEnabled()) {
			if (_cursorHints[i].cursor == -1) {
				uint16 var_value = _varStore->getVar(_cursorHints[i].variableHint.var);

				if (var_value >= _cursorHints[i].variableHint.numStates)
					warning("Variable %d Out of Range in variable HINT Resource %d", _cursorHints[i].variableHint.var, i);
				else {
					_currentCursor = _cursorHints[i].variableHint.values[var_value];
					if (_currentCursor == 0)
						_currentCursor = _mainCursor;
					_gfx->changeCursor(_currentCursor);
				}
			} else if (_currentCursor != _cursorHints[i].cursor) {
				if (_cursorHints[i].cursor == 0)
					_currentCursor = _mainCursor;
				else
					_currentCursor = _cursorHints[i].cursor;

				_gfx->changeCursor(_currentCursor);
			}
			return;
		}

	if (_currentCursor != _mainCursor) {
		_currentCursor = _mainCursor;
		_gfx->changeCursor(_currentCursor);
	}
}

void MohawkEngine_Myst::setResourceEnabled(uint16 resourceId, bool enable) {
	if (resourceId < _resources.size()) {
		_resources[resourceId]->setEnabled(enable);
	} else
		warning("Attempt to change unknown resource enable state");
}

static MystResource *loadResource(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) {
	uint16 type = rlstStream->readUint16LE();

	debugC(kDebugResource, "\tType: %d", type);
	debugC(kDebugResource, "\tSub_Record: %d", (parent == NULL) ? 0 : 1);

	switch (type) {
		case kMystForwardResource:
		case kMystLeftResource:
		case kMystRightResource:
		case kMystDownResource:
		case kMystUpResource:
		case 14: // TODO: kMystBackwardResource?
			return new MystResource(vm, rlstStream, parent);
		case kMystActionResource:
			return new MystResourceType5(vm, rlstStream, parent);
		case kMystVideoResource:
			return new MystResourceType6(vm, rlstStream, parent);
		case kMystSwitchResource:
			return new MystResourceType7(vm, rlstStream, parent);
		case 8:
			return new MystResourceType8(vm, rlstStream, parent);
		case 10:
			return new MystResourceType10(vm, rlstStream, parent);
		case 11:
			return new MystResourceType11(vm, rlstStream, parent);
		case 12:
			return new MystResourceType12(vm, rlstStream, parent);
		case 13:
			return new MystResourceType13(vm, rlstStream, parent);
		default:
			error ("Unknown/Unhandled MystResource type %d", type);
	}
}

void MohawkEngine_Myst::loadResources() {
	_resources.clear();

	if (!_view.rlst) {
		debugC(kDebugResource, "No RLST present");
		return;
	}

	Common::SeekableReadStream *rlstStream = getRawData(ID_RLST, _view.rlst);
	uint16 resourceCount = rlstStream->readUint16LE();
	debugC(kDebugResource, "RLST Resource Count: %d", resourceCount);

	for (uint16 i = 0; i < resourceCount; i++) {
		debugC(kDebugResource, "Resource #%d:", i);
		_resources.push_back(loadResource(this, rlstStream, NULL));
	}
	delete rlstStream;
}

MystResource::MystResource(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) {
	_vm = vm;
	_parent = parent;
	
	if (parent == NULL) {
		_flags = rlstStream->readUint16LE();
		_rect.left = rlstStream->readSint16LE();
		_rect.top = rlstStream->readSint16LE();

		if (_rect.top == -1) {
			warning("Invalid _rect.top of -1 found - clamping to 0");
			_rect.top = 0;
		}

		_rect.right = rlstStream->readSint16LE();
		_rect.bottom = rlstStream->readSint16LE();
		_dest = rlstStream->readUint16LE();
	} else {
		_flags = parent->_flags;
		_rect.left = parent->_rect.left;
		_rect.top = parent->_rect.top;
		_rect.right = parent->_rect.right;
		_rect.bottom = parent->_rect.bottom;
		_dest = parent->_dest;
	}

	debugC(kDebugResource, "\tflags: 0x%04X", _flags);
	debugC(kDebugResource, "\tleft: %d", _rect.left);
	debugC(kDebugResource, "\ttop: %d", _rect.top);
	debugC(kDebugResource, "\tright: %d", _rect.right);
	debugC(kDebugResource, "\tbottom: %d", _rect.bottom);
	debugC(kDebugResource, "\tdest: %d", _dest);
	
	// Default Enable based on flags...
	if (_vm->_zipMode)
		_enabled = (_flags & kMystZipModeEnableFlag) != 0 ||
		           (_flags & kMystHotspotEnableFlag) != 0 ||
		           (_flags & kMystSubimageEnableFlag) != 0;
	else
		_enabled = (_flags & kMystZipModeEnableFlag) == 0 &&
		           ((_flags & kMystHotspotEnableFlag) != 0 ||
		            (_flags & kMystSubimageEnableFlag) != 0);
}

void MystResource::handleMouseUp() {
	if (_dest != 0)
		_vm->changeToCard(_dest);
	else
		warning("Movement type resource with null destination at position (%d, %d), (%d, %d)", _rect.left, _rect.top, _rect.right, _rect.bottom);
}

MystResourceType5::MystResourceType5(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResource(vm, rlstStream, parent) {
	debugC(kDebugResource, "\tResource Type 5 Script:");

	_scriptCount = rlstStream->readUint16LE();

	debugC(kDebugResource, "\tOpcode Count: %d", _scriptCount);

	if (_scriptCount == 0)
		return;

	_scripts = new MystScriptEntry[_scriptCount];
	for (uint16 i = 0; i < _scriptCount; i++) {
		_scripts[i].opcode = rlstStream->readUint16LE();
		_scripts[i].var = rlstStream->readUint16LE();
		_scripts[i].numValues = rlstStream->readUint16LE();
		_scripts[i].values = new uint16[_scripts[i].numValues];

		debugC(kDebugResource, "\tOpcode %d: %s", i, _vm->_scriptParser->getOpcodeDesc(_scripts[i].opcode));
		debugC(kDebugResource, "\t\tUses Variable %d", _scripts[i].var);
		debugC(kDebugResource, "\t\tHas %d Arguments:", _scripts[i].numValues);

		for (uint16 j = 0; j < _scripts[i].numValues; j++) {
			_scripts[i].values[j] = rlstStream->readUint16LE();
			debugC(kDebugResource, "\t\tArgument %d: %d", j, _scripts[i].values[j]);
		}
	}
}

void MystResourceType5::handleMouseUp() {
	_vm->_scriptParser->runScript(_scriptCount, _scripts, this);
}

// In Myst/Making of Myst, the paths are hardcoded ala Windows style without extension. Convert them.
Common::String MystResourceType6::convertMystVideoName(Common::String name) {
	Common::String temp;

	for (uint32 i = 1; i < name.size(); i++) {
		if (name[i] == '\\')
			temp += '/';
		else
			temp += name[i];
	}

	return temp + ".mov";
}

MystResourceType6::MystResourceType6(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResourceType5(vm, rlstStream, parent) {
	char c = 0;
	
	do {
		c = rlstStream->readByte();
		_videoFile += c;
	} while (c);

	rlstStream->skip(_videoFile.size() & 1);

	// Trim method does not remove extra trailing nulls
	while (_videoFile.size() != 0 && _videoFile.lastChar() == 0)
		_videoFile.deleteLastChar();

	_videoFile = convertMystVideoName(_videoFile);

	// Position values require modulus 10000 to keep in sane range.
	_left = rlstStream->readUint16LE() % 10000;
	_top = rlstStream->readUint16LE() % 10000;
	_loop = rlstStream->readUint16LE();
	_u0 = rlstStream->readUint16LE();
	_playBlocking = rlstStream->readUint16LE();
	_playOnCardChange = rlstStream->readUint16LE();
	_u3 = rlstStream->readUint16LE();

	if (_u0 != 1)
		warning("Type 6 _u0 != 1");
	if (_u3 != 0)
		warning("Type 6 _u3 != 0");

	debugC(kDebugResource, "\tvideoFile: \"%s\"", _videoFile.c_str());
	debugC(kDebugResource, "\tleft: %d", _left);
	debugC(kDebugResource, "\ttop: %d", _top);
	debugC(kDebugResource, "\tloop: %d", _loop);
	debugC(kDebugResource, "\tu0: %d", _u0);
	debugC(kDebugResource, "\tplayBlocking: %d", _playBlocking);
	debugC(kDebugResource, "\tplayOnCardChange: %d", _playOnCardChange);
	debugC(kDebugResource, "\tu3: %d", _u3);

	_videoRunning = false;
}

void MystResourceType6::handleAnimation() {
	// TODO: Implement Code to allow _playOnCardChange when set
	//       and trigger by Opcode 9 when clear

	if (!_videoRunning) {
		// NOTE: The left and top coordinates are often incorrect and do not make sense.
		// We use the rect coordinates here instead.

		if (_playBlocking)
			_vm->_video->playMovie(_videoFile, _rect.left, _rect.top);
		else
			_vm->_video->playBackgroundMovie(_videoFile, _rect.left, _rect.top, _loop);

		_videoRunning = true;
	}
}

MystResourceType7::MystResourceType7(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResource(vm, rlstStream, parent) {
	_var7 = rlstStream->readUint16LE();
	_numSubResources = rlstStream->readUint16LE();
	debugC(kDebugResource, "\tvar7: %d", _var7);
	debugC(kDebugResource, "\tnumSubResources: %d", _numSubResources);

	for (uint16 i = 0; i < _numSubResources; i++)
		_subResources.push_back(loadResource(vm, rlstStream, this));
}

// TODO: All these functions to switch subresource are very similar.
//       Find way to share code (function pointer pass?)
void MystResourceType7::drawDataToScreen() {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->drawDataToScreen();
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_varStore->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->drawDataToScreen();
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->drawDataToScreen();
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

void MystResourceType7::handleAnimation() {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->handleAnimation();
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_varStore->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->handleAnimation();
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->handleAnimation();
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

void MystResourceType7::handleMouseUp() {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->handleMouseUp();
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_varStore->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->handleMouseUp();
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->handleMouseUp();
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

void MystResourceType7::handleMouseDown() {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->handleMouseDown();
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_varStore->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->handleMouseDown();
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->handleMouseDown();
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

void MystResourceType7::handleMouseEnter() {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->handleMouseEnter();
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_varStore->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->handleMouseEnter();
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->handleMouseEnter();
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

void MystResourceType7::handleMouseLeave() {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->handleMouseLeave();
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_varStore->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->handleMouseLeave();
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->handleMouseLeave();
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

MystResourceType8::MystResourceType8(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResourceType7(vm, rlstStream, parent) {
	_var8 = rlstStream->readUint16LE();
	_numSubImages = rlstStream->readUint16LE();
	debugC(kDebugResource, "\tvar8: %d", _var8);
	debugC(kDebugResource, "\tnumSubImages: %d", _numSubImages);

	_subImages = new MystResourceType8::SubImage[_numSubImages];
	for (uint16 i = 0; i < _numSubImages; i++) {
		debugC(kDebugResource, "\tSubimage %d:", i);

		_subImages[i].wdib = rlstStream->readUint16LE();
		_subImages[i].rect.left = rlstStream->readSint16LE();

		if (_subImages[i].rect.left != -1) {
			_subImages[i].rect.top = rlstStream->readSint16LE();
			_subImages[i].rect.right = rlstStream->readSint16LE();
			_subImages[i].rect.bottom = rlstStream->readSint16LE();
		} else {
			_subImages[i].rect.top = 0;
			_subImages[i].rect.right = 0;
			_subImages[i].rect.bottom = 0;
		}

		debugC(kDebugResource, "\twdib: %d", _subImages[i].wdib);
		debugC(kDebugResource, "\tleft: %d", _subImages[i].rect.left);
		debugC(kDebugResource, "\ttop: %d", _subImages[i].rect.top);
		debugC(kDebugResource, "\tright: %d", _subImages[i].rect.right);
		debugC(kDebugResource, "\tbottom: %d", _subImages[i].rect.bottom);
	}
}

void MystResourceType8::drawDataToScreen() {
	// Need to call overidden Type 7 function to ensure
	// switch section is processed correctly.
	MystResourceType7::drawDataToScreen();

	bool drawSubImage = false;
	int16 subImageId = 0;

	if (_var8 == 0xFFFF) {
		if (_numSubImages == 1) {
			subImageId = 0;
			drawSubImage = true;
		} else if (_numSubImages != 0)
			warning("Type 8 Resource with _numSubImages of %d, but no control variable", _numSubImages);
	} else {
		uint16 varValue = _vm->_varStore->getVar(_var8);

		if (_numSubImages == 1 && varValue != 0) {
			subImageId = 0;
			drawSubImage = true;
		} else if (_numSubImages != 0) {
			if (varValue < _numSubImages) {
				subImageId = varValue;
				drawSubImage = true;
			} else
				warning("Type 8 Image Var %d: %d exceeds number of subImages %d", _var8, varValue, _numSubImages);
		}
	}

	if (drawSubImage) {
		uint16 imageToDraw = 0;

		if (_subImages[subImageId].wdib == 0xFFFF) {
			// TODO: Think the reason for problematic screen updates in some rects is that they
			//       are these -1 cases.
			// They need to be redrawn i.e. if the Myst marker switches are changed, but I don't think
			// the rects are valid. This does not matter in the original engine as the screen update redraws
			// the VIEW images, followed by the RLST resource images, and -1 for the WDIB is interpreted as
			// "Do Not Draw Image" i.e so the VIEW image is shown through.. We need to fix screen update
			// to do this same behaviour.
			if (_vm->_view.conditionalImageCount == 0)
				imageToDraw = _vm->_view.mainImage;
			else {
				for (uint16 i = 0; i < _vm->_view.conditionalImageCount; i++)
					if (_vm->_varStore->getVar(_vm->_view.conditionalImages[i].var) < _vm->_view.conditionalImages[i].numStates)
						imageToDraw = _vm->_view.conditionalImages[i].values[_vm->_varStore->getVar(_vm->_view.conditionalImages[i].var)];
			}
		} else
			imageToDraw = _subImages[subImageId].wdib;

		if (_subImages[subImageId].rect.left == -1)
			_vm->_gfx->copyImageSectionToScreen(imageToDraw, _rect, _rect);
			//vm->_gfx->copyImageToScreen(imageToDraw, Common::Rect(0, 0, 544, 333));
		// TODO: Think this is the case when the image is full screen.. need to modify graphics to add functions for returning size of image.			
		// This is not right either...
		//else if (_rect.width() != _subImages[draw_subimage_id].rect.width() || _rect.height() != _subImages[draw_subimage_id].rect.height())
		// HACK: Hardcode cases of this until general rule can be ascertained
		//       These cases seem to have the source rect in the wdib with an vertical i.e. top+X, bottom+X where X is a constant, but could
		//       be negative, translations, when in fact both the source and dest should be equal...
		//else if ((vm->getCurStack() == kSeleniticStack   && vm->getCurCard() == 1155 && tmp == 1) || // X=
		//        (vm->getCurStack() == kSeleniticStack   && vm->getCurCard() == 1225 && tmp == 1) || // X=
		//        (vm->getCurStack() == kMystStack        && vm->getCurCard() == 4247 && tmp == 0) || // X=
		//        (vm->getCurStack() == kChannelwoodStack && vm->getCurCard() == 3161 && tmp == 0))   // X=
		//	vm->_gfx->copyImageSectionToScreen(imageToDraw, _rect, _rect);
		//    // TODO: Small vertical movement remains on change. Suspect off by one error from these to real
		//	//        solution.
		else
			_vm->_gfx->copyImageSectionToScreen(imageToDraw, _subImages[subImageId].rect, _rect);
	}
}

uint16 MystResourceType8::getType8Var() {
	return _var8;
}

// No MystResourceType9!

MystResourceType10::MystResourceType10(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResourceType8(vm, rlstStream, parent) {
	_kind = rlstStream->readUint16LE();
	// NOTE: l,r,t,b differs from standard l,t,r,b order
	_rect10.left = rlstStream->readUint16LE();
	_rect10.right = rlstStream->readUint16LE();
	_rect10.top = rlstStream->readUint16LE();
	_rect10.bottom = rlstStream->readUint16LE();
	_u0 = rlstStream->readUint16LE();
	_u1 = rlstStream->readUint16LE();
	_mouseDownOpcode = rlstStream->readUint16LE();
	_mouseDragOpcode = rlstStream->readUint16LE();
	_mouseUpOpcode = rlstStream->readUint16LE();

	// TODO: Need to work out meaning of kind...
	debugC(kDebugResource, "\tkind: %d", _kind);
	debugC(kDebugResource, "\trect10.left: %d", _rect10.left);
	debugC(kDebugResource, "\trect10.right: %d", _rect10.right);
	debugC(kDebugResource, "\trect10.top: %d", _rect10.top);
	debugC(kDebugResource, "\trect10.bottom: %d", _rect10.bottom);
	debugC(kDebugResource, "\tu0: %d", _u0);
	debugC(kDebugResource, "\tu1: %d", _u1);
	debugC(kDebugResource, "\t_mouseDownOpcode: %d", _mouseDownOpcode);
	debugC(kDebugResource, "\t_mouseDragOpcode: %d", _mouseDragOpcode);
	debugC(kDebugResource, "\t_mouseUpOpcode: %d", _mouseUpOpcode);

	// TODO: Think that u0 and u1 are unused in Type 10
	if (_u0)
		warning("Type 10 u0 non-zero");
	if (_u1)
		warning("Type 10 u1 non-zero");

	// TODO: Not sure about order of Mouse Down, Mouse Drag and Mouse Up
	//       Or whether this is slightly different...
	printf("Type 10 _mouseDownOpcode: %d\n", _mouseDownOpcode);
	printf("Type 10 _mouseDragOpcode: %d\n", _mouseDragOpcode);
	printf("Type 10 _mouseUpOpcode: %d\n", _mouseUpOpcode);

	for (byte i = 0; i < 4; i++) {
		debugC(kDebugResource, "\tList %d:", i);

		_lists[i].listCount = rlstStream->readUint16LE();
		debugC(kDebugResource, "\t%d values", _lists[i].listCount);

		_lists[i].list = new uint16[_lists[i].listCount];
		for (uint16 j = 0; j < _lists[i].listCount; j++) {
			_lists[i].list[j] = rlstStream->readUint16LE();
			debugC(kDebugResource, "\tValue %d: %d", j, _lists[i].list[j]);
		}
	}

	warning("TODO: Card contains Type 10 Resource - Function not yet implemented");
}

void MystResourceType10::handleMouseUp() {
	// TODO
}

MystResourceType11::MystResourceType11(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResourceType8(vm, rlstStream, parent) {
	_kind = rlstStream->readUint16LE();
	// NOTE: l,r,t,b differs from standard l,t,r,b order
	_rect11.left = rlstStream->readUint16LE();
	_rect11.right = rlstStream->readUint16LE();
	_rect11.top = rlstStream->readUint16LE();
	_rect11.bottom = rlstStream->readUint16LE();
	_u0 = rlstStream->readUint16LE();
	_u1 = rlstStream->readUint16LE();
	_mouseDownOpcode = rlstStream->readUint16LE();
	_mouseDragOpcode = rlstStream->readUint16LE();
	_mouseUpOpcode = rlstStream->readUint16LE();

	debugC(kDebugResource, "\tkind: %d", _kind);
	debugC(kDebugResource, "\trect11.left: %d", _rect11.left);
	debugC(kDebugResource, "\trect11.right: %d", _rect11.right);
	debugC(kDebugResource, "\trect11.top: %d", _rect11.top);
	debugC(kDebugResource, "\trect11.bottom: %d", _rect11.bottom);
	debugC(kDebugResource, "\tu0: %d", _u0);
	debugC(kDebugResource, "\tu1: %d", _u1);
	debugC(kDebugResource, "\t_mouseDownOpcode: %d", _mouseDownOpcode);
	debugC(kDebugResource, "\t_mouseDragOpcode: %d", _mouseDragOpcode);
	debugC(kDebugResource, "\t_mouseUpOpcode: %d", _mouseUpOpcode);

	// TODO: Think that u0 and u1 are unused in Type 11
	if (_u0)
		warning("Type 11 u0 non-zero");
	if (_u1)
		warning("Type 11 u1 non-zero");

	// TODO: Not sure about order of Mouse Down, Mouse Drag and Mouse Up
	//       Or whether this is slightly different...
	printf("Type 11 _mouseDownOpcode: %d\n", _mouseDownOpcode);
	printf("Type 11 _mouseDragOpcode: %d\n", _mouseDragOpcode);
	printf("Type 11 _mouseUpOpcode: %d\n", _mouseUpOpcode);

	for (byte i = 0; i < 3; i++) {
		debugC(kDebugResource, "\tList %d:", i);

		_lists[i].listCount = rlstStream->readUint16LE();
		debugC(kDebugResource, "\t%d values", _lists[i].listCount);

		_lists[i].list = new uint16[_lists[i].listCount];
		for (uint16 j = 0; j < _lists[i].listCount; j++) {
			_lists[i].list[j] = rlstStream->readUint16LE();
			debugC(kDebugResource, "\tValue %d: %d", j, _lists[i].list[j]);
		}
	}

	warning("TODO: Card contains Type 11 Resource - Function not yet implemented");
}

void MystResourceType11::handleMouseUp() {
	// TODO

	// HACK: Myst Card 4059 (Fireplace Code Book) to usuable state
	if (_mouseDownOpcode == 191) {
		uint16 tmp = _vm->_varStore->getVar(0);
		if (tmp > 0)
			_vm->_varStore->setVar(0, tmp - 1);
	} else if (_mouseDownOpcode == 190) {
		_vm->_varStore->setVar(0, _vm->_varStore->getVar(0) + 1);
	}
}

MystResourceType12::MystResourceType12(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResourceType8(vm, rlstStream, parent) {
	_kind = rlstStream->readUint16LE();
	// NOTE: l,r,t,b differs from standard l,t,r,b order
	_rect11.left = rlstStream->readUint16LE();
	_rect11.right = rlstStream->readUint16LE();
	_rect11.top = rlstStream->readUint16LE();
	_rect11.bottom = rlstStream->readUint16LE();
	_state0Frame = rlstStream->readUint16LE();
	_state1Frame = rlstStream->readUint16LE();
	_mouseDownOpcode = rlstStream->readUint16LE();
	_mouseDragOpcode = rlstStream->readUint16LE();
	_mouseUpOpcode = rlstStream->readUint16LE();

	debugC(kDebugResource, "\tkind: %d", _kind);
	debugC(kDebugResource, "\trect11.left: %d", _rect11.left);
	debugC(kDebugResource, "\trect11.right: %d", _rect11.right);
	debugC(kDebugResource, "\trect11.top: %d", _rect11.top);
	debugC(kDebugResource, "\trect11.bottom: %d", _rect11.bottom);
	debugC(kDebugResource, "\t_state0Frame: %d", _state0Frame);
	debugC(kDebugResource, "\t_state1Frame: %d", _state1Frame);
	debugC(kDebugResource, "\t_mouseDownOpcode: %d", _mouseDownOpcode);
	debugC(kDebugResource, "\t_mouseDragOpcode: %d", _mouseDragOpcode);
	debugC(kDebugResource, "\t_mouseUpOpcode: %d", _mouseUpOpcode);

	// TODO: Think that u0 and u1 are animation frames to be
	//       drawn for var == 0 and var == 1
	printf("Type 12 _state0Frame: %d\n", _state0Frame);
	printf("Type 12 _state1Frame: %d\n", _state1Frame);

	// TODO: Not sure about order of Mouse Down, Mouse Drag and Mouse Up
	//       Or whether this is slightly different...
	printf("Type 12 _mouseDownOpcode: %d\n", _mouseDownOpcode);
	printf("Type 12 _mouseDragOpcode: %d\n", _mouseDragOpcode);
	printf("Type 12 _mouseUpOpcode: %d\n", _mouseUpOpcode);

	for (byte i = 0; i < 3; i++) {
		debugC(kDebugResource, "\tList %d:", i);

		_lists[i].listCount = rlstStream->readUint16LE();
		debugC(kDebugResource, "\t%d values", _lists[i].listCount);

		_lists[i].list = new uint16[_lists[i].listCount];
		for (uint16 j = 0; j < _lists[i].listCount; j++) {
			_lists[i].list[j] = rlstStream->readUint16LE();
			debugC(kDebugResource, "\tValue %d: %d", j, _lists[i].list[j]);
		}
	}

	warning("TODO: Card contains Type 12, Type 11 section Resource - Function not yet implemented");

	_numFrames = rlstStream->readUint16LE();
	_firstFrame = rlstStream->readUint16LE();
	uint16 frameWidth = rlstStream->readUint16LE();
	uint16 frameHeight = rlstStream->readUint16LE();
	_frameRect.left = rlstStream->readUint16LE();
	_frameRect.top = rlstStream->readUint16LE();

	_frameRect.right = _frameRect.left + frameWidth;
	_frameRect.bottom = _frameRect.top + frameHeight;

	debugC(kDebugResource, "\t_numFrames: %d", _numFrames);
	debugC(kDebugResource, "\t_firstFrame: %d", _firstFrame);
	debugC(kDebugResource, "\tframeWidth: %d", frameWidth);
	debugC(kDebugResource, "\tframeHeight: %d", frameHeight);
	debugC(kDebugResource, "\t_frameRect.left: %d", _frameRect.left);
	debugC(kDebugResource, "\t_frameRect.top: %d", _frameRect.top);
	debugC(kDebugResource, "\t_frameRect.right: %d", _frameRect.right);
	debugC(kDebugResource, "\t_frameRect.bottom: %d", _frameRect.bottom);

	_doAnimation = false;
}

void MystResourceType12::handleAnimation() {
	// TODO: Probably not final version. Variable/Type 11 Controlled?
	if (_doAnimation) {
		_vm->_gfx->copyImageToScreen(_currentFrame++, _frameRect);
		if ((_currentFrame - _firstFrame) >= _numFrames)
			_doAnimation = false;
	}
}

void MystResourceType12::handleMouseUp() {
	// HACK/TODO: Trigger Animation on Mouse Click. Probably not final version. Variable/Type 11 Controlled?
	_currentFrame = _firstFrame;
	_doAnimation = true;
}

MystResourceType13::MystResourceType13(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResource(vm, rlstStream, parent) {
	_enterOpcode = rlstStream->readUint16LE();
	_leaveOpcode = rlstStream->readUint16LE();

	debugC(kDebugResource, "\t_enterOpcode: %d", _enterOpcode);
	debugC(kDebugResource, "\t_leaveOpcode: %d", _leaveOpcode);
}

void MystResourceType13::handleMouseEnter() {
	// Pass along the enter opcode (with no parameters) to the script parser
	_vm->_scriptParser->runOpcode(_enterOpcode);
}

void MystResourceType13::handleMouseLeave() {
	// Pass along the leave opcode (with no parameters) to the script parser
	_vm->_scriptParser->runOpcode(_leaveOpcode);
}

void MystResourceType13::handleMouseUp() {
	// Type 13 Resources do nothing on Mouse Clicks.
	// This is required to override the inherited default 
	// i.e. MystResource::handleMouseUp
}

void MohawkEngine_Myst::runLoadDialog() {
	runDialog(*_loadDialog);
}

Common::Error MohawkEngine_Myst::loadGameState(int slot) {
	if (_saveLoad->loadGame(_saveLoad->generateSaveGameList()[slot])) {
		changeToStack(kIntroStack);
		changeToCard(5);
		return Common::kNoError;
	} else
		return Common::kUnknownError;
}

Common::Error MohawkEngine_Myst::saveGameState(int slot, const char *desc) {
	Common::StringList saveList = _saveLoad->generateSaveGameList();

	if ((uint)slot < saveList.size()) 
		_saveLoad->deleteSave(saveList[slot]);
	
	return _saveLoad->saveGame(Common::String(desc)) ? Common::kNoError : Common::kUnknownError;
}

} // End of namespace Mohawk
