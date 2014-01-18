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

#ifndef FULLPIPE_FULLPIPE_H
#define FULLPIPE_FULLPIPE_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/system.h"

#include "audio/mixer.h"

#include "graphics/surface.h"

#include "engines/engine.h"

#include "gui/debugger.h"
#include "fullpipe/console.h"

struct ADGameDescription;

namespace Fullpipe {

enum FullpipeGameFeatures {
};

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
struct MovTable;
class MGM;
class NGIArchive;
class PictureObject;
struct PreloadItem;
class Scene;
class SoundList;
class StaticANIObject;
class Vars;

int global_messageHandler1(ExCommand *cmd);
int global_messageHandler2(ExCommand *cmd);
int global_messageHandler3(ExCommand *cmd);
int global_messageHandler4(ExCommand *cmd);
void global_messageHandler_handleSound(ExCommand *cmd);


class FullpipeEngine : public ::Engine {
protected:

	Common::Error run();

public:
	FullpipeEngine(OSystem *syst, const ADGameDescription *gameDesc);
	virtual ~FullpipeEngine();

	Console *_console;
	GUI::Debugger *getDebugger() { return _console; }

	void initialize();

	void setMusicAllowed(int val) { _musicAllowed = val; }

	// Detection related functions
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	Common::Platform getPlatform() const;
	bool hasFeature(EngineFeature f) const;

	Common::RandomSource *_rnd;

	Common::KeyCode _keyState;
	uint16 _buttonState;

	void updateEvents();

	Graphics::Surface _backgroundSurface;

	GameLoader *_gameLoader;
	GameProject *_gameProject;
	bool loadGam(const char *fname, int scene = 0);

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

	Common::Rect _sceneRect;
	int _sceneWidth;
	int _sceneHeight;
	Scene *_currentScene;
	Scene *_loaderScene;
	Scene *_scene2;
	Scene *_scene3;
	StaticANIObject *_aniMan;
	StaticANIObject *_aniMan2;
	byte *_globalPalette;

	InputController *_inputController;
	bool _inputDisabled;

	int _currentCheat;
	int _currentCheatPos;

	void defHandleKeyDown(int key);

	SoundList *_currSoundList1[11];
	int _currSoundListCount;
	bool _soundEnabled;
	bool _flgSoundList;

	void stopAllSounds();
	void toggleMute();
	void playSound(int id, int flag);
	void playTrack(GameVar *sceneVar, const char *name, bool delayed);
	void startSceneTrack();
	void stopSoundStream2();
	void stopAllSoundStreams();
	void stopAllSoundInstances(int id);

	int _sfxVolume;

	GlobalMessageQueueList *_globalMessageQueueList;
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

	BehaviorManager *_behaviorManager;

	MovTable *_movTable;

	Floaters *_floaters;
	MGM *_mgm;

	Common::Array<Common::Point *> _arcadeKeys;

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

	int _cursorId;
	int _minCursorId;
	int _maxCursorId;
	Common::Array<int> _objectIdCursors;
	GameObject *_objectAtCursor;
	int _objectIdAtCursor;

	void setCursor(int id);
	void updateCursorCommon();

	int getObjectState(const char *objname);
	void setObjectState(const char *name, int state);
	int getObjectEnumState(const char *name, const char *state);

	bool sceneSwitcher(EntranceInfo *entrance);
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

	int lift_getButtonIdP(int objid);
	void lift_setButton(const char *name, int state);
	void lift_sub5(Scene *sc, int qu1, int qu2);
	void lift_exitSeq(ExCommand *ex);
	void lift_closedoorSeq();
	void lift_animation3();
	void lift_goAnimation();
	void lift_sub1(StaticANIObject *ani);
	void lift_startExitQueue();
	void lift_sub05(ExCommand *ex);
	bool lift_checkButton(const char *varname);

	GameVar *_musicGameVar;
	Audio::SoundHandle _sceneTrackHandle;
public:

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }

};

extern FullpipeEngine *g_fp;
extern Vars *g_vars;

} // End of namespace Fullpipe

#endif /* FULLPIPE_FULLPIPE_H */
