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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_GAME_H
#define WINTERMUTE_BASE_GAME_H

#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/base_game_custom_actions.h"
#include "engines/wintermute/base/base_string_table.h"
#include "engines/wintermute/ext/plugin_event.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/debugger.h"
#include "common/events.h"
#include "common/random.h"
#if EXTENDED_DEBUGGER_ENABLED
#include "engines/wintermute/base/scriptables/debuggable/debuggable_script_engine.h"
#endif

namespace Wintermute {

typedef void (*ENGINE_LOG_CALLBACK)(char *text, bool result, void *data);

class BaseSoundMgr;
class BaseFader;
class BaseFont;
class BaseFileManager;
class BaseTransitionMgr;
class BaseFontStorage;
class BaseGameMusic;
class BaseQuickMsg;
class BaseViewport;
class BaseRenderer;
class BaseRegistry;
class BaseSurfaceStorage;
class BaseKeyboardState;
class BaseGameSettings;
class ScEngine;
class SXMath;
class SXDirectory;
class UIWindow;
class VideoPlayer;
class VideoTheoraPlayer;
class SaveThumbHelper;

#ifdef ENABLE_WME3D
class BaseRenderer3D;
struct FogParameters;
#endif

#define NUM_MUSIC_CHANNELS 5

class BaseGame: public BaseObject {
public:


	DECLARE_PERSISTENT(BaseGame, BaseObject)

	virtual bool onScriptShutdown(ScScript *script);

	virtual bool getLayerSize(int *LayerWidth, int *LayerHeight, Common::Rect32 *viewport, bool *customViewport);
#ifdef ENABLE_WME3D
	virtual uint32 getAmbientLightColor();
	virtual bool getFogParams(bool *fogEnabled, uint32 *fogColor, float *start, float *end);
#endif
	//virtual CBObject *GetNextAccessObject(CBObject *CurrObject);
	//virtual CBObject *GetPrevAccessObject(CBObject *CurrObject);

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

	bool _autorunDisabled;
	uint32 _lastMiniUpdate;
	bool _miniUpdateEnabled;
	virtual bool miniUpdate();

	void getMousePos(Common::Point32 *pos);
	Common::Rect32 _mouseLockRect;

	bool _shuttingDown;

	virtual bool displayDebugInfo();
	bool _debugShowFPS;

	bool _suspendedRendering;
	int32 _soundBufferSizeSec;
	virtual bool renderShadowGeometry();

	TTextEncoding _textEncoding;
	bool _textRTL;

	BaseSprite *_loadingIcon;
	int32 _loadingIconX;
	int32 _loadingIconY;
	int32 _loadingIconPersistent;

	virtual bool resetContent();

	void DEBUG_DumpClassRegistry();
	bool setWaitCursor(const char *filename);

	char *_localSaveDir;
	bool _saveDirChecked;

#ifdef ENABLE_WME3D
	bool _supportsRealTimeShadows;
	TShadowType _maxShadowType;
	bool setMaxShadowType(TShadowType maxShadowType);
	virtual TShadowType getMaxShadowType(BaseObject *object = nullptr);
#endif

	bool _indicatorDisplay;
	uint32 _indicatorColor;
	int32 _indicatorProgress;
	int32 _indicatorX;
	int32 _indicatorY;
	int32 _indicatorWidth;
	int32 _indicatorHeight;

	char *_savedGameExt;
	bool _richSavedGames;

#ifdef ENABLE_WME3D
	int32 _editorResolutionWidth;
	int32 _editorResolutionHeight;
#endif

	char *_loadImageName;
	char *_saveImageName;
	int32 _saveImageX;
	int32 _saveImageY;
	int32 _loadImageX;
	int32 _loadImageY;
	BaseSurface *_saveLoadImage;
	bool _hasDrawnSaveLoadImage;

	bool displayIndicator();
#ifdef ENABLE_FOXTAIL
	bool displayIndicatorFoxTail();
#endif
	uint32 _thumbnailWidth;
	uint32 _thumbnailHeight;

	bool _reportTextureFormat;
	void setResourceModule(void *resModule);

	void setEngineLogCallback(ENGINE_LOG_CALLBACK callback = nullptr, void *data = nullptr);
	ENGINE_LOG_CALLBACK _engineLogCallback;
	void *_engineLogCallbackData;

	bool _editorMode;
	void getOffset(int *offsetX, int *offsetY) const;
	void setOffset(int32 offsetX, int32 offsetY);
	int getSequence();
	int32 _offsetY;
	int32 _offsetX;
	float _offsetPercentX;
	float _offsetPercentY;
	BaseObject *_mainObject;
	bool initInput();
	bool initLoop();
	uint32 _currentTime{};
	uint32 _deltaTime;
	BaseFont *_systemFont;
	BaseFont *_videoFont;
	bool initConfManSettings();
	bool initRenderer();
	bool initialize1();
	bool initialize2();
	bool initialize3();
	//CBAccessMgr *m_AccessMgr;
	BaseTransitionMgr *_transMgr;

	void LOG(bool res, const char *fmt, ...);
	BaseRenderer *_renderer;
#ifdef ENABLE_WME3D
	BaseRenderer3D *_renderer3D;
	bool _playing3DGame;
#endif
	BaseSoundMgr *_soundMgr;
#if EXTENDED_DEBUGGER_ENABLED
	DebuggableScEngine *_scEngine;
#else
	ScEngine *_scEngine;
#endif
	BaseScriptable *_mathClass;
	BaseScriptable *_directoryClass;
	BaseSurfaceStorage *_surfaceStorage;
	BaseFontStorage *_fontStorage;
	BaseGame(const Common::String &targetName);
	~BaseGame() override;
	void debugDisable();
	void debugEnable(const char *filename = nullptr);
	bool _debugMode;
	void *_debugLogFile;
	int32 _sequence;
	virtual bool loadFile(const char *filename);
	virtual bool loadBuffer(char *buffer, bool complete = true);
	BaseArray<BaseQuickMsg *> _quickMessages;
	BaseArray<UIWindow *> _windows;
	BaseArray<BaseViewport *> _viewportStack;
	int32 _viewportSP;
	bool _mouseLeftDown;
	bool _mouseRightDown;
	bool _mouseMidlleDown;
	BaseStringTable *_stringTable;

	int _settingsResWidth;
	int _settingsResHeight;
	bool _settingsRequireAcceleration;
	bool _settingsAllowWindowed;
	bool _settingsAllowAdvanced;
	bool _settingsAllowAccessTab;
	bool _settingsAllowAboutTab;
	bool _settingsRequireSound;
	bool _settingsAllowDesktopRes;
	int32 _settingsTLMode;
	char *_settingsGameFile;
	BaseFader *_fader;
	bool _suppressScriptErrors;

	bool invalidateDeviceObjects() override;
	bool restoreDeviceObjects() override;

	virtual bool externalCall(ScScript *script, ScStack *stack, ScStack *thisStack, char *name);

	// scripting interface
	ScValue *scGetProperty(const char *name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;

	// compatibility bits
	bool _compatKillMethodThreads;

private:
	// FPS stuff
	uint32 _lastTime;
	uint32 _fpsTime;
	uint32 _framesRendered;

public:
	uint32 _surfaceGCCycleTime;
	bool _smartCache; // RO
	bool _videoSubtitles;
	bool _subtitles; // RO
	uint32 _musicStartTime[NUM_MUSIC_CHANNELS];
	bool _compressedSavegames;
	int32 _scheduledLoadSlot;
	bool _loading;
	bool _personalizedSave;
	static bool emptySaveSlot(int slot);
	static bool isSaveSlotUsed(int slot);
	static bool getSaveSlotDescription(int slot, Common::String &desc);
	static void getSaveSlotTimestamp(int slot, TimeDate *time);
	static bool getSaveSlotFilename(int slot, Common::String &desc);
	void setWindowTitle();
	bool handleMouseWheel(int32 delta) override;
	bool _quitting;
	virtual bool getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor) const;
	bool handleKeypress(Common::Event *event, bool printable = false) override;
	virtual void handleKeyRelease(Common::Event *event);
	//bool HandleAccessKey(bool Printable, DWORD CharCode, DWORD KeyData);
	virtual bool handleCustomActionStart(BaseGameCustomAction action);
	virtual bool handleCustomActionEnd(BaseGameCustomAction action);
	int32 _freezeLevel;
	bool unfreeze();
	bool freeze(bool includingMusic = true);
	bool focusWindow(UIWindow *window);
	VideoPlayer *_videoPlayer;
	VideoTheoraPlayer *_theoraPlayer;
	bool _loadInProgress;
	UIWindow *_focusedWindow;
	bool _editorForceScripts;
	static void afterLoadScene(void *scene, void *data);
	static void afterLoadRegion(void *region, void *data);
	static void afterLoadSubFrame(void *subframe, void *data);
	static void afterLoadSound(void *sound, void *data);
	static void afterLoadFont(void *font, void *data);
#ifdef ENABLE_WME3D
	static void afterLoadXModel(void *model, void *data);
#endif
	static void afterLoadScript(void *script, void *data);
	static void invalidateValues(void *value, void *data);
	bool loadSettings(const char *filename);
	bool resumeMusic(int channel);
	bool setMusicStartTime(int channel, uint32 time);
	bool pauseMusic(int channel);
	bool stopMusic(int channel);
	bool playMusic(int channel, const char *filename, bool looping = true, uint32 loopStart = 0);
	BaseSound *_music[NUM_MUSIC_CHANNELS];
	bool _musicCrossfadeRunning;
	bool _musicCrossfadeSwap;
	uint32 _musicCrossfadeStartTime;
	uint32 _musicCrossfadeLength;
	int32 _musicCrossfadeChannel1;
	int32 _musicCrossfadeChannel2;
	int32 _musicCrossfadeVolume1;
	int32 _musicCrossfadeVolume2;
	bool displayWindows(bool inGame = false);
	Common::String readRegistryString(const Common::String &key, const Common::String &initValue) const;
	bool _useD3D;
	virtual bool cleanup();
	bool loadGame(uint32 slot);
	bool loadGame(const char *filename);
	bool saveGame(int32 slot, const char *desc, bool quickSave = false);
	bool showCursor() override;
	BaseSprite *_cursorNoninteractive;
	BaseObject *_activeObject;
	BaseKeyboardState *_keyboardState;
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

	BaseObject *_capturedObject;
	Common::Point32 _mousePos;
	bool validObject(BaseObject *object);
	bool unregisterObject(BaseObject *object);
	bool registerObject(BaseObject *object);
	void quickMessage(const char *text);
	void quickMessageForm(char *fmt, ...);
	bool displayQuickMsg();
	uint32 _fps;
	bool updateMusicCrossfade();

	bool isVideoPlaying();
	bool stopVideo();

	BaseArray<BaseObject *> _regObjects;

	// accessibility flags
/*	bool m_AccessTTSEnabled;
	bool m_AccessTTSTalk;
	bool m_AccessTTSCaptions;
	bool m_AccessTTSKeypress;
	bool m_AccessKeyboardEnabled;
	bool m_AccessKeyboardCursorSkip;
	bool m_AccessKeyboardPause;

	bool m_AccessGlobalPaused;

	CUIWindow *m_AccessShieldWin;
	HRESULT AccessPause();
	HRESULT AccessUnpause();*/

public:
	virtual bool displayContent(bool update = true, bool displayAll = false);
	virtual bool displayContentSimple();
	bool _forceNonStreamedSounds;
	void resetMousePos();
	int32 _subtitlesSpeed;
	void setInteractive(bool state);
	virtual bool windowLoadHook(UIWindow *win, char **buf, char **params);
	virtual bool windowScriptMethodHook(UIWindow *win, ScScript *script, ScStack *stack, const char *name);
	bool getCurrentViewportOffset(int *offsetX = nullptr, int *offsetY = nullptr) const;
	bool getCurrentViewportRect(Common::Rect32 *rect, bool *custom = nullptr) const;
	bool popViewport();
	bool pushViewport(BaseViewport *viewport);
	bool setActiveObject(BaseObject *obj);

	BaseSprite *_lastCursor;
	bool drawCursor(BaseSprite *cursor);

	virtual bool initAfterLoad();

	SaveThumbHelper *_cachedThumbnail;

private:
	bool getSaveDir(char *Buffer);



protected:
	// WME Lite specific
	bool _autoSaveOnExit;
	uint32 _autoSaveSlot;
	bool _cursorHidden;

public:
	BaseGameMusic *_musicSystem;
	Common::String _targetName;

	bool isLeftDoubleClick();
	bool isRightDoubleClick();

	void setIndicatorVal(int value);
	bool getBilinearFiltering() { return _bilinearFiltering; }
	void addMem(int32 bytes);

	bool _bilinearFiltering{};
#ifdef ENABLE_WME3D
	bool _force2dRenderer{};
#endif

	AnsiString getDeviceType() const;

	struct LastClickInfo {
		LastClickInfo() {
			posX = posY = 0;
			time = 0;
		}

		int32 posX;
		int32 posY;
		uint32 time;
	};

	LastClickInfo _lastClick[2];
	bool isDoubleClick(int32 buttonIndex);
	uint32 _usedMem;

	void autoSaveOnExit();
	PluginEvent &pluginEvents() { return _pluginEvents; }

private:

#ifdef ENABLE_HEROCRAFT
	// HeroCraft games specific random source with ability a in-script function to set the seed
	Common::RandomSource *_rndHc;

	// HeroCraft games specific checksum function, used in Papa's Daughters 2 selfcheck
	uint8 getFilePartChecksumHc(const char *filename, uint32 begin, uint32 end);
#endif

	PluginEvent _pluginEvents;
};

} // End of namespace Wintermute

#endif
