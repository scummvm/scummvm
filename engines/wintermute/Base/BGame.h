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

#include "engines/wintermute/Base/BDebugger.h"
#include "engines/wintermute/Base/BRenderer.h"
#include "engines/wintermute/Base/BObject.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/coll_templ.h"
#include "common/events.h"

namespace WinterMute {

typedef void (*ENGINE_LOG_CALLBACK)(char *Text, HRESULT Result, void *Data);

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
class CSXStore;
class CSXMath;
class CBKeyboardState;
class CVidPlayer;
class CVidTheoraPlayer;

#define NUM_MUSIC_CHANNELS 5

class CBGame: public CBObject {
public:
	DECLARE_PERSISTENT(CBGame, CBObject)

	virtual HRESULT onScriptShutdown(CScScript *script);

	virtual HRESULT onActivate(bool activate, bool refreshMouse);
	virtual HRESULT onMouseLeftDown();
	virtual HRESULT onMouseLeftUp();
	virtual HRESULT onMouseLeftDblClick();
	virtual HRESULT onMouseRightDblClick();
	virtual HRESULT onMouseRightDown();
	virtual HRESULT onMouseRightUp();
	virtual HRESULT onMouseMiddleDown();
	virtual HRESULT onMouseMiddleUp();
	virtual HRESULT onPaint();
	virtual HRESULT onWindowClose();

	bool isLeftDoubleClick();
	bool isRightDoubleClick();

	bool _autorunDisabled;

	uint32 _lastMiniUpdate;
	bool _miniUpdateEnabled;

	virtual HRESULT MiniUpdate();

	void getMousePos(POINT *Pos);
	RECT _mouseLockRect;

	bool _shuttingDown;

	virtual HRESULT displayDebugInfo();
	bool _dEBUG_ShowFPS;

	bool _suspendedRendering;
	int _soundBufferSizeSec;

	TTextEncoding _textEncoding;
	bool _textRTL;

	CBSprite *_loadingIcon;
	int _loadingIconX;
	int _loadingIconY;
	int _loadingIconPersistent;

	virtual HRESULT resetContent();

	void DEBUG_DumpClassRegistry();
	HRESULT setWaitCursor(const char *filename);
	char *_localSaveDir;
	bool _saveDirChecked;


	bool _indicatorDisplay;
	uint32 _indicatorColor;
	int _indicatorProgress;
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

	HRESULT displayIndicator();

	int _thumbnailWidth;
	int _thumbnailHeight;

	bool _reportTextureFormat;
	HMODULE _resourceModule;
	void setResourceModule(HMODULE ResModule);

	void setEngineLogCallback(ENGINE_LOG_CALLBACK Callback = NULL, void *Data = NULL);
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

	HRESULT initInput(HINSTANCE hInst, HWND hWnd);
	HRESULT initLoop();
	uint32 _currentTime;
	uint32 _deltaTime;
	CBFont *_systemFont;
	CBFont *_videoFont;
	HRESULT initialize1();
	HRESULT initialize2();
	HRESULT initialize3();
	CBFileManager *_fileManager;
	CBTransitionMgr *_transMgr;
	CBDebugger *getDebugMgr();

	void LOG(HRESULT res, LPCSTR fmt, ...);

	CBRenderer *_renderer;
	CBSoundMgr *_soundMgr;
	CScEngine *_scEngine;
	CSXMath *_mathClass;
	CSXStore *_store;
	CBSurfaceStorage *_surfaceStorage;
	CBFontStorage *_fontStorage;
	CBGame();

	virtual ~CBGame();
	void DEBUG_DebugDisable();
	void DEBUG_DebugEnable(const char *filename = NULL);
	bool _dEBUG_DebugMode;
	bool _dEBUG_AbsolutePathWarning;

	void *_dEBUG_LogFile;
	int _sequence;
	virtual HRESULT loadFile(const char *filename);
	virtual HRESULT loadBuffer(byte *buffer, bool complete = true);
	CBArray<CBQuickMsg *, CBQuickMsg *> _quickMessages;
	CBArray<CUIWindow *, CUIWindow *> _windows;
	CBArray<CBViewport *, CBViewport *> _viewportStack;

	int _viewportSP;
	bool _mouseLeftDown;
	bool _mouseRightDown;
	bool _mouseMidlleDown;
	CBStringTable *_stringTable;

	int _settingsResWidth;
	int _settingsResHeight;
	bool _settingsRequireAcceleration;
	bool _settingsAllowWindowed;
	bool _settingsAllowAdvanced;
	bool _settingsAllowAccessTab;
	bool _settingsAllowAboutTab;
	bool _settingsRequireSound;
	bool _settingsAllowDesktopRes;
	int _settingsTLMode;
	char *_settingsGameFile;
	CBFader *_fader;
	bool _suppressScriptErrors;

	virtual HRESULT invalidateDeviceObjects();
	virtual HRESULT restoreDeviceObjects();

	virtual void PublishNatives();
	virtual HRESULT ExternalCall(CScScript *script, CScStack *stack, CScStack *thisStack, char *name);
	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual HRESULT scSetProperty(const char *name, CScValue *value);
	virtual HRESULT scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();
	// compatibility bits
	bool _compatKillMethodThreads;

private:
	// FPS stuff
	uint32 _lastTime;
	uint32 _fpsTime;
	uint32 _framesRendered;

public:
	uint32 _surfaceGCCycleTime;
	bool _smartCache;
	bool _videoSubtitles;
	bool _subtitles;
	uint32 _musicStartTime[NUM_MUSIC_CHANNELS];
	bool _compressedSavegames;
	int _scheduledLoadSlot;
	bool _loading;
	bool _personalizedSave;
	HRESULT emptySaveSlot(int slot);
	bool isSaveSlotUsed(int slot);
	HRESULT getSaveSlotDescription(int slot, char *buffer);
	HRESULT getSaveSlotFilename(int slot, char *buffer);
	void setWindowTitle();
	virtual bool handleMouseWheel(int Delta);
	bool _quitting;
	virtual HRESULT getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor);

	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	virtual void handleKeyRelease(Common::Event *event);
	int _freezeLevel;
	HRESULT unfreeze();
	HRESULT freeze(bool includingMusic = true);
	HRESULT focusWindow(CUIWindow *window);
	CVidPlayer *_videoPlayer;
	CVidTheoraPlayer *_theoraPlayer;
	bool _loadInProgress;
	CUIWindow *_focusedWindow;
	bool _editorForceScripts;
	static void afterLoadRegion(void *region, void *data);
	static void afterLoadSubFrame(void *subframe, void *data);
	static void afterLoadSound(void *sound, void *data);
	static void afterLoadFont(void *font, void *data);
	static void afterLoadScript(void *script, void *data);
	static void invalidateValues(void *value, void *data);

	HRESULT loadSettings(const char *filename);
	HRESULT resumeMusic(int channel);
	HRESULT setMusicStartTime(int channel, uint32 time);
	HRESULT pauseMusic(int channel);
	HRESULT stopMusic(int channel);
	HRESULT playMusic(int channel, const char *filename, bool looping = true, uint32 loopStart = 0);
	CBSound *_music[NUM_MUSIC_CHANNELS];
	bool _musicCrossfadeRunning;
	bool _musicCrossfadeSwap;
	uint32 _musicCrossfadeStartTime;
	uint32 _musicCrossfadeLength;
	int _musicCrossfadeChannel1;
	int _musicCrossfadeChannel2;
	HRESULT displayWindows(bool inGame = false);
	CBRegistry *_registry;
	bool _useD3D;
	virtual HRESULT cleanup();
	virtual HRESULT loadGame(int slot);
	virtual HRESULT loadGame(const char *filename);
	virtual HRESULT SaveGame(int slot, const char *desc, bool quickSave = false);
	virtual HRESULT showCursor();

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
	POINT _mousePos;
	bool validObject(CBObject *object);
	HRESULT unregisterObject(CBObject *object);
	HRESULT registerObject(CBObject *object);
	void quickMessage(const char *text);
	void quickMessageForm(LPSTR fmt, ...);
	HRESULT displayQuickMsg();
	uint32 _fps;
	HRESULT updateMusicCrossfade();

	bool isVideoPlaying();
	HRESULT stopVideo();

	CBArray<CBObject *, CBObject *> _regObjects;
public:
	virtual HRESULT displayContent(bool update = true, bool displayAll = false);
	virtual HRESULT displayContentSimple();
	bool _forceNonStreamedSounds;
	void resetMousePos();
	int _subtitlesSpeed;
	void SetInteractive(bool State);
	virtual HRESULT windowLoadHook(CUIWindow *win, char **buf, char **params);
	virtual HRESULT windowScriptMethodHook(CUIWindow *Win, CScScript *script, CScStack *stack, const char *name);
	HRESULT getCurrentViewportOffset(int *offsetX = NULL, int *offsetY = NULL);
	HRESULT getCurrentViewportRect(RECT *rect, bool *custom = NULL);
	HRESULT popViewport();
	HRESULT pushViewport(CBViewport *Viewport);
	HRESULT setActiveObject(CBObject *Obj);
	CBSprite *_lastCursor;
	HRESULT drawCursor(CBSprite *Cursor);

	virtual HRESULT initAfterLoad();
	CBSaveThumbHelper *_cachedThumbnail;
	AnsiString getDataDir();
	void addMem(int bytes);

	bool _touchInterface;
	bool _constrainedMemory;
	AnsiString GetDeviceType() const;

private:
	CBDebugger *_debugMgr;

	struct LastClickInfo {
		LastClickInfo() {
			PosX = PosY = 0;
			Time = 0;
		}

		int PosX;
		int PosY;
		uint32 Time;
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
