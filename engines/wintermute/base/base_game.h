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
#include "engines/wintermute/base/timer.h"
#include "engines/wintermute/ext/plugin_event.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/math/rect32.h"
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

class BaseGame: public BaseObject {
public:
	DECLARE_PERSISTENT(BaseGame, BaseObject)

	virtual bool getLayerSize(int *LayerWidth, int *LayerHeight, Rect32 *viewport, bool *customViewport);
#ifdef ENABLE_WME3D
	virtual uint32 getAmbientLightColor();
	virtual bool getFogParams(bool *fogEnabled, uint32 *fogColor, float *start, float *end);
#endif

	virtual bool onScriptShutdown(ScScript *script);

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

	virtual void miniUpdate();

	void getMousePos(Point32 *Pos);
	Rect32 _mouseLockRect;

	bool _shuttingDown;

	virtual bool displayDebugInfo();

	void setShowFPS(bool enabled) { _debugShowFPS = enabled; }
	bool getBilinearFiltering() { return _bilinearFiltering; }
	bool getSuspendedRendering() const { return _suspendedRendering; }
	virtual bool renderShadowGeometry();

	TTextEncoding _textEncoding;
	bool _textRTL;

	virtual bool resetContent();

	void DEBUG_DumpClassRegistry();
	bool setWaitCursor(const char *filename);

#ifdef ENABLE_WME3D
	bool _supportsRealTimeShadows;
	TShadowType _maxShadowType;

	bool setMaxShadowType(TShadowType maxShadowType);
	virtual TShadowType getMaxShadowType(BaseObject *object = nullptr);

	int32 _editorResolutionWidth;
	int32 _editorResolutionHeight;
#endif

	uint32 getSaveThumbWidth() const { return _thumbnailWidth; }
	uint32 getSaveThumbHeight() const { return _thumbnailHeight; }

	bool _editorMode;
	void getOffset(int *offsetX, int *offsetY) const;
	void setOffset(int32 offsetX, int32 offsetY);
	int getSequence();

	int32 _offsetY;
	int32 _offsetX;
	float _offsetPercentX;
	float _offsetPercentY;

	inline BaseObject *getMainObject() { return _mainObject; }
	inline BaseFont *getSystemFont() { return _systemFont; }
	inline BaseFont *getVideoFont() { return _videoFont; }

	bool initInput();
	bool initLoop();
	uint32 _currentTime;
	uint32 _deltaTime;

	// Init-functions:
	bool initConfManSettings();
	bool initRenderer();
	bool loadGameSettingsFile();
	bool initialize1();
	bool initialize2();
	bool initialize3();
	BaseTransitionMgr *_transMgr;

	// String Table
	void expandStringByStringTable(char **str) const;
	void expandStringByStringTable(Common::String &str) const;
	char *getKeyFromStringTable(const char *str) const;

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

	bool _debugDebugMode;

	int32 _sequence;
	virtual bool loadFile(const char *filename);
	virtual bool loadBuffer(char *buffer, bool complete = true);

	int32 _viewportSP;

	bool _suppressScriptErrors;
	bool _mouseLeftDown; // TODO: Hide

	virtual bool externalCall(ScScript *script, ScStack *stack, ScStack *thisStack, char *name);
	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
	// compatibility bits
	bool _compatKillMethodThreads;

	const char* getGameTargetName() const { return _targetName.c_str(); }
	void setGameTargetName(const Common::String& targetName) { _targetName = targetName; }
	uint32 _surfaceGCCycleTime;
	bool _smartCache; // RO
	bool _subtitles; // RO

	int32 _scheduledLoadSlot;

	bool getIsLoading() const { return _loading; }

	bool handleMouseWheel(int32 delta) override;
	bool _quitting;
	virtual bool getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor) const;

	bool handleKeypress(Common::Event *event, bool printable = false) override;
	virtual void handleKeyRelease(Common::Event *event);
	virtual bool handleCustomActionStart(BaseGameCustomAction action);
	virtual bool handleCustomActionEnd(BaseGameCustomAction action);

	bool unfreeze();
	bool freeze(bool includingMusic = true);
	bool focusWindow(UIWindow *window);
	bool _loadInProgress;
	UIWindow *_focusedWindow;
	bool _editorForceScripts;

	static void invalidateValues(void *value, void *data);

	bool loadSettings(const char *filename);

	bool displayWindows(bool inGame = false);
	bool _useD3D;
	virtual bool cleanup();
	bool loadGame(uint32 slot);
	bool loadGame(const char *filename);
	bool saveGame(int32 slot, const char *desc, bool quickSave = false);
	bool showCursor() override;

	BaseObject *_activeObject;

	bool _interactive;
	TGameState _state;
	TGameState _origState;
	bool _origInteractive;

	const Timer *getTimer() const { return &_timerNormal; }
	const Timer *getLiveTimer() const { return &_timerLive; }
private:
	Timer _timerNormal;
	Timer _timerLive;
public:
	BaseObject *_capturedObject;
	Point32 _mousePos;
	bool validObject(BaseObject *object);
	bool unregisterObject(BaseObject *object);
	bool registerObject(BaseObject *object);
	void quickMessage(const char *text);
	void quickMessageForm(char *fmt, ...);
	bool displayQuickMsg();

	virtual bool displayContent(bool update = true, bool displayAll = false);
	virtual bool displayContentSimple();
	bool _forceNonStreamedSounds;
	void resetMousePos();
	int32 _subtitlesSpeed;
	void setInteractive(bool state);
	virtual bool windowLoadHook(UIWindow *win, char **buf, char **params);
	virtual bool windowScriptMethodHook(UIWindow *win, ScScript *script, ScStack *stack, const char *name);
	bool getCurrentViewportOffset(int *offsetX = nullptr, int *offsetY = nullptr) const;
	bool getCurrentViewportRect(Rect32 *rect, bool *custom = nullptr) const;
	bool popViewport();
	bool pushViewport(BaseViewport *Viewport);
	bool setActiveObject(BaseObject *Obj);
	BaseSprite *_lastCursor;
	bool drawCursor(BaseSprite *Cursor);
	bool storeSaveThumbnail();
	void deleteSaveThumbnail();

	SaveThumbHelper *_cachedThumbnail;
	void addMem(int32 bytes);
	bool _touchInterface;
	bool _constrainedMemory;

	bool stopVideo();
protected:
	BaseFont *_systemFont;
	BaseFont *_videoFont;

	BaseSprite *_loadingIcon;
	int32 _loadingIconX;
	int32 _loadingIconY;
	int32 _loadingIconPersistent;

	BaseFader *_fader;

	int32 _freezeLevel;
	VideoPlayer *_videoPlayer;
	VideoTheoraPlayer *_theoraPlayer;
private:
	bool _debugShowFPS;
	bool _bilinearFiltering;
#ifdef ENABLE_WME3D
	bool _force2dRenderer;
#endif
	void *_debugLogFile;
	void DEBUG_DebugDisable();
	void DEBUG_DebugEnable(const char *filename = nullptr);

	BaseObject *_mainObject;

	bool _mouseRightDown;
	bool _mouseMidlleDown;

	BaseGameSettings *_settings;

	int32 _soundBufferSizeSec;

	bool invalidateDeviceObjects() override;
	bool restoreDeviceObjects() override;

	// TODO: This can probably be removed completely:
	bool _saveDirChecked;

	Common::String _localSaveDir;
	bool _loading;

	bool _reportTextureFormat;

	// FPS stuff
	uint32 _lastTime;
	uint32 _fpsTime;
	uint32 _framesRendered;
	Common::String _targetName;

	void setEngineLogCallback(ENGINE_LOG_CALLBACK callback = nullptr, void *data = nullptr);
	ENGINE_LOG_CALLBACK _engineLogCallback;
	void *_engineLogCallbackData;

	bool _videoSubtitles;

	bool _personalizedSave;

	uint32 _thumbnailWidth;
	uint32 _thumbnailHeight;

	void setWindowTitle();

	bool _suspendedRendering;

	BaseSprite *_cursorNoninteractive;
	BaseKeyboardState *_keyboardState;

	uint32 _fps;
	BaseGameMusic *_musicSystem;

	bool isVideoPlaying();

	BaseArray<BaseQuickMsg *> _quickMessages;
	BaseArray<UIWindow *> _windows;
	BaseArray<BaseViewport *> _viewportStack;
	BaseArray<BaseObject *> _regObjects;

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

// TODO: This should be expanded into a proper class eventually:
	Common::String readRegistryString(const Common::String &key, const Common::String &initValue) const;


protected:
	// WME Lite specific
	bool _autoSaveOnExit;
	uint32 _autoSaveSlot;
	bool _cursorHidden;

public:
	void autoSaveOnExit();
	PluginEvent &pluginEvents() { return _pluginEvents; }

#ifdef ENABLE_HEROCRAFT
private:
	// HeroCraft games specific random source with ability a in-script function to set the seed
	Common::RandomSource *_rndHc;

	// HeroCraft games specific checksum function, used in Papa's Daughters 2 selfcheck
	uint8 getFilePartChecksumHc(const char *filename, uint32 begin, uint32 end);
#endif

	PluginEvent _pluginEvents;
};

} // End of namespace Wintermute

#endif
