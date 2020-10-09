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

#ifndef NGI_FULLPIPE_H
#define NGI_FULLPIPE_H

#include "audio/mixer.h"
#include "common/scummsys.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/ptr.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "engines/engine.h"
#include "ngi/console.h"

namespace Audio {
class SoundHandle;
}

namespace NGI {

enum {
	kDebugPathfinding	= 1 << 0,
	kDebugDrawing		= 1 << 1,
	kDebugLoading		= 1 << 2,
	kDebugAnimation		= 1 << 3,
	kDebugMemory		= 1 << 4,
	kDebugEvents		= 1 << 5,
	kDebugBehavior		= 1 << 6,
	kDebugInventory		= 1 << 7,
	kDebugSceneLogic	= 1 << 8,
	kDebugInteractions	= 1 << 9,
	kDebugXML			= 1 << 10
};

#define MAXGAMEOBJH 10000

struct NGIGameDescription;


class BehaviorManager;
class BaseModalObject;
class GameLoader;
class GameVar;
class InputController;
class Inventory2;
struct CursorInfo;
struct EntranceInfo;
class ExCommand;
class Floaters;
class GameProject;
class GameObject;
class GlobalMessageQueueList;
struct MessageHandler;
class MessageQueue;
class AniHandler;
class NGIArchive;
class PictureObject;
struct PreloadItem;
class Scene;
class SoundList;
class StaticANIObject;
class Vars;
typedef Common::Array<int16> MovTable;
typedef Common::Array<Common::Point> PointList;

struct Palette {
	uint32 pal[256];
	uint size;

	Palette() {
		size = 0;
		memset(pal, 0, 256 * 4);
	}

	void copy(const Palette &src) {
		size = src.size;
		memcpy(pal, src.pal, 256 * 4);
	}
};

typedef Common::HashMap<uint16, Common::String> GameObjHMap;

int global_messageHandler1(ExCommand *cmd);
int global_messageHandler2(ExCommand *cmd);
int global_messageHandler3(ExCommand *cmd);
int global_messageHandler4(ExCommand *cmd);
void global_messageHandler_handleSound(ExCommand *cmd);


class NGIEngine : public ::Engine {
protected:

	Common::Error run() override;

public:
	NGIEngine(OSystem *syst, const NGIGameDescription *gameDesc);
	~NGIEngine() override;

	void initialize();
	void restartGame();
	bool shouldQuit();

	void setMusicAllowed(int val) { _musicAllowed = val; }

	// Detection related functions
	const NGIGameDescription *_gameDescription;
	uint32 getFeatures() const;
	bool isDemo();
	Common::Language getLanguage() const;
	const char *getGameId() const;
	int getGameGID() const;

	Common::RandomSource _rnd;

	Common::KeyCode _keyState;
	uint16 _buttonState;

	void updateEvents();

	Graphics::Surface _backgroundSurface;
	Graphics::PixelFormat _origFormat;

	Common::ScopedPtr<GameLoader> _gameLoader;
	GameProject *_gameProject;
	bool loadGam(const char *fname, int scene = 0);

	void loadGameObjH();
	Common::String gameIdToStr(uint16 id);

	GameVar *getGameLoaderGameVar();
	InputController *getGameLoaderInputController();

	int _gameProjectVersion;
	int _pictureScale;
	int _scrollSpeed;
	bool _updateFlag;
	bool _flgCanOpenMap;
	bool _gamePaused;
	bool _flgGameIsRunning;
	bool _inputArFlag;
	bool _recordEvents;
	bool _mainMenu_debugEnabled;

	Common::Rect _sceneRect;
	int _sceneWidth;
	int _sceneHeight;
	Scene *_currentScene;
	Scene *_loaderScene;
	Scene *_scene2;
	Scene *_scene3;
	StaticANIObject *_aniMan;
	StaticANIObject *_aniMan2;
	Palette _defaultPalette;
	const Palette *_globalPalette;

	InputController *_inputController;
	bool _inputDisabled;

	int _currentCheat;
	int _currentCheatPos;

	void defHandleKeyDown(int key);

	SoundList *_currSoundList1[11];
	int _currSoundListCount;
	bool _soundEnabled;
	bool _flgSoundList;
	Common::String _sceneTracks[10];
	int _numSceneTracks;
	bool _sceneTrackHasSequence;
	int _musicMinDelay;
	int _musicMaxDelay;
	int _musicLocal;
	Common::String _trackName;
	int _trackStartDelay;
	Common::String _sceneTracksCurrentTrack;
	bool _sceneTrackIsPlaying;

	void stopAllSounds();
	void toggleMute();
	void playSound(int id, int flag);
	void playTrack(GameVar *sceneVar, const char *name, bool delayed);
	int getSceneTrack();
	void updateTrackDelay();
	void startSceneTrack();
	void startSoundStream1(const Common::String &trackName);
	void playOggSound(const Common::String &trackName, Audio::SoundHandle &stream);
	void stopSoundStream2();
	void stopAllSoundStreams();
	void stopAllSoundInstances(int id);
	void updateSoundVolume();
	void setMusicVolume(int vol);

	int _sfxVolume;
	int _musicVolume;

	Common::ScopedPtr<GlobalMessageQueueList> _globalMessageQueueList;
	MessageHandler *_messageHandlers;

	int _msgX;
	int _msgY;
	int _msgObjectId2;
	int _msgId;

	Common::List<ExCommand *> _exCommandList;
	bool _isProcessingMessages;

	int _mouseVirtX;
	int _mouseVirtY;
	Common::Point _mouseScreenPos;

	Common::ScopedPtr<BehaviorManager> _behaviorManager;

	Common::ScopedPtr<MovTable> _movTable;

	Common::ScopedPtr<Floaters> _floaters;
	Common::ScopedPtr<AniHandler> _aniHandler;

	Common::Array<Common::Point> _arcadeKeys;

	void deleteModalObject();

	void initMap();
	void updateMap(PreloadItem *pre);
	void updateMapPiece(int mapId, int update);
	void updateScreen();

	void freeGameLoader();
	void cleanup();

	bool _gameContinue;
	bool _needRestart;
	bool _flgPlayIntro;
	int _musicAllowed;
	bool _normalSpeed;

	void enableSaves() { _isSaveAllowed = true; }
	void disableSaves(ExCommand *ex);
	bool isSaveAllowed();

	void initObjectStates();
	void setLevelStates();
	void setSwallowedEggsState();
	void loadAllScenes();

	void initCursors();
	void addCursor(CursorInfo *cursorInfo, Scene *inv, int pictureId, int hotspotX, int hotspotY, int itemPictureOffsX, int itemPictureOffsY);

	int32 _mapTable[200];

	Scene *_inventoryScene;
	Inventory2 *_inventory;
	int _currSelectedInventoryItemId;

	int32 _updateTicks;
	int32 _lastInputTicks;
	int32 _lastButtonUpTicks;

	BaseModalObject *_modalObject;

	int (*_updateScreenCallback)();
	int (*_updateCursorCallback)();

	void drawAlphaRectangle(int x1, int y1, int x2, int y2, int alpha);
	void sceneFade(Scene *sc, bool direction);

	int _cursorId;
	int _minCursorId;
	int _maxCursorId;
	Common::Array<int> _objectIdCursors;
	GameObject *_objectAtCursor;
	int _objectIdAtCursor;

	void setCursor(int id);
	void updateCursorCommon();

	int getObjectState(const Common::String &objname);
	void setObjectState(const Common::String &name, int state);
	int getObjectEnumState(const Common::String &name, const char *state);

	void sceneAutoScrolling();
	bool sceneSwitcher(const EntranceInfo &entrance);
	Scene *accessScene(int sceneId);
	void setSceneMusicParameters(GameVar *var);
	int convertScene(int scene);
	int getSceneEntrance(int scene);
	int getSceneFromTag(int tag);

	NGIArchive *_currArchive;

	void openMap();
	void openHelp();
	void openMainMenu();

	PictureObject *_arcadeOverlay;
	PictureObject *_arcadeOverlayHelper;
	int _arcadeOverlayX;
	int _arcadeOverlayY;
	int _arcadeOverlayMidX;
	int _arcadeOverlayMidY;

	void initArcadeKeys(const char *varname);
	void processArcade(ExCommand *ex);
	void winArcade();
	void setArcadeOverlay(int picId);
	int drawArcadeOverlay(int adjust);

	void getAllInventory();

	StaticANIObject *_lastLiftButton;
	MessageQueue *_liftEnterMQ;
	MessageQueue *_liftExitMQ;
	StaticANIObject *_lift;
	int _liftX;
	int _liftY;

	int lift_getButtonIdP(int objid);
	int lift_getButtonIdH(int objid);
	int lift_getButtonIdN(int objid);
	void lift_setButton(const char *name, int state);
	void lift_init(Scene *sc, int qu1, int qu2);
	void lift_setButtonStatics(Scene *sc, int buttonId);
	void lift_exitSeq(ExCommand *ex);
	void lift_closedoorSeq();
	void lift_clickButton();
	void lift_walkAndGo();
	void lift_goAnimation();
	void lift_animateButton(StaticANIObject *button);
	void lift_startExitQueue();
	void lift_hoverButton(ExCommand *ex);
	bool lift_checkButton(const char *varname);
	void lift_openLift();

	GameVar *_musicGameVar;
	Audio::SoundHandle _soundStream1;
	Audio::SoundHandle _soundStream2;
	Audio::SoundHandle _soundStream3;
	Audio::SoundHandle _soundStream4;

	bool _stream2playing;

	GameObjHMap _gameObjH;

public:

	bool _isSaveAllowed;

	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &description, bool isAutosave = false) override;
	virtual Common::String getSaveStateName(int slot) const override;

	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override { return _isSaveAllowed; }
	bool hasFeature(EngineFeature f) const override;

};

extern NGIEngine *g_nmi;
extern Vars *g_vars;

} // End of namespace NGI

#endif /* NGI_FULLPIPE_H */
