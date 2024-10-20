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

#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_fgetc
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr
#define FORBIDDEN_SYMBOL_EXCEPTION_stdin
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "common/archive.h"
#include "common/debug-channels.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/config-manager.h"
#include "common/compression/stuffit.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "graphics/renderer.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
#include "graphics/opengl/context.h"
#endif

#include "gui/error.h"
#include "gui/gui-manager.h"
#include "gui/message.h"

#include "image/png.h"

#include "engines/engine.h"
#include "engines/util.h"

#include "engines/grim/md5check.h"
#include "engines/grim/md5checkdialog.h"
#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/lua.h"
#include "engines/grim/lua_v1.h"
#include "engines/grim/emi/poolsound.h"
#include "engines/grim/emi/layer.h"
#include "engines/grim/actor.h"
#include "engines/grim/movie/movie.h"
#include "engines/grim/savegame.h"
#include "engines/grim/registry.h"
#include "engines/grim/resource.h"
#include "engines/grim/localize.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/font.h"
#include "engines/grim/primitives.h"
#include "engines/grim/objectstate.h"
#include "engines/grim/set.h"
#include "engines/grim/sound.h"
#include "engines/grim/debugger.h"
#include "engines/grim/remastered/overlay.h"
#include "engines/grim/remastered/lua_remastered.h"
#include "engines/grim/remastered/commentary.h"
#include "engines/grim/imuse/imuse.h"
#include "engines/grim/emi/sound/emisound.h"
#include "engines/grim/lua/lua.h"

namespace Grim {

GrimEngine *g_grim = nullptr;
GfxBase *g_driver = nullptr;
int g_imuseState = -1;

GrimEngine::GrimEngine(OSystem *syst, uint32 gameFlags, GrimGameType gameType, Common::Platform platform, Common::Language language) :
		Engine(syst), _currSet(nullptr), _selectedActor(nullptr), _pauseStartTime(0), _language(0) {
	g_grim = this;

	setDebugger(new Debugger());
	_gameType = gameType;
	_gameFlags = gameFlags;
	_gamePlatform = platform;
	_gameLanguage = language;

	if (getGameType() == GType_GRIM)
		g_registry = new Registry();
	else
		g_registry = nullptr;

	g_resourceloader = nullptr;
	g_localizer = nullptr;
	g_movie = nullptr;
	g_imuse = nullptr;

	// Set default settings
	ConfMan.registerDefault("use_arb_shaders", true);

	_showFps = ConfMan.getBool("show_fps");

	_softRenderer = true;

	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 192);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_currSet = nullptr;
	_selectedActor = nullptr;
	_controlsEnabled = new bool[KEYCODE_EXTRA_LAST];
	_controlsState = new bool[KEYCODE_EXTRA_LAST];
	for (int i = 0; i < KEYCODE_EXTRA_LAST; i++) {
		_controlsEnabled[i] = false;
		_controlsState[i] = false;
	}
	_joyAxisPosition = new float[NUM_JOY_AXES];
	for (int i = 0; i < NUM_JOY_AXES; i++) {
		_joyAxisPosition[i] = 0;
	}
	_speechMode = TextAndVoice;
	_textSpeed = 7;
	_mode = _previousMode = NormalMode;
	_flipEnable = true;
	int speed = ConfMan.getInt("engine_speed");
	if (speed == 0) {
		_speedLimitMs = 0;
	} else if (speed < 0 || speed > 100) {
		_speedLimitMs = 1000 / 60;
		ConfMan.setInt("engine_speed", 1000 / _speedLimitMs);
	} else {
		_speedLimitMs = 1000 / speed;
	}
	_listFilesIter = nullptr;
	_savedState = nullptr;
	_fps[0] = 0;
	_iris = new Iris();
	_buildActiveActorsList = false;
	_justSaveLoaded = false;

	Color c(0, 0, 0);

	_printLineDefaults.setX(0);
	_printLineDefaults.setY(100);
	_printLineDefaults.setWidth(0);
	_printLineDefaults.setHeight(0);
	_printLineDefaults.setFGColor(c);
	_printLineDefaults.setFont(nullptr);
	_printLineDefaults.setJustify(TextObject::LJUSTIFY);

	_sayLineDefaults.setX(0);
	_sayLineDefaults.setY(100);
	_sayLineDefaults.setWidth(0);
	_sayLineDefaults.setHeight(0);
	_sayLineDefaults.setFGColor(c);
	_sayLineDefaults.setFont(nullptr);
	_sayLineDefaults.setJustify(TextObject::CENTER);

	_blastTextDefaults.setX(0);
	_blastTextDefaults.setY(200);
	_blastTextDefaults.setWidth(0);
	_blastTextDefaults.setHeight(0);
	_blastTextDefaults.setFGColor(c);
	_blastTextDefaults.setFont(nullptr);
	_blastTextDefaults.setJustify(TextObject::LJUSTIFY);

	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "movies"); // Add 'movies' subdirectory for the demo
	SearchMan.addSubDirectoryMatching(gameDataDir, "credits");
	SearchMan.addSubDirectoryMatching(gameDataDir, "widescreen");


	// Remastered:
	if (isRemastered()) {
		for (uint32 i = 0; i < kNumCutscenes; i++) {
			_cutsceneEnabled[i] = false;
		}
		for (uint32 i = 0; i < kNumConcepts; i++) {
			_conceptEnabled[i] = false;
		}

		_saveMeta1 = "";
		_saveMeta2 = 0;
		_saveMeta3 = "";
	}
	_commentary = nullptr;
}

GrimEngine::~GrimEngine() {
	delete[] _controlsEnabled;
	delete[] _controlsState;
	delete[] _joyAxisPosition;

	clearPools();

	delete LuaBase::instance();
	if (g_registry) {
		g_registry->save();
		delete g_registry;
		g_registry = nullptr;
	}
	delete g_movie;
	g_movie = nullptr;
	delete g_imuse;
	g_imuse = nullptr;
	delete g_emiSound;
	g_emiSound = nullptr;
	delete g_sound;
	g_sound = nullptr;
	delete g_localizer;
	g_localizer = nullptr;
	delete g_resourceloader;
	g_resourceloader = nullptr;
	delete g_driver;
	g_driver = nullptr;
	delete _iris;

	// Remastered:
	delete _commentary;

	ConfMan.flushToDisk();

	g_grim = nullptr;
}

void GrimEngine::clearPools() {
	Set::getPool().deleteObjects();
	Actor::getPool().deleteObjects();
	PrimitiveObject::getPool().deleteObjects();
	TextObject::getPool().deleteObjects();
	Bitmap::getPool().deleteObjects();
	BitmapFont::getPool().deleteObjects();
	FontTTF::getPool().deleteObjects();
	ObjectState::getPool().deleteObjects();

	_currSet = nullptr;
}

LuaBase *GrimEngine::createLua() {
	if (isRemastered()) {
		return new Lua_Remastered();
	} else {
		return new Lua_V1();
	}
}

GfxBase *GrimEngine::createRenderer(int screenW, int screenH) {
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::Renderer::parseTypeCode(rendererConfig);
	uint32 availableRendererTypes = Graphics::Renderer::getAvailableTypes();

	availableRendererTypes &=
#if defined(USE_OPENGL_GAME)
			Graphics::kRendererTypeOpenGL |
#endif
#if defined(USE_OPENGL_SHADERS)
			Graphics::kRendererTypeOpenGLShaders |
#endif
#if defined(USE_TINYGL)
			Graphics::kRendererTypeTinyGL |
#endif
			0;

	// For Grim Fandango, Korean fan translation can only use OpenGL renderer
	if (getGameType() == GType_GRIM && g_grim->getGameLanguage() == Common::KO_KOR) {
		availableRendererTypes &= ~Graphics::kRendererTypeOpenGLShaders;
		availableRendererTypes &= ~Graphics::kRendererTypeTinyGL;
	}

	// For Grim Fandango, OpenGL renderer without shaders is preferred if available
	if (desiredRendererType == Graphics::kRendererTypeDefault &&
		(availableRendererTypes & Graphics::kRendererTypeOpenGL) &&
	    getGameType() == GType_GRIM) {
		availableRendererTypes &= ~Graphics::kRendererTypeOpenGLShaders;
	}

	// Not supported yet.
	if (getLanguage() == Common::Language::ZH_CHN || getLanguage() == Common::Language::ZH_TWN
		|| getGameLanguage() == Common::Language::ZH_CHN || getGameLanguage() == Common::Language::ZH_TWN)
		availableRendererTypes &= ~Graphics::kRendererTypeOpenGLShaders;

	Graphics::RendererType matchingRendererType = Graphics::Renderer::getBestMatchingType(desiredRendererType, availableRendererTypes);

	_softRenderer = matchingRendererType == Graphics::kRendererTypeTinyGL;
	if (!_softRenderer) {
		initGraphics3d(screenW, screenH);
	} else {
		initGraphics(screenW, screenH, nullptr);
	}

	GfxBase *renderer = nullptr;
#if defined(USE_OPENGL_SHADERS)
	if (matchingRendererType == Graphics::kRendererTypeOpenGLShaders) {
		renderer = CreateGfxOpenGLShader();
	}
#endif
#if defined(USE_OPENGL_GAME)
	if (matchingRendererType == Graphics::kRendererTypeOpenGL) {
		renderer = CreateGfxOpenGL();
	}
#endif
#if defined(USE_TINYGL)
	if (matchingRendererType == Graphics::kRendererTypeTinyGL) {
		renderer = CreateGfxTinyGL();
	}
#endif

	if (!renderer) {
		/* We should never end up here, getBestMatchingRendererType would have failed before */
		error("Unable to create a renderer");
	}

	renderer->setupScreen(screenW, screenH);
	renderer->loadEmergFont();
	return renderer;
}

const char *GrimEngine::getUpdateFilename() {
	if (!(getGameFlags() & ADGF_DEMO))
		return "gfupd101.exe";
	else
		return nullptr;
}

Common::Error GrimEngine::run() {
	// Try to see if we have the EMI Mac installer present
	// Currently, this requires the data fork to be standalone
	if (getGameType() == GType_MONKEY4) {
		if (SearchMan.hasFile("Monkey Island 4 Installer")) {
			Common::Archive *archive = Common::createStuffItArchive("Monkey Island 4 Installer", true);

			if (archive)
				SearchMan.add("Monkey Island 4 Installer", archive, 0, true);
		}
		if (SearchMan.hasFile("EFMI Installer")) {
			Common::Archive *archive = Common::createStuffItArchive("EFMI Installer", true);

			if (archive)
				SearchMan.add("EFMI Installer", archive, 0, true);
		}
	}

	ConfMan.registerDefault("check_gamedata", true);
	if (ConfMan.getBool("check_gamedata") && !isRemastered()) {
		MD5CheckDialog d;
		if (!d.runModal()) {
			Common::U32String confirmString = Common::U32String::format(_(
			        "ScummVM found some problems with your game data files.\n"
			        "Running ScummVM nevertheless may cause game bugs or even crashes.\n"
			        "Do you still want to run %s?"),
			        GType_MONKEY4 == getGameType() ? "Escape From Monkey Island" : "Grim Fandango"
			        );
			GUI::MessageDialog msg(confirmString, _("Yes"), _("No"));
			if (msg.runModal() != GUI::kMessageOK) {
				return Common::kUserCanceled;
			}
		}

		ConfMan.setBool("check_gamedata", false);
		ConfMan.flushToDisk();
	}

	g_resourceloader = new ResourceLoader();
	bool demo = getGameFlags() & ADGF_DEMO;
	if (getGameType() == GType_GRIM)
		g_movie = CreateSmushPlayer(demo);
	else if (getGameType() == GType_MONKEY4) {
		if (_gamePlatform == Common::kPlatformPS2)
			g_movie = CreateMpegPlayer();
		else
			g_movie = CreateBinkPlayer(demo);
	}
	if (getGameType() == GType_GRIM) {
		g_imuse = new Imuse(20, demo);
		g_emiSound = nullptr;
		if (g_grim->isRemastered()) {
			// This must happen here, since we need the resource loader set up.
			_commentary = new Commentary();
		}
	} else if (getGameType() == GType_MONKEY4) {
		g_emiSound = new EMISound(20);
		g_imuse = nullptr;
	}
	g_sound = new SoundPlayer();

	if (getGameType() == GType_GRIM && g_grim->isRemastered()) {
		g_driver = createRenderer(1600, 900);
	} else {
		g_driver = createRenderer(640, 480);
	}

	if (getGameType() == GType_MONKEY4 && getGameLanguage() == Common::Language::ZH_TWN) {
		_transcodeChineseToSimplified = ConfMan.hasKey("language") && (Common::parseLanguage(ConfMan.get("language")) == Common::Language::ZH_CHN);

		if (_transcodeChineseToSimplified) {
			FontTTF *f = new FontTTF();
			f->loadTTFFromArchive("NotoSansSC-Regular.otf", 1200);
			_overrideFont = f;
		} else {
			Common::File img, imgmap;
			if (img.open("font.tga") && imgmap.open("map.bin")) {
				BitmapFont *f = new BitmapFont();
				f->loadTGA("font.tga", &imgmap, &img);
				_overrideFont = f;
			}
		}
	}

	if (getGameType() == GType_MONKEY4 && SearchMan.hasFile("AMWI.m4b")) {
		// Play EMI Mac Aspyr logo
		playAspyrLogo();
	}

	Bitmap *splash_bm = nullptr;
	if (!(_gameFlags & ADGF_DEMO) && getGameType() == GType_GRIM)
		splash_bm = Bitmap::create("splash.bm");
	else if ((_gameFlags & ADGF_DEMO) && getGameType() == GType_MONKEY4)
		splash_bm = Bitmap::create("splash.til");
	else if (getGamePlatform() == Common::kPlatformPS2 && getGameType() == GType_MONKEY4)
		splash_bm = Bitmap::create("load.tga");

	g_driver->clearScreen();

	if (splash_bm != nullptr)
		splash_bm->draw();

	// This flipBuffer() may make the OpenGL renderer show garbage instead of the splash,
	// while the TinyGL renderer needs it.
	if (_softRenderer)
		g_driver->flipBuffer();

	LuaBase *lua = createLua();

	lua->registerOpcodes();
	lua->registerLua();

	// One version of the demo doesn't set the demo flag in scripts.
	if (getGameType() == GType_GRIM && _gameFlags & ADGF_DEMO) {
		lua->forceDemo();
	}

	//Initialize Localizer first. In system-script are already localizeable Strings
	g_localizer = new Localizer();
	lua->loadSystemScript();
	lua->boot();

	_savegameLoadRequest = false;
	_savegameSaveRequest = false;

	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		loadGameState(ConfMan.getInt("save_slot"));
	}

	g_grim->setMode(NormalMode);
	delete splash_bm;
	g_grim->mainLoop();

	return Common::kNoError;
}

Common::KeymapArray GrimEngine::initKeymapsGrim(const char *target) {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "grim", "Grim Fandango");
	Action *act;

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setKeyEvent(KEYCODE_UP);
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(KEYCODE_DOWN);
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setKeyEvent(KEYCODE_LEFT);
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setKeyEvent(KEYCODE_RIGHT);
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("BRUN", _("Run"));
	act->setKeyEvent(KeyState(KEYCODE_LSHIFT));
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("EXAM", _("Examine"));
	act->setKeyEvent(KeyState(KEYCODE_e, 'e'));
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action("BUSE", _("Use/Talk"));
	act->setKeyEvent(KeyState(KEYCODE_u, 'u'));
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("PICK", _("Pick up/Put away"));
	act->setKeyEvent(KeyState(KEYCODE_p, 'p'));
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("INVT", _("Inventory"));
	act->setKeyEvent(KeyState(KEYCODE_i, 'i'));
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	act = new Action("SKLI", _("Skip dialog lines"));
	act->setKeyEvent(KeyState(KEYCODE_PERIOD, '.'));
	act->addDefaultInputMapping("PERIOD");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionSkip, _("Skip"));
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("RETURN", _("Confirm"));
	act->setKeyEvent(KeyState(KEYCODE_RETURN, ASCII_RETURN));
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("GMNU", _("Menu"));
	act->setKeyEvent(KeyState(KEYCODE_F1));
	act->addDefaultInputMapping("JOY_GUIDE");
	engineKeyMap->addAction(act);

	act = new Action("PAUSE", _("Pause"));
	act->setKeyEvent(KeyState(KEYCODE_PAUSE));
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

Common::KeymapArray GrimEngine::initKeymapsEMI(const char *target) {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "monkey4", "Escape from the Monkey Island");
	Action *act;

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setKeyEvent(KEYCODE_UP);
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(KEYCODE_DOWN);
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setKeyEvent(KEYCODE_LEFT);
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setKeyEvent(KEYCODE_RIGHT);
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("COUP", _("Cycle Objects Up"));
	act->setKeyEvent(KeyState(KEYCODE_PAGEUP));
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	engineKeyMap->addAction(act);

	act = new Action("CODW", _("Cycle Objects Down"));
	act->setKeyEvent(KeyState(KEYCODE_PAGEDOWN));
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	engineKeyMap->addAction(act);

	act = new Action("BRUN", _("Run"));
	act->setKeyEvent(KeyState(KEYCODE_LSHIFT));
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("QEXT", _("Quick Room Exit"));
	act->setKeyEvent(KeyState(KEYCODE_o, 'o'));
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("EXAM", _("Examine/Look"));
	act->setKeyEvent(KeyState(KEYCODE_e, 'e'));
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action("BUSE", _("Use/Talk"));
	act->setKeyEvent(KeyState(KEYCODE_u, 'u'));
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("PICK", _("Pick up/Put away"));
	act->setKeyEvent(KeyState(KEYCODE_KP_PLUS, '+'));
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("INVT", _("Inventory"));
	act->setKeyEvent(KeyState(KEYCODE_INSERT, 'i'));
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	act = new Action("SKLI", _("Skip dialog lines"));
	act->setKeyEvent(KeyState(KEYCODE_PERIOD, '.'));
	act->addDefaultInputMapping("PERIOD");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionSkip, _("Skip"));
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("RETURN", _("Confirm"));
	act->setKeyEvent(KeyState(KEYCODE_RETURN, ASCII_RETURN));
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("GMNU", _("Menu"));
	act->setKeyEvent(KeyState(KEYCODE_F1, 0));
	act->addDefaultInputMapping("JOY_GUIDE");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

void GrimEngine::playAspyrLogo() {
	// A trimmed down version of the code found in mainloop
	// for the purpose of playing the Aspyr-logo.
	// The reason for this, is that the logo needs a different
	// codec than all the other videos (which are Bink).
	// Code is provided to keep within the fps-limit, as well as to
	// allow for pressing ESC to skip the movie.
	MoviePlayer *defaultPlayer = g_movie;
	g_movie = CreateQuickTimePlayer();
	g_movie->play("AMWI.m4b", false, 0, 0);
	setMode(SmushMode);
	while (g_movie->isPlaying()) {
		_doFlip = true;
		uint32 startTime = g_system->getMillis();

		updateDisplayScene();
		doFlip();
		// Process events to allow the user to skip the logo.
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			// Ignore everything but ESC when movies are playing
			Common::EventType type = event.type;
			if (type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				g_movie->stop();
				break;
			}
		}

		uint32 endTime = g_system->getMillis();
		if (startTime > endTime)
			continue;
		uint32 diffTime = endTime - startTime;
		if (_speedLimitMs == 0)
			continue;
		if (diffTime < _speedLimitMs) {
			uint32 delayTime = _speedLimitMs - diffTime;
			g_system->delayMillis(delayTime);
		}
	}
	delete g_movie;
	setMode(NormalMode);
	g_movie = defaultPlayer;
}

Common::Error GrimEngine::loadGameState(int slot) {
	assert(slot >= 0);
	if (getGameType() == GType_MONKEY4) {
		if (getGamePlatform() == Common::kPlatformPS2) {
			_savegameFileName = Common::String::format("efmi%03d.ps2", slot);
		} else {
			_savegameFileName = Common::String::format("efmi%03d.gsv", slot);
		}
	} else {
		_savegameFileName = Common::String::format("grim%02d.gsv", slot);
	}
	_savegameLoadRequest = true;
	return Common::kNoError;
}

void GrimEngine::handlePause() {
	if (!LuaBase::instance()->callback("pauseHandler")) {
		error("handlePause: invalid handler");
	}
}

void GrimEngine::handleExit() {
	if (!LuaBase::instance()->callback("exitHandler")) {
		error("handleExit: invalid handler");
	}
}

void GrimEngine::handleUserPaint() {
	if (!LuaBase::instance()->callback("userPaintHandler")) {
		error("handleUserPaint: invalid handler");
	}
}

void GrimEngine::cameraChangeHandle(int prev, int next) {
	LuaObjects objects;
	objects.add(prev);
	objects.add(next);
	LuaBase::instance()->callback("camChangeHandler", objects);
}

void GrimEngine::cameraPostChangeHandle(int num) {
	LuaObjects objects;
	objects.add(num);
	LuaBase::instance()->callback("postCamChangeHandler", objects);
}

void GrimEngine::savegameCallback() {
	if (!LuaBase::instance()->callback("saveGameCallback")) {
		error("GrimEngine::savegameCallback: invalid handler");
	}
}

void GrimEngine::handleDebugLoadResource() {
	void *resource = nullptr;
	int c, i = 0;
	char buf[513];

	// Tool for debugging the loading of a particular resource without
	// having to actually make it all the way to it in the game
	fprintf(stderr, "Enter resource to load (extension specifies type): ");
	while (i < 512 && (c = fgetc(stdin)) != EOF && c != '\n')
		buf[i++] = c;

	buf[i] = '\0';
	if (strstr(buf, ".key"))
		resource = (void *)g_resourceloader->loadKeyframe(buf);
	else if (strstr(buf, ".zbm") || strstr(buf, ".bm"))
		resource = (void *)Bitmap::create(buf);
	else if (strstr(buf, ".cmp"))
		resource = (void *)g_resourceloader->loadColormap(buf);
	else if (strstr(buf, ".cos"))
		resource = (void *)g_resourceloader->loadCostume(buf, nullptr, nullptr);
	else if (strstr(buf, ".lip"))
		resource = (void *)g_resourceloader->loadLipSync(buf);
	else if (strstr(buf, ".snm"))
		resource = (void *)g_movie->play(buf, false, 0, 0);
	else if (strstr(buf, ".wav") || strstr(buf, ".imu")) {
		if (g_imuse)
			g_imuse->startSfx(buf);
		resource = (void *)1;
	} else if (strstr(buf, ".mat")) {
		CMap *cmap = g_resourceloader->loadColormap("item.cmp");
		warning("Default colormap applied to resources loaded in this fashion");
		// Default to repeating the texture as in GRIM
		resource = (void *)g_resourceloader->loadMaterial(buf, cmap, false);
	} else {
		warning("Resource type not understood");
	}
	if (!resource)
		warning("Requested resouce (%s) not found", buf);
}

void GrimEngine::drawTextObjects() {
	for (TextObject *t : TextObject::getPool()) {
		t->draw();
	}
}

void GrimEngine::playIrisAnimation(Iris::Direction dir, int x, int y, int time) {
	_iris->play(dir, x, y, time);
}

void GrimEngine::luaUpdate() {
	if (_savegameLoadRequest || _savegameSaveRequest || _changeHardwareState)
		return;

	// Update timing information
	unsigned newStart = g_system->getMillis();
	if (newStart < _frameStart) {
		_frameStart = newStart;
		return;
	}
	_frameTime = newStart - _frameStart;
	_frameStart = newStart;

	if (_mode == PauseMode || _shortFrame) {
		_frameTime = 0;
	}

	LuaBase::instance()->update(_frameTime, _movieTime);

	if (_currSet && (_mode == NormalMode || _mode == SmushMode)) {
		// call updateTalk() before calling update(), since it may modify costumes state, and
		// the costumes are updated in update().
		for (Common::List<Actor *>::iterator i = _talkingActors.begin(); i != _talkingActors.end(); ++i) {
			Actor *a = *i;
			if (!a->updateTalk(_frameTime)) {
				i = _talkingActors.reverse_erase(i);
			}
		}

		// Update the actors. Do it here so that we are sure to react asap to any change
		// in the actors state caused by lua.
		buildActiveActorsList();
		for (Actor *a : _activeActors) {
			// Note that the actor need not be visible to update chores, for example:
			// when Manny has just brought Meche back he is offscreen several times
			// when he needs to perform certain chores
			a->update(_frameTime);
		}

		_iris->update(_frameTime);

		for (TextObject *t : TextObject::getPool()) {
			t->update();
		}
	}
}

void GrimEngine::updateDisplayScene() {
	_doFlip = true;

	if (_mode == SmushMode) {
		if (g_movie->isPlaying()) {
			_movieTime = g_movie->getMovieTime();
			if (g_movie->isUpdateNeeded()) {
				g_driver->prepareMovieFrame(g_movie->getDstSurface(), g_movie->getDstPalette());
				g_movie->clearUpdateNeeded();
			}
			int frame = g_movie->getFrame();
			if (frame >= 0) {
				if (frame != _prevSmushFrame) {
					_prevSmushFrame = g_movie->getFrame();
					g_driver->drawMovieFrame(g_movie->getX(), g_movie->getY());
					if (_showFps)
						g_driver->drawEmergString(550, 25, _fps, Color(255, 255, 255));
				} else
					_doFlip = false;
			} else
				g_driver->releaseMovieFrame();
		}
		// Draw Primitives
		_iris->draw();

		g_movie->drawMovieSubtitle();

	} else if (_mode == NormalMode || _mode == OverworldMode) {
		updateNormalMode();
	} else if (_mode == DrawMode) {
		updateDrawMode();
	}
}

void GrimEngine::updateNormalMode() {
	if (!_currSet || !_flipEnable)
		return;

	g_driver->clearScreen();

	drawNormalMode();

	_iris->draw();
	drawTextObjects();
}

void GrimEngine::updateDrawMode() {
	_doFlip = false;
	_prevSmushFrame = 0;
	_movieTime = 0;
}

void GrimEngine::drawNormalMode() {
	_prevSmushFrame = 0;
	_movieTime = 0;

	_currSet->drawBackground();

	// Draw underlying scene components
	// Background objects are drawn underneath everything except the background
	// There are a bunch of these, especially in the tube-switcher room
	_currSet->drawBitmaps(ObjectState::OBJSTATE_BACKGROUND);

	// State objects are drawn on top of other things, such as the flag
	// on Manny's message tube
	_currSet->drawBitmaps(ObjectState::OBJSTATE_STATE);

	// Play SMUSH Animations
	// This should occur on top of all underlying scene objects,
	// a good example is the tube switcher room where some state objects
	// need to render underneath the animation or you can't see what's going on
	// This should not occur on top of everything though or Manny gets covered
	// up when he's next to Glottis's service room
	if (g_movie->isPlaying() && _movieSetup == _currSet->getCurrSetup()->_name) {
		_movieTime = g_movie->getMovieTime();
		if (g_movie->isUpdateNeeded()) {
			g_driver->prepareMovieFrame(g_movie->getDstSurface(), g_movie->getDstPalette());
			g_movie->clearUpdateNeeded();
		}
		if (g_movie->getFrame() >= 0)
			g_driver->drawMovieFrame(g_movie->getX(), g_movie->getY());
		else
			g_driver->releaseMovieFrame();
	}

	// Underlay objects must be drawn on top of movies
	// Otherwise the lighthouse door will always be open as the underlay for
	// the closed door will be overdrawn by a movie used as background image.
	_currSet->drawBitmaps(ObjectState::OBJSTATE_UNDERLAY);

	// Draw Primitives
	for (PrimitiveObject *p : PrimitiveObject::getPool()) {
		p->draw();
	}

	for (Overlay *p : Overlay::getPool()) {
		p->draw();
	}

	_currSet->setupCamera();

	g_driver->set3DMode();

	if (_setupChanged) {
		cameraPostChangeHandle(_currSet->getSetup());
		_setupChanged = false;
	}

	// Draw actors
	buildActiveActorsList();
	for (Actor *a : _activeActors) {
		if (a->isVisible())
			a->draw();
	}

	flagRefreshShadowMask(false);

	// Draw overlying scene components
	// The overlay objects should be drawn on top of everything else,
	// including 3D objects such as Manny and the message tube
	_currSet->drawBitmaps(ObjectState::OBJSTATE_OVERLAY);
}

void GrimEngine::doFlip() {
	_frameCounter++;
	// When possible, flip the buffer
	// This makes sure the screen is refreshed on a regular basis
	// The image is properly resized if needed and backend overlays are displayed
	if (!_doFlip || (_mode == PauseMode)) {
		g_driver->flipBuffer(true);
		return;
	}

	if (_showFps && _mode != DrawMode)
		g_driver->drawEmergString(550, 25, _fps, Color(255, 255, 255));

	if (_flipEnable)
		g_driver->flipBuffer();

	if (_showFps && _mode != DrawMode) {
		unsigned int currentTime = g_system->getMillis();
		unsigned int delta = currentTime - _lastFrameTime;
		if (delta > 500) {
			snprintf(_fps, sizeof(_fps), "%7.2f", (double)(_frameCounter * 1000) / (double)delta);
			_frameCounter = 0;
			_lastFrameTime = currentTime;
		}
	}
}

void GrimEngine::mainLoop() {
	_movieTime = 0;
	_frameTime = 0;
	_frameStart = g_system->getMillis();
	_frameCounter = 0;
	_lastFrameTime = 0;
	_prevSmushFrame = 0;
	_refreshShadowMask = false;
	_shortFrame = false;
	bool resetShortFrame = false;
	_changeHardwareState = false;
	_setupChanged = true;

	for (;;) {
		uint32 startTime = g_system->getMillis();
		if (_shortFrame) {
			if (resetShortFrame) {
				_shortFrame = false;
			}
			resetShortFrame = !resetShortFrame;
		}

		if (shouldQuit())
			return;

		if (_savegameLoadRequest) {
			savegameRestore();
		}
		if (_savegameSaveRequest) {
			savegameSave();
		}

		// If the backend destroys the OpenGL context or the user switched to a different
		// renderer, the GFX driver needs to be recreated.
		if (_changeHardwareState) {
			_changeHardwareState = false;

			uint screenWidth = g_driver->getScreenWidth();
			uint screenHeight = g_driver->getScreenHeight();

			EngineMode mode = getMode();

			_savegameFileName = "";
			savegameSave();
			clearPools();

			delete g_driver;
			g_driver = createRenderer(screenWidth, screenHeight);
			savegameRestore();

			if (mode == DrawMode) {
				setMode(GrimEngine::NormalMode);
				updateDisplayScene();
				g_driver->storeDisplay();
				g_driver->dimScreen();
			}
			setMode(mode);
		}

		g_sound->flushTracks();
		if (g_imuse) {
			g_imuse->refreshScripts();
		}

		// Process events
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			// Handle any buttons, keys and joystick operations
			Common::EventType type = event.type;
			if (type == Common::EVENT_KEYDOWN || type == Common::EVENT_KEYUP) {
				if (type == Common::EVENT_KEYDOWN) {
					// Ignore everything but ESC when movies are playing
					// This matches the retail and demo versions of EMI
					// This also allows the PS2 version to skip movies
					if (_mode == SmushMode && g_grim->getGameType() == GType_MONKEY4) {
						if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
							g_movie->stop();
							break;
						}
						continue;
					}

					if (_mode != DrawMode && _mode != SmushMode && ((event.kbd.ascii == 'q') || (event.kbd.ascii == 'x' && (event.kbd.flags & Common::KBD_ALT)))) {
						handleExit();
						break;
					} else if (_mode != DrawMode && (event.kbd.keycode == Common::KEYCODE_PAUSE)) {
						handlePause();
						break;
					} else {
						handleChars(type, event.kbd);
					}
				}

				handleControls(type, event.kbd);

				if (getGameType() != GType_MONKEY4) {
					// Allow lua to react to the event.
					// Without this lua_update switching the entries in the menu is slow because
					// if the button is not kept pressed the KEYUP will arrive just after the KEYDOWN
					// and it will break the lua scripts that checks for the state of the button
					// with GetControlState()
					//
					// This call seems to be only necessary to handle Grim's menu correctly.
					// In EMI it would have the side-effect that luaUpdate() is sometimes called
					// in the same millisecond which causes getPerSecond() to return 0 for
					// any given rate which is not compatible with e.g. actor walking.

					// We do not want the scripts to update while a movie is playing in the PS2-version.
					if (!(getGamePlatform() == Common::kPlatformPS2 && _mode == SmushMode)) {
						luaUpdate();
					}
				}
			}
			if (type == Common::EVENT_JOYAXIS_MOTION)
				handleJoyAxis(event.joystick.axis, event.joystick.position);
			if (type == Common::EVENT_JOYBUTTON_DOWN || type == Common::EVENT_JOYBUTTON_UP)
				handleJoyButton(type, event.joystick.button);

			if (type == Common::EVENT_LBUTTONUP) {
				_cursorX = event.mouse.x;
				_cursorY = event.mouse.y;
				Common::KeyState k;
				k.keycode = (Common::KeyCode)KEYCODE_MOUSE_B1;
				handleControls(Common::EVENT_KEYUP, k);
			}
			if (type == Common::EVENT_LBUTTONDOWN) {
				_cursorX = event.mouse.x;
				_cursorY = event.mouse.y;
				Common::KeyState k;
				k.keycode = (Common::KeyCode)KEYCODE_MOUSE_B1;
				handleControls(Common::EVENT_KEYDOWN, k);
			}
			if (type == Common::EVENT_MOUSEMOVE) {
				_cursorX = event.mouse.x;
				_cursorY = event.mouse.y;
				handleMouseAxis(0, _cursorX);
				handleMouseAxis(1, _cursorY);
			}
			if (type == Common::EVENT_SCREEN_CHANGED) {
				handleUserPaint();
			}
		}

		if (_mode != PauseMode) {
			// Draw the display scene before doing the luaUpdate.
			// This give a large performance boost as OpenGL stores commands
			// in a queue on the gpu to be rendered later. When doFlip is
			// called the cpu must wait for the gpu to finish its queue.
			// Now, it will queue all the OpenGL commands and draw them on the
			// GPU while the CPU is busy updating the game world.
			updateDisplayScene();
		}

		doFlip();

		// We do not want the scripts to update while a movie is playing in the PS2-version.
		if (!(getGamePlatform() == Common::kPlatformPS2 && _mode == SmushMode)) {
			luaUpdate();
		}

		if (g_imuseState != -1) {
			g_sound->setMusicState(g_imuseState);
			g_imuseState = -1;
		}

		uint32 endTime = g_system->getMillis();
		if (startTime > endTime)
			continue;
		uint32 diffTime = endTime - startTime;
		if (diffTime < _speedLimitMs) {
			uint32 delayTime = _speedLimitMs - diffTime;
			g_system->delayMillis(delayTime);
		}
#if defined(__EMSCRIPTEN__)
		else {
			// If SDL_HINT_EMSCRIPTEN_ASYNCIFY is enabled, SDL pauses the application and gives
			// back control to the browser automatically by calling emscripten_sleep via SDL_Delay.
			// Without this the page would completely lock up.
			g_system->delayMillis(0);
		}
#endif
	}
}

void GrimEngine::changeHardwareState() {
	_changeHardwareState = true;
}

void GrimEngine::saveGame(const Common::String &file) {
	_savegameFileName = file;
	_savegameSaveRequest = true;
}

void GrimEngine::loadGame(const Common::String &file) {
	_savegameFileName = file;
	_savegameLoadRequest = true;
}

void GrimEngine::savegameRestore() {
	debug(2, "GrimEngine::savegameRestore() started.");
	_savegameLoadRequest = false;
	Common::String filename;
	if (_savegameFileName.size() == 0) {
		filename = "grim.sav";
	} else {
		filename = _savegameFileName;
	}
	_savedState = SaveGame::openForLoading(filename);
	if (!_savedState || !_savedState->isCompatible())
		return;
	if (g_imuse) {
		g_imuse->stopAllSounds();
		g_imuse->resetState();
	}
	g_movie->stop();
	if (g_imuse)
		g_imuse->pause(true);
	g_movie->pause(true);
	if (g_registry)
		g_registry->save();

	_selectedActor = nullptr;
	delete _currSet;
	_currSet = nullptr;

	Bitmap::getPool().restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "Bitmaps restored successfully.");

	BitmapFont::getPool().restoreObjects(_savedState);
	if (_savedState->saveMinorVersion() >= 28) {
		FontTTF::getPool().restoreObjects(_savedState);
	}
	Debug::debug(Debug::Engine, "Fonts restored successfully.");

	ObjectState::getPool().restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "ObjectStates restored successfully.");

	Set::getPool().restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "Sets restored successfully.");

	TextObject::getPool().restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "TextObjects restored successfully.");

	PrimitiveObject::getPool().restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "PrimitiveObjects restored successfully.");

	Actor::getPool().restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "Actors restored successfully.");

	if (getGameType() == GType_MONKEY4) {
		PoolSound::getPool().restoreObjects(_savedState);
		Debug::debug(Debug::Engine, "Pool sounds saved successfully.");

		Layer::getPool().restoreObjects(_savedState);
		Debug::debug(Debug::Engine, "Layers restored successfully.");
	}

	restoreGRIM();
	Debug::debug(Debug::Engine, "Engine restored successfully.");

	g_driver->restoreState(_savedState);
	Debug::debug(Debug::Engine, "Renderer restored successfully.");

	g_sound->restoreState(_savedState);
	Debug::debug(Debug::Engine, "iMuse restored successfully.");

	g_movie->restoreState(_savedState);
	Debug::debug(Debug::Engine, "Movie restored successfully.");

	_iris->restoreState(_savedState);
	Debug::debug(Debug::Engine, "Iris restored successfully.");

	lua_Restore(_savedState);
	Debug::debug(Debug::Engine, "Lua restored successfully.");

	if (getGameType() == GType_GRIM && !(getGameFlags() & ADGF_DEMO) &&
		_savedState->saveMajorVersion() == 22 &&
		_savedState->saveMinorVersion() >= 7 &&
		_savedState->saveMinorVersion() <= 28) {
		// Since ResidualVM 0.2.0, a ResidualVM/ScummVM specific patch was provided broken.
		// We patch here the code to fix all saves containing this invalid code.
		// cf. bug #13139 and #14987
		lua_PatchGrimSave();
	}

	delete _savedState;

	_justSaveLoaded = true;

	//Re-read the values, since we may have been in some state that changed them when loading the savegame,
	//e.g. running a cutscene, which sets the sfx volume to 0.
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	LuaBase::instance()->postRestoreHandle();
	if (g_imuse)
		g_imuse->pause(false);
	g_movie->pause(false);

	debug(2, "GrimEngine::savegameRestore() finished.");

	_shortFrame = true;
	clearEventQueue();
	invalidateActiveActorsList();
	buildActiveActorsList();

	_currSet->setupCamera();
	g_driver->set3DMode();
}

void GrimEngine::restoreGRIM() {
	_savedState->beginSection('GRIM');

	_mode = (EngineMode)_savedState->readLEUint32();
	_previousMode = (EngineMode)_savedState->readLEUint32();

	// Actor stuff
	int32 id = _savedState->readLESint32();
	if (id != 0) {
		_selectedActor = Actor::getPool().getObject(id);
	}

	//TextObject stuff
	_sayLineDefaults.setFGColor(_savedState->readColor());
	_sayLineDefaults.setFont(Font::load(_savedState));
	_sayLineDefaults.setHeight(_savedState->readLESint32());
	_sayLineDefaults.setJustify(_savedState->readLESint32());
	_sayLineDefaults.setWidth(_savedState->readLESint32());
	_sayLineDefaults.setX(_savedState->readLESint32());
	_sayLineDefaults.setY(_savedState->readLESint32());
	_sayLineDefaults.setDuration(_savedState->readLESint32());
	if (_savedState->saveMinorVersion() > 5) {
		_movieSubtitle = TextObject::getPool().getObject(_savedState->readLESint32());
	}

	// Set stuff
	_currSet = Set::getPool().getObject(_savedState->readLESint32());
	if (_savedState->saveMinorVersion() > 4) {
		_movieSetup = _savedState->readString();
	} else {
		_movieSetup = _currSet->getCurrSetup()->_name;
	}

	_savedState->endSection();
}

void GrimEngine::storeSaveGameMetadata(SaveGame *state) {
	if (!g_grim->isRemastered()) {
		return;
	}
	state->beginSection('META');
	state->writeString(_saveMeta1);
	state->writeLEUint32(_saveMeta2);
	state->writeString(_saveMeta3);
	state->endSection();
}

void GrimEngine::storeSaveGameImage(SaveGame *state) {
	const Graphics::PixelFormat image_format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	int width = 250, height = 188;
	Bitmap *screenshot;

	debug(2, "GrimEngine::StoreSaveGameImage() started.");

	screenshot = g_driver->getScreenshot(width, height, true);
	state->beginSection('SIMG');
	if (screenshot) {
		int size = screenshot->getWidth() * screenshot->getHeight();
		screenshot->setActiveImage(0);
		screenshot->getBitmapData()->convertToColorFormat(image_format);
		const uint16 *data = (const uint16 *)screenshot->getData().getPixels();
		for (int l = 0; l < size; l++) {
			state->writeLEUint16(data[l]);
		}
	} else {
		error("Unable to store screenshot");
	}
	state->endSection();
	delete screenshot;
	debug(2, "GrimEngine::StoreSaveGameImage() finished.");
}

void GrimEngine::savegameSave() {
	debug(2, "GrimEngine::savegameSave() started.");
	_savegameSaveRequest = false;
	Common::String filename;
	if (_savegameFileName.size() == 0) {
		filename = "grim.sav";
	} else {
		filename = _savegameFileName;
	}
	if (getGameType() == GType_MONKEY4 && filename.contains('/')) {
		filename = Common::lastPathComponent(filename, '/');
	}
	_savedState = SaveGame::openForSaving(filename);
	if (!_savedState) {
		//TODO: Translate this!
		GUI::displayErrorDialog(_("Error: the game could not be saved."));
		return;
	}
	storeSaveGameMetadata(_savedState);

	storeSaveGameImage(_savedState);

	if (g_imuse)
		g_imuse->pause(true);
	g_movie->pause(true);

	savegameCallback();

	Bitmap::getPool().saveObjects(_savedState);
	Debug::debug(Debug::Engine, "Bitmaps saved successfully.");

	BitmapFont::getPool().saveObjects(_savedState);
	FontTTF::getPool().saveObjects(_savedState);
	Debug::debug(Debug::Engine, "Fonts saved successfully.");

	ObjectState::getPool().saveObjects(_savedState);
	Debug::debug(Debug::Engine, "ObjectStates saved successfully.");

	Set::getPool().saveObjects(_savedState);
	Debug::debug(Debug::Engine, "Sets saved successfully.");

	TextObject::getPool().saveObjects(_savedState);
	Debug::debug(Debug::Engine, "TextObjects saved successfully.");

	PrimitiveObject::getPool().saveObjects(_savedState);
	Debug::debug(Debug::Engine, "PrimitiveObjects saved successfully.");

	Actor::getPool().saveObjects(_savedState);
	Debug::debug(Debug::Engine, "Actors saved successfully.");

	if (getGameType() == GType_MONKEY4) {
		PoolSound::getPool().saveObjects(_savedState);
		Debug::debug(Debug::Engine, "Pool sounds saved successfully.");

		Layer::getPool().saveObjects(_savedState);
		Debug::debug(Debug::Engine, "Layers saved successfully.");
	}

	saveGRIM();
	Debug::debug(Debug::Engine, "Engine saved successfully.");

	g_driver->saveState(_savedState);
	Debug::debug(Debug::Engine, "Renderer saved successfully.");

	g_sound->saveState(_savedState);
	Debug::debug(Debug::Engine, "iMuse saved successfully.");

	g_movie->saveState(_savedState);
	Debug::debug(Debug::Engine, "Movie saved successfully.");

	_iris->saveState(_savedState);
	Debug::debug(Debug::Engine, "Iris saved successfully.");

	lua_Save(_savedState);

	delete _savedState;

	if (g_imuse)
		g_imuse->pause(false);
	g_movie->pause(false);
	debug(2, "GrimEngine::savegameSave() finished.");

	_shortFrame = true;
	clearEventQueue();
}

void GrimEngine::saveGRIM() {
	_savedState->beginSection('GRIM');

	_savedState->writeLEUint32((uint32)_mode);
	_savedState->writeLEUint32((uint32)_previousMode);

	//Actor stuff
	if (_selectedActor) {
		_savedState->writeLESint32(_selectedActor->getId());
	} else {
		_savedState->writeLESint32(0);
	}

	//TextObject stuff
	_savedState->writeColor(_sayLineDefaults.getFGColor());
	Font::save(_sayLineDefaults.getFont(), _savedState);
	_savedState->writeLESint32(_sayLineDefaults.getHeight());
	_savedState->writeLESint32(_sayLineDefaults.getJustify());
	_savedState->writeLESint32(_sayLineDefaults.getWidth());
	_savedState->writeLESint32(_sayLineDefaults.getX());
	_savedState->writeLESint32(_sayLineDefaults.getY());
	_savedState->writeLESint32(_sayLineDefaults.getDuration());
	_savedState->writeLESint32(_movieSubtitle ? _movieSubtitle->getId() : 0);

	//Set stuff
	_savedState->writeLESint32(_currSet->getId());
	_savedState->writeString(_movieSetup);

	_savedState->endSection();
}

Set *GrimEngine::findSet(const Common::String &name) {
	// Find scene object
	for (Set *s : Set::getPool()) {
		if (s->getName() == name)
			return s;
	}
	return nullptr;
}

void GrimEngine::setSetLock(const char *name, bool lockStatus) {
	Set *scene = findSet(name);

	if (!scene) {
		Debug::warning(Debug::Engine, "Set object '%s' not found in list", name);
		return;
	}
	// Change the locking status
	scene->_locked = lockStatus;
}

Set *GrimEngine::loadSet(const Common::String &name) {
	Set *s = findSet(name);

	if (!s) {
		Common::String filename(name);
		// EMI-scripts refer to their .setb files as .set
		if (g_grim->getGameType() == GType_MONKEY4) {
			filename += "b";
		}
		Common::SeekableReadStream *stream;
		stream = g_resourceloader->openNewStreamFile(filename.c_str());
		if (!stream)
			error("Could not find scene file %s", name.c_str());

		s = new Set(name, stream);
		delete stream;
	}

	return s;
}

void GrimEngine::setSet(const char *name) {
	setSet(loadSet(name));
}

void GrimEngine::setSet(Set *scene) {
	if (scene == _currSet)
		return;

	if (getGameType() == GType_MONKEY4) {
		for (PoolSound *s : PoolSound::getPool()) {
			s->stop();
		}
	}
	// Stop the actors. This fixes bug #289 (https://github.com/residualvm/residualvm/issues/289)
	// and it makes sense too, since when changing set the directions
	// and coords change too.
	for (Actor *a : Actor::getPool()) {
		a->stopWalking();
	}

	Set *lastSet = _currSet;
	_currSet = scene;
	_currSet->setSoundParameters(20, 127);
	// should delete the old scene after setting the new one
	if (lastSet && !lastSet->_locked) {
		delete lastSet;
	}
	_shortFrame = true;
	_setupChanged = true;
	invalidateActiveActorsList();
}

void GrimEngine::makeCurrentSetup(int num) {
	int prevSetup = g_grim->getCurrSet()->getSetup();
	if (prevSetup != num) {
		getCurrSet()->setSetup(num);
		getCurrSet()->setSoundParameters(20, 127);
		cameraChangeHandle(prevSetup, num);
		// here should be set sound position

		_setupChanged = true;
	}
}

void GrimEngine::setTextSpeed(int speed) {
	if (speed < 1)
		_textSpeed = 1;
	if (speed > 10)
		_textSpeed = 10;
	_textSpeed = speed;
}

float GrimEngine::getControlAxis(int num) {
	int idx = num - KEYCODE_AXIS_JOY1_X;
	if (idx >= 0 && idx < NUM_JOY_AXES) {
		return _joyAxisPosition[idx];
	}
	return 0;
}

bool GrimEngine::getControlState(int num) {
	return _controlsState[num];
}

float GrimEngine::getPerSecond(float rate) const {
	return rate * _frameTime / 1000;
}

void GrimEngine::invalidateActiveActorsList() {
	_buildActiveActorsList = true;
}

void GrimEngine::immediatelyRemoveActor(Actor *actor) {
	_activeActors.remove(actor);
	_talkingActors.remove(actor);
}

void GrimEngine::buildActiveActorsList() {
	if (!_buildActiveActorsList) {
		return;
	}

	_activeActors.clear();
	for (Actor *a : Actor::getPool()) {
		if (((_mode == NormalMode || _mode == DrawMode) && a->isDrawableInSet(_currSet->getName())) || a->isInOverworld()) {
			_activeActors.push_back(a);
		}
	}
	_buildActiveActorsList = false;
}

void GrimEngine::addTalkingActor(Actor *a) {
	_talkingActors.push_back(a);
}

bool GrimEngine::areActorsTalking() const {
	//This takes into account that there may be actors which are still talking, but in the background.
	bool talking = false;
	for (Actor *a : _talkingActors) {
		if (a->isTalkingForeground()) {
			talking = true;
			break;
		}
	}
	return talking;
}

void GrimEngine::setMovieSubtitle(TextObject *to) {
	if (_movieSubtitle != to) {
		delete _movieSubtitle;
		_movieSubtitle = to;
	}
}

void GrimEngine::drawMovieSubtitle() {
	if (_movieSubtitle)
		_movieSubtitle->draw();
}

void GrimEngine::setMovieSetup() {
	_movieSetup = _currSet->getCurrSetup()->_name;
}

void GrimEngine::setMode(EngineMode mode) {
	_mode = mode;
	invalidateActiveActorsList();
}

void GrimEngine::clearEventQueue() {
	g_system->getEventManager()->purgeKeyboardEvents();
	g_system->getEventManager()->purgeMouseEvents();

	for (int i = 0; i < KEYCODE_EXTRA_LAST; ++i) {
		_controlsState[i] = false;
	}
}

bool GrimEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime);
}

void GrimEngine::pauseEngineIntern(bool pause) {
	if (g_imuse)
		g_imuse->pause(pause);
	if (g_movie)
		g_movie->pause(pause);

	if (pause) {
		_pauseStartTime = _system->getMillis();
	} else {
		_frameStart += _system->getMillis() - _pauseStartTime;
	}
}

void GrimEngine::debugLua(const Common::String &str) {
	lua_dostring(str.c_str());
}

Common::String GrimEngine::getLanguagePrefix() const {
	switch (getLanguage()) {
		case 0:
			return Common::String("en");
		case 1:
			return Common::String("de");
		case 2:
			return Common::String("es");
		case 3:
			return Common::String("fr");
		case 4:
			return Common::String("it");
		case 5:
			return Common::String("pt");
		default:
			error("Unknown language id %d", getLanguage());
	}
}

bool GrimEngine::isConceptEnabled(uint32 number) const {
	assert (number < kNumConcepts);
	return _conceptEnabled[number];
}

void GrimEngine::enableConcept(uint32 number) {
	assert (number < kNumConcepts);
	_conceptEnabled[number] = true;
}

bool GrimEngine::isCutsceneEnabled(uint32 number) const {
	assert (number < kNumCutscenes);
	return _cutsceneEnabled[number];
}

void GrimEngine::enableCutscene(uint32 number) {
	assert (number < kNumCutscenes);
	_cutsceneEnabled[number] = true;
}

Graphics::RendererType GrimEngine::getRendererType() {
	return g_driver->type;
}

void GrimEngine::setSaveMetaData(const char *meta1, int meta2, const char *meta3) {
	_saveMeta1 = meta1;
	_saveMeta2 = meta2;
	_saveMeta3 = meta3;
}

} // end of namespace Grim
