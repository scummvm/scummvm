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

	virtual HRESULT OnScriptShutdown(CScScript *script);

	virtual HRESULT OnActivate(bool Activate, bool RefreshMouse);
	virtual HRESULT OnMouseLeftDown();
	virtual HRESULT OnMouseLeftUp();
	virtual HRESULT OnMouseLeftDblClick();
	virtual HRESULT OnMouseRightDblClick();
	virtual HRESULT OnMouseRightDown();
	virtual HRESULT OnMouseRightUp();
	virtual HRESULT OnMouseMiddleDown();
	virtual HRESULT OnMouseMiddleUp();
	virtual HRESULT OnPaint();
	virtual HRESULT OnWindowClose();

	bool IsLeftDoubleClick();
	bool IsRightDoubleClick();

	bool _autorunDisabled;

	uint32 _lastMiniUpdate;
	bool _miniUpdateEnabled;

	virtual HRESULT MiniUpdate();

	void GetMousePos(POINT *Pos);
	RECT _mouseLockRect;

	bool _shuttingDown;

	virtual HRESULT DisplayDebugInfo();
	bool _dEBUG_ShowFPS;

	bool _suspendedRendering;
	int _soundBufferSizeSec;

	TTextEncoding _textEncoding;
	bool _textRTL;

	CBSprite *_loadingIcon;
	int _loadingIconX;
	int _loadingIconY;
	int _loadingIconPersistent;

	virtual HRESULT ResetContent();

	void DEBUG_DumpClassRegistry();
	HRESULT SetWaitCursor(const char *Filename);
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

	HRESULT DisplayIndicator();

	int _thumbnailWidth;
	int _thumbnailHeight;

	bool _reportTextureFormat;
	HMODULE _resourceModule;
	void SetResourceModule(HMODULE ResModule);

	void SetEngineLogCallback(ENGINE_LOG_CALLBACK Callback = NULL, void *Data = NULL);
	ENGINE_LOG_CALLBACK _engineLogCallback;
	void *_engineLogCallbackData;
	bool _editorMode;

	bool _doNotExpandStrings;
	void GetOffset(int *OffsetX, int *OffsetY);
	void SetOffset(int OffsetX, int OffsetY);
	int GetSequence();
	int _offsetY;
	int _offsetX;
	float _offsetPercentX;
	float _offsetPercentY;
	CBObject *_mainObject;

	HRESULT InitInput(HINSTANCE hInst, HWND hWnd);
	HRESULT InitLoop();
	uint32 _currentTime;
	uint32 _deltaTime;
	CBFont *_systemFont;
	CBFont *_videoFont;
	HRESULT initialize1();
	HRESULT initialize2();
	HRESULT initialize3();
	CBFileManager *_fileManager;
	CBTransitionMgr *_transMgr;
	CBDebugger *GetDebugMgr();

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
	void DEBUG_DebugEnable(const char *Filename = NULL);
	bool _dEBUG_DebugMode;
	bool _dEBUG_AbsolutePathWarning;

	void *_dEBUG_LogFile;
	int _sequence;
	virtual HRESULT loadFile(const char *Filename);
	virtual HRESULT loadBuffer(byte  *Buffer, bool Complete = true);
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
	virtual HRESULT scSetProperty(const char *name, CScValue *Value);
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
	HRESULT EmptySaveSlot(int Slot);
	bool IsSaveSlotUsed(int Slot);
	HRESULT GetSaveSlotDescription(int Slot, char *Buffer);
	HRESULT GetSaveSlotFilename(int Slot, char *Buffer);
	void SetWindowTitle();
	virtual bool handleMouseWheel(int Delta);
	bool _quitting;
	virtual HRESULT GetVersion(byte  *VerMajor, byte *VerMinor, byte *ExtMajor, byte *ExtMinor);

	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	virtual void handleKeyRelease(Common::Event *event);
	int _freezeLevel;
	HRESULT Unfreeze();
	HRESULT Freeze(bool IncludingMusic = true);
	HRESULT FocusWindow(CUIWindow *Window);
	CVidPlayer *_videoPlayer;
	CVidTheoraPlayer *_theoraPlayer;
	bool _loadInProgress;
	CUIWindow *_focusedWindow;
	bool _editorForceScripts;
	static void AfterLoadRegion(void *Region, void *Data);
	static void AfterLoadSubFrame(void *Subframe, void *Data);
	static void AfterLoadSound(void *Sound, void *Data);
	static void AfterLoadFont(void *Font, void *Data);
	static void AfterLoadScript(void *script, void *data);
	static void InvalidateValues(void *Value, void *Data);

	HRESULT LoadSettings(const char *Filename);
	HRESULT ResumeMusic(int Channel);
	HRESULT SetMusicStartTime(int Channel, uint32 Time);
	HRESULT PauseMusic(int Channel);
	HRESULT StopMusic(int Channel);
	HRESULT PlayMusic(int Channel, const char *Filename, bool Looping = true, uint32 LoopStart = 0);
	CBSound *_music[NUM_MUSIC_CHANNELS];
	bool _musicCrossfadeRunning;
	bool _musicCrossfadeSwap;
	uint32 _musicCrossfadeStartTime;
	uint32 _musicCrossfadeLength;
	int _musicCrossfadeChannel1;
	int _musicCrossfadeChannel2;
	HRESULT DisplayWindows(bool InGame = false);
	CBRegistry *_registry;
	bool _useD3D;
	virtual HRESULT cleanup();
	virtual HRESULT LoadGame(int Slot);
	virtual HRESULT LoadGame(const char *Filename);
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
	bool ValidObject(CBObject *Object);
	HRESULT UnregisterObject(CBObject *Object);
	HRESULT RegisterObject(CBObject *Object);
	void QuickMessage(const char *Text);
	void QuickMessageForm(LPSTR fmt, ...);
	HRESULT DisplayQuickMsg();
	uint32 _fps;
	HRESULT UpdateMusicCrossfade();

	bool IsVideoPlaying();
	HRESULT StopVideo();

	CBArray<CBObject *, CBObject *> _regObjects;
public:
	virtual HRESULT DisplayContent(bool Update = true, bool DisplayAll = false);
	virtual HRESULT DisplayContentSimple();
	bool _forceNonStreamedSounds;
	void ResetMousePos();
	int _subtitlesSpeed;
	void SetInteractive(bool State);
	virtual HRESULT WindowLoadHook(CUIWindow *Win, char **Buf, char **Params);
	virtual HRESULT WindowScriptMethodHook(CUIWindow *Win, CScScript *script, CScStack *stack, const char *name);
	HRESULT GetCurrentViewportOffset(int *OffsetX = NULL, int *OffsetY = NULL);
	HRESULT GetCurrentViewportRect(RECT *Rect, bool *Custom = NULL);
	HRESULT PopViewport();
	HRESULT PushViewport(CBViewport *Viewport);
	HRESULT SetActiveObject(CBObject *Obj);
	CBSprite *_lastCursor;
	HRESULT DrawCursor(CBSprite *Cursor);

	virtual HRESULT InitAfterLoad();
	CBSaveThumbHelper *_cachedThumbnail;
	AnsiString GetDataDir();
	void AddMem(int bytes);

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
	bool IsDoubleClick(int buttonIndex);
	uint32 _usedMem;



protected:
	// WME Lite specific
	bool _autoSaveOnExit;
	int _autoSaveSlot;
	bool _cursorHidden;

public:
	void AutoSaveOnExit();

};

} // end of namespace WinterMute

#endif
