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
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFader.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BImage.h"
#include "engines/wintermute/Base/BKeyboardState.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BQuickMsg.h"
#include "engines/wintermute/Base/BRegistry.h"
#include "engines/wintermute/Base/BRenderSDL.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BSoundMgr.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BSubFrame.h"
#include "engines/wintermute/Base/BSurfaceSDL.h"
#include "engines/wintermute/Base/BTransitionMgr.h"
#include "engines/wintermute/Base/BViewport.h"
#include "engines/wintermute/Base/BStringTable.h"
#include "engines/wintermute/Base/BRegion.h"
#include "engines/wintermute/Base/BSaveThumbHelper.h"
#include "engines/wintermute/Base/BSurfaceStorage.h"
#include "engines/wintermute/utils/crc.h"
#include "engines/wintermute/utils/PathUtil.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/UI/UIWindow.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScEngine.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/SXMath.h"
#include "engines/wintermute/Base/scriptables/SXStore.h"
#include "engines/wintermute/video/VidPlayer.h"
#include "engines/wintermute/video/VidTheoraPlayer.h"
#include "engines/wintermute/wintermute.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/keyboard.h"
#include "common/system.h"
#include "common/file.h"

#ifdef __IPHONEOS__
#   include "ios_utils.h"
#endif

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(CBGame, true)


//////////////////////////////////////////////////////////////////////
CBGame::CBGame(): CBObject(this) {
	_shuttingDown = false;

	_state = GAME_RUNNING;
	_origState = GAME_RUNNING;
	_freezeLevel = 0;

	_interactive = true;
	_origInteractive = false;

	_surfaceStorage = NULL;
	_fontStorage = NULL;
	_renderer = NULL;
	_soundMgr = NULL;
	_fileManager = NULL;
	_transMgr = NULL;
	_debugMgr = NULL;
	_scEngine = NULL;
	_keyboardState = NULL;

	_mathClass = NULL;

	_dEBUG_LogFile = NULL;
	_dEBUG_DebugMode = false;
	_dEBUG_AbsolutePathWarning = true;
	_dEBUG_ShowFPS = false;

	_systemFont = NULL;
	_videoFont = NULL;

	_videoPlayer = NULL;
	_theoraPlayer = NULL;

	_mainObject = NULL;
	_activeObject = NULL;

	_fader = NULL;

	_offsetX = _offsetY = 0;
	_offsetPercentX = _offsetPercentY = 0.0f;

	_subtitles = true;
	_videoSubtitles = true;

	_timer = 0;
	_timerDelta = 0;
	_timerLast = 0;

	_liveTimer = 0;
	_liveTimerDelta = 0;
	_liveTimerLast = 0;

	_sequence = 0;

	_mousePos.x = _mousePos.y = 0;
	_mouseLeftDown = _mouseRightDown = _mouseMidlleDown = false;
	_capturedObject = NULL;

	// FPS counters
	_lastTime = _fpsTime = _deltaTime = _framesRendered = _fps = 0;

	_cursorNoninteractive = NULL;

	_useD3D = false;

	_registry = new CBRegistry(this);
	_stringTable = new CBStringTable(this);

	for (int i = 0; i < NUM_MUSIC_CHANNELS; i++) {
		_music[i] = NULL;
		_musicStartTime[i] = 0;
	}

	_settingsResWidth = 800;
	_settingsResHeight = 600;
	_settingsRequireAcceleration = false;
	_settingsRequireSound = false;
	_settingsTLMode = 0;
	_settingsAllowWindowed = true;
	_settingsGameFile = NULL;
	_settingsAllowAdvanced = false;
	_settingsAllowAccessTab = true;
	_settingsAllowAboutTab = true;
	_settingsAllowDesktopRes = false;

	_editorForceScripts = false;
	_editorAlwaysRegister = false;

	_focusedWindow = NULL;

	_loadInProgress = false;

	_quitting = false;
	_loading = false;
	_scheduledLoadSlot = -1;

	_personalizedSave = false;
	_compressedSavegames = true;

	_editorMode = false;
	_doNotExpandStrings = false;

	_engineLogCallback = NULL;
	_engineLogCallbackData = NULL;

	_smartCache = false;
	_surfaceGCCycleTime = 10000;

	_reportTextureFormat = false;

	_viewportSP = -1;

	_subtitlesSpeed = 70;

	_forceNonStreamedSounds = false;

	_thumbnailWidth = _thumbnailHeight = 0;

	_indicatorDisplay = false;
	_indicatorColor = DRGBA(255, 0, 0, 128);
	_indicatorProgress = 0;
	_indicatorX = -1;
	_indicatorY = -1;
	_indicatorWidth = -1;
	_indicatorHeight = 8;
	_richSavedGames = false;
	_savedGameExt = NULL;
	CBUtils::setString(&_savedGameExt, "dsv");

	_musicCrossfadeRunning = false;
	_musicCrossfadeStartTime = 0;
	_musicCrossfadeLength = 0;
	_musicCrossfadeChannel1 = -1;
	_musicCrossfadeChannel2 = -1;
	_musicCrossfadeSwap = false;

	_loadImageName = NULL;
	_saveImageName = NULL;
	_saveLoadImage = NULL;

	_saveImageX = _saveImageY = 0;
	_loadImageX = _loadImageY = 0;

	_localSaveDir = NULL;
	CBUtils::setString(&_localSaveDir, "saves");
	_saveDirChecked = false;

	_loadingIcon = NULL;
	_loadingIconX = _loadingIconY = 0;
	_loadingIconPersistent = false;

	_textEncoding = TEXT_ANSI;
	_textRTL = false;

	_soundBufferSizeSec = 3;
	_suspendedRendering = false;

	_lastCursor = NULL;


	CBPlatform::SetRectEmpty(&_mouseLockRect);

	_suppressScriptErrors = false;
	_lastMiniUpdate = 0;
	_miniUpdateEnabled = false;

	_cachedThumbnail = NULL;

	_autorunDisabled = false;

	// compatibility bits
	_compatKillMethodThreads = false;

	_usedMem = 0;


	_autoSaveOnExit = true;
	_autoSaveSlot = 999;
	_cursorHidden = false;

#ifdef __IPHONEOS__
	_touchInterface = true;
	_constrainedMemory = true; // TODO differentiate old and new iOS devices
#else
	_touchInterface = false;
	_constrainedMemory = false;
#endif

	_store = NULL;
}


//////////////////////////////////////////////////////////////////////
CBGame::~CBGame() {
	_shuttingDown = true;

	LOG(0, "");
	LOG(0, "Shutting down...");

	getDebugMgr()->onGameShutdown();

	_registry->writeBool("System", "LastRun", true);

	cleanup();

	delete[] _localSaveDir;
	delete[] _settingsGameFile;
	delete[] _savedGameExt;

	delete _cachedThumbnail;

	delete _saveLoadImage;
	delete _mathClass;

	delete _transMgr;
	delete _scEngine;
	delete _fontStorage;
	delete _surfaceStorage;
	delete _videoPlayer;
	delete _theoraPlayer;
	delete _soundMgr;
	delete _debugMgr;
	//SAFE_DELETE(_keyboardState);

	delete _renderer;
	delete _fileManager;
	delete _registry;
	delete _stringTable;

	_localSaveDir = NULL;
	_settingsGameFile = NULL;
	_savedGameExt = NULL;

	_cachedThumbnail = NULL;

	_saveLoadImage = NULL;
	_mathClass = NULL;

	_transMgr = NULL;
	_scEngine = NULL;
	_fontStorage = NULL;
	_surfaceStorage = NULL;
	_videoPlayer = NULL;
	_theoraPlayer = NULL;
	_soundMgr = NULL;
	_debugMgr = NULL;

	_renderer = NULL;
	_fileManager = NULL;
	_registry = NULL;
	_stringTable = NULL;

	DEBUG_DebugDisable();
	CBPlatform::OutputDebugString("--- shutting down normally ---\n");
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::cleanup() {
	delete _loadingIcon;
	_loadingIcon = NULL;

	_engineLogCallback = NULL;
	_engineLogCallbackData = NULL;

	for (int i = 0; i < NUM_MUSIC_CHANNELS; i++) {
		delete _music[i];
		_music[i] = NULL;
		_musicStartTime[i] = 0;
	}

	unregisterObject(_store);
	_store = NULL;

	unregisterObject(_fader);
	_fader = NULL;

	for (int i = 0; i < _regObjects.GetSize(); i++) {
		delete _regObjects[i];
		_regObjects[i] = NULL;
	}
	_regObjects.RemoveAll();

	_windows.RemoveAll(); // refs only
	_focusedWindow = NULL; // ref only

	delete[] _saveImageName;
	delete[] _loadImageName;
	_saveImageName = NULL;
	_loadImageName = NULL;

	delete _cursorNoninteractive;
	delete _cursor;
	delete _activeCursor;
	_cursorNoninteractive = NULL;
	_cursor = NULL;
	_activeCursor = NULL;

	delete _scValue;
	delete _sFX;
	_scValue = NULL;
	_sFX = NULL;

	for (int i = 0; i < _scripts.GetSize(); i++) {
		_scripts[i]->_owner = NULL;
		_scripts[i]->finish();
	}
	_scripts.RemoveAll();

	_fontStorage->removeFont(_systemFont);
	_systemFont = NULL;

	_fontStorage->removeFont(_videoFont);
	_videoFont = NULL;

	for (int i = 0; i < _quickMessages.GetSize(); i++) delete _quickMessages[i];
	_quickMessages.RemoveAll();

	_viewportStack.RemoveAll();
	_viewportSP = -1;

	delete[] _name;
	delete[] _filename;
	_name = NULL;
	_filename = NULL;
	for (int i = 0; i < 7; i++) {
		delete[] _caption[i];
		_caption[i] = NULL;
	}

	_lastCursor = NULL;

	delete _keyboardState;
	_keyboardState = NULL;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBGame::initialize1() {
	bool loaded = false; // Not really a loop, but a goto-replacement.
	while (!loaded) {
		_surfaceStorage = new CBSurfaceStorage(this);
		if (_surfaceStorage == NULL)
			break;

		_fontStorage = new CBFontStorage(this);
		if (_fontStorage == NULL)
			break;

		_fileManager = new CBFileManager(this);
		if (_fileManager == NULL)
			break;

		_soundMgr = new CBSoundMgr(this);
		if (_soundMgr == NULL)
			break;

		_debugMgr = new CBDebugger(this);
		if (_debugMgr == NULL)
			break;

		_mathClass = new CSXMath(this);
		if (_mathClass == NULL)
			break;

		_scEngine = new CScEngine(this);
		if (_scEngine == NULL)
			break;

		_videoPlayer = new CVidPlayer(this);
		if (_videoPlayer == NULL)
			break;

		_transMgr = new CBTransitionMgr(this);
		if (_transMgr == NULL)
			break;

		_keyboardState = new CBKeyboardState(this);
		if (_keyboardState == NULL)
			break;

		_fader = new CBFader(this);
		if (_fader == NULL)
			break;
		registerObject(_fader);

		_store = new CSXStore(this);
		if (_store == NULL)
			break;
		registerObject(_store);
		
		loaded = true;
	}
	if (loaded == true) {
		return S_OK;
	} else {
		delete _mathClass;
		delete _store;
		delete _keyboardState;
		delete _transMgr;
		delete _debugMgr;
		delete _surfaceStorage;
		delete _fontStorage;
		delete _soundMgr;
		delete _fileManager;
		delete _scEngine;
		delete _videoPlayer;
		return E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////
HRESULT CBGame::initialize2() { // we know whether we are going to be accelerated
	_renderer = new CBRenderSDL(this);
	if (_renderer == NULL) return E_FAIL;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBGame::initialize3() { // renderer is initialized
	_posX = _renderer->_width / 2;
	_posY = _renderer->_height / 2;

	if (_indicatorY == -1) _indicatorY = _renderer->_height - _indicatorHeight;
	if (_indicatorX == -1) _indicatorX = 0;
	if (_indicatorWidth == -1) _indicatorWidth = _renderer->_width;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////
void CBGame::DEBUG_DebugEnable(const char *filename) {
	_dEBUG_DebugMode = true;

#ifndef __IPHONEOS__
	//if (filename)_dEBUG_LogFile = fopen(filename, "a+");
	//else _dEBUG_LogFile = fopen("./zz_debug.log", "a+");

	if (!_dEBUG_LogFile) {
		AnsiString safeLogFileName = PathUtil::getSafeLogFileName();
		//_dEBUG_LogFile = fopen(safeLogFileName.c_str(), "a+");
	}

	//if (_dEBUG_LogFile != NULL) fprintf((FILE *)_dEBUG_LogFile, "\n");
	warning("BGame::DEBUG_DebugEnable - No logfile is currently created"); //TODO: Use a dumpfile?
#endif

	/*  time_t timeNow;
	    time(&timeNow);
	    struct tm *tm = localtime(&timeNow);

	#ifdef _DEBUG
	    LOG(0, "********** DEBUG LOG OPENED %02d-%02d-%04d (Debug Build) *******************", tm->tm_mday, tm->tm_mon, tm->tm_year + 1900);
	#else
	    LOG(0, "********** DEBUG LOG OPENED %02d-%02d-%04d (Release Build) *****************", tm->tm_mday, tm->tm_mon, tm->tm_year + 1900);
	#endif*/
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

	LOG(0, "%s ver %d.%d.%d%s, Compiled on " __DATE__ ", " __TIME__, DCGF_NAME, DCGF_VER_MAJOR, DCGF_VER_MINOR, DCGF_VER_BUILD, DCGF_VER_SUFFIX);
	//LOG(0, "Extensions: %s ver %d.%02d", EXT_NAME, EXT_VER_MAJOR, EXT_VER_MINOR);

	AnsiString platform = CBPlatform::GetPlatformName();
	LOG(0, "Platform: %s", platform.c_str());
	LOG(0, "");
}


//////////////////////////////////////////////////////////////////////
void CBGame::DEBUG_DebugDisable() {
	if (_dEBUG_LogFile != NULL) {
		LOG(0, "********** DEBUG LOG CLOSED ********************************************");
		//fclose((FILE *)_dEBUG_LogFile);
		_dEBUG_LogFile = NULL;
	}
	_dEBUG_DebugMode = false;
}


//////////////////////////////////////////////////////////////////////
void CBGame::LOG(HRESULT res, LPCSTR fmt, ...) {
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
	if (_debugMgr) _debugMgr->onLog(res, buff);

	debugCN(kWinterMuteDebugLog, "%02d:%02d:%02d: %s\n", hours, mins, secs, buff);

	//fprintf((FILE *)_dEBUG_LogFile, "%02d:%02d:%02d: %s\n", hours, mins, secs, buff);
	//fflush((FILE *)_dEBUG_LogFile);

	//QuickMessage(buff);
}


//////////////////////////////////////////////////////////////////////////
void CBGame::setEngineLogCallback(ENGINE_LOG_CALLBACK callback, void *data) {
	_engineLogCallback = callback;
	_engineLogCallbackData = data;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBGame::initLoop() {
	_viewportSP = -1;

	_currentTime = CBPlatform::GetTime();

	getDebugMgr()->onGameTick();
	_renderer->initLoop();
	_soundMgr->initLoop();
	updateMusicCrossfade();

	_surfaceStorage->initLoop();
	_fontStorage->initLoop();


	//_activeObject = NULL;

	// count FPS
	_deltaTime = _currentTime - _lastTime;
	_lastTime  = _currentTime;
	_fpsTime += _deltaTime;

	_liveTimerDelta = _liveTimer - _liveTimerLast;
	_liveTimerLast = _liveTimer;
	_liveTimer += MIN((uint32)1000, _deltaTime);

	if (_state != GAME_FROZEN) {
		_timerDelta = _timer - _timerLast;
		_timerLast = _timer;
		_timer += MIN((uint32)1000, _deltaTime);
	} else _timerDelta = 0;

	_framesRendered++;
	if (_fpsTime > 1000) {
		_fps = _framesRendered;
		_framesRendered  = 0;
		_fpsTime = 0;
	}
	//Game->LOG(0, "%d", _fps);

	getMousePos(&_mousePos);

	_focusedWindow = NULL;
	for (int i = _windows.GetSize() - 1; i >= 0; i--) {
		if (_windows[i]->_visible) {
			_focusedWindow = _windows[i];
			break;
		}
	}

	updateSounds();

	if (_fader) _fader->update();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBGame::initInput(HINSTANCE hInst, HWND hWnd) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CBGame::getSequence() {
	return ++_sequence;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::setOffset(int offsetX, int offsetY) {
	_offsetX = offsetX;
	_offsetY = offsetY;
}

//////////////////////////////////////////////////////////////////////////
void CBGame::getOffset(int *offsetX, int *offsetY) {
	if (offsetX != NULL) *offsetX = _offsetX;
	if (offsetY != NULL) *offsetY = _offsetY;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::loadFile(const char *filename) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CBGame::LoadFile failed for file '%s'", filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (FAILED(ret = loadBuffer(buffer, true))) Game->LOG(0, "Error parsing GAME file '%s'", filename);

	delete [] buffer;

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
HRESULT CBGame::loadBuffer(byte *buffer, bool complete) {
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

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_GAME) {
			Game->LOG(0, "'GAME' keyword expected.");
			return E_FAIL;
		}
		buffer = params;
	}

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_SYSTEM_FONT:
			if (_systemFont) _fontStorage->removeFont(_systemFont);
			_systemFont = NULL;

			_systemFont = Game->_fontStorage->addFont((char *)params);
			break;

		case TOKEN_VIDEO_FONT:
			if (_videoFont) _fontStorage->removeFont(_videoFont);
			_videoFont = NULL;

			_videoFont = Game->_fontStorage->addFont((char *)params);
			break;


		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new CBSprite(Game);
			if (!_cursor || FAILED(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_ACTIVE_CURSOR:
			delete _activeCursor;
			_activeCursor = NULL;
			_activeCursor = new CBSprite(Game);
			if (!_activeCursor || FAILED(_activeCursor->loadFile((char *)params))) {
				delete _activeCursor;
				_activeCursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NONINTERACTIVE_CURSOR:
			delete _cursorNoninteractive;
			_cursorNoninteractive = new CBSprite(Game);
			if (!_cursorNoninteractive || FAILED(_cursorNoninteractive->loadFile((char *)params))) {
				delete _cursorNoninteractive;
				_cursorNoninteractive = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_PERSONAL_SAVEGAMES:
			parser.scanStr((char *)params, "%b", &_personalizedSave);
			break;

		case TOKEN_SUBTITLES:
			parser.scanStr((char *)params, "%b", &_subtitles);
			break;

		case TOKEN_SUBTITLES_SPEED:
			parser.scanStr((char *)params, "%d", &_subtitlesSpeed);
			break;

		case TOKEN_VIDEO_SUBTITLES:
			parser.scanStr((char *)params, "%b", &_videoSubtitles);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		case TOKEN_THUMBNAIL_WIDTH:
			parser.scanStr((char *)params, "%d", &_thumbnailWidth);
			break;

		case TOKEN_THUMBNAIL_HEIGHT:
			parser.scanStr((char *)params, "%d", &_thumbnailHeight);
			break;

		case TOKEN_INDICATOR_X:
			parser.scanStr((char *)params, "%d", &_indicatorX);
			break;

		case TOKEN_INDICATOR_Y:
			parser.scanStr((char *)params, "%d", &_indicatorY);
			break;

		case TOKEN_INDICATOR_COLOR: {
			int r, g, b, a;
			parser.scanStr((char *)params, "%d,%d,%d,%d", &r, &g, &b, &a);
			_indicatorColor = DRGBA(r, g, b, a);
		}
		break;

		case TOKEN_INDICATOR_WIDTH:
			parser.scanStr((char *)params, "%d", &_indicatorWidth);
			break;

		case TOKEN_INDICATOR_HEIGHT:
			parser.scanStr((char *)params, "%d", &_indicatorHeight);
			break;

		case TOKEN_SAVE_IMAGE:
			CBUtils::setString(&_saveImageName, (char *)params);
			break;

		case TOKEN_SAVE_IMAGE_X:
			parser.scanStr((char *)params, "%d", &_saveImageX);
			break;

		case TOKEN_SAVE_IMAGE_Y:
			parser.scanStr((char *)params, "%d", &_saveImageY);
			break;

		case TOKEN_LOAD_IMAGE:
			CBUtils::setString(&_loadImageName, (char *)params);
			break;

		case TOKEN_LOAD_IMAGE_X:
			parser.scanStr((char *)params, "%d", &_loadImageX);
			break;

		case TOKEN_LOAD_IMAGE_Y:
			parser.scanStr((char *)params, "%d", &_loadImageY);
			break;

		case TOKEN_LOCAL_SAVE_DIR:
			CBUtils::setString(&_localSaveDir, (char *)params);
			break;

		case TOKEN_COMPAT_KILL_METHOD_THREADS:
			parser.scanStr((char *)params, "%b", &_compatKillMethodThreads);
			break;
		}
	}

	if (!_systemFont) _systemFont = Game->_fontStorage->addFont("system_font.fnt");


	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in GAME definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading GAME definition");
		return E_FAIL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// LOG
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "LOG") == 0) {
		stack->correctParams(1);
		LOG(0, stack->pop()->getString());
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Caption") == 0) {
		HRESULT res = CBObject::scCallMethod(script, stack, thisStack, name);
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
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RunScript
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RunScript") == 0) {
		Game->LOG(0, "**Warning** The 'RunScript' method is now obsolete. Use 'AttachScript' instead (same syntax)");
		stack->correctParams(1);
		if (FAILED(addScript(stack->pop()->getString())))
			stack->pushBool(false);
		else
			stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadStringTable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadStringTable") == 0) {
		stack->correctParams(2);
		const char *filename = stack->pop()->getString();
		CScValue *Val = stack->pop();

		bool ClearOld;
		if (Val->isNULL()) ClearOld = true;
		else ClearOld = Val->getBool();

		if (FAILED(_stringTable->loadFile(filename, ClearOld)))
			stack->pushBool(false);
		else
			stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ValidObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ValidObject") == 0) {
		stack->correctParams(1);
		CBScriptable *obj = stack->pop()->getNative();
		if (validObject((CBObject *) obj)) stack->pushBool(true);
		else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Reset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Reset") == 0) {
		stack->correctParams(0);
		resetContent();
		stack->pushNULL();

		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// UnloadObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadObject") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		CBObject *obj = (CBObject *)val->getNative();
		unregisterObject(obj);
		if (val->getType() == VAL_VARIABLE_REF) val->setNULL();

		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadWindow") == 0) {
		stack->correctParams(1);
		CUIWindow *win = new CUIWindow(Game);
		if (win && SUCCEEDED(win->loadFile(stack->pop()->getString()))) {
			_windows.Add(win);
			registerObject(win);
			stack->pushNative(win, true);
		} else {
			delete win;
			win = NULL;
			stack->pushNULL();
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ExpandString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ExpandString") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		char *str = new char[strlen(val->getString()) + 1];
		strcpy(str, val->getString());
		_stringTable->expand(&str);
		stack->pushString(str);
		delete [] str;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayMusic / PlayMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlayMusic") == 0 || strcmp(name, "PlayMusicChannel") == 0) {
		int channel = 0;
		if (strcmp(name, "PlayMusic") == 0) stack->correctParams(3);
		else {
			stack->correctParams(4);
			channel = stack->pop()->getInt();
		}

		const char *filename = stack->pop()->getString();
		CScValue *valLooping = stack->pop();
		bool looping = valLooping->isNULL() ? true : valLooping->getBool();

		CScValue *valLoopStart = stack->pop();
		uint32 loopStart = (uint32)(valLoopStart->isNULL() ? 0 : valLoopStart->getInt());


		if (FAILED(playMusic(channel, filename, looping, loopStart))) stack->pushBool(false);
		else stack->pushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopMusic / StopMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopMusic") == 0 || strcmp(name, "StopMusicChannel") == 0) {
		int channel = 0;

		if (strcmp(name, "StopMusic") == 0) stack->correctParams(0);
		else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (FAILED(stopMusic(channel))) stack->pushBool(false);
		else stack->pushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseMusic / PauseMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PauseMusic") == 0 || strcmp(name, "PauseMusicChannel") == 0) {
		int channel = 0;

		if (strcmp(name, "PauseMusic") == 0) stack->correctParams(0);
		else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (FAILED(pauseMusic(channel))) stack->pushBool(false);
		else stack->pushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResumeMusic / ResumeMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ResumeMusic") == 0 || strcmp(name, "ResumeMusicChannel") == 0) {
		int channel = 0;
		if (strcmp(name, "ResumeMusic") == 0) stack->correctParams(0);
		else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (FAILED(resumeMusic(channel))) stack->pushBool(false);
		else stack->pushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetMusic / GetMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetMusic") == 0 || strcmp(name, "GetMusicChannel") == 0) {
		int channel = 0;
		if (strcmp(name, "GetMusic") == 0) stack->correctParams(0);
		else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}
		if (channel < 0 || channel >= NUM_MUSIC_CHANNELS) stack->pushNULL();
		else {
			if (!_music[channel] || !_music[channel]->_soundFilename) stack->pushNULL();
			else stack->pushString(_music[channel]->_soundFilename);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetMusicPosition / SetMusicChannelPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetMusicPosition") == 0 || strcmp(name, "SetMusicChannelPosition") == 0 || strcmp(name, "SetMusicPositionChannel") == 0) {
		int channel = 0;
		if (strcmp(name, "SetMusicPosition") == 0) stack->correctParams(1);
		else {
			stack->correctParams(2);
			channel = stack->pop()->getInt();
		}

		uint32 time = stack->pop()->getInt();

		if (FAILED(setMusicStartTime(channel, time))) stack->pushBool(false);
		else stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetMusicPosition / GetMusicChannelPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetMusicPosition") == 0 || strcmp(name, "GetMusicChannelPosition") == 0) {
		int channel = 0;
		if (strcmp(name, "GetMusicPosition") == 0) stack->correctParams(0);
		else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (channel < 0 || channel >= NUM_MUSIC_CHANNELS || !_music[channel]) stack->pushInt(0);
		else stack->pushInt(_music[channel]->getPositionTime());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsMusicPlaying / IsMusicChannelPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsMusicPlaying") == 0 || strcmp(name, "IsMusicChannelPlaying") == 0) {
		int channel = 0;
		if (strcmp(name, "IsMusicPlaying") == 0) stack->correctParams(0);
		else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (channel < 0 || channel >= NUM_MUSIC_CHANNELS || !_music[channel]) stack->pushBool(false);
		else stack->pushBool(_music[channel]->isPlaying());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetMusicVolume / SetMusicChannelVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetMusicVolume") == 0 || strcmp(name, "SetMusicChannelVolume") == 0) {
		int channel = 0;
		if (strcmp(name, "SetMusicVolume") == 0) stack->correctParams(1);
		else {
			stack->correctParams(2);
			channel = stack->pop()->getInt();
		}

		int volume = stack->pop()->getInt();
		if (channel < 0 || channel >= NUM_MUSIC_CHANNELS || !_music[channel]) stack->pushBool(false);
		else {
			if (FAILED(_music[channel]->setVolume(volume))) stack->pushBool(false);
			else stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetMusicVolume / GetMusicChannelVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetMusicVolume") == 0 || strcmp(name, "GetMusicChannelVolume") == 0) {
		int channel = 0;
		if (strcmp(name, "GetMusicVolume") == 0) stack->correctParams(0);
		else {
			stack->correctParams(1);
			channel = stack->pop()->getInt();
		}

		if (channel < 0 || channel >= NUM_MUSIC_CHANNELS || !_music[channel]) stack->pushInt(0);
		else stack->pushInt(_music[channel]->getVolume());

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MusicCrossfade
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MusicCrossfade") == 0) {
		stack->correctParams(4);
		int channel1 = stack->pop()->getInt(0);
		int channel2 = stack->pop()->getInt(0);
		uint32 fadeLength = (uint32)stack->pop()->getInt(0);
		bool swap = stack->pop()->getBool(true);

		if (_musicCrossfadeRunning) {
			script->runtimeError("Game.MusicCrossfade: Music crossfade is already in progress.");
			stack->pushBool(false);
			return S_OK;
		}

		_musicCrossfadeStartTime = _liveTimer;
		_musicCrossfadeChannel1 = channel1;
		_musicCrossfadeChannel2 = channel2;
		_musicCrossfadeLength = fadeLength;
		_musicCrossfadeSwap = swap;

		_musicCrossfadeRunning = true;

		stack->pushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSoundLength
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSoundLength") == 0) {
		stack->correctParams(1);

		int length = 0;
		const char *filename = stack->pop()->getString();

		CBSound *sound = new CBSound(Game);
		if (sound && SUCCEEDED(sound->setSound(filename, SOUND_MUSIC, true))) {
			length = sound->getLength();
			delete sound;
			sound = NULL;
		}
		stack->pushInt(length);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetMousePos
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetMousePos") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		x = MAX(x, 0);
		x = MIN(x, _renderer->_width);
		y = MAX(y, 0);
		y = MIN(y, _renderer->_height);
		POINT p;
		p.x = x + _renderer->_drawOffsetX;
		p.y = y + _renderer->_drawOffsetY;

		CBPlatform::SetCursorPos(p.x, p.y);

		stack->pushNULL();
		return S_OK;
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

		if (right < left) CBUtils::swap(&left, &right);
		if (bottom < top) CBUtils::swap(&top, &bottom);

		CBPlatform::SetRect(&_mouseLockRect, left, top, right, bottom);

		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayVideo
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlayVideo") == 0) {
		/*      stack->correctParams(0);
		        stack->pushBool(false);

		        return S_OK;
		        // TODO: ADDVIDEO
		        */

		Game->LOG(0, "Warning: Game.PlayVideo() is now deprecated. Use Game.PlayTheora() instead.");

		stack->correctParams(6);
		const char *filename = stack->pop()->getString();
		warning("PlayVideo: %s - not implemented yet", filename);
		CScValue *valType = stack->pop();
		int Type;
		if (valType->isNULL()) Type = (int)VID_PLAY_STRETCH;
		else Type = valType->getInt();

		int xVal = stack->pop()->getInt();
		int yVal = stack->pop()->getInt();
		bool FreezeMusic = stack->pop()->getBool(true);

		CScValue *valSub = stack->pop();
		const char *SubtitleFile = valSub->isNULL() ? NULL : valSub->getString();

		if (Type < (int)VID_PLAY_POS || Type > (int)VID_PLAY_CENTER)
			Type = (int)VID_PLAY_STRETCH;

		if (SUCCEEDED(Game->_videoPlayer->initialize(filename, SubtitleFile))) {
			if (SUCCEEDED(Game->_videoPlayer->play((TVideoPlayback)Type, xVal, yVal, FreezeMusic))) {
				stack->pushBool(true);
				script->sleep(0);
			} else stack->pushBool(false);
		} else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlayTheora") == 0) {
		/*      stack->correctParams(0);
		        stack->pushBool(false);

		        return S_OK;*/
		// TODO: ADDVIDEO

		stack->correctParams(7);
		const char *filename = stack->pop()->getString();
		CScValue *valType = stack->pop();
		int type;
		if (valType->isNULL())
			type = (int)VID_PLAY_STRETCH;
		else type = valType->getInt();

		int xVal = stack->pop()->getInt();
		int yVal = stack->pop()->getInt();
		bool freezeMusic = stack->pop()->getBool(true);
		bool dropFrames = stack->pop()->getBool(true);

		CScValue *valSub = stack->pop();
		const char *SubtitleFile = valSub->isNULL() ? NULL : valSub->getString();

		if (type < (int)VID_PLAY_POS || type > (int)VID_PLAY_CENTER) type = (int)VID_PLAY_STRETCH;

		delete _theoraPlayer;
		_theoraPlayer = new CVidTheoraPlayer(this);
		if (_theoraPlayer && SUCCEEDED(_theoraPlayer->initialize(filename, SubtitleFile))) {
			_theoraPlayer->_dontDropFrames = !dropFrames;
			if (SUCCEEDED(_theoraPlayer->play((TVideoPlayback)type, xVal, yVal, true, freezeMusic))) {
				stack->pushBool(true);
				script->sleep(0);
			} else stack->pushBool(false);
		} else {
			stack->pushBool(false);
			delete _theoraPlayer;
			_theoraPlayer = NULL;
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// QuitGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "QuitGame") == 0) {
		stack->correctParams(0);
		stack->pushNULL();
		_quitting = true;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegWriteNumber
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RegWriteNumber") == 0) {
		stack->correctParams(2);
		const char *key = stack->pop()->getString();
		int val = stack->pop()->getInt();
		_registry->writeInt("PrivateSettings", key, val);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegReadNumber
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RegReadNumber") == 0) {
		stack->correctParams(2);
		const char *key = stack->pop()->getString();
		int initVal = stack->pop()->getInt();
		stack->pushInt(_registry->readInt("PrivateSettings", key, initVal));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegWriteString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RegWriteString") == 0) {
		stack->correctParams(2);
		const char *key = stack->pop()->getString();
		const char *val = stack->pop()->getString();
		_registry->writeString("PrivateSettings", key, val);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegReadString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RegReadString") == 0) {
		stack->correctParams(2);
		const char *key = stack->pop()->getString();
		const char *initVal = stack->pop()->getString();
		AnsiString val = _registry->readString("PrivateSettings", key, initVal);
		stack->pushString(val.c_str());
		return S_OK;
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
		if (FAILED(SaveGame(slot, desc, quick))) {
			stack->pop();
			stack->pushBool(false);
		}
		delete [] desc;
		return S_OK;
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
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsSaveSlotUsed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsSaveSlotUsed") == 0) {
		stack->correctParams(1);
		int Slot = stack->pop()->getInt();
		stack->pushBool(isSaveSlotUsed(Slot));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSaveSlotDescription
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSaveSlotDescription") == 0) {
		stack->correctParams(1);
		int slot = stack->pop()->getInt();
		char desc[512];
		desc[0] = '\0';
		getSaveSlotDescription(slot, desc);
		stack->pushString(desc);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EmptySaveSlot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EmptySaveSlot") == 0) {
		stack->correctParams(1);
		int slot = stack->pop()->getInt();
		emptySaveSlot(slot);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalSFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetGlobalSFXVolume") == 0) {
		stack->correctParams(1);
		Game->_soundMgr->setVolumePercent(SOUND_SFX, (byte)stack->pop()->getInt());
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalSpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetGlobalSpeechVolume") == 0) {
		stack->correctParams(1);
		Game->_soundMgr->setVolumePercent(SOUND_SPEECH, (byte)stack->pop()->getInt());
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalMusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetGlobalMusicVolume") == 0) {
		stack->correctParams(1);
		Game->_soundMgr->setVolumePercent(SOUND_MUSIC, (byte)stack->pop()->getInt());
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalMasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetGlobalMasterVolume") == 0) {
		stack->correctParams(1);
		Game->_soundMgr->setMasterVolumePercent((byte)stack->pop()->getInt());
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalSFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetGlobalSFXVolume") == 0) {
		stack->correctParams(0);
		stack->pushInt(_soundMgr->getVolumePercent(SOUND_SFX));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalSpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetGlobalSpeechVolume") == 0) {
		stack->correctParams(0);
		stack->pushInt(_soundMgr->getVolumePercent(SOUND_SPEECH));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalMusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetGlobalMusicVolume") == 0) {
		stack->correctParams(0);
		stack->pushInt(_soundMgr->getVolumePercent(SOUND_MUSIC));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalMasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetGlobalMasterVolume") == 0) {
		stack->correctParams(0);
		stack->pushInt(_soundMgr->getMasterVolumePercent());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetActiveCursor") == 0) {
		stack->correctParams(1);
		if (SUCCEEDED(setActiveCursor(stack->pop()->getString()))) stack->pushBool(true);
		else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetActiveCursor") == 0) {
		stack->correctParams(0);
		if (!_activeCursor || !_activeCursor->_filename) stack->pushNULL();
		else stack->pushString(_activeCursor->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetActiveCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetActiveCursorObject") == 0) {
		stack->correctParams(0);
		if (!_activeCursor) stack->pushNULL();
		else stack->pushNative(_activeCursor, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveActiveCursor") == 0) {
		stack->correctParams(0);
		delete _activeCursor;
		_activeCursor = NULL;
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HasActiveCursor") == 0) {
		stack->correctParams(0);

		if (_activeCursor) stack->pushBool(true);
		else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FileExists
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FileExists") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();

		// TODO: Replace with fileExists
		Common::SeekableReadStream *file = _fileManager->openFile(filename, false);
		if (!file) stack->pushBool(false);
		else {
			_fileManager->closeFile(file);
			stack->pushBool(true);
		}
		return S_OK;
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

		bool system = (strcmp(name, "SystemFadeOut") == 0 || strcmp(name, "SystemFadeOutAsync") == 0);

		_fader->fadeOut(DRGBA(red, green, blue, alpha), duration, system);
		if (strcmp(name, "FadeOutAsync") != 0 && strcmp(name, "SystemFadeOutAsync") != 0) script->waitFor(_fader);

		stack->pushNULL();
		return S_OK;
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

		bool System = (strcmp(name, "SystemFadeIn") == 0 || strcmp(name, "SystemFadeInAsync") == 0);

		_fader->fadeIn(DRGBA(red, green, blue, alpha), duration, system);
		if (strcmp(name, "FadeInAsync") != 0 && strcmp(name, "SystemFadeInAsync") != 0) script->waitFor(_fader);

		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFadeColor") == 0) {
		stack->correctParams(0);
		stack->pushInt(_fader->getCurrentColor());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Screenshot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Screenshot") == 0) {
		stack->correctParams(1);
		char filename[MAX_PATH];

		CScValue *Val = stack->pop();

		warning("BGame::ScCallMethod - Screenshot not reimplemented"); //TODO
		int fileNum = 0;

		while (true) {
			sprintf(filename, "%s%03d.bmp", Val->isNULL() ? _name : Val->getString(), fileNum);
			if (!Common::File::exists(filename))
				break;
			fileNum++;
		}

		bool ret = false;
		CBImage *image = Game->_renderer->takeScreenshot();
		if (image) {
			ret = SUCCEEDED(image->saveBMPFile(filename));
			delete image;
		} else ret = false;

		stack->pushBool(ret);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScreenshotEx
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScreenshotEx") == 0) {
		stack->correctParams(3);
		const char *filename = stack->pop()->getString();
		int sizeX = stack->pop()->getInt(_renderer->_width);
		int sizeY = stack->pop()->getInt(_renderer->_height);

		bool ret = false;
		CBImage *image = Game->_renderer->takeScreenshot();
		if (image) {
			ret = SUCCEEDED(image->resize(sizeX, sizeY));
			if (ret) ret = SUCCEEDED(image->saveBMPFile(filename));
			delete image;
		} else ret = false;

		stack->pushBool(ret);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateWindow") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();

		CUIWindow *win = new CUIWindow(Game);
		_windows.Add(win);
		registerObject(win);
		if (!val->isNULL()) win->setName(val->getString());
		stack->pushNative(win, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteWindow") == 0) {
		stack->correctParams(1);
		CBObject *obj = (CBObject *)stack->pop()->getNative();
		for (int i = 0; i < _windows.GetSize(); i++) {
			if (_windows[i] == obj) {
				unregisterObject(_windows[i]);
				stack->pushBool(true);
				return S_OK;
			}
		}
		stack->pushBool(false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OpenDocument
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OpenDocument") == 0) {
		stack->correctParams(0);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DEBUG_DumpClassRegistry
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DEBUG_DumpClassRegistry") == 0) {
		stack->correctParams(0);
		DEBUG_DumpClassRegistry();
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetLoadingScreen
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetLoadingScreen") == 0) {
		stack->correctParams(3);
		CScValue *val = stack->pop();
		_loadImageX = stack->pop()->getInt();
		_loadImageY = stack->pop()->getInt();

		if (val->isNULL()) {
			delete[] _loadImageName;
			_loadImageName = NULL;
		} else {
			CBUtils::setString(&_loadImageName, val->getString());
		}
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSavingScreen
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetSavingScreen") == 0) {
		stack->correctParams(3);
		CScValue *val = stack->pop();
		_saveImageX = stack->pop()->getInt();
		_saveImageY = stack->pop()->getInt();

		if (val->isNULL()) {
			delete[] _saveImageName;
			_saveImageName = NULL;
		} else {
			CBUtils::setString(&_saveImageName, val->getString());
		}
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetWaitCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetWaitCursor") == 0) {
		stack->correctParams(1);
		if (SUCCEEDED(setWaitCursor(stack->pop()->getString()))) stack->pushBool(true);
		else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveWaitCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveWaitCursor") == 0) {
		stack->correctParams(0);
		delete _cursorNoninteractive;
		_cursorNoninteractive = NULL;

		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetWaitCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetWaitCursor") == 0) {
		stack->correctParams(0);
		if (!_cursorNoninteractive || !_cursorNoninteractive->_filename) stack->pushNULL();
		else stack->pushString(_cursorNoninteractive->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetWaitCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetWaitCursorObject") == 0) {
		stack->correctParams(0);
		if (!_cursorNoninteractive) stack->pushNULL();
		else stack->pushNative(_cursorNoninteractive, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClearScriptCache
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ClearScriptCache") == 0) {
		stack->correctParams(0);
		stack->pushBool(SUCCEEDED(_scEngine->emptyScriptCache()));
		return S_OK;
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
		_loadingIcon = new CBSprite(this);
		if (!_loadingIcon || FAILED(_loadingIcon->loadFile(filename))) {
			delete _loadingIcon;
			_loadingIcon = NULL;
		} else {
			displayContent(false, true);
			Game->_renderer->flip();
			Game->_renderer->initLoop();
		}
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HideLoadingIcon
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HideLoadingIcon") == 0) {
		stack->correctParams(0);
		delete _loadingIcon;
		_loadingIcon = NULL;
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DumpTextureStats
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DumpTextureStats") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();

		_renderer->dumpData(filename);

		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccOutputText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccOutputText") == 0) {
		stack->correctParams(2);
		/* const char *Str = */
		stack->pop()->getString();
		/* int Type = */
		stack->pop()->getInt();
		// do nothing
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StoreSaveThumbnail
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StoreSaveThumbnail") == 0) {
		stack->correctParams(0);
		delete _cachedThumbnail;
		_cachedThumbnail = new CBSaveThumbHelper(this);
		if (FAILED(_cachedThumbnail->storeThumbnail())) {
			delete _cachedThumbnail;
			_cachedThumbnail = NULL;
			stack->pushBool(false);
		} else stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteSaveThumbnail
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteSaveThumbnail") == 0) {
		stack->correctParams(0);
		delete _cachedThumbnail;
		_cachedThumbnail = NULL;
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFileChecksum
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFileChecksum") == 0) {
		stack->correctParams(2);
		const char *filename = stack->pop()->getString();
		bool asHex = stack->pop()->getBool(false);

		Common::SeekableReadStream *file = _fileManager->openFile(filename, false);
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
				char Hex[100];
				sprintf(Hex, "%x", checksum);
				stack->pushString(Hex);
			} else
				stack->pushInt(checksum);

			_fileManager->closeFile(file);
			file = NULL;
		} else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EnableScriptProfiling
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EnableScriptProfiling") == 0) {
		stack->correctParams(0);
		_scEngine->enableProfiling();
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisableScriptProfiling
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisableScriptProfiling") == 0) {
		stack->correctParams(0);
		_scEngine->disableProfiling();
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ShowStatusLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ShowStatusLine") == 0) {
		stack->correctParams(0);
#ifdef __IPHONEOS__
		IOS_ShowStatusLine(TRUE);
#endif
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HideStatusLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HideStatusLine") == 0) {
		stack->correctParams(0);
#ifdef __IPHONEOS__
		IOS_ShowStatusLine(FALSE);
#endif
		stack->pushNULL();

		return S_OK;
	}

	else return CBObject::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBGame::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("game");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Name") == 0) {
		_scValue->setString(_name);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Hwnd (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Hwnd") == 0) {
		_scValue->setInt((int)_renderer->_window);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CurrentTime (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CurrentTime") == 0) {
		_scValue->setInt((int)_timer);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WindowsTime (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WindowsTime") == 0) {
		_scValue->setInt((int)CBPlatform::GetTime());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WindowedMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WindowedMode") == 0) {
		_scValue->setBool(_renderer->_windowed);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MouseX") == 0) {
		_scValue->setInt(_mousePos.x);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MouseY") == 0) {
		_scValue->setInt(_mousePos.y);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MainObject") == 0) {
		_scValue->setNative(_mainObject, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ActiveObject (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ActiveObject") == 0) {
		_scValue->setNative(_activeObject, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScreenWidth (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScreenWidth") == 0) {
		_scValue->setInt(_renderer->_width);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScreenHeight (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScreenHeight") == 0) {
		_scValue->setInt(_renderer->_height);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Interactive") == 0) {
		_scValue->setBool(_interactive);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DebugMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DebugMode") == 0) {
		_scValue->setBool(_dEBUG_DebugMode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundAvailable (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundAvailable") == 0) {
		_scValue->setBool(_soundMgr->_soundAvailable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SFXVolume") == 0) {
		Game->LOG(0, "**Warning** The SFXVolume attribute is obsolete");
		_scValue->setInt(_soundMgr->getVolumePercent(SOUND_SFX));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SpeechVolume") == 0) {
		Game->LOG(0, "**Warning** The SpeechVolume attribute is obsolete");
		_scValue->setInt(_soundMgr->getVolumePercent(SOUND_SPEECH));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MusicVolume") == 0) {
		Game->LOG(0, "**Warning** The MusicVolume attribute is obsolete");
		_scValue->setInt(_soundMgr->getVolumePercent(SOUND_MUSIC));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MasterVolume") == 0) {
		Game->LOG(0, "**Warning** The MasterVolume attribute is obsolete");
		_scValue->setInt(_soundMgr->getMasterVolumePercent());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyboard (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Keyboard") == 0) {
		if (_keyboardState) _scValue->setNative(_keyboardState, true);
		else _scValue->setNULL();

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Subtitles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Subtitles") == 0) {
		_scValue->setBool(_subtitles);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesSpeed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesSpeed") == 0) {
		_scValue->setInt(_subtitlesSpeed);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// VideoSubtitles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "VideoSubtitles") == 0) {
		_scValue->setBool(_videoSubtitles);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FPS (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FPS") == 0) {
		_scValue->setInt(_fps);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AcceleratedMode / Accelerated (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AcceleratedMode") == 0 || strcmp(name, "Accelerated") == 0) {
		_scValue->setBool(_useD3D);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextEncoding
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TextEncoding") == 0) {
		_scValue->setInt(_textEncoding);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextRTL
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TextRTL") == 0) {
		_scValue->setBool(_textRTL);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundBufferSize
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundBufferSize") == 0) {
		_scValue->setInt(_soundBufferSizeSec);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuspendedRendering
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SuspendedRendering") == 0) {
		_scValue->setBool(_suspendedRendering);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuppressScriptErrors
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SuppressScriptErrors") == 0) {
		_scValue->setBool(_suppressScriptErrors);
		return _scValue;
	}


	//////////////////////////////////////////////////////////////////////////
	// Frozen
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Frozen") == 0) {
		_scValue->setBool(_state == GAME_FROZEN);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSEnabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccTTSEnabled") == 0) {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSTalk
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccTTSTalk") == 0) {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSCaptions
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccTTSCaptions") == 0) {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSKeypress
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccTTSKeypress") == 0) {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccKeyboardEnabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccKeyboardEnabled") == 0) {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccKeyboardCursorSkip
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccKeyboardCursorSkip") == 0) {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccKeyboardPause
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccKeyboardPause") == 0) {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutorunDisabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutorunDisabled") == 0) {
		_scValue->setBool(_autorunDisabled);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveDirectory (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SaveDirectory") == 0) {
		AnsiString dataDir = getDataDir();
		_scValue->setString(dataDir.c_str());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveOnExit
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutoSaveOnExit") == 0) {
		_scValue->setBool(_autoSaveOnExit);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveSlot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutoSaveSlot") == 0) {
		_scValue->setInt(_autoSaveSlot);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorHidden
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorHidden") == 0) {
		_scValue->setBool(_cursorHidden);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Platform (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Platform") == 0) {
		_scValue->setString(CBPlatform::GetPlatformName().c_str());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeviceType (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeviceType") == 0) {
		_scValue->setString(getDeviceType().c_str());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MostRecentSaveSlot (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MostRecentSaveSlot") == 0) {
		_scValue->setInt(_registry->readInt("System", "MostRecentSaveSlot", -1));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Store (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Store") == 0) {
		if (_store) _scValue->setNative(_store, true);
		else _scValue->setNULL();

		return _scValue;
	}

	else return CBObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MouseX") == 0) {
		_mousePos.x = value->getInt();
		resetMousePos();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MouseY") == 0) {
		_mousePos.y = value->getInt();
		resetMousePos();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Name") == 0) {
		HRESULT res = CBObject::scSetProperty(name, value);
		setWindowTitle();
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MainObject") == 0) {
		CBScriptable *obj = value->getNative();
		if (obj == NULL || validObject((CBObject *)obj)) _mainObject = (CBObject *)obj;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Interactive") == 0) {
		setInteractive(value->getBool());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SFXVolume") == 0) {
		Game->LOG(0, "**Warning** The SFXVolume attribute is obsolete");
		Game->_soundMgr->setVolumePercent(SOUND_SFX, (byte)value->getInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SpeechVolume") == 0) {
		Game->LOG(0, "**Warning** The SpeechVolume attribute is obsolete");
		Game->_soundMgr->setVolumePercent(SOUND_SPEECH, (byte)value->getInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MusicVolume") == 0) {
		Game->LOG(0, "**Warning** The MusicVolume attribute is obsolete");
		Game->_soundMgr->setVolumePercent(SOUND_MUSIC, (byte)value->getInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MasterVolume") == 0) {
		Game->LOG(0, "**Warning** The MasterVolume attribute is obsolete");
		Game->_soundMgr->setMasterVolumePercent((byte)value->getInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Subtitles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Subtitles") == 0) {
		_subtitles = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesSpeed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesSpeed") == 0) {
		_subtitlesSpeed = value->getInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// VideoSubtitles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "VideoSubtitles") == 0) {
		_videoSubtitles = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextEncoding
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TextEncoding") == 0) {
		int Enc = value->getInt();
		if (Enc < 0) Enc = 0;
		if (Enc >= NUM_TEXT_ENCODINGS) Enc = NUM_TEXT_ENCODINGS - 1;
		_textEncoding = (TTextEncoding)Enc;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextRTL
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TextRTL") == 0) {
		_textRTL = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundBufferSize
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundBufferSize") == 0) {
		_soundBufferSizeSec = value->getInt();
		_soundBufferSizeSec = MAX(3, _soundBufferSizeSec);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuspendedRendering
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SuspendedRendering") == 0) {
		_suspendedRendering = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuppressScriptErrors
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SuppressScriptErrors") == 0) {
		_suppressScriptErrors = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutorunDisabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutorunDisabled") == 0) {
		_autorunDisabled = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveOnExit
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutoSaveOnExit") == 0) {
		_autoSaveOnExit = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveSlot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutoSaveSlot") == 0) {
		_autoSaveSlot = value->getInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorHidden
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorHidden") == 0) {
		_cursorHidden = value->getBool();
		return S_OK;
	}

	else return CBObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CBGame::scToString() {
	return "[game object]";
}



#define QUICK_MSG_DURATION 3000
//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::displayQuickMsg() {
	if (_quickMessages.GetSize() == 0 || !_systemFont) return S_OK;

	// update
	for (int i = 0; i < _quickMessages.GetSize(); i++) {
		if (_currentTime - _quickMessages[i]->_startTime >= QUICK_MSG_DURATION) {
			delete _quickMessages[i];
			_quickMessages.RemoveAt(i);
			i--;
		}
	}

	int posY = 20;

	// display
	for (int i = 0; i < _quickMessages.GetSize(); i++) {
		_systemFont->drawText((byte *)_quickMessages[i]->getText(), 0, posY, _renderer->_width);
		posY += _systemFont->getTextHeight((byte *)_quickMessages[i]->getText(), _renderer->_width);
	}
	return S_OK;
}


#define MAX_QUICK_MSG 5
//////////////////////////////////////////////////////////////////////////
void CBGame::quickMessage(const char *text) {
	if (_quickMessages.GetSize() >= MAX_QUICK_MSG) {
		delete _quickMessages[0];
		_quickMessages.RemoveAt(0);
	}
	_quickMessages.Add(new CBQuickMsg(Game, text));
}


//////////////////////////////////////////////////////////////////////////
void CBGame::quickMessageForm(LPSTR fmt, ...) {
	char buff[256];
	va_list va;

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	quickMessage(buff);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::registerObject(CBObject *object) {
	_regObjects.Add(object);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::unregisterObject(CBObject *object) {
	if (!object) return S_OK;

	// is it a window?
	for (int i = 0; i < _windows.GetSize(); i++) {
		if ((CBObject *)_windows[i] == object) {
			_windows.RemoveAt(i);

			// get new focused window
			if (_focusedWindow == object) _focusedWindow = NULL;

			break;
		}
	}

	// is it active object?
	if (_activeObject == object) _activeObject = NULL;

	// is it main object?
	if (_mainObject == object) _mainObject = NULL;

	if (_store) _store->OnObjectDestroyed(object);

	// destroy object
	for (int i = 0; i < _regObjects.GetSize(); i++) {
		if (_regObjects[i] == object) {
			_regObjects.RemoveAt(i);
			if (!_loadInProgress) CSysClassRegistry::getInstance()->enumInstances(invalidateValues, "CScValue", (void *)object);
			delete object;
			return S_OK;
		}
	}

	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::invalidateValues(void *value, void *data) {
	CScValue *val = (CScValue *)value;
	if (val->isNative() && val->getNative() == data) {
		if (!val->_persistent && ((CBScriptable *)data)->_refCount == 1) {
			((CBScriptable *)data)->_refCount++;
		}
		val->setNative(NULL);
		val->setNULL();
	}
}



//////////////////////////////////////////////////////////////////////////
bool CBGame::validObject(CBObject *object) {
	if (!object) return false;
	if (object == this) return true;

	for (int i = 0; i < _regObjects.GetSize(); i++) {
		if (_regObjects[i] == object) return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::ExternalCall(CScScript *script, CScStack *stack, CScStack *thisStack, char *name) {
	CScValue *thisObj;

	//////////////////////////////////////////////////////////////////////////
	// LOG
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "LOG") == 0) {
		stack->correctParams(1);
		Game->LOG(0, "sc: %s", stack->pop()->getString());
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// String
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "String") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXString(Game, stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// MemBuffer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MemBuffer") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXMemBuffer(Game, stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// File
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "File") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXFile(Game, stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Date
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Date") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXDate(Game, stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Array
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Array") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXArray(Game, stack));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Object
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Object") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXObject(Game, stack));
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

		CBScriptable *obj = stack->pop()->getNative();
		if (validObject((CBObject *)obj)) script->waitForExclusive((CBObject *)obj);
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Random
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Random") == 0) {
		stack->correctParams(2);

		int from = stack->pop()->getInt();
		int to   = stack->pop()->getInt();

		stack->pushInt(CBUtils::randomInt(from, to));
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
		CScValue *val = stack->pop();
		if (val->isNULL()) a = 255;
		else a = val->getInt();

		stack->pushInt(DRGBA(r, g, b, a));
	}

	//////////////////////////////////////////////////////////////////////////
	// MakeHSL
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MakeHSL") == 0) {
		stack->correctParams(3);
		int h = stack->pop()->getInt();
		int s = stack->pop()->getInt();
		int l = stack->pop()->getInt();

		stack->pushInt(CBUtils::HSLtoRGB(h, s, l));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetRValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetRValue") == 0) {
		stack->correctParams(1);

		uint32 rgba = (uint32)stack->pop()->getInt();
		stack->pushInt(D3DCOLGetR(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetGValue") == 0) {
		stack->correctParams(1);

		uint32 rgba = (uint32)stack->pop()->getInt();
		stack->pushInt(D3DCOLGetG(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetBValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetBValue") == 0) {
		stack->correctParams(1);

		uint32 rgba = (uint32)stack->pop()->getInt();
		stack->pushInt(D3DCOLGetB(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetAValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetAValue") == 0) {
		stack->correctParams(1);

		uint32 rgba = (uint32)stack->pop()->getInt();
		stack->pushInt(D3DCOLGetA(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHValue") == 0) {
		stack->correctParams(1);
		uint32 rgb = (uint32)stack->pop()->getInt();

		byte H, S, L;
		CBUtils::RGBtoHSL(rgb, &H, &S, &L);
		stack->pushInt(H);
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSValue") == 0) {
		stack->correctParams(1);
		uint32 rgb = (uint32)stack->pop()->getInt();

		byte H, S, L;
		CBUtils::RGBtoHSL(rgb, &H, &S, &L);
		stack->pushInt(S);
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetLValue") == 0) {
		stack->correctParams(1);
		uint32 rgb = (uint32)stack->pop()->getInt();

		byte H, S, L;
		CBUtils::RGBtoHSL(rgb, &H, &S, &L);
		stack->pushInt(L);
	}

	//////////////////////////////////////////////////////////////////////////
	// Debug
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Debug") == 0) {
		stack->correctParams(0);

		if (Game->getDebugMgr()->_enabled) {
			Game->getDebugMgr()->onScriptHitBreakpoint(script);
			script->sleep(0);
		}
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
		delete [] str2;
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

	//////////////////////////////////////////////////////////////////////////
	// failure
	else {
		script->runtimeError("Call to undefined function '%s'. Ignored.", name);
		stack->correctParams(0);
		stack->pushNULL();
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::showCursor() {
	if (_cursorHidden) return S_OK;

	if (!_interactive && Game->_state == GAME_RUNNING) {
		if (_cursorNoninteractive) return drawCursor(_cursorNoninteractive);
	} else {
		if (_activeObject && !FAILED(_activeObject->showCursor())) return S_OK;
		else {
			if (_activeObject && _activeCursor && _activeObject->getExtendedFlag("usable")) return drawCursor(_activeCursor);
			else if (_cursor) return drawCursor(_cursor);
		}
	}
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::SaveGame(int slot, const char *desc, bool quickSave) {
	char filename[MAX_PATH + 1];
	getSaveSlotFilename(slot, filename);

	LOG(0, "Saving game '%s'...", filename);

	Game->applyEvent("BeforeSave", true);

	HRESULT ret;

	_indicatorDisplay = true;
	_indicatorProgress = 0;
	CBPersistMgr *pm = new CBPersistMgr(Game);
	if (FAILED(ret = pm->initSave(desc))) goto save_finish;

	if (!quickSave) {
		delete _saveLoadImage;
		_saveLoadImage = NULL;
		if (_saveImageName) {
			_saveLoadImage = new CBSurfaceSDL(this);

			if (!_saveLoadImage || FAILED(_saveLoadImage->create(_saveImageName, true, 0, 0, 0))) {
				delete _saveLoadImage;
				_saveLoadImage = NULL;
			}
		}
	}

	if (FAILED(ret = CSysClassRegistry::getInstance()->saveTable(Game, pm, quickSave))) goto save_finish;
	if (FAILED(ret = CSysClassRegistry::getInstance()->saveInstances(Game, pm, quickSave))) goto save_finish;
	if (FAILED(ret = pm->saveFile(filename))) goto save_finish;

	_registry->writeInt("System", "MostRecentSaveSlot", slot);

save_finish: // TODO: Remove gotos
	delete pm;
	_indicatorDisplay = false;

	delete _saveLoadImage;
	_saveLoadImage = NULL;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::loadGame(int slot) {
	//Game->LOG(0, "Load start %d", CBUtils::GetUsedMemMB());

	_loading = false;
	_scheduledLoadSlot = -1;

	char filename[MAX_PATH + 1];
	getSaveSlotFilename(slot, filename);

	return loadGame(filename);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::loadGame(const char *filename) {
	LOG(0, "Loading game '%s'...", filename);
	getDebugMgr()->onGameShutdown();

	HRESULT ret;

	delete _saveLoadImage;
	_saveLoadImage = NULL;
	if (_loadImageName) {
		_saveLoadImage = new CBSurfaceSDL(this);

		if (!_saveLoadImage || FAILED(_saveLoadImage->create(_loadImageName, true, 0, 0, 0))) {
			delete _saveLoadImage;
			_saveLoadImage = NULL;
		}
	}


	_loadInProgress = true;
	_indicatorDisplay = true;
	_indicatorProgress = 0;
	CBPersistMgr *pm = new CBPersistMgr(Game);
	_dEBUG_AbsolutePathWarning = false;
	if (FAILED(ret = pm->initLoad(filename))) goto load_finish;

	//if(FAILED(ret = cleanup())) goto load_finish;
	if (FAILED(ret = CSysClassRegistry::getInstance()->loadTable(Game, pm))) goto load_finish;
	if (FAILED(ret = CSysClassRegistry::getInstance()->loadInstances(Game, pm))) goto load_finish;

	// data initialization after load
	initAfterLoad();

	Game->applyEvent("AfterLoad", true);

	displayContent(true, false);
	//_renderer->flip();

	getDebugMgr()->onGameInit();

load_finish:
	_dEBUG_AbsolutePathWarning = true;

	_indicatorDisplay = false;
	delete pm;
	_loadInProgress = false;

	delete _saveLoadImage;
	_saveLoadImage = NULL;

	//Game->LOG(0, "Load end %d", CBUtils::GetUsedMemMB());

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::initAfterLoad() {
	CSysClassRegistry::getInstance()->enumInstances(afterLoadRegion,   "CBRegion",   NULL);
	CSysClassRegistry::getInstance()->enumInstances(afterLoadSubFrame, "CBSubFrame", NULL);
	CSysClassRegistry::getInstance()->enumInstances(afterLoadSound,    "CBSound",    NULL);
	CSysClassRegistry::getInstance()->enumInstances(afterLoadFont,     "CBFontTT",   NULL);
	CSysClassRegistry::getInstance()->enumInstances(afterLoadScript,   "CScScript",  NULL);

	_scEngine->refreshScriptBreakpoints();
	if (_store) _store->afterLoad();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBGame::afterLoadRegion(void *region, void *data) {
	((CBRegion *)region)->createRegion();
}


//////////////////////////////////////////////////////////////////////////
void CBGame::afterLoadSubFrame(void *subframe, void *data) {
	((CBSubFrame *)subframe)->setSurfaceSimple();
}


//////////////////////////////////////////////////////////////////////////
void CBGame::afterLoadSound(void *sound, void *data) {
	((CBSound *)sound)->setSoundSimple();
}

//////////////////////////////////////////////////////////////////////////
void CBGame::afterLoadFont(void *font, void *data) {
	((CBFont *)font)->afterLoad();
}

//////////////////////////////////////////////////////////////////////////
void CBGame::afterLoadScript(void *script, void *data) {
	((CScScript *)script)->afterLoad();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::displayWindows(bool inGame) {
	HRESULT res;

	// did we lose focus? focus topmost window
	if (_focusedWindow == NULL || !_focusedWindow->_visible || _focusedWindow->_disable) {
		_focusedWindow = NULL;
		for (int i = _windows.GetSize() - 1; i >= 0; i--) {
			if (_windows[i]->_visible && !_windows[i]->_disable) {
				_focusedWindow = _windows[i];
				break;
			}
		}
	}

	// display all windows
	for (int i = 0; i < _windows.GetSize(); i++) {
		if (_windows[i]->_visible && _windows[i]->_inGame == inGame) {

			res = _windows[i]->display();
			if (FAILED(res)) return res;
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::playMusic(int channel, const char *filename, bool looping, uint32 loopStart) {
	if (channel >= NUM_MUSIC_CHANNELS) {
		Game->LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", channel, NUM_MUSIC_CHANNELS);
		return E_FAIL;
	}

	delete _music[channel];
	_music[channel] = NULL;

	_music[channel] = new CBSound(Game);
	if (_music[channel] && SUCCEEDED(_music[channel]->setSound(filename, SOUND_MUSIC, true))) {
		if (_musicStartTime[channel]) {
			_music[channel]->setPositionTime(_musicStartTime[channel]);
			_musicStartTime[channel] = 0;
		}
		if (loopStart) _music[channel]->setLoopStart(loopStart);
		return _music[channel]->play(looping);
	} else {
		delete _music[channel];
		_music[channel] = NULL;
		return E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::stopMusic(int channel) {
	if (channel >= NUM_MUSIC_CHANNELS) {
		Game->LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", channel, NUM_MUSIC_CHANNELS);
		return E_FAIL;
	}

	if (_music[channel]) {
		_music[channel]->stop();
		delete _music[channel];
		_music[channel] = NULL;
		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::pauseMusic(int channel) {
	if (channel >= NUM_MUSIC_CHANNELS) {
		Game->LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", channel, NUM_MUSIC_CHANNELS);
		return E_FAIL;
	}

	if (_music[channel]) return _music[channel]->pause();
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::resumeMusic(int channel) {
	if (channel >= NUM_MUSIC_CHANNELS) {
		Game->LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", channel, NUM_MUSIC_CHANNELS);
		return E_FAIL;
	}

	if (_music[channel]) return _music[channel]->resume();
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::setMusicStartTime(int channel, uint32 time) {
	if (channel >= NUM_MUSIC_CHANNELS) {
		Game->LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", channel, NUM_MUSIC_CHANNELS);
		return E_FAIL;
	}

	_musicStartTime[channel] = time;
	if (_music[channel] && _music[channel]->isPlaying()) return _music[channel]->setPositionTime(time);
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::loadSettings(const char *filename) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(SETTINGS)
	TOKEN_TABLE(GAME)
	TOKEN_TABLE(STRING_TABLE)
	TOKEN_TABLE(RESOLUTION)
	TOKEN_TABLE(REQUIRE_3D_ACCELERATION)
	TOKEN_TABLE(REQUIRE_SOUND)
	TOKEN_TABLE(HWTL_MODE)
	TOKEN_TABLE(ALLOW_WINDOWED_MODE)
	TOKEN_TABLE(ALLOW_ACCESSIBILITY_TAB)
	TOKEN_TABLE(ALLOW_ABOUT_TAB)
	TOKEN_TABLE(ALLOW_ADVANCED)
	TOKEN_TABLE(ALLOW_DESKTOP_RES)
	TOKEN_TABLE(REGISTRY_PATH)
	TOKEN_TABLE(RICH_SAVED_GAMES)
	TOKEN_TABLE(SAVED_GAME_EXT)
	TOKEN_TABLE(GUID)
	TOKEN_TABLE_END


	byte *origBuffer = Game->_fileManager->readWholeFile(filename);
	if (origBuffer == NULL) {
		Game->LOG(0, "CBGame::LoadSettings failed for file '%s'", filename);
		return E_FAIL;
	}

	HRESULT ret = S_OK;

	byte *buffer = origBuffer;
	byte *params;
	int cmd;
	CBParser parser(Game);

	if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_SETTINGS) {
		Game->LOG(0, "'SETTINGS' keyword expected in game settings file.");
		return E_FAIL;
	}
	buffer = params;
	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_GAME:
			delete[] _settingsGameFile;
			_settingsGameFile = new char[strlen((char *)params) + 1];
			if (_settingsGameFile) strcpy(_settingsGameFile, (char *)params);
			break;

		case TOKEN_STRING_TABLE:
			if (FAILED(_stringTable->loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_RESOLUTION:
			parser.scanStr((char *)params, "%d,%d", &_settingsResWidth, &_settingsResHeight);
			break;

		case TOKEN_REQUIRE_3D_ACCELERATION:
			parser.scanStr((char *)params, "%b", &_settingsRequireAcceleration);
			break;

		case TOKEN_REQUIRE_SOUND:
			parser.scanStr((char *)params, "%b", &_settingsRequireSound);
			break;

		case TOKEN_HWTL_MODE:
			parser.scanStr((char *)params, "%d", &_settingsTLMode);
			break;

		case TOKEN_ALLOW_WINDOWED_MODE:
			parser.scanStr((char *)params, "%b", &_settingsAllowWindowed);
			break;

		case TOKEN_ALLOW_DESKTOP_RES:
			parser.scanStr((char *)params, "%b", &_settingsAllowDesktopRes);
			break;

		case TOKEN_ALLOW_ADVANCED:
			parser.scanStr((char *)params, "%b", &_settingsAllowAdvanced);
			break;

		case TOKEN_ALLOW_ACCESSIBILITY_TAB:
			parser.scanStr((char *)params, "%b", &_settingsAllowAccessTab);
			break;

		case TOKEN_ALLOW_ABOUT_TAB:
			parser.scanStr((char *)params, "%b", &_settingsAllowAboutTab);
			break;

		case TOKEN_REGISTRY_PATH:
			_registry->setBasePath((char *)params);
			break;

		case TOKEN_RICH_SAVED_GAMES:
			parser.scanStr((char *)params, "%b", &_richSavedGames);
			break;

		case TOKEN_SAVED_GAME_EXT:
			CBUtils::setString(&_savedGameExt, (char *)params);
			break;

		case TOKEN_GUID:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in game settings '%s'", filename);
		ret = E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading game settings '%s'", filename);
		ret = E_FAIL;
	}

	_settingsAllowWindowed = _registry->readBool("Debug", "AllowWindowed", _settingsAllowWindowed);
	_compressedSavegames = _registry->readBool("Debug", "CompressedSavegames", _compressedSavegames);
	//_compressedSavegames = false;

	delete [] origBuffer;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::persist(CBPersistMgr *persistMgr) {
	if (!persistMgr->_saving)
		cleanup();

	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_activeObject));
	persistMgr->transfer(TMEMBER(_capturedObject));
	persistMgr->transfer(TMEMBER(_cursorNoninteractive));
	persistMgr->transfer(TMEMBER(_doNotExpandStrings));
	persistMgr->transfer(TMEMBER(_editorMode));
	persistMgr->transfer(TMEMBER(_fader));
	persistMgr->transfer(TMEMBER(_freezeLevel));
	persistMgr->transfer(TMEMBER(_focusedWindow));
	persistMgr->transfer(TMEMBER(_fontStorage));
	persistMgr->transfer(TMEMBER(_interactive));
	persistMgr->transfer(TMEMBER(_keyboardState));
	persistMgr->transfer(TMEMBER(_lastTime));
	persistMgr->transfer(TMEMBER(_mainObject));
	for (int i = 0; i < NUM_MUSIC_CHANNELS; i++) {
		persistMgr->transfer(TMEMBER(_music[i]));
		persistMgr->transfer(TMEMBER(_musicStartTime[i]));
	}

	persistMgr->transfer(TMEMBER(_offsetX));
	persistMgr->transfer(TMEMBER(_offsetY));
	persistMgr->transfer(TMEMBER(_offsetPercentX));
	persistMgr->transfer(TMEMBER(_offsetPercentY));

	persistMgr->transfer(TMEMBER(_origInteractive));
	persistMgr->transfer(TMEMBER_INT(_origState));
	persistMgr->transfer(TMEMBER(_personalizedSave));
	persistMgr->transfer(TMEMBER(_quitting));

	_regObjects.persist(persistMgr);

	persistMgr->transfer(TMEMBER(_scEngine));
	//persistMgr->transfer(TMEMBER(_soundMgr));
	persistMgr->transfer(TMEMBER_INT(_state));
	//persistMgr->transfer(TMEMBER(_surfaceStorage));
	persistMgr->transfer(TMEMBER(_subtitles));
	persistMgr->transfer(TMEMBER(_subtitlesSpeed));
	persistMgr->transfer(TMEMBER(_systemFont));
	persistMgr->transfer(TMEMBER(_videoFont));
	persistMgr->transfer(TMEMBER(_videoSubtitles));

	persistMgr->transfer(TMEMBER(_timer));
	persistMgr->transfer(TMEMBER(_timerDelta));
	persistMgr->transfer(TMEMBER(_timerLast));

	persistMgr->transfer(TMEMBER(_liveTimer));
	persistMgr->transfer(TMEMBER(_liveTimerDelta));
	persistMgr->transfer(TMEMBER(_liveTimerLast));

	persistMgr->transfer(TMEMBER(_musicCrossfadeRunning));
	persistMgr->transfer(TMEMBER(_musicCrossfadeStartTime));
	persistMgr->transfer(TMEMBER(_musicCrossfadeLength));
	persistMgr->transfer(TMEMBER(_musicCrossfadeChannel1));
	persistMgr->transfer(TMEMBER(_musicCrossfadeChannel2));
	persistMgr->transfer(TMEMBER(_musicCrossfadeSwap));

	persistMgr->transfer(TMEMBER(_loadImageName));
	persistMgr->transfer(TMEMBER(_saveImageName));
	persistMgr->transfer(TMEMBER(_saveImageX));
	persistMgr->transfer(TMEMBER(_saveImageY));
	persistMgr->transfer(TMEMBER(_loadImageX));
	persistMgr->transfer(TMEMBER(_loadImageY));

	persistMgr->transfer(TMEMBER_INT(_textEncoding));
	persistMgr->transfer(TMEMBER(_textRTL));

	persistMgr->transfer(TMEMBER(_soundBufferSizeSec));
	persistMgr->transfer(TMEMBER(_suspendedRendering));

	persistMgr->transfer(TMEMBER(_mouseLockRect));

	_windows.persist(persistMgr);

	persistMgr->transfer(TMEMBER(_suppressScriptErrors));
	persistMgr->transfer(TMEMBER(_autorunDisabled));

	persistMgr->transfer(TMEMBER(_autoSaveOnExit));
	persistMgr->transfer(TMEMBER(_autoSaveSlot));
	persistMgr->transfer(TMEMBER(_cursorHidden));

	if (persistMgr->checkVersion(1, 0, 1))
		persistMgr->transfer(TMEMBER(_store));
	else
		_store = NULL;

	if (!persistMgr->_saving)
		_quitting = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::focusWindow(CUIWindow *Window) {
	CUIWindow *Prev = _focusedWindow;

	for (int i = 0; i < _windows.GetSize(); i++) {
		if (_windows[i] == Window) {
			if (i < _windows.GetSize() - 1) {
				_windows.RemoveAt(i);
				_windows.Add(Window);

				Game->_focusedWindow = Window;
			}

			if (Window->_mode == WINDOW_NORMAL && Prev != Window && Game->validObject(Prev) && (Prev->_mode == WINDOW_EXCLUSIVE || Prev->_mode == WINDOW_SYSTEM_EXCLUSIVE))
				return focusWindow(Prev);
			else return S_OK;
		}
	}
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::freeze(bool includingMusic) {
	if (_freezeLevel == 0) {
		_scEngine->pauseAll();
		_soundMgr->pauseAll(includingMusic);
		_origState = _state;
		_origInteractive = _interactive;
		_interactive = true;
	}
	_state = GAME_FROZEN;
	_freezeLevel++;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::unfreeze() {
	if (_freezeLevel == 0) return S_OK;

	_freezeLevel--;
	if (_freezeLevel == 0) {
		_state = _origState;
		_interactive = _origInteractive;
		_scEngine->resumeAll();
		_soundMgr->resumeAll();
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBGame::handleKeypress(Common::Event *event, bool printable) {
	if (isVideoPlaying()) {
		if (event->kbd.keycode == Common::KEYCODE_ESCAPE)
			stopVideo();
		return true;
	}
#ifdef __WIN32__
	// TODO: Do we really need to handle this in-engine?
	// handle Alt+F4 on windows
	if (event->type == Common::EVENT_KEYDOWN && event->kbd.keycode == Common::KEYCODE_F4 && (event->kbd.flags == Common::KBD_ALT)) {
		onWindowClose();
		return true;
		//TODO
	}
#endif

	if (event->type == Common::EVENT_KEYDOWN && event->kbd.keycode == Common::KEYCODE_RETURN && (event->kbd.flags == Common::KBD_ALT)) {
		// TODO: Handle alt-enter as well as alt-return.
		_renderer->switchFullscreen();
		return true;
	}


	_keyboardState->handleKeyPress(event);
	_keyboardState->readKey(event);
// TODO

	if (_focusedWindow) {
		if (!Game->_focusedWindow->handleKeypress(event, _keyboardState->_currentPrintable)) {
			/*if (event->type != SDL_TEXTINPUT) {*/
			if (Game->_focusedWindow->canHandleEvent("Keypress"))
				Game->_focusedWindow->applyEvent("Keypress");
			else
				applyEvent("Keypress");
			/*}*/
		}
		return true;
	} else { /*if (event->type != SDL_TEXTINPUT)*/
		applyEvent("Keypress");
		return true;
	} //else return true;

	return false;
}

void CBGame::handleKeyRelease(Common::Event *event) {
	_keyboardState->handleKeyRelease(event);
}


//////////////////////////////////////////////////////////////////////////
bool CBGame::handleMouseWheel(int Delta) {
	bool handled = false;
	if (_focusedWindow) {
		handled = Game->_focusedWindow->handleMouseWheel(Delta);

		if (!handled) {
			if (Delta < 0 && Game->_focusedWindow->canHandleEvent("MouseWheelDown")) {
				Game->_focusedWindow->applyEvent("MouseWheelDown");
				handled = true;
			} else if (Game->_focusedWindow->canHandleEvent("MouseWheelUp")) {
				Game->_focusedWindow->applyEvent("MouseWheelUp");
				handled = true;
			}

		}
	}

	if (!handled) {
		if (Delta < 0) {
			applyEvent("MouseWheelDown");
		} else {
			applyEvent("MouseWheelUp");
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor) {
	if (verMajor) *verMajor = DCGF_VER_MAJOR;
	if (verMinor) *verMinor = DCGF_VER_MINOR;

	if (extMajor) *extMajor = 0;
	if (extMinor) *extMinor = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::setWindowTitle() {
	if (_renderer) {
		char title[512];
		strcpy(title, _caption[0]);
		if (title[0] != '\0') strcat(title, " - ");
		strcat(title, "WME Lite");


		Utf8String utf8Title;
		if (_textEncoding == TEXT_UTF8) {
			utf8Title = Utf8String(title);
		} else {
			warning("CBGame::SetWindowTitle -Ignoring textencoding");
			utf8Title = Utf8String(title);
			/*          WideString wstr = StringUtil::AnsiToWide(Title);
			            title = StringUtil::WideToUtf8(wstr);*/
		}
#if 0
		CBRenderSDL *renderer = static_cast<CBRenderSDL *>(_renderer);
		// TODO

		SDL_SetWindowTitle(renderer->GetSdlWindow(), title.c_str());
#endif
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::getSaveSlotFilename(int slot, char *buffer) {
	AnsiString dataDir = getDataDir();
	//sprintf(Buffer, "%s/save%03d.%s", dataDir.c_str(), Slot, _savedGameExt);
	sprintf(buffer, "save%03d.%s", slot, _savedGameExt);
	warning("Saving %s - we really should prefix these things to avoid collisions.", buffer);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
AnsiString CBGame::getDataDir() {
	AnsiString userDir = PathUtil::getUserDirectory();
#ifdef __IPHONEOS__
	return userDir;
#else
	AnsiString baseDir = _registry->getBasePath();
	return PathUtil::combine(userDir, baseDir);
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::getSaveSlotDescription(int slot, char *buffer) {
	buffer[0] = '\0';

	char filename[MAX_PATH + 1];
	getSaveSlotFilename(slot, filename);
	CBPersistMgr *pm = new CBPersistMgr(Game);
	if (!pm) return E_FAIL;

	_dEBUG_AbsolutePathWarning = false;
	if (FAILED(pm->initLoad(filename))) {
		_dEBUG_AbsolutePathWarning = true;
		delete pm;
		return E_FAIL;
	}

	_dEBUG_AbsolutePathWarning = true;
	strcpy(buffer, pm->_savedDescription);
	delete pm;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBGame::isSaveSlotUsed(int slot) {
	char filename[MAX_PATH + 1];
	getSaveSlotFilename(slot, filename);

	warning("CBGame::IsSaveSlotUsed(%d) - FIXME, ugly solution", slot);
	Common::SeekableReadStream *File = g_wintermute->getSaveFileMan()->openForLoading(filename);
	if (!File) return false;
	delete File;
	return true;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::emptySaveSlot(int slot) {
	char filename[MAX_PATH + 1];
	getSaveSlotFilename(slot, filename);

	CBPlatform::DeleteFile(filename);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::setActiveObject(CBObject *obj) {
	// not-active when game is frozen
	if (obj && !Game->_interactive && !obj->_nonIntMouseEvents) {
		obj = NULL;
	}

	if (obj == _activeObject) return S_OK;

	if (_activeObject) _activeObject->applyEvent("MouseLeave");
	//if(ValidObject(_activeObject)) _activeObject->applyEvent("MouseLeave");
	_activeObject = obj;
	if (_activeObject) {
		_activeObject->applyEvent("MouseEntry");
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::pushViewport(CBViewport *viewport) {
	_viewportSP++;
	if (_viewportSP >= _viewportStack.GetSize()) _viewportStack.Add(viewport);
	else _viewportStack[_viewportSP] = viewport;

	_renderer->setViewport(viewport->getRect());

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::popViewport() {
	_viewportSP--;
	if (_viewportSP < -1) Game->LOG(0, "Fatal: Viewport stack underflow!");

	if (_viewportSP >= 0 && _viewportSP < _viewportStack.GetSize()) _renderer->setViewport(_viewportStack[_viewportSP]->getRect());
	else _renderer->setViewport(_renderer->_drawOffsetX,
		                            _renderer->_drawOffsetY,
		                            _renderer->_width + _renderer->_drawOffsetX,
		                            _renderer->_height + _renderer->_drawOffsetY);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::getCurrentViewportRect(RECT *rect, bool *custom) {
	if (rect == NULL) return E_FAIL;
	else {
		if (_viewportSP >= 0) {
			CBPlatform::CopyRect(rect, _viewportStack[_viewportSP]->getRect());
			if (custom) *custom = true;
		} else {
			CBPlatform::SetRect(rect,   _renderer->_drawOffsetX,
			                    _renderer->_drawOffsetY,
			                    _renderer->_width + _renderer->_drawOffsetX,
			                    _renderer->_height + _renderer->_drawOffsetY);
			if (custom) *custom = false;
		}

		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::getCurrentViewportOffset(int *offsetX, int *offsetY) {
	if (_viewportSP >= 0) {
		if (offsetX) *offsetX = _viewportStack[_viewportSP]->_offsetX;
		if (offsetY) *offsetY = _viewportStack[_viewportSP]->_offsetY;
	} else {
		if (offsetX) *offsetX = 0;
		if (offsetY) *offsetY = 0;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::windowLoadHook(CUIWindow *win, char **buf, char **params) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::windowScriptMethodHook(CUIWindow *win, CScScript *script, CScStack *stack, const char *name) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::setInteractive(bool state) {
	_interactive = state;
	if (_transMgr) _transMgr->_origInteractive = state;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::resetMousePos() {
	POINT p;
	p.x = _mousePos.x + _renderer->_drawOffsetX;
	p.y = _mousePos.y + _renderer->_drawOffsetY;

	CBPlatform::SetCursorPos(p.x, p.y);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::displayContent(bool update, bool displayAll) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::displayContentSimple() {
	// fill black
	_renderer->fill(0, 0, 0);
	if (_indicatorDisplay) displayIndicator();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::displayIndicator() {
	if (_saveLoadImage) {
		RECT rc;
		CBPlatform::SetRect(&rc, 0, 0, _saveLoadImage->getWidth(), _saveLoadImage->getHeight());
		if (_loadInProgress) _saveLoadImage->displayTrans(_loadImageX, _loadImageY, rc);
		else _saveLoadImage->displayTrans(_saveImageX, _saveImageY, rc);
	}

	if ((!_indicatorDisplay && _indicatorWidth <= 0) || _indicatorHeight <= 0) return S_OK;
	_renderer->setupLines();
	for (int i = 0; i < _indicatorHeight; i++)
		_renderer->drawLine(_indicatorX, _indicatorY + i, _indicatorX + (int)(_indicatorWidth * (float)((float)_indicatorProgress / 100.0f)), _indicatorY + i, _indicatorColor);

	_renderer->setup2D();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::updateMusicCrossfade() {
	/* byte GlobMusicVol = _soundMgr->getVolumePercent(SOUND_MUSIC); */

	if (!_musicCrossfadeRunning) return S_OK;
	if (_state == GAME_FROZEN) return S_OK;

	if (_musicCrossfadeChannel1 < 0 || _musicCrossfadeChannel1 >= NUM_MUSIC_CHANNELS || !_music[_musicCrossfadeChannel1]) {
		_musicCrossfadeRunning = false;
		return S_OK;
	}
	if (_musicCrossfadeChannel2 < 0 || _musicCrossfadeChannel2 >= NUM_MUSIC_CHANNELS || !_music[_musicCrossfadeChannel2]) {
		_musicCrossfadeRunning = false;
		return S_OK;
	}

	if (!_music[_musicCrossfadeChannel1]->isPlaying()) _music[_musicCrossfadeChannel1]->play();
	if (!_music[_musicCrossfadeChannel2]->isPlaying()) _music[_musicCrossfadeChannel2]->play();

	uint32 currentTime = Game->_liveTimer - _musicCrossfadeStartTime;

	if (currentTime >= _musicCrossfadeLength) {
		_musicCrossfadeRunning = false;
		//_music[_musicCrossfadeChannel2]->setVolume(GlobMusicVol);
		_music[_musicCrossfadeChannel2]->setVolume(100);

		_music[_musicCrossfadeChannel1]->stop();
		//_music[_musicCrossfadeChannel1]->setVolume(GlobMusicVol);
		_music[_musicCrossfadeChannel1]->setVolume(100);


		if (_musicCrossfadeSwap) {
			// swap channels
			CBSound *dummy = _music[_musicCrossfadeChannel1];
			int dummyInt = _musicStartTime[_musicCrossfadeChannel1];

			_music[_musicCrossfadeChannel1] = _music[_musicCrossfadeChannel2];
			_musicStartTime[_musicCrossfadeChannel1] = _musicStartTime[_musicCrossfadeChannel2];

			_music[_musicCrossfadeChannel2] = dummy;
			_musicStartTime[_musicCrossfadeChannel2] = dummyInt;
		}
	} else {
		//_music[_musicCrossfadeChannel1]->setVolume(GlobMusicVol - (float)CurrentTime / (float)_musicCrossfadeLength * GlobMusicVol);
		//_music[_musicCrossfadeChannel2]->setVolume((float)CurrentTime / (float)_musicCrossfadeLength * GlobMusicVol);
		_music[_musicCrossfadeChannel1]->setVolume(100 - (float)currentTime / (float)_musicCrossfadeLength * 100);
		_music[_musicCrossfadeChannel2]->setVolume((float)currentTime / (float)_musicCrossfadeLength * 100);

		//Game->QuickMessageForm("%d %d", _music[_musicCrossfadeChannel1]->GetVolume(), _music[_musicCrossfadeChannel2]->GetVolume());
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::resetContent() {
	_scEngine->clearGlobals();
	//_timer = 0;
	//_liveTimer = 0;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBGame::DEBUG_DumpClassRegistry() {
	warning("DEBUG_DumpClassRegistry - untested");
	Common::DumpFile *f = new Common::DumpFile;
	f->open("zz_class_reg_dump.log");

	CSysClassRegistry::getInstance()->dumpClasses(f);

	f->close();
	delete f;
	Game->quickMessage("Classes dump completed.");
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::invalidateDeviceObjects() {
	for (int i = 0; i < _regObjects.GetSize(); i++) {
		_regObjects[i]->invalidateDeviceObjects();
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::restoreDeviceObjects() {
	for (int i = 0; i < _regObjects.GetSize(); i++) {
		_regObjects[i]->restoreDeviceObjects();
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::setWaitCursor(const char *filename) {
	delete _cursorNoninteractive;
	_cursorNoninteractive = NULL;

	_cursorNoninteractive = new CBSprite(Game);
	if (!_cursorNoninteractive || FAILED(_cursorNoninteractive->loadFile(filename))) {
		delete _cursorNoninteractive;
		_cursorNoninteractive = NULL;
		return E_FAIL;
	} else return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBGame::isVideoPlaying() {
	if (_videoPlayer->isPlaying()) return true;
	if (_theoraPlayer && _theoraPlayer->isPlaying()) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::stopVideo() {
	if (_videoPlayer->isPlaying()) _videoPlayer->stop();
	if (_theoraPlayer && _theoraPlayer->isPlaying()) {
		_theoraPlayer->stop();
		delete _theoraPlayer;
		_theoraPlayer = NULL;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::drawCursor(CBSprite *cursor) {
	if (!cursor) return E_FAIL;
	if (cursor != _lastCursor) {
		cursor->reset();
		_lastCursor = cursor;
	}
	return cursor->draw(_mousePos.x, _mousePos.y);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onActivate(bool activate, bool refreshMouse) {
	if (_shuttingDown || !_renderer) return S_OK;

	_renderer->_active = activate;

	if (refreshMouse) {
		POINT p;
		getMousePos(&p);
		setActiveObject(_renderer->getObjectAt(p.x, p.y));
	}

	if (activate) _soundMgr->resumeAll();
	else _soundMgr->pauseAll();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onMouseLeftDown() {
	if (_activeObject) _activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_LEFT);

	bool handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("LeftClick"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("LeftClick");
		}
	}

	if (_activeObject != NULL) _capturedObject = _activeObject;
	_mouseLeftDown = true;
	CBPlatform::SetCapture(_renderer->_window);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onMouseLeftUp() {
	if (_activeObject) _activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_LEFT);

	CBPlatform::ReleaseCapture();
	_capturedObject = NULL;
	_mouseLeftDown = false;

	bool handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("LeftRelease"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("LeftRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onMouseLeftDblClick() {
	if (_state == GAME_RUNNING && !_interactive) return S_OK;

	if (_activeObject) _activeObject->handleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_LEFT);

	bool handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("LeftDoubleClick"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("LeftDoubleClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onMouseRightDblClick() {
	if (_state == GAME_RUNNING && !_interactive) return S_OK;

	if (_activeObject) _activeObject->handleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_RIGHT);

	bool handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("RightDoubleClick"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("RightDoubleClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onMouseRightDown() {
	if (_activeObject) _activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_RIGHT);

	bool handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("RightClick"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("RightClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onMouseRightUp() {
	if (_activeObject) _activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_RIGHT);

	bool handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("RightRelease"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("RightRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onMouseMiddleDown() {
	if (_state == GAME_RUNNING && !_interactive) return S_OK;

	if (_activeObject) _activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_MIDDLE);

	bool handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("MiddleClick"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("MiddleClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onMouseMiddleUp() {
	if (_activeObject) _activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_MIDDLE);

	bool handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("MiddleRelease"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("MiddleRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onPaint() {
	if (_renderer && _renderer->_windowed && _renderer->_ready) {
		_renderer->initLoop();
		displayContent(false, true);
		displayDebugInfo();
		_renderer->windowedBlt();
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onWindowClose() {
	if (canHandleEvent("QuitGame")) {
		if (_state != GAME_FROZEN) Game->applyEvent("QuitGame");
		return S_OK;
	} else return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::displayDebugInfo() {
	char str[100];

	if (_dEBUG_ShowFPS) {
		sprintf(str, "FPS: %d", Game->_fps);
		_systemFont->drawText((byte *)str, 0, 0, 100, TAL_LEFT);
	}

	if (Game->_dEBUG_DebugMode) {
		if (!Game->_renderer->_windowed)
			sprintf(str, "Mode: %dx%dx%d", _renderer->_width, _renderer->_height, _renderer->_bPP);
		else
			sprintf(str, "Mode: %dx%d windowed", _renderer->_width, _renderer->_height);

		strcat(str, " (");
		strcat(str, _renderer->getName());
		strcat(str, ")");
		_systemFont->drawText((byte *)str, 0, 0, _renderer->_width, TAL_RIGHT);

		_renderer->displayDebugInfo();

		int scrTotal, scrRunning, scrWaiting, scrPersistent;
		scrTotal = _scEngine->getNumScripts(&scrRunning, &scrWaiting, &scrPersistent);
		sprintf(str, "Running scripts: %d (r:%d w:%d p:%d)", scrTotal, scrRunning, scrWaiting, scrPersistent);
		_systemFont->drawText((byte *)str, 0, 70, _renderer->_width, TAL_RIGHT);


		sprintf(str, "Timer: %d", _timer);
		Game->_systemFont->drawText((byte *)str, 0, 130, _renderer->_width, TAL_RIGHT);

		if (_activeObject != NULL) _systemFont->drawText((byte *)_activeObject->_name, 0, 150, _renderer->_width, TAL_RIGHT);

		sprintf(str, "GfxMem: %dMB", _usedMem / (1024 * 1024));
		_systemFont->drawText((byte *)str, 0, 170, _renderer->_width, TAL_RIGHT);

	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CBDebugger *CBGame::getDebugMgr() {
	if (!_debugMgr) _debugMgr = new CBDebugger(this);
	return _debugMgr;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::getMousePos(POINT *pos) {
	CBPlatform::GetCursorPos(pos);

	pos->x -= _renderer->_drawOffsetX;
	pos->y -= _renderer->_drawOffsetY;

	/*
	// Windows can squish maximized window if it's larger than desktop
	// so we need to modify mouse position appropriately (tnx mRax)
	if (_renderer->_windowed && ::IsZoomed(_renderer->_window)) {
	    RECT rc;
	    ::GetClientRect(_renderer->_window, &rc);
	    Pos->x *= Game->_renderer->_realWidth;
	    Pos->x /= (rc.right - rc.left);
	    Pos->y *= Game->_renderer->_realHeight;
	    Pos->y /= (rc.bottom - rc.top);
	}
	*/

	if (_mouseLockRect.left != 0 && _mouseLockRect.right != 0 && _mouseLockRect.top != 0 && _mouseLockRect.bottom != 0) {
		if (!CBPlatform::PtInRect(&_mouseLockRect, *pos)) {
			pos->x = MAX(_mouseLockRect.left, pos->x);
			pos->y = MAX(_mouseLockRect.top, pos->y);

			pos->x = MIN(_mouseLockRect.right, pos->x);
			pos->y = MIN(_mouseLockRect.bottom, pos->y);

			POINT newPos = *pos;

			newPos.x += _renderer->_drawOffsetX;
			newPos.y += _renderer->_drawOffsetY;

			CBPlatform::SetCursorPos(newPos.x, newPos.y);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::miniUpdate() {
	if (!_miniUpdateEnabled) return S_OK;

	if (CBPlatform::GetTime() - _lastMiniUpdate > 200) {
		if (_soundMgr) _soundMgr->initLoop();
		_lastMiniUpdate = CBPlatform::GetTime();
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::onScriptShutdown(CScScript *script) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBGame::isLeftDoubleClick() {
	return isDoubleClick(0);
}

//////////////////////////////////////////////////////////////////////////
bool CBGame::isRightDoubleClick() {
	return isDoubleClick(1);
}

//////////////////////////////////////////////////////////////////////////
bool CBGame::isDoubleClick(int buttonIndex) {
	uint32 maxDoubleCLickTime = 500;
	int maxMoveX = 4;
	int maxMoveY = 4;

#if __IPHONEOS__
	maxMoveX = 16;
	maxMoveY = 16;
#endif

	POINT pos;
	CBPlatform::GetCursorPos(&pos);

	int moveX = abs(pos.x - _lastClick[buttonIndex].PosX);
	int moveY = abs(pos.y - _lastClick[buttonIndex].PosY);


	if (_lastClick[buttonIndex].Time == 0 || CBPlatform::GetTime() - _lastClick[buttonIndex].Time > maxDoubleCLickTime || moveX > maxMoveX || moveY > maxMoveY) {
		_lastClick[buttonIndex].Time = CBPlatform::GetTime();
		_lastClick[buttonIndex].PosX = pos.x;
		_lastClick[buttonIndex].PosY = pos.y;
		return false;
	} else {
		_lastClick[buttonIndex].Time = 0;
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBGame::autoSaveOnExit() {
	_soundMgr->saveSettings();
	_registry->saveValues();

	if (!_autoSaveOnExit) return;
	if (_state == GAME_FROZEN) return;

	SaveGame(_autoSaveSlot, "autosave", true);
}

//////////////////////////////////////////////////////////////////////////
void CBGame::addMem(int bytes) {
	_usedMem += bytes;
}

//////////////////////////////////////////////////////////////////////////
AnsiString CBGame::getDeviceType() const {
#ifdef __IPHONEOS__
	char devType[128];
	IOS_GetDeviceType(devType);
	return AnsiString(devType);
#else
	return "computer";
#endif
}

} // end of namespace WinterMute
