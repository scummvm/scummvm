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

#ifndef BBVS_BBVS_H
#define BBVS_BBVS_H

#include "audio/mixer.h"

#include "common/array.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/random.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/system.h"

#include "engines/engine.h"

#include "bbvs/detection.h"

struct ADGameDescription;

namespace Bbvs {

class ActionCommands;
struct Action;
class GameModule;
struct Condition;
struct Conditions;
struct ActionResult;
struct ActionResults;
struct ActionCommand;
struct CameraInit;
struct SceneObjectDef;
struct SceneObjectInit;
struct SceneExit;
struct Animation;
struct SceneSound;
class DrawList;
class SpriteModule;
class Screen;
class SoundMan;

#define BBVS_SAVEGAME_VERSION 0

enum {
	kVerbLook      = 0,
	kVerbUse       = 1,
	kVerbTalk      = 2,
	kVerbWalk      = 3,
	kVerbInvItem   = 4,
	kVerbShowInv   = 5
};

enum {
	kITNone         = 0,
	kITEmpty        = 1,
	KITSceneObject  = 2,
	kITBgObject     = 3,
	kITDialog       = 4,
	kITScroll       = 5,
	kITSceneExit    = 6,
	kITInvItem      = 7
};

enum {
	kGSScene        = 0,
	kGSInventory    = 1,
	kGSVerbs        = 2,
	kGSWait         = 3,
	kGSDialog       = 4,
	kGSWaitDialog   = 5
};

enum {
	kActionCmdStop                  = 0,
	kActionCmdWalkObject            = 3,
	kActionCmdMoveObject            = 4,
	kActionCmdAnimObject            = 5,
	kActionCmdSetCameraPos          = 7,
	kActionCmdPlaySpeech            = 8,
	kActionCmdPlaySound             = 10,
	kActionCmdStartBackgroundSound  = 11,
	kActionCmdStopBackgroundSound   = 12
};

enum {
	kCondUnused                     = 1,
	kCondSceneObjectVerb            = 2,
	kCondBgObjectVerb               = 3,
	kCondSceneObjectInventory       = 4,
	kCondBgObjectInventory          = 5,
	kCondHasInventoryItem           = 6,
	kCondHasNotInventoryItem        = 7,
	kCondIsGameVar                  = 8,
	kCondIsNotGameVar               = 9,
	kCondIsPrevSceneNum             = 10,
	kCondIsCurrTalkObject           = 11,
	kCondIsDialogItem               = 12,
	kCondIsCameraNum                = 13,
	kCondIsNotPrevSceneNum          = 14,
	kCondDialogItem0                = 15,
	kCondIsButtheadAtBgObject       = 16,
	kCondIsNotSceneVisited          = 17,
	kCondIsSceneVisited             = 18,
	kCondIsCameraNumTransition      = 19
};

enum {
	kActResAddInventoryItem     = 1,
	kActResRemoveInventoryItem  = 2,
	kActResSetGameVar           = 3,
	kActResUnsetGameVar         = 4,
	kActResStartDialog          = 5,
	kActResChangeScene          = 6
};

enum {
	kLeftButtonClicked  = 1,
	kRightButtonClicked = 2,
	kLeftButtonDown     = 4,
	kRightButtonDown    = 8,
	kAnyButtonClicked   = kLeftButtonClicked | kRightButtonClicked,
	kAnyButtonDown      = kLeftButtonDown | kRightButtonDown
};

struct BBPoint {
	int16 x, y;
};

struct BBRect {
	int16 x, y, width, height;
};

struct BBPolygon {
	const BBPoint *points;
	int pointsCount;
};

struct Rect {
	int16 left, top, right, bottom;
};

struct SceneObject {
	uint32 x, y;
	SceneObjectDef *sceneObjectDef;
	Animation *anim;
	int animIndex;
	int frameIndex;
	int frameTicks;
	int walkCount;
	int xIncr, yIncr;
	int turnValue, turnCount, turnTicks;
	Common::Point walkDestPt;
	SceneObject() : sceneObjectDef(0), anim(0) {
	}
};

struct SceneObjectAction {
	int sceneObjectIndex;
	int animationIndex;
	Common::Point walkDest;
};

struct WalkInfo {
	int16 x, y;
	int delta;
	int direction;
	Common::Point midPt;
	int walkAreaIndex;
};

struct WalkArea {
	int16 x, y, width, height;
	bool checked;
	int linksCount;
	WalkArea *links[16];
	WalkInfo *linksD1[32];
	WalkInfo *linksD2[32];
	bool contains(const Common::Point &pt) const;
};

const int kSnapshotSize = 23072;
const int kSceneObjectsCount = 64;
const int kSceneSoundsCount = 8;
const int kInventoryItemStatusCount = 50;
const int kDialogItemStatusCount = 50;
const int kGameVarsCount = 2000;
const int kSceneVisitedCount = 64;

const int kMainMenu = 44;
const int kCredits  = 45;

const int kMaxDistance = 0xFFFFFF;
static const int8 kWalkTurnTbl[] = {
	7, 9, 4, 8, 6, 10, 5, 11
};

class BbvsEngine : public Engine {
protected:
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
public:
	BbvsEngine(OSystem *syst, const ADGameDescription *gd);
	~BbvsEngine() override;
	void newGame();
	void continueGameFromQuickSave();
	void setNewSceneNum(int newSceneNum);
	const Common::String getTargetName() { return _targetName; }
	const ADGameDescription *_gameDescription;

	bool isLoogieDemo() const;

private:
	Graphics::PixelFormat _pixelFormat;

#ifdef USE_TRANSLATION
	Common::String _oldGUILanguage;
#endif

public:
	Common::RandomSource *_random;

	GameModule *_gameModule;
	SpriteModule *_spriteModule;
	SoundMan *_sound;

	Screen *_screen;

	int _bootSaveSlot;

	int _mouseX, _mouseY;
	uint _mouseButtons;
	Common::KeyCode _keyCode;

	int _mouseCursorSpriteIndex;

	int _gameState;
	int _gameTicks;

	Common::Point _mousePos;
	Common::Point _verbPos;
	Common::Point _walkMousePos;

	int _activeItemType;
	int _activeItemIndex;
	int _currTalkObjectIndex;

	Common::Point _cameraPos, _newCameraPos;

	int _newSceneNum, _prevSceneNum, _currSceneNum;
	int _playVideoNumber;

	int _dialogSlotCount;
	byte _dialogItemStatus[kDialogItemStatusCount];

	byte _gameVars[kGameVarsCount];
	byte _sceneVisited[kSceneVisitedCount];

	int _currVerbNum;

	int _currInventoryItem;
	byte _inventoryItemStatus[kInventoryItemStatusCount];
	int _inventoryButtonIndex;

	Action *_currAction;
	uint32 _currActionCommandTimeStamp;
	int _currActionCommandIndex;

	Common::Array<Action*> _walkAreaActions;

	SceneObject _sceneObjects[kSceneObjectsCount];
	Common::Array<SceneObjectAction> _sceneObjectActions;

	SceneObject *_buttheadObject, *_beavisObject;
	int _currCameraNum;

	byte _backgroundSoundsActive[kSceneSoundsCount];
	Audio::SoundHandle _speechSoundHandle;

	int _walkAreasCount;
	WalkArea _walkAreas[80];
	int _walkInfosCount;
	WalkInfo _walkInfos[256];
	int _walkableRectsCount;
	Common::Rect _walkableRects[256];
	Common::Rect _tempWalkableRects1[256];
	Common::Rect _tempWalkableRects2[256];
	WalkInfo *_walkInfoPtrs[256];

	WalkArea *_sourceWalkArea, *_destWalkArea;
	Common::Point _sourceWalkAreaPt, _destWalkAreaPt, _finalWalkPt;
	int _currWalkDistance;
	bool _walkReachedDestArea;

	bool _hasSnapshot;
	byte *_snapshot;
	Common::SeekableMemoryWriteStream *_snapshotStream;

	char _easterEggInput[7];

	void updateEvents();
	int getRandom(int max);

	void drawDebugInfo();
	void drawScreen();

	void updateGame();

	bool evalCondition(Conditions &conditions);
	bool evalCameraCondition(Conditions &conditions, int value);
	int evalDialogCondition(Conditions &conditions);
	void evalActionResults(ActionResults &results);

	void updateBackgroundSounds();

	void loadScene(int sceneNum);
	void initScene(bool sounds);
	bool changeScene();
	bool update(int mouseX, int mouseY, uint mouseButtons, Common::KeyCode keyCode);

	void buildDrawList(DrawList &drawList);

	void updateVerbs();
	void updateDialog(bool clicked);
	void updateInventory(bool clicked);
	void updateScene(bool clicked);

	bool performActionCommand(ActionCommand *actionCommand);
	bool processCurrAction();
	void skipCurrAction();

	void updateCommon();

	void updateWalkableRects();
	void startWalkObject(SceneObject *sceneObject);
	void updateWalkObject(SceneObject *sceneObject);
	void walkObject(SceneObject *sceneObject, const Common::Point &destPt, int walkSpeed);
	void turnObject(SceneObject *sceneObject);

	int rectSubtract(const Common::Rect &rect1, const Common::Rect &rect2, Common::Rect *outRects);

	WalkInfo *addWalkInfo(int16 x, int16 y, int delta, int direction, int16 midPtX, int16 midPtY, int walkAreaIndex);
	void initWalkAreas(SceneObject *sceneObject);
	WalkArea *getWalkAreaAtPos(const Common::Point &pt);
	bool canButtheadWalkToDest(const Common::Point &destPt);
	void canWalkToDest(WalkArea *walkArea, int infoCount);
	bool walkTestLineWalkable(const Common::Point &sourcePt, const Common::Point &destPt, WalkInfo *walkInfo);
	void walkFindPath(WalkArea *sourceWalkArea, int infoCount);
	int calcDistance(const Common::Point &pt1, const Common::Point &pt2);
	void walkFoundPath(int count);

	void updateSceneObjectsTurnValue();
	void updateDialogConditions();

	void playSpeech(int soundNum);
	void stopSpeech();

	void playSound(uint soundNum, bool loop = false);
	void stopSound(uint soundNum);
	void stopSounds();

	bool runMinigame(int minigameNum);
	void playVideo(int videoNum);

	void runMainMenu();
	void checkEasterEgg(char key);

	// Savegame API

	enum kReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;
		uint32 saveDate;
		uint32 saveTime;
		uint32 playTime;
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently() override { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() override { return _isSaveAllowed; }
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &description, bool isAutosave = false) override;
	void savegame(const char *filename, const char *description);
	void loadgame(const char *filename);
	bool existsSavegame(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	WARN_UNUSED_RESULT static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail = true);

	void allocSnapshot();
	void freeSnapshot();
	void saveSnapshot();

	void writeContinueSavegame();

};

} // End of namespace Bbvs

#endif // BBVS_BBVS_H
