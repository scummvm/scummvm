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

typedef void (*ENGINE_LOG_CALLBACK)(char *Text, ERRORCODE Result, void *Data);

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

	virtual ERRORCODE onScriptShutdown(CScScript *script);

	virtual ERRORCODE onActivate(bool activate, bool refreshMouse);
	virtual ERRORCODE onMouseLeftDown();
	virtual ERRORCODE onMouseLeftUp();
	virtual ERRORCODE onMouseLeftDblClick();
	virtual ERRORCODE onMouseRightDblClick();
	virtual ERRORCODE onMouseRightDown();
	virtual ERRORCODE onMouseRightUp();
	virtual ERRORCODE onMouseMiddleDown();
	virtual ERRORCODE onMouseMiddleUp();
	virtual ERRORCODE onPaint();
	virtual ERRORCODE onWindowClose();

	bool isLeftDoubleClick();
	bool isRightDoubleClick();

	bool _autorunDisabled;

	uint32 _lastMiniUpdate;
	bool _miniUpdateEnabled;

	virtual ERRORCODE miniUpdate();

	void getMousePos(Common::Point *Pos);
	Common::Rect _mouseLockRect;

	bool _shuttingDown;

	virtual ERRORCODE displayDebugInfo();
	bool _dEBUG_ShowFPS;

	bool _suspendedRendering;
	int _soundBufferSizeSec;

	TTextEncoding _textEncoding;
	bool _textRTL;

	CBSprite *_loadingIcon;
	int _loadingIconX;
	int _loadingIconY;
	int _loadingIconPersistent;

	virtual ERRORCODE resetContent();

	void DEBUG_DumpClassRegistry();
	ERRORCODE setWaitCursor(const char *filename);
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

	ERRORCODE displayIndicator();

	int _thumbnailWidth;
	int _thumbnailHeight;

	bool _reportTextureFormat;

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

	ERRORCODE initInput();
	ERRORCODE initLoop();
	uint32 _currentTime;
	uint32 _deltaTime;
	CBFont *_systemFont;
	CBFont *_videoFont;
	ERRORCODE initialize1();
	ERRORCODE initialize2();
	ERRORCODE initialize3();
	CBFileManager *_fileManager;
	CBTransitionMgr *_transMgr;
	CBDebugger *getDebugMgr();

	void LOG(ERRORCODE res, const char *fmt, ...);

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
	virtual ERRORCODE loadFile(const char *filename);
	virtual ERRORCODE loadBuffer(byte *buffer, bool complete = true);
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

	virtual ERRORCODE invalidateDeviceObjects();
	virtual ERRORCODE restoreDeviceObjects();

	virtual ERRORCODE ExternalCall(CScScript *script, CScStack *stack, CScStack *thisStack, char *name);
	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual ERRORCODE scSetProperty(const char *name, CScValue *value);
	virtual ERRORCODE scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
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
	ERRORCODE emptySaveSlot(int slot);
	bool isSaveSlotUsed(int slot);
	ERRORCODE getSaveSlotDescription(int slot, char *buffer);
	ERRORCODE getSaveSlotFilename(int slot, char *buffer);
	void setWindowTitle();
	virtual bool handleMouseWheel(int delta);
	bool _quitting;
	virtual ERRORCODE getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor);

	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	virtual void handleKeyRelease(Common::Event *event);
	int _freezeLevel;
	ERRORCODE unfreeze();
	ERRORCODE freeze(bool includingMusic = true);
	ERRORCODE focusWindow(CUIWindow *window);
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

	ERRORCODE loadSettings(const char *filename);
	ERRORCODE resumeMusic(int channel);
	ERRORCODE setMusicStartTime(int channel, uint32 time);
	ERRORCODE pauseMusic(int channel);
	ERRORCODE stopMusic(int channel);
	ERRORCODE playMusic(int channel, const char *filename, bool looping = true, uint32 loopStart = 0);
	CBSound *_music[NUM_MUSIC_CHANNELS];
	bool _musicCrossfadeRunning;
	bool _musicCrossfadeSwap;
	uint32 _musicCrossfadeStartTime;
	uint32 _musicCrossfadeLength;
	int _musicCrossfadeChannel1;
	int _musicCrossfadeChannel2;
	ERRORCODE displayWindows(bool inGame = false);
	CBRegistry *_registry;
	bool _useD3D;
	virtual ERRORCODE cleanup();
	virtual ERRORCODE loadGame(int slot);
	virtual ERRORCODE loadGame(const char *filename);
	virtual ERRORCODE SaveGame(int slot, const char *desc, bool quickSave = false);
	virtual ERRORCODE showCursor();

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
	Common::Point _mousePos;
	bool validObject(CBObject *object);
	ERRORCODE unregisterObject(CBObject *object);
	ERRORCODE registerObject(CBObject *object);
	void quickMessage(const char *text);
	void quickMessageForm(char *fmt, ...);
	ERRORCODE displayQuickMsg();
	uint32 _fps;
	ERRORCODE updateMusicCrossfade();

	bool isVideoPlaying();
	ERRORCODE stopVideo();

	CBArray<CBObject *, CBObject *> _regObjects;
public:
	virtual ERRORCODE displayContent(bool update = true, bool displayAll = false);
	virtual ERRORCODE displayContentSimple();
	bool _forceNonStreamedSounds;
	void resetMousePos();
	int _subtitlesSpeed;
	void setInteractive(bool state);
	virtual ERRORCODE windowLoadHook(CUIWindow *win, char **buf, char **params);
	virtual ERRORCODE windowScriptMethodHook(CUIWindow *win, CScScript *script, CScStack *stack, const char *name);
	ERRORCODE getCurrentViewportOffset(int *offsetX = NULL, int *offsetY = NULL);
	ERRORCODE getCurrentViewportRect(Common::Rect *rect, bool *custom = NULL);
	ERRORCODE popViewport();
	ERRORCODE pushViewport(CBViewport *Viewport);
	ERRORCODE setActiveObject(CBObject *Obj);
	CBSprite *_lastCursor;
	ERRORCODE drawCursor(CBSprite *Cursor);

	virtual ERRORCODE initAfterLoad();
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
