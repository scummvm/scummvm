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

#include "engines/wintermute/BDebugger.h"
//#include "BSaveThumbHelper.h"
//#include "BFader.h"
#include "engines/wintermute/BRenderer.h"
//#include "BSurfaceStorage.h"
#include "engines/wintermute/BObject.h"
#include "engines/wintermute/persistent.h"
#include "coll_templ.h"

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
class CBSurfaceStorage;
class CSXStore;
class CSXMath;
class CBKeyboardState;

#define NUM_MUSIC_CHANNELS 5

class CBGame: public CBObject {
public:
	DECLARE_PERSISTENT(CBGame, CBObject)

	virtual HRESULT OnScriptShutdown(CScScript *Script);

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

	bool m_AutorunDisabled;

	uint32 m_LastMiniUpdate;
	bool m_MiniUpdateEnabled;
	
	virtual HRESULT MiniUpdate() {} // <- TODO Unstub
#if 0
	virtual HRESULT MiniUpdate();

	void GetMousePos(POINT *Pos);
	RECT m_MouseLockRect;

	bool m_ShuttingDown;

	virtual HRESULT DisplayDebugInfo();
	bool m_DEBUG_ShowFPS;

	bool m_SuspendedRendering;
	int m_SoundBufferSizeSec;

	TTextEncoding m_TextEncoding;
	bool m_TextRTL;

	CBSprite *m_LoadingIcon;
	int m_LoadingIconX;
	int m_LoadingIconY;
	int m_LoadingIconPersistent;

	virtual HRESULT ResetContent();

	void DEBUG_DumpClassRegistry();
	HRESULT SetWaitCursor(char *Filename);

	char *m_LocalSaveDir;
	bool m_SaveDirChecked;


	bool m_IndicatorDisplay;
	uint32 m_IndicatorColor;
	int m_IndicatorProgress;
	int m_IndicatorX;
	int m_IndicatorY;
	int m_IndicatorWidth;
	int m_IndicatorHeight;

	bool m_RichSavedGames;
	char *m_SavedGameExt;

	char *m_LoadImageName;
	char *m_SaveImageName;
	int m_SaveImageX;
	int m_SaveImageY;
	int m_LoadImageX;
	int m_LoadImageY;
	CBSurface *m_SaveLoadImage;

	HRESULT DisplayIndicator();

	int m_ThumbnailWidth;
	int m_ThumbnailHeight;

	bool m_ReportTextureFormat;
	HMODULE m_ResourceModule;
	void SetResourceModule(HMODULE ResModule);

	void SetEngineLogCallback(ENGINE_LOG_CALLBACK Callback = NULL, void *Data = NULL);
	ENGINE_LOG_CALLBACK m_EngineLogCallback;
	void *m_EngineLogCallbackData;
#endif
	bool m_EditorMode;
#if 0
	bool m_DoNotExpandStrings;
	void GetOffset(int *OffsetX, int *OffsetY);
	void SetOffset(int OffsetX, int OffsetY);
	int GetSequence();
	int m_OffsetY;
	int m_OffsetX;
	float m_OffsetPercentX;
	float m_OffsetPercentY;
	CBObject *m_MainObject;
	HRESULT InitInput(HINSTANCE hInst, HWND hWnd);
	HRESULT InitLoop();
	uint32 m_CurrentTime;
	uint32 m_DeltaTime;
	CBFont *m_SystemFont;
	CBFont *m_VideoFont;
	HRESULT Initialize1();
	HRESULT Initialize2();
	HRESULT Initialize3();
	CBFileManager *m_FileManager;
	CBTransitionMgr *m_TransMgr;
#endif //TODO: STUB
	CBDebugger *GetDebugMgr();
	void LOG(HRESULT res, LPCSTR fmt, ...) {}

	CBRenderer *m_Renderer;
	CBSoundMgr *m_SoundMgr;
	CScEngine *m_ScEngine;
	CSXMath *m_MathClass;
	CSXStore *m_Store;
	CBSurfaceStorage *m_SurfaceStorage;
	CBFontStorage *m_FontStorage;
	CBGame();
#if 0
	virtual ~CBGame();
	void DEBUG_DebugDisable();
	void DEBUG_DebugEnable(const char *Filename = NULL);
	bool m_DEBUG_DebugMode;
	bool m_DEBUG_AbsolutePathWarning;
	FILE *m_DEBUG_LogFile;
	int m_Sequence;
	virtual HRESULT LoadFile(const char *Filename);
	virtual HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);
	CBArray<CBQuickMsg *, CBQuickMsg *> m_QuickMessages;
	CBArray<CUIWindow *, CUIWindow *> m_Windows;
	CBArray<CBViewport *, CBViewport *> m_ViewportStack;
#endif
	int m_ViewportSP;
	bool m_MouseLeftDown;
	bool m_MouseRightDown;
	bool m_MouseMidlleDown;
	CBStringTable *m_StringTable;

	int m_SettingsResWidth;
	int m_SettingsResHeight;
	bool m_SettingsRequireAcceleration;
	bool m_SettingsAllowWindowed;
	bool m_SettingsAllowAdvanced;
	bool m_SettingsAllowAccessTab;
	bool m_SettingsAllowAboutTab;
	bool m_SettingsRequireSound;
	bool m_SettingsAllowDesktopRes;
	int m_SettingsTLMode;
	char *m_SettingsGameFile;
	CBFader *m_Fader;
	bool m_SuppressScriptErrors;

	virtual HRESULT InvalidateDeviceObjects();
	virtual HRESULT RestoreDeviceObjects();

	virtual void PublishNatives();
	virtual HRESULT ExternalCall(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	// scripting interface
	virtual CScValue *ScGetProperty(char *Name);
	virtual HRESULT ScSetProperty(char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	virtual char *ScToString();
	// compatibility bits
	bool m_CompatKillMethodThreads;

private:
	// FPS stuff
	uint32 m_LastTime;
	uint32 m_FpsTime;
	uint32 m_FramesRendered;

public:
	uint32 m_SurfaceGCCycleTime;
	bool m_SmartCache;
	bool m_VideoSubtitles;
	bool m_Subtitles;
	uint32 m_MusicStartTime[NUM_MUSIC_CHANNELS];
	bool m_CompressedSavegames;
	int m_ScheduledLoadSlot;
	bool m_Loading;
	bool m_PersonalizedSave;
#if 0
	HRESULT EmptySaveSlot(int Slot);
	bool IsSaveSlotUsed(int Slot);
	HRESULT GetSaveSlotDescription(int Slot, char *Buffer);
	HRESULT GetSaveSlotFilename(int Slot, char *Buffer);
	void SetWindowTitle();
	virtual bool HandleMouseWheel(int Delta);
	bool m_Quitting;
	virtual HRESULT GetVersion(byte  *VerMajor, byte *VerMinor, byte *ExtMajor, byte *ExtMinor);
	virtual bool HandleKeypress(SDL_Event *event);
	int m_FreezeLevel;
	HRESULT Unfreeze();
	HRESULT Freeze(bool IncludingMusic = true);
	HRESULT FocusWindow(CUIWindow *Window);
	bool m_LoadInProgress;
	CUIWindow *m_FocusedWindow;
	bool m_EditorForceScripts;
	static void AfterLoadRegion(void *Region, void *Data);
	static void AfterLoadSubFrame(void *Subframe, void *Data);
	static void AfterLoadSound(void *Sound, void *Data);
	static void AfterLoadFont(void *Font, void *Data);
	static void AfterLoadScript(void *script, void *data);
	static void InvalidateValues(void *Value, void *Data);
	HRESULT LoadSettings(char *Filename);
	HRESULT ResumeMusic(int Channel);
	HRESULT SetMusicStartTime(int Channel, uint32 Time);
	HRESULT PauseMusic(int Channel);
	HRESULT StopMusic(int Channel);
	HRESULT PlayMusic(int Channel, char *Filename, bool Looping = true, uint32 LoopStart = 0);
	CBSound *m_Music[NUM_MUSIC_CHANNELS];
	bool m_MusicCrossfadeRunning;
	bool m_MusicCrossfadeSwap;
	uint32 m_MusicCrossfadeStartTime;
	uint32 m_MusicCrossfadeLength;
	int m_MusicCrossfadeChannel1;
	int m_MusicCrossfadeChannel2;
	HRESULT DisplayWindows(bool InGame = false);
	CBRegistry *m_Registry;
	bool m_UseD3D;
	virtual HRESULT Cleanup();
	virtual HRESULT LoadGame(int Slot);
	virtual HRESULT LoadGame(char *Filename);
	virtual HRESULT SaveGame(int slot, char *desc, bool quickSave = false);
	virtual HRESULT ShowCursor();
#endif
	CBSprite *m_CursorNoninteractive;
	CBObject *m_ActiveObject;
	CBKeyboardState *m_KeyboardState;
	bool m_Interactive;
	TGameState m_State;
	TGameState m_OrigState;
	bool m_OrigInteractive;
	uint32 m_Timer;
	uint32 m_TimerDelta;
	uint32 m_TimerLast;

	uint32 m_LiveTimer;
	uint32 m_LiveTimerDelta;
	uint32 m_LiveTimerLast;
#if 0
	CBObject *m_CapturedObject;
	POINT m_MousePos;
	bool ValidObject(CBObject *Object);
	HRESULT UnregisterObject(CBObject *Object);
	HRESULT RegisterObject(CBObject *Object);
#endif
	void QuickMessage(char *Text);
#if 0
	void QuickMessageForm(LPSTR fmt, ...);
	HRESULT DisplayQuickMsg();
	uint32 m_Fps;
	HRESULT UpdateMusicCrossfade();

	CBArray<CBObject *, CBObject *> m_RegObjects;

public:
	virtual HRESULT DisplayContent(bool Update = true, bool DisplayAll = false);
	virtual HRESULT DisplayContentSimple();
	bool m_ForceNonStreamedSounds;
	void ResetMousePos();
	int m_SubtitlesSpeed;
	void SetInteractive(bool State);
	virtual HRESULT WindowLoadHook(CUIWindow *Win, char **Buf, char **Params);
	virtual HRESULT WindowScriptMethodHook(CUIWindow *Win, CScScript *Script, CScStack *Stack, char *Name);
	HRESULT GetCurrentViewportOffset(int *OffsetX = NULL, int *OffsetY = NULL);
	HRESULT GetCurrentViewportRect(RECT *Rect, bool *Custom = NULL);
	HRESULT PopViewport();
	HRESULT PushViewport(CBViewport *Viewport);
	HRESULT SetActiveObject(CBObject *Obj);

	CBSprite *m_LastCursor;
	HRESULT DrawCursor(CBSprite *Cursor);

	virtual HRESULT InitAfterLoad();

	CBSaveThumbHelper *m_CachedThumbnail;

	AnsiString GetDataDir();

	void AddMem(int bytes);

	bool m_TouchInterface;
	bool m_ConstrainedMemory;
	AnsiString GetDeviceType() const;

private:
	CBDebugger *m_DebugMgr;

	struct LastClickInfo {
		LastClickInfo() {
			PosX = PosY = 0;
			Time = 0;
		}

		int PosX;
		int PosY;
		uint32 Time;
	};

	LastClickInfo m_LastClick[2];
	bool IsDoubleClick(int buttonIndex);
	uint32 m_UsedMem;



protected:
	// WME Lite specific
	bool m_AutoSaveOnExit;
	int m_AutoSaveSlot;
	bool m_CursorHidden;

public:
	void AutoSaveOnExit();
#endif

};

} // end of namespace WinterMute

#endif
