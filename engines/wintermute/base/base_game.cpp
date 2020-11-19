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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_game_music.h"
#include "engines/wintermute/base/base_game_settings.h"
#include "engines/wintermute/base/base_fader.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#ifdef ENABLE_WME3D
#include "engines/wintermute/base/gfx/base_renderer3d.h"
#endif
#include "engines/wintermute/base/base_keyboard_state.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_quick_msg.h"
#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/base/base_transition_manager.h"
#include "engines/wintermute/base/base_viewport.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/saveload.h"
#include "engines/wintermute/base/save_thumb_helper.h"
#include "engines/wintermute/base/scriptables/script_ext_array.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/ext/plugins.h"
#include "engines/wintermute/video/video_player.h"
#include "engines/wintermute/video/video_theora_player.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/utils/crc.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/platform_osystem.h"
#include "base/version.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/keyboard.h"
#include "common/system.h"
#include "common/file.h"
#include "graphics/scaler.h"

#if EXTENDED_DEBUGGER_ENABLED
#include "engines/wintermute/base/scriptables/debuggable/debuggable_script_engine.h"
#endif

#ifdef ENABLE_WME3D
#include "graphics/renderer.h"
#include "engines/util.h"
#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
#include "graphics/opengl/context.h"
#endif
#endif

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(BaseGame, true)


//////////////////////////////////////////////////////////////////////
BaseGame::BaseGame(const Common::String &targetName) : BaseObject(this), _targetName(targetName), _timerNormal(), _timerLive() {
	_shuttingDown = false;

	_state = GAME_RUNNING;
	_origState = GAME_RUNNING;
	_freezeLevel = 0;

	_interactive = true;
	_origInteractive = false;

	_surfaceStorage = nullptr;
	_fontStorage = nullptr;
	_renderer = nullptr;
	_soundMgr = nullptr;
	_transMgr = nullptr;
	_scEngine = nullptr;
	_keyboardState = nullptr;

	_mathClass = nullptr;
	_directoryClass = nullptr;

	_debugLogFile = nullptr;
	_debugDebugMode = false;
	_debugShowFPS = false;

	_systemFont = nullptr;
	_videoFont = nullptr;

	_videoPlayer = nullptr;
	_theoraPlayer = nullptr;

	_mainObject = nullptr;
	_activeObject = nullptr;

	_fader = nullptr;

	_offsetX = _offsetY = 0;
	_offsetPercentX = _offsetPercentY = 0.0f;

	_subtitles = true;
	_videoSubtitles = true;

	_sequence = 0;

	_mousePos.x = _mousePos.y = 0;
	_mouseLeftDown = _mouseRightDown = _mouseMidlleDown = false;
	_capturedObject = nullptr;

	// FPS counters
	_lastTime = _fpsTime = _deltaTime = _framesRendered = _fps = 0;

	_cursorNoninteractive = nullptr;

#ifdef ENABLE_WME3D
	_useD3D = true;
	_renderer3D = nullptr;
	_playing3DGame = false;

	_supportsRealTimeShadows = false;
	_maxShadowType = SHADOW_STENCIL;
#else
	_useD3D = false;
#endif

	_musicSystem = new BaseGameMusic(this);

	_editorForceScripts = false;
	_editorAlwaysRegister = false;

	_focusedWindow = nullptr;

	_loadInProgress = false;

	_quitting = false;
	_loading = false;
	_scheduledLoadSlot = -1;

	_personalizedSave = false;

	_editorMode = false;
	//_doNotExpandStrings = false;

	_engineLogCallback = nullptr;
	_engineLogCallbackData = nullptr;

	_smartCache = false;
	_surfaceGCCycleTime = 10000;

	_reportTextureFormat = false;

	_viewportSP = -1;

	_subtitlesSpeed = 70;

	_forceNonStreamedSounds = false;

	// These are NOT the actual engine defaults (they are 0, 0),
	// but we have a use for thumbnails even for games that don't
	// use them in-game, hence we set a default that is suitably
	// sized for the GMM (expecting 4:3 ratio)
	_thumbnailWidth = kThumbnailWidth;
	_thumbnailHeight = kThumbnailHeight2;

	_localSaveDir = "saves";

	_saveDirChecked = false;

	_loadingIcon = nullptr;
	_loadingIconX = _loadingIconY = 0;
	_loadingIconPersistent = false;

	_textEncoding = TEXT_ANSI;
	_textRTL = false;

	_soundBufferSizeSec = 3;
	_suspendedRendering = false;

	_lastCursor = nullptr;

	_mouseLockRect.setEmpty();

	_suppressScriptErrors = false;
	_lastMiniUpdate = 0;
	_miniUpdateEnabled = false;

	_cachedThumbnail = nullptr;

	_autorunDisabled = false;

	// compatibility bits
	_compatKillMethodThreads = false;

	_usedMem = 0;


	_autoSaveOnExit = true;
	_autoSaveSlot = 999;
	_cursorHidden = false;

	// Block kept as a reminder that the engine CAN run in constrained/touch-mode
	/*#ifdef __IPHONEOS__
	    _touchInterface = true;
	    _constrainedMemory = true; // TODO differentiate old and new iOS devices
	#else*/
	_touchInterface = false;
	_constrainedMemory = false;

	_settings = new BaseGameSettings(this);

#ifdef ENABLE_HEROCRAFT
	_rndHc = new Common::RandomSource("HeroCraft");
#endif
}


//////////////////////////////////////////////////////////////////////
BaseGame::~BaseGame() {
	_shuttingDown = true;

	LOG(0, "");
	LOG(0, "Shutting down...");

	ConfMan.setBool("last_run", true);
	ConfMan.flushToDisk();

	cleanup();

	delete _mathClass;
	delete _directoryClass;

	delete _transMgr;
	delete _scEngine;
	delete _fontStorage;
	delete _surfaceStorage;
	delete _videoPlayer;
	delete _theoraPlayer;
	delete _soundMgr;
	//SAFE_DELETE(_keyboardState);

	delete _renderer;
	delete _musicSystem;
	delete _settings;

	_mathClass = nullptr;
	_directoryClass = nullptr;

	_transMgr = nullptr;
	_scEngine = nullptr;
	_fontStorage = nullptr;
	_surfaceStorage = nullptr;
	_videoPlayer = nullptr;
	_theoraPlayer = nullptr;
	_soundMgr = nullptr;

	_renderer = nullptr;
	_musicSystem = nullptr;
	_settings = nullptr;

#ifdef ENABLE_HEROCRAFT
	delete _rndHc;
	_rndHc = nullptr;
#endif

	DEBUG_DebugDisable();
	debugC(kWintermuteDebugLog, "--- shutting down normally ---\n");
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::cleanup() {
	delete _loadingIcon;
	_loadingIcon = nullptr;

	deleteSaveThumbnail();

	_engineLogCallback = nullptr;
	_engineLogCallbackData = nullptr;

	_musicSystem->cleanup();

	unregisterObject(_fader);
	_fader = nullptr;

	for (uint32 i = 0; i < _regObjects.size(); i++) {
		delete _regObjects[i];
		_regObjects[i] = nullptr;
	}
	_regObjects.clear();

	_windows.clear(); // refs only
	_focusedWindow = nullptr; // ref only

	delete _cursorNoninteractive;
	delete _cursor;
	delete _activeCursor;
	_cursorNoninteractive = nullptr;
	_cursor = nullptr;
	_activeCursor = nullptr;

	delete _scValue;
	delete _sFX;
	_scValue = nullptr;
	_sFX = nullptr;

	for (uint32 i = 0; i < _scripts.size(); i++) {
		_scripts[i]->_owner = nullptr;
		_scripts[i]->finish();
	}
	_scripts.clear();

	_fontStorage->removeFont(_systemFont);
	_systemFont = nullptr;

	_fontStorage->removeFont(_videoFont);
	_videoFont = nullptr;

	for (uint32 i = 0; i < _quickMessages.size(); i++) {
		delete _quickMessages[i];
	}
	_quickMessages.clear();

	_viewportStack.clear();
	_viewportSP = -1;

	setName(nullptr);
	setFilename(nullptr);
	for (int i = 0; i < 7; i++) {
		delete[] _caption[i];
		_caption[i] = nullptr;
	}

	_lastCursor = nullptr;

	delete _keyboardState;
	_keyboardState = nullptr;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
bool BaseGame::initConfManSettings() {
	if (ConfMan.hasKey("debug_mode")) {
		if (ConfMan.getBool("debug_mode")) {
			DEBUG_DebugEnable("./wme.log");
		}
	}

	if (ConfMan.hasKey("show_fps")) {
		_debugShowFPS = ConfMan.getBool("show_fps");
	} else {
		_debugShowFPS = false;
	}

	if (ConfMan.hasKey("bilinear_filtering")) {
		_bilinearFiltering = ConfMan.getBool("bilinear_filtering");
	} else {
		_bilinearFiltering = false;
	}

	if (ConfMan.hasKey("disable_smartcache")) {
		_smartCache = ConfMan.getBool("disable_smartcache");
	} else {
		_smartCache = true;
	}

	if (!_smartCache) {
		LOG(0, "Smart cache is DISABLED");
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
bool BaseGame::initRenderer() {
	bool windowedMode = !ConfMan.getBool("fullscreen");
	return _renderer->initRenderer(_settings->getResWidth(), _settings->getResHeight(), windowedMode);
}

//////////////////////////////////////////////////////////////////////
bool BaseGame::loadGameSettingsFile() {
	return loadFile(_settings->getGameFile());
}

//////////////////////////////////////////////////////////////////////
bool BaseGame::initialize1() {
	bool loaded = false; // Not really a loop, but a goto-replacement.
	while (!loaded) {
		_surfaceStorage = new BaseSurfaceStorage(this);
		if (_surfaceStorage == nullptr) {
			break;
		}

		_fontStorage = new BaseFontStorage(this);
		if (_fontStorage == nullptr) {
			break;
		}

		_soundMgr = new BaseSoundMgr(this);
		if (_soundMgr == nullptr) {
			break;
		}

		_mathClass = makeSXMath(this);
		if (_mathClass == nullptr) {
			break;
		}

		_directoryClass = makeSXDirectory(this);
		if (_directoryClass == nullptr) {
			break;
		}

#if EXTENDED_DEBUGGER_ENABLED
		_scEngine = new DebuggableScEngine(this);
#else
		_scEngine = new ScEngine(this);
#endif
		if (_scEngine == nullptr) {
			break;
		}

		_videoPlayer = new VideoPlayer(this);
		if (_videoPlayer == nullptr) {
			break;
		}

		_transMgr = new BaseTransitionMgr(this);
		if (_transMgr == nullptr) {
			break;
		}

		_keyboardState = new BaseKeyboardState(this);
		if (_keyboardState == nullptr) {
			break;
		}

		_fader = new BaseFader(this);
		if (_fader == nullptr) {
			break;
		}
		registerObject(_fader);

		loaded = true;
	}
	if (loaded == true) {
		return STATUS_OK;
	} else {
		delete _mathClass;
		delete _directoryClass;
		delete _keyboardState;
		delete _transMgr;
		delete _surfaceStorage;
		delete _fontStorage;
		delete _soundMgr;
		delete _scEngine;
		delete _videoPlayer;
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////
bool BaseGame::initialize2() { // we know whether we are going to be accelerated
#ifdef ENABLE_WME3D
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::parseRendererTypeCode(rendererConfig);

	if (!_playing3DGame && desiredRendererType == Graphics::kRendererTypeTinyGL) {
		_renderer = makeOSystemRenderer(this);
		if (_renderer == nullptr) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
	initGraphics3d(_settings->getResWidth(), _settings->getResHeight());
	bool backendCapableOpenGL = g_system->hasFeature(OSystem::kFeatureOpenGLForGame);
#endif

#if defined(USE_OPENGL_GAME)
	// Check the OpenGL context actually supports shaders
	if (backendCapableOpenGL && desiredRendererType == Graphics::kRendererTypeOpenGLShaders && !OpenGLContext.shadersSupported) {
		desiredRendererType = Graphics::kRendererTypeOpenGL;
	}
#endif

#if defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
	if (backendCapableOpenGL && desiredRendererType == Graphics::kRendererTypeOpenGLShaders) {
		_renderer3D = makeOpenGL3DShaderRenderer(this);
	}
#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
#if defined(USE_OPENGL_GAME)
	if (backendCapableOpenGL && desiredRendererType == Graphics::kRendererTypeOpenGL) {
		_renderer3D = makeOpenGL3DRenderer(this);
	}
#endif // defined(USE_OPENGL)
	if (_playing3DGame && desiredRendererType == Graphics::kRendererTypeTinyGL) {
		_renderer3D = nullptr;// TODO: makeTinyGL3DRenderer(this);
		error("3D software renderered is not supported yet");
	}
	_renderer = _renderer3D;
#if !defined(USE_OPENGL_GAME) && !defined(USE_OPENGL_SHADERS) && !defined(USE_GLES2)
	if (!_playing3DGame && !_renderer3D)
		_renderer = makeOSystemRenderer(this);
#endif
#else
	_renderer = makeOSystemRenderer(this);
#endif
	if (_renderer == nullptr) {
		return STATUS_FAILED;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
bool BaseGame::initialize3() { // renderer is initialized
	_posX = _renderer->getWidth() / 2;
	_posY = _renderer->getHeight() / 2;
	_renderer->initIndicator();
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
void BaseGame::DEBUG_DebugEnable(const char *filename) {
	_debugDebugMode = true;

	int secs = g_system->getMillis() / 1000;
	int hours = secs / 3600;
	secs = secs % 3600;
	int mins = secs / 60;
	secs = secs % 60;

#ifdef _DEBUG
	LOG(0, "********** DEBUG LOG OPENED %02d-%02d-%02d (Debug Build) *******************", hours, mins, secs);
#else
	LOG(0, "********** DEBUG LOG OPENED %02d-%02d-%02d (Release Build) *****************", hours, mins, secs);
#endif

	LOG(0, "%s - %s ver %d.%d.%d%s ", gScummVMFullVersion, DCGF_NAME, DCGF_VER_MAJOR, DCGF_VER_MINOR, DCGF_VER_BUILD, DCGF_VER_SUFFIX);

	AnsiString platform = BasePlatform::getPlatformName();
	LOG(0, "Platform: %s", platform.c_str());
	LOG(0, "");
}


//////////////////////////////////////////////////////////////////////
void BaseGame::DEBUG_DebugDisable() {
	if (_debugLogFile != nullptr) {
		LOG(0, "********** DEBUG LOG CLOSED ********************************************");
		//fclose((FILE *)_debugLogFile);
		_debugLogFile = nullptr;
	}
	_debugDebugMode = false;
}


//////////////////////////////////////////////////////////////////////
void BaseGame::LOG(bool res, const char *fmt, ...) {
	uint32 secs = g_system->getMillis() / 1000;
	uint32 hours = secs / 3600;
	secs = secs % 3600;
	uint32 mins = secs / 60;
	secs = secs % 60;

	char buff[512];
	va_list va;

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	// redirect to an engine's own callback
	if (_engineLogCallback) {
		_engineLogCallback(buff, res, _engineLogCallbackData);
	}

	debugCN(kWintermuteDebugLog, "%02d:%02d:%02d: %s\n", hours, mins, secs, buff);

	//fprintf((FILE *)_debugLogFile, "%02d:%02d:%02d: %s\n", hours, mins, secs, buff);
	//fflush((FILE *)_debugLogFile);

	//QuickMessage(buff);
}

#ifdef ENABLE_WME3D
bool BaseGame::setMaxShadowType(TShadowType maxShadowType) {
	if (maxShadowType > SHADOW_STENCIL) {
		maxShadowType = SHADOW_STENCIL;
	}

	if (maxShadowType < 0) {
		maxShadowType = SHADOW_NONE;
	}

	if (maxShadowType == SHADOW_FLAT && !_supportsRealTimeShadows) {
		maxShadowType = SHADOW_SIMPLE;
	}

	_maxShadowType = maxShadowType;

	return STATUS_OK;
}

TShadowType BaseGame::getMaxShadowType(BaseObject *object) {
	if (object) {
		return MIN(_maxShadowType, object->_shadowType);
	} else {
		return _maxShadowType;
	}
}

uint32 BaseGame::getAmbientLightColor() {
	return 0x00000000;
}

bool BaseGame::getFogParams(FogParameters &fogParameters) {
	fogParameters._enabled = false;
	return true;
}
#endif

//////////////////////////////////////////////////////////////////////////
void BaseGame::setEngineLogCallback(ENGINE_LOG_CALLBACK callback, void *data) {
	_engineLogCallback = callback;
	_engineLogCallbackData = data;
}


//////////////////////////////////////////////////////////////////////
bool BaseGame::initLoop() {
	_viewportSP = -1;

	_currentTime = g_system->getMillis();

	_renderer->initLoop();
	_musicSystem->updateMusicCrossfade();

	_surfaceStorage->initLoop();
	_fontStorage->initLoop();


	//_activeObject = nullptr;

	// count FPS
	_deltaTime = _currentTime - _lastTime;
	_lastTime  = _currentTime;
	_fpsTime += _deltaTime;

	_timerLive.updateTime(_deltaTime, 1000);

	if (_state != GAME_FROZEN) {
		_timerNormal.updateTime(_deltaTime, 1000);
	} else {
		_timerNormal.setTimeDelta(0);
	}

	_framesRendered++;
	if (_fpsTime > 1000) {
		_fps = _framesRendered;
		_framesRendered  = 0;
		_fpsTime = 0;
	}
	//_gameRef->LOG(0, "%d", _fps);

	getMousePos(&_mousePos);

	_focusedWindow = nullptr;
	for (int i = _windows.size() - 1; i >= 0; i--) {
		if (_windows[i]->isVisible()) {
			_focusedWindow = _windows[i];
			break;
		}
	}

	updateSounds();

	if (_fader) {
		_fader->update();
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
bool BaseGame::initInput() {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int BaseGame::getSequence() {
	return ++_sequence;
}


//////////////////////////////////////////////////////////////////////////
void BaseGame::setOffset(int32 offsetX, int32 offsetY) {
	_offsetX = offsetX;
	_offsetY = offsetY;
}

//////////////////////////////////////////////////////////////////////////
void BaseGame::getOffset(int *offsetX, int *offsetY) const {
	if (offsetX != nullptr) {
		*offsetX = _offsetX;
	}
	if (offsetY != nullptr) {
		*offsetY = _offsetY;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::loadFile(const char *filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_gameRef->LOG(0, "BaseGame::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing GAME file '%s'", filename);
	}

	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(GAME)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(SYSTEM_FONT)
TOKEN_DEF(VIDEO_FONT)
TOKEN_DEF(EVENTS)
TOKEN_DEF(CURSOR)
TOKEN_DEF(ACTIVE_CURSOR)
TOKEN_DEF(NONINTERACTIVE_CURSOR)
TOKEN_DEF(STRING_TABLE)
TOKEN_DEF(RESOLUTION)
TOKEN_DEF(SETTINGS)
TOKEN_DEF(REQUIRE_3D_ACCELERATION)
TOKEN_DEF(REQUIRE_SOUND)
TOKEN_DEF(HWTL_MODE)
TOKEN_DEF(ALLOW_WINDOWED_MODE)
TOKEN_DEF(ALLOW_ACCESSIBILITY_TAB)
TOKEN_DEF(ALLOW_ABOUT_TAB)
TOKEN_DEF(ALLOW_ADVANCED)
TOKEN_DEF(ALLOW_DESKTOP_RES)
TOKEN_DEF(REGISTRY_PATH)
TOKEN_DEF(PERSONAL_SAVEGAMES)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(SUBTITLES_SPEED)
TOKEN_DEF(SUBTITLES)
TOKEN_DEF(VIDEO_SUBTITLES)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(THUMBNAIL_WIDTH)
TOKEN_DEF(THUMBNAIL_HEIGHT)
TOKEN_DEF(INDICATOR_X)
TOKEN_DEF(INDICATOR_Y)
TOKEN_DEF(INDICATOR_WIDTH)
TOKEN_DEF(INDICATOR_HEIGHT)
TOKEN_DEF(INDICATOR_COLOR)
TOKEN_DEF(SAVE_IMAGE_X)
TOKEN_DEF(SAVE_IMAGE_Y)
TOKEN_DEF(SAVE_IMAGE)
TOKEN_DEF(LOAD_IMAGE_X)
TOKEN_DEF(LOAD_IMAGE_Y)
TOKEN_DEF(LOAD_IMAGE)
TOKEN_DEF(LOCAL_SAVE_DIR)
TOKEN_DEF(RICH_SAVED_GAMES)
TOKEN_DEF(SAVED_GAME_EXT)
TOKEN_DEF(GUID)
TOKEN_DEF(COMPAT_KILL_METHOD_THREADS)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool BaseGame::loadBuffer(char *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(GAME)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(SYSTEM_FONT)
	TOKEN_TABLE(VIDEO_FONT)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(ACTIVE_CURSOR)
	TOKEN_TABLE(NONINTERACTIVE_CURSOR)
	TOKEN_TABLE(PERSONAL_SAVEGAMES)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(SUBTITLES_SPEED)
	TOKEN_TABLE(SUBTITLES)
	TOKEN_TABLE(VIDEO_SUBTITLES)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(THUMBNAIL_WIDTH)
	TOKEN_TABLE(THUMBNAIL_HEIGHT)
	TOKEN_TABLE(INDICATOR_X)
	TOKEN_TABLE(INDICATOR_Y)
	TOKEN_TABLE(INDICATOR_WIDTH)
	TOKEN_TABLE(INDICATOR_HEIGHT)
	TOKEN_TABLE(INDICATOR_COLOR)
	TOKEN_TABLE(SAVE_IMAGE_X)
	TOKEN_TABLE(SAVE_IMAGE_Y)
	TOKEN_TABLE(SAVE_IMAGE)
	TOKEN_TABLE(LOAD_IMAGE_X)
	TOKEN_TABLE(LOAD_IMAGE_Y)
	TOKEN_TABLE(LOAD_IMAGE)
	TOKEN_TABLE(LOCAL_SAVE_DIR)
	TOKEN_TABLE(COMPAT_KILL_METHOD_THREADS)
	TOKEN_TABLE_END

	// Declare a few variables necessary for moving data from these settings over to the renderer:
	// The values are the same as the defaults set in BaseRenderer.
	int loadImageX = 0;
	int loadImageY = 0;
	int saveImageX = 0;
	int saveImageY = 0;
	int indicatorX = -1;
	int indicatorY = -1;
	int indicatorWidth = -1;
	int indicatorHeight = 8;
	uint32 indicatorColor = BYTETORGBA(255, 0, 0, 128);
	Common::String loadImageName = "";
	Common::String saveImageName = "";

	char *params;
	int cmd;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand(&buffer, commands, &params) != TOKEN_GAME) {
			_gameRef->LOG(0, "'GAME' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile(params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NAME:
			setName(params);
			break;

		case TOKEN_CAPTION:
			setCaption(params);
			break;

		case TOKEN_SYSTEM_FONT:
			if (_systemFont) {
				_fontStorage->removeFont(_systemFont);
			}
			_systemFont = nullptr;

			_systemFont = _gameRef->_fontStorage->addFont(params);
			break;

		case TOKEN_VIDEO_FONT:
			if (_videoFont) {
				_fontStorage->removeFont(_videoFont);
			}
			_videoFont = nullptr;

			_videoFont = _gameRef->_fontStorage->addFont(params);
			break;


		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new BaseSprite(_gameRef);
			if (!_cursor || DID_FAIL(_cursor->loadFile(params))) {
				delete _cursor;
				_cursor = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_ACTIVE_CURSOR:
			delete _activeCursor;
			_activeCursor = nullptr;
			_activeCursor = new BaseSprite(_gameRef);
			if (!_activeCursor || DID_FAIL(_activeCursor->loadFile(params))) {
				delete _activeCursor;
				_activeCursor = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NONINTERACTIVE_CURSOR:
			delete _cursorNoninteractive;
			_cursorNoninteractive = new BaseSprite(_gameRef);
			if (!_cursorNoninteractive || DID_FAIL(_cursorNoninteractive->loadFile(params))) {
				delete _cursorNoninteractive;
				_cursorNoninteractive = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_SCRIPT:
			addScript(params);
			break;

		case TOKEN_PERSONAL_SAVEGAMES:
			parser.scanStr(params, "%b", &_personalizedSave);
			break;

		case TOKEN_SUBTITLES:
			parser.scanStr(params, "%b", &_subtitles);
			break;

		case TOKEN_SUBTITLES_SPEED:
			parser.scanStr(params, "%d", &_subtitlesSpeed);
			break;

		case TOKEN_VIDEO_SUBTITLES:
			parser.scanStr(params, "%b", &_videoSubtitles);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		case TOKEN_THUMBNAIL_WIDTH:
			parser.scanStr(params, "%d", &_thumbnailWidth);
			break;

		case TOKEN_THUMBNAIL_HEIGHT:
			parser.scanStr(params, "%d", &_thumbnailHeight);
			break;

		case TOKEN_INDICATOR_X:
			parser.scanStr(params, "%d", &indicatorX);
			break;

		case TOKEN_INDICATOR_Y:
			parser.scanStr(params, "%d", &indicatorY);
			break;

		case TOKEN_INDICATOR_COLOR: {
			int r, g, b, a;
			parser.scanStr(params, "%d,%d,%d,%d", &r, &g, &b, &a);
			indicatorColor = BYTETORGBA(r, g, b, a);
		}
		break;

		case TOKEN_INDICATOR_WIDTH:
			parser.scanStr(params, "%d", &indicatorWidth);
			break;

		case TOKEN_INDICATOR_HEIGHT:
			parser.scanStr(params, "%d", &indicatorHeight);
			break;

		case TOKEN_SAVE_IMAGE:
			saveImageName = params;
			break;

		case TOKEN_SAVE_IMAGE_X:
			parser.scanStr(params, "%d", &saveImageX);
			break;

		case TOKEN_SAVE_IMAGE_Y:
			parser.scanStr(params, "%d", &saveImageY);
			break;

		case TOKEN_LOAD_IMAGE:
			loadImageName = params;
			break;

		case TOKEN_LOAD_IMAGE_X:
			parser.scanStr(params, "%d", &loadImageX);
			break;

		case TOKEN_LOAD_IMAGE_Y:
			parser.scanStr(params, "%d", &loadImageY);
			break;

		case TOKEN_LOCAL_SAVE_DIR:
			_localSaveDir = params;
			break;

		case TOKEN_COMPAT_KILL_METHOD_THREADS:
			parser.scanStr(params, "%b", &_compatKillMethodThreads);
			break;

		default:
			break;
		}
	}

	_renderer->setIndicator(indicatorWidth, indicatorHeight, indicatorX, indicatorY, indicatorColor);
	_renderer->initIndicator(); // In case we just reset the values.
	_renderer->setSaveImage(saveImageName.c_str(), saveImageX, saveImageY);
	_renderer->setLoadingScreen(loadImageName.c_str(), loadImageX, loadImageY);

	if (!_systemFont) {
		_systemFont = _gameRef->_fontStorage->addFont("system_font.fnt");
	}


	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in GAME definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading GAME definition");
		return STATUS_FAILED;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool BaseGame::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// LOG
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "LOG") == 0) {
		stack->correctParams(1);
		LOG(0, stack->pop()->getString());
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Caption") == 0) {
		bool res = BaseObject::scCallMethod(script, stack, thisStack, name);
		setWindowTitle();
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	// Msg
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Msg") == 0) {
		stack->correctParams(1);
		quickMessage(stack->pop()->getString());
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RunScript
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RunScript") == 0) {
		_gameRef->LOG(0, "**Warning** The 'RunScript' method is now obsolete. Use 'AttachScript' instead (same syntax)");
		stack->correctParams(1);
		if (DID_FAIL(addScript(stack->pop()->getString()))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadStringTable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadStringTable") == 0) {
		stack->correctParams(2);
		const char *filename = stack->pop()->getString();
		ScValue *val = stack->pop();

		bool clearOld;
		if (val->isNULL()) {
			clearOld = true;
		} else {
			clearOld = val->getBool();
		}

		if (DID_FAIL(_settings->loadStringTable(filename, clearOld))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ValidObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ValidObject") == 0) {
		stack->correctParams(1);
		BaseScriptable *obj = stack->pop()->getNative();
		if (validObject((BaseObject *) obj)) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Reset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Reset") == 0) {
		stack->correctParams(0);
		resetContent();
		stack->pushNULL();

		return STATUS_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// UnloadObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadObject") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		BaseObject *obj = (BaseObject *)val->getNative();
		unregisterObject(obj);
		if (val->getType() == VAL_VARIABLE_REF) {
			val->setNULL();
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadWindow") == 0) {
		stack->correctParams(1);
		UIWindow *win = new UIWindow(_gameRef);
		if (win && DID_SUCCEED(win->loadFile(stack->pop()->getString()))) {
			_windows.add(win);
			registerObject(win);
			stack->pushNative(win, true);
		} else {
			delete win;
			win = nullptr;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ExpandString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ExpandString") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		char *str = new char[strlen(val->getString()) + 1];
		strcpy(str, val->getString());
		expandStringByStringTable(&str);
		stack->pushString(str);
		delete[] str;
		return STATUS_OK;
	}

	else if (_musicSystem->scCallMethod(script, stack, thisStack, name) == STATUS_OK) {
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetMousePos
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetMousePos") == 0) {
		stack->correctParams(2);
		int32 x = stack->pop()->getInt();
		int32 y = stack->pop()->getInt();
		x = MAX<int32>(x, 0);
		x = MIN(x, _renderer->getWidth());
		y = MAX<int32>(y, 0);
		y = MIN(y, _renderer->getHeight());
		Point32 p;
		p.x = x + _renderer->_drawOffsetX;
		p.y = y + _renderer->_drawOffsetY;

		BasePlatform::setCursorPos(p.x, p.y);

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LockMouseRect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LockMouseRect") == 0) {
		stack->correctParams(4);
		int left = stack->pop()->getInt();
		int top = stack->pop()->getInt();
		int right = stack->pop()->getInt();
		int bottom = stack->pop()->getInt();

		if (right < left) {
			BaseUtils::swap(&left, &right);
		}
		if (bottom < top) {
			BaseUtils::swap(&top, &bottom);
		}

		_mouseLockRect.setRect(left, top, right, bottom);

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayVideo
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlayVideo") == 0) {
		_gameRef->LOG(0, "Warning: Game.PlayVideo() is now deprecated. Use Game.PlayTheora() instead.");

		stack->correctParams(6);
		const char *filename = stack->pop()->getString();
		warning("PlayVideo: %s - not implemented yet", filename);
		ScValue *valType = stack->pop();
		int type;
		if (valType->isNULL()) {
			type = (int)VID_PLAY_STRETCH;
		} else {
			type = valType->getInt();
		}

		int xVal = stack->pop()->getInt();
		int yVal = stack->pop()->getInt();
		bool freezeMusic = stack->pop()->getBool(true);

		ScValue *valSub = stack->pop();
		const char *subtitleFile = valSub->isNULL() ? nullptr : valSub->getString();

		if (type < (int)VID_PLAY_POS || type > (int)VID_PLAY_CENTER) {
			type = (int)VID_PLAY_STRETCH;
		}

		if (DID_SUCCEED(_gameRef->_videoPlayer->initialize(filename, subtitleFile))) {
			if (DID_SUCCEED(_gameRef->_videoPlayer->play((TVideoPlayback)type, xVal, yVal, freezeMusic))) {
				stack->pushBool(true);
				script->sleep(0);
			} else {
				stack->pushBool(false);
			}
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlayTheora") == 0) {
		stack->correctParams(7);
		const char *filename = stack->pop()->getString();
		ScValue *valType = stack->pop();
		int type;
		if (valType->isNULL()) {
			type = (int)VID_PLAY_STRETCH;
		} else {
			type = valType->getInt();
		}

		int xVal = stack->pop()->getInt();
		int yVal = stack->pop()->getInt();
		bool freezeMusic = stack->pop()->getBool(true);
		bool dropFrames = stack->pop()->getBool(true);

		ScValue *valSub = stack->pop();
		const char *subtitleFile = valSub->isNULL() ? nullptr : valSub->getString();

		if (type < (int)VID_PLAY_POS || type > (int)VID_PLAY_CENTER) {
			type = (int)VID_PLAY_STRETCH;
		}

		delete _theoraPlayer;
		_theoraPlayer = new VideoTheoraPlayer(this);
		if (_theoraPlayer && DID_SUCCEED(_theoraPlayer->initialize(filename, subtitleFile))) {
			_theoraPlayer->_dontDropFrames = !dropFrames;
			if (DID_SUCCEED(_theoraPlayer->play((TVideoPlayback)type, xVal, yVal, true, freezeMusic))) {
				stack->pushBool(true);
				script->sleep(0);
			} else {
				stack->pushBool(false);
			}
		} else {
			stack->pushBool(false);
			delete _theoraPlayer;
			_theoraPlayer = nullptr;
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// QuitGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "QuitGame") == 0) {
		stack->correctParams(0);
		stack->pushNULL();
		_quitting = true;
		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] RegistryFlush
	// Return value is never used
	// Used at SaveGameSettings() and Game.RegistryFlush()
	// Called after a series of RegWriteNumber calls
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RegistryFlush") == 0) {
		stack->correctParams(0);
		ConfMan.flushToDisk();
		stack->pushNULL();
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// RegWriteNumber
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RegWriteNumber") == 0) {
		stack->correctParams(2);
		const char *key = stack->pop()->getString();
		int val = stack->pop()->getInt();
		Common::String privKey = "priv_" + StringUtil::encodeSetting(key);
		ConfMan.setInt(privKey, val);
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegReadNumber
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RegReadNumber") == 0) {
		stack->correctParams(2);
		const char *key = stack->pop()->getString();
		int initVal = stack->pop()->getInt();
		Common::String privKey = "priv_" + StringUtil::encodeSetting(key);
		int result = initVal;
		if (ConfMan.hasKey(privKey)) {
			result = ConfMan.getInt(privKey);
		}
		stack->pushInt(result);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegWriteString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RegWriteString") == 0) {
		stack->correctParams(2);
		const char *key = stack->pop()->getString();
		const char *val = stack->pop()->getString();
		Common::String privKey = "wme_" + StringUtil::encodeSetting(key);
		Common::String privVal = StringUtil::encodeSetting(val);
		ConfMan.set(privKey, privVal);
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegReadString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RegReadString") == 0) {
		stack->correctParams(2);
		const char *key = stack->pop()->getString();
		const char *initVal = stack->pop()->getString();
		Common::String result = readRegistryString(key, initVal);
		stack->pushString(result.c_str());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SaveGame") == 0) {
		stack->correctParams(3);
		int slot = stack->pop()->getInt();
		const char *xdesc = stack->pop()->getString();
		bool quick = stack->pop()->getBool(false);

		char *desc = new char[strlen(xdesc) + 1];
		strcpy(desc, xdesc);
		stack->pushBool(true);
		if (DID_FAIL(saveGame(slot, desc, quick))) {
			stack->pop();
			stack->pushBool(false);
		}
		delete[] desc;
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadGame") == 0) {
		stack->correctParams(1);
		_scheduledLoadSlot = stack->pop()->getInt();
		_loading = true;
		stack->pushBool(false);
		script->sleep(0);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsSaveSlotUsed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsSaveSlotUsed") == 0) {
		stack->correctParams(1);
		int slot = stack->pop()->getInt();
		stack->pushBool(SaveLoad::isSaveSlotUsed(slot));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSaveSlotDescription
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSaveSlotDescription") == 0) {
		stack->correctParams(1);
		int slot = stack->pop()->getInt();
		Common::String desc = SaveLoad::getSaveSlotDescription(slot);
		stack->pushString(desc.c_str());
		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] GetSaveSlotDescriptionTimestamp
	// Return struct with "Description" and "Timestamp" fields in 1.2.362-
	// Return array  with "Description" and "Timestamp" items  in 1.2.527+
	// Timestamps should be comparable types
	// Used to sort saved games by timestamps at save.script & load.script
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSaveSlotDescriptionTimestamp") == 0) {
		stack->correctParams(1);
		int slot = stack->pop()->getInt();

		TimeDate time;
		SaveLoad::getSaveSlotTimestamp(slot, &time);
		stack->pushInt(time.tm_sec);
		stack->pushInt(time.tm_min);
		stack->pushInt(time.tm_hour);
		stack->pushInt(time.tm_mday);
		stack->pushInt(time.tm_mon + 1);
		stack->pushInt(time.tm_year + 1900);
		stack->pushInt(6);
		BaseScriptable *date = makeSXDate(_gameRef, stack);
		stack->pushNative(date, false);

		Common::String desc = SaveLoad::getSaveSlotDescription(slot);
		stack->pushString(desc.c_str());

		BaseScriptable *obj;
		if (BaseEngine::instance().isFoxTail(FOXTAIL_1_2_527, FOXTAIL_LATEST_VERSION)) {
			stack->pushInt(2);
			obj = makeSXArray(_gameRef, stack);
		} else {
			stack->pushInt(0);
			obj = makeSXObject(_gameRef, stack);
			obj->scSetProperty("Description", stack->pop());
			obj->scSetProperty("Timestamp", stack->pop());
		}
		stack->pushNative(obj, false);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] ValidSaveSlotVersion
	// Checks if given slot stores game state of compatible game version
	// This version always returs true
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ValidSaveSlotVersion") == 0) {
		stack->correctParams(1);
		/* int slot = */ stack->pop()->getInt();
		// do nothing
		stack->pushBool(true);
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// EmptySaveSlot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EmptySaveSlot") == 0) {
		stack->correctParams(1);
		int slot = stack->pop()->getInt();
		SaveLoad::emptySaveSlot(slot);
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalSFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetGlobalSFXVolume") == 0) {
		stack->correctParams(1);
		_gameRef->_soundMgr->setVolumePercent(Audio::Mixer::kSFXSoundType, (byte)stack->pop()->getInt());
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalSpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetGlobalSpeechVolume") == 0) {
		stack->correctParams(1);
		_gameRef->_soundMgr->setVolumePercent(Audio::Mixer::kSpeechSoundType, (byte)stack->pop()->getInt());
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalMusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetGlobalMusicVolume") == 0) {
		stack->correctParams(1);
		_gameRef->_soundMgr->setVolumePercent(Audio::Mixer::kMusicSoundType, (byte)stack->pop()->getInt());
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalMasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetGlobalMasterVolume") == 0) {
		stack->correctParams(1);
		_gameRef->_soundMgr->setMasterVolumePercent((byte)stack->pop()->getInt());
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalSFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetGlobalSFXVolume") == 0) {
		stack->correctParams(0);
		stack->pushInt(_soundMgr->getVolumePercent(Audio::Mixer::kSFXSoundType));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalSpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetGlobalSpeechVolume") == 0) {
		stack->correctParams(0);
		stack->pushInt(_soundMgr->getVolumePercent(Audio::Mixer::kSpeechSoundType));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalMusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetGlobalMusicVolume") == 0) {
		stack->correctParams(0);
		stack->pushInt(_soundMgr->getVolumePercent(Audio::Mixer::kMusicSoundType));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalMasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetGlobalMasterVolume") == 0) {
		stack->correctParams(0);
		stack->pushInt(_soundMgr->getMasterVolumePercent());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetActiveCursor") == 0) {
		stack->correctParams(1);
		if (DID_SUCCEED(setActiveCursor(stack->pop()->getString()))) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetActiveCursor") == 0) {
		stack->correctParams(0);
		if (!_activeCursor || !_activeCursor->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_activeCursor->getFilename());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetActiveCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetActiveCursorObject") == 0) {
		stack->correctParams(0);
		if (!_activeCursor) {
			stack->pushNULL();
		} else {
			stack->pushNative(_activeCursor, true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveActiveCursor") == 0) {
		stack->correctParams(0);
		delete _activeCursor;
		_activeCursor = nullptr;
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HasActiveCursor") == 0) {
		stack->correctParams(0);

		if (_activeCursor) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FileExists
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FileExists") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();

		bool exists = BaseFileManager::getEngineInstance()->hasFile(filename); // Had absPathWarning = false
		stack->pushBool(exists);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeOut / FadeOutAsync / SystemFadeOut / SystemFadeOutAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeOut") == 0 || strcmp(name, "FadeOutAsync") == 0 || strcmp(name, "SystemFadeOut") == 0 || strcmp(name, "SystemFadeOutAsync") == 0) {
		stack->correctParams(5);
		uint32 duration = stack->pop()->getInt(500);
		byte red = stack->pop()->getInt(0);
		byte green = stack->pop()->getInt(0);
		byte blue = stack->pop()->getInt(0);
		byte alpha = stack->pop()->getInt(0xFF);

		// HACK: Corrosion fades screen to black while opening main menu
		// Thus, we get black screenshots when saving game from in-game menus
		// Let's take & keep screenshot before entering main menu
		if (duration == 750 && BaseEngine::instance().getGameId() == "corrosion") {
			storeSaveThumbnail();
		}

		bool system = (strcmp(name, "SystemFadeOut") == 0 || strcmp(name, "SystemFadeOutAsync") == 0);

		_fader->fadeOut(BYTETORGBA(red, green, blue, alpha), duration, system);
		if (strcmp(name, "FadeOutAsync") != 0 && strcmp(name, "SystemFadeOutAsync") != 0) {
			script->waitFor(_fader);
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeIn / FadeInAsync / SystemFadeIn / SystemFadeInAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeIn") == 0 || strcmp(name, "FadeInAsync") == 0 || strcmp(name, "SystemFadeIn") == 0 || strcmp(name, "SystemFadeInAsync") == 0) {
		stack->correctParams(5);
		uint32 duration = stack->pop()->getInt(500);
		byte red = stack->pop()->getInt(0);
		byte green = stack->pop()->getInt(0);
		byte blue = stack->pop()->getInt(0);
		byte alpha = stack->pop()->getInt(0xFF);

		bool system = (strcmp(name, "SystemFadeIn") == 0 || strcmp(name, "SystemFadeInAsync") == 0);

		_fader->fadeIn(BYTETORGBA(red, green, blue, alpha), duration, system);
		if (strcmp(name, "FadeInAsync") != 0 && strcmp(name, "SystemFadeInAsync") != 0) {
			script->waitFor(_fader);
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFadeColor") == 0) {
		stack->correctParams(0);
		stack->pushInt(_fader->getCurrentColor());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Screenshot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Screenshot") == 0) {
		stack->correctParams(1);
		char filename[MAX_PATH_LENGTH];

		ScValue *val = stack->pop();

		warning("BGame::ScCallMethod - Screenshot not reimplemented"); //TODO
		int fileNum = 0;

		while (true) {
			sprintf(filename, "%s%03d.bmp", val->isNULL() ? getName() : val->getString(), fileNum);
			if (!Common::File::exists(filename)) {
				break;
			}
			fileNum++;
		}

		bool ret = _gameRef->_renderer->saveScreenShot(filename);

		stack->pushBool(ret);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScreenshotEx
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScreenshotEx") == 0) {
		stack->correctParams(3);
		const char *filename = stack->pop()->getString();
		int sizeX = stack->pop()->getInt(_renderer->getWidth());
		int sizeY = stack->pop()->getInt(_renderer->getHeight());

		bool ret = _gameRef->_renderer->saveScreenShot(filename, sizeX, sizeY);

		stack->pushBool(ret);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateWindow") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		UIWindow *win = new UIWindow(_gameRef);
		_windows.add(win);
		registerObject(win);
		if (!val->isNULL()) {
			win->setName(val->getString());
		}
		stack->pushNative(win, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteWindow") == 0) {
		stack->correctParams(1);
		BaseObject *obj = (BaseObject *)stack->pop()->getNative();
		for (uint32 i = 0; i < _windows.size(); i++) {
			if (_windows[i] == obj) {
				unregisterObject(_windows[i]);
				stack->pushBool(true);
				return STATUS_OK;
			}
		}
		stack->pushBool(false);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OpenDocument
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OpenDocument") == 0) {
		stack->correctParams(1);
		g_system->openUrl(stack->pop()->getString());
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DEBUG_DumpClassRegistry
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DEBUG_DumpClassRegistry") == 0) {
		stack->correctParams(0);
		DEBUG_DumpClassRegistry();
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetLoadingScreen
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetLoadingScreen") == 0) {
		stack->correctParams(3);
		ScValue *val = stack->pop();
		int loadImageX = stack->pop()->getInt();
		int loadImageY = stack->pop()->getInt();

		if (val->isNULL()) {
			_renderer->setLoadingScreen(NULL, loadImageX, loadImageY);
		} else {
			_renderer->setLoadingScreen(val->getString(), loadImageX, loadImageY);
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSavingScreen
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetSavingScreen") == 0) {
		stack->correctParams(3);
		ScValue *val = stack->pop();
		int saveImageX = stack->pop()->getInt();
		int saveImageY = stack->pop()->getInt();

		if (val->isNULL()) {
			_renderer->setSaveImage(NULL, saveImageX, saveImageY);
		} else {
			_renderer->setSaveImage(NULL, saveImageX, saveImageY);
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetWaitCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetWaitCursor") == 0) {
		stack->correctParams(1);
		if (DID_SUCCEED(setWaitCursor(stack->pop()->getString()))) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveWaitCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveWaitCursor") == 0) {
		stack->correctParams(0);
		delete _cursorNoninteractive;
		_cursorNoninteractive = nullptr;

		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetWaitCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetWaitCursor") == 0) {
		stack->correctParams(0);
		if (!_cursorNoninteractive || !_cursorNoninteractive->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_cursorNoninteractive->getFilename());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetWaitCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetWaitCursorObject") == 0) {
		stack->correctParams(0);
		if (!_cursorNoninteractive) {
			stack->pushNULL();
		} else {
			stack->pushNative(_cursorNoninteractive, true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClearScriptCache
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ClearScriptCache") == 0) {
		stack->correctParams(0);
		stack->pushBool(DID_SUCCEED(_scEngine->emptyScriptCache()));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisplayLoadingIcon
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisplayLoadingIcon") == 0) {
		stack->correctParams(4);

		const char *filename = stack->pop()->getString();
		_loadingIconX = stack->pop()->getInt();
		_loadingIconY = stack->pop()->getInt();
		_loadingIconPersistent = stack->pop()->getBool();

		delete _loadingIcon;
		_loadingIcon = new BaseSprite(this);
		if (!_loadingIcon || DID_FAIL(_loadingIcon->loadFile(filename))) {
			delete _loadingIcon;
			_loadingIcon = nullptr;
		} else {
			displayContent(false, true);
			_gameRef->_renderer->flip();
			_gameRef->_renderer->initLoop();
		}
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HideLoadingIcon
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HideLoadingIcon") == 0) {
		stack->correctParams(0);
		delete _loadingIcon;
		_loadingIcon = nullptr;
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DumpTextureStats
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DumpTextureStats") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();

		_renderer->dumpData(filename);

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccOutputText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccOutputText") == 0) {
		stack->correctParams(2);
		/* const char *str = */	stack->pop()->getString();
		/* int type = */ stack->pop()->getInt();
		// do nothing
		stack->pushNULL();

		return STATUS_OK;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// IsShadowTypeSupported
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsShadowTypeSupported") == 0) {
		stack->correctParams(1);
		TShadowType type = static_cast<TShadowType>(stack->pop()->getInt());

		switch (type) {
		case SHADOW_NONE:
		case SHADOW_SIMPLE:
			stack->pushBool(true);
			break;

		case SHADOW_FLAT:
			stack->pushBool(_supportsRealTimeShadows);
			break;

		case SHADOW_STENCIL:
			stack->pushBool(_gameRef->_renderer3D->stencilSupported());
			break;

		default:
			stack->pushBool(false);
		}

		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// StoreSaveThumbnail
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StoreSaveThumbnail") == 0) {
		stack->correctParams(0);
		stack->pushBool(storeSaveThumbnail());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteSaveThumbnail
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteSaveThumbnail") == 0) {
		stack->correctParams(0);
		deleteSaveThumbnail();
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFileChecksum
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFileChecksum") == 0) {
		stack->correctParams(2);
		const char *filename = stack->pop()->getString();
		bool asHex = stack->pop()->getBool(false);

		Common::SeekableReadStream *file = BaseFileManager::getEngineInstance()->openFile(filename, false);
		if (file) {
			crc remainder = crc_initialize();
			byte buf[1024];
			int bytesRead = 0;

			while (bytesRead < file->size()) {
				int bufSize = MIN((uint32)1024, (uint32)(file->size() - bytesRead));
				bytesRead += file->read(buf, bufSize);

				for (int i = 0; i < bufSize; i++) {
					remainder = crc_process_byte(buf[i], remainder);
				}
			}
			crc checksum = crc_finalize(remainder);

			if (asHex) {
				char hex[100];
				sprintf(hex, "%x", checksum);
				stack->pushString(hex);
			} else {
				stack->pushInt(checksum);
			}

			BaseFileManager::getEngineInstance()->closeFile(file);
			file = nullptr;
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

#ifdef ENABLE_HEROCRAFT
	//////////////////////////////////////////////////////////////////////////
	// [HeroCraft] GetSpriteControl
	// Returns some internal state
	// Known return values are:
	// * 44332211: MUST be returned at "game.script" to allow game start
	// * 77885566: may be returned at "mainMenu.script" to force open registration window
	// * 90123679: may be returned at "mainMenu.script" to make "Buy Game" button visible
	// Used at "Pole Chudes" only
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSpriteControl") == 0) {
		stack->correctParams(0);
		stack->pushInt(44332211L);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [HeroCraft] RandomInitSeed
	// Additional method to be called before RandomSeed()
	// Used at "Pole Chudes" only
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RandomInitSeed") == 0) {
		stack->correctParams(1);
		int seed = stack->pop()->getInt();

		_rndHc->setSeed(seed);

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [HeroCraft] RandomSeed
	// Similar to usual Random() function, but using seed provided earlier
	// Used at "Pole Chudes" only
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RandomSeed") == 0) {
		stack->correctParams(2);

		int from = stack->pop()->getInt();
		int to   = stack->pop()->getInt();
		int rnd  = _rndHc->getRandomNumberRng(from, to);

		stack->pushInt(rnd);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [HeroCraft] GetImageInfo
	// Returns image size in "<width>;<height>" format, e.g. "800;600"
	// Known params: "fsdata\\splash1.jpg"
	// Game script turn off scaling if returned value is "1024;768"
	// Used at "Papa's Daughters 1" only
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetImageInfo") == 0) {
		stack->correctParams(1);
		/*const char *filename =*/ stack->pop()->getString();
		stack->pushString("1024;768");
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [HeroCraft] A lot of functions used for self-check
	// Used at "Papa's Daughters 2" only
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteItems") == 0 || strcmp(name, "CreateActorItems") == 0 || strcmp(name, "DeleteActorItems") == 0 || strcmp(name, "PrepareItems") == 0 || strcmp(name, "CreateEntityItems") == 0 || strcmp(name, "DeleteEntityItems") == 0 || strcmp(name, "PrepareItemsWin") == 0 || strcmp(name, "CreateItems") == 0) {
		stack->correctParams(3);
		uint32 a = (uint32)stack->pop()->getInt();
		uint32 b = (uint32)stack->pop()->getInt();
		uint32 c = (uint32)stack->pop()->getInt();

		uint32 result = 0;
		const char* fname = "PapasDaughters2.wrp.exe";
		if (strcmp(name, "PrepareItems") == 0 || strcmp(name, "CreateEntityItems") == 0 || strcmp(name, "DeleteEntityItems") == 0) {
			result = getFilePartChecksumHc(fname, b, a);
		} else if (strcmp(name, "PrepareItemsWin") == 0) {
			result = getFilePartChecksumHc(fname, b, c);
		} else if (strcmp(name, "CreateItems") == 0) {
			result = getFilePartChecksumHc(fname, a, c);
		} else {
			result = getFilePartChecksumHc(fname, a, b);
		}

		stack->pushInt(result);
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// EnableScriptProfiling
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EnableScriptProfiling") == 0) {
		stack->correctParams(0);
		_scEngine->enableProfiling();
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisableScriptProfiling
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisableScriptProfiling") == 0) {
		stack->correctParams(0);
		_scEngine->disableProfiling();
		stack->pushNULL();

		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] GetScreenType
	// Returns 0 on fullscreen and 1 on window
	// Used to init and update controls at options.script and methods.script
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetScreenType") == 0) {
		stack->correctParams(0);
		int type = _renderer->isWindowed() ? 1 : 0;
		stack->pushInt(type);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] GetScreenMode
	// Returns integer to be used as a pixelization mode multiplier
	// (e.g. it returns 2 for 640x360, 3 for 960x540, etc...)
	// Used to init and update controls at options.script and methods.script
	// This implementation always return 2 to fake window size of 2*320 x 2*180
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetScreenMode") == 0) {
		stack->correctParams(0);
		stack->pushInt(2);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] GetDesktopDisplayMode
	// Return struct with "w" and "h" fields in 1.2.362-
	// Return array  with "w" and "h" items  in 1.2.527+
	// Used to init and update controls at options.script and methods.script
	// w,h of actual desktop size expected to calcucate maximum available size
	// Available screen modes are calcucated as 2...N, N*320<w and N*180<h
	// This implementation fakes available size as 2*320 x 2*180 only
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetDesktopDisplayMode") == 0) {
		stack->correctParams(0);
		stack->pushInt(2 * 180 + 1);
		stack->pushInt(2 * 320 + 1);

		BaseScriptable *obj;
		if (BaseEngine::instance().isFoxTail(FOXTAIL_1_2_527, FOXTAIL_LATEST_VERSION)) {
			stack->pushInt(2);
			obj = makeSXArray(_gameRef, stack);
		} else {
			stack->pushInt(0);
			obj = makeSXObject(_gameRef, stack);
			obj->scSetProperty("w", stack->pop());
			obj->scSetProperty("h", stack->pop());
		}
		stack->pushNative(obj, false);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] SetScreenTypeMode
	// This implementation ignores mode, toggles screen type only
	// Used to change screen type&mode at options.script and methods.script
	// Return value is never used
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetScreenTypeMode") == 0) {
		stack->correctParams(2);
		int type = stack->pop()->getInt();
		stack->pop()->getInt(); //mode is unused

		_renderer->setWindowed(type);
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] ChangeWindowGrab
	// Used at game.script on "Keypress" event on F11
	// Readme of FoxTail says: "F11 - free the mouse pointer from the window"
	// This implementation does nothing
	// Return value is never used
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ChangeWindowGrab") == 0) {
		stack->correctParams(0);
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] GetFiles
	// Used at kalimba.script on F9 keypress to reload list of available music
	// Known params: "*.mb"
	// Original implementation does not seem to look up at DCP packages
	// This implementation looks up at savegame storage and for actual files 
	// Return value expected to be an Array of Strings
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFiles") == 0) {
		stack->correctParams(1);
		const char *pattern = stack->pop()->getString();

		Common::StringArray fnames;
		BaseFileManager::getEngineInstance()->listMatchingFiles(fnames, pattern);
		
		stack->pushInt(0);
		BaseScriptable *arr = makeSXArray(_gameRef, stack);
		for (uint32 i = 0; i < fnames.size(); i++) {
			stack->pushString(fnames[i].c_str());
			((SXArray *)arr)->push(stack->pop());
		}

 		stack->pushNative(arr, false);
 		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// ShowStatusLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ShowStatusLine") == 0) {
		stack->correctParams(0);
		// Block kept to show intention of opcode.
		/*#ifdef __IPHONEOS__
		        IOS_ShowStatusLine(TRUE);
		#endif*/
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HideStatusLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HideStatusLine") == 0) {
		stack->correctParams(0);
		// Block kept to show intention of opcode.
		/*#ifdef __IPHONEOS__
		        IOS_ShowStatusLine(FALSE);
		#endif*/
		stack->pushNULL();

		return STATUS_OK;
	} else {
		return BaseObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *BaseGame::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("game");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Name") {
		_scValue->setString(getName());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Hwnd (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Hwnd") {
		_scValue->setInt((int)_renderer->_window);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CurrentTime (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "CurrentTime") {
		_scValue->setInt((int)getTimer()->getTime());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WindowsTime (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "WindowsTime") {
		_scValue->setInt((int)g_system->getMillis());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WindowedMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "WindowedMode") {
		_scValue->setBool(_renderer->isWindowed());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MouseX") {
		_scValue->setInt(_mousePos.x);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MouseY") {
		_scValue->setInt(_mousePos.y);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainObject
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MainObject") {
		_scValue->setNative(_mainObject, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ActiveObject (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ActiveObject") {
		_scValue->setNative(_activeObject, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScreenWidth (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ScreenWidth") {
		_scValue->setInt(_renderer->getWidth());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScreenHeight (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ScreenHeight") {
		_scValue->setInt(_renderer->getHeight());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Interactive") {
		_scValue->setBool(_interactive);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DebugMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "DebugMode") {
		_scValue->setBool(_debugDebugMode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundAvailable (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SoundAvailable") {
		_scValue->setBool(_soundMgr->_soundAvailable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SFXVolume") {
		_gameRef->LOG(0, "**Warning** The SFXVolume attribute is obsolete");
		_scValue->setInt(_soundMgr->getVolumePercent(Audio::Mixer::kSFXSoundType));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SpeechVolume") {
		_gameRef->LOG(0, "**Warning** The SpeechVolume attribute is obsolete");
		_scValue->setInt(_soundMgr->getVolumePercent(Audio::Mixer::kSpeechSoundType));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MusicVolume") {
		_gameRef->LOG(0, "**Warning** The MusicVolume attribute is obsolete");
		_scValue->setInt(_soundMgr->getVolumePercent(Audio::Mixer::kMusicSoundType));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MasterVolume") {
		_gameRef->LOG(0, "**Warning** The MasterVolume attribute is obsolete");
		_scValue->setInt(_soundMgr->getMasterVolumePercent());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyboard (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Keyboard") {
		if (_keyboardState) {
			_scValue->setNative(_keyboardState, true);
		} else {
			_scValue->setNULL();
		}

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Subtitles
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Subtitles") {
		_scValue->setBool(_subtitles);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesSpeed
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SubtitlesSpeed") {
		_scValue->setInt(_subtitlesSpeed);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// VideoSubtitles
	//////////////////////////////////////////////////////////////////////////
	else if (name == "VideoSubtitles") {
		_scValue->setBool(_videoSubtitles);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FPS (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "FPS") {
		_scValue->setInt(_fps);
		return _scValue;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// Shadows (obsolete)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Shadows") {
		_scValue->setBool(_maxShadowType > SHADOW_NONE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SimpleShadows (obsolete)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SimpleShadows") {
		_scValue->setBool(_maxShadowType == SHADOW_SIMPLE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SupportsRealTimeShadows (obsolete)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SupportsRealTimeShadows") {
		_renderer3D->enableShadows();
		_scValue->setBool(_supportsRealTimeShadows);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxShadowType
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MaxShadowType") {
		_scValue->setInt(_maxShadowType);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxActiveLights
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MaxActiveLights") {
		if (_useD3D) {
			_scValue->setInt(_renderer3D->maximumLightsCount());
		} else {
			_scValue->setInt(0);
		}

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Direct3DDevice
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Direct3DDevice") {
		warning("BaseGame::scGetProperty Direct3D device is not available");
		_scValue->setNULL();

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DirectDrawInterface
	//////////////////////////////////////////////////////////////////////////
	else if (name == "DirectDrawInterface") {
		warning("BaseGame::scGetProperty DirectDraw interface is not available");
		_scValue->setNULL();

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// HardwareTL
	//////////////////////////////////////////////////////////////////////////
	else if (name == "HardwareTL") {
		// TODO: Once we have a TinyGL renderer, we could potentially return false here
		// otherwise, as long as WME3D is enabled, vertex processing is done by the hardware
		_scValue->setBool(true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// UsedMemory
	//////////////////////////////////////////////////////////////////////////
	else if (name == "UsedMemory") {
		// wme only returns a non-zero value in debug mode
		_scValue->setInt(0);
		return _scValue;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// AcceleratedMode / Accelerated (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AcceleratedMode" || name == "Accelerated") {
		_scValue->setBool(_useD3D);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextEncoding
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TextEncoding") {
		_scValue->setInt(_textEncoding);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextRTL
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TextRTL") {
		_scValue->setBool(_textRTL);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundBufferSize
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SoundBufferSize") {
		_scValue->setInt(_soundBufferSizeSec);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuspendedRendering
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SuspendedRendering") {
		_scValue->setBool(_suspendedRendering);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuppressScriptErrors
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SuppressScriptErrors") {
		_scValue->setBool(_suppressScriptErrors);
		return _scValue;
	}


	//////////////////////////////////////////////////////////////////////////
	// Frozen
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Frozen") {
		_scValue->setBool(_state == GAME_FROZEN);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSEnabled
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AccTTSEnabled") {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSTalk
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AccTTSTalk") {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSCaptions
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AccTTSCaptions") {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSKeypress
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AccTTSKeypress") {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccKeyboardEnabled
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AccKeyboardEnabled") {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccKeyboardCursorSkip
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AccKeyboardCursorSkip") {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccKeyboardPause
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AccKeyboardPause") {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutorunDisabled
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AutorunDisabled") {
		_scValue->setBool(_autorunDisabled);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveDirectory (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SaveDirectory") {
		AnsiString dataDir = "saves"; // See also: SXDirectory::scGetProperty("TempDirectory")
		_scValue->setString(dataDir.c_str());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveOnExit
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AutoSaveOnExit") {
		_scValue->setBool(_autoSaveOnExit);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveSlot
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AutoSaveSlot") {
		_scValue->setInt(_autoSaveSlot);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorHidden
	//////////////////////////////////////////////////////////////////////////
	else if (name == "CursorHidden") {
		_scValue->setBool(_cursorHidden);
		return _scValue;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] SystemLanguage (RO)
	// Returns Steam API language name string
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SystemLanguage") {
		switch (Common::parseLanguage(ConfMan.get("language"))) {
		case Common::CZ_CZE:
			_scValue->setString("czech");
			break;
		case Common::DA_DAN:
			_scValue->setString("danish");
			break;
		case Common::DE_DEU:
			_scValue->setString("german");
			break;
		case Common::ES_ESP:
			_scValue->setString("spanish");
			break;
		case Common::FI_FIN:
			_scValue->setString("finnish");
			break;
		case Common::FR_FRA:
			_scValue->setString("french");
			break;
		case Common::GR_GRE:
			_scValue->setString("greek");
			break;
		case Common::HU_HUN:
			_scValue->setString("hungarian");
			break;
		case Common::IT_ITA:
			_scValue->setString("italian");
			break;
		case Common::JA_JPN:
			_scValue->setString("japanese");
			break;
		case Common::KO_KOR:
			_scValue->setString("koreana");
			break;
		case Common::NB_NOR:
			_scValue->setString("norwegian");
			break;
		case Common::NL_NLD:
			_scValue->setString("dutch");
			break;
		case Common::PT_BRA:
			_scValue->setString("brazilian");
			break;
		case Common::PT_POR:
			_scValue->setString("portuguese");
			break;
		case Common::PL_POL:
			_scValue->setString("polish");
			break;
		case Common::RU_RUS:
			_scValue->setString("russian");
			break;
		case Common::SE_SWE:
			_scValue->setString("swedish");
			break;
		case Common::UA_UKR:
			_scValue->setString("ukrainian");
			break;
		case Common::ZH_CNA:
			_scValue->setString("schinese");
			break;
		case Common::ZH_TWN:
			_scValue->setString("tchinese");
			break;
		default:
			_scValue->setString("english");
			break;
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] BuildVersion (RO)
	// Used to display full game version at options.script in UpdateControls()
	// Returns FoxTail engine version number as a dotted string
	//////////////////////////////////////////////////////////////////////////
	else if (name == "BuildVersion") {
		if (BaseEngine::instance().getTargetExecutable() == FOXTAIL_1_2_227) {
			_scValue->setString("1.2.227");
		} else if (BaseEngine::instance().getTargetExecutable() == FOXTAIL_1_2_230) {
			_scValue->setString("1.2.230");
		} else if (BaseEngine::instance().getTargetExecutable() == FOXTAIL_1_2_304) {
			_scValue->setString("1.2.304");
		} else if (BaseEngine::instance().getTargetExecutable() == FOXTAIL_1_2_362) {
			_scValue->setString("1.2.362");
		} else if (BaseEngine::instance().getTargetExecutable() == FOXTAIL_1_2_527) {
			_scValue->setString("1.2.527");
		} else if (BaseEngine::instance().getTargetExecutable() == FOXTAIL_1_2_896) {
			_scValue->setString("1.2.896");
		} else if (BaseEngine::instance().getTargetExecutable() == FOXTAIL_1_2_902) {
			_scValue->setString("1.2.902");
		} else {
			_scValue->setString("UNKNOWN");
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] GameVersion (RO)
	// Used to display full game version at options.script in UpdateControls()
	// Returns FoxTail version number as a string
	//////////////////////////////////////////////////////////////////////////
	else if (name == "GameVersion") {
		uint32 gameVersion = 0;
		BaseFileManager *fileManager = BaseEngine::instance().getFileManager();
		if (fileManager) {
			gameVersion = fileManager->getPackageVersion("data.dcp");
		}
		char tmp[16];
		sprintf(tmp,"%u",gameVersion);
		_scValue->setString(tmp);
		return _scValue;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// Platform (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Platform") {
		_scValue->setString(BasePlatform::getPlatformName().c_str());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeviceType (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "DeviceType") {
		_scValue->setString(getDeviceType().c_str());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MostRecentSaveSlot (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MostRecentSaveSlot") {
		if (!ConfMan.hasKey("most_recent_saveslot")) {
			_scValue->setInt(-1);
		} else {
			_scValue->setInt(ConfMan.getInt("most_recent_saveslot"));
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Store (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Store") {
		_scValue->setNULL();
		error("Request for a SXStore-object, which is not supported by ScummVM");

		return _scValue;
	} else {
		return BaseObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MouseX") == 0) {
		_mousePos.x = value->getInt();
		resetMousePos();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MouseY") == 0) {
		_mousePos.y = value->getInt();
		resetMousePos();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Name") == 0) {
		bool res = BaseObject::scSetProperty(name, value);
		setWindowTitle();
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MainObject") == 0) {
		BaseScriptable *obj = value->getNative();
		if (obj == nullptr || validObject((BaseObject *)obj)) {
			_mainObject = (BaseObject *)obj;
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Interactive") == 0) {
		setInteractive(value->getBool());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SFXVolume") == 0) {
		_gameRef->LOG(0, "**Warning** The SFXVolume attribute is obsolete");
		_gameRef->_soundMgr->setVolumePercent(Audio::Mixer::kSFXSoundType, (byte)value->getInt());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SpeechVolume") == 0) {
		_gameRef->LOG(0, "**Warning** The SpeechVolume attribute is obsolete");
		_gameRef->_soundMgr->setVolumePercent(Audio::Mixer::kSpeechSoundType, (byte)value->getInt());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MusicVolume") == 0) {
		_gameRef->LOG(0, "**Warning** The MusicVolume attribute is obsolete");
		_gameRef->_soundMgr->setVolumePercent(Audio::Mixer::kMusicSoundType, (byte)value->getInt());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MasterVolume") == 0) {
		_gameRef->LOG(0, "**Warning** The MasterVolume attribute is obsolete");
		_gameRef->_soundMgr->setMasterVolumePercent((byte)value->getInt());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Subtitles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Subtitles") == 0) {
		_subtitles = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesSpeed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesSpeed") == 0) {
		_subtitlesSpeed = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// VideoSubtitles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "VideoSubtitles") == 0) {
		_videoSubtitles = value->getBool();
		return STATUS_OK;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// Shadows (obsolete)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Shadows") == 0) {
		if (value->getBool()) {
			setMaxShadowType(SHADOW_STENCIL);
		} else {
			setMaxShadowType(SHADOW_NONE);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SimpleShadows (obsolete)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SimpleShadows") == 0) {
		if (value->getBool()) {
			setMaxShadowType(SHADOW_SIMPLE);
		} else {
			setMaxShadowType(SHADOW_STENCIL);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxShadowType
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxShadowType") == 0) {
		setMaxShadowType(static_cast<TShadowType>(value->getInt()));
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// TextEncoding
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TextEncoding") == 0) {
		int enc = value->getInt();
		if (enc < 0) {
			enc = 0;
		}
		if (enc >= NUM_TEXT_ENCODINGS) {
			enc = NUM_TEXT_ENCODINGS - 1;
		}
		_textEncoding = (TTextEncoding)enc;
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextRTL
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TextRTL") == 0) {
		_textRTL = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundBufferSize
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundBufferSize") == 0) {
		_soundBufferSizeSec = value->getInt();
		_soundBufferSizeSec = MAX<int32>(3, _soundBufferSizeSec);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuspendedRendering
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SuspendedRendering") == 0) {
		_suspendedRendering = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuppressScriptErrors
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SuppressScriptErrors") == 0) {
		_suppressScriptErrors = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutorunDisabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutorunDisabled") == 0) {
		_autorunDisabled = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveOnExit
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutoSaveOnExit") == 0) {
		_autoSaveOnExit = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveSlot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutoSaveSlot") == 0) {
		_autoSaveSlot = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorHidden
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorHidden") == 0) {
		_cursorHidden = value->getBool();
		return STATUS_OK;
	} else {
		return BaseObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *BaseGame::scToString() {
	return "[game object]";
}



#define QUICK_MSG_DURATION 3000
//////////////////////////////////////////////////////////////////////////
bool BaseGame::displayQuickMsg() {
	if (_quickMessages.size() == 0 || !_systemFont) {
		return STATUS_OK;
	}

	// update
	for (uint32 i = 0; i < _quickMessages.size(); i++) {
		if (_currentTime - _quickMessages[i]->getStartTime() >= QUICK_MSG_DURATION) {
			delete _quickMessages[i];
			_quickMessages.remove_at(i);
			i--;
		}
	}

	int posY = 20;

	// display
	for (uint32 i = 0; i < _quickMessages.size(); i++) {
		_systemFont->drawText((const byte *)_quickMessages[i]->getText(), 0, posY, _renderer->getWidth());
		posY += _systemFont->getTextHeight((const byte *)_quickMessages[i]->getText(), _renderer->getWidth());
	}
	return STATUS_OK;
}


#define MAX_QUICK_MSG 5
//////////////////////////////////////////////////////////////////////////
void BaseGame::quickMessage(const char *text) {
	if (_quickMessages.size() >= MAX_QUICK_MSG) {
		delete _quickMessages[0];
		_quickMessages.remove_at(0);
	}
	_quickMessages.add(new BaseQuickMsg(_currentTime,  text));
}


//////////////////////////////////////////////////////////////////////////
void BaseGame::quickMessageForm(char *fmt, ...) {
	char buff[256];
	va_list va;

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	quickMessage(buff);
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::registerObject(BaseObject *object) {
	_regObjects.add(object);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::unregisterObject(BaseObject *object) {
	if (!object) {
		return STATUS_OK;
	}

	// is it a window?
	for (uint32 i = 0; i < _windows.size(); i++) {
		if ((BaseObject *)_windows[i] == object) {
			_windows.remove_at(i);

			// get new focused window
			if (_focusedWindow == object) {
				_focusedWindow = nullptr;
			}

			break;
		}
	}

	// is it active object?
	if (_activeObject == object) {
		_activeObject = nullptr;
	}

	// is it main object?
	if (_mainObject == object) {
		_mainObject = nullptr;
	}

	// destroy object
	for (uint32 i = 0; i < _regObjects.size(); i++) {
		if (_regObjects[i] == object) {
			_regObjects.remove_at(i);
			if (!_loadInProgress) {
				SystemClassRegistry::getInstance()->enumInstances(invalidateValues, "ScValue", (void *)object);
			}
			delete object;
			return STATUS_OK;
		}
	}

	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
void BaseGame::invalidateValues(void *value, void *data) {
	ScValue *val = (ScValue *)value;
	if (val->isNative() && val->getNative() == data) {
		if (!val->_persistent && ((BaseScriptable *)data)->_refCount == 1) {
			((BaseScriptable *)data)->_refCount++;
		}
		val->setNative(nullptr);
		val->setNULL();
	}
}



//////////////////////////////////////////////////////////////////////////
bool BaseGame::validObject(BaseObject *object) {
	if (!object) {
		return false;
	}
	if (object == this) {
		return true;
	}

	for (uint32 i = 0; i < _regObjects.size(); i++) {
		if (_regObjects[i] == object) {
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::externalCall(ScScript *script, ScStack *stack, ScStack *thisStack, char *name) {
	ScValue *thisObj;

	//////////////////////////////////////////////////////////////////////////
	// LOG
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "LOG") == 0) {
		stack->correctParams(1);
		_gameRef->LOG(0, "sc: %s", stack->pop()->getString());
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// String
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "String") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXString(_gameRef,  stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// MemBuffer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MemBuffer") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXMemBuffer(_gameRef,  stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// File
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "File") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXFile(_gameRef,  stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Directory
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Directory") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXDirectory(_gameRef));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Date
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Date") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXDate(_gameRef,  stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Array
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Array") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXArray(_gameRef,  stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Object
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Object") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXObject(_gameRef,  stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Sleep
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Sleep") == 0) {
		stack->correctParams(1);

		script->sleep((uint32)stack->pop()->getInt());
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// WaitFor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WaitFor") == 0) {
		stack->correctParams(1);

		BaseScriptable *obj = stack->pop()->getNative();
		if (validObject((BaseObject *)obj)) {
			script->waitForExclusive((BaseObject *)obj);
		}
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Random
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Random") == 0) {
		stack->correctParams(2);

		int from = stack->pop()->getInt();
		int to   = stack->pop()->getInt();

		stack->pushInt(BaseUtils::randomInt(from, to));
	}

	//////////////////////////////////////////////////////////////////////////
	// SetScriptTimeSlice
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetScriptTimeSlice") == 0) {
		stack->correctParams(1);

		script->_timeSlice = (uint32)stack->pop()->getInt();
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// MakeRGBA / MakeRGB / RGB
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MakeRGBA") == 0 || strcmp(name, "MakeRGB") == 0 || strcmp(name, "RGB") == 0) {
		stack->correctParams(4);
		int r = stack->pop()->getInt();
		int g = stack->pop()->getInt();
		int b = stack->pop()->getInt();
		int a;
		ScValue *val = stack->pop();
		if (val->isNULL()) {
			a = 255;
		} else {
			a = val->getInt();
		}

		stack->pushInt(BYTETORGBA(r, g, b, a));
	}

	//////////////////////////////////////////////////////////////////////////
	// MakeHSL
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MakeHSL") == 0) {
		stack->correctParams(3);
		int h = stack->pop()->getInt();
		int s = stack->pop()->getInt();
		int l = stack->pop()->getInt();

		stack->pushInt(BaseUtils::HSLtoRGB(h, s, l));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetRValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetRValue") == 0) {
		stack->correctParams(1);

		uint32 rgba = (uint32)stack->pop()->getInt();
		stack->pushInt(RGBCOLGetR(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetGValue") == 0) {
		stack->correctParams(1);

		uint32 rgba = (uint32)stack->pop()->getInt();
		stack->pushInt(RGBCOLGetG(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetBValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetBValue") == 0) {
		stack->correctParams(1);

		uint32 rgba = (uint32)stack->pop()->getInt();
		stack->pushInt(RGBCOLGetB(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetAValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetAValue") == 0) {
		stack->correctParams(1);

		uint32 rgba = (uint32)stack->pop()->getInt();
		stack->pushInt(RGBCOLGetA(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHValue") == 0) {
		stack->correctParams(1);
		uint32 rgb = (uint32)stack->pop()->getInt();

		byte H, S, L;
		BaseUtils::RGBtoHSL(rgb, &H, &S, &L);
		stack->pushInt(H);
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSValue") == 0) {
		stack->correctParams(1);
		uint32 rgb = (uint32)stack->pop()->getInt();

		byte H, S, L;
		BaseUtils::RGBtoHSL(rgb, &H, &S, &L);
		stack->pushInt(S);
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetLValue") == 0) {
		stack->correctParams(1);
		uint32 rgb = (uint32)stack->pop()->getInt();

		byte H, S, L;
		BaseUtils::RGBtoHSL(rgb, &H, &S, &L);
		stack->pushInt(L);
	}

	//////////////////////////////////////////////////////////////////////////
	// Debug
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Debug") == 0) {
		stack->correctParams(0);
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// ToString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ToString") == 0) {
		stack->correctParams(1);
		const char *str = stack->pop()->getString();
		char *str2 = new char[strlen(str) + 1];
		strcpy(str2, str);
		stack->pushString(str2);
		delete[] str2;
	}

	//////////////////////////////////////////////////////////////////////////
	// ToInt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ToInt") == 0) {
		stack->correctParams(1);
		int val = stack->pop()->getInt();
		stack->pushInt(val);
	}

	//////////////////////////////////////////////////////////////////////////
	// ToFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ToFloat") == 0) {
		stack->correctParams(1);
		double val = stack->pop()->getFloat();
		stack->pushFloat(val);
	}

	//////////////////////////////////////////////////////////////////////////
	// ToBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ToBool") == 0) {
		stack->correctParams(1);
		bool val = stack->pop()->getBool();
		stack->pushBool(val);
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] IsNumber
	// Used at kalimba.script to check if string token is a number
	// If true is returned, then ToInt() is called for same parameter
	// ToInt(string) implementation is using atoi(), so let's use it here too
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsNumber") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		
		bool result = false;
		if (val->isInt() || val->isFloat()) {
			result = true;
		} else if (val->isString()) {
			const char *str = val->getString();
			result = (atoi(str) != 0) || (strcmp(str, "0") == 0);
		}

		stack->pushBool(result);
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] Split
	// Returns array of words of a string, using another as a delimeter
	// Used to split strings by 1-2 characters delimeter in various scripts
	// All the delimeters ever used in FoxTail are:
	//   " ", "@", "#", "$", "&", ",", "\\", "@h", "@i", "@p", "@s"
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Split") == 0) {
		stack->correctParams(2);
		const char *str = stack->pop()->getString();
		Common::String sep = stack->pop()->getString();
		uint32 size = strlen(str) + 1;

		// Let's make copies before modifying stack
		char *copy = new char[size];
		strcpy(copy, str);

		// There is no way to makeSXArray() with exactly 1 given element
		// That's why we are creating empty Array and SXArray::push() later
		stack->pushInt(0);
		BaseScriptable *arr = makeSXArray(_gameRef, stack);

		// Iterating string copy, replacing delimeter with '\0' and pushing matches
		// Only non-empty matches should be pushed
		char *begin = copy;
		for (char *it = copy; it < copy + size; it++) {
			if (strncmp(it, sep.c_str(), sep.size()) == 0 || *it == '\0') {
				*it = '\0';
				if (it != begin) {
					stack->pushString(begin);
					((SXArray *)arr)->push(stack->pop());
				}
				begin = it + sep.size();
				it = begin - 1;
			}
		}

		stack->pushNative(arr, false);

		delete[] copy;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] Trim / lTrim / rTrim
	// Removes whitespaces from a string from the left & right
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Trim") == 0 || strcmp(name, "lTrim") == 0 || strcmp(name, "rTrim") == 0) {
		stack->correctParams(1);
		const char *str = stack->pop()->getString();
		char *copy = new char[strlen(str) + 1];
		strcpy(copy, str); 

		char *ptr = copy;
		if (strcmp(name, "rTrim") != 0) {
			ptr = Common::ltrim(ptr);
		}
		if (strcmp(name, "lTrim") != 0) {
			ptr = Common::rtrim(ptr);
		}

		stack->pushString(ptr);

		delete[] copy;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// Plugins: emulate object constructors from known "wme_*.dll" plugins
	//////////////////////////////////////////////////////////////////////////
	else if(!DID_FAIL(EmulatePluginCall(_gameRef, stack, thisStack, name))) {
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// failure
	else {
		script->runtimeError("Call to undefined function '%s'. Ignored.", name);
		stack->correctParams(0);
		stack->pushNULL();
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::showCursor() {
	if (_cursorHidden) {
		return STATUS_OK;
	}

	if (!_interactive && _gameRef->_state == GAME_RUNNING) {
		if (_cursorNoninteractive) {
			return drawCursor(_cursorNoninteractive);
		}
	} else {
		if (_activeObject && !DID_FAIL(_activeObject->showCursor())) {
			return STATUS_OK;
		} else {
			if (_activeObject && _activeCursor && _activeObject->getExtendedFlag("usable")) {
				return drawCursor(_activeCursor);
			} else if (_cursor) {
				return drawCursor(_cursor);
			}
		}
	}
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::saveGame(int32 slot, const char *desc, bool quickSave) {
	return SaveLoad::saveGame(slot, desc, quickSave, _gameRef);
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::loadGame(uint32 slot) {
	//_gameRef->LOG(0, "Load start %d", BaseUtils::GetUsedMemMB());

	_loading = false;
	_scheduledLoadSlot = -1;

	Common::String filename = SaveLoad::getSaveSlotFilename(slot);

	return loadGame(filename.c_str());
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::loadGame(const char *filename) {
	return SaveLoad::loadGame(filename, _gameRef);
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::displayWindows(bool inGame) {
	bool res;

	// did we lose focus? focus topmost window
	if (_focusedWindow == nullptr || !_focusedWindow->isVisible() || _focusedWindow->isDisabled()) {
		_focusedWindow = nullptr;
		for (int i = _windows.size() - 1; i >= 0; i--) {
			if (_windows[i]->isVisible() && !_windows[i]->isDisabled()) {
				_focusedWindow = _windows[i];
				break;
			}
		}
	}

	// display all windows
	for (uint32 i = 0; i < _windows.size(); i++) {
		if (_windows[i]->isVisible() && _windows[i]->getInGame() == inGame) {

			res = _windows[i]->display();
			if (DID_FAIL(res)) {
				return res;
			}
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::persist(BasePersistenceManager *persistMgr) {
	if (!persistMgr->getIsSaving()) {
		cleanup();
	}

	BaseObject::persist(persistMgr);

	persistMgr->transferPtr(TMEMBER_PTR(_activeObject));
	persistMgr->transferPtr(TMEMBER_PTR(_capturedObject));
	persistMgr->transferPtr(TMEMBER_PTR(_cursorNoninteractive));
	persistMgr->transferBool(TMEMBER(_editorMode));
	persistMgr->transferPtr(TMEMBER_PTR(_fader));
	persistMgr->transferSint32(TMEMBER(_freezeLevel));
	persistMgr->transferPtr(TMEMBER_PTR(_focusedWindow));
	persistMgr->transferPtr(TMEMBER_PTR(_fontStorage));
	persistMgr->transferBool(TMEMBER(_interactive));
	persistMgr->transferPtr(TMEMBER_PTR(_keyboardState));
	persistMgr->transferUint32(TMEMBER(_lastTime));
	persistMgr->transferPtr(TMEMBER_PTR(_mainObject));
	_musicSystem->persistChannels(persistMgr);
	_musicSystem->persistCrossfadeSettings(persistMgr);

	persistMgr->transferSint32(TMEMBER(_offsetX));
	persistMgr->transferSint32(TMEMBER(_offsetY));
	persistMgr->transferFloat(TMEMBER(_offsetPercentX));
	persistMgr->transferFloat(TMEMBER(_offsetPercentY));

	persistMgr->transferBool(TMEMBER(_origInteractive));
	persistMgr->transferSint32(TMEMBER_INT(_origState));
	persistMgr->transferBool(TMEMBER(_personalizedSave));
	persistMgr->transferBool(TMEMBER(_quitting));

	_regObjects.persist(persistMgr);

	persistMgr->transferPtr(TMEMBER_PTR(_scEngine));
	//persistMgr->transfer(TMEMBER(_soundMgr));
	persistMgr->transferSint32(TMEMBER_INT(_state));
	//persistMgr->transfer(TMEMBER(_surfaceStorage));
	persistMgr->transferBool(TMEMBER(_subtitles));
	persistMgr->transferSint32(TMEMBER(_subtitlesSpeed));
	persistMgr->transferPtr(TMEMBER_PTR(_systemFont));
	persistMgr->transferPtr(TMEMBER_PTR(_videoFont));
	persistMgr->transferBool(TMEMBER(_videoSubtitles));

	_timerNormal.persist(persistMgr);
	_timerLive.persist(persistMgr);

	_renderer->persistSaveLoadImages(persistMgr);

	persistMgr->transferSint32(TMEMBER_INT(_textEncoding));
	persistMgr->transferBool(TMEMBER(_textRTL));

	persistMgr->transferSint32(TMEMBER(_soundBufferSizeSec));
	persistMgr->transferBool(TMEMBER(_suspendedRendering));

	persistMgr->transferRect32(TMEMBER(_mouseLockRect));

	_windows.persist(persistMgr);

	persistMgr->transferBool(TMEMBER(_suppressScriptErrors));
	persistMgr->transferBool(TMEMBER(_autorunDisabled));

	persistMgr->transferBool(TMEMBER(_autoSaveOnExit));
	persistMgr->transferUint32(TMEMBER(_autoSaveSlot));
	persistMgr->transferBool(TMEMBER(_cursorHidden));

	if (persistMgr->checkVersion(1, 3, 1)) {
		_settings->persist(persistMgr);
	}

	if (!persistMgr->getIsSaving()) {
		_quitting = false;
	}

#ifdef ENABLE_WME3D
	if (BaseEngine::instance().getFlags() & GF_3D) {
		persistMgr->transferSint32(TMEMBER_INT(_maxShadowType));
	}
#endif

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::focusWindow(UIWindow *window) {
	UIWindow *prev = _focusedWindow;

	for (uint32 i = 0; i < _windows.size(); i++) {
		if (_windows[i] == window) {
			if (i < _windows.size() - 1) {
				_windows.remove_at(i);
				_windows.add(window);

				_gameRef->_focusedWindow = window;
			}

			if (window->getMode() == WINDOW_NORMAL && prev != window && _gameRef->validObject(prev) && (prev->getMode() == WINDOW_EXCLUSIVE || prev->getMode() == WINDOW_SYSTEM_EXCLUSIVE)) {
				return focusWindow(prev);
			} else {
				return STATUS_OK;
			}
		}
	}
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::freeze(bool includingMusic) {
	if (_freezeLevel == 0) {
		_scEngine->pauseAll();
		_soundMgr->pauseAll(includingMusic);
		_origState = _state;
		_origInteractive = _interactive;
		_interactive = true;
	}
	_state = GAME_FROZEN;
	_freezeLevel++;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::unfreeze() {
	if (_freezeLevel == 0) {
		return STATUS_OK;
	}

	_freezeLevel--;
	if (_freezeLevel == 0) {
		_state = _origState;
		_interactive = _origInteractive;
		_scEngine->resumeAll();
		_soundMgr->resumeAll();
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::handleKeypress(Common::Event *event, bool printable) {
	if (isVideoPlaying()) {
		if (event->kbd.keycode == Common::KEYCODE_ESCAPE) {
			stopVideo();
		}
		return true;
	}

	if (event->type == Common::EVENT_QUIT) {
		onWindowClose();
		return true;
	}

	_keyboardState->handleKeyPress(event);
	_keyboardState->readKey(event);
// TODO

	if (_focusedWindow) {
		if (!_gameRef->_focusedWindow->handleKeypress(event, _keyboardState->isCurrentPrintable())) {
			/*if (event->type != SDL_TEXTINPUT) {*/
			if (_gameRef->_focusedWindow->canHandleEvent("Keypress")) {
				_gameRef->_focusedWindow->applyEvent("Keypress");
			} else {
				applyEvent("Keypress");
			}
			/*}*/
		}
		return true;
	} else { /*if (event->type != SDL_TEXTINPUT)*/
		applyEvent("Keypress");
		return true;
	}

	return false;
}

void BaseGame::handleKeyRelease(Common::Event *event) {
	_keyboardState->handleKeyRelease(event);
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::handleMouseWheel(int32 delta) {
	bool handled = false;
	if (_focusedWindow) {
		handled = _gameRef->_focusedWindow->handleMouseWheel(delta);

		if (!handled) {
			if (delta < 0 && _gameRef->_focusedWindow->canHandleEvent("MouseWheelDown")) {
				_gameRef->_focusedWindow->applyEvent("MouseWheelDown");
				handled = true;
			} else if (_gameRef->_focusedWindow->canHandleEvent("MouseWheelUp")) {
				_gameRef->_focusedWindow->applyEvent("MouseWheelUp");
				handled = true;
			}

		}
	}

	if (!handled) {
		if (delta < 0) {
			applyEvent("MouseWheelDown");
		} else {
			applyEvent("MouseWheelUp");
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::handleCustomActionStart(BaseGameCustomAction action) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::handleCustomActionEnd(BaseGameCustomAction action) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor) const {
	if (verMajor) {
		*verMajor = DCGF_VER_MAJOR;
	}
	if (verMinor) {
		*verMinor = DCGF_VER_MINOR;
	}

	if (extMajor) {
		*extMajor = 0;
	}
	if (extMinor) {
		*extMinor = 0;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void BaseGame::setWindowTitle() {
	if (_renderer) {
		char title[512];
		Common::strlcpy(title, _caption[0], 512);
		if (title[0] != '\0') {
			Common::strlcat(title, " - ", 512);
		}
		Common::strlcat(title, "WME Lite", 512);


		Utf8String utf8Title;
		if (_textEncoding == TEXT_UTF8) {
			utf8Title = Utf8String(title);
		} else {
			warning("BaseGame::SetWindowTitle - Ignoring textencoding");
			utf8Title = Utf8String(title);
			/*          WideString wstr = StringUtil::AnsiToWide(Title);
			            title = StringUtil::WideToUtf8(wstr);*/
		}
		warning("BaseGame::SetWindowTitle: Ignoring value: %s", utf8Title.c_str());
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::setActiveObject(BaseObject *obj) {
	// not-active when game is frozen
	if (obj && !_gameRef->_interactive && !obj->_nonIntMouseEvents) {
		obj = nullptr;
	}

	if (obj == _activeObject) {
		return STATUS_OK;
	}

	if (_activeObject) {
		_activeObject->applyEvent("MouseLeave");
	}
	//if (ValidObject(_activeObject)) _activeObject->applyEvent("MouseLeave");
	_activeObject = obj;
	if (_activeObject) {
		_activeObject->applyEvent("MouseEntry");
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::pushViewport(BaseViewport *viewport) {
	_viewportSP++;
	if (_viewportSP >= (int32)_viewportStack.size()) {
		_viewportStack.add(viewport);
	} else {
		_viewportStack[_viewportSP] = viewport;
	}

	_renderer->setViewport(viewport->getRect());

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::popViewport() {
	_viewportSP--;
	if (_viewportSP < -1) {
		_gameRef->LOG(0, "Fatal: Viewport stack underflow!");
	}

	if (_viewportSP >= 0 && _viewportSP < (int32)_viewportStack.size()) {
		_renderer->setViewport(_viewportStack[_viewportSP]->getRect());
	} else _renderer->setViewport(_renderer->_drawOffsetX,
		                              _renderer->_drawOffsetY,
		                              _renderer->getWidth() + _renderer->_drawOffsetX,
		                              _renderer->getHeight() + _renderer->_drawOffsetY);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::getCurrentViewportRect(Rect32 *rect, bool *custom) const {
	if (rect == nullptr) {
		return STATUS_FAILED;
	} else {
		if (_viewportSP >= 0) {
			BasePlatform::copyRect(rect, _viewportStack[_viewportSP]->getRect());
			if (custom) {
				*custom = true;
			}
		} else {
			rect->setRect(_renderer->_drawOffsetX,
						  _renderer->_drawOffsetY,
						  _renderer->getWidth() + _renderer->_drawOffsetX,
						  _renderer->getHeight() + _renderer->_drawOffsetY);
			if (custom) {
				*custom = false;
			}
		}

		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::getCurrentViewportOffset(int *offsetX, int *offsetY) const {
	if (_viewportSP >= 0) {
		if (offsetX) {
			*offsetX = _viewportStack[_viewportSP]->_offsetX;
		}
		if (offsetY) {
			*offsetY = _viewportStack[_viewportSP]->_offsetY;
		}
	} else {
		if (offsetX) {
			*offsetX = 0;
		}
		if (offsetY) {
			*offsetY = 0;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::windowLoadHook(UIWindow *win, char **buf, char **params) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::windowScriptMethodHook(UIWindow *win, ScScript *script, ScStack *stack, const char *name) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
void BaseGame::setInteractive(bool state) {
	_interactive = state;
	if (_transMgr) {
		_transMgr->_origInteractive = state;
	}
}


//////////////////////////////////////////////////////////////////////////
void BaseGame::resetMousePos() {
	Common::Point p;
	p.x = _mousePos.x + _renderer->_drawOffsetX;
	p.y = _mousePos.y + _renderer->_drawOffsetY;

	BasePlatform::setCursorPos(p.x, p.y);
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::displayContent(bool doUpdate, bool displayAll) {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::displayContentSimple() {
	// fill black
	_renderer->fill(0, 0, 0);
	_renderer->displayIndicator();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::resetContent() {
	_scEngine->clearGlobals();
	//_timer = 0;
	//_liveTimer = 0;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseGame::DEBUG_DumpClassRegistry() {
	warning("DEBUG_DumpClassRegistry - untested");
	Common::DumpFile *f = new Common::DumpFile;
	f->open("zz_class_reg_dump.log");

	SystemClassRegistry::getInstance()->dumpClasses(f);

	f->close();
	delete f;
	_gameRef->quickMessage("Classes dump completed.");
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::invalidateDeviceObjects() {
	for (uint32 i = 0; i < _regObjects.size(); i++) {
		_regObjects[i]->invalidateDeviceObjects();
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::restoreDeviceObjects() {
	for (uint32 i = 0; i < _regObjects.size(); i++) {
		_regObjects[i]->restoreDeviceObjects();
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::setWaitCursor(const char *filename) {
	delete _cursorNoninteractive;
	_cursorNoninteractive = nullptr;

	_cursorNoninteractive = new BaseSprite(_gameRef);
	if (!_cursorNoninteractive || DID_FAIL(_cursorNoninteractive->loadFile(filename))) {
		delete _cursorNoninteractive;
		_cursorNoninteractive = nullptr;
		return STATUS_FAILED;
	} else {
		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::isVideoPlaying() {
	if (_videoPlayer->isPlaying()) {
		return true;
	}
	if (_theoraPlayer && _theoraPlayer->isPlaying()) {
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::stopVideo() {
	if (_videoPlayer->isPlaying()) {
		_videoPlayer->stop();
	}
	if (_theoraPlayer && _theoraPlayer->isPlaying()) {
		_theoraPlayer->stop();
		delete _theoraPlayer;
		_theoraPlayer = nullptr;
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseGame::drawCursor(BaseSprite *cursor) {
	if (!cursor) {
		return STATUS_FAILED;
	}
	if (cursor != _lastCursor) {
		cursor->reset();
		_lastCursor = cursor;
	}
	return cursor->draw(_mousePos.x, _mousePos.y);
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::storeSaveThumbnail() {
	delete _cachedThumbnail;
	_cachedThumbnail = new SaveThumbHelper(this);
	if (DID_FAIL(_cachedThumbnail->storeThumbnail())) {
		deleteSaveThumbnail();
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void BaseGame::deleteSaveThumbnail() {
	delete _cachedThumbnail;
	_cachedThumbnail = nullptr;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool BaseGame::onActivate(bool activate, bool refreshMouse) {
	if (_shuttingDown || !_renderer) {
		return STATUS_OK;
	}

	_renderer->_active = activate;

	if (refreshMouse) {
		Point32 p;
		getMousePos(&p);
		setActiveObject(_renderer->getObjectAt(p.x, p.y));
	}

	if (activate) {
		_soundMgr->resumeAll();
	} else {
		_soundMgr->pauseAll();
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onMouseLeftDown() {
	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_LEFT);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("LeftClick"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("LeftClick");
		}
	}

	if (_activeObject != nullptr) {
		_capturedObject = _activeObject;
	}
	_mouseLeftDown = true;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onMouseLeftUp() {
	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_LEFT);
	}

	_capturedObject = nullptr;
	_mouseLeftDown = false;

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("LeftRelease"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("LeftRelease");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onMouseLeftDblClick() {
	if (_state == GAME_RUNNING && !_interactive) {
		return STATUS_OK;
	}

	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_LEFT);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("LeftDoubleClick"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("LeftDoubleClick");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onMouseRightDblClick() {
	if (_state == GAME_RUNNING && !_interactive) {
		return STATUS_OK;
	}

	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_RIGHT);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("RightDoubleClick"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("RightDoubleClick");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onMouseRightDown() {
	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_RIGHT);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("RightClick"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("RightClick");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onMouseRightUp() {
	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_RIGHT);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("RightRelease"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("RightRelease");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onMouseMiddleDown() {
	if (_state == GAME_RUNNING && !_interactive) {
		return STATUS_OK;
	}

	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_MIDDLE);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("MiddleClick"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("MiddleClick");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onMouseMiddleUp() {
	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_MIDDLE);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("MiddleRelease"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("MiddleRelease");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onPaint() {
	if (_renderer && _renderer->isWindowed() && _renderer->isReady()) {
		_renderer->initLoop();
		displayContent(false, true);
		displayDebugInfo();
		_renderer->windowedBlt();
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onWindowClose() {
	if (canHandleEvent("QuitGame")) {
		if (_state != GAME_FROZEN) {
			_gameRef->applyEvent("QuitGame");
		}
		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::displayDebugInfo() {
	const uint32 strLength = 100;
	char str[strLength];

	if (_debugShowFPS) {
		sprintf(str, "FPS: %d", _gameRef->_fps);
		_systemFont->drawText((byte *)str, 0, 0, 100, TAL_LEFT);
	}

	if (_gameRef->_debugDebugMode) {
		if (!_gameRef->_renderer->isWindowed()) {
			sprintf(str, "Mode: %dx%dx%d", _renderer->getWidth(), _renderer->getHeight(), _renderer->getBPP());
		} else {
			sprintf(str, "Mode: %dx%d windowed", _renderer->getWidth(), _renderer->getHeight());
		}

		Common::strlcat(str, " (", strLength);
		Common::strlcat(str, _renderer->getName().c_str(), strLength);
		Common::strlcat(str, ")", strLength);
		_systemFont->drawText((byte *)str, 0, 0, _renderer->getWidth(), TAL_RIGHT);

		_renderer->displayDebugInfo();

		int scrTotal, scrRunning, scrWaiting, scrPersistent;
		scrTotal = _scEngine->getNumScripts(&scrRunning, &scrWaiting, &scrPersistent);
		sprintf(str, "Running scripts: %d (r:%d w:%d p:%d)", scrTotal, scrRunning, scrWaiting, scrPersistent);
		_systemFont->drawText((byte *)str, 0, 70, _renderer->getWidth(), TAL_RIGHT);


		sprintf(str, "Timer: %d", getTimer()->getTime());
		_gameRef->_systemFont->drawText((byte *)str, 0, 130, _renderer->getWidth(), TAL_RIGHT);

		if (_activeObject != nullptr) {
			_systemFont->drawText((const byte *)_activeObject->getName(), 0, 150, _renderer->getWidth(), TAL_RIGHT);
		}

		sprintf(str, "GfxMem: %dMB", _usedMem / (1024 * 1024));
		_systemFont->drawText((byte *)str, 0, 170, _renderer->getWidth(), TAL_RIGHT);

	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseGame::getMousePos(Point32 *pos) {
	BasePlatform::getCursorPos(pos);

	pos->x -= _renderer->_drawOffsetX;
	pos->y -= _renderer->_drawOffsetY;

	/*
	// Windows can squish maximized window if it's larger than desktop
	// so we need to modify mouse position appropriately (tnx mRax)
	if (_renderer->_windowed && ::IsZoomed(_renderer->_window)) {
	    Common::Rect rc;
	    ::GetClientRect(_renderer->_window, &rc);
	    Pos->x *= _gameRef->_renderer->_realWidth;
	    Pos->x /= (rc.right - rc.left);
	    Pos->y *= _gameRef->_renderer->_realHeight;
	    Pos->y /= (rc.bottom - rc.top);
	}
	*/

	if (_mouseLockRect.left != 0 && _mouseLockRect.right != 0 && _mouseLockRect.top != 0 && _mouseLockRect.bottom != 0) {
		if (!BasePlatform::ptInRect(&_mouseLockRect, *pos)) {
			pos->x = MAX(_mouseLockRect.left, pos->x);
			pos->y = MAX(_mouseLockRect.top, pos->y);

			pos->x = MIN(_mouseLockRect.right, pos->x);
			pos->y = MIN(_mouseLockRect.bottom, pos->y);

			Point32 newPos = *pos;

			newPos.x += _renderer->_drawOffsetX;
			newPos.y += _renderer->_drawOffsetY;

			BasePlatform::setCursorPos(newPos.x, newPos.y);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void BaseGame::miniUpdate() { // TODO: Is this really necessary, it used to update sound, but the mixer does that now.
	if (!_miniUpdateEnabled) {
		return;
	}

	if (g_system->getMillis() - _lastMiniUpdate > 200) {
		_lastMiniUpdate = g_system->getMillis();
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::onScriptShutdown(ScScript *script) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::isLeftDoubleClick() {
	return isDoubleClick(0);
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::isRightDoubleClick() {
	return isDoubleClick(1);
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::isDoubleClick(int32 buttonIndex) {
	uint32 maxDoubleCLickTime = 500;
	int maxMoveX = 4;
	int maxMoveY = 4;

	Point32 pos;
	BasePlatform::getCursorPos(&pos);

	int moveX = abs(pos.x - _lastClick[buttonIndex].posX);
	int moveY = abs(pos.y - _lastClick[buttonIndex].posY);


	if (_lastClick[buttonIndex].time == 0 || g_system->getMillis() - _lastClick[buttonIndex].time > maxDoubleCLickTime || moveX > maxMoveX || moveY > maxMoveY) {
		_lastClick[buttonIndex].time = g_system->getMillis();
		_lastClick[buttonIndex].posX = pos.x;
		_lastClick[buttonIndex].posY = pos.y;
		return false;
	} else {
		_lastClick[buttonIndex].time = 0;
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
void BaseGame::autoSaveOnExit() {
	_soundMgr->saveSettings();

	if (!_autoSaveOnExit) {
		return;
	}
	if (_state == GAME_FROZEN) {
		return;
	}

	saveGame(_autoSaveSlot, "autosave", true);
}

//////////////////////////////////////////////////////////////////////////
void BaseGame::addMem(int32 bytes) {
	_usedMem += bytes;
}

//////////////////////////////////////////////////////////////////////////
AnsiString BaseGame::getDeviceType() const {
	return "computer";
}

//////////////////////////////////////////////////////////////////////////
bool BaseGame::loadSettings(const char *filename) {
	return _settings->loadSettings(filename);
}

//////////////////////////////////////////////////////////////////////////
void BaseGame::expandStringByStringTable(char **str) const {
	_settings->expandStringByStringTable(str);
}

//////////////////////////////////////////////////////////////////////////
void BaseGame::expandStringByStringTable(Common::String &str) const {
	_settings->expandStringByStringTable(str);
}

char *BaseGame::getKeyFromStringTable(const char *str) const {
	return _settings->getKeyFromStringTable(str);
}

#ifdef ENABLE_HEROCRAFT
uint8 BaseGame::getFilePartChecksumHc(const char *filename, uint32 begin, uint32 end) {
	if (begin >= end) {
		warning("Wrong limits for checksum check");
		return 0;
	}

	uint32 size;
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename, &size);
	if (buffer == nullptr) {
		warning("Failed to open '%s' for checksum check", filename);
		return 0;
	}

	if (size < end) {
		warning("File '%s' is too small for checksum check", filename);
		delete[] buffer;
		return 0;
	}

	uint8 result = 0;
	for (uint32 i = begin; i < end; i++) {
		uint8 tmp = buffer[i];
		result += tmp;
		if (result < tmp) {
			result++;
		}
	}

	delete[] buffer;
	return result;
}
#endif

Common::String BaseGame::readRegistryString(const Common::String &key, const Common::String &initValue) const {
	// Game specific hacks:
	Common::String result = initValue;
	// James Peris:
	if (BaseEngine::instance().getGameId() == "jamesperis" && key == "Language") {
		Common::Language language = BaseEngine::instance().getLanguage();
		if (language == Common::EN_ANY) {
			result = "english";
		} else if (language == Common::ES_ESP) {
			result = "spanish";
		} else {
			error("Invalid language set for James Peris");
		}
	} else { // Just fallback to using ConfMan for now
		Common::String privKey = "wme_" + StringUtil::encodeSetting(key);
		if (ConfMan.hasKey(privKey)) {
			result = StringUtil::decodeSetting(ConfMan.get(privKey));
		}
	}
	return result;
}

} // End of namespace Wintermute
