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

#include "twine/twine.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/text-to-speech.h"
#include "common/textconsole.h"
#include "engines/metaengine.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/debugger/console.h"
#include "twine/debugger/debug_state.h"
#include "twine/detection.h"
#include "twine/holomap_v1.h"
#include "twine/holomap_v2.h"
#include "twine/input.h"
#include "twine/menu/interface.h"
#include "twine/menu/menu.h"
#include "twine/menu/menuoptions.h"
#include "twine/movies.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/scene/buggy.h"
#include "twine/scene/collision.h"
#include "twine/scene/dart.h"
#include "twine/scene/extra.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/scene/rain.h"
#include "twine/scene/scene.h"
#include "twine/scene/wagon.h"
#include "twine/script/script_life_v1.h"
#include "twine/script/script_life_v2.h"
#include "twine/script/script_move_v1.h"
#include "twine/script/script_move_v2.h"
#include "twine/shared.h"
#include "twine/slideshow.h"
#include "twine/text.h"
#ifdef USE_IMGUI
#include "twine/debugger/debugtools.h"
#endif
namespace TwinE {

ScopedCursor::ScopedCursor(TwinEEngine *engine) : _engine(engine) {
	_engine->pushMouseCursorVisible();
}

ScopedCursor::~ScopedCursor() {
	_engine->popMouseCursorVisible();
	_engine->_input->resetLastHoveredMousePosition();
}

FrameMarker::FrameMarker(TwinEEngine *engine, uint32 fps) : _engine(engine), _fps(fps) {
	_start = g_system->getMillis();
	++_engine->_frameCounter;
}

FrameMarker::~FrameMarker() {
	_engine->_frontVideoBuffer.update();
	if (_fps == 0) {
		return;
	}
	const uint32 end = g_system->getMillis();
	const uint32 frameTime = end - _start;
	const uint32 maxDelay = 1000 / _fps;
	const int32 waitMillis = (int32)maxDelay - (int32)frameTime;
	_engine->_debugState->addFrameData(frameTime, waitMillis, maxDelay);
	if (waitMillis < 0) {
		debug(5, "Frame took longer than the max allowed time: %u (max is %u)", frameTime, maxDelay);
		return;
	}
	g_system->delayMillis((uint)waitMillis);
}

TwineScreen::TwineScreen(TwinEEngine *engine) : _engine(engine) {
}

void TwineScreen::update() {
	if (_lastFrame == _engine->_frameCounter) {
		return;
	}
	_lastFrame = _engine->_frameCounter;

	if (_engine->_redraw->_flagMCGA) {
		markAllDirty();
		Graphics::ManagedSurface zoomWorkVideoBuffer;
		zoomWorkVideoBuffer.copyFrom(_engine->_workVideoBuffer);
		const int maxW = zoomWorkVideoBuffer.w;
		const int maxH = zoomWorkVideoBuffer.h;
		const int left = CLIP<int>(_engine->_redraw->_sceneryViewX - maxW / 4, 0, maxW / 2);
		const int top = CLIP<int>(_engine->_redraw->_sceneryViewY - maxH / 4, 0, maxH / 2);
		const Common::Rect srcRect(left, top, left + maxW / 2, top + maxH / 2);
		const Common::Rect &destRect = zoomWorkVideoBuffer.getBounds();
		zoomWorkVideoBuffer.blitFrom(*this, srcRect, destRect);
		blitFrom(zoomWorkVideoBuffer);
		// TODO: we need to redraw everything because we just modified the screen buffer itself
		_engine->_redraw->_firstTime = true;
	}
	Super::update();
}

int32 boundRuleThree(int32 val1, int32 val2, int32 nbstep, int32 step) { // BoundRegleTrois
	if (step <= 0) {
		return val1;
	}

	if (step >= nbstep) {
		return val2;
	}

	return val1 + (((val2 - val1) * step) / nbstep);
}

int32 ruleThree32(int32 val1, int32 val2, int32 nbstep, int32 step) {  // RegleTrois32
	if (nbstep < 0) {
		return val2;
	}
	return (((val2 - val1) * step) / nbstep) + val1;
}

TwinEEngine::TwinEEngine(OSystem *system, Common::Language language, uint32 flags, Common::Platform platform, TwineGameType gameType)
	: Engine(system), _gameType(gameType), _gameLang(language), _frontVideoBuffer(this), _gameFlags(flags), _platform(platform), _rnd("twine") {
	// Add default file directories
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "fla");
	SearchMan.addSubDirectoryMatching(gameDataDir, "vox");
	if (isLBA2()) {
		SearchMan.addSubDirectoryMatching(gameDataDir, "video");
		SearchMan.addSubDirectoryMatching(gameDataDir, "music");
	}

	if (isLba1Classic()) {
		SearchMan.addSubDirectoryMatching(gameDataDir, "common");
		SearchMan.addSubDirectoryMatching(gameDataDir, "commonclassic");
		SearchMan.addSubDirectoryMatching(gameDataDir, "common/fla");
		SearchMan.addSubDirectoryMatching(gameDataDir, "common/vox");
		SearchMan.addSubDirectoryMatching(gameDataDir, "common/music");
		SearchMan.addSubDirectoryMatching(gameDataDir, "common/midi");
		SearchMan.addSubDirectoryMatching(gameDataDir, "commonclassic/images");
		SearchMan.addSubDirectoryMatching(gameDataDir, "commonclassic/voices/de_voice");
		SearchMan.addSubDirectoryMatching(gameDataDir, "commonclassic/voices/en_voice");
		SearchMan.addSubDirectoryMatching(gameDataDir, "commonclassic/voices/fr_voice");
	}

	if (isDotEmuEnhanced()) {
		SearchMan.addSubDirectoryMatching(gameDataDir, "resources/lba_files/hqr");
		SearchMan.addSubDirectoryMatching(gameDataDir, "resources/lba_files/fla");
		SearchMan.addSubDirectoryMatching(gameDataDir, "resources/lba_files/vox/de_voice");
		SearchMan.addSubDirectoryMatching(gameDataDir, "resources/lba_files/vox/en_voice");
		SearchMan.addSubDirectoryMatching(gameDataDir, "resources/lba_files/vox/fr_voice");
		SearchMan.addSubDirectoryMatching(gameDataDir, "resources");
#ifdef USE_MAD
		SearchMan.addSubDirectoryMatching(gameDataDir, "resources/lba_files/music");
		SearchMan.addSubDirectoryMatching(gameDataDir, "resources/lba_files/midi_mp3");
#endif
#ifdef USE_VORBIS
		SearchMan.addSubDirectoryMatching(gameDataDir, "resources/lba_files/music/ogg");
		SearchMan.addSubDirectoryMatching(gameDataDir, "resources/lba_files/midi_mp3/ogg");
#endif
	}

	if (isLBA2()) {
		LBAAngles::lba2();
	} else {
		LBAAngles::lba1();
	}

	_actor = new Actor(this);
	_animations = new Animations(this);
	_collision = new Collision(this);
	_extra = new Extra(this);
	_gameState = new GameState(this);
	_grid = new Grid(this);
	_movements = new Movements(this);
	_interface = new Interface(this);
	_menu = new Menu(this);
	_movie = new Movies(this);
	_menuOptions = new MenuOptions(this);
	_music = new Music(this);
	_redraw = new Redraw(this);
	_renderer = new Renderer(this);
	_resources = new Resources(this);
	_scene = new Scene(this);
	_screens = new Screens(this);
	if (isLBA1()) {
		_scriptLife = new ScriptLifeV1(this);
		_scriptMove = new ScriptMoveV1(this);
		_holomap = new HolomapV1(this);
	} else {
		_scriptLife = new ScriptLifeV2(this);
		_scriptMove = new ScriptMoveV2(this);
		_buggy = new Buggy(this);
		_dart = new Dart(this);
		_rain = new Rain(this);
		_wagon = new Wagon(this);
		_holomap = new HolomapV2(this);
	}
	_sound = new Sound(this);
	_text = new Text(this);
	_input = new Input(this);
	_debugState = new DebugState(this);
	setDebugger(new TwinEConsole(this));
}

TwinEEngine::~TwinEEngine() {
	ConfMan.flushToDisk();
	delete _actor;
	delete _animations;
	delete _collision;
	delete _extra;
	delete _gameState;
	delete _grid;
	delete _movements;
	delete _interface;
	delete _menu;
	delete _movie;
	delete _menuOptions;
	delete _music;
	delete _redraw;
	delete _renderer;
	delete _resources;
	delete _scene;
	delete _screens;
	delete _scriptLife;
	delete _scriptMove;
	delete _holomap;
	delete _sound;
	delete _text;
	delete _input;
	delete _debugState;
}

void TwinEEngine::pushMouseCursorVisible() {
	++_mouseCursorState;
	if (!_cfgfile.Mouse) {
		return;
	}
	if (_mouseCursorState == 1) {
		CursorMan.showMouse(_cfgfile.Mouse);
	}
}

void TwinEEngine::popMouseCursorVisible() {
	--_mouseCursorState;
	if (_mouseCursorState == 0) {
		CursorMan.showMouse(false);
	}
}

Common::Error TwinEEngine::run() {
#ifdef USE_IMGUI
	ImGuiCallbacks callbacks;
	bool drawImGui = debugChannelSet(-1, kDebugImGui);
	callbacks.init = TwinE::onImGuiInit;
	callbacks.render = drawImGui ? TwinE::onImGuiRender : nullptr;
	callbacks.cleanup = TwinE::onImGuiCleanup;
	_system->setImGuiCallbacks(callbacks);
#endif

	debug("Based on TwinEngine v0.2.2");
	debug("(c) 2002 The TwinEngine team.");
	debug("(c) 2020-2022 The ScummVM team.");
	debug("Refer to the credits for further details.");
	debug("The original Little Big Adventure game is:");
	debug("(c) 1994 by Adeline Software International, All Rights Reserved.");

	ConfMan.registerDefault("usehighres", false);

	const Common::String &gameTarget = ConfMan.getActiveDomainName();
	AchMan.setActiveDomain(getMetaEngine()->getAchievementsInfo(gameTarget));

	syncSoundSettings();
	int32 w = originalWidth();
	int32 h = originalHeight();
	const bool highRes = ConfMan.getBool("usehighres");
	if (highRes) {
		w = 1024;
		h = 768;
	}

	initGraphics(w, h);
	allocVideoMemory(w, h);
	if (isLBASlideShow()) {
		playSlideShow(this);
		return Common::kNoError;
	}
	_renderer->init(w, h);
	_grid->init(w, h);
	initAll();
	introduction();
	_sound->stopSamples();
	saveFrontBuffer();

	_menu->init();
	_holomap->loadLocations();

	if (ConfMan.hasKey("save_slot")) {
		const int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999) {
			Common::Error state = loadGameState(saveSlot);
			if (state.getCode() != Common::kNoError) {
				return state;
			}
		}
	}
	if (ConfMan.hasKey("boot_param")) {
		const int sceneIndex = ConfMan.getInt("boot_param");
		if (sceneIndex < 0 || sceneIndex >= LBA1SceneId::SceneIdMax) {
			warning("Scene index out of bounds");
		} else {
			debug("Boot parameter: %i", sceneIndex);
			_gameState->initEngineVars();
			_text->normalWinDial();
			_text->_flagMessageShade = true;
			_text->_renderTextTriangle = false;
			_scene->_newCube = sceneIndex;
			_scene->_heroPositionType = ScenePositionType::kScene;
			_state = EngineState::GameLoop;
		}
	}

	bool quitGame = false;
	while (!quitGame && !shouldQuit()) {
		readKeys();
		switch (_state) {
		case EngineState::QuitGame: {
			quitGame = true;
			break;
		}
		case EngineState::LoadedGame:
			debug("Loaded game");
			if (_scene->_newHeroPos.x == -1) {
				_scene->_heroPositionType = ScenePositionType::kNoPosition;
			}
			_text->_renderTextTriangle = false;
			_text->normalWinDial();
			_text->_flagMessageShade = true;
			_state = EngineState::GameLoop;
			break;
		case EngineState::GameLoop:
			if (mainLoop()) {
				_menuOptions->showCredits();
				_menuOptions->showEndSequence();
			}
			_state = EngineState::Menu;
			break;
		case EngineState::Menu:
#if 0
			// this will enter the game and execute the commands in the file "debug"
			_gameState->initEngineVars();
			_text->textClipSmall();
			_text->_drawTextBoxBackground = true;
			_text->_renderTextTriangle = false;
			if (!((TwinEConsole*)getDebugger())->exec("debug")) {
				debug("Failed to execute debug file before entering the scene");
			}
			gameEngineLoop();
#else
			_state = _menu->run();
#endif
			break;
		}
#ifdef USE_IMGUI
		// For performance reasons, disable the renderer callback if the ImGui debug flag isn't set
		if (debugChannelSet(-1, kDebugImGui) != drawImGui) {
			drawImGui = !drawImGui;
			callbacks.render = drawImGui ? TwinE::onImGuiRender : nullptr;
			_system->setImGuiCallbacks(callbacks);
		}
#endif
	}

	ConfMan.setBool("combatauto", _actor->_combatAuto);
	ConfMan.setInt("shadow", _cfgfile.ShadowMode);
	ConfMan.setBool("scezoom", _cfgfile.SceZoom);
	ConfMan.setInt("polygondetails", _cfgfile.PolygonDetails);

	_sound->stopSamples();
	_music->stopMusic();
	return Common::kNoError;
}

bool TwinEEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case EngineFeature::kSupportsReturnToLauncher:
	case EngineFeature::kSupportsLoadingDuringRuntime:
	case EngineFeature::kSupportsSavingDuringRuntime:
	case EngineFeature::kSupportsChangingOptionsDuringRuntime:
		return true;
	default:
		break;
	}
	return false;
}

SaveStateList TwinEEngine::getSaveSlots() const {
	return getMetaEngine()->listSaves(_targetName.c_str());
}

void TwinEEngine::wipeSaveSlot(int slot) {
	Common::SaveFileManager *saveFileMan = getSaveFileManager();
	const Common::String &saveFile = getSaveStateName(slot);
	saveFileMan->removeSavefile(saveFile);
}

bool TwinEEngine::canSaveGameStateCurrently(Common::U32String *msg) { return _scene->isGameRunning(); }

Common::Error TwinEEngine::loadGameStream(Common::SeekableReadStream *stream) {
	debug("load game stream");
	if (!_gameState->loadGame(stream)) {
		return Common::Error(Common::kReadingFailed);
	}
	_state = EngineState::LoadedGame;
	return Common::Error(Common::kNoError);
}

Common::Error TwinEEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (!_gameState->saveGame(stream)) {
		return Common::Error(Common::kWritingFailed);
	}
	return Common::Error(Common::kNoError);
}

void TwinEEngine::autoSave() {
	debug("Autosave %s", _gameState->_sceneName);
	saveGameState(getAutosaveSlot(), _gameState->_sceneName, true);
}

void TwinEEngine::allocVideoMemory(int32 w, int32 h) {
	const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();

	// original resolution of the game
	_imageBuffer.create(originalWidth(), originalHeight(), format);

	_workVideoBuffer.create(w, h, format);
	_frontVideoBuffer.create(w, h, format);
}

static int getLanguageTypeIndex(const char *languageName) {
	char buffer[256];
	Common::strlcpy(buffer, languageName, sizeof(buffer));

	char *ptr = strchr(buffer, ' ');
	if (ptr != nullptr) {
		*ptr = '\0';
	}

	const int32 length = ARRAYSIZE(ListLanguage);
	for (int32 i = 0; i < length; i++) {
		if (!strcmp(ListLanguage[i].name, buffer)) {
			return i;
		}
	}

	debug("Failed to detect language %s - falling back to english", languageName);
	// select english for the fan translations
	return 0; // English
}

#define ConfGetOrDefault(key, defaultVal) (ConfMan.hasKey(key) ? ConfMan.get(key) : Common::String(defaultVal))
#define ConfGetIntOrDefault(key, defaultVal) (ConfMan.hasKey(key) ? atoi(ConfMan.get(key).c_str()) : (defaultVal))
#define ConfGetBoolOrDefault(key, defaultVal) (ConfMan.hasKey(key) ? ConfMan.get(key) == "true" || atoi(ConfMan.get(key).c_str()) == 1 : (defaultVal))

void TwinEEngine::initConfigurations() {
	// TODO: use existing entries for some of the settings - like volume and so on.
	ConfMan.registerDefault("wallcollision", false);

	const char *lng = Common::getLanguageDescription(_gameLang);
	_cfgfile._languageId = getLanguageTypeIndex(lng);
	ConfMan.registerDefault("audio_language", ListLanguage[_cfgfile._languageId].voice);

	_cfgfile.FlagDisplayText = ConfGetBoolOrDefault("displaytext", true); // TODO: use subtitles
	const Common::String midiType = ConfGetOrDefault("miditype", "auto");
	if (midiType == "None") {
		_cfgfile.MidiType = MIDIFILE_NONE;
	} else {
		Common::File midiHqr;
		if (midiHqr.exists(Resources::HQR_MIDI_MI_WIN_FILE)) {
			_cfgfile.MidiType = MIDIFILE_WIN;
			debug("Use %s for midi", Resources::HQR_MIDI_MI_WIN_FILE);
		} else if (midiHqr.exists(Resources::HQR_MIDI_MI_DOS_FILE)) {
			_cfgfile.MidiType = MIDIFILE_DOS;
			debug("Use %s for midi", Resources::HQR_MIDI_MI_DOS_FILE);
		} else {
			_cfgfile.MidiType = MIDIFILE_NONE;
			debug("Could not find midi hqr file");
		}
	}

	if (_gameFlags & TwineFeatureFlags::TF_VERSION_EUROPE) {
		_cfgfile.Version = EUROPE_VERSION;
	} else if (_gameFlags & TwineFeatureFlags::TF_VERSION_USA) {
		_cfgfile.Version = USA_VERSION;
	} else if (_gameFlags & TwineFeatureFlags::TF_VERSION_CUSTOM) {
		_cfgfile.Version = MODIFICATION_VERSION;
	}

	if (_gameFlags & TwineFeatureFlags::TF_USE_GIF) {
		_cfgfile.Movie = CONF_MOVIE_FLAGIF;
	}

	_cfgfile.Sound = ConfGetBoolOrDefault("sound", true);
	_cfgfile.Fps = ConfGetIntOrDefault("fps", DEFAULT_FRAMES_PER_SECOND);
	_cfgfile.Mouse = ConfGetBoolOrDefault("mouse", true);

	_cfgfile.UseAutoSaving = ConfGetBoolOrDefault("useautosaving", false);
	_cfgfile.WallCollision = ConfGetBoolOrDefault("wallcollision", false);

	_actor->_combatAuto = ConfGetBoolOrDefault("combatauto", true);
	_cfgfile.ShadowMode = ConfGetIntOrDefault("shadow", 2);
	_cfgfile.SceZoom = ConfGetBoolOrDefault("scezoom", false);
	_cfgfile.PolygonDetails = ConfGetIntOrDefault("polygondetails", 2);

	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr)
		ttsMan->enable(ConfGetBoolOrDefault("tts_narrator", false));

	debug(1, "Sound:          %s", (_cfgfile.Sound ? "true" : "false"));
	debug(1, "Movie:          %i", _cfgfile.Movie);
	debug(1, "Fps:            %i", _cfgfile.Fps);
	debug(1, "UseAutoSaving:  %s", (_cfgfile.UseAutoSaving ? "true" : "false"));
	debug(1, "WallCollision:  %s", (_cfgfile.WallCollision ? "true" : "false"));
	debug(1, "AutoAggressive: %s", (_actor->_combatAuto ? "true" : "false"));
	debug(1, "ShadowMode:     %i", _cfgfile.ShadowMode);
	debug(1, "PolygonDetails: %i", _cfgfile.PolygonDetails);
	debug(1, "SceZoom:        %s", (_cfgfile.SceZoom ? "true" : "false"));
}

void TwinEEngine::queueMovie(const char *filename) {
	_queuedFlaMovie = filename;
}

void TwinEEngine::adjustScreenMax(Common::Rect &rect, int16 x, int16 y) {
	if (x < rect.left) {
		rect.left = x;
	}

	if (x > rect.right) {
		rect.right = x;
	}

	if (y < rect.top) {
		rect.top = y;
	}

	if (y > rect.bottom) {
		rect.bottom = y;
	}
}

void TwinEEngine::clearScreenMinMax(Common::Rect &rect) {
	rect.left = 0x7D00; // SCENE_SIZE_MAX
	rect.right = -0x7D00;
	rect.top = 0x7D00;
	rect.bottom = -0x7D00;
}

void TwinEEngine::introduction() {
	_input->enableKeyMap(cutsceneKeyMapId);
	// Display company logo
	bool abort = false;

	if (isLBA2()) {
		// abort |= _screens->loadImageDelay(_resources->activisionLogo(), 7);
		abort |= _screens->loadImageDelay(_resources->eaLogo(), 7);
	}

	if (isLba1Classic()) {
		abort |= _screens->loadBitmapDelay("Logo2Point21_640_480_256.bmp", 3);
		if (!abort) {
			abort |= _screens->adelineLogo();
		}
		if (!abort) {
			abort |= _screens->loadBitmapDelay("TLBA1C_640_480_256.bmp", 3);
		}
	} else {
		if (isDotEmuEnhanced()) {
			abort |= _screens->loadBitmapDelay("splash_1.png", 3);
		}
		abort |= _screens->adelineLogo();

		if (isLBA1()) {
			// verify game version screens
			if (!abort && _cfgfile.Version == EUROPE_VERSION) {
				// Little Big Adventure screen
				abort |= _screens->loadImageDelay(_resources->lbaLogo(), 3);
				if (!abort && !isDotEmuEnhanced()) {
					// Electronic Arts Logo
					abort |= _screens->loadImageDelay(_resources->eaLogo(), 2);
				}
			} else if (!abort && _cfgfile.Version == USA_VERSION) {
				// Relentless screen
				abort |= _screens->loadImageDelay(_resources->relentLogo(), 3);
				if (!abort && !isDotEmuEnhanced()) {
					// Electronic Arts Logo
					abort |= _screens->loadImageDelay(_resources->eaLogo(), 2);
				}
			} else if (!abort && _cfgfile.Version == MODIFICATION_VERSION) {
				// Modification screen
				abort |= _screens->loadImageDelay(_resources->relentLogo(), 2);
			}
		}
	}

	if (!abort) {
		if (isLBA1()) {
			_movie->playMovie(FLA_DRAGON3);
		} else {
			_movie->playMovie("INTRO");
		}
	}
}

void TwinEEngine::extInitMcga() {
	_redraw->_flagMCGA = true;
	if (_screens->_flagPalettePcx)
		setPalette(_screens->_palettePcx);
	else
		setPalette(_screens->_ptrPal);
}

void TwinEEngine::extInitSvga() {
	_redraw->_flagMCGA = false;
	if (_screens->_flagPalettePcx)
		setPalette(_screens->_palettePcx);
	else
		setPalette(_screens->_ptrPal);
}

void TwinEEngine::testRestoreModeSVGA(bool redraw) {
	if (_redraw->_flagMCGA) {
		extInitSvga();
		if (redraw) {
			_redraw->drawScene(redraw);
		}
	}
}

void TwinEEngine::initAll() {
	_scene->_sceneHero = _scene->getActor(OWN_ACTOR_SCENE_INDEX);

	// Set clip to fullscreen by default, allows main menu to render properly after load
	_interface->unsetClip();

	// getting configuration file
	initConfigurations();

	_resources->initResources();

	extInitSvga();

	_screens->clearScreen();

	// Check if LBA CD-Rom is on drive
	_music->initCdrom();
}

int TwinEEngine::getRandomNumber(uint max) {
	if (max == 0) {
		return 0;
	}
	return _rnd.getRandomNumber(max - 1);
}

void TwinEEngine::saveTimer(bool pause) {
	if (_isTimeFreezed == 0) {
		_saveFreezedTime = timerRef;
		debugC(3, kDebugLevels::kDebugTimers, "freezeTime: timer %i", timerRef);
		if (pause)
			_pauseToken = pauseEngine();
	}
	_isTimeFreezed++;
	debugC(3, kDebugLevels::kDebugTimers, "freezeTime: %i", _isTimeFreezed);
}

void TwinEEngine::restoreTimer() {
	--_isTimeFreezed;
	debugC(3, kDebugLevels::kDebugTimers, "unfreezeTime: %i", _isTimeFreezed);
	if (_isTimeFreezed == 0) {
		timerRef = _saveFreezedTime;
		debugC(3, kDebugLevels::kDebugTimers, "unfreezeTime: time %i", timerRef);
		if (_pauseToken.isActive()) {
			_pauseToken.clear();
		}
	}
}

void TwinEEngine::processActorSamplePosition(int32 actorIdx) {
	const ActorStruct *actor = _scene->getActor(actorIdx);
	const int32 channelIdx = _sound->getActorChannel(actorIdx);
	_sound->setSamplePosition(channelIdx, actor->posObj());
}

void TwinEEngine::processBookOfBu() {
	_screens->fadeToBlack(_screens->_ptrPal);
	_screens->loadImage(TwineImage(Resources::HQR_RESS_FILE, 15, 16));
	_screens->fadeToPal(_screens->_palettePcx);
	_text->initDial(TextBankId::Inventory_Intro_and_Holomap);
	_text->_flagMessageShade = false;
	_text->bigWinDial();
	_text->setFontCrossColor(COLOR_WHITE);
	const bool tmpFlagDisplayText = _cfgfile.FlagDisplayText;
	_cfgfile.FlagDisplayText = true;
	_text->drawTextProgressive(TextId::kBookOfBu);
	_cfgfile.FlagDisplayText = tmpFlagDisplayText;
	_text->normalWinDial();
	_text->_flagMessageShade = true;
	_text->initSceneTextBank();
	_screens->fadeToBlack(_screens->_palettePcx);
	_screens->clearScreen();
	_screens->_flagFade = true;
}

void TwinEEngine::processBonusList() {
	_text->initDial(TextBankId::Inventory_Intro_and_Holomap);
	_text->bigWinDial();
	_text->setFontCrossColor(COLOR_WHITE);
	const bool tmpFlagDisplayText = _cfgfile.FlagDisplayText;
	_cfgfile.FlagDisplayText = true;
	_text->drawTextProgressive(TextId::kBonusList);
	_cfgfile.FlagDisplayText = tmpFlagDisplayText;
	_text->normalWinDial();
	_text->initSceneTextBank();
}

void TwinEEngine::processInventoryAction() {
	saveTimer(false);
	testRestoreModeSVGA(true) ;
	_menu->inventory();

	switch (_loopInventoryItem) {
	case kiHolomap:
		_holomap->holoMap();
		_screens->_flagFade = true;
		break;
	case kiMagicBall:
		if (_gameState->_usingSabre) {
			_actor->initBody(BodyType::btNormal, OWN_ACTOR_SCENE_INDEX);
		}
		_gameState->_usingSabre = false;
		break;
	case kiUseSabre:
		if (_scene->_sceneHero->_genBody != BodyType::btSabre) {
			if (_actor->_heroBehaviour == HeroBehaviourType::kProtoPack) {
				_actor->setBehaviour(HeroBehaviourType::kNormal);
			}
			_actor->initBody(BodyType::btSabre, OWN_ACTOR_SCENE_INDEX);
			_animations->initAnim(AnimationTypes::kSabreUnknown, AnimType::kAnimationThen, AnimationTypes::kStanding, OWN_ACTOR_SCENE_INDEX);

			_gameState->_usingSabre = true;
		}
		break;
	case kiBookOfBu: {
		processBookOfBu();
		break;
	}
	case kiProtoPack:
		if (_gameState->hasItem(InventoryItems::kSendellsMedallion)) {
			_scene->_sceneHero->_genBody = BodyType::btNormal;
		} else {
			_scene->_sceneHero->_genBody = BodyType::btTunic;
		}

		if (_actor->_heroBehaviour == HeroBehaviourType::kProtoPack) {
			_actor->setBehaviour(HeroBehaviourType::kNormal);
		} else {
			_actor->setBehaviour(HeroBehaviourType::kProtoPack);
		}
		break;
	case kiPenguin: {
		ActorStruct *penguin = _scene->getActor(_scene->_mecaPenguinIdx);

		const IVec2 &destPos = _renderer->rotate(0, 800, _scene->_sceneHero->_beta);

		penguin->_posObj = _scene->_sceneHero->posObj();
		penguin->_posObj.x += destPos.x;
		penguin->_posObj.z += destPos.y;
		// TODO: HACK for https://bugs.scummvm.org/ticket/13731
		// The movement of the meca penguin is different from dos version
		// the problem is that the value set to 1 even if the penguin is not yet spawned
		// this might either be a problem with initObject() not being called for the penguin
		// or some other flaw that doesn't ignore the penguin until spawned
		penguin->_workFlags.bIsFalling = 0;

		penguin->_beta = _scene->_sceneHero->_beta;
		debug("penguin angle: %i", penguin->_beta);

		if (_collision->checkValidObjPos(_scene->_mecaPenguinIdx)) {
			penguin->setLife(getMaxLife());
			penguin->_genBody = BodyType::btNone;
			_actor->initBody(BodyType::btNormal, _scene->_mecaPenguinIdx);
			penguin->_workFlags.bIsDead = 0;
			penguin->setCollision(ShapeType::kNone);
			_movements->initRealAngleConst(penguin->_beta, penguin->_beta, penguin->_srot, &penguin->realAngle);
			_gameState->removeItem(InventoryItems::kiPenguin);
			penguin->_delayInMillis = timerRef + toSeconds(30);
		}
		break;
	}
	case kiBonusList: {
		restoreTimer();
		_redraw->drawScene(true);
		saveTimer(false);
		processBonusList();
		break;
	}
	case kiCloverLeaf:
		if (_scene->_sceneHero->_lifePoint < getMaxLife()) {
			if (_gameState->_inventoryNumLeafs > 0) {
				_scene->_sceneHero->setLife(getMaxLife());
				_gameState->setMagicPoints(_gameState->_magicLevelIdx * 20);
				_gameState->addLeafs(-1);
				_redraw->addOverlay(OverlayType::koInventoryItem, InventoryItems::kiCloverLeaf, 0, 0, 0, OverlayPosType::koNormal, 3);
			}
		}
		break;
	}

	restoreTimer();
	_redraw->drawScene(true);
}

int32 TwinEEngine::toSeconds(int x) const {
	if (isLBA1()) {
		return DEFAULT_HZ * x;
	}
	return x * 1000;
}

void TwinEEngine::processOptionsMenu() {
	saveTimer(false);
	testRestoreModeSVGA(true) ;
	_sound->pauseSamples();
	_menu->inGameOptionsMenu();
	_scene->playSceneMusic();
	_sound->resumeSamples();
	restoreTimer();
	_redraw->drawScene(true);
}

bool TwinEEngine::runGameEngine() { // mainLoopInteration
	g_system->delayMillis(2);

	FrameMarker frame(this, 60);
	_input->enableKeyMap(mainKeyMapId);

	readKeys();

	if (!_queuedFlaMovie.empty()) {
		_movie->playMovie(_queuedFlaMovie.c_str());
		_queuedFlaMovie.clear();
	}

	if (_scene->_newCube > -1) {
		if (!isMod() && isDemo() && isLBA1()) {
			// the demo only has these scenes
			if (_scene->_newCube != LBA1SceneId::Citadel_Island_Prison && _scene->_newCube != LBA1SceneId::Citadel_Island_outside_the_citadel && _scene->_newCube != LBA1SceneId::Citadel_Island_near_the_tavern) {
				_music->playMidiFile(6);
				return true;
			}
		}
		_scene->changeCube();
	}

	_movements->update();

	_debugState->update();

	if (_menuOptions->flagCredits) {
		if (isLBA1()) {
			if (isCDROM()) {
				if (_music->getMusicCD() != 8) {
					_music->playCdTrack(8);
				}
			} else if (!_music->isMidiPlaying()) {
				_music->playMidiFile(9);
			}
		}
		if (_input->toggleAbortAction()) {
			return true;
		}
	} else if (!_screens->_flagFade) {
		// Process give up menu - Press ESC
		if (_input->toggleAbortAction() && _scene->_sceneHero->_lifePoint > 0 && _scene->_sceneHero->_body != -1 && !_scene->_sceneHero->_flags.bIsInvisible) {
			saveTimer(false);
			testRestoreModeSVGA(true) ;
			const int giveUp = _menu->quitMenu();
			if (giveUp == kQuitEngine) {
				return false;
			}
			if (giveUp == 1) {
				restoreTimer();
				_redraw->drawScene(true);
				_sceneLoopState = SceneLoopState::ReturnToMenu;
				return false;
			}
			restoreTimer();
			_redraw->drawScene(true);
		}

		if (_input->toggleActionIfActive(TwinEActionType::OptionsMenu)) {
			processOptionsMenu();
		}

		// inventory menu
		_loopInventoryItem = -1;
		if (_input->isActionActive(TwinEActionType::InventoryMenu) && _scene->_sceneHero->_body != -1 && _scene->_sceneHero->_controlMode == ControlMode::kManual) {
			processInventoryAction();
		}

		if (_input->toggleActionIfActive(TwinEActionType::ChangeBehaviourNormal)) {
			_actor->setBehaviour(HeroBehaviourType::kNormal);
		} else if (_input->toggleActionIfActive(TwinEActionType::ChangeBehaviourAthletic)) {
			_actor->setBehaviour(HeroBehaviourType::kAthletic);
		} else if (_input->toggleActionIfActive(TwinEActionType::ChangeBehaviourAggressive)) {
			_actor->setBehaviour(HeroBehaviourType::kAggressive);
		} else if (_input->toggleActionIfActive(TwinEActionType::ChangeBehaviourDiscreet)) {
			_actor->setBehaviour(HeroBehaviourType::kDiscrete);
		}

		// Process behaviour menu
		const bool behaviourMenu = _input->isActionActive(TwinEActionType::BehaviourMenu, false);
		if ((behaviourMenu ||
			 _input->isActionActive(TwinEActionType::QuickBehaviourNormal, false) ||
			 _input->isActionActive(TwinEActionType::QuickBehaviourAthletic, false) ||
			 _input->isActionActive(TwinEActionType::QuickBehaviourAggressive, false) ||
			 _input->isActionActive(TwinEActionType::QuickBehaviourDiscreet, false)) &&
			_scene->_sceneHero->_body != -1 && _scene->_sceneHero->_controlMode == ControlMode::kManual) {
			if (_input->isActionActive(TwinEActionType::QuickBehaviourNormal, false)) {
				_actor->_heroBehaviour = HeroBehaviourType::kNormal;
			} else if (_input->isActionActive(TwinEActionType::QuickBehaviourAthletic, false)) {
				_actor->_heroBehaviour = HeroBehaviourType::kAthletic;
			} else if (_input->isActionActive(TwinEActionType::QuickBehaviourAggressive, false)) {
				_actor->_heroBehaviour = HeroBehaviourType::kAggressive;
			} else if (_input->isActionActive(TwinEActionType::QuickBehaviourDiscreet, false)) {
				_actor->_heroBehaviour = HeroBehaviourType::kDiscrete;
			}
			saveTimer(false);
			testRestoreModeSVGA(true);
			_menu->processBehaviourMenu(behaviourMenu);
			restoreTimer();
			_redraw->drawScene(true);
		}

		// use Proto-Pack
		if (_input->toggleActionIfActive(TwinEActionType::UseProtoPack) && _gameState->hasItem(InventoryItems::kiProtoPack)) {
			if (_gameState->hasItem(InventoryItems::kiBookOfBu)) {
				_scene->_sceneHero->_genBody = BodyType::btNormal;
			} else {
				_scene->_sceneHero->_genBody = BodyType::btTunic;
			}

			if (_actor->_heroBehaviour == HeroBehaviourType::kProtoPack) {
				_actor->setBehaviour(HeroBehaviourType::kNormal);
			} else {
				_actor->setBehaviour(HeroBehaviourType::kProtoPack);
			}
		}

		// Recenter Screen
		if (_input->toggleActionIfActive(TwinEActionType::RecenterScreenOnTwinsen) && !_cameraZone) {
			const ActorStruct *currentlyFollowedActor = _scene->getActor(_scene->_numObjFollow);
			_grid->centerOnActor(currentlyFollowedActor);
		}

		// Draw holomap
		if (_input->toggleActionIfActive(TwinEActionType::OpenHolomap) && _gameState->hasItem(InventoryItems::kiHolomap) && !_gameState->inventoryDisabled()) {
			testRestoreModeSVGA(true);
			saveTimer(false);
			_holomap->holoMap();
			// unfreeze here - the redrawEngineActions is also doing a freeze
			// see https://bugs.scummvm.org/ticket/14808
			restoreTimer();
			_screens->_flagFade = true;
			_redraw->drawScene(true);
		}

		// Process Pause
		if (_input->toggleActionIfActive(TwinEActionType::Pause)) {
			saveTimer(true);
			const char *PauseString = "Pause";
			_text->setFontColor(COLOR_WHITE);
			if (_redraw->_flagMCGA) {
				_text->drawText(_redraw->_sceneryViewX + 5, _redraw->_sceneryViewY, PauseString);
			} else {
				const int width = _text->sizeFont(PauseString);
				const int bottom = height() - _text->lineHeight;
				_text->drawText(5, bottom, PauseString);
				copyBlockPhys(5, bottom, 5 + width, bottom + _text->lineHeight);
			}
			do {
				FrameMarker frameWait(this);
				readKeys();
				if (shouldQuit()) {
					break;
				}
			} while (!_input->toggleActionIfActive(TwinEActionType::Pause));
			restoreTimer();
			_redraw->drawScene(true);
		}

		if (_input->toggleActionIfActive(TwinEActionType::SceneryZoom)) {
			_redraw->_flagMCGA ^= true;
			if (_redraw->_flagMCGA) {
				extInitMcga();
			} else {
				extInitSvga();
				_redraw->_firstTime = true;
			}
		}
	}

	_stepFalling = _realFalling.getRealValueFromTime(timerRef);
	if (!_stepFalling) {
		_stepFalling = 1;
	}

	_movements->initRealValue(LBAAngles::ANGLE_0, -LBAAngles::ANGLE_90, LBAAngles::ANGLE_1, &_realFalling);
	_cameraZone = false;

	_scene->processEnvironmentSound();

	// Reset HitBy state
	for (int32 a = 0; a < _scene->_nbObjets; a++) {
		_scene->getActor(a)->_hitBy = -1;
	}

	_extra->gereExtras();

	for (int32 a = 0; a < _scene->_nbObjets; a++) {
		ActorStruct *actor = _scene->getActor(a);

		if (actor->_workFlags.bIsDead) {
			continue;
		}

		if (actor->_lifePoint == 0) {
			if (IS_HERO(a)) {
				_animations->initAnim(AnimationTypes::kLandDeath, AnimType::kAnimationSet, AnimationTypes::kStanding, OWN_ACTOR_SCENE_INDEX);
				actor->_controlMode = ControlMode::kNoMove;
#if 0 // TODO: enable me - found in the lba1 community release source code
				// Disable collisions on Twinsen to allow other objects to continue their tracks
				// while the death animation is playing
				actor->_flags.bObjFallable = 1;
				actor->_flags.bCheckZone = 0;
				actor->_flags.bComputeCollisionWithObj = 0;
				actor->_flags.bComputeCollisionWithBricks = 0;
				actor->_flags.bCanDrown = 1;
				actor->_workFlags.bIsHitting = 0;
#endif
			} else {
				_sound->playSample(Samples::Explode, 1, actor->posObj(), a);

				if (a == _scene->_mecaPenguinIdx) {
					_extra->extraExplo(actor->posObj());
				}
			}

			if (!actor->_bonusParameter.givenNothing && (actor->_bonusParameter.cloverleaf || actor->_bonusParameter.kashes || actor->_bonusParameter.key || actor->_bonusParameter.lifepoints || actor->_bonusParameter.magicpoints)) {
				_actor->giveExtraBonus(a);
			}
		}

		_movements->doDir(a);

		actor->_oldPos = actor->posObj();

		if (actor->_offsetTrack != -1) {
			_scriptMove->doTrack(a);
		}

		_animations->doAnim(a);

		if (actor->_flags.bCheckZone) {
			_scene->checkZoneSce(a);
		}

		if (actor->_offsetLife != -1) {
			_scriptLife->doLife(a);
		}

		processActorSamplePosition(a);

		if (_sceneLoopState != SceneLoopState::Continue) {
			return _sceneLoopState == SceneLoopState::Finished;
		}

		if (actor->_flags.bCanDrown) {
			const uint8 brickSound = _grid->worldCodeBrick(actor->_posObj.x, actor->_posObj.y - 1, actor->_posObj.z);
			actor->_brickSound = brickSound;

			if (brickSound == WATER_BRICK) {
				if (IS_HERO(a)) {
					// we are dying if we aren't using the protopack to fly over water
					if (_actor->_heroBehaviour != HeroBehaviourType::kProtoPack || actor->_genAnim != AnimationTypes::kForward) {
						if (!_actor->_cropBottomScreen) {
							_animations->initAnim(AnimationTypes::kDrawn, AnimType::kAnimationSet, AnimationTypes::kStanding, OWN_ACTOR_SCENE_INDEX);
						}
						const IVec3 &projPos = _renderer->projectPoint(actor->posObj() - _grid->_worldCube);
						actor->_controlMode = ControlMode::kNoMove;
						actor->setLife(-1);
						_actor->_cropBottomScreen = projPos.y;
						actor->_flags.bNoShadow = 1;
					}
				} else {
					_sound->playSample(Samples::Explode, 1, actor->posObj(), a);
					if (actor->_bonusParameter.cloverleaf || actor->_bonusParameter.kashes || actor->_bonusParameter.key || actor->_bonusParameter.lifepoints || actor->_bonusParameter.magicpoints) {
						if (!actor->_bonusParameter.givenNothing) {
							_actor->giveExtraBonus(a);
						}
						actor->setLife(0);
					}
				}
			}
		}

		if (actor->_lifePoint <= 0) {
			if (IS_HERO(a)) {
				if (actor->_workFlags.bAnimEnded) {
					if (_gameState->_inventoryNumLeafs > 0) { // use clover leaf automaticaly
						_scene->_sceneHero->_posObj = _scene->_newHeroPos;

						_scene->_newCube = _scene->_numCube;
						_gameState->setMaxMagicPoints();

						_grid->centerOnActor(_scene->_sceneHero);

						_scene->_heroPositionType = ScenePositionType::kReborn;

						_scene->_sceneHero->setLife(getMaxLife());
						_redraw->_firstTime = true;
						_screens->_flagFade = true;
						_gameState->addLeafs(-1);
						_actor->_cropBottomScreen = 0;
					} else { // game over
						_gameState->setLeafBoxes(2);
						_gameState->setLeafs(1);
						_gameState->setMaxMagicPoints();
						_actor->_heroBehaviour = _actor->_previousHeroBehaviour;
						actor->_beta = _actor->_previousHeroAngle;
						actor->setLife(getMaxLife());

						if (_scene->_oldcube != _scene->_numCube) {
							_scene->_newHeroPos.x = -1;
							_scene->_newHeroPos.y = -1;
							_scene->_newHeroPos.z = -1;
							_scene->_numCube = _scene->_oldcube;
							_scene->stopRunningGame();
						}

						autoSave();
						_gameState->processGameoverAnimation();
						_sceneLoopState = SceneLoopState::ReturnToMenu;
						return false;
					}
				}
			} else {
				_actor->checkCarrier(a);
				actor->_workFlags.bIsDead = 1;
				actor->_body = -1;
				actor->_zoneSce = -1;
			}
		}

		if (_scene->_newCube != -1) {
			return false;
		}
	}

	_grid->centerScreenOnActor();

	_redraw->drawScene(_redraw->_firstTime);

	// workaround to fix hero redraw after drowning
	if (_actor->_cropBottomScreen && _redraw->_firstTime) {
		_scene->_sceneHero->_flags.bIsInvisible = 1;
		_redraw->drawScene(true);
		_scene->_sceneHero->_flags.bIsInvisible = 0;
	}

	_scene->_newCube = SCENE_CEILING_GRID_FADE_1;
	_redraw->_firstTime = false;

	return false;
}

bool TwinEEngine::mainLoop() {
	_redraw->_firstTime = true;
	_screens->_flagFade = true;
	_movements->initRealValue(LBAAngles::ANGLE_0, -LBAAngles::ANGLE_90, LBAAngles::ANGLE_1, &_realFalling);

	while (_sceneLoopState == SceneLoopState::Continue) {
		if (runGameEngine()) {
			return true;
		}
		timerRef++;
		if (shouldQuit()) {
			break;
		}
	}
	return false;
}

bool TwinEEngine::delaySkip(uint32 time) {
	uint32 startTicks = _system->getMillis();
	uint32 stopTicks = 0;
	do {
		FrameMarker frame(this);
		readKeys();
		if (_input->toggleAbortAction()) {
			return true;
		}
		if (shouldQuit()) {
			return true;
		}
		stopTicks = _system->getMillis() - startTicks;
		//_lbaTime++;
	} while (stopTicks <= time);
	return false;
}

void TwinEEngine::saveFrontBuffer() { // CopyScreen(Log, Screen)
	_screens->copyScreen(_frontVideoBuffer, _workVideoBuffer);
}

void TwinEEngine::restoreFrontBuffer() { // CopyScreen
	_screens->copyScreen(_workVideoBuffer, _frontVideoBuffer);
}

void TwinEEngine::blitWorkToFront(const Common::Rect &rect) {
	_interface->blitBox(rect, _workVideoBuffer, _frontVideoBuffer);
	copyBlockPhys(rect);
}

void TwinEEngine::blitFrontToWork(const Common::Rect &rect) {
	_interface->blitBox(rect, _frontVideoBuffer, _workVideoBuffer);
}

void TwinEEngine::setPalette(const Graphics::Palette &palette, uint startColor) {
	debugC(1, TwinE::kDebugPalette, "Change palette (%i colors, starting at %i)", (int)palette.size(), (int)startColor);
	_frontVideoBuffer.setPalette(palette, startColor);
}

void TwinEEngine::setPalette(uint startColor, uint numColors, const byte *palette) {
	if (numColors == 0 || palette == nullptr) {
		warning("Could not set palette");
		return;
	}
	debugC(1, TwinE::kDebugPalette, "Change palette (%i colors, starting at %i)", (int)numColors, (int)startColor);
	_frontVideoBuffer.setPalette(palette, startColor, numColors);
}

void TwinEEngine::copyBlockPhys(const Common::Rect &rect) {
	copyBlockPhys(rect.left, rect.top, rect.right, rect.bottom);
}

void TwinEEngine::copyBlockPhys(int32 left, int32 top, int32 right, int32 bottom) {
	assert(left <= right);
	assert(top <= bottom);
	int32 width = right - left + 1;
	int32 height = bottom - top + 1;
	if (left + width > this->width()) {
		width = this->width() - left;
	}
	if (top + height > this->height()) {
		height = this->height() - top;
	}
	if (width <= 0 || height <= 0) {
		return;
	}
	_frontVideoBuffer.addDirtyRect(Common::Rect(left, top, right, bottom));
}

void TwinEEngine::readKeys() {
	_input->readKeys();
}

void TwinEEngine::drawText(int32 x, int32 y, const Common::String &text, bool center, bool bigFont, int width) {
	const Graphics::Font *font = FontMan.getFontByUsage(bigFont ? Graphics::FontManager::kBigGUIFont : Graphics::FontManager::kGUIFont);
	if (!font) {
		return;
	}
	font->drawString(&_frontVideoBuffer, text,
					 x, y, width,
					 _frontVideoBuffer.format.RGBToColor(255, 255, 255),
					 center ? Graphics::kTextAlignCenter : Graphics::kTextAlignLeft, 0, true);
}

Common::Language TwinEEngine::getGameLang() const {
	return _gameLang;
}

const char *TwinEEngine::getGameId() const {
	if (isLBA1()) {
		return "lba";
	}
	assert(isLBA2());
	return "lba2";
}

bool TwinEEngine::unlockAchievement(const Common::String &id) {
	return AchMan.setAchievement(id);
}

Common::Rect TwinEEngine::centerOnScreenX(int32 w, int32 y, int32 h) const {
	const int32 left = width() / 2 - w / 2;
	const int32 right = left + w;
	const int32 top = y;
	const int32 bottom = top + h;
	return Common::Rect(left, top, right, bottom);
}

Common::Rect TwinEEngine::centerOnScreen(int32 w, int32 h) const {
	const int32 left = width() / 2 - w / 2;
	const int32 right = left + w;
	const int32 top = height() / 2 - h / 2;
	const int32 bottom = top + h;
	return Common::Rect(left, top, right, bottom);
}

} // namespace TwinE
