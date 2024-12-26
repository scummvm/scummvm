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
#include "dgds/head.h"
#include "dgds/dgds_rect.h"
#include "dgds/minigames/shell_game.h"
#include "dgds/scene_condition.h"
#include "dgds/scene_op.h"

namespace Dgds {

class ResourceManager;
class Decompressor;
class DgdsFont;
class SoundRaw;
class TTMInterpreter;
class TTMEnviro;


class HotArea {
public:
	DgdsRect _rect;
	uint16 _num; //
	uint16 _cursorNum;

	// Used in Willy Beamish
	uint16 _cursorNum2;
	uint16 _objInteractionRectNum;

	Common::Array<SceneConditions> enableConditions;
	Common::Array<SceneOp> onRClickOps;
	Common::Array<SceneOp> onLDownOps;
	Common::Array<SceneOp> onLClickOps;

	virtual ~HotArea() {}

	virtual Common::String dump(const Common::String &indent) const;
};

class DynamicRect {
public:
	DynamicRect() : _num(0) {};
	uint16 _num;
	DgdsRect _rect;
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
		return (_droppedItemNum == 0xFFFF || _droppedItemNum == droppedItemNum)
				&& _targetItemNum == targetItemNum;
	}

	Common::String dump(const Common::String &indent) const;

private:
	uint16 _droppedItemNum;
	uint16 _targetItemNum;

};

class SceneTrigger {
public:
	SceneTrigger(uint16 num) : _num(num), _enabled(false), _timesToCheckBeforeRunning(0), _checksUntilRun(0) {}
	Common::String dump(const Common::String &indent) const;

	Common::Array<SceneConditions> conditionList;
	Common::Array<SceneOp> sceneOpList;

	uint16 _timesToCheckBeforeRunning; // Only used in Beamish.
	uint16 _checksUntilRun;
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

	// These are all static as they are potentially run over scene changes.
	static bool checkConditions(const Common::Array<SceneConditions> &cond);

	static void segmentStateOps(const Common::Array<uint16> &args);
	static void setItemAttrOp(const Common::Array<uint16> &args);
	static void setDragItemOp(const Common::Array<uint16> &args);

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
	int16 getGlobal(uint16 num) const;
	int16 setGlobal(uint16 num, int16 val);

	const Common::Array<MouseCursor> &getCursorList() const { return _cursorList; }
	void drawItems(Graphics::ManagedSurface &surf);
	Common::Array<GameItem> &getGameItems() { return _gameItems; }
	int countItemsInInventory() const;

	const Common::Array<ObjectInteraction> &getObjInteractions1() { return _objInteractions1; }
	const Common::Array<ObjectInteraction> &getObjInteractions2() { return _objInteractions2; }

	Common::Error syncState(Common::Serializer &s) override;
	void initIconSizes();
	GameItem *getActiveItem();

	int16 getDefaultMouseCursor() const { return _defaultMouseCursor; }
	int16 getDefaultMouseCursor2() const { return _defaultMouseCursor2; }
	int16 getOtherDefaultMouseCursor() const { return _defaultOtherMouseCursor; }
	uint16 getInvIconNum() const { return _invIconNum; }
	int16 getInvIconMouseCursor() const { return _invIconMouseCursor; }

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
	int16 _defaultMouseCursor;
	int16 _defaultMouseCursor2;
	uint16 _invIconNum;
	int16 _invIconMouseCursor;
	int16 _defaultOtherMouseCursor;
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
	void enableTrigger(uint16 sceneNum, uint16 num, bool enable = true);

	Dialog *loadDialogData(uint16 num);
	void freeDialogData(uint16 num);
	bool loadTalkData(uint16 num);
	void freeTalkData(uint16 num);
	void updateVisibleTalkers();
	void loadTalkDataAndSetFlags(uint16 talknum, uint16 headnum);
	void drawVisibleHeads(Graphics::ManagedSurface *dst);
	bool hasVisibleHead() const;

	// dragon-specific scene ops
	void addAndShowTiredDialog();

	void prevChoice();
	void nextChoice();
	void activateChoice();
	bool isTriggerEnabled(uint16 num);
	bool isLButtonDown() const { return _lbuttonDown; }
	bool isRButtonDown() const { return _rbuttonDown; }
	void showDialog(uint16 fileNum, uint16 dlgNum);
	const Common::Array<ConditionalSceneOp> &getConditionalOps() { return _conditionalOps; }
	void updateHotAreasFromDynamicRects();
	void setDynamicSceneRect(int16 num, int16 x, int16 y, int16 width, int16 height);
	void setSceneNum(int16 num) { _num = num; }
	void drawDebugHotAreas(Graphics::ManagedSurface *dst) const;

protected:
	HotArea *findAreaUnderMouse(const Common::Point &pt);

private:
	Dialog *getVisibleDialog();
	bool readTalkData(Common::SeekableReadStream *s, TalkData &dst);

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
	Common::Array<DynamicRect> _dynamicRects; // Only used in Willy Beamish
	//uint _field12_0x2b;
	//uint _field15_0x33;

	Common::Array<TalkData> _talkData;

	// From here on is mutable stuff that might need saving
	Common::Array<Dialog> _dialogs;
	Common::Array<SceneTrigger> _triggers;
	Conversation _conversation;

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
