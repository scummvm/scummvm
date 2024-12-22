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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "sci/sci.h"
#include "sci/debug.h"
#include "sci/console.h"
#include "sci/event.h"

#include "sci/engine/features.h"
#include "sci/engine/guest_additions.h"
#include "sci/engine/message.h"
#include "sci/engine/object.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"	// for script_adjust_opcode_formats
#include "sci/engine/script_patches.h"
#include "sci/engine/selector.h"	// for SELECTOR
#include "sci/engine/scriptdebug.h"
#include "sci/engine/tts.h"

#include "sci/sound/audio.h"
#include "sci/sound/music.h"
#include "sci/sound/sync.h"
#include "sci/sound/soundcmd.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/controls16.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/drivers/gfxdriver.h"
#include "sci/graphics/macfont.h"
#include "sci/graphics/maciconbar.h"
#include "sci/graphics/menu.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/remap.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/transitions.h"

#ifdef ENABLE_SCI32
#include "sci/graphics/controls32.h"
#include "sci/graphics/cursor32.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/maccursor32.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap32.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/transitions32.h"
#include "sci/graphics/video32.h"
#include "sci/sound/audio32.h"
#endif

namespace Sci {

SciEngine *g_sci = nullptr;

SciEngine::SciEngine(OSystem *syst, const ADGameDescription *desc, SciGameId gameId) :
	Engine(syst),
	_gfxAnimate(nullptr),
	_gfxCache(nullptr),
	_gfxCompare(nullptr),
	_gfxControls16(nullptr),
	_gfxCoordAdjuster(nullptr),
	_gfxCursor(nullptr),
	_gfxMenu(nullptr),
	_gfxPalette16(nullptr),
	_gfxRemap16(nullptr),
	_gfxPaint16(nullptr),
	_gfxPorts(nullptr),
	_gfxScreen(nullptr),
	_gfxText16(nullptr),
	_gfxTransitions(nullptr),
	_gfxMacFontManager(nullptr),
	_gfxMacIconBar(nullptr),
#ifdef ENABLE_SCI32
	_gfxControls32(nullptr),
	_gfxPalette32(nullptr),
	_gfxRemap32(nullptr),
	_gfxPaint32(nullptr),
	_gfxText32(nullptr),
	_audio32(nullptr),
	_video32(nullptr),
	_gfxFrameout(nullptr),
	_gfxTransitions32(nullptr),
	_gfxCursor32(nullptr),
#endif
	_audio(nullptr),
	_sync(nullptr),
	_soundCmd(nullptr),
	_features(nullptr),
	_guestAdditions(nullptr),
	_opcode_formats(nullptr),
	_debugState(),
	_speedThrottleDelay(kSpeedThrottleDefaultDelay),
	_gameDescription(desc),
	_gameId(gameId),
	_resMan(nullptr),
	_scriptPatcher(nullptr),
	_gamestate(nullptr),
	_kernel(nullptr),
	_vocabulary(nullptr),
	_vocabularyLanguage(1), // we load english vocabulary on startup
	_eventMan(nullptr),
	_gameObjectAddress(),
	_console(nullptr),
	_tts(nullptr),
	_rng("sci"),
	_useHiresGraphics(false),
	_inErrorString(false) {

	assert(g_sci == nullptr);
	g_sci = this;

	const Common::FSNode gameDataDir(ConfMan.getPath("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "actors");	// KQ6 hi-res portraits
	SearchMan.addSubDirectoryMatching(gameDataDir, "aud");	// resource.aud and audio files
	SearchMan.addSubDirectoryMatching(gameDataDir, "audio");// resource.aud and audio files
	SearchMan.addSubDirectoryMatching(gameDataDir, "audiosfx");// resource.aud and audio files
	SearchMan.addSubDirectoryMatching(gameDataDir, "wav");	// speech files in WAV format
	SearchMan.addSubDirectoryMatching(gameDataDir, "sfx");	// music/sound files in WAV format
	SearchMan.addSubDirectoryMatching(gameDataDir, "avi");	// AVI movie files for Windows versions
	SearchMan.addSubDirectoryMatching(gameDataDir, "seq");	// SEQ movie files for DOS versions
	SearchMan.addSubDirectoryMatching(gameDataDir, "robot");	// robot movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "robots");	// robot movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "movie");	// VMD movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "movies");	// VMD movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "music");	// LSL7 music files (GOG version)
	SearchMan.addSubDirectoryMatching(gameDataDir, "music/22s16");	// LSL7 music files
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");	// LSL7 Polish patch files (Trac #10066)
	SearchMan.addSubDirectoryMatching(gameDataDir, "vmd");	// VMD movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "duk");	// Duck movie files in Phantasmagoria 2
	SearchMan.addSubDirectoryMatching(gameDataDir, "Robot Folder"); // Mac robot files
	SearchMan.addSubDirectoryMatching(gameDataDir, "Sound Folder"); // Mac audio files
	SearchMan.addSubDirectoryMatching(gameDataDir, "Voices Folder", 0, 2, true); // Mac audio36 files (recursive for Torin)
	SearchMan.addSubDirectoryMatching(gameDataDir, "Voices"); // Mac audio36 files
	SearchMan.addSubDirectoryMatching(gameDataDir, "Voices/AUD#"); // LSL6 Mac audio36 files
	SearchMan.addSubDirectoryMatching(gameDataDir, "VMD Folder"); // Mac VMD files

	// Add the patches directory, except for KQ6CD; The patches folder in some versions of KQ6CD
	// (e.g. KQ Collection 1997) is for the demo of Phantasmagoria, included in the disk
	if (_gameId != GID_KQ6) {
		// Patch files in the root directory of Phantasmagoria 2 are higher
		// priority than patch files in the patches directory (the SSCI
		// installer copies these patches to HDD and gives the HDD directory
		// top priority)
		const int priority = _gameId == GID_PHANTASMAGORIA2 ? -1 : 0;
		SearchMan.addSubDirectoryMatching(gameDataDir, "patches", priority);	// resource patches
	}

	// Some releases (e.g. Pointsoft Torin) use a different patch directory name
	SearchMan.addSubDirectoryMatching(gameDataDir, "patch");	// resource patches

	// LSL5 uses the name "bonus" for the patch directory on disk 1.
	// The installer would copy these patches to the root game directory.
	if (_gameId == GID_LSL5) {
		SearchMan.addSubDirectoryMatching(gameDataDir, "bonus"); // resource patches
	}

	switch (desc->language) {
	case Common::DE_DEU:
		SearchMan.addSubDirectoryMatching(gameDataDir, "german/msg");
		break;
	case Common::EN_ANY:
	case Common::EN_GRB:
	case Common::EN_USA:
		SearchMan.addSubDirectoryMatching(gameDataDir, "english/msg");
		break;
	case Common::ES_ESP:
		SearchMan.addSubDirectoryMatching(gameDataDir, "spanish/msg");
		break;
	case Common::FR_FRA:
		SearchMan.addSubDirectoryMatching(gameDataDir, "french/msg");
		break;
	case Common::IT_ITA:
		SearchMan.addSubDirectoryMatching(gameDataDir, "italian/msg");
		break;
	default:
		break;
	}
}

SciEngine::~SciEngine() {
#ifdef ENABLE_SCI32
	delete _gfxControls32;
	delete _gfxPaint32;
	delete _gfxText32;
	// GfxFrameout and GfxPalette32 must be deleted after Video32 since
	// destruction of screen items in the Video32 destructor relies on these
	// components
	delete _video32;
	delete _gfxCursor32;
	delete _gfxPalette32;
	delete _gfxTransitions32;
	delete _gfxFrameout;
	delete _gfxRemap32;
	delete _audio32;
#endif
	delete _gfxMenu;
	delete _gfxControls16;
	delete _gfxText16;
	delete _gfxAnimate;
	delete _gfxPaint16;
	delete _gfxTransitions;
	delete _gfxCompare;
	delete _gfxCoordAdjuster;
	delete _gfxPorts;
	delete _gfxCache;
	delete _gfxPalette16;
	delete _gfxRemap16;
	delete _gfxCursor;
	delete _gfxScreen;

	delete _audio;
	delete _sync;
	delete _soundCmd;
	delete _kernel;
	delete _vocabulary;
	//_console deleted by Engine
	delete _guestAdditions;
	delete _features;
	delete _gfxMacFontManager;
	delete _gfxMacIconBar;

	delete _eventMan;
	delete _gamestate->_segMan;
	delete _gamestate;

	delete[] _opcode_formats;

	delete _scriptPatcher;
	delete _tts;
	delete _resMan;	// should be deleted last
	g_sci = nullptr;
}

extern int showScummVMDialog(const Common::U32String &message, const Common::U32String &altButton = Common::U32String(), bool alignCenter = true);

Common::Error SciEngine::run() {
	_tts = new SciTTS();

	_resMan = new ResourceManager();
	_resMan->addAppropriateSources();
	_resMan->init();

	// TODO: Add error handling. Check return values of addAppropriateSources
	// and init. We first have to *add* sensible return values, though ;).
/*
	if (!_resMan) {
		warning("No resources found, aborting");
		return Common::kNoGameDataFoundError;
	}
*/

	// Add the after market patches for the specified game, if they exist
	_resMan->addNewGMPatch(_gameId);
	_resMan->addNewD110Patch(_gameId);
	_gameObjectAddress = _resMan->findGameObject(true);

	_scriptPatcher = new ScriptPatcher();
	SegManager *segMan = new SegManager(_resMan, _scriptPatcher);

	// Load the Mac executable and fonts if available
	if (getSciVersion() < SCI_VERSION_2 && getPlatform() == Common::kPlatformMacintosh) {
		loadMacExecutable();
		loadMacFonts();
	}

	// Read user option for forcing hires graphics
	// Only show/selectable for:
	//  - King's Quest 6 CD
	//  - King's Quest 6 CD demo
	//  - Gabriel Knight 1 CD
	//  - Police Quest 4 CD
	//  - SCI1/1.1 Mac games with hires fonts
	//
	// Gabriel Knight 1 on Mac is hi-res only, so it should NOT get this option.
	// Confirmed by [md5] and originally by clone2727.
	if (Common::checkGameGUIOption(GAMEOPTION_HIGH_RESOLUTION_GRAPHICS, ConfMan.get("guioptions"))) {
		// GAMEOPTION_HIGH_RESOLUTION_GRAPHICS is available for the currently detected game,
		// so read the user option now.
		// We need to do this, because the option's default is "true", but we don't want "true"
		// for any game that does not have this option.
		_useHiresGraphics = ConfMan.getBool("enable_high_resolution_graphics");
	} else if (hasMacFonts()) {
		// Default to using hires Mac fonts if GUI option isn't present, as it was added later.
		_useHiresGraphics = true;
	}

	if (getSciVersion() < SCI_VERSION_2) {
		bool undither = ConfMan.getBool("disable_dithering");
		Common::RenderMode renderMode = SciGfxDriver::getRenderMode();

		// Disable undithering for CGA, Hercules and other unsuitable video modes. The render mode should have been set to
		// kRenderDefault by determineRenderMode() if undithering is selected, but we want to make sure that this matches.
		if (renderMode != Common::kRenderDefault)
			undither = false;

		// Initialize the game screen
		_gfxScreen = new GfxScreen(_resMan, renderMode);
		_gfxScreen->enableUndithering(undither);
	}

	_kernel = new Kernel(_resMan, segMan);
	_features = new GameFeatures(segMan, _kernel);
	_vocabulary = hasParser() ? new Vocabulary(_resMan, false) : nullptr;

	_gamestate = new EngineState(segMan);
	_guestAdditions = new GuestAdditions(_gamestate, _features, _kernel);
	_eventMan = new EventManager(_resMan->detectFontExtended());
#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		_audio32 = new Audio32(_resMan);
	} else
#endif
		_audio = new AudioPlayer(_resMan);
#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		_video32 = new Video32(segMan, _eventMan);
	}
#endif
	_sync = new Sync(_resMan, segMan);

	// Create debugger console. It requires GFX and _gamestate to be initialized
	_console = new Console(this);
	setDebugger(_console);

	// The game needs to be initialized before the graphics system is initialized, as
	// the graphics code checks parts of the seg manager upon initialization (e.g. for
	// the presence of the fastCast object)
	if (!initGame()) { /* Initialize */
		warning("Game initialization failed: Aborting...");
		// TODO: Add an "init failed" error?
		return Common::kUnknownError;
	}

	// we try to find the super class address of the game object, we can't do that earlier
	const Object *gameObject = segMan->getObject(_gameObjectAddress);
	if (!gameObject) {
		warning("Could not get game object, aborting");
		return Common::kUnknownError;
	}

	script_adjust_opcode_formats();

	// Must be called after game_init(), as they use _features
	_kernel->loadKernelNames(_features);

	// Initialize all graphics related subsystems
	initGraphics();

	// Sound must be initialized after graphics because SysEx transfers at the
	// start of the game must pump the event loop to avoid making the OS think
	// that ScummVM is hanged, and pumping the event loop requires GfxCursor to
	// be initialized
	_soundCmd = new SoundCommandParser(_resMan, segMan, _kernel, _audio, _features->detectDoSoundType());

	syncSoundSettings();
	_guestAdditions->syncAudioOptionsFromScummVM();
	_guestAdditions->patchGameSaveRestore();
	setLauncherLanguage();

	// Check whether loading a savestate was requested
	int directSaveSlotLoading = ConfMan.getInt("save_slot");
	if (directSaveSlotLoading >= 0) {
		_gamestate->_delayedRestoreGameId = directSaveSlotLoading;

		// Jones only initializes its menus when restarting/restoring, thus set
		// the gameIsRestarting flag here before initializing. Fixes bug #6536.
		if (getGameId() == GID_JONES)
			_gamestate->gameIsRestarting = GAMEISRESTARTING_RESTORE;
	}

	// Show any special warnings for buggy scripts with severe game bugs,
	// which have been patched by Sierra
	if (getGameId() == GID_KQ7 && ConfMan.getBool("subtitles")) {
		showScummVMDialog(_("Subtitles are enabled, but subtitling in King's"
						  " Quest 7 was unfinished and disabled in the release"
						  " version of the game. ScummVM allows the subtitles"
						  " to be re-enabled, but because they were removed from"
						  " the original game, they do not always render"
						  " properly or reflect the actual game speech."
						  " This is not a ScummVM bug -- it is a problem with"
						  " the game's assets."));
	}

	// Show a warning if the user has selected a General MIDI device, no GM patch exists
	// (i.e. patch 4) and the game is one of the known 8 SCI1 games that Sierra has provided
	// after market patches for in their "General MIDI Utility".
	if (_soundCmd->getMusicType() == MT_GM && !ConfMan.getBool("native_mt32")) {
		if (!_resMan->findResource(ResourceId(kResourceTypePatch, 4), 0)) {
			switch (getGameId()) {
			case GID_ECOQUEST:
			case GID_HOYLE3:
			case GID_LSL1:
			case GID_LSL5:
			case GID_LONGBOW:
			case GID_SQ1:
			case GID_SQ4:
			case GID_FAIRYTALES:
				showScummVMDialog(_("You have selected General MIDI as a sound device. Sierra "
				                  "has provided after-market support for General MIDI for this "
				                  "game in their \"General MIDI Utility\". Please, apply this "
				                  "patch in order to enjoy MIDI music with this game. Once you "
				                  "have obtained it, you can unpack all of the included *.PAT "
				                  "files in your ScummVM extras folder and ScummVM will add the "
				                  "appropriate patch automatically. Alternatively, you can follow "
				                  "the instructions in the READ.ME file included in the patch and "
				                  "rename the associated *.PAT file to 4.PAT and place it in the "
				                  "game folder. Without this patch, General MIDI music for this "
				                  "game will sound badly distorted."));
				break;
			default:
				break;
			}
		}
	}

	if (gameHasFanMadePatch()) {
		warning("Fan made script patch detected");
	}

	if (getGameId() == GID_GK2 && ConfMan.getBool("subtitles") && !_resMan->testResource(ResourceId(kResourceTypeSync, 10))) {
		suggestDownloadGK2SubTitlesPatch();
	}

	runGame();

	ConfMan.flushToDisk();

	return Common::kNoError;
}

bool SciEngine::gameHasFanMadePatch() {
	struct FanMadePatchInfo {
		SciGameId gameID;
		uint16 targetScript;
		uint16 targetSize;
		uint16 patchedByteOffset;
		byte patchedByte;
	};

	const FanMadePatchInfo patchInfo[] = {
		// game        script    size  offset   byte
		// ** NRS Patches **************************
		{ GID_HOYLE3,     994,   2580,    656,  0x78 },
		{ GID_KQ1,         85,   5156,    631,  0x02 },
		{ GID_LAURABOW2,  994,   4382,      0,  0x00 },
		{ GID_LONGBOW,    994,   4950,   1455,  0x78 },	// English
		{ GID_LONGBOW,    994,   5020,   1469,  0x78 },	// German
		{ GID_LSL1,       803,    592,    342,  0x01 },
		{ GID_LSL3,       380,   6148,    195,  0x35 },
		{ GID_LSL5,       994,   4810,   1342,  0x78 },	// English
		{ GID_LSL5,       994,   4942,   1392,  0x76 },	// German
		// GOG includes PQ1 patches. Incompatibilities are resolved by our script patches
		{ GID_PQ1,        994,   4332,   1473,  0x78 },
		{ GID_PQ2,        200,  10614,      0,  0x00 },
		// GOG includes PQ3 patches. Incompatibilities are resolved by our script patches
		{ GID_PQ3,        994,   4686,   1291,  0x78 },	// English
		{ GID_PQ3,        994,   4734,   1283,  0x78 },	// German
		{ GID_QFG1VGA,    994,   4388,      0,  0x00 },
		{ GID_QFG3,       994,   4714,      2,  0x48 },
		// GOG includes QFG4 patches. Incompatibilities are resolved by our script patches
		{ GID_QFG4,       710,  11433,      0,  0x00 },
		{ GID_SQ1,        994,   4740,      0,  0x00 },
		{ GID_SQ5,        994,   4142,   1496,  0x78 },	// English/German/French
		//{ GID_SQ5,        994,   4148,      0,  0x00 },	// Italian - patched file is the same size as the original
		{ GID_SQ6,        380,  16308,  15042,  0x0C },	// English
		//{ GID_SQ6,        380,  11652,      0,  0x00 },	// German - patched file is the same size as the original
	};

	for (int i = 0; i < ARRAYSIZE(patchInfo); ++i) {
		if (patchInfo[i].gameID == getGameId()) {
			Resource *targetScript = _resMan->findResource(ResourceId(kResourceTypeScript, patchInfo[i].targetScript), 0);

			if (targetScript && targetScript->size() + 2 == patchInfo[i].targetSize) {
				if (patchInfo[i].patchedByteOffset == 0)
					return true;
				else if (targetScript->getUint8At(patchInfo[i].patchedByteOffset - 2) == patchInfo[i].patchedByte)
					return true;
			}
		}
	}

	return false;
}

void SciEngine::suggestDownloadGK2SubTitlesPatch() {
	Common::U32String altButton;
	Common::U32String downloadMessage;

	if (_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		altButton = _("Download patch");
		downloadMessage = _("(or click 'Download patch' button. But note - it only downloads, you will have to continue from there)\n");
	}
	else {
		altButton = "";
		downloadMessage = "";
	}

	int result = showScummVMDialog(_("GK2 has fan made subtitles, available thanks to the good people at SierraHelp.\n\n"
		"Installation:\n"
		"- download http://www.sierrahelp.com/Files/Patches/GabrielKnight/GK2Subtitles.zip\n" +
		downloadMessage +
		"- extract zip file\n"
		"- no need to run the .exe file\n"
		"- extract the .exe file with a file archiver, like 7-zip\n"
		"- create a PATCHES subdirectory inside your GK2 directory\n"
		"- copy the content of GK2Subtitles\\SUBPATCH to the PATCHES subdirectory\n"
		"- replace files with similar names\n"
		"- restart the game\n"), altButton, false);
	if (result) {
		char url[] = "http://www.sierrahelp.com/Files/Patches/GabrielKnight/GK2Subtitles.zip";
		_system->openUrl(url);
	}
}

bool SciEngine::initGame() {
	// Script 0 needs to be allocated here before anything else!
	int script0Segment = _gamestate->_segMan->getScriptSegment(0, SCRIPT_GET_LOCK);
	DataStack *stack = _gamestate->_segMan->allocateStack(VM_STACK_SIZE);

	_gamestate->initMessageState();
	_gamestate->gcCountDown = GC_INTERVAL - 1;

	// Script 0 should always be at segment 1
	if (script0Segment != 1) {
		debug(2, "Failed to instantiate script 0");
		return false;
	}

	_gamestate->initGlobals();
	_gamestate->_segMan->initSysStrings();

	_gamestate->r_acc = _gamestate->r_prev = NULL_REG;

	_gamestate->_executionStack.clear();    // Start without any execution stack
	_gamestate->executionStackBase = -1; // No vm is running yet
	_gamestate->_executionStackPosChanged = false;

	_gamestate->abortScriptProcessing = kAbortNone;
	_gamestate->gameIsRestarting = GAMEISRESTARTING_NONE;

	_gamestate->stack_base = stack->_entries;
	_gamestate->stack_top = stack->_entries + stack->_capacity;

	if (!_gamestate->_segMan->instantiateScript(0)) {
		error("initGame(): Could not instantiate script 0");
		return false;
	}

	// Reset parser
	if (_vocabulary)
		_vocabulary->reset();

	_gamestate->lastWaitTime = _gamestate->_screenUpdateTime = _system->getMillis();

	// Load game language into printLang property of game object
	setSciLanguage();

#ifdef ENABLE_SCI32
	_guestAdditions->sciEngineInitGameHook();
#endif

	return true;
}

void SciEngine::initGraphics() {
#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		_gfxPalette32 = new GfxPalette32(_resMan);
		_gfxRemap32 = new GfxRemap32();
	} else {
#endif
		_gfxPalette16 = new GfxPalette(_resMan, _gfxScreen);
		if (getGameId() == GID_QFG4DEMO || _resMan->testResource(ResourceId(kResourceTypeVocab, 184)))
			_gfxRemap16 = new GfxRemap(_gfxPalette16);
#ifdef ENABLE_SCI32
	}
#endif

	_gfxCache = new GfxCache(_resMan, _gfxScreen, _gfxPalette16);

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		// SCI32 graphic objects creation
		if (getPlatform() == Common::kPlatformMacintosh && _resMan->hasResourceType(kResourceTypeCursor)) {
			_gfxCursor32 = new GfxMacCursor32();
		} else {
			_gfxCursor32 = new GfxCursor32();
		}
		_gfxCompare = new GfxCompare(_gamestate->_segMan, _gfxCache, nullptr, _gfxCoordAdjuster);
		_gfxPaint32 = new GfxPaint32(_gamestate->_segMan);
		_gfxTransitions32 = new GfxTransitions32(_gamestate->_segMan);
		_gfxFrameout = new GfxFrameout(_gamestate->_segMan, _gfxPalette32, _gfxTransitions32, _gfxCursor32);
		_gfxCursor32->init(_gfxFrameout->getCurrentBuffer());
		_gfxText32 = new GfxText32(_gamestate->_segMan, _gfxCache);
		_gfxControls32 = new GfxControls32(_gamestate->_segMan, _gfxCache, _gfxText32);
		_gfxFrameout->run();
	} else {
#endif
		// SCI0-SCI1.1 graphic objects creation
		_gfxPorts = new GfxPorts(_gamestate->_segMan, _gfxScreen);
		_gfxCoordAdjuster = new GfxCoordAdjuster16(_gfxPorts);
		_gfxCursor = new GfxCursor(_resMan, _gfxPalette16, _gfxScreen, _gfxCoordAdjuster, _eventMan);
		_gfxCompare = new GfxCompare(_gamestate->_segMan, _gfxCache, _gfxScreen, _gfxCoordAdjuster);
		_gfxTransitions = new GfxTransitions(_gfxScreen, _gfxPalette16);
		_gfxPaint16 = new GfxPaint16(_resMan, _gamestate->_segMan, _gfxCache, _gfxPorts, _gfxCoordAdjuster, _gfxScreen, _gfxPalette16, _gfxTransitions, _audio);
		_gfxAnimate = new GfxAnimate(_gamestate, _scriptPatcher, _gfxCache, _gfxCompare, _gfxPorts, _gfxPaint16, _gfxScreen, _gfxPalette16, _gfxCursor, _gfxTransitions);
		_gfxText16 = new GfxText16(_gfxCache, _gfxPorts, _gfxPaint16, _gfxScreen, _gfxMacFontManager);
		_gfxControls16 = new GfxControls16(_gamestate->_segMan, _gfxPorts, _gfxPaint16, _gfxText16, _gfxScreen);
		_gfxMenu = new GfxMenu(_eventMan, _gamestate->_segMan, _gfxPorts, _gfxPaint16, _gfxText16, _gfxScreen, _gfxCursor);

		_gfxMenu->reset();

		_gfxPorts->init(_features->usesOldGfxFunctions(), _gfxPaint16, _gfxText16);
		_gfxPaint16->init(_gfxAnimate, _gfxText16);

		if (hasMacIconBar()) {
			_gfxMacIconBar = new GfxMacIconBar(_resMan, _eventMan, _gamestate->_segMan, _gfxScreen, _gfxPalette16);
		}
#ifdef ENABLE_SCI32
	}
#endif

	if (getSciVersion() < SCI_VERSION_2) {
		// Set default (EGA, amiga or resource 999) palette
		_gfxPalette16->setDefault();
	}
}

void SciEngine::initStackBaseWithSelector(Selector selector) {
	_gamestate->stack_base[0] = make_reg(0, (uint16)selector);
	_gamestate->stack_base[1] = NULL_REG;

	// Register the first element on the execution stack
	if (!send_selector(_gamestate, _gameObjectAddress, _gameObjectAddress, _gamestate->stack_base, 2, _gamestate->stack_base)) {
		printObject(_gameObjectAddress);
		error("initStackBaseWithSelector: error while registering the first selector in the call stack");
	}

}

void SciEngine::runGame() {
	setTotalPlayTime(17);

	initStackBaseWithSelector(SELECTOR(play)); // Call the play selector

	// Attach the debug console on game startup, if requested
	if (DebugMan.isDebugChannelEnabled(kDebugLevelOnStartup))
		_console->attach();

	_guestAdditions->reset();

	do {
		_gamestate->_executionStackPosChanged = false;
		run_vm(_gamestate);

		// Stop audio and sound components, unless loading a game.
		// EngineState::saveLoadWithSerializer has already handled that.
		if (_gamestate->abortScriptProcessing != kAbortLoadGame) {
			if (_audio) { // SCI16
				_audio->stopAllAudio();
			}
			_sync->stop();
			_soundCmd->clearPlayList();
		}

		// Clear execution stack
		_gamestate->_executionStack.clear();
		_gamestate->xs = nullptr;

		// Close all opened file handles
		_gamestate->_fileHandles.clear();
		_gamestate->_fileHandles.resize(5);

		_guestAdditions->sciEngineRunGameHook();

		if (_gamestate->abortScriptProcessing == kAbortRestartGame) {
			// SCI16 game has been restarted with kRestartGame16.
			// Reset engine state and prepare the VM to call the play method
			// on the next iteration, but set the gameIsRestarting flag so
			// that scripts can detect the restart with kGameIsRestarting.
			_gamestate->_segMan->resetSegMan();
			initGame();
			initStackBaseWithSelector(SELECTOR(play));
			_guestAdditions->patchGameSaveRestore();
			setLauncherLanguage();
			_gamestate->gameIsRestarting = GAMEISRESTARTING_RESTART;
			_gamestate->_throttleLastTime = 0;
			if (_gfxMenu)
				_gfxMenu->reset();
			_gamestate->abortScriptProcessing = kAbortNone;
			_guestAdditions->reset();
		} else if (_gamestate->abortScriptProcessing == kAbortLoadGame) {
			// Game has been restored from within the game or the launcher.
			// Prepare the VM to call the replay method of the game object
			// on the next iteration.
			_gamestate->abortScriptProcessing = kAbortNone;
			initStackBaseWithSelector(SELECTOR(replay));
			_guestAdditions->patchGameSaveRestore();
			setLauncherLanguage();
			_gamestate->shrinkStackToBase();
			_gamestate->abortScriptProcessing = kAbortNone;

			syncSoundSettings();
			_guestAdditions->syncAudioOptionsFromScummVM();
		} else {
			break;	// exit loop
		}
	} while (true);
}

// When `error` is called, this function adds additional SCI engine context to the message
// to help with bug reporting. It is critical that this function not crash, or else the
// original error message will be lost and the debugger will be unavailable. This function
// must not cause a second `error` call, or the original error message will also be unavailable,
// although we detect this to prevent infinite recursion and crashing with a stack overflow.
// This function must be cautious about the state it inspects, because it can be called at
// any time during the engine lifecycle.
void SciEngine::errorString(const char *buf_input, char *buf_output, int buf_output_size) {
	// safeguard to prevent infinite recursion in case there's a code path that calls `error`.
	if (_inErrorString) {
		warning("error called during errorString");
		Common::strlcpy(buf_output, buf_input, buf_output_size);
		return;
	}
	_inErrorString = true;

	// Detailed context can only be included if VM execution has begun.
	EngineState *s = _gamestate;
	if (s != nullptr && !s->_executionStack.empty() && _kernel != nullptr) {
		// Determine the name of the current function and the pc
		Common::String function;
		// Query the top-most stack frame even if it's not committed yet within the VM cycle.
		const ExecStack &call = s->_executionStack.back();
		reg_t pc = call.addr.pc;
		switch (call.type) {
		case EXEC_STACK_TYPE_CALL: { // Script function
			if (call.debugSelector != -1) {
				const char *objectName = s->_segMan->getObjectName(call.sendp);
				function = Common::String::format("%s::%s", objectName, _kernel->getSelectorName(call.debugSelector).c_str());
			} else if (call.debugExportId != -1) {
				function = Common::String::format("export %d", call.debugExportId);
			} else if (call.debugLocalCallOffset != -1) {
				function = Common::String::format("call %x", call.debugLocalCallOffset);
			}
			break;
		}
		case EXEC_STACK_TYPE_KERNEL: { // Kernel function
			if (call.debugKernelSubFunction == -1) {
				function = Common::String::format("k%s", _kernel->getKernelName(call.debugKernelFunction).c_str());
			} else {
				function = Common::String::format("k%s", _kernel->getKernelName(call.debugKernelFunction, call.debugKernelSubFunction).c_str());
			}
			// Kernel calls do not have a pc. walk the stack back to the most recent for script number.
			Common::List<ExecStack>::const_iterator it;
			for (it = s->_executionStack.reverse_begin(); it != s->_executionStack.end(); --it) {
				if (it->type != EXEC_STACK_TYPE_KERNEL) {
					pc = it->addr.pc;
					break;
				}
			}
			break;
		}
		default:
			break;
		}

		// Get game version string from globals. Could also read from VERSION file
		// as a fallback, but I/O seems risky for this error handler.
		// Version global may not be set yet if the error occurs early in script init.
		Common::String version = s->getGameVersionFromGlobal();
		if (!version.empty()) {
			version.insertChar('-', 0);
		}

		// There are two script numbers we care about: the current room and the
		// the script that was executing when the error occurred. These are often
		// the same, so to save space, only include both when they are different.
		uint16 roomNumber = s->currentRoomNumber();
		Common::String scriptStr = Common::String::format("%d", roomNumber);
		Script *script = s->_segMan->getScriptIfLoaded(pc.getSegment());
		if (script != nullptr) {
			uint16 scriptNumber = script->getScriptNumber();
			if (roomNumber != scriptNumber) {
				scriptStr += Common::String::format("/%d", scriptNumber);
			}
		}

		// The error string is a deliberately terse single-line header, because
		// the goal is for bug reports to automatically include useful context
		// even when users only provide the single error message.
		// Target name is used because this identifies the game and includes
		// useful information about variants such as language.
		// Note that only the pc offset is included, because that's all that's
		// needed to identify the current instruction when the script is known.
		snprintf(buf_output, buf_output_size, "[%s%s %s %s @ %04x]: %s",
			_targetName.c_str(), version.c_str(), scriptStr.c_str(),
			function.c_str(), pc.getOffset(), buf_input);
	} else {
		// VM not initialized yet, so just copy over the target name and error message.
		snprintf(buf_output, buf_output_size, "[%s]: %s", _targetName.c_str(), buf_input);
	}
	_inErrorString = false;
}

// Invoked by debugger when a severe error occurs
void SciEngine::severeError() {
	if (_gamestate) {
		if (!_gamestate->_executionStack.empty()) {
			ExecStack &xs = _gamestate->_executionStack.back();
			xs.addr.pc.setOffset(_debugState.old_pc_offset);
			xs.sp = _debugState.old_sp;
		}
	}

	_debugState.runningStep = 0; // Stop multiple execution
	_debugState.seeking = kDebugSeekNothing; // Stop special seeks
}

Console *SciEngine::getSciDebugger() {
	return _console;
}

const char *SciEngine::getGameIdStr() const {
	return _gameDescription->gameId;
}

Common::Language SciEngine::getLanguage() const {
	return _gameDescription->language;
}

bool SciEngine::isLanguageRTL() const {
	return getLanguage() == Common::HE_ISR;
}

Common::Platform SciEngine::getPlatform() const {
	return _gameDescription->platform;
}

bool SciEngine::isDemo() const {
	return _gameDescription->flags & ADGF_DEMO;
}

bool SciEngine::isCD() const {
	return _gameDescription->flags & ADGF_CD;
}

bool SciEngine::useHiresGraphics() const {
	return _useHiresGraphics;
}

bool SciEngine::isBE() const{
	switch(_gameDescription->platform) {
	case Common::kPlatformAmiga:
	case Common::kPlatformMacintosh:
		return true;
	default:
		return false;
	}
}

bool SciEngine::hasParser() const {
	// Only SCI0, SCI01 and SCI1 EGA games used a parser, along with
	//  multilingual LSL3 and SQ3 Amiga which are SCI_VERSION_1_MIDDLE
	return getSciVersion() <= SCI_VERSION_1_EGA_ONLY ||
			getGameId() == GID_LSL3 || getGameId() == GID_SQ3;
}

bool SciEngine::hasMacFonts() const {
	return _gfxMacFontManager != nullptr;
}

bool SciEngine::hasMacIconBar() const {
	return _resMan->isSci11Mac() &&
			(getGameId() == GID_KQ6 || getGameId() == GID_FREDDYPHARKAS);
}

bool SciEngine::hasMacSaveRestoreDialogs() const {
	return _gameDescription->platform == Common::kPlatformMacintosh &&
			(getSciVersion() <= SCI_VERSION_2_1_EARLY ||
			 getGameId() == GID_GK2 ||
			 getGameId() == GID_SQ6 ||
			 getGameId() == GID_LIGHTHOUSE);
}

Common::String SciEngine::getSavegameName(int nr) const {
	return _targetName + Common::String::format(".%03d", nr);
}

Common::String SciEngine::getSavegamePattern() const {
	return _targetName + ".###";
}

Common::String SciEngine::getFilePrefix() const {
	return _targetName;
}

Common::String SciEngine::wrapFilename(const Common::String &name) const {
	Common::String prefix = getFilePrefix() + "-";
	if (name.hasPrefix(prefix.c_str()))
		return name;
	else
		return prefix + name;
}

Common::String SciEngine::unwrapFilename(const Common::String &name) const {
	Common::String prefix = getFilePrefix() + "-";
	if (name.hasPrefix(prefix.c_str()))
		return Common::String(name.c_str() + prefix.size());
	else
		return name;
}

const char *SciEngine::getGameObjectName() {
	return _gamestate->_segMan->getObjectName(_gameObjectAddress);
}

int SciEngine::inQfGImportRoom() const {
	if (_gameId == GID_QFG2 && _gamestate->currentRoomNumber() == 805) {
		// QFG2 character import screen
		return 2;
	} else if (_gameId == GID_QFG3 && _gamestate->currentRoomNumber() == 54) {
		// QFG3 character import screen
		return 3;
	} else if (_gameId == GID_QFG4 && _gamestate->currentRoomNumber() == 54) {
		return 4;
	}
	return 0;
}

void SciEngine::showQfgImportMessageBox() const {
	showScummVMDialog(_("Characters saved inside ScummVM are shown "
			"automatically. Character files saved in the original "
			"interpreter need to be put inside ScummVM's saved games "
			"directory and a prefix needs to be added depending on which "
			"game it was saved in: 'qfg1-' for Quest for Glory 1, 'qfg2-' "
			"for Quest for Glory 2. Example: 'qfg2-thief.sav'."));
}

void SciEngine::sleep(uint32 msecs) {
	if (!msecs) {
		return;
	}

	const uint32 wakeUpTime = _system->getMillis() + msecs;

	for (;;) {
		// let backend process events and update the screen
		_eventMan->getSciEvent(kSciEventPeek);

		// There is no point in waiting any more if we are just waiting to quit
		if (shouldQuit()) {
			return;
		}

#ifdef ENABLE_SCI32
		// If a game is in a wait loop, kFrameOut is not called, but mouse
		// movement is still occurring and the screen needs to be updated to
		// reflect it
		if (getSciVersion() >= SCI_VERSION_2) {
			_gfxFrameout->updateScreen();
		}
#endif
		uint32 time = _system->getMillis();
		if (time + 10 < wakeUpTime) {
			_system->delayMillis(10);
		} else {
			if (time < wakeUpTime)
				_system->delayMillis(wakeUpTime - time);
			break;
		}
	}
}

void SciEngine::setLauncherLanguage() {
	if (_gameDescription->flags & ADGF_ADDENGLISH) {
		// If game is multilingual
		Common::Language chosenLanguage = Common::parseLanguage(ConfMan.get("language"));
		uint16 languageToSet = 0;

		switch (chosenLanguage) {
		case Common::EN_ANY:
			// and English was selected as language
			languageToSet = K_LANG_ENGLISH;
			break;
		case Common::JA_JPN: {
			// Set Japanese for FM-Towns games
			// KQ5 on FM-Towns has no initial language set
			if (getPlatform() == Common::kPlatformFMTowns) {
				languageToSet = K_LANG_JAPANESE;
			}
		}
		default:
			break;
		}

		if (languageToSet) {
			if (SELECTOR(printLang) != -1) // set text language
				writeSelectorValue(_gamestate->_segMan, _gameObjectAddress, SELECTOR(printLang), languageToSet);
			if (SELECTOR(parseLang) != -1) // and set parser language as well
				writeSelectorValue(_gamestate->_segMan, _gameObjectAddress, SELECTOR(parseLang), languageToSet);
		}
	}
}

void SciEngine::pauseEngineIntern(bool pause) {
	_mixer->pauseAll(pause);
	if (_soundCmd)
		_soundCmd->pauseAll(pause);
}

void SciEngine::syncSoundSettings() {
	updateSoundMixerVolumes();
	_guestAdditions->syncSoundSettingsFromScummVM();
}

void SciEngine::updateSoundMixerVolumes() {
	Engine::syncSoundSettings();

	// ScummVM adjusts the software mixer volume in Engine::syncSoundSettings,
	// but MIDI either does not run through the ScummVM mixer (e.g. hardware
	// synth) or it uses a kPlainSoundType channel type, so the master MIDI
	// volume must be adjusted here for MIDI playback volume to be correct
	if (_soundCmd) {
		const int16 musicVolume = (ConfMan.getInt("music_volume") + 1) * MUSIC_MASTERVOLUME_MAX / Audio::Mixer::kMaxMixerVolume;
		_soundCmd->setMasterVolume(ConfMan.getBool("mute") ? 0 : musicVolume);
	}
}

void SciEngine::loadMacExecutable() {
	Common::Path filename = _resMan->getMacExecutableName();
	if (filename.empty())
		return;

	if (!_macExecutable.open(filename) || !_macExecutable.hasResFork()) {
		// KQ6/Freddy require the executable to load their icon bar palettes
		if (hasMacIconBar())
			error("Could not load Mac resource fork '%s'", filename.toString().c_str());
	}
}

void SciEngine::loadMacFonts() {
	// Load Mac fonts from the appropriate place, depending on the game.
	// If we're unable to load Mac fonts, then fall back to using SCI fonts.
	// Mac font support was added after these games were supported, so it's
	// important to not require that fonts be present.
	switch (getGameId()) {
	case GID_CASTLEBRAIN:
	case GID_FREDDYPHARKAS:
	// case GID_KQ5: // not supported yet
	case GID_KQ6:
	case GID_LSL1:
	case GID_LSL5:
	case GID_SQ1:
		// These Mac games have fonts in the resource fork of their executable
		// along with a SCI to Mac font mapping table.
		if (_macExecutable.hasResFork()) {
			_gfxMacFontManager = new GfxMacFontManager(&_macExecutable);
			if (!_gfxMacFontManager->hasFonts()) {
				delete _gfxMacFontManager;
				_gfxMacFontManager = nullptr;
			}
		} else {
			Common::Path filename = _resMan->getMacExecutableName();
			warning("Macintosh executable \"%s\" not found, using SCI fonts", filename.toString().c_str());
		}
		break;
	case GID_LSL6:
	case GID_QFG1VGA:
		// These Mac games have interpreters that are hard-coded to use Palatino.
		// Attempt to load Palatino from classicmacfonts.dat.
		_gfxMacFontManager = new GfxMacFontManager();
		if (!_gfxMacFontManager->hasFonts()) {
			warning("Classic Macintosh fonts not found, using SCI fonts");
			delete _gfxMacFontManager;
			_gfxMacFontManager = nullptr;
		}
		break;
	default:
		break;
	}
}

uint32 SciEngine::getTickCount() {
	return getTotalPlayTime() * 60 / 1000;
}
void SciEngine::setTickCount(const uint32 ticks) {
	return setTotalPlayTime(ticks * 1000 / 60);
}
} // End of namespace Sci
