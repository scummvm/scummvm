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

#ifndef DGDS_SCENE_H
#define DGDS_SCENE_H

#include "common/stream.h"
#include "common/array.h"
#include "common/serializer.h"

#include "dgds/dialog.h"
#include "dgds/dgds_rect.h"

namespace Dgds {

class ResourceManager;
class Decompressor;
class Font;

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
	kSceneOpOpenChinaOpenGameOverMenu = 114,	// args: none.
	kSceneOpOpenChinaOpenSkipCreditsMenu = 115,	// args: none.
	kSceneOpOpenChinaStartIntro = 116,	// args: none.
};

class SceneOp {
public:
	Common::Array<SceneConditions> _conditionList;
	Common::Array<uint16> _args;
	SceneOpCode _opCode;

	Common::String dump(const Common::String &indent) const;
};

class GameItem : public HotArea {
public:
	Common::Array<SceneOp> onDragFinishedOps;
	Common::Array<SceneOp> opList5;
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
	SceneTrigger(uint16 num) : _num(num), _enabled(false) {}
	Common::String dump(const Common::String &indent) const;

	Common::Array<SceneConditions> conditionList;
	Common::Array<SceneOp> sceneOpList;
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
	PerSceneGlobal(uint16 num, uint16 scene) : _num(num), _sceneNo(scene) {}

	Common::String dump(const Common::String &indent) const;
	bool matches(uint16 num, uint16 scene) const { return num == _num && (_sceneNo == 0 || _sceneNo == scene); }

	int16 _val;

private:
	// Immutable, read from the data file
	uint16 _num;
	uint16 _sceneNo;
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
	void runPreTickOps() { runOps(_preTickOps); }
	void runPostTickOps() { runOps(_postTickOps); }

	void mouseMoved(const Common::Point pt);
	void mouseClicked(const Common::Point pt);

	bool runOps(const Common::Array<SceneOp> &ops, int16 addMinutes = 0);
	virtual Common::Error syncState(Common::Serializer &s) = 0;
	virtual void enableTrigger(uint16 numm, bool enable = true) {}

protected:
	bool readConditionList(Common::SeekableReadStream *s, Common::Array<SceneConditions> &list) const;
	bool readHotArea(Common::SeekableReadStream *s, HotArea &dst) const;
	bool readHotAreaList(Common::SeekableReadStream *s, Common::List<HotArea> &list) const;
	bool readGameItemList(Common::SeekableReadStream *s, Common::Array<GameItem> &list) const;
	bool readMouseHotspotList(Common::SeekableReadStream *s, Common::Array<MouseCursor> &list) const;
	bool readObjInteractionList(Common::SeekableReadStream *s, Common::Array<ObjectInteraction> &list) const;
	bool readOpList(Common::SeekableReadStream *s, Common::Array<SceneOp> &list) const;
	bool readDialogList(Common::SeekableReadStream *s, Common::Array<Dialog> &list) const;
	bool readTriggerList(Common::SeekableReadStream *s, Common::Array<SceneTrigger> &list) const;
	bool readDialogActionList(Common::SeekableReadStream *s, Common::Array<DialogAction> &list) const;

	bool checkConditions(const Common::Array<SceneConditions> &cond) const;

	virtual void showDialog(uint16 num) {}
	virtual void globalOps(const Common::Array<uint16> &args) {}
	virtual void segmentStateOps(const Common::Array<uint16> &args);

	void setItemAttrOp(const Common::Array<uint16> &args);
	void setDragItemOp(const Common::Array<uint16> &args);

	void drawDragonCountdown1();
	void drawDragonCountdown2();

	bool runSceneOp(const SceneOp &op);
	bool runDragonOp(const SceneOp &op);
	bool runChinaOp(const SceneOp &op);

	uint32 _magic;
	Common::String _version;

	Common::Array<SceneOp> _preTickOps;
	Common::Array<SceneOp> _postTickOps;
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
	void globalOps(const Common::Array<uint16> &args) override;
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

private:
	Common::String _iconFile;
	Common::Array<GameItem> _gameItems;
	Common::Array<SceneOp> _startGameOps;
	Common::Array<SceneOp> _quitGameOps;
	Common::Array<SceneOp> _onChangeSceneOps;
	Common::Array<MouseCursor> _cursorList;
	Common::Array<PerSceneGlobal> _perSceneGlobals;
	Common::Array<ObjectInteraction> _objInteractions1;
	Common::Array<ObjectInteraction> _objInteractions2;
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

	void globalOps(const Common::Array<uint16> &args) override;

	void mouseMoved(const Common::Point &pt);
	void mouseLDown(const Common::Point &pt);
	void mouseLUp(const Common::Point &pt);
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
	void enableTrigger(uint16 num, bool enable = true) override;

	// dragon-specific scene ops
	void addAndShowTiredDialog();
	void sceneOpUpdatePasscodeGlobal();

protected:
	HotArea *findAreaUnderMouse(const Common::Point &pt);

private:
	void showDialog(uint16 num) override;
	Dialog *getVisibleDialog();

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

	// From here on is mutable stuff that might need saving
	Common::Array<Dialog> _dialogs;
	Common::Array<SceneTrigger> _triggers;

	GameItem *_dragItem;
	bool _shouldClearDlg;
	bool _ignoreMouseUp;

	static bool _dlgWithFlagLo8IsClosing;
	static DialogFlags _sceneDialogFlags;
};

} // End of namespace Dgds

#endif // DGDS_SCENE_H
