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

#ifndef DGDS_SCENE_H
#define DGDS_SCENE_H

#include "common/stream.h"
#include "common/array.h"
#include "common/serializer.h"

#include "dgds/dialog.h"
#include "dgds/dgds_rect.h"
#include "dgds/minigames/shell_game.h"

namespace Dgds {

class ResourceManager;
class Decompressor;
class DgdsFont;

enum SceneCondition {
	kSceneCondNone = 0,
	kSceneCondLessThan = 1,
	kSceneCondEqual = 2,
	kSceneCondNegate = 4,
	kSceneCondAbsVal = 8,
	kSceneCondOr = 0x10,
	kSceneCondNeedItemSceneNum = 0x20,
	kSceneCondNeedItemQuality = 0x40,
	kSceneCondSceneState = 0x80
};

class SceneConditions {
public:
	SceneConditions(uint16 num, SceneCondition cond, uint16 val) : _num(num), _flags(cond), _val(val) {}
	Common::String dump(const Common::String &indent) const;

	uint16 getNum() const { return _num; }
	SceneCondition getCond() const { return _flags; }
	uint16 getVal() const { return _val; }

private:
	uint16 _num;
	SceneCondition _flags; /* eg, see usage in FUN_1f1a_2106 */
	uint16 _val;
};

class HotArea {
public:
	DgdsRect _rect;
	uint16 _num; //
	uint16 _cursorNum;

	// Used in Willy Beamish
	uint16 _otherCursorNum;
	uint16 _objInteractionListFlag;

	Common::Array<SceneConditions> enableConditions;
	Common::Array<SceneOp> onRClickOps;
	Common::Array<SceneOp> onLDownOps;
	Common::Array<SceneOp> onLClickOps;

	virtual ~HotArea() {}

	virtual Common::String dump(const Common::String &indent) const;
};

enum SceneOpCode {
	kSceneOpNone = 0,
	kSceneOpChangeScene = 1,  	// args: scene num
	kSceneOpNoop = 2,		 	// args: none. Maybe should close dialogue?
	kSceneOpGlobal = 3,			// args: array of uints
	kSceneOpSegmentStateOps = 4,	// args: array of uint pairs [op seg, op seg], term with 0,0 that modify segment states
	kSceneOpSetItemAttr = 5,	// args: [item num, item param 0x28, item param 0x2c]. set item attrs?
	kSceneOpSetDragItem = 6,		// args: item num. give item?
	kSceneOpOpenInventory = 7,	// args: none.
	kSceneOpShowDlg = 8,		// args: dialogue number.
	kSceneOpShowInvButton = 9,		// args: none.
	kSceneOpHideInvButton = 10,	// args: none.
	kSceneOpEnableTrigger = 11,	// args: trigger num
	kSceneOpChangeSceneToStored = 12,	// args: none. Change scene to stored number
	kSceneOpAddFlagToDragItem = 13,			// args: none.
	kSceneOpOpenInventoryZoom = 14,	// args: none.
	kSceneOpMoveItemsBetweenScenes = 15,	// args: none.
	kSceneOpShowClock = 16,		// args: none.  set clock script-visible.
	kSceneOpHideClock = 17,		// args: none.  set clock script-hidden.
	kSceneOpShowMouse = 18,		// args: none.
	kSceneOpHideMouse = 19,		// args: none.
	// Op 20 onward are common, but not in dragon

	kSceneOpLoadTalkDataAndSetFlags = 20, // args: tdsnum to load, headnum
	kSceneOpDrawVisibleTalkHeads = 21, // args: none
	kSceneOpLoadTalkData = 22, 	// args: tds num to load
	kSceneOpLoadDDSData = 24, 	// args: dds num to load
	kSceneOpFreeDDSData = 25,	// args: dds num to free
	kSceneOpFreeTalkData = 26, 	// args: tds num to free

	// Dragon-specific opcodes
	kSceneOpPasscode = 100,			// args: none.
	kSceneOpMeanwhile = 101,	// args: none. Clears screen and displays "meanwhile".
	kSceneOpOpenGameOverMenu = 102,	// args: none.
	kSceneOpTiredDialog = 103,			// args: none. Something about "boy am I tired"?
	kSceneOpArcadeTick = 104,			// args: none. Called in arcade post-tick.
	kSceneOpDrawDragonCountdown1 = 105,			// args: none. Draw special countdown number at 141, 56
	kSceneOpDrawDragonCountdown2 = 106,			// args: none. Draw some number at 250, 42
	kSceneOpOpenPlaySkipIntroMenu = 107, // args: none.  DRAGON: Show menu 50, the "Play Introduction" / "Skip Introduction" menu.
	kSceneOpOpenBetterSaveGameMenu = 108,			// args: none. DRAGON: Show menu 46, the "Before arcade maybe you better save your game" menu.

	// China-specific opcodes
	kSceneOpChinaTankInit = 100,
	kSceneOpChinaTankEnd = 101,
	kSceneOpChinaTankTick = 102,
	kSceneOpChinaSetLanding = 103,
	kSceneOpChinaScrollIntro = 104,
	kSceneOpChinaScrollLeft = 105,
	kSceneOpChinaScrollRight = 107,
	kSceneOpShellGameInit = 108,
	kSceneOpShellGameEnd = 109,
	kSceneOpShellGameTick = 110,
	kSceneOpChinaTrainInit = 111,
	kSceneOpChinaTrainEnd = 112,
	kSceneOpChinaTrainTick = 113,
	kSceneOpChinaOpenGameOverMenu = 114,	// args: none.
	kSceneOpChinaOpenSkipCreditsMenu = 115,	// args: none.
	kSceneOpChinaOnIntroTick = 116,	// args: none.
	kSceneOpChinaOnIntroInit = 117,	// args: none.
	kSceneOpChinaOnIntroEnd = 118,	// args: none.

	// Beamish-specific opcodes
	kSceneOpOpenBeamishGameOverMenu = 100,
	kSceneOpOpenBeamishOpenSkipCreditsMenu = 101,

	kSceneOpMaxCode = 255, // for checking file load
};

class SceneOp {
public:
	Common::Array<SceneConditions> _conditionList;
	Common::Array<uint16> _args;
	SceneOpCode _opCode;

	Common::String dump(const Common::String &indent) const;
};

class ConditionalSceneOp {
public:
	uint _opCode;
	Common::Array<SceneConditions> _conditionList;
	Common::Array<SceneOp> _opList;

	Common::String dump(const Common::String &indent) const;
};

class GameItem : public HotArea {
public:
	Common::Array<SceneOp> onDragFinishedOps;
	Common::Array<SceneOp> onBothButtonsOps;
	uint16 _altCursor;
	uint16 _iconNum;

	// mutable values
	uint16 _inSceneNum;
	uint16 _flags;
	uint16 _quality;

	Common::String dump(const Common::String &indent) const override;
};

class MouseCursor {
public:
	MouseCursor(uint16 hotX, uint16 hotY) : _hot(hotX, hotY) {}

	Common::String dump(const Common::String &indent) const;

	const Common::Point getHot() const { return _hot; }

private:
	Common::Point _hot;
};

// Interactions between two objects when one is dropped on the other
class ObjectInteraction {
public:
	ObjectInteraction(uint16 dropped, uint16 target) : _droppedItemNum(dropped), _targetItemNum(target) {}

	Common::Array<SceneOp> opList;

	bool matches(uint16 droppedItemNum, uint16 targetItemNum) const {
		return _droppedItemNum == droppedItemNum && _targetItemNum == targetItemNum;
	}

	Common::String dump(const Common::String &indent) const;

private:
	uint16 _droppedItemNum;
	uint16 _targetItemNum;

};

class SceneTrigger {
public:
	SceneTrigger(uint16 num) : _num(num), _enabled(false), _timesToCheckBeforeRunning(0) {}
	Common::String dump(const Common::String &indent) const;

	Common::Array<SceneConditions> conditionList;
	Common::Array<SceneOp> sceneOpList;

	uint16 _timesToCheckBeforeRunning; // Only used in Beamish.
	bool _enabled;
	uint16 getNum() const { return _num; }

private:
	uint16 _num;
};

/* A global value that only applies on a per-SDS-scene,
   but stays with the GDS data as it sticks around during
   the game */
class PerSceneGlobal {
public:
	PerSceneGlobal(uint16 num, uint16 scene) : _num(num), _sceneNo(scene), _val(0) {}

	Common::String dump(const Common::String &indent) const;
	bool matches(uint16 num, uint16 scene) const { return num == _num && (_sceneNo == 0 || _sceneNo == scene); }
	bool numMatches(uint16 num) const { return num == _num; }

	int16 _val;

private:
	// Immutable, read from the data file
	uint16 _num;
	uint16 _sceneNo;
};


class TalkDataHeadFrame {
public:
	TalkDataHeadFrame() : _xoff(0), _yoff(0), _frameNo(0) {}
	Common::String dump(const Common::String &indent) const;

	uint16 _frameNo;
	uint16 _xoff;
	uint16 _yoff;
};

enum HeadFlags {
	kHeadFlagNone = 0,
	kHeadFlag1 = 1,
	kHeadFlag2 = 2,
	kHeadFlag4 = 4,
	kHeadFlag8 = 8,
	kHeadFlag10 = 0x10,
	kHeadFlagVisible = 0x20,
	kHeadFlag40 = 0x40,
	kHeadFlag80 = 0x80,
};

class TalkDataHead {
public:
	TalkDataHead() : _num(0), _drawType(0), _drawCol(0), _val3(0), _flags(kHeadFlagNone) {}
	Common::String dump(const Common::String &indent) const;

	uint16 _num;
	uint16 _drawType;
	uint16 _drawCol;
	DgdsRect _rect;
	Common::Array<TalkDataHeadFrame> _headFrames;
	uint16 _val3;
	HeadFlags _flags;
};

class TalkData {
public:
	TalkData() : _num(0), _val(0) {}
	Common::String dump(const Common::String &indent) const;

	uint16 _num;
	Common::SharedPtr<Image> _shape;
	Common::Array<TalkDataHead> _heads;
	uint16 _val;
	Common::String _bmpFile;
};


/**
 * A scene is described by an SDS file, which points to the ADS script to load
 * and holds the dialog info.
 */
class Scene {
public:
	Scene();
	virtual ~Scene() {};

	virtual bool parse(Common::SeekableReadStream *s) = 0;

	bool isVersionOver(const char *version) const;
	bool isVersionUnder(const char *version) const;

	uint32 getMagic() const { return _magic; }
	const Common::String &getVersion() const { return _version; }

	bool runPreTickOps() { return runOps(_preTickOps); }
	bool runPostTickOps() { return runOps(_postTickOps); }

	static bool runOps(const Common::Array<SceneOp> ops, int16 addMinutes = 0);

	virtual Common::Error syncState(Common::Serializer &s) = 0;

protected:
	bool readConditionList(Common::SeekableReadStream *s, Common::Array<SceneConditions> &list) const;
	bool readHotArea(Common::SeekableReadStream *s, HotArea &dst) const;
	bool readHotAreaList(Common::SeekableReadStream *s, Common::List<HotArea> &list) const;
	bool readGameItemList(Common::SeekableReadStream *s, Common::Array<GameItem> &list) const;
	bool readMouseHotspotList(Common::SeekableReadStream *s, Common::Array<MouseCursor> &list) const;
	bool readObjInteractionList(Common::SeekableReadStream *s, Common::Array<ObjectInteraction> &list) const;
	bool readOpList(Common::SeekableReadStream *s, Common::Array<SceneOp> &list) const;
	bool readDialogList(Common::SeekableReadStream *s, Common::Array<Dialog> &list, int16 filenum = 0) const;
	bool readTriggerList(Common::SeekableReadStream *s, Common::Array<SceneTrigger> &list) const;
	bool readDialogActionList(Common::SeekableReadStream *s, Common::Array<DialogAction> &list) const;
	bool readConditionalSceneOpList(Common::SeekableReadStream *s, Common::Array<ConditionalSceneOp> &list) const;

	static void segmentStateOps(const Common::Array<uint16> &args);
	static void setItemAttrOp(const Common::Array<uint16> &args);
	static void setDragItemOp(const Common::Array<uint16> &args);

	// These are all static as they are potentially run over scene changes.
	static bool checkConditions(const Common::Array<SceneConditions> &cond);
	static bool runSceneOp(const SceneOp &op);
	static bool runDragonOp(const SceneOp &op);
	static bool runChinaOp(const SceneOp &op);
	static bool runBeamishOp(const SceneOp &op);

	uint32 _magic;
	Common::String _version;

	Common::Array<SceneOp> _preTickOps;
	Common::Array<SceneOp> _postTickOps;
	Common::Array<ConditionalSceneOp> _conditionalOps; // Beamish only
};


class GDSScene : public Scene {
public:
	GDSScene();

	bool load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor);
	bool loadRestart(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor);
	bool parse(Common::SeekableReadStream *s) override;
	bool parseInf(Common::SeekableReadStream *s);
	const Common::String &getIconFile() const { return _iconFile; }
	bool readPerSceneGlobals(Common::SeekableReadStream *s);

	Common::String dump(const Common::String &indent) const;

	void runStartGameOps() { runOps(_startGameOps); }
	void runQuitGameOps() { runOps(_quitGameOps); }
	void runChangeSceneOps() { runOps(_onChangeSceneOps); }
	void globalOps(const Common::Array<uint16> &args);
	int16 getGlobal(uint16 num);
	int16 setGlobal(uint16 num, int16 val);

	const Common::Array<MouseCursor> &getCursorList() const { return _cursorList; }
	void drawItems(Graphics::ManagedSurface &surf);
	Common::Array<GameItem> &getGameItems() { return _gameItems; }
	int countItemsInScene2() const;

	const Common::Array<ObjectInteraction> &getObjInteractions1() { return _objInteractions1; }
	const Common::Array<ObjectInteraction> &getObjInteractions2() { return _objInteractions2; }

	Common::Error syncState(Common::Serializer &s) override;
	void initIconSizes();
	GameItem *getActiveItem();

	uint16 getDefaultMouseCursor() const { return _defaultMouseCursor; }
	uint16 getInvIconNum() const { return _invIconNum; }
	uint16 getInvIconMouseCursor() const { return _invIconMouseCursor; }

private:
	Common::String _iconFile;
	Common::Array<GameItem> _gameItems;
	Common::Array<SceneOp> _startGameOps;
	Common::Array<SceneOp> _quitGameOps;
	Common::Array<SceneOp> _onChangeSceneOps;
	Common::Array<MouseCursor> _cursorList;
	Common::Array<PerSceneGlobal> _perSceneGlobals;
	Common::Array<ObjectInteraction> _objInteractions2;
	Common::Array<ObjectInteraction> _objInteractions1;

	// Additional fields that appear in Willy Beamish (unused in others)
	uint16 _defaultMouseCursor;
	uint16 _field3a;
	uint16 _invIconNum;
	uint16 _invIconMouseCursor;
	uint16 _field40;
};

class SDSScene : public Scene {
public:
	SDSScene();

	bool load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor);
	bool parse(Common::SeekableReadStream *s) override;
	void unload();

	const Common::String &getAdsFile() const { return _adsFile; }
	void runEnterSceneOps() { runOps(_enterSceneOps); }
	void runLeaveSceneOps() { runOps(_leaveSceneOps); }
	void checkTriggers();

	int getNum() const { return _num; }
	Common::String dump(const Common::String &indent) const;

	bool checkDialogActive();
	void drawActiveDialogBgs(Graphics::ManagedSurface *dst);
	bool drawAndUpdateDialogs(Graphics::ManagedSurface *dst);
	bool checkForClearedDialogs();

	void mouseMoved(const Common::Point &pt);
	void mouseLDown(const Common::Point &pt);
	void mouseLUp(const Common::Point &pt);
	void mouseRDown(const Common::Point &pt);
	void mouseRUp(const Common::Point &pt);

	void addInvButtonToHotAreaList();
	void removeInvButtonFromHotAreaList();

	const Common::List<HotArea> &getHotAreas() const { return _hotAreaList; }

	const GameItem *getDragItem() const { return _dragItem; }
	GameItem *getDragItem() { return _dragItem; }
	void setDragItem(GameItem *item) { _dragItem = item; }

	const Common::Array<ObjectInteraction> &getObjInteractions1() { return _objInteractions1; }
	const Common::Array<ObjectInteraction> &getObjInteractions2() { return _objInteractions2; }

	bool hasVisibleDialog();
	bool hasVisibleOrOpeningDialog() const;

	Common::Error syncState(Common::Serializer &s) override;

	void onDragFinish(const Common::Point &pt);
	void enableTrigger(uint16 num, bool enable = true);

	Dialog *loadDialogData(uint16 num);
	void freeDialogData(uint16 num);
	bool loadTalkData(uint16 num);
	void freeTalkData(uint16 num);
	void updateVisibleTalkers();
	void loadTalkDataAndSetFlags(uint16 talknum, uint16 headnum);
	void drawVisibleHeads(Graphics::ManagedSurface *dst);

	// dragon-specific scene ops
	void addAndShowTiredDialog();
	void sceneOpUpdatePasscodeGlobal();

	void prevChoice();
	void nextChoice();
	void activateChoice();
	bool isTriggerEnabled(uint16 num);
	bool isLButtonDown() const { return _lbuttonDown; }
	bool isRButtonDown() const { return _rbuttonDown; }
	void showDialog(uint16 fileNum, uint16 dlgNum);
	const Common::Array<ConditionalSceneOp> &getConditionalOps() { return _conditionalOps; }

protected:
	HotArea *findAreaUnderMouse(const Common::Point &pt);

private:
	Dialog *getVisibleDialog();
	bool readTalkData(Common::SeekableReadStream *s, TalkData &dst);
	void updateHead(TalkDataHead &head);
	void drawHead(Graphics::ManagedSurface *dst, const TalkData &data, const TalkDataHead &head);
	void drawHeadType1(Graphics::ManagedSurface *dst, const TalkDataHead &head, const Image &img);
	void drawHeadType2(Graphics::ManagedSurface *dst, const TalkDataHead &head, const Image &img);
	void drawHeadType3(Graphics::ManagedSurface *dst, const TalkDataHead &head, const Image &img);

	int _num;
	Common::Array<SceneOp> _enterSceneOps;
	Common::Array<SceneOp> _leaveSceneOps;
	//uint _field5_0x12;
	uint _field6_0x14;
	Common::String _adsFile;
	//uint _field8_0x23;
	Common::List<HotArea> _hotAreaList;
	Common::Array<ObjectInteraction> _objInteractions1;
	Common::Array<ObjectInteraction> _objInteractions2;
	//uint _field12_0x2b;
	//uint _field15_0x33;

	Common::Array<TalkData> _talkData;

	// From here on is mutable stuff that might need saving
	Common::Array<Dialog> _dialogs;
	Common::Array<SceneTrigger> _triggers;

	GameItem *_dragItem;
	bool _shouldClearDlg;
	bool _ignoreMouseUp;
	bool _lbuttonDown;
	bool _rbuttonDown;

	static bool _dlgWithFlagLo8IsClosing;
	static DialogFlags _sceneDialogFlags;
};

} // End of namespace Dgds

#endif // DGDS_SCENE_H
