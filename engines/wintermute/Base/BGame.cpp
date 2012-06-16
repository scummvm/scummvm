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

	_resourceModule = NULL;

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
	CBUtils::SetString(&_savedGameExt, "dsv");

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
	CBUtils::SetString(&_localSaveDir, "saves");
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

	GetDebugMgr()->OnGameShutdown();

	_registry->WriteBool("System", "LastRun", true);

	Cleanup();

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
HRESULT CBGame::Cleanup() {
	delete _loadingIcon;
	_loadingIcon = NULL;

	_engineLogCallback = NULL;
	_engineLogCallbackData = NULL;

	for (int i = 0; i < NUM_MUSIC_CHANNELS; i++) {
		delete _music[i];
		_music[i] = NULL;
		_musicStartTime[i] = 0;
	}

	UnregisterObject(_store);
	_store = NULL;

	UnregisterObject(_fader);
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
		_scripts[i]->Finish();
	}
	_scripts.RemoveAll();

	_fontStorage->RemoveFont(_systemFont);
	_systemFont = NULL;

	_fontStorage->RemoveFont(_videoFont);
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
HRESULT CBGame::Initialize1() {
	_surfaceStorage = new CBSurfaceStorage(this);
	if (_surfaceStorage == NULL) goto init_fail;

	_fontStorage = new CBFontStorage(this);
	if (_fontStorage == NULL) goto init_fail;

	_fileManager = new CBFileManager(this);
	if (_fileManager == NULL) goto init_fail;

	_soundMgr = new CBSoundMgr(this);
	if (_soundMgr == NULL) goto init_fail;

	_debugMgr = new CBDebugger(this);
	if (_debugMgr == NULL) goto init_fail;

	_mathClass = new CSXMath(this);
	if (_mathClass == NULL) goto init_fail;

	_scEngine = new CScEngine(this);
	if (_scEngine == NULL) goto init_fail;
	
	_videoPlayer = new CVidPlayer(this);
	if (_videoPlayer==NULL) goto init_fail;

	_transMgr = new CBTransitionMgr(this);
	if (_transMgr == NULL) goto init_fail;

	_keyboardState = new CBKeyboardState(this);
	if (_keyboardState == NULL) goto init_fail;

	_fader = new CBFader(this);
	if (_fader == NULL) goto init_fail;
	RegisterObject(_fader);

	_store = new CSXStore(this);
	if (_store == NULL) goto init_fail;
	RegisterObject(_store);

	return S_OK;

init_fail:
	if (_mathClass) delete _mathClass;
	if (_store) delete _store;
	if (_keyboardState) delete _keyboardState;
	if (_transMgr) delete _transMgr;
	if (_debugMgr) delete _debugMgr;
	if (_surfaceStorage) delete _surfaceStorage;
	if (_fontStorage) delete _fontStorage;
	if (_soundMgr) delete _soundMgr;
	if (_fileManager) delete _fileManager;
	if (_scEngine) delete _scEngine;
	if (_videoPlayer) delete _videoPlayer;
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBGame::Initialize2() { // we know whether we are going to be accelerated
	_renderer = new CBRenderSDL(this);
	if (_renderer == NULL) goto init_fail;

	return S_OK;

init_fail:
	if (_renderer) delete _renderer;
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBGame::Initialize3() { // renderer is initialized
	_posX = _renderer->_width / 2;
	_posY = _renderer->_height / 2;

	if (_indicatorY == -1) _indicatorY = _renderer->_height - _indicatorHeight;
	if (_indicatorX == -1) _indicatorX = 0;
	if (_indicatorWidth == -1) _indicatorWidth = _renderer->_width;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////
void CBGame::DEBUG_DebugEnable(const char *Filename) {
	_dEBUG_DebugMode = true;

#ifndef __IPHONEOS__
	//if (Filename)_dEBUG_LogFile = fopen(Filename, "a+");
	//else _dEBUG_LogFile = fopen("./zz_debug.log", "a+");

	if (!_dEBUG_LogFile) {
		AnsiString safeLogFileName = PathUtil::GetSafeLogFileName();
		//_dEBUG_LogFile = fopen(safeLogFileName.c_str(), "a+");
	}

	//if (_dEBUG_LogFile != NULL) fprintf((FILE *)_dEBUG_LogFile, "\n");
	warning("BGame::DEBUG_DebugEnable - No logfile is currently created"); //TODO: Use a dumpfile?
#endif

/*	time_t timeNow;
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
#ifndef __IPHONEOS__
	if (!_dEBUG_DebugMode) return;
#endif
/*	time_t timeNow;
	time(&timeNow);
	struct tm *tm = localtime(&timeNow);*/
	int secs = g_system->getMillis() / 1000;
	int hours = secs / 3600;
	secs = secs % 3600;
	int mins = secs / 60;
	secs = secs % 60;

	char buff[512];
	va_list va;

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

#ifdef __IPHONEOS__
	printf("%02d:%02d:%02d: %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, buff);
	fflush(stdout);
#else
	if (_dEBUG_LogFile == NULL) return;

	// redirect to an engine's own callback
	if (_engineLogCallback) {
		_engineLogCallback(buff, res, _engineLogCallbackData);
	}
	if (_debugMgr) _debugMgr->OnLog(res, buff);

	warning("%02d:%02d:%02d: %s\n", hours, mins, secs, buff);
	//fprintf((FILE *)_dEBUG_LogFile, "%02d:%02d:%02d: %s\n", hours, mins, secs, buff);
	//fflush((FILE *)_dEBUG_LogFile);
#endif

	//QuickMessage(buff);
}


//////////////////////////////////////////////////////////////////////////
void CBGame::SetEngineLogCallback(ENGINE_LOG_CALLBACK Callback, void *Data) {
	_engineLogCallback = Callback;
	_engineLogCallbackData = Data;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBGame::InitLoop() {
	_viewportSP = -1;

	_currentTime = CBPlatform::GetTime();

	GetDebugMgr()->OnGameTick();
	_renderer->InitLoop();
	_soundMgr->initLoop();
	UpdateMusicCrossfade();

	_surfaceStorage->InitLoop();
	_fontStorage->InitLoop();


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

	GetMousePos(&_mousePos);

	_focusedWindow = NULL;
	for (int i = _windows.GetSize() - 1; i >= 0; i--) {
		if (_windows[i]->_visible) {
			_focusedWindow = _windows[i];
			break;
		}
	}

	UpdateSounds();

	if (_fader) _fader->Update();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBGame::InitInput(HINSTANCE hInst, HWND hWnd) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CBGame::GetSequence() {
	return ++_sequence;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::SetOffset(int OffsetX, int OffsetY) {
	_offsetX = OffsetX;
	_offsetY = OffsetY;
}

//////////////////////////////////////////////////////////////////////////
void CBGame::GetOffset(int *OffsetX, int *OffsetY) {
	if (OffsetX != NULL) *OffsetX = _offsetX;
	if (OffsetY != NULL) *OffsetY = _offsetY;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::LoadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CBGame::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing GAME file '%s'", Filename);


	delete [] Buffer;

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
HRESULT CBGame::LoadBuffer(byte  *Buffer, bool Complete) {
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

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_GAME) {
			Game->LOG(0, "'GAME' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_CAPTION:
			SetCaption((char *)params);
			break;

		case TOKEN_SYSTEM_FONT:
			if (_systemFont) _fontStorage->RemoveFont(_systemFont);
			_systemFont = NULL;

			_systemFont = Game->_fontStorage->AddFont((char *)params);
			break;

		case TOKEN_VIDEO_FONT:
			if (_videoFont) _fontStorage->RemoveFont(_videoFont);
			_videoFont = NULL;

			_videoFont = Game->_fontStorage->AddFont((char *)params);
			break;


		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new CBSprite(Game);
			if (!_cursor || FAILED(_cursor->LoadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_ACTIVE_CURSOR:
			delete _activeCursor;
			_activeCursor = NULL;
			_activeCursor = new CBSprite(Game);
			if (!_activeCursor || FAILED(_activeCursor->LoadFile((char *)params))) {
				delete _activeCursor;
				_activeCursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NONINTERACTIVE_CURSOR:
			delete _cursorNoninteractive;
			_cursorNoninteractive = new CBSprite(Game);
			if (!_cursorNoninteractive || FAILED(_cursorNoninteractive->LoadFile((char *)params))) {
				delete _cursorNoninteractive;
				_cursorNoninteractive = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_PERSONAL_SAVEGAMES:
			parser.ScanStr((char *)params, "%b", &_personalizedSave);
			break;

		case TOKEN_SUBTITLES:
			parser.ScanStr((char *)params, "%b", &_subtitles);
			break;

		case TOKEN_SUBTITLES_SPEED:
			parser.ScanStr((char *)params, "%d", &_subtitlesSpeed);
			break;

		case TOKEN_VIDEO_SUBTITLES:
			parser.ScanStr((char *)params, "%b", &_videoSubtitles);
			break;

		case TOKEN_PROPERTY:
			ParseProperty(params, false);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;

		case TOKEN_THUMBNAIL_WIDTH:
			parser.ScanStr((char *)params, "%d", &_thumbnailWidth);
			break;

		case TOKEN_THUMBNAIL_HEIGHT:
			parser.ScanStr((char *)params, "%d", &_thumbnailHeight);
			break;

		case TOKEN_INDICATOR_X:
			parser.ScanStr((char *)params, "%d", &_indicatorX);
			break;

		case TOKEN_INDICATOR_Y:
			parser.ScanStr((char *)params, "%d", &_indicatorY);
			break;

		case TOKEN_INDICATOR_COLOR: {
			int r, g, b, a;
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &r, &g, &b, &a);
			_indicatorColor = DRGBA(r, g, b, a);
		}
		break;

		case TOKEN_INDICATOR_WIDTH:
			parser.ScanStr((char *)params, "%d", &_indicatorWidth);
			break;

		case TOKEN_INDICATOR_HEIGHT:
			parser.ScanStr((char *)params, "%d", &_indicatorHeight);
			break;

		case TOKEN_SAVE_IMAGE:
			CBUtils::SetString(&_saveImageName, (char *)params);
			break;

		case TOKEN_SAVE_IMAGE_X:
			parser.ScanStr((char *)params, "%d", &_saveImageX);
			break;

		case TOKEN_SAVE_IMAGE_Y:
			parser.ScanStr((char *)params, "%d", &_saveImageY);
			break;

		case TOKEN_LOAD_IMAGE:
			CBUtils::SetString(&_loadImageName, (char *)params);
			break;

		case TOKEN_LOAD_IMAGE_X:
			parser.ScanStr((char *)params, "%d", &_loadImageX);
			break;

		case TOKEN_LOAD_IMAGE_Y:
			parser.ScanStr((char *)params, "%d", &_loadImageY);
			break;

		case TOKEN_LOCAL_SAVE_DIR:
			CBUtils::SetString(&_localSaveDir, (char *)params);
			break;

		case TOKEN_COMPAT_KILL_METHOD_THREADS:
			parser.ScanStr((char *)params, "%b", &_compatKillMethodThreads);
			break;
		}
	}

	if (!_systemFont) _systemFont = Game->_fontStorage->AddFont("system_font.fnt");


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
HRESULT CBGame::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// LOG
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "LOG") == 0) {
		Stack->CorrectParams(1);
		LOG(0, Stack->Pop()->GetString());
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Caption") == 0) {
		HRESULT res = CBObject::ScCallMethod(Script, Stack, ThisStack, Name);
		SetWindowTitle();
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	// Msg
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Msg") == 0) {
		Stack->CorrectParams(1);
		QuickMessage(Stack->Pop()->GetString());
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RunScript
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RunScript") == 0) {
		Game->LOG(0, "**Warning** The 'RunScript' method is now obsolete. Use 'AttachScript' instead (same syntax)");
		Stack->CorrectParams(1);
		if (FAILED(AddScript(Stack->Pop()->GetString())))
			Stack->PushBool(false);
		else
			Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadStringTable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadStringTable") == 0) {
		Stack->CorrectParams(2);
		const char *Filename = Stack->Pop()->GetString();
		CScValue *Val = Stack->Pop();

		bool ClearOld;
		if (Val->IsNULL()) ClearOld = true;
		else ClearOld = Val->GetBool();

		if (FAILED(_stringTable->LoadFile(Filename, ClearOld)))
			Stack->PushBool(false);
		else
			Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ValidObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ValidObject") == 0) {
		Stack->CorrectParams(1);
		CBScriptable *obj = Stack->Pop()->GetNative();
		if (ValidObject((CBObject *) obj)) Stack->PushBool(true);
		else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Reset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Reset") == 0) {
		Stack->CorrectParams(0);
		ResetContent();
		Stack->PushNULL();

		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// UnloadObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "UnloadObject") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		CBObject *obj = (CBObject *)val->GetNative();
		UnregisterObject(obj);
		if (val->GetType() == VAL_VARIABLE_REF) val->SetNULL();

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadWindow") == 0) {
		Stack->CorrectParams(1);
		CUIWindow *win = new CUIWindow(Game);
		if (win && SUCCEEDED(win->LoadFile(Stack->Pop()->GetString()))) {
			_windows.Add(win);
			RegisterObject(win);
			Stack->PushNative(win, true);
		} else {
			delete win;
			win = NULL;
			Stack->PushNULL();
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ExpandString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ExpandString") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		char *str = new char[strlen(val->GetString()) + 1];
		strcpy(str, val->GetString());
		_stringTable->Expand(&str);
		Stack->PushString(str);
		delete [] str;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayMusic / PlayMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PlayMusic") == 0 || strcmp(Name, "PlayMusicChannel") == 0) {
		int Channel = 0;
		if (strcmp(Name, "PlayMusic") == 0) Stack->CorrectParams(3);
		else {
			Stack->CorrectParams(4);
			Channel = Stack->Pop()->GetInt();
		}

		const char *Filename = Stack->Pop()->GetString();
		CScValue *ValLooping = Stack->Pop();
		bool Looping = ValLooping->IsNULL() ? true : ValLooping->GetBool();

		CScValue *ValLoopStart = Stack->Pop();
		uint32 LoopStart = (uint32)(ValLoopStart->IsNULL() ? 0 : ValLoopStart->GetInt());


		if (FAILED(PlayMusic(Channel, Filename, Looping, LoopStart))) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopMusic / StopMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StopMusic") == 0 || strcmp(Name, "StopMusicChannel") == 0) {
		int Channel = 0;

		if (strcmp(Name, "StopMusic") == 0) Stack->CorrectParams(0);
		else {
			Stack->CorrectParams(1);
			Channel = Stack->Pop()->GetInt();
		}

		if (FAILED(StopMusic(Channel))) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseMusic / PauseMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PauseMusic") == 0 || strcmp(Name, "PauseMusicChannel") == 0) {
		int Channel = 0;

		if (strcmp(Name, "PauseMusic") == 0) Stack->CorrectParams(0);
		else {
			Stack->CorrectParams(1);
			Channel = Stack->Pop()->GetInt();
		}

		if (FAILED(PauseMusic(Channel))) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResumeMusic / ResumeMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ResumeMusic") == 0 || strcmp(Name, "ResumeMusicChannel") == 0) {
		int Channel = 0;
		if (strcmp(Name, "ResumeMusic") == 0) Stack->CorrectParams(0);
		else {
			Stack->CorrectParams(1);
			Channel = Stack->Pop()->GetInt();
		}

		if (FAILED(ResumeMusic(Channel))) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetMusic / GetMusicChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetMusic") == 0 || strcmp(Name, "GetMusicChannel") == 0) {
		int Channel = 0;
		if (strcmp(Name, "GetMusic") == 0) Stack->CorrectParams(0);
		else {
			Stack->CorrectParams(1);
			Channel = Stack->Pop()->GetInt();
		}
		if (Channel < 0 || Channel >= NUM_MUSIC_CHANNELS) Stack->PushNULL();
		else {
			if (!_music[Channel] || !_music[Channel]->_soundFilename) Stack->PushNULL();
			else Stack->PushString(_music[Channel]->_soundFilename);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetMusicPosition / SetMusicChannelPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetMusicPosition") == 0 || strcmp(Name, "SetMusicChannelPosition") == 0 || strcmp(Name, "SetMusicPositionChannel") == 0) {
		int Channel = 0;
		if (strcmp(Name, "SetMusicPosition") == 0) Stack->CorrectParams(1);
		else {
			Stack->CorrectParams(2);
			Channel = Stack->Pop()->GetInt();
		}

		uint32 Time = Stack->Pop()->GetInt();

		if (FAILED(SetMusicStartTime(Channel, Time))) Stack->PushBool(false);
		else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetMusicPosition / GetMusicChannelPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetMusicPosition") == 0 || strcmp(Name, "GetMusicChannelPosition") == 0) {
		int Channel = 0;
		if (strcmp(Name, "GetMusicPosition") == 0) Stack->CorrectParams(0);
		else {
			Stack->CorrectParams(1);
			Channel = Stack->Pop()->GetInt();
		}

		if (Channel < 0 || Channel >= NUM_MUSIC_CHANNELS || !_music[Channel]) Stack->PushInt(0);
		else Stack->PushInt(_music[Channel]->GetPositionTime());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsMusicPlaying / IsMusicChannelPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsMusicPlaying") == 0 || strcmp(Name, "IsMusicChannelPlaying") == 0) {
		int Channel = 0;
		if (strcmp(Name, "IsMusicPlaying") == 0) Stack->CorrectParams(0);
		else {
			Stack->CorrectParams(1);
			Channel = Stack->Pop()->GetInt();
		}

		if (Channel < 0 || Channel >= NUM_MUSIC_CHANNELS || !_music[Channel]) Stack->PushBool(false);
		else Stack->PushBool(_music[Channel]->IsPlaying());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetMusicVolume / SetMusicChannelVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetMusicVolume") == 0 || strcmp(Name, "SetMusicChannelVolume") == 0) {
		int Channel = 0;
		if (strcmp(Name, "SetMusicVolume") == 0) Stack->CorrectParams(1);
		else {
			Stack->CorrectParams(2);
			Channel = Stack->Pop()->GetInt();
		}

		int Volume = Stack->Pop()->GetInt();
		if (Channel < 0 || Channel >= NUM_MUSIC_CHANNELS || !_music[Channel]) Stack->PushBool(false);
		else {
			if (FAILED(_music[Channel]->SetVolume(Volume))) Stack->PushBool(false);
			else Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetMusicVolume / GetMusicChannelVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetMusicVolume") == 0 || strcmp(Name, "GetMusicChannelVolume") == 0) {
		int Channel = 0;
		if (strcmp(Name, "GetMusicVolume") == 0) Stack->CorrectParams(0);
		else {
			Stack->CorrectParams(1);
			Channel = Stack->Pop()->GetInt();
		}

		if (Channel < 0 || Channel >= NUM_MUSIC_CHANNELS || !_music[Channel]) Stack->PushInt(0);
		else Stack->PushInt(_music[Channel]->GetVolume());

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MusicCrossfade
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MusicCrossfade") == 0) {
		Stack->CorrectParams(4);
		int Channel1 = Stack->Pop()->GetInt(0);
		int Channel2 = Stack->Pop()->GetInt(0);
		uint32 FadeLength = (uint32)Stack->Pop()->GetInt(0);
		bool Swap = Stack->Pop()->GetBool(true);

		if (_musicCrossfadeRunning) {
			Script->RuntimeError("Game.MusicCrossfade: Music crossfade is already in progress.");
			Stack->PushBool(false);
			return S_OK;
		}

		_musicCrossfadeStartTime = _liveTimer;
		_musicCrossfadeChannel1 = Channel1;
		_musicCrossfadeChannel2 = Channel2;
		_musicCrossfadeLength = FadeLength;
		_musicCrossfadeSwap = Swap;

		_musicCrossfadeRunning = true;

		Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSoundLength
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSoundLength") == 0) {
		Stack->CorrectParams(1);

		int Length = 0;
		const char *Filename = Stack->Pop()->GetString();

		CBSound *Sound = new CBSound(Game);
		if (Sound && SUCCEEDED(Sound->SetSound(Filename, SOUND_MUSIC, true))) {
			Length = Sound->GetLength();
			delete Sound;
			Sound = NULL;
		}
		Stack->PushInt(Length);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetMousePos
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetMousePos") == 0) {
		Stack->CorrectParams(2);
		int x = Stack->Pop()->GetInt();
		int y = Stack->Pop()->GetInt();
		x = MAX(x, 0);
		x = MIN(x, _renderer->_width);
		y = MAX(y, 0);
		y = MIN(y, _renderer->_height);
		POINT p;
		p.x = x + _renderer->_drawOffsetX;
		p.y = y + _renderer->_drawOffsetY;

		CBPlatform::SetCursorPos(p.x, p.y);

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LockMouseRect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LockMouseRect") == 0) {
		Stack->CorrectParams(4);
		int left = Stack->Pop()->GetInt();
		int top = Stack->Pop()->GetInt();
		int right = Stack->Pop()->GetInt();
		int bottom = Stack->Pop()->GetInt();

		if (right < left) CBUtils::Swap(&left, &right);
		if (bottom < top) CBUtils::Swap(&top, &bottom);

		CBPlatform::SetRect(&_mouseLockRect, left, top, right, bottom);

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayVideo
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PlayVideo") == 0) {
/*		Stack->CorrectParams(0);
		Stack->PushBool(false);

		return S_OK;
		// TODO: ADDVIDEO
		*/
		
		Game->LOG(0, "Warning: Game.PlayVideo() is now deprecated. Use Game.PlayTheora() instead.");
		
		Stack->CorrectParams(6);
		const char* Filename = Stack->Pop()->GetString();
		warning("PlayVideo: %s - not implemented yet", Filename);
		CScValue* valType = Stack->Pop();
		int Type;
		if (valType->IsNULL()) Type = (int)VID_PLAY_STRETCH;
		else Type = valType->GetInt();
		
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();
		bool FreezeMusic = Stack->Pop()->GetBool(true);
		
		CScValue* valSub = Stack->Pop();
		const char* SubtitleFile = valSub->IsNULL()?NULL:valSub->GetString();
		
		if (Type < (int)VID_PLAY_POS || Type > (int)VID_PLAY_CENTER) 
			Type = (int)VID_PLAY_STRETCH;
		
		if (SUCCEEDED(Game->_videoPlayer->initialize(Filename, SubtitleFile))) {
			if (SUCCEEDED(Game->_videoPlayer->play((TVideoPlayback)Type, X, Y, FreezeMusic))) {
				Stack->PushBool(true);
				Script->Sleep(0);
			}
			else Stack->PushBool(false);
		}
		else Stack->PushBool(false);
		
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PlayTheora") == 0) {
/*		Stack->CorrectParams(0);
		Stack->PushBool(false);

		return S_OK;*/
		// TODO: ADDVIDEO

		Stack->CorrectParams(7);
		const char* Filename = Stack->Pop()->GetString();
		CScValue* valType = Stack->Pop();
		int Type;
		if (valType->IsNULL()) 
			Type = (int)VID_PLAY_STRETCH;
		else Type = valType->GetInt();
		
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();
		bool FreezeMusic = Stack->Pop()->GetBool(true);
		bool DropFrames = Stack->Pop()->GetBool(true);
		
		CScValue* valSub = Stack->Pop();
		const char* SubtitleFile = valSub->IsNULL()?NULL:valSub->GetString();

		if (Type < (int)VID_PLAY_POS || Type > (int)VID_PLAY_CENTER) Type = (int)VID_PLAY_STRETCH;

		delete _theoraPlayer;
		_theoraPlayer = new CVidTheoraPlayer(this);
		if (_theoraPlayer && SUCCEEDED(_theoraPlayer->initialize(Filename, SubtitleFile))) {
			_theoraPlayer->_dontDropFrames = !DropFrames;
			if (SUCCEEDED(_theoraPlayer->play((TVideoPlayback)Type, X, Y, true, FreezeMusic))) {
				Stack->PushBool(true);
				Script->Sleep(0);
			}
			else Stack->PushBool(false);
		}
		else {
			Stack->PushBool(false);
			delete _theoraPlayer;
			_theoraPlayer = NULL;
		}
		
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// QuitGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "QuitGame") == 0) {
		Stack->CorrectParams(0);
		Stack->PushNULL();
		_quitting = true;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegWriteNumber
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RegWriteNumber") == 0) {
		Stack->CorrectParams(2);
		const char *Key = Stack->Pop()->GetString();
		int Val = Stack->Pop()->GetInt();
		_registry->WriteInt("PrivateSettings", Key, Val);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegReadNumber
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RegReadNumber") == 0) {
		Stack->CorrectParams(2);
		const char *Key = Stack->Pop()->GetString();
		int InitVal = Stack->Pop()->GetInt();
		Stack->PushInt(_registry->ReadInt("PrivateSettings", Key, InitVal));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegWriteString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RegWriteString") == 0) {
		Stack->CorrectParams(2);
		const char *Key = Stack->Pop()->GetString();
		const char *Val = Stack->Pop()->GetString();
		_registry->WriteString("PrivateSettings", Key, Val);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RegReadString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RegReadString") == 0) {
		Stack->CorrectParams(2);
		const char *Key = Stack->Pop()->GetString();
		const char *InitVal = Stack->Pop()->GetString();
		AnsiString val = _registry->ReadString("PrivateSettings", Key, InitVal);
		Stack->PushString((char *)val.c_str());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SaveGame") == 0) {
		Stack->CorrectParams(3);
		int Slot = Stack->Pop()->GetInt();
		const char *xdesc = Stack->Pop()->GetString();
		bool quick = Stack->Pop()->GetBool(false);

		char *Desc = new char[strlen(xdesc) + 1];
		strcpy(Desc, xdesc);
		Stack->PushBool(true);
		if (FAILED(SaveGame(Slot, Desc, quick))) {
			Stack->Pop();
			Stack->PushBool(false);
		}
		delete [] Desc;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadGame") == 0) {
		Stack->CorrectParams(1);
		_scheduledLoadSlot = Stack->Pop()->GetInt();
		_loading = true;
		Stack->PushBool(false);
		Script->Sleep(0);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsSaveSlotUsed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsSaveSlotUsed") == 0) {
		Stack->CorrectParams(1);
		int Slot = Stack->Pop()->GetInt();
		Stack->PushBool(IsSaveSlotUsed(Slot));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSaveSlotDescription
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSaveSlotDescription") == 0) {
		Stack->CorrectParams(1);
		int Slot = Stack->Pop()->GetInt();
		char Desc[512];
		Desc[0] = '\0';
		GetSaveSlotDescription(Slot, Desc);
		Stack->PushString(Desc);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EmptySaveSlot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "EmptySaveSlot") == 0) {
		Stack->CorrectParams(1);
		int Slot = Stack->Pop()->GetInt();
		EmptySaveSlot(Slot);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalSFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetGlobalSFXVolume") == 0) {
		Stack->CorrectParams(1);
		Game->_soundMgr->setVolumePercent(SOUND_SFX, (byte)Stack->Pop()->GetInt());
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalSpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetGlobalSpeechVolume") == 0) {
		Stack->CorrectParams(1);
		Game->_soundMgr->setVolumePercent(SOUND_SPEECH, (byte)Stack->Pop()->GetInt());
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalMusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetGlobalMusicVolume") == 0) {
		Stack->CorrectParams(1);
		Game->_soundMgr->setVolumePercent(SOUND_MUSIC, (byte)Stack->Pop()->GetInt());
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetGlobalMasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetGlobalMasterVolume") == 0) {
		Stack->CorrectParams(1);
		Game->_soundMgr->setMasterVolumePercent((byte)Stack->Pop()->GetInt());
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalSFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetGlobalSFXVolume") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_soundMgr->getVolumePercent(SOUND_SFX));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalSpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetGlobalSpeechVolume") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_soundMgr->getVolumePercent(SOUND_SPEECH));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalMusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetGlobalMusicVolume") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_soundMgr->getVolumePercent(SOUND_MUSIC));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGlobalMasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetGlobalMasterVolume") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_soundMgr->getMasterVolumePercent());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetActiveCursor") == 0) {
		Stack->CorrectParams(1);
		if (SUCCEEDED(SetActiveCursor(Stack->Pop()->GetString()))) Stack->PushBool(true);
		else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetActiveCursor") == 0) {
		Stack->CorrectParams(0);
		if (!_activeCursor || !_activeCursor->_filename) Stack->PushNULL();
		else Stack->PushString(_activeCursor->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetActiveCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetActiveCursorObject") == 0) {
		Stack->CorrectParams(0);
		if (!_activeCursor) Stack->PushNULL();
		else Stack->PushNative(_activeCursor, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RemoveActiveCursor") == 0) {
		Stack->CorrectParams(0);
		delete _activeCursor;
		_activeCursor = NULL;
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasActiveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HasActiveCursor") == 0) {
		Stack->CorrectParams(0);

		if (_activeCursor) Stack->PushBool(true);
		else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FileExists
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FileExists") == 0) {
		Stack->CorrectParams(1);
		const char *Filename = Stack->Pop()->GetString();

		Common::SeekableReadStream *File = _fileManager->OpenFile(Filename, false);
		if (!File) Stack->PushBool(false);
		else {
			_fileManager->CloseFile(File);
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeOut / FadeOutAsync / SystemFadeOut / SystemFadeOutAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeOut") == 0 || strcmp(Name, "FadeOutAsync") == 0 || strcmp(Name, "SystemFadeOut") == 0 || strcmp(Name, "SystemFadeOutAsync") == 0) {
		Stack->CorrectParams(5);
		uint32 Duration = Stack->Pop()->GetInt(500);
		byte Red = Stack->Pop()->GetInt(0);
		byte Green = Stack->Pop()->GetInt(0);
		byte Blue = Stack->Pop()->GetInt(0);
		byte Alpha = Stack->Pop()->GetInt(0xFF);

		bool System = (strcmp(Name, "SystemFadeOut") == 0 || strcmp(Name, "SystemFadeOutAsync") == 0);

		_fader->FadeOut(DRGBA(Red, Green, Blue, Alpha), Duration, System);
		if (strcmp(Name, "FadeOutAsync") != 0 && strcmp(Name, "SystemFadeOutAsync") != 0) Script->WaitFor(_fader);

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeIn / FadeInAsync / SystemFadeIn / SystemFadeInAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeIn") == 0 || strcmp(Name, "FadeInAsync") == 0 || strcmp(Name, "SystemFadeIn") == 0 || strcmp(Name, "SystemFadeInAsync") == 0) {
		Stack->CorrectParams(5);
		uint32 Duration = Stack->Pop()->GetInt(500);
		byte Red = Stack->Pop()->GetInt(0);
		byte Green = Stack->Pop()->GetInt(0);
		byte Blue = Stack->Pop()->GetInt(0);
		byte Alpha = Stack->Pop()->GetInt(0xFF);

		bool System = (strcmp(Name, "SystemFadeIn") == 0 || strcmp(Name, "SystemFadeInAsync") == 0);

		_fader->FadeIn(DRGBA(Red, Green, Blue, Alpha), Duration, System);
		if (strcmp(Name, "FadeInAsync") != 0 && strcmp(Name, "SystemFadeInAsync") != 0) Script->WaitFor(_fader);

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFadeColor") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_fader->GetCurrentColor());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Screenshot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Screenshot") == 0) {
		Stack->CorrectParams(1);
		char Filename[MAX_PATH];

		CScValue *Val = Stack->Pop();

		warning("BGame::ScCallMethod - Screenshot not reimplemented"); //TODO
		int FileNum = 0;
		
		while (true) {
			sprintf(Filename, "%s%03d.bmp", Val->IsNULL() ? _name : Val->GetString(), FileNum);
			if (!Common::File::exists(Filename))
				break;
			FileNum++;
		}

		bool ret = false;
		CBImage *Image = Game->_renderer->TakeScreenshot();
		if (Image) {
			ret = SUCCEEDED(Image->SaveBMPFile(Filename));
			delete Image;
		} else ret = false;

		Stack->PushBool(ret);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScreenshotEx
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScreenshotEx") == 0) {
		Stack->CorrectParams(3);
		const char *Filename = Stack->Pop()->GetString();
		int SizeX = Stack->Pop()->GetInt(_renderer->_width);
		int SizeY = Stack->Pop()->GetInt(_renderer->_height);

		bool ret = false;
		CBImage *Image = Game->_renderer->TakeScreenshot();
		if (Image) {
			ret = SUCCEEDED(Image->Resize(SizeX, SizeY));
			if (ret) ret = SUCCEEDED(Image->SaveBMPFile(Filename));
			delete Image;
		} else ret = false;

		Stack->PushBool(ret);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateWindow") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIWindow *Win = new CUIWindow(Game);
		_windows.Add(Win);
		RegisterObject(Win);
		if (!Val->IsNULL()) Win->SetName(Val->GetString());
		Stack->PushNative(Win, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteWindow") == 0) {
		Stack->CorrectParams(1);
		CBObject *Obj = (CBObject *)Stack->Pop()->GetNative();
		for (int i = 0; i < _windows.GetSize(); i++) {
			if (_windows[i] == Obj) {
				UnregisterObject(_windows[i]);
				Stack->PushBool(true);
				return S_OK;
			}
		}
		Stack->PushBool(false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OpenDocument
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OpenDocument") == 0) {
		Stack->CorrectParams(0);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DEBUG_DumpClassRegistry
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DEBUG_DumpClassRegistry") == 0) {
		Stack->CorrectParams(0);
		DEBUG_DumpClassRegistry();
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetLoadingScreen
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetLoadingScreen") == 0) {
		Stack->CorrectParams(3);
		CScValue *Val = Stack->Pop();
		_loadImageX = Stack->Pop()->GetInt();
		_loadImageY = Stack->Pop()->GetInt();

		if (Val->IsNULL()) {
			delete[] _loadImageName;
			_loadImageName = NULL;
		} else {
			CBUtils::SetString(&_loadImageName, Val->GetString());
		}
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSavingScreen
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetSavingScreen") == 0) {
		Stack->CorrectParams(3);
		CScValue *Val = Stack->Pop();
		_saveImageX = Stack->Pop()->GetInt();
		_saveImageY = Stack->Pop()->GetInt();

		if (Val->IsNULL()) {
			delete[] _saveImageName;
			_saveImageName = NULL;
		} else {
			CBUtils::SetString(&_saveImageName, Val->GetString());
		}
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetWaitCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetWaitCursor") == 0) {
		Stack->CorrectParams(1);
		if (SUCCEEDED(SetWaitCursor(Stack->Pop()->GetString()))) Stack->PushBool(true);
		else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveWaitCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RemoveWaitCursor") == 0) {
		Stack->CorrectParams(0);
		delete _cursorNoninteractive;
		_cursorNoninteractive = NULL;

		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetWaitCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetWaitCursor") == 0) {
		Stack->CorrectParams(0);
		if (!_cursorNoninteractive || !_cursorNoninteractive->_filename) Stack->PushNULL();
		else Stack->PushString(_cursorNoninteractive->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetWaitCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetWaitCursorObject") == 0) {
		Stack->CorrectParams(0);
		if (!_cursorNoninteractive) Stack->PushNULL();
		else Stack->PushNative(_cursorNoninteractive, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClearScriptCache
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ClearScriptCache") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(SUCCEEDED(_scEngine->EmptyScriptCache()));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisplayLoadingIcon
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DisplayLoadingIcon") == 0) {
		Stack->CorrectParams(4);

		const char *Filename = Stack->Pop()->GetString();
		_loadingIconX = Stack->Pop()->GetInt();
		_loadingIconY = Stack->Pop()->GetInt();
		_loadingIconPersistent = Stack->Pop()->GetBool();

		delete _loadingIcon;
		_loadingIcon = new CBSprite(this);
		if (!_loadingIcon || FAILED(_loadingIcon->LoadFile(Filename))) {
			delete _loadingIcon;
			_loadingIcon = NULL;
		} else {
			DisplayContent(false, true);
			Game->_renderer->Flip();
			Game->_renderer->InitLoop();
		}
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HideLoadingIcon
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HideLoadingIcon") == 0) {
		Stack->CorrectParams(0);
		delete _loadingIcon;
		_loadingIcon = NULL;
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DumpTextureStats
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DumpTextureStats") == 0) {
		Stack->CorrectParams(1);
		const char *Filename = Stack->Pop()->GetString();

		_renderer->DumpData(Filename);

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccOutputText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccOutputText") == 0) {
		Stack->CorrectParams(2);
		/* const char *Str = */ Stack->Pop()->GetString();
		/* int Type = */ Stack->Pop()->GetInt();
		// do nothing
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StoreSaveThumbnail
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StoreSaveThumbnail") == 0) {
		Stack->CorrectParams(0);
		delete _cachedThumbnail;
		_cachedThumbnail = new CBSaveThumbHelper(this);
		if (FAILED(_cachedThumbnail->StoreThumbnail())) {
			delete _cachedThumbnail;
			_cachedThumbnail = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteSaveThumbnail
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteSaveThumbnail") == 0) {
		Stack->CorrectParams(0);
		delete _cachedThumbnail;
		_cachedThumbnail = NULL;
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFileChecksum
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFileChecksum") == 0) {
		Stack->CorrectParams(2);
		const char *Filename = Stack->Pop()->GetString();
		bool AsHex = Stack->Pop()->GetBool(false);

		Common::SeekableReadStream *File = _fileManager->OpenFile(Filename, false);
		if (File) {
			crc remainder = crc_initialize();
			byte Buf[1024];
			int BytesRead = 0;

			while (BytesRead < File->size()) {
				int BufSize = MIN((uint32)1024, (uint32)(File->size() - BytesRead));
				BytesRead += File->read(Buf, BufSize);

				for (int i = 0; i < BufSize; i++) {
					remainder = crc_process_byte(Buf[i], remainder);
				}
			}
			crc checksum = crc_finalize(remainder);

			if (AsHex) {
				char Hex[100];
				sprintf(Hex, "%x", checksum);
				Stack->PushString(Hex);
			} else
				Stack->PushInt(checksum);

			_fileManager->CloseFile(File);
			File = NULL;
		} else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EnableScriptProfiling
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "EnableScriptProfiling") == 0) {
		Stack->CorrectParams(0);
		_scEngine->EnableProfiling();
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisableScriptProfiling
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DisableScriptProfiling") == 0) {
		Stack->CorrectParams(0);
		_scEngine->DisableProfiling();
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ShowStatusLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ShowStatusLine") == 0) {
		Stack->CorrectParams(0);
#ifdef __IPHONEOS__
		IOS_ShowStatusLine(TRUE);
#endif
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HideStatusLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HideStatusLine") == 0) {
		Stack->CorrectParams(0);
#ifdef __IPHONEOS__
		IOS_ShowStatusLine(FALSE);
#endif
		Stack->PushNULL();

		return S_OK;
	}

	else return CBObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBGame::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("game");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Name") == 0) {
		_scValue->SetString(_name);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Hwnd (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Hwnd") == 0) {
		_scValue->SetInt((int)_renderer->_window);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CurrentTime (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CurrentTime") == 0) {
		_scValue->SetInt((int)_timer);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WindowsTime (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WindowsTime") == 0) {
		_scValue->SetInt((int)CBPlatform::GetTime());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WindowedMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WindowedMode") == 0) {
		_scValue->SetBool(_renderer->_windowed);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MouseX") == 0) {
		_scValue->SetInt(_mousePos.x);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MouseY") == 0) {
		_scValue->SetInt(_mousePos.y);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MainObject") == 0) {
		_scValue->SetNative(_mainObject, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ActiveObject (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ActiveObject") == 0) {
		_scValue->SetNative(_activeObject, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScreenWidth (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScreenWidth") == 0) {
		_scValue->SetInt(_renderer->_width);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScreenHeight (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScreenHeight") == 0) {
		_scValue->SetInt(_renderer->_height);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Interactive") == 0) {
		_scValue->SetBool(_interactive);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DebugMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DebugMode") == 0) {
		_scValue->SetBool(_dEBUG_DebugMode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundAvailable (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundAvailable") == 0) {
		_scValue->SetBool(_soundMgr->_soundAvailable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SFXVolume") == 0) {
		Game->LOG(0, "**Warning** The SFXVolume attribute is obsolete");
		_scValue->SetInt(_soundMgr->getVolumePercent(SOUND_SFX));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SpeechVolume") == 0) {
		Game->LOG(0, "**Warning** The SpeechVolume attribute is obsolete");
		_scValue->SetInt(_soundMgr->getVolumePercent(SOUND_SPEECH));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MusicVolume") == 0) {
		Game->LOG(0, "**Warning** The MusicVolume attribute is obsolete");
		_scValue->SetInt(_soundMgr->getVolumePercent(SOUND_MUSIC));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MasterVolume") == 0) {
		Game->LOG(0, "**Warning** The MasterVolume attribute is obsolete");
		_scValue->SetInt(_soundMgr->getMasterVolumePercent());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyboard (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Keyboard") == 0) {
		if (_keyboardState) _scValue->SetNative(_keyboardState, true);
		else _scValue->SetNULL();

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Subtitles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Subtitles") == 0) {
		_scValue->SetBool(_subtitles);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesSpeed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesSpeed") == 0) {
		_scValue->SetInt(_subtitlesSpeed);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// VideoSubtitles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "VideoSubtitles") == 0) {
		_scValue->SetBool(_videoSubtitles);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FPS (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FPS") == 0) {
		_scValue->SetInt(_fps);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AcceleratedMode / Accelerated (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AcceleratedMode") == 0 || strcmp(Name, "Accelerated") == 0) {
		_scValue->SetBool(_useD3D);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextEncoding
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TextEncoding") == 0) {
		_scValue->SetInt(_textEncoding);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextRTL
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TextRTL") == 0) {
		_scValue->SetBool(_textRTL);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundBufferSize
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundBufferSize") == 0) {
		_scValue->SetInt(_soundBufferSizeSec);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuspendedRendering
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SuspendedRendering") == 0) {
		_scValue->SetBool(_suspendedRendering);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuppressScriptErrors
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SuppressScriptErrors") == 0) {
		_scValue->SetBool(_suppressScriptErrors);
		return _scValue;
	}


	//////////////////////////////////////////////////////////////////////////
	// Frozen
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Frozen") == 0) {
		_scValue->SetBool(_state == GAME_FROZEN);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSEnabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccTTSEnabled") == 0) {
		_scValue->SetBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSTalk
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccTTSTalk") == 0) {
		_scValue->SetBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSCaptions
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccTTSCaptions") == 0) {
		_scValue->SetBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccTTSKeypress
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccTTSKeypress") == 0) {
		_scValue->SetBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccKeyboardEnabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccKeyboardEnabled") == 0) {
		_scValue->SetBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccKeyboardCursorSkip
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccKeyboardCursorSkip") == 0) {
		_scValue->SetBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccKeyboardPause
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccKeyboardPause") == 0) {
		_scValue->SetBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutorunDisabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AutorunDisabled") == 0) {
		_scValue->SetBool(_autorunDisabled);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveDirectory (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SaveDirectory") == 0) {
		AnsiString dataDir = GetDataDir();
		_scValue->SetString((char *)dataDir.c_str());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveOnExit
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AutoSaveOnExit") == 0) {
		_scValue->SetBool(_autoSaveOnExit);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveSlot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AutoSaveSlot") == 0) {
		_scValue->SetInt(_autoSaveSlot);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorHidden
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CursorHidden") == 0) {
		_scValue->SetBool(_cursorHidden);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Platform (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Platform") == 0) {
		_scValue->SetString(CBPlatform::GetPlatformName().c_str());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeviceType (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeviceType") == 0) {
		_scValue->SetString(GetDeviceType().c_str());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MostRecentSaveSlot (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MostRecentSaveSlot") == 0) {
		_scValue->SetInt(_registry->ReadInt("System", "MostRecentSaveSlot", -1));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Store (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Store") == 0) {
		if (_store) _scValue->SetNative(_store, true);
		else _scValue->SetNULL();

		return _scValue;
	}

	else return CBObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::ScSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
		SetName(Value->GetString());

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MouseX") == 0) {
		_mousePos.x = Value->GetInt();
		ResetMousePos();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MouseY") == 0) {
		_mousePos.y = Value->GetInt();
		ResetMousePos();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Name") == 0) {
		HRESULT res = CBObject::ScSetProperty(Name, Value);
		SetWindowTitle();
		return res;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MainObject") == 0) {
		CBScriptable *obj = Value->GetNative();
		if (obj == NULL || ValidObject((CBObject *)obj)) _mainObject = (CBObject *)obj;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Interactive") == 0) {
		SetInteractive(Value->GetBool());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SFXVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SFXVolume") == 0) {
		Game->LOG(0, "**Warning** The SFXVolume attribute is obsolete");
		Game->_soundMgr->setVolumePercent(SOUND_SFX, (byte)Value->GetInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SpeechVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SpeechVolume") == 0) {
		Game->LOG(0, "**Warning** The SpeechVolume attribute is obsolete");
		Game->_soundMgr->setVolumePercent(SOUND_SPEECH, (byte)Value->GetInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MusicVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MusicVolume") == 0) {
		Game->LOG(0, "**Warning** The MusicVolume attribute is obsolete");
		Game->_soundMgr->setVolumePercent(SOUND_MUSIC, (byte)Value->GetInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MasterVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MasterVolume") == 0) {
		Game->LOG(0, "**Warning** The MasterVolume attribute is obsolete");
		Game->_soundMgr->setMasterVolumePercent((byte)Value->GetInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Subtitles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Subtitles") == 0) {
		_subtitles = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesSpeed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesSpeed") == 0) {
		_subtitlesSpeed = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// VideoSubtitles
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "VideoSubtitles") == 0) {
		_videoSubtitles = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextEncoding
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TextEncoding") == 0) {
		int Enc = Value->GetInt();
		if (Enc < 0) Enc = 0;
		if (Enc >= NUM_TEXT_ENCODINGS) Enc = NUM_TEXT_ENCODINGS - 1;
		_textEncoding = (TTextEncoding)Enc;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextRTL
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TextRTL") == 0) {
		_textRTL = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundBufferSize
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundBufferSize") == 0) {
		_soundBufferSizeSec = Value->GetInt();
		_soundBufferSizeSec = MAX(3, _soundBufferSizeSec);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuspendedRendering
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SuspendedRendering") == 0) {
		_suspendedRendering = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SuppressScriptErrors
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SuppressScriptErrors") == 0) {
		_suppressScriptErrors = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutorunDisabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AutorunDisabled") == 0) {
		_autorunDisabled = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveOnExit
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AutoSaveOnExit") == 0) {
		_autoSaveOnExit = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoSaveSlot
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AutoSaveSlot") == 0) {
		_autoSaveSlot = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorHidden
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CursorHidden") == 0) {
		_cursorHidden = Value->GetBool();
		return S_OK;
	}

	else return CBObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CBGame::ScToString() {
	return "[game object]";
}



#define QUICK_MSG_DURATION 3000
//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::DisplayQuickMsg() {
	if (_quickMessages.GetSize() == 0 || !_systemFont) return S_OK;

	int i;

	// update
	for (i = 0; i < _quickMessages.GetSize(); i++) {
		if (_currentTime - _quickMessages[i]->_startTime >= QUICK_MSG_DURATION) {
			delete _quickMessages[i];
			_quickMessages.RemoveAt(i);
			i--;
		}
	}

	int PosY = 20;

	// display
	for (i = 0; i < _quickMessages.GetSize(); i++) {
		_systemFont->DrawText((byte *)_quickMessages[i]->GetText(), 0, PosY, _renderer->_width);
		PosY += _systemFont->GetTextHeight((byte *)_quickMessages[i]->GetText(), _renderer->_width);
	}
	return S_OK;
}


#define MAX_QUICK_MSG 5
//////////////////////////////////////////////////////////////////////////
void CBGame::QuickMessage(const char *Text) {
	if (_quickMessages.GetSize() >= MAX_QUICK_MSG) {
		delete _quickMessages[0];
		_quickMessages.RemoveAt(0);
	}
	_quickMessages.Add(new CBQuickMsg(Game, Text));
}


//////////////////////////////////////////////////////////////////////////
void CBGame::QuickMessageForm(LPSTR fmt, ...) {
	char buff[256];
	va_list va;

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	QuickMessage(buff);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::RegisterObject(CBObject *Object) {
	_regObjects.Add(Object);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::UnregisterObject(CBObject *Object) {
	if (!Object) return S_OK;

	int i;

	// is it a window?
	for (i = 0; i < _windows.GetSize(); i++) {
		if ((CBObject *)_windows[i] == Object) {
			_windows.RemoveAt(i);

			// get new focused window
			if (_focusedWindow == Object) _focusedWindow = NULL;

			break;
		}
	}

	// is it active object?
	if (_activeObject == Object) _activeObject = NULL;

	// is it main object?
	if (_mainObject == Object) _mainObject = NULL;

	if (_store) _store->OnObjectDestroyed(Object);

	// destroy object
	for (i = 0; i < _regObjects.GetSize(); i++) {
		if (_regObjects[i] == Object) {
			_regObjects.RemoveAt(i);
			if (!_loadInProgress) CSysClassRegistry::GetInstance()->EnumInstances(InvalidateValues, "CScValue", (void *)Object);
			delete Object;
			return S_OK;
		}
	}

	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::InvalidateValues(void *Value, void *Data) {
	CScValue *val = (CScValue *)Value;
	if (val->IsNative() && val->GetNative() == Data) {
		if (!val->_persistent && ((CBScriptable *)Data)->_refCount == 1) {
			((CBScriptable *)Data)->_refCount++;
		}
		val->SetNative(NULL);
		val->SetNULL();
	}
}



//////////////////////////////////////////////////////////////////////////
bool CBGame::ValidObject(CBObject *Object) {
	if (!Object) return false;
	if (Object == this) return true;

	for (int i = 0; i < _regObjects.GetSize(); i++) {
		if (_regObjects[i] == Object) return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::PublishNatives() {
	if (!_scEngine || !_scEngine->_compilerAvailable) return;

	_scEngine->ExtDefineFunction("LOG");
	_scEngine->ExtDefineFunction("String");
	_scEngine->ExtDefineFunction("MemBuffer");
	_scEngine->ExtDefineFunction("File");
	_scEngine->ExtDefineFunction("Date");
	_scEngine->ExtDefineFunction("Array");
	_scEngine->ExtDefineFunction("TcpClient");
	_scEngine->ExtDefineFunction("Object");
	//_scEngine->ExtDefineFunction("Game");
	_scEngine->ExtDefineFunction("Sleep");
	_scEngine->ExtDefineFunction("WaitFor");
	_scEngine->ExtDefineFunction("Random");
	_scEngine->ExtDefineFunction("SetScriptTimeSlice");
	_scEngine->ExtDefineFunction("MakeRGBA");
	_scEngine->ExtDefineFunction("MakeRGB");
	_scEngine->ExtDefineFunction("MakeHSL");
	_scEngine->ExtDefineFunction("RGB");
	_scEngine->ExtDefineFunction("GetRValue");
	_scEngine->ExtDefineFunction("GetGValue");
	_scEngine->ExtDefineFunction("GetBValue");
	_scEngine->ExtDefineFunction("GetAValue");
	_scEngine->ExtDefineFunction("GetHValue");
	_scEngine->ExtDefineFunction("GetSValue");
	_scEngine->ExtDefineFunction("GetLValue");
	_scEngine->ExtDefineFunction("Debug");

	_scEngine->ExtDefineFunction("ToString");
	_scEngine->ExtDefineFunction("ToInt");
	_scEngine->ExtDefineFunction("ToBool");
	_scEngine->ExtDefineFunction("ToFloat");

	_scEngine->ExtDefineVariable("Game");
	_scEngine->ExtDefineVariable("Math");
	_scEngine->ExtDefineVariable("Directory");
	_scEngine->ExtDefineVariable("self");
	_scEngine->ExtDefineVariable("this");
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::ExternalCall(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	CScValue *this_obj;

	//////////////////////////////////////////////////////////////////////////
	// LOG
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "LOG") == 0) {
		Stack->CorrectParams(1);
		Game->LOG(0, "sc: %s", Stack->Pop()->GetString());
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// String
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "String") == 0) {
		this_obj = ThisStack->GetTop();

		this_obj->SetNative(makeSXString(Game, Stack));
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// MemBuffer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MemBuffer") == 0) {
		this_obj = ThisStack->GetTop();

		this_obj->SetNative(makeSXMemBuffer(Game, Stack));
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// File
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "File") == 0) {
		this_obj = ThisStack->GetTop();

		this_obj->SetNative(makeSXFile(Game, Stack));
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Date
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Date") == 0) {
		this_obj = ThisStack->GetTop();

		this_obj->SetNative(makeSXDate(Game, Stack));
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Array
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Array") == 0) {
		this_obj = ThisStack->GetTop();

		this_obj->SetNative(makeSXArray(Game, Stack));
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Object
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Object") == 0) {
		this_obj = ThisStack->GetTop();

		this_obj->SetNative(makeSXObject(Game, Stack));
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Sleep
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Sleep") == 0) {
		Stack->CorrectParams(1);

		Script->Sleep((uint32)Stack->Pop()->GetInt());
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// WaitFor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WaitFor") == 0) {
		Stack->CorrectParams(1);

		CBScriptable *obj = Stack->Pop()->GetNative();
		if (ValidObject((CBObject *)obj)) Script->WaitForExclusive((CBObject *)obj);
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Random
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Random") == 0) {
		Stack->CorrectParams(2);

		int from = Stack->Pop()->GetInt();
		int to   = Stack->Pop()->GetInt();

		Stack->PushInt(CBUtils::RandomInt(from, to));
	}

	//////////////////////////////////////////////////////////////////////////
	// SetScriptTimeSlice
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetScriptTimeSlice") == 0) {
		Stack->CorrectParams(1);

		Script->_timeSlice = (uint32)Stack->Pop()->GetInt();
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// MakeRGBA / MakeRGB / RGB
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MakeRGBA") == 0 || strcmp(Name, "MakeRGB") == 0 || strcmp(Name, "RGB") == 0) {
		Stack->CorrectParams(4);
		int r = Stack->Pop()->GetInt();
		int g = Stack->Pop()->GetInt();
		int b = Stack->Pop()->GetInt();
		int a;
		CScValue *val = Stack->Pop();
		if (val->IsNULL()) a = 255;
		else a = val->GetInt();

		Stack->PushInt(DRGBA(r, g, b, a));
	}

	//////////////////////////////////////////////////////////////////////////
	// MakeHSL
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MakeHSL") == 0) {
		Stack->CorrectParams(3);
		int h = Stack->Pop()->GetInt();
		int s = Stack->Pop()->GetInt();
		int l = Stack->Pop()->GetInt();

		Stack->PushInt(CBUtils::HSLtoRGB(h, s, l));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetRValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetRValue") == 0) {
		Stack->CorrectParams(1);

		uint32 rgba = (uint32)Stack->Pop()->GetInt();
		Stack->PushInt(D3DCOLGetR(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetGValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetGValue") == 0) {
		Stack->CorrectParams(1);

		uint32 rgba = (uint32)Stack->Pop()->GetInt();
		Stack->PushInt(D3DCOLGetG(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetBValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetBValue") == 0) {
		Stack->CorrectParams(1);

		uint32 rgba = (uint32)Stack->Pop()->GetInt();
		Stack->PushInt(D3DCOLGetB(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetAValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetAValue") == 0) {
		Stack->CorrectParams(1);

		uint32 rgba = (uint32)Stack->Pop()->GetInt();
		Stack->PushInt(D3DCOLGetA(rgba));
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHValue") == 0) {
		Stack->CorrectParams(1);
		uint32 rgb = (uint32)Stack->Pop()->GetInt();

		byte H, S, L;
		CBUtils::RGBtoHSL(rgb, &H, &S, &L);
		Stack->PushInt(H);
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSValue") == 0) {
		Stack->CorrectParams(1);
		uint32 rgb = (uint32)Stack->Pop()->GetInt();

		byte H, S, L;
		CBUtils::RGBtoHSL(rgb, &H, &S, &L);
		Stack->PushInt(S);
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLValue
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetLValue") == 0) {
		Stack->CorrectParams(1);
		uint32 rgb = (uint32)Stack->Pop()->GetInt();

		byte H, S, L;
		CBUtils::RGBtoHSL(rgb, &H, &S, &L);
		Stack->PushInt(L);
	}

	//////////////////////////////////////////////////////////////////////////
	// Debug
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Debug") == 0) {
		Stack->CorrectParams(0);

		if (Game->GetDebugMgr()->_enabled) {
			Game->GetDebugMgr()->OnScriptHitBreakpoint(Script);
			Script->Sleep(0);
		}
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// ToString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ToString") == 0) {
		Stack->CorrectParams(1);
		const char *Str = Stack->Pop()->GetString();
		char *Str2 = new char[strlen(Str) + 1];
		strcpy(Str2, Str);
		Stack->PushString(Str2);
		delete [] Str2;
	}

	//////////////////////////////////////////////////////////////////////////
	// ToInt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ToInt") == 0) {
		Stack->CorrectParams(1);
		int Val = Stack->Pop()->GetInt();
		Stack->PushInt(Val);
	}

	//////////////////////////////////////////////////////////////////////////
	// ToFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ToFloat") == 0) {
		Stack->CorrectParams(1);
		double Val = Stack->Pop()->GetFloat();
		Stack->PushFloat(Val);
	}

	//////////////////////////////////////////////////////////////////////////
	// ToBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ToBool") == 0) {
		Stack->CorrectParams(1);
		bool Val = Stack->Pop()->GetBool();
		Stack->PushBool(Val);
	}

	//////////////////////////////////////////////////////////////////////////
	// failure
	else {
		Script->RuntimeError("Call to undefined function '%s'. Ignored.", Name);
		Stack->CorrectParams(0);
		Stack->PushNULL();
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::ShowCursor() {
	if (_cursorHidden) return S_OK;

	if (!_interactive && Game->_state == GAME_RUNNING) {
		if (_cursorNoninteractive) return DrawCursor(_cursorNoninteractive);
	} else {
		if (_activeObject && !FAILED(_activeObject->ShowCursor())) return S_OK;
		else {
			if (_activeObject && _activeCursor && _activeObject->GetExtendedFlag("usable")) return DrawCursor(_activeCursor);
			else if (_cursor) return DrawCursor(_cursor);
		}
	}
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::SaveGame(int slot, const char *desc, bool quickSave) {
	char Filename[MAX_PATH + 1];
	GetSaveSlotFilename(slot, Filename);

	LOG(0, "Saving game '%s'...", Filename);

	Game->ApplyEvent("BeforeSave", true);

	HRESULT ret;

	_indicatorDisplay = true;
	_indicatorProgress = 0;
	CBPersistMgr *pm = new CBPersistMgr(Game);
	if (FAILED(ret = pm->InitSave(desc))) goto save_finish;

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

	if (FAILED(ret = CSysClassRegistry::GetInstance()->SaveTable(Game, pm, quickSave))) goto save_finish;
	if (FAILED(ret = CSysClassRegistry::GetInstance()->SaveInstances(Game, pm, quickSave))) goto save_finish;
	if (FAILED(ret = pm->SaveFile(Filename))) goto save_finish;

	_registry->WriteInt("System", "MostRecentSaveSlot", slot);

save_finish:
	delete pm;
	_indicatorDisplay = false;

	delete _saveLoadImage;
	_saveLoadImage = NULL;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::LoadGame(int Slot) {
	//Game->LOG(0, "Load start %d", CBUtils::GetUsedMemMB());

	_loading = false;
	_scheduledLoadSlot = -1;

	char Filename[MAX_PATH + 1];
	GetSaveSlotFilename(Slot, Filename);

	return LoadGame(Filename);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::LoadGame(const char *Filename) {
	LOG(0, "Loading game '%s'...", Filename);
	GetDebugMgr()->OnGameShutdown();

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
	if (FAILED(ret = pm->InitLoad(Filename))) goto load_finish;

	//if(FAILED(ret = Cleanup())) goto load_finish;
	if (FAILED(ret = CSysClassRegistry::GetInstance()->LoadTable(Game, pm))) goto load_finish;
	if (FAILED(ret = CSysClassRegistry::GetInstance()->LoadInstances(Game, pm))) goto load_finish;

	// data initialization after load
	InitAfterLoad();

	Game->ApplyEvent("AfterLoad", true);

	DisplayContent(true, false);
	//_renderer->Flip();

	GetDebugMgr()->OnGameInit();

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
HRESULT CBGame::InitAfterLoad() {
	CSysClassRegistry::GetInstance()->EnumInstances(AfterLoadRegion,   "CBRegion",   NULL);
	CSysClassRegistry::GetInstance()->EnumInstances(AfterLoadSubFrame, "CBSubFrame", NULL);
	CSysClassRegistry::GetInstance()->EnumInstances(AfterLoadSound,    "CBSound",    NULL);
	CSysClassRegistry::GetInstance()->EnumInstances(AfterLoadFont,     "CBFontTT",   NULL);
	CSysClassRegistry::GetInstance()->EnumInstances(AfterLoadScript,   "CScScript",  NULL);

	_scEngine->RefreshScriptBreakpoints();
	if (_store) _store->AfterLoad();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBGame::AfterLoadRegion(void *Region, void *Data) {
	((CBRegion *)Region)->CreateRegion();
}


//////////////////////////////////////////////////////////////////////////
void CBGame::AfterLoadSubFrame(void *Subframe, void *Data) {
	((CBSubFrame *)Subframe)->SetSurfaceSimple();
}


//////////////////////////////////////////////////////////////////////////
void CBGame::AfterLoadSound(void *Sound, void *Data) {
	((CBSound *)Sound)->SetSoundSimple();
}

//////////////////////////////////////////////////////////////////////////
void CBGame::AfterLoadFont(void *Font, void *Data) {
	((CBFont*)Font)->AfterLoad();
}

//////////////////////////////////////////////////////////////////////////
void CBGame::AfterLoadScript(void *script, void *data) {
	((CScScript *)script)->AfterLoad();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::DisplayWindows(bool InGame) {
	HRESULT res;

	int i;

	// did we lose focus? focus topmost window
	if (_focusedWindow == NULL || !_focusedWindow->_visible || _focusedWindow->_disable) {
		_focusedWindow = NULL;
		for (i = _windows.GetSize() - 1; i >= 0; i--) {
			if (_windows[i]->_visible && !_windows[i]->_disable) {
				_focusedWindow = _windows[i];
				break;
			}
		}
	}

	// display all windows
	for (i = 0; i < _windows.GetSize(); i++) {
		if (_windows[i]->_visible && _windows[i]->_inGame == InGame) {

			res = _windows[i]->Display();
			if (FAILED(res)) return res;
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::PlayMusic(int Channel, const char *Filename, bool Looping, uint32 LoopStart) {
	if (Channel >= NUM_MUSIC_CHANNELS) {
		Game->LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", Channel, NUM_MUSIC_CHANNELS);
		return E_FAIL;
	}

	delete _music[Channel];
	_music[Channel] = NULL;

	_music[Channel] = new CBSound(Game);
	if (_music[Channel] && SUCCEEDED(_music[Channel]->SetSound(Filename, SOUND_MUSIC, true))) {
		if (_musicStartTime[Channel]) {
			_music[Channel]->SetPositionTime(_musicStartTime[Channel]);
			_musicStartTime[Channel] = 0;
		}
		if (LoopStart) _music[Channel]->SetLoopStart(LoopStart);
		return _music[Channel]->Play(Looping);
	} else {
		delete _music[Channel];
		_music[Channel] = NULL;
		return E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::StopMusic(int Channel) {
	if (Channel >= NUM_MUSIC_CHANNELS) {
		Game->LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", Channel, NUM_MUSIC_CHANNELS);
		return E_FAIL;
	}

	if (_music[Channel]) {
		_music[Channel]->Stop();
		delete _music[Channel];
		_music[Channel] = NULL;
		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::PauseMusic(int Channel) {
	if (Channel >= NUM_MUSIC_CHANNELS) {
		Game->LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", Channel, NUM_MUSIC_CHANNELS);
		return E_FAIL;
	}

	if (_music[Channel]) return _music[Channel]->Pause();
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::ResumeMusic(int Channel) {
	if (Channel >= NUM_MUSIC_CHANNELS) {
		Game->LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", Channel, NUM_MUSIC_CHANNELS);
		return E_FAIL;
	}

	if (_music[Channel]) return _music[Channel]->Resume();
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::SetMusicStartTime(int Channel, uint32 Time) {

	if (Channel >= NUM_MUSIC_CHANNELS) {
		Game->LOG(0, "**Error** Attempting to use music channel %d (max num channels: %d)", Channel, NUM_MUSIC_CHANNELS);
		return E_FAIL;
	}

	_musicStartTime[Channel] = Time;
	if (_music[Channel] && _music[Channel]->IsPlaying()) return _music[Channel]->SetPositionTime(Time);
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::LoadSettings(const char *Filename) {
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


	byte *OrigBuffer = Game->_fileManager->ReadWholeFile(Filename);
	if (OrigBuffer == NULL) {
		Game->LOG(0, "CBGame::LoadSettings failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret = S_OK;

	byte *Buffer = OrigBuffer;
	byte *params;
	int cmd;
	CBParser parser(Game);

	if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_SETTINGS) {
		Game->LOG(0, "'SETTINGS' keyword expected in game settings file.");
		return E_FAIL;
	}
	Buffer = params;
	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_GAME:
			delete[] _settingsGameFile;
			_settingsGameFile = new char[strlen((char *)params) + 1];
			if (_settingsGameFile) strcpy(_settingsGameFile, (char *)params);
			break;

		case TOKEN_STRING_TABLE:
			if (FAILED(_stringTable->LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_RESOLUTION:
			parser.ScanStr((char *)params, "%d,%d", &_settingsResWidth, &_settingsResHeight);
			break;

		case TOKEN_REQUIRE_3D_ACCELERATION:
			parser.ScanStr((char *)params, "%b", &_settingsRequireAcceleration);
			break;

		case TOKEN_REQUIRE_SOUND:
			parser.ScanStr((char *)params, "%b", &_settingsRequireSound);
			break;

		case TOKEN_HWTL_MODE:
			parser.ScanStr((char *)params, "%d", &_settingsTLMode);
			break;

		case TOKEN_ALLOW_WINDOWED_MODE:
			parser.ScanStr((char *)params, "%b", &_settingsAllowWindowed);
			break;

		case TOKEN_ALLOW_DESKTOP_RES:
			parser.ScanStr((char *)params, "%b", &_settingsAllowDesktopRes);
			break;

		case TOKEN_ALLOW_ADVANCED:
			parser.ScanStr((char *)params, "%b", &_settingsAllowAdvanced);
			break;

		case TOKEN_ALLOW_ACCESSIBILITY_TAB:
			parser.ScanStr((char *)params, "%b", &_settingsAllowAccessTab);
			break;

		case TOKEN_ALLOW_ABOUT_TAB:
			parser.ScanStr((char *)params, "%b", &_settingsAllowAboutTab);
			break;

		case TOKEN_REGISTRY_PATH:
			_registry->SetBasePath((char *)params);
			break;

		case TOKEN_RICH_SAVED_GAMES:
			parser.ScanStr((char *)params, "%b", &_richSavedGames);
			break;

		case TOKEN_SAVED_GAME_EXT:
			CBUtils::SetString(&_savedGameExt, (char *)params);
			break;

		case TOKEN_GUID:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in game settings '%s'", Filename);
		ret = E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading game settings '%s'", Filename);
		ret = E_FAIL;
	}

	_settingsAllowWindowed = _registry->ReadBool("Debug", "AllowWindowed", _settingsAllowWindowed);
	_compressedSavegames = _registry->ReadBool("Debug", "CompressedSavegames", _compressedSavegames);
	//_compressedSavegames = false;

	delete [] OrigBuffer;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::Persist(CBPersistMgr *PersistMgr) {
	if (!PersistMgr->_saving) Cleanup();

	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_activeObject));
	PersistMgr->Transfer(TMEMBER(_capturedObject));
	PersistMgr->Transfer(TMEMBER(_cursorNoninteractive));
	PersistMgr->Transfer(TMEMBER(_doNotExpandStrings));
	PersistMgr->Transfer(TMEMBER(_editorMode));
	PersistMgr->Transfer(TMEMBER(_fader));
	PersistMgr->Transfer(TMEMBER(_freezeLevel));
	PersistMgr->Transfer(TMEMBER(_focusedWindow));
	PersistMgr->Transfer(TMEMBER(_fontStorage));
	PersistMgr->Transfer(TMEMBER(_interactive));
	PersistMgr->Transfer(TMEMBER(_keyboardState));
	PersistMgr->Transfer(TMEMBER(_lastTime));
	PersistMgr->Transfer(TMEMBER(_mainObject));
	for (int i = 0; i < NUM_MUSIC_CHANNELS; i++) {
		PersistMgr->Transfer(TMEMBER(_music[i]));
		PersistMgr->Transfer(TMEMBER(_musicStartTime[i]));
	}

	PersistMgr->Transfer(TMEMBER(_offsetX));
	PersistMgr->Transfer(TMEMBER(_offsetY));
	PersistMgr->Transfer(TMEMBER(_offsetPercentX));
	PersistMgr->Transfer(TMEMBER(_offsetPercentY));

	PersistMgr->Transfer(TMEMBER(_origInteractive));
	PersistMgr->Transfer(TMEMBER_INT(_origState));
	PersistMgr->Transfer(TMEMBER(_personalizedSave));
	PersistMgr->Transfer(TMEMBER(_quitting));

	_regObjects.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_scEngine));
	//PersistMgr->Transfer(TMEMBER(_soundMgr));
	PersistMgr->Transfer(TMEMBER_INT(_state));
	//PersistMgr->Transfer(TMEMBER(_surfaceStorage));
	PersistMgr->Transfer(TMEMBER(_subtitles));
	PersistMgr->Transfer(TMEMBER(_subtitlesSpeed));
	PersistMgr->Transfer(TMEMBER(_systemFont));
	PersistMgr->Transfer(TMEMBER(_videoFont));
	PersistMgr->Transfer(TMEMBER(_videoSubtitles));

	PersistMgr->Transfer(TMEMBER(_timer));
	PersistMgr->Transfer(TMEMBER(_timerDelta));
	PersistMgr->Transfer(TMEMBER(_timerLast));

	PersistMgr->Transfer(TMEMBER(_liveTimer));
	PersistMgr->Transfer(TMEMBER(_liveTimerDelta));
	PersistMgr->Transfer(TMEMBER(_liveTimerLast));

	PersistMgr->Transfer(TMEMBER(_musicCrossfadeRunning));
	PersistMgr->Transfer(TMEMBER(_musicCrossfadeStartTime));
	PersistMgr->Transfer(TMEMBER(_musicCrossfadeLength));
	PersistMgr->Transfer(TMEMBER(_musicCrossfadeChannel1));
	PersistMgr->Transfer(TMEMBER(_musicCrossfadeChannel2));
	PersistMgr->Transfer(TMEMBER(_musicCrossfadeSwap));

	PersistMgr->Transfer(TMEMBER(_loadImageName));
	PersistMgr->Transfer(TMEMBER(_saveImageName));
	PersistMgr->Transfer(TMEMBER(_saveImageX));
	PersistMgr->Transfer(TMEMBER(_saveImageY));
	PersistMgr->Transfer(TMEMBER(_loadImageX));
	PersistMgr->Transfer(TMEMBER(_loadImageY));

	PersistMgr->Transfer(TMEMBER_INT(_textEncoding));
	PersistMgr->Transfer(TMEMBER(_textRTL));

	PersistMgr->Transfer(TMEMBER(_soundBufferSizeSec));
	PersistMgr->Transfer(TMEMBER(_suspendedRendering));

	PersistMgr->Transfer(TMEMBER(_mouseLockRect));

	_windows.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_suppressScriptErrors));
	PersistMgr->Transfer(TMEMBER(_autorunDisabled));

	PersistMgr->Transfer(TMEMBER(_autoSaveOnExit));
	PersistMgr->Transfer(TMEMBER(_autoSaveSlot));
	PersistMgr->Transfer(TMEMBER(_cursorHidden));

	if (PersistMgr->CheckVersion(1, 0, 1))
		PersistMgr->Transfer(TMEMBER(_store));
	else
		_store = NULL;

	if (!PersistMgr->_saving) _quitting = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::FocusWindow(CUIWindow *Window) {
	CUIWindow *Prev = _focusedWindow;

	int i;
	for (i = 0; i < _windows.GetSize(); i++) {
		if (_windows[i] == Window) {
			if (i < _windows.GetSize() - 1) {
				_windows.RemoveAt(i);
				_windows.Add(Window);

				Game->_focusedWindow = Window;
			}

			if (Window->_mode == WINDOW_NORMAL && Prev != Window && Game->ValidObject(Prev) && (Prev->_mode == WINDOW_EXCLUSIVE || Prev->_mode == WINDOW_SYSTEM_EXCLUSIVE))
				return FocusWindow(Prev);
			else return S_OK;
		}
	}
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::Freeze(bool IncludingMusic) {
	if (_freezeLevel == 0) {
		_scEngine->PauseAll();
		_soundMgr->pauseAll(IncludingMusic);
		_origState = _state;
		_origInteractive = _interactive;
		_interactive = true;
	}
	_state = GAME_FROZEN;
	_freezeLevel++;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::Unfreeze() {
	if (_freezeLevel == 0) return S_OK;

	_freezeLevel--;
	if (_freezeLevel == 0) {
		_state = _origState;
		_interactive = _origInteractive;
		_scEngine->ResumeAll();
		_soundMgr->resumeAll();
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBGame::HandleKeypress(Common::Event *event, bool printable) {
	if(IsVideoPlaying()) {
		if(event->kbd.keycode == Common::KEYCODE_ESCAPE) 
			StopVideo();
		return true;
	}
#ifdef __WIN32__
	// TODO: Do we really need to handle this in-engine?
	// handle Alt+F4 on windows
	if (event->type == Common::EVENT_KEYDOWN && event->kbd.keycode == Common::KEYCODE_F4 && (event->kbd.flags == Common::KBD_ALT)) {
		OnWindowClose();
		return true;
		//TODO
	}
#endif

	if (event->type == Common::EVENT_KEYDOWN && event->kbd.keycode == Common::KEYCODE_RETURN && (event->kbd.flags == Common::KBD_ALT)) {
		// TODO: Handle alt-enter as well as alt-return.
		_renderer->SwitchFullscreen();
		return true;
	}


	_keyboardState->handleKeyPress(event);
	_keyboardState->ReadKey(event);
// TODO

	if (_focusedWindow) {
		if (!Game->_focusedWindow->HandleKeypress(event, _keyboardState->_currentPrintable)) {
			/*if (event->type != SDL_TEXTINPUT) {*/
				if (Game->_focusedWindow->CanHandleEvent("Keypress"))
					Game->_focusedWindow->ApplyEvent("Keypress");
				else
					ApplyEvent("Keypress");
			/*}*/
		}
		return true;
	} else /*if (event->type != SDL_TEXTINPUT)*/ {
		ApplyEvent("Keypress");
		return true;
	} //else return true;

	return false;
}

void CBGame::handleKeyRelease(Common::Event *event) {
	_keyboardState->handleKeyRelease(event);
}


//////////////////////////////////////////////////////////////////////////
bool CBGame::HandleMouseWheel(int Delta) {
	bool Handled = false;
	if (_focusedWindow) {
		Handled = Game->_focusedWindow->HandleMouseWheel(Delta);

		if (!Handled) {
			if (Delta < 0 && Game->_focusedWindow->CanHandleEvent("MouseWheelDown")) {
				Game->_focusedWindow->ApplyEvent("MouseWheelDown");
				Handled = true;
			} else if (Game->_focusedWindow->CanHandleEvent("MouseWheelUp")) {
				Game->_focusedWindow->ApplyEvent("MouseWheelUp");
				Handled = true;
			}

		}
	}

	if (!Handled) {
		if (Delta < 0) {
			ApplyEvent("MouseWheelDown");
		} else {
			ApplyEvent("MouseWheelUp");
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::GetVersion(byte  *VerMajor, byte *VerMinor, byte *ExtMajor, byte *ExtMinor) {
	if (VerMajor) *VerMajor = DCGF_VER_MAJOR;
	if (VerMinor) *VerMinor = DCGF_VER_MINOR;

	if (ExtMajor) *ExtMajor = 0;
	if (ExtMinor) *ExtMinor = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::SetWindowTitle() {
	if (_renderer) {
		char Title[512];
		strcpy(Title, _caption[0]);
		if (Title[0] != '\0') strcat(Title, " - ");
		strcat(Title, "WME Lite");


		Utf8String title;
		if (_textEncoding == TEXT_UTF8) {
			title = Utf8String(Title);
		} else {
			warning("CBGame::SetWindowTitle -Ignoring textencoding");
			title = Utf8String(Title);
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
HRESULT CBGame::GetSaveSlotFilename(int Slot, char *Buffer) {
	AnsiString dataDir = GetDataDir();
	//sprintf(Buffer, "%s/save%03d.%s", dataDir.c_str(), Slot, _savedGameExt);
	sprintf(Buffer, "save%03d.%s", Slot, _savedGameExt);
	warning("Saving %s - we really should prefix these things to avoid collisions.", Buffer);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
AnsiString CBGame::GetDataDir() {
	AnsiString userDir = PathUtil::GetUserDirectory();
#ifdef __IPHONEOS__
	return userDir;
#else
	AnsiString baseDir = _registry->GetBasePath();
	return PathUtil::Combine(userDir, baseDir);
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::GetSaveSlotDescription(int Slot, char *Buffer) {
	Buffer[0] = '\0';

	char Filename[MAX_PATH + 1];
	GetSaveSlotFilename(Slot, Filename);
	CBPersistMgr *pm = new CBPersistMgr(Game);
	if (!pm) return E_FAIL;

	_dEBUG_AbsolutePathWarning = false;
	if (FAILED(pm->InitLoad(Filename))) {
		_dEBUG_AbsolutePathWarning = true;
		delete pm;
		return E_FAIL;
	}

	_dEBUG_AbsolutePathWarning = true;
	strcpy(Buffer, pm->_savedDescription);
	delete pm;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBGame::IsSaveSlotUsed(int Slot) {
	char Filename[MAX_PATH + 1];
	GetSaveSlotFilename(Slot, Filename);

	Common::SeekableReadStream *File = _fileManager->OpenFile(Filename, false);
	if (!File) return false;

	_fileManager->CloseFile(File);
	return true;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::EmptySaveSlot(int Slot) {
	char Filename[MAX_PATH + 1];
	GetSaveSlotFilename(Slot, Filename);

	CBPlatform::DeleteFile(Filename);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::SetActiveObject(CBObject *Obj) {
	// not-active when game is frozen
	if (Obj && !Game->_interactive && !Obj->_nonIntMouseEvents) {
		Obj = NULL;
	}

	if (Obj == _activeObject) return S_OK;

	if (_activeObject) _activeObject->ApplyEvent("MouseLeave");
	//if(ValidObject(_activeObject)) _activeObject->ApplyEvent("MouseLeave");
	_activeObject = Obj;
	if (_activeObject) {
		_activeObject->ApplyEvent("MouseEntry");
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::PushViewport(CBViewport *Viewport) {
	_viewportSP++;
	if (_viewportSP >= _viewportStack.GetSize()) _viewportStack.Add(Viewport);
	else _viewportStack[_viewportSP] = Viewport;

	_renderer->SetViewport(Viewport->GetRect());

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::PopViewport() {
	_viewportSP--;
	if (_viewportSP < -1) Game->LOG(0, "Fatal: Viewport stack underflow!");

	if (_viewportSP >= 0 && _viewportSP < _viewportStack.GetSize()) _renderer->SetViewport(_viewportStack[_viewportSP]->GetRect());
	else _renderer->SetViewport(_renderer->_drawOffsetX,
		                            _renderer->_drawOffsetY,
		                            _renderer->_width + _renderer->_drawOffsetX,
		                            _renderer->_height + _renderer->_drawOffsetY);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::GetCurrentViewportRect(RECT *Rect, bool *Custom) {
	if (Rect == NULL) return E_FAIL;
	else {
		if (_viewportSP >= 0) {
			CBPlatform::CopyRect(Rect, _viewportStack[_viewportSP]->GetRect());
			if (Custom) *Custom = true;
		} else {
			CBPlatform::SetRect(Rect,   _renderer->_drawOffsetX,
			                    _renderer->_drawOffsetY,
			                    _renderer->_width + _renderer->_drawOffsetX,
			                    _renderer->_height + _renderer->_drawOffsetY);
			if (Custom) *Custom = false;
		}

		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::GetCurrentViewportOffset(int *OffsetX, int *OffsetY) {
	if (_viewportSP >= 0) {
		if (OffsetX) *OffsetX = _viewportStack[_viewportSP]->_offsetX;
		if (OffsetY) *OffsetY = _viewportStack[_viewportSP]->_offsetY;
	} else {
		if (OffsetX) *OffsetX = 0;
		if (OffsetY) *OffsetY = 0;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::WindowLoadHook(CUIWindow *Win, char **Buf, char **Params) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::WindowScriptMethodHook(CUIWindow *Win, CScScript *Script, CScStack *Stack, const char *Name) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::SetInteractive(bool State) {
	_interactive = State;
	if (_transMgr) _transMgr->_origInteractive = State;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::ResetMousePos() {
	POINT p;
	p.x = _mousePos.x + _renderer->_drawOffsetX;
	p.y = _mousePos.y + _renderer->_drawOffsetY;

	CBPlatform::SetCursorPos(p.x, p.y);
}


//////////////////////////////////////////////////////////////////////////
void CBGame::SetResourceModule(HMODULE ResModule) {
	_resourceModule = ResModule;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::DisplayContent(bool update, bool displayAll) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::DisplayContentSimple() {
	// fill black
	_renderer->Fill(0, 0, 0);
	if (_indicatorDisplay) DisplayIndicator();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::DisplayIndicator() {
	if (_saveLoadImage) {
		RECT rc;
		CBPlatform::SetRect(&rc, 0, 0, _saveLoadImage->getWidth(), _saveLoadImage->getHeight());
		if (_loadInProgress) _saveLoadImage->displayTrans(_loadImageX, _loadImageY, rc);
		else _saveLoadImage->displayTrans(_saveImageX, _saveImageY, rc);
	}

	if ((!_indicatorDisplay && _indicatorWidth <= 0) || _indicatorHeight <= 0) return S_OK;
	_renderer->SetupLines();
	for (int i = 0; i < _indicatorHeight; i++)
		_renderer->DrawLine(_indicatorX, _indicatorY + i, _indicatorX + (int)(_indicatorWidth * (float)((float)_indicatorProgress / 100.0f)), _indicatorY + i, _indicatorColor);

	_renderer->Setup2D();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::UpdateMusicCrossfade() {
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

	if (!_music[_musicCrossfadeChannel1]->IsPlaying()) _music[_musicCrossfadeChannel1]->Play();
	if (!_music[_musicCrossfadeChannel2]->IsPlaying()) _music[_musicCrossfadeChannel2]->Play();

	uint32 CurrentTime = Game->_liveTimer - _musicCrossfadeStartTime;

	if (CurrentTime >= _musicCrossfadeLength) {
		_musicCrossfadeRunning = false;
		//_music[_musicCrossfadeChannel2]->SetVolume(GlobMusicVol);
		_music[_musicCrossfadeChannel2]->SetVolume(100);

		_music[_musicCrossfadeChannel1]->Stop();
		//_music[_musicCrossfadeChannel1]->SetVolume(GlobMusicVol);
		_music[_musicCrossfadeChannel1]->SetVolume(100);


		if (_musicCrossfadeSwap) {
			// swap channels
			CBSound *Dummy = _music[_musicCrossfadeChannel1];
			int DummyInt = _musicStartTime[_musicCrossfadeChannel1];

			_music[_musicCrossfadeChannel1] = _music[_musicCrossfadeChannel2];
			_musicStartTime[_musicCrossfadeChannel1] = _musicStartTime[_musicCrossfadeChannel2];

			_music[_musicCrossfadeChannel2] = Dummy;
			_musicStartTime[_musicCrossfadeChannel2] = DummyInt;
		}
	} else {
		//_music[_musicCrossfadeChannel1]->SetVolume(GlobMusicVol - (float)CurrentTime / (float)_musicCrossfadeLength * GlobMusicVol);
		//_music[_musicCrossfadeChannel2]->SetVolume((float)CurrentTime / (float)_musicCrossfadeLength * GlobMusicVol);
		_music[_musicCrossfadeChannel1]->SetVolume(100 - (float)CurrentTime / (float)_musicCrossfadeLength * 100);
		_music[_musicCrossfadeChannel2]->SetVolume((float)CurrentTime / (float)_musicCrossfadeLength * 100);

		//Game->QuickMessageForm("%d %d", _music[_musicCrossfadeChannel1]->GetVolume(), _music[_musicCrossfadeChannel2]->GetVolume());
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::ResetContent() {
	_scEngine->ClearGlobals();
	//_timer = 0;
	//_liveTimer = 0;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBGame::DEBUG_DumpClassRegistry() {
	warning("DEBUG_DumpClassRegistry - untested");
	Common::DumpFile *f = new Common::DumpFile;
	f->open("zz_class_reg_dump.log");

	CSysClassRegistry::GetInstance()->DumpClasses(f);
	
	f->close();
	delete f;
	Game->QuickMessage("Classes dump completed.");
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::InvalidateDeviceObjects() {
	for (int i = 0; i < _regObjects.GetSize(); i++) {
		_regObjects[i]->InvalidateDeviceObjects();
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::RestoreDeviceObjects() {
	for (int i = 0; i < _regObjects.GetSize(); i++) {
		_regObjects[i]->RestoreDeviceObjects();
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::SetWaitCursor(const char *Filename) {
	delete _cursorNoninteractive;
	_cursorNoninteractive = NULL;

	_cursorNoninteractive = new CBSprite(Game);
	if (!_cursorNoninteractive || FAILED(_cursorNoninteractive->LoadFile(Filename))) {
		delete _cursorNoninteractive;
		_cursorNoninteractive = NULL;
		return E_FAIL;
	} else return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBGame::IsVideoPlaying()
{
	if (_videoPlayer->isPlaying()) return true;
	if (_theoraPlayer && _theoraPlayer->isPlaying()) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::StopVideo()
{
	if (_videoPlayer->isPlaying()) _videoPlayer->stop();
	if (_theoraPlayer && _theoraPlayer->isPlaying()) {
		_theoraPlayer->stop();
		delete _theoraPlayer;
		_theoraPlayer = NULL;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::DrawCursor(CBSprite *Cursor) {
	if (!Cursor) return E_FAIL;
	if (Cursor != _lastCursor) {
		Cursor->Reset();
		_lastCursor = Cursor;
	}
	return Cursor->Draw(_mousePos.x, _mousePos.y);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnActivate(bool Activate, bool RefreshMouse) {
	if (_shuttingDown || !_renderer) return S_OK;

	_renderer->_active = Activate;

	if (RefreshMouse) {
		POINT p;
		GetMousePos(&p);
		SetActiveObject(_renderer->GetObjectAt(p.x, p.y));
	}

	if (Activate) _soundMgr->resumeAll();
	else _soundMgr->pauseAll();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnMouseLeftDown() {
	if (_activeObject) _activeObject->HandleMouse(MOUSE_CLICK, MOUSE_BUTTON_LEFT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("LeftClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("LeftClick");
		}
	}

	if (_activeObject != NULL) _capturedObject = _activeObject;
	_mouseLeftDown = true;
	CBPlatform::SetCapture(_renderer->_window);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnMouseLeftUp() {
	if (_activeObject) _activeObject->HandleMouse(MOUSE_RELEASE, MOUSE_BUTTON_LEFT);

	CBPlatform::ReleaseCapture();
	_capturedObject = NULL;
	_mouseLeftDown = false;

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("LeftRelease"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("LeftRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnMouseLeftDblClick() {
	if (_state == GAME_RUNNING && !_interactive) return S_OK;

	if (_activeObject) _activeObject->HandleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_LEFT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("LeftDoubleClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("LeftDoubleClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnMouseRightDblClick() {
	if (_state == GAME_RUNNING && !_interactive) return S_OK;

	if (_activeObject) _activeObject->HandleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_RIGHT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("RightDoubleClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("RightDoubleClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnMouseRightDown() {
	if (_activeObject) _activeObject->HandleMouse(MOUSE_CLICK, MOUSE_BUTTON_RIGHT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("RightClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("RightClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnMouseRightUp() {
	if (_activeObject) _activeObject->HandleMouse(MOUSE_RELEASE, MOUSE_BUTTON_RIGHT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("RightRelease"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("RightRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnMouseMiddleDown() {
	if (_state == GAME_RUNNING && !_interactive) return S_OK;

	if (_activeObject) _activeObject->HandleMouse(MOUSE_CLICK, MOUSE_BUTTON_MIDDLE);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("MiddleClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("MiddleClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnMouseMiddleUp() {
	if (_activeObject) _activeObject->HandleMouse(MOUSE_RELEASE, MOUSE_BUTTON_MIDDLE);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("MiddleRelease"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("MiddleRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnPaint() {
	if (_renderer && _renderer->_windowed && _renderer->_ready) {
		_renderer->InitLoop();
		DisplayContent(false, true);
		DisplayDebugInfo();
		_renderer->WindowedBlt();
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnWindowClose() {
	if (CanHandleEvent("QuitGame")) {
		if (_state != GAME_FROZEN) Game->ApplyEvent("QuitGame");
		return S_OK;
	} else return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::DisplayDebugInfo() {
	char str[100];

	if (_dEBUG_ShowFPS) {
		sprintf(str, "FPS: %d", Game->_fps);
		_systemFont->DrawText((byte *)str, 0, 0, 100, TAL_LEFT);
	}

	if (Game->_dEBUG_DebugMode) {
		if (!Game->_renderer->_windowed)
			sprintf(str, "Mode: %dx%dx%d", _renderer->_width, _renderer->_height, _renderer->_bPP);
		else
			sprintf(str, "Mode: %dx%d windowed", _renderer->_width, _renderer->_height);

		strcat(str, " (");
		strcat(str, _renderer->GetName());
		strcat(str, ")");
		_systemFont->DrawText((byte *)str, 0, 0, _renderer->_width, TAL_RIGHT);

		_renderer->DisplayDebugInfo();

		int ScrTotal, ScrRunning, ScrWaiting, ScrPersistent;
		ScrTotal = _scEngine->GetNumScripts(&ScrRunning, &ScrWaiting, &ScrPersistent);
		sprintf(str, "Running scripts: %d (r:%d w:%d p:%d)", ScrTotal, ScrRunning, ScrWaiting, ScrPersistent);
		_systemFont->DrawText((byte *)str, 0, 70, _renderer->_width, TAL_RIGHT);


		sprintf(str, "Timer: %d", _timer);
		Game->_systemFont->DrawText((byte *)str, 0, 130, _renderer->_width, TAL_RIGHT);

		if (_activeObject != NULL) _systemFont->DrawText((byte *)_activeObject->_name, 0, 150, _renderer->_width, TAL_RIGHT);

		sprintf(str, "GfxMem: %dMB", _usedMem / (1024 * 1024));
		_systemFont->DrawText((byte *)str, 0, 170, _renderer->_width, TAL_RIGHT);

	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CBDebugger *CBGame::GetDebugMgr() {
	if (!_debugMgr) _debugMgr = new CBDebugger(this);
	return _debugMgr;
}


//////////////////////////////////////////////////////////////////////////
void CBGame::GetMousePos(POINT *Pos) {
	CBPlatform::GetCursorPos(Pos);

	Pos->x -= _renderer->_drawOffsetX;
	Pos->y -= _renderer->_drawOffsetY;

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
		if (!CBPlatform::PtInRect(&_mouseLockRect, *Pos)) {
			Pos->x = MAX(_mouseLockRect.left, Pos->x);
			Pos->y = MAX(_mouseLockRect.top, Pos->y);

			Pos->x = MIN(_mouseLockRect.right, Pos->x);
			Pos->y = MIN(_mouseLockRect.bottom, Pos->y);

			POINT NewPos = *Pos;

			NewPos.x += _renderer->_drawOffsetX;
			NewPos.y += _renderer->_drawOffsetY;

			CBPlatform::SetCursorPos(NewPos.x, NewPos.y);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::MiniUpdate() {
	if (!_miniUpdateEnabled) return S_OK;

	if (CBPlatform::GetTime() - _lastMiniUpdate > 200) {
		if (_soundMgr) _soundMgr->initLoop();
		_lastMiniUpdate = CBPlatform::GetTime();
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBGame::OnScriptShutdown(CScScript *Script) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBGame::IsLeftDoubleClick() {
	return IsDoubleClick(0);
}

//////////////////////////////////////////////////////////////////////////
bool CBGame::IsRightDoubleClick() {
	return IsDoubleClick(1);
}

//////////////////////////////////////////////////////////////////////////
bool CBGame::IsDoubleClick(int buttonIndex) {
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
void CBGame::AutoSaveOnExit() {
	_soundMgr->saveSettings();
	_registry->SaveValues();

	if (!_autoSaveOnExit) return;
	if (_state == GAME_FROZEN) return;

	SaveGame(_autoSaveSlot, "autosave", true);
}

//////////////////////////////////////////////////////////////////////////
void CBGame::AddMem(int bytes) {
	_usedMem += bytes;
}

//////////////////////////////////////////////////////////////////////////
AnsiString CBGame::GetDeviceType() const {
#ifdef __IPHONEOS__
	char devType[128];
	IOS_GetDeviceType(devType);
	return AnsiString(devType);
#else
	return "computer";
#endif
}

} // end of namespace WinterMute
