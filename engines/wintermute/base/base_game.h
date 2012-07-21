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

#ifndef WINTERMUTE_BGAME_H
#define WINTERMUTE_BGAME_H

#include "engines/wintermute/base/base_Debugger.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/math/rect32.h"
#include "common/events.h"

namespace WinterMute {

typedef void (*ENGINE_LOG_CALLBACK)(char *Text, bool Result, void *Data);

class CBSoundMgr;
class CBFader;
class CBFont;
class CBFileManager;
class CBTransitionMgr;
class CScEngine;
class CBFontStorage;
class CBStringTable;
class CBQuickMsg;
class CUIWindow;
class CBViewport;
class CBRenderer;
class CBRegistry;
class CBSaveThumbHelper;
class CBSurfaceStorage;
class CSXMath;
class CBKeyboardState;
class CVidPlayer;
class CVidTheoraPlayer;

#define NUM_MUSIC_CHANNELS 5

class CBGame: public CBObject {
public:
	DECLARE_PERSISTENT(CBGame, CBObject)

	virtual bool onScriptShutdown(CScScript *script);

	virtual bool onActivate(bool activate, bool refreshMouse);
	virtual bool onMouseLeftDown();
	virtual bool onMouseLeftUp();
	virtual bool onMouseLeftDblClick();
	virtual bool onMouseRightDblClick();
	virtual bool onMouseRightDown();
	virtual bool onMouseRightUp();
	virtual bool onMouseMiddleDown();
	virtual bool onMouseMiddleUp();
	virtual bool onPaint();
	virtual bool onWindowClose();

	bool isLeftDoubleClick();
	bool isRightDoubleClick();

	bool _autorunDisabled;

	uint32 _lastMiniUpdate;
	bool _miniUpdateEnabled;

	virtual bool miniUpdate();

	void getMousePos(Point32 *Pos);
	Rect32 _mouseLockRect;

	bool _shuttingDown;

	virtual bool displayDebugInfo();
	bool _debugShowFPS;

	bool _suspendedRendering;
	int _soundBufferSizeSec;

	TTextEncoding _textEncoding;
	bool _textRTL;

	CBSprite *_loadingIcon;
	int _loadingIconX;
	int _loadingIconY;
	int _loadingIconPersistent;

	virtual bool resetContent();

	void DEBUG_DumpClassRegistry();
	bool setWaitCursor(const char *filename);
	char *_localSaveDir;
	bool _saveDirChecked;

	int _indicatorProgress;
protected:
	bool _indicatorDisplay;
	uint32 _indicatorColor;
	int _indicatorX;
	int _indicatorY;
	int _indicatorWidth;
	int _indicatorHeight;

	bool _richSavedGames;
	char *_savedGameExt;

	char *_loadImageName;
	char *_saveImageName;
	int _saveImageX;
	int _saveImageY;
	int _loadImageX;
	int _loadImageY;

	CBSurface *_saveLoadImage;
	bool displayIndicator();

	bool _reportTextureFormat;
public:
	int _thumbnailWidth;
	int _thumbnailHeight;

	void setEngineLogCallback(ENGINE_LOG_CALLBACK callback = NULL, void *data = NULL);
	ENGINE_LOG_CALLBACK _engineLogCallback;
	void *_engineLogCallbackData;
	bool _editorMode;

	bool _doNotExpandStrings;
	void getOffset(int *offsetX, int *offsetY);
	void setOffset(int offsetX, int offsetY);
	int getSequence();
	int _offsetY;
	int _offsetX;
	float _offsetPercentX;
	float _offsetPercentY;
	CBObject *_mainObject;

	bool initInput();
	bool initLoop();
	uint32 _currentTime;
	uint32 _deltaTime;
	CBFont *_systemFont;
	CBFont *_videoFont;
	bool initialize1();
	bool initialize2();
	bool initialize3();
	CBFileManager *_fileManager;
	CBTransitionMgr *_transMgr;
	CBDebugger *getDebugMgr();

	void LOG(bool res, const char *fmt, ...);

	CBRenderer *_renderer;
	CBSoundMgr *_soundMgr;
	CScEngine *_scEngine;
	CSXMath *_mathClass;
	CBSurfaceStorage *_surfaceStorage;
	CBFontStorage *_fontStorage;
	CBGame();

	virtual ~CBGame();
	void DEBUG_DebugDisable();
	void DEBUG_DebugEnable(const char *filename = NULL);
	bool _debugDebugMode;
	bool _debugAbsolutePathWarning;

	void *_debugLogFile;
	int _sequence;
	virtual bool loadFile(const char *filename);
	virtual bool loadBuffer(byte *buffer, bool complete = true);
	CBArray<CBQuickMsg *, CBQuickMsg *> _quickMessages;
	CBArray<CUIWindow *, CUIWindow *> _windows;
	CBArray<CBViewport *, CBViewport *> _viewportStack;

	int _viewportSP;

	CBStringTable *_stringTable;
	int _settingsResWidth;
	int _settingsResHeight;
	char *_settingsGameFile;
	bool _suppressScriptErrors;
	bool _mouseLeftDown;
protected:
	bool _mouseRightDown;
	bool _mouseMidlleDown;
	bool _settingsRequireAcceleration;
	bool _settingsAllowWindowed;
	bool _settingsAllowAdvanced;
	bool _settingsAllowAccessTab;
	bool _settingsAllowAboutTab;
	bool _settingsRequireSound;
	bool _settingsAllowDesktopRes;
	int _settingsTLMode;
	CBFader *_fader;
	virtual bool invalidateDeviceObjects();
	virtual bool restoreDeviceObjects();
public:
	virtual bool ExternalCall(CScScript *script, CScStack *stack, CScStack *thisStack, char *name);
	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual bool scSetProperty(const char *name, CScValue *value);
	virtual bool scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();
	// compatibility bits
	bool _compatKillMethodThreads;

private:
	// FPS stuff
	uint32 _lastTime;
	uint32 _fpsTime;
	uint32 _framesRendered;
	Common::String _gameId;
public:
	const char* getGameId() { return _gameId.c_str(); }
	void setGameId(const Common::String& gameId) { _gameId = gameId; }
	uint32 _surfaceGCCycleTime;
	bool _smartCache;
	bool _videoSubtitles;
	bool _subtitles;
	uint32 _musicStartTime[NUM_MUSIC_CHANNELS];
	bool _compressedSavegames;
	int _scheduledLoadSlot;
	bool _loading;
	bool _personalizedSave;
	bool emptySaveSlot(int slot);
	bool isSaveSlotUsed(int slot);
	bool getSaveSlotDescription(int slot, char *buffer);
	bool getSaveSlotFilename(int slot, char *buffer);
	void setWindowTitle();
	virtual bool handleMouseWheel(int delta);
	bool _quitting;
	virtual bool getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor);

	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	virtual void handleKeyRelease(Common::Event *event);
protected:
	int _freezeLevel;
public:
	bool unfreeze();
	bool freeze(bool includingMusic = true);
	bool focusWindow(CUIWindow *window);
	CVidPlayer *_videoPlayer;
	CVidTheoraPlayer *_theoraPlayer;
	bool _loadInProgress;
	CUIWindow *_focusedWindow;
	bool _editorForceScripts;
protected:
	static void afterLoadRegion(void *region, void *data);
	static void afterLoadSubFrame(void *subframe, void *data);
	static void afterLoadSound(void *sound, void *data);
	static void afterLoadFont(void *font, void *data);
	static void afterLoadScript(void *script, void *data);
public:
	static void invalidateValues(void *value, void *data);

	bool loadSettings(const char *filename);
	bool resumeMusic(int channel);
	bool setMusicStartTime(int channel, uint32 time);
	bool pauseMusic(int channel);
	bool stopMusic(int channel);
	bool playMusic(int channel, const char *filename, bool looping = true, uint32 loopStart = 0);
	CBSound *_music[NUM_MUSIC_CHANNELS];
	bool _musicCrossfadeRunning;
	bool _musicCrossfadeSwap;
	uint32 _musicCrossfadeStartTime;
	uint32 _musicCrossfadeLength;
	int _musicCrossfadeChannel1;
	int _musicCrossfadeChannel2;
	bool displayWindows(bool inGame = false);
	CBRegistry *_registry;
	bool _useD3D;
	virtual bool cleanup();
	virtual bool loadGame(int slot);
	virtual bool loadGame(const char *filename);
	virtual bool SaveGame(int slot, const char *desc, bool quickSave = false);
	virtual bool showCursor();

	CBSprite *_cursorNoninteractive;
	CBObject *_activeObject;
	CBKeyboardState *_keyboardState;
	bool _interactive;
	TGameState _state;
	TGameState _origState;
	bool _origInteractive;
	uint32 _timer;
	uint32 _timerDelta;
	uint32 _timerLast;

	uint32 _liveTimer;
	uint32 _liveTimerDelta;
	uint32 _liveTimerLast;

	CBObject *_capturedObject;
	Point32 _mousePos;
	bool validObject(CBObject *object);
	bool unregisterObject(CBObject *object);
	bool registerObject(CBObject *object);
	void quickMessage(const char *text);
	void quickMessageForm(char *fmt, ...);
	bool displayQuickMsg();
	uint32 _fps;
	bool updateMusicCrossfade();

	bool isVideoPlaying();
	bool stopVideo();

	CBArray<CBObject *, CBObject *> _regObjects;
public:
	virtual bool displayContent(bool update = true, bool displayAll = false);
	virtual bool displayContentSimple();
	bool _forceNonStreamedSounds;
	void resetMousePos();
	int _subtitlesSpeed;
	void setInteractive(bool state);
	virtual bool windowLoadHook(CUIWindow *win, char **buf, char **params);
	virtual bool windowScriptMethodHook(CUIWindow *win, CScScript *script, CScStack *stack, const char *name);
	bool getCurrentViewportOffset(int *offsetX = NULL, int *offsetY = NULL);
	bool getCurrentViewportRect(Rect32 *rect, bool *custom = NULL);
	bool popViewport();
	bool pushViewport(CBViewport *Viewport);
	bool setActiveObject(CBObject *Obj);
	CBSprite *_lastCursor;
	bool drawCursor(CBSprite *Cursor);

	virtual bool initAfterLoad();
	CBSaveThumbHelper *_cachedThumbnail;
	AnsiString getDataDir();
	void addMem(int bytes);

	bool _touchInterface;
	bool _constrainedMemory;
	AnsiString getDeviceType() const;

private:
	CBDebugger *_debugMgr;

	struct LastClickInfo {
		LastClickInfo() {
			posX = posY = 0;
			time = 0;
		}

		int posX;
		int posY;
		uint32 time;
	};

	LastClickInfo _lastClick[2];
	bool isDoubleClick(int buttonIndex);
	uint32 _usedMem;



protected:
	// WME Lite specific
	bool _autoSaveOnExit;
	int _autoSaveSlot;
	bool _cursorHidden;

public:
	void autoSaveOnExit();

};

} // end of namespace WinterMute

#endif
