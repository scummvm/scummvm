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

#include "lastexpress/game/action.h"

#include "lastexpress/data/animation.h"
#include "lastexpress/data/cursor.h"
#include "lastexpress/data/snd.h"
#include "lastexpress/data/scene.h"

#include "lastexpress/entities/abbot.h"
#include "lastexpress/entities/anna.h"

#include "lastexpress/game/beetle.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

static const int _animationListSize = 273;

// List of animations
static const struct {
	const char *filename;
	uint16 time;
} _animationList[_animationListSize] = {
	{"", 0},
	{"1002",    255},
	{"1002D",   255},
	{"1003",    0},
	{"1005",    195},
	{"1006",    750},   // 5
	{"1006A",   750},
	{"1008",    765},
	{"1008N",   765},
	{"1008A",   750},
	{"1008AN",  750},   // 10
	{"1009",    0},
	{"1011",    1005},
	{"1011A",   780},
	{"1012",    300},
	{"1013",    285},
	{"1017",    870},   // 15
	{"1017A",   0},     // Not in the data files?
	{"1019",    120},
	{"1019D",   120},
	{"1020",    120},   // 20
	{"1022",    525},
	{"1022A",   180},
	{"1022AD",  210},
	{"1022B",   210},
	{"1022C",   210},   // 25
	{"1023",    135},
	{"1025",    945},
	{"1028",    300},
	{"1030",    390},
	{"1031",    375},   // 30
	{"1032",    1050},
	{"1033",    945},
	{"1034",    495},
	{"1035",    1230},
	{"1037",    1425},  // 35
	{"1038",    195},
	{"1038A",   405},
	{"1039",    600},
	{"1040",    945},
	{"1041",    510},   // 40
	{"1042",    540},
	{"1043",    855},
	{"1044",    645},
	{"1046",    0},
	{"1047",    0},     // 45
	{"1047A",   0},
	{"1059",    1005},
	{"1060",    255},
	{"1063",    0},
	{"1101",    255},   // 50
	{"1102",    1320},
	{"1103",    210},
	{"1104",    120},
	{"1105",    1350},
	{"1106",    315},   // 55
	{"1106A",   315},
	{"1106D",   315},
	{"1107",    1},
	{"1107A",   660},
	{"1108",    300},   // 60
	{"1109",    1305},
	{"1110",    300},
	{"1112",    0},
	{"1115",    0},
	{"1115A",   0},     // 65
	{"1115B",   0},
	{"1115C",   0},
	{"1115D",   0},
	{"1115E",   0},
	{"1115F",   0},     // 70
	{"1115G",   0},
	{"1115H",   0},
	{"1116",    0},
	{"1117",    0},
	{"1118",    105},   // 75
	{"1202",    510},
	{"1202A",   510},
	{"1203",    720},
	{"1204",    120},
	{"1205",    465},   // 80
	{"1206",    690},
	{"1206A",   450},
	{"1208",    465},
	{"1210",    1020},
	{"1211",    600},   // 85
	{"1212",    435},
	{"1213",    525},
	{"1213A",   150},
	{"1215",    390},
	{"1216",    0},     // 90
	{"1219",    240},
	{"1222",    1095},
	{"1223",    0},
	{"1224",    720},
	{"1225",    1005},  // 95
	{"1227",    840},
	{"1227A",   840},
	{"1303",    450},
	{"1303N",   450},
	{"1304",    450},   // 100
	{"1304N",   450},
	{"1305",    630},
	{"1309",    0},
	{"1311",    1710},
	{"1312",    240},   // 105
	{"1312D",   240},
	{"1313",    930},
	{"1315",    1035},
	{"1315A",   1035},
	{"1401",    540},   // 110
	{"1402",    150},
	{"1402B",   150},
	{"1403",    90},
	{"1404",    885},
	{"1404A",   0},     // 115
	{"1405",    135},
	{"1406",    1665},
	{"1501",    285},
	{"1501A",   285},
	{"1502",    165},   // 120
	{"1502A",   165},
	{"1502D",   165},
	{"1503",    0},
	{"1504",    0},
	{"1505",    0},     // 125
	{"1505A",   0},
	{"1506",    300},
	{"1506A",   180},
	{"1508",    0},
	{"1509",    450},   // 130
	{"1509S",   450},
	{"1509A",   450},
	{"1509AS",  450},
	{"1509N",   450},
	{"1509SN",  450},   // 135
	{"1509AN",  450},
	{"1509BN",  450},
	{"1511",    150},
	{"1511A",   150},
	{"1511B",   90},    // 140
	{"1511BA",  90},
	{"1511C",   135},
	{"1511D",   105},
	{"1930",    0},
	{"1511E",   150},   // 145
	{"1512",    165},
	{"1513",    180},
	{"1517",    0},
	{"1517A",   165},
	{"1518",    165},   // 150
	{"1518A",   165},
	{"1518B",   165},
	{"1591",    450},
	{"1592",    450},
	{"1593",    450},   // 155
	{"1594",    450},
	{"1595",    450},
	{"1596",    450},
	{"1601",    0},
	{"1603",    0},     // 160
	{"1606B",   315},
	{"1607A",   0},
	{"1610",    0},
	{"1611",    0},
	{"1612",    0},     // 165
	{"1615",    0},
	{"1619",    0},
	{"1620",    120},
	{"1621",    105},
	{"1622",    105},   // 170
	{"1629",    450},
	{"1630",    450},
	{"1631",    525},
	{"1632",    0},
	{"1633",    615},   // 175
	{"1634",    180},
	{"1702",    180},
	{"1702DD",  180},
	{"1702NU",  180},
	{"1702ND",  180},   // 180
	{"1704",    300},
	{"1704D",   300},
	{"1705",    195},
	{"1705D",   195},
	{"1706",    195},   // 185
	{"1706DD",  195},
	{"1706ND",  195},
	{"1706NU",  195},
	{"1901",    135},
	{"1902",    1410},  // 190
	{"1903",    0},
	{"1904",    1920},
	{"1908",    600},
	{"1908A",   195},
	{"1908B",   105},   // 195
	{"1908C",   165},
	{"1908CD",  0},
	{"1909A",   150},
	{"1909B",   150},
	{"1909C",   150},   // 200
	{"1910A",   180},
	{"1910B",   180},
	{"1910C",   180},
	{"1911A",   90},
	{"1911B",   90},    // 205
	{"1911C",   90},
	{"1912",    0},
	{"1913",    0},
	{"1917",    0},
	{"1918",    390},   // 210
	{"1919",    360},
	{"1919A",   105},
	{"1920",    75},
	{"1922",    75},
	{"1923",    150},   // 215
	{"8001",    120},
	{"8001A",   120},
	{"8002",    120},
	{"8002A",   120},
	{"8002B",   120},   // 220
	{"8003",    105},
	{"8003A",   105},
	{"8004",    105},
	{"8004A",   105},
	{"8005",    270},   // 225
	{"8005B",   270},
	{"8010",    270},
	{"8013",    120},
	{"8013A",   120},
	{"8014",    165},   // 230
	{"8014A",   165},
	{"8014R",   165},
	{"8014AR",  165},
	{"8015",    150},
	{"8015A",   150},   // 235
	{"8015R",   150},
	{"8015AR",  150},
	{"8017",    120},
	{"8017A",   120},
	{"8017R",   120},   // 240
	{"8017AR",  120},
	{"8017N",   90},
	{"8023",    135},
	{"8023A",   135},
	{"8023M",   135},   // 245
	{"8024",    150},
	{"8024A",   180},
	{"8024M",   180},
	{"8025",    150},
	{"8025A",   150},   // 250
	{"8025M",   150},
	{"8027",    75},
	{"8028",    75},
	{"8029",    120},
	{"8029A",   120},   // 255
	{"8031",    375},
	{"8032",    0},
	{"8032A",   0},
	{"8033",    105},
	{"8035",    195},   // 260
	{"8035A",   120},
	{"8035B",   180},
	{"8035C",   135},
	{"8036",    105},
	{"8037",    195},   // 265
	{"8037A",   195},
	{"8040",    240},
	{"8040A",   240},
	{"8041",    195},
	{"8041A",   195},   // 270
	{"8042",    600},
	{"8042A",   600}
};

template<class Arg, class Res, class T>
class Functor1MemConst : public Common::Functor1<Arg, Res> {
public:
	typedef Res (T::*FuncType)(Arg) const;

	Functor1MemConst(T *t, const FuncType &func) : _t(t), _func(func) {}

	bool isValid() const override { return _func != nullptr && _t != nullptr; }
	Res operator()(Arg v1) const override {
		return (_t->*_func)(v1);
	}
private:
	mutable T *_t;
	const FuncType _func;
};

Action::Action(LastExpressEngine *engine) : _engine(engine) {
	ADD_ACTION(dummy);
	ADD_ACTION(inventory);
	ADD_ACTION(savePoint);
	ADD_ACTION(playSound);
	ADD_ACTION(playMusic);
	ADD_ACTION(knock);                    // 5
	ADD_ACTION(compartment);
	ADD_ACTION(playSounds);
	ADD_ACTION(playAnimation);
	ADD_ACTION(openCloseObject);
	ADD_ACTION(setModel);                 // 10
	ADD_ACTION(setItem);
	ADD_ACTION(knockInside);
	ADD_ACTION(pickItem);
	ADD_ACTION(dropItem);
	ADD_ACTION(dummy);                    // 15
	ADD_ACTION(enterCompartment);
	ADD_ACTION(dummy);
	ADD_ACTION(leanOutWindow);
	ADD_ACTION(almostFall);
	ADD_ACTION(climbInWindow);           // 20
	ADD_ACTION(climbLadder);
	ADD_ACTION(climbDownTrain);
	ADD_ACTION(kronosSanctum);
	ADD_ACTION(escapeBaggage);
	ADD_ACTION(enterBaggage);            // 25
	ADD_ACTION(bombPuzzle);
	ADD_ACTION(27);
	ADD_ACTION(kronosConcert);
	ADD_ACTION(29);
	ADD_ACTION(catchBeetle);              // 30
	ADD_ACTION(exitCompartment);
	ADD_ACTION(outsideTrain);
	ADD_ACTION(firebirdPuzzle);
	ADD_ACTION(openMatchBox);
	ADD_ACTION(openBed);                 // 35
	ADD_ACTION(dummy);
	ADD_ACTION(dialog);
	ADD_ACTION(eggBox);
	ADD_ACTION(39);
	ADD_ACTION(bed);                     // 40
	ADD_ACTION(playMusicChapter);
	ADD_ACTION(playMusicChapterSetupTrain);
	ADD_ACTION(switchChapter);
	ADD_ACTION(44);
}

Action::~Action() {
	for (uint i = 0; i < _actions.size(); i++)
		SAFE_DELETE(_actions[i]);

	_actions.clear();

	// Zero-out passed pointers
	_engine = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Processing hotspot
//////////////////////////////////////////////////////////////////////////
SceneIndex Action::processHotspot(const SceneHotspot &hotspot) {
	if (!hotspot.action || hotspot.action >= (int)_actions.size())
		return kSceneInvalid;

	return (*_actions[hotspot.action])(hotspot);
}

//////////////////////////////////////////////////////////////////////////
// Actions
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Action 0
IMPLEMENT_ACTION(dummy)
	error("[Action::action_dummy] Dummy action function called (hotspot action: %d)", hotspot.action);
}

//////////////////////////////////////////////////////////////////////////
// Action 1
IMPLEMENT_ACTION(inventory)
	if (!getState()->sceneUseBackup)
		return kSceneInvalid;

	SceneIndex index = kSceneNone;
	if (getState()->sceneBackup2) {
		index = getState()->sceneBackup2;
		getState()->sceneBackup2 = kSceneNone;
	} else {
		getState()->sceneUseBackup = false;
		index = getState()->sceneBackup;

		Scene *backup = getScenes()->get(getState()->sceneBackup);
		if (getEntities()->getPosition(backup->car, backup->position))
			index = getScenes()->processIndex(getState()->sceneBackup);
	}

	getScenes()->loadScene(index);

	if (!getInventory()->getSelectedItem())
		return kSceneInvalid;

	if (!getInventory()->getSelectedEntry()->isSelectable || (!getState()->sceneBackup2 && getInventory()->getFirstExaminableItem()))
		getInventory()->selectItem(getInventory()->getFirstExaminableItem());

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 2
IMPLEMENT_ACTION(savePoint)
	getSavePoints()->push(kCharacterCath, (CharacterIndex)hotspot.param1, (CharacterActions)hotspot.param2);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 3
IMPLEMENT_ACTION(playSound)

	// Check that the file is not already buffered
	if (hotspot.param2 || !getSoundQueue()->isBuffered(Common::String::format("LIB%03d", hotspot.param1), true))
		getSound()->playSoundEvent(kCharacterCath, hotspot.param1, hotspot.param2);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 4
IMPLEMENT_ACTION(playMusic)
	// Check that the file is not already buffered
	Common::String filename = Common::String::format("MUS%03d", hotspot.param1);

	if (!getSoundQueue()->isBuffered(filename) && (hotspot.param1 != 50 || getProgress().chapter == kChapter5))
		getSound()->playSound(kCharacterCath, filename, kVolumeFull, hotspot.param2);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 5
IMPLEMENT_ACTION(knock)
	ObjectIndex object = (ObjectIndex)hotspot.param1;
	if (object >= kObjectMax)
		return kSceneInvalid;

	if (getObjects()->get(object).entity) {
		getSavePoints()->push(kCharacterCath, getObjects()->get(object).entity, kCharacterActionKnock, object);
	} else {
		if (!getSoundQueue()->isBuffered("LIB012", true))
			getSound()->playSoundEvent(kCharacterCath, 12);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 6
IMPLEMENT_ACTION(compartment)
	ObjectIndex compartment = (ObjectIndex)hotspot.param1;

	if (compartment >= kObjectMax)
		return kSceneInvalid;

	if (getObjects()->get(compartment).entity) {
		getSavePoints()->push(kCharacterCath, getObjects()->get(compartment).entity, kCharacterActionOpenDoor, compartment);

		// Stop processing further
		return kSceneNone;
	}

	if (handleOtherCompartment(compartment, true, true)) {
		// Stop processing further
		return kSceneNone;
	}

	ObjectLocation location = getObjects()->get(compartment).status;
	if (location == kObjectLocation1 || location == kObjectLocation3 || getEntities()->checkFields2(compartment)) {

		if (location != kObjectLocation1 || getEntities()->checkFields2(compartment)
		 || (getInventory()->getSelectedItem() != kItemKey
		 && (compartment != kObjectCompartment1
		  || !getInventory()->hasItem(kItemKey)
		  || (getInventory()->getSelectedItem() != kItemFirebird && getInventory()->getSelectedItem() != kItemBriefcase)))) {
			if (!getSoundQueue()->isBuffered("LIB13"))
				getSound()->playSoundEvent(kCharacterCath, 13);

			// Stop processing further
			return kSceneNone;
		}

		getSound()->playSoundEvent(kCharacterCath, 32);

		if ((compartment >= kObjectCompartment1 && compartment <= kObjectCompartment3) || (compartment >= kObjectCompartmentA && compartment <= kObjectCompartmentF))
			getObjects()->update(compartment, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		getSound()->playSoundEvent(kCharacterCath, 15, 22);
		getInventory()->unselectItem();

		return kSceneInvalid;
	}

	if (hotspot.action != SceneHotspot::kActionEnterCompartment || getInventory()->getSelectedItem() != kItemKey) {
		if (compartment == kObjectCageMax) {
			getSound()->playSoundEvent(kCharacterCath, 26);
		} else {
			getSound()->playSoundEvent(kCharacterCath, 14);
			getSound()->playSoundEvent(kCharacterCath, 15, 22);
		}
		return kSceneInvalid;
	}

	getObjects()->update(kObjectCompartment1, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
	getSound()->playSoundEvent(kCharacterCath, 16);
	getInventory()->unselectItem();

	// Stop processing further
	return kSceneNone;
}

//////////////////////////////////////////////////////////////////////////
// Action 7
IMPLEMENT_ACTION(playSounds)
	getSound()->playSoundEvent(kCharacterCath, hotspot.param1);
	getSound()->playSoundEvent(kCharacterCath, hotspot.param3, hotspot.param2);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 8
IMPLEMENT_ACTION(playAnimation)
	if (HELPERgetEvent(hotspot.param1))
		return kSceneInvalid;

	playAnimation((EventIndex)hotspot.param1);

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 9
IMPLEMENT_ACTION(openCloseObject)
	ObjectIndex object = (ObjectIndex)hotspot.param1;
	ObjectLocation location = (ObjectLocation)hotspot.param2;

	if (object >= kObjectMax)
		return kSceneInvalid;

	getObjects()->update(object, getObjects()->get(object).entity, location, kCursorKeepValue, kCursorKeepValue);

	bool isNotWindow = ((object <= kObjectCompartment8  || object >= kObjectHandleBathroom) && (object <= kObjectCompartmentH || object >= kObject48));

	switch (location) {
	default:
		break;

	case kObjectLocation1:
		if (isNotWindow)
			getSound()->playSoundEvent(kCharacterCath, 24);
		else
			getSound()->playSoundEvent(kCharacterCath, 21);
		break;

	case kObjectLocation2:
		if (isNotWindow)
			getSound()->playSoundEvent(kCharacterCath, 36);
		else
			getSound()->playSoundEvent(kCharacterCath, 20);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 10
IMPLEMENT_ACTION(setModel)
	ObjectIndex object = (ObjectIndex)hotspot.param1;
	ObjectModel model = (ObjectModel)hotspot.param2;

	if (object >= kObjectMax)
		return kSceneInvalid;

	getObjects()->updateModel(object, model);

	if (object != kObject112 || getSoundQueue()->isBuffered("LIB096")) {
		if (object == 1)
			getSound()->playSoundEvent(kCharacterCath, 73);
	} else {
		getSound()->playSoundEvent(kCharacterCath, 96);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 11
IMPLEMENT_ACTION(setItem)
	InventoryItem item = (InventoryItem)hotspot.param1;
	if (item >= kPortraitOriginal)
		return kSceneInvalid;

	Inventory::InventoryEntry *entry = getInventory()->get(item);
	if (entry->inPocket)
		return kSceneInvalid;

	entry->location = (ObjectLocation)hotspot.param2;

	if (item == kItemCorpse) {
		ObjectLocation corpseLocation = getInventory()->get(kItemCorpse)->location;
		getProgress().eventCorpseMovedFromFloor = (corpseLocation == kObjectLocation3 || corpseLocation == kObjectLocation4);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 12
IMPLEMENT_ACTION(knockInside)
	ObjectIndex object = (ObjectIndex)hotspot.param1;
	if (object >= kObjectMax)
		return kSceneInvalid;

	if (getObjects()->get(object).entity)
		getSavePoints()->push(kCharacterCath, getObjects()->get(object).entity, kCharacterActionKnock, object);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 13
IMPLEMENT_ACTION(pickItem)
	InventoryItem item = (InventoryItem)hotspot.param1;
	ObjectLocation location = (ObjectLocation)hotspot.param2;
	bool process = (hotspot.scene == 0);
	SceneIndex sceneIndex = kSceneInvalid;

	if (item >= kPortraitOriginal)
		return kSceneInvalid;

	Inventory::InventoryEntry *entry = getInventory()->get(item);
	if (!entry->location)
		return kSceneInvalid;

	// Special case for corpse
	if (item == kItemCorpse) {
		pickCorpse(location, process);
		return kSceneInvalid;
	}

	// Add and process items
	getInventory()->addItem(item);

	switch (item) {
	default:
		break;

	case kItemGreenJacket:
		pickGreenJacket(process);
		break;

	case kItemScarf:
		pickScarf(process);

		// stop processing
		return kSceneInvalid;

	case kItemParchemin:
		if (location != kObjectLocation2)
			break;

		getInventory()->addItem(kItemParchemin);
		getInventory()->get(kItem11)->location = kObjectLocation1;
		getSound()->playSoundEvent(kCharacterCath, 9);
		break;

	case kItemBomb:
		RESET_ENTITY_STATE(kCharacterAbbot, Abbot, setup_catchCath);
		break;

	case kItemBriefcase:
		getSound()->playSoundEvent(kCharacterCath, 83);
		break;
	}

	// Load item scene
	if (getInventory()->get(item)->scene) {
		if (!getState()->sceneUseBackup) {
			getState()->sceneUseBackup = true;
			getState()->sceneBackup = (hotspot.scene ? hotspot.scene : getState()->scene);
		}

		getScenes()->loadScene(getInventory()->get(item)->scene);

		// do not process further
		sceneIndex = kSceneNone;
	}

	// Select item
	if (getInventory()->get(item)->isSelectable) {
		getInventory()->selectItem(item);
		_engine->getCursor()->setStyle(getInventory()->get(item)->cursor);
	}

	return sceneIndex;
}

//////////////////////////////////////////////////////////////////////////
// Action 14
IMPLEMENT_ACTION(dropItem)
	InventoryItem item = (InventoryItem)hotspot.param1;
	ObjectLocation location = (ObjectLocation)hotspot.param2;
	bool process = (hotspot.scene == kSceneNone);

	if (item >= kPortraitOriginal)
		return kSceneInvalid;

	if (!getInventory()->hasItem(item))
		return kSceneInvalid;

	if (location < kObjectLocation1)
		return kSceneInvalid;

	// Handle actions
	if (item == kItemBriefcase) {
		getSound()->playSoundEvent(kCharacterCath, 82);

		if (location == kObjectLocation2) {
			if (!getProgress().field_58) {
				getSaveLoad()->saveGame(kSavegameTypeTime, kCharacterCath, kTimeNone);
				getProgress().field_58 = 1;
			}

			if (getInventory()->get(kItemParchemin)->location == kObjectLocation2) {
				getInventory()->addItem(kItemParchemin);
				getInventory()->get(kItem11)->location = kObjectLocation1;
				getSound()->playSoundEvent(kCharacterCath, 9);
			}
		}
	}

	// Update item location
	getInventory()->removeItem(item, location);

	if (item == kItemCorpse)
		dropCorpse(process);

	// Unselect item
	getInventory()->unselectItem();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 15: Dummy action

//////////////////////////////////////////////////////////////////////////
// Action 16
IMPLEMENT_ACTION(enterCompartment)
	if (getObjects()->get(kObjectCompartment1).status == kObjectLocation1 || getObjects()->get(kObjectCompartment1).status == kObjectLocation3 || getInventory()->getSelectedItem() == kItemKey)
		return action_compartment(hotspot);

	if (getProgress().eventCorpseFound) {
		if (hotspot.action != SceneHotspot::kActionEnterCompartment || getInventory()->get(kItemBriefcase)->location != kObjectLocation2)
			return action_compartment(hotspot);

		getSound()->playSoundEvent(kCharacterCath, 14);
		getSound()->playSoundEvent(kCharacterCath, 15, 22);

		if (getProgress().field_78 && !getSoundQueue()->isBuffered("MUS003")) {
			getSound()->playSound(kCharacterCath, "MUS003", kVolumeFull);
			getProgress().field_78 = 0;
		}

		getScenes()->loadSceneFromPosition(kCarGreenSleeping, 77);

		return kSceneNone;
	}

	getSaveLoad()->saveGame(kSavegameTypeTime, kCharacterCath, kTimeNone);
	getSound()->playSound(kCharacterCath, "LIB014");
	playAnimation(kEventCathFindCorpse);
	getSound()->playSound(kCharacterCath, "LIB015");
	getProgress().eventCorpseFound = true;

	return kSceneCompartmentCorpse;
}

//////////////////////////////////////////////////////////////////////////
// Action 17: Dummy action

//////////////////////////////////////////////////////////////////////////
// Action 18
IMPLEMENT_ACTION(leanOutWindow)
	ObjectIndex object = (ObjectIndex)hotspot.param1;

	if ((HELPERgetEvent(kEventCathLookOutsideWindowDay) || HELPERgetEvent(kEventCathLookOutsideWindowNight) || getObjects()->get(kObjectCompartment1).model == kObjectModel1)
	  && getProgress().isTrainRunning
	  && (object != kObjectOutsideAnnaCompartment || (!getEntities()->isInsideCompartment(kCharacterRebecca, kCarRedSleeping, kPosition_4840) && getObjects()->get(kObjectOutsideBetweenCompartments).status == kObjectLocation2))
	  && getInventory()->getSelectedItem() != kItemFirebird
	  && getInventory()->getSelectedItem() != kItemBriefcase) {

		switch (object) {
		default:
			return kSceneInvalid;

		case kObjectOutsideTylerCompartment:
			HELPERgetEvent(kEventCathLookOutsideWindowDay) = 1;
			playAnimation(isNightOld() ? kEventCathGoOutsideTylerCompartmentNight : kEventCathGoOutsideTylerCompartmentDay);
			getProgress().field_C8 = 1;
			break;

		case kObjectOutsideBetweenCompartments:
			HELPERgetEvent(kEventCathLookOutsideWindowDay) = 1;
			playAnimation(isNightOld() ? kEventCathGoOutsideNight : kEventCathGoOutsideDay);
			getProgress().field_C8 = 1;
			break;

		case kObjectOutsideAnnaCompartment:
			HELPERgetEvent(kEventCathLookOutsideWindowDay) = 1;
			playAnimation(isNightOld() ? kEventCathGetInsideNight : kEventCathGetInsideDay);
			if (!hotspot.scene)
				getScenes()->processScene();
			break;
		}
	} else {
		if (object == kObjectOutsideTylerCompartment || object == kObjectOutsideBetweenCompartments || object == kObjectOutsideAnnaCompartment) {
			playAnimation(isNightOld() ? kEventCathLookOutsideWindowNight : kEventCathLookOutsideWindowDay);
			getScenes()->processScene();
			return kSceneNone;
		}
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 19
IMPLEMENT_ACTION(almostFall)
	switch((ObjectIndex)hotspot.param1) {
	default:
		return kSceneInvalid;

	case kObjectOutsideTylerCompartment:
		playAnimation(isNightOld() ? kEventCathSlipTylerCompartmentNight : kEventCathSlipTylerCompartmentDay);
		break;

	case kObjectOutsideBetweenCompartments:
		playAnimation(isNightOld() ? kEventCathSlipNight : kEventCathSlipDay);
		break;
	}

	getProgress().field_C8 = 0;

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 20
IMPLEMENT_ACTION(climbInWindow)
	switch ((ObjectIndex)hotspot.param1) {
	default:
		return kSceneInvalid;

	case kObjectOutsideTylerCompartment:
		playAnimation(isNightOld() ? kEventCathGetInsideTylerCompartmentNight : kEventCathGetInsideTylerCompartmentDay);
		break;

	case kObjectOutsideBetweenCompartments:
		playAnimation(isNightOld() ? kEventCathGetInsideNight : kEventCathGetInsideDay);
		break;

	case kObjectOutsideAnnaCompartment:
		playAnimation(kEventCathGettingInsideAnnaCompartment);
		break;
	}

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 21
IMPLEMENT_ACTION(climbLadder)
	byte action = hotspot.param1;

	if (action != 1 && action != 2)
		return kSceneInvalid;

	switch (getProgress().chapter) {
	default:
		break;

	case kChapter2:
	case kChapter3:
		if (action == 2)
			playAnimation(kEventCathClimbUpTrainGreenJacket);

		playAnimation(kEventCathTopTrainGreenJacket);
		break;

	case kChapter5:
		if (action == 2)
			playAnimation(getProgress().isNightTime ? kEventCathClimbUpTrainNoJacketNight : kEventCathClimbUpTrainNoJacketDay);

		playAnimation(getProgress().isNightTime ? kEventCathTopTrainNoJacketNight : kEventCathTopTrainNoJacketDay);
		break;
	}

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 22
IMPLEMENT_ACTION(climbDownTrain)
	EventIndex evt = kEventNone;
	switch (getProgress().chapter) {
	default:
		return kSceneInvalid;

	case kChapter2:
	case kChapter3:
		evt = kEventCathClimbDownTrainGreenJacket;
		break;

	case kChapter5:
		evt = (getProgress().isNightTime ? kEventCathClimbDownTrainNoJacketNight : kEventCathClimbDownTrainNoJacketDay);
		break;
	}

	playAnimation(evt);
	if (evt == kEventCathClimbDownTrainNoJacketDay)
		getSound()->playSoundEvent(kCharacterCath, 37);

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 23
IMPLEMENT_ACTION(kronosSanctum)
	switch (hotspot.param1) {
	default:
		break;

	case 1:
		getSavePoints()->push(kCharacterCath, kCharacterMaster, kCharacterActionBreakCeiling);
		break;

	case 2:
		getSavePoints()->push(kCharacterCath, kCharacterMaster, kCharacterActionJumpDownCeiling);
		break;

	case 3:
		if (getInventory()->getSelectedItem() == kItemBriefcase) {
			getInventory()->removeItem(kItemBriefcase, kObjectLocation3);
			getSound()->playSoundEvent(kCharacterCath, 82);
			getInventory()->unselectItem();
		}

		// Show animation with or without briefcase
		playAnimation((getInventory()->get(kItemBriefcase)->location - 3) ? kEventCathJumpUpCeilingBriefcase : kEventCathJumpUpCeiling);

		if (!hotspot.scene)
			getScenes()->processScene();

		break;

	case 4:
		if (getProgress().chapter == kChapter1)
			getSavePoints()->push(kCharacterCath, kCharacterKronos, kCharacterAction202621266);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 24
IMPLEMENT_ACTION(escapeBaggage)
	byte action = hotspot.param1;

	switch (action) {
	default:
		break;

	case 1:
		playAnimation(kEventCathStruggleWithBonds);
		if (hotspot.scene)
			getScenes()->processScene();
		break;

	case 2:
		playAnimation(kEventCathBurnRope);
		if (hotspot.scene)
			getScenes()->processScene();
		break;

	case 3:
		if (HELPERgetEvent(kEventCathBurnRope)) {
			playAnimation(kEventCathRemoveBonds);
			getProgress().field_84 = 0;
			getScenes()->loadSceneFromPosition(kCarBaggageRear, 89);
			return kSceneNone;
		}
		break;

	case 4:
		if (!HELPERgetEvent(kEventCathStruggleWithBonds2)) {
			playAnimation(kEventCathStruggleWithBonds2);
			getSound()->playSoundEvent(kCharacterCath, 101);
			getInventory()->setLocationAndProcess(kItemMatch, kObjectLocation2);
			if (!hotspot.scene)
				getScenes()->processScene();
		}
		break;

	case 5:
		getSavePoints()->push(kCharacterCath, kCharacterIvo, kCharacterAction192637492);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 25
IMPLEMENT_ACTION(enterBaggage)
	switch(hotspot.param1) {
	default:
		break;

	case 1:
		getSavePoints()->push(kCharacterCath, kCharacterAnna, kCharacterAction272177921);
		break;

	case 2:
		if (!getSoundQueue()->isBuffered("MUS021"))
			getSound()->playSound(kCharacterCath, "MUS021", kVolumeFull);
		break;

	case 3:
		getSound()->playSoundEvent(kCharacterCath, 43);
		if (!getInventory()->hasItem(kItemKey) && !HELPERgetEvent(kEventAnnaBaggageArgument)) {
			RESET_ENTITY_STATE(kCharacterAnna, Anna, setup_baggageFight);
			return kSceneNone;
		}
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 26
IMPLEMENT_ACTION(bombPuzzle)
	switch(hotspot.param1) {
	default:
		return kSceneInvalid;

	case 1:
		getSavePoints()->push(kCharacterCath, kCharacterMaster, kCharacterAction158610240);
		break;

	case 2:
		getSavePoints()->push(kCharacterCath, kCharacterMaster, kCharacterAction225367984);
		getInventory()->unselectItem();
		return kSceneNone;

	case 3:
		getSavePoints()->push(kCharacterCath, kCharacterMaster, kCharacterAction191001984);
		return kSceneNone;

	case 4:
		getSavePoints()->push(kCharacterCath, kCharacterMaster, kCharacterAction201959744);
		return kSceneNone;

	case 5:
		getSavePoints()->push(kCharacterCath, kCharacterMaster, kCharacterAction169300225);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 27
IMPLEMENT_ACTION(27)
	if (!getSoundQueue()->isBuffered("LIB031", true))
		getSound()->playSoundEvent(kCharacterCath, 31);

	switch (getEntityData(kCharacterCath)->car) {
	default:
		break;

	case kCarGreenSleeping:
		getSavePoints()->push(kCharacterCath, kCharacterCond1, kCharacterAction225358684, hotspot.param1);
		break;

	case kCarRedSleeping:
		getSavePoints()->push(kCharacterCath, kCharacterCond2, kCharacterAction225358684, hotspot.param1);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 28
IMPLEMENT_ACTION(kronosConcert)
	switch(hotspot.param1) {
	default:
		return kSceneInvalid;

	case 1:
		playAnimation(kEventConcertSit);
		break;

	case 2:
		playAnimation(kEventConcertCough);
		break;
	}

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 29
IMPLEMENT_ACTION(29)
	getProgress().field_C = 1;
	getSound()->playSoundEvent(kCharacterCath, hotspot.param1, hotspot.param2);

	Common::String filename = Common::String::format("MUS%03d", hotspot.param3);
	if (!getSoundQueue()->isBuffered(filename))
		getSound()->playSound(kCharacterCath, filename, kVolumeFull);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 30
IMPLEMENT_ACTION(catchBeetle)
	if (!getBeetle()->isLoaded())
		return kSceneInvalid;

	if (getBeetle()->catchBeetle()) {
		getBeetle()->unload();
		getInventory()->get(kItemBeetle)->location = kObjectLocation1;
		getSavePoints()->push(kCharacterCath, kCharacterMaster, kCharacterActionCatchBeetle);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 31
IMPLEMENT_ACTION(exitCompartment)
	if (!getProgress().field_30 && getProgress().jacket != kJacketOriginal) {
		getSaveLoad()->saveGame(kSavegameTypeTime, kCharacterCath, kTimeNone);
		getProgress().field_30 = 1;
	}

	getObjects()->updateModel(kObjectCompartment1, (ObjectModel)hotspot.param2);

	// fall to case enterCompartment action
	return action_enterCompartment(hotspot);
}

//////////////////////////////////////////////////////////////////////////
// Action 32
IMPLEMENT_ACTION(outsideTrain)
	switch(hotspot.param1) {
	default:
		break;

	case 1:
		getSavePoints()->push(kCharacterCath, kCharacterSalko, kCharacterAction167992577);
		break;

	case 2:
		getSavePoints()->push(kCharacterCath, kCharacterVesna, kCharacterAction202884544);
		break;

	case 3:
		if (getProgress().chapter == kChapter5) {
			getSavePoints()->push(kCharacterCath, kCharacterAbbot, kCharacterAction168646401);
			getSavePoints()->push(kCharacterCath, kCharacterMilos, kCharacterAction168646401);
		} else {
			getSavePoints()->push(kCharacterCath, kCharacterClerk, kCharacterAction203339360);
		}
		// Stop processing further scenes
		return kSceneNone;

	case 4:
		getSavePoints()->push(kCharacterCath, kCharacterMilos, kCharacterAction169773228);
		break;

	case 5:
		getSavePoints()->push(kCharacterCath, kCharacterVesna, kCharacterAction167992577);
		break;

	case 6:
		getSavePoints()->push(kCharacterCath, kCharacterAugust, kCharacterAction203078272);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 33
IMPLEMENT_ACTION(firebirdPuzzle)
	EventIndex evt = kEventNone;
	SceneIndex sceneIndex = kSceneInvalid;

	switch (hotspot.param1) {
	default:
		break;

	case 1:
		if (HELPERgetEvent(kEventKronosBringFirebird)) {
			getSavePoints()->push(kCharacterCath, kCharacterAnna, kCharacterAction205294778);
			break;
		}

		if (getEntities()->isInsideCompartment(kCharacterCath, kCarGreenSleeping, kPosition_8200)) {
			evt = kEventCathOpenEgg;

			Scene *scene = getScenes()->get(hotspot.scene);
			if (scene->getHotspot())
				sceneIndex = scene->getHotspot()->scene;

		} else {
			evt = kEventCathOpenEggNoBackground;
		}
		getProgress().isEggOpen = true;
		break;

	case 2:
		if (HELPERgetEvent(kEventKronosBringFirebird)) {
			getSavePoints()->push(kCharacterCath, kCharacterAnna, kCharacterAction224309120);
			break;
		}

		evt = (getEntities()->isInsideCompartment(kCharacterCath, kCarGreenSleeping, kPosition_8200)) ? kEventCathCloseEgg : kEventCathCloseEggNoBackground;
		getProgress().isEggOpen = false;
		break;

	case 3:
		if (HELPERgetEvent(kEventKronosBringFirebird)) {
			getSavePoints()->push(kCharacterCath, kCharacterAnna, kCharacterActionUseWhistle);
			break;
		}

		evt = (getEntities()->isInsideCompartment(kCharacterCath, kCarGreenSleeping, kPosition_8200)) ? kEventCathUseWhistleOpenEgg : kEventCathUseWhistleOpenEggNoBackground;
		break;

	}

	if (evt != kEventNone) {
		playAnimation(evt);
		if (sceneIndex == kSceneNone || !hotspot.scene)
			getScenes()->processScene();
	}

	return sceneIndex;
}

//////////////////////////////////////////////////////////////////////////
// Action 34
IMPLEMENT_ACTION(openMatchBox)
	// If the match is already in the inventory, do nothing
	if (!getInventory()->get(kItemMatch)->location
	 || getInventory()->get(kItemMatch)->inPocket)
		return kSceneInvalid;

	getInventory()->addItem(kItemMatch);
	getSound()->playSoundEvent(kCharacterCath, 102);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 35
IMPLEMENT_ACTION(openBed)
	getSound()->playSoundEvent(kCharacterCath, 59);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 36: Dummy action

//////////////////////////////////////////////////////////////////////////
// Action 37
IMPLEMENT_ACTION(dialog)
	getSound()->playDialog(kCharacterTableE, (CharacterIndex)hotspot.param1, kVolumeFull, 0);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 38
IMPLEMENT_ACTION(eggBox)
	getSound()->playSoundEvent(kCharacterCath, 43);
	if (getProgress().field_7C && !getSoundQueue()->isBuffered("MUS003")) {
		getSound()->playSound(kCharacterCath, "MUS003", kVolumeFull);
		getProgress().field_7C = 0;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 39
IMPLEMENT_ACTION(39)
	getSound()->playSoundEvent(kCharacterCath, 24);
	if (getProgress().field_80 && !getSoundQueue()->isBuffered("MUS003")) {
		getSound()->playSound(kCharacterCath, "MUS003", kVolumeFull);
		getProgress().field_80 = 0;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 40
IMPLEMENT_ACTION(bed)
	getSound()->playSoundEvent(kCharacterCath, 85);
	// falls to case knockNoSound
	return action_knockInside(hotspot);
}

//////////////////////////////////////////////////////////////////////////
// Action 41
IMPLEMENT_ACTION(playMusicChapter)
	byte id = 0;
	switch (getProgress().chapter) {
	default:
		break;

	case kChapter1:
		id = hotspot.param1;
		break;

	case kChapter2:
	case kChapter3:
		id = hotspot.param2;
		break;

	case kChapter4:
	case kChapter5:
		id = hotspot.param3;
		break;
	}

	if (id) {
		Common::String filename = Common::String::format("MUS%03d", id);

		if (!getSoundQueue()->isBuffered(filename))
			getSound()->playSound(kCharacterCath, filename, kVolumeFull);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 42
IMPLEMENT_ACTION(playMusicChapterSetupTrain)
	int id = 0;
	switch (getProgress().chapter) {
	default:
		break;

	case kChapter1:
		id = 1;
		break;

	case kChapter2:
	case kChapter3:
		id = 2;
		break;

	case kChapter4:
	case kChapter5:
		id = 4;
		break;
	}

	Common::String filename = Common::String::format("MUS%03d", hotspot.param1);

	if (!getSoundQueue()->isBuffered(filename) && hotspot.param3 & id) {
		getSound()->playSound(kCharacterCath, filename, kVolumeFull);

		getSavePoints()->call(kCharacterCath, kCharacterClerk, kCharacterAction203863200, filename);
		getSavePoints()->push(kCharacterCath, kCharacterClerk, kCharacterAction222746496, hotspot.param2);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// // Action 43
IMPLEMENT_ACTION(switchChapter)
	// Nothing to do here as a hotspot action
	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 44
IMPLEMENT_ACTION(44)
	switch (hotspot.param1) {
	default:
		break;

	case 1:
		getSavePoints()->push(kCharacterCath, kCharacterRebecca, kCharacterAction205034665);
		break;

	case 2:
		getSavePoints()->push(kCharacterCath, kCharacterMaster, kCharacterAction225358684);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////
void Action::pickGreenJacket(bool process) const {
	getProgress().jacket = kJacketGreen;
	getInventory()->addItem(kItemMatchBox);

	getObjects()->update(kObjectOutsideTylerCompartment, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
	playAnimation(kEventPickGreenJacket);

	getInventory()->setPortrait(kPortraitGreen);

	if (process)
		getScenes()->processScene();
}

void Action::pickScarf(bool process) const {
	playAnimation(getProgress().jacket == kJacketGreen ? kEventPickScarfGreen : kEventPickScarfOriginal);

	if (process)
		getScenes()->processScene();
}

void Action::pickCorpse(ObjectLocation bedPosition, bool process) const {

	if (getProgress().jacket == kJacketOriginal)
		getProgress().jacket = kJacketBlood;

	switch(getInventory()->get(kItemCorpse)->location) {
	// No way to pick the corpse
	default:
		break;

	// Floor
	case kObjectLocation1:
		// Bed is fully opened, move corpse directly there
		if (bedPosition == 4) {
			playAnimation(kEventCorpsePickFloorOpenedBedOriginal);

			getInventory()->get(kItemCorpse)->location = kObjectLocation5;
			break;
		}

		playAnimation(getProgress().jacket == kJacketGreen ? kEventCorpsePickFloorGreen : kEventCorpsePickFloorOriginal);
		break;

	// Bed
	case kObjectLocation2:
		playAnimation(getProgress().jacket == kJacketGreen ? kEventCorpsePickBedGreen : kEventCorpsePickBedOriginal);
		break;
	}

	if (process)
		getScenes()->processScene();

	// Add corpse to inventory
	if (bedPosition != 4) { // bed is not fully opened
		getInventory()->addItem(kItemCorpse);
		getInventory()->selectItem(kItemCorpse);
		_engine->getCursor()->setStyle(kCursorCorpse);
	}
}

void Action::dropCorpse(bool process) const {
	switch(getInventory()->get(kItemCorpse)->location) {
	default:
		break;

	case kObjectLocation1:	// Floor
		playAnimation(getProgress().jacket == kJacketGreen ? kEventCorpseDropFloorGreen : kEventCorpseDropFloorOriginal);
		break;

	case kObjectLocation2:	// Bed
		playAnimation(getProgress().jacket == kJacketGreen ? kEventCorpseDropBedGreen : kEventCorpseDropBedOriginal);
		break;

	case kObjectLocation4: // Window
		// Say goodbye to an old friend
		getInventory()->get(kItemCorpse)->location = kObjectLocationNone;
		getProgress().eventCorpseThrown = true;

		if (getState()->time <= kTime1138500) {
			playAnimation(getProgress().jacket == kJacketGreen ? kEventCorpseDropWindowGreen : kEventCorpseDropWindowOriginal);

			getProgress().field_24 = true;
		} else {
			playAnimation(kEventCorpseDropBridge);
		}

		getProgress().eventCorpseMovedFromFloor = true;
		break;
	}

	if (process)
		getScenes()->processScene();
}

bool Action::handleOtherCompartment(ObjectIndex object, bool doPlaySound, bool doLoadScene) const {
#define ENTITY_PARAMS(entity, index, id) \
	((EntityData::EntityParametersIIII*)getEntities()->get(entity)->getParamData()->getParameters(8, index))->param##id

	// Only handle compartments
	if (getEntityData(kCharacterCath)->location != kLocationOutsideCompartment
	|| ((object < kObjectCompartment2 || object > kObjectCompartment8) && (object < kObjectCompartmentA || object > kObjectCompartmentH)))
		return false;

	//////////////////////////////////////////////////////////////////////////
	// Gendarmes
	if (getEntityData(kCharacterCath)->car == getEntityData(kCharacterPolice)->car
	&& getEntityData(kCharacterPolice)->location == kLocationOutsideCompartment
	&& !getEntities()->compare(kCharacterCath, kCharacterPolice)) {
		if (doPlaySound)
			playCompartmentSoundEvents(object);

		if (doLoadScene)
			getScenes()->loadSceneFromObject(object);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Mertens
	if (getEntityData(kCharacterCath)->car == kCarGreenSleeping
	 && getEntityData(kCharacterCond1)->car == kCarGreenSleeping
	 && !getEntityData(kCharacterCond1)->location
	 && !ENTITY_PARAMS(kCharacterCond1, 0, 1)) {

		if (!getEntities()->compare(kCharacterCath, kCharacterCond1)) {

			if (getEntityData(kCharacterCond1)->entityPosition < kPosition_2740
			 && getEntityData(kCharacterCond1)->entityPosition > kPosition_850
			 && (getEntityData(kCharacterCond2)->car != kCarGreenSleeping || getEntityData(kCharacterCond2)->entityPosition > kPosition_2740)
			 && (getEntityData(kCharacterTrainM)->car != kCarGreenSleeping || getEntityData(kCharacterTrainM)->entityPosition > kPosition_2740)) {
				if (doPlaySound)
					playCompartmentSoundEvents(object);

				if (!getSoundQueue()->isBuffered(kCharacterCond1))
					getSound()->playWarningCompartment(kCharacterCond1, object);

				getSavePoints()->push(kCharacterCath, kCharacterCond1, kCharacterAction305159806);

				if (doLoadScene)
					getScenes()->loadSceneFromObject(object);

				return true;
			}

			if (getEntityData(kCharacterCond1)->direction == kDirectionUp
			 && getEntityData(kCharacterCond1)->entityPosition < getEntityData(kCharacterCath)->entityPosition) {
				if (doPlaySound)
					playCompartmentSoundEvents(object);

				if (!getSoundQueue()->isBuffered(kCharacterCond1))
					getSound()->playSound(kCharacterCond1, (rnd(2)) ? "JAC1000" : "JAC1000A");

				if (doLoadScene)
					getScenes()->loadSceneFromObject(object);
			}

			if (getEntityData(kCharacterCond1)->direction == kDirectionDown
			 && getEntityData(kCharacterCond1)->entityPosition > getEntityData(kCharacterCath)->entityPosition) {
				if (doPlaySound)
					playCompartmentSoundEvents(object);

				if (!getSoundQueue()->isBuffered(kCharacterCond1))
					getSound()->playSound(kCharacterCond1, (rnd(2)) ? "JAC1000" : "JAC1000A");

				if (doLoadScene)
					getScenes()->loadSceneFromObject(object, true);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Coudert
	if (getEntityData(kCharacterCath)->car != kCarRedSleeping
	 || !getEntityData(kCharacterCond2)->car
	 || getEntityData(kCharacterCond2)->location != kLocationOutsideCompartment
	 || ENTITY_PARAMS(kCharacterCond2, 0, 1))
	 return false;

	if (!getEntities()->compare(kCharacterCath, kCharacterCond2)) {

		if (getEntityData(kCharacterCond2)->entityPosition < kPosition_2740
		 && getEntityData(kCharacterCond2)->entityPosition > kPosition_850
		 && (getEntityData(kCharacterCond1)->car != kCarRedSleeping || getEntityData(kCharacterCond1)->entityPosition > kPosition_2740)
		 && (getEntityData(kCharacterTrainM)->car != kCarRedSleeping || getEntityData(kCharacterTrainM)->entityPosition > kPosition_2740)
		 && (getEntityData(kCharacterMadame)->car != kCarRedSleeping || getEntityData(kCharacterMadame)->entityPosition > kPosition_2740)) {
			if (doPlaySound)
				playCompartmentSoundEvents(object);

			if (!getSoundQueue()->isBuffered(kCharacterCond2))
				getSound()->playWarningCompartment(kCharacterCond2, object);

			getSavePoints()->push(kCharacterCath, kCharacterCond2, kCharacterAction305159806);

			if (doLoadScene)
				getScenes()->loadSceneFromObject(object);

			return true;
		}

		// Direction = Up
		if (getEntityData(kCharacterCond2)->direction == kDirectionUp
		 && getEntityData(kCharacterCond2)->entityPosition < getEntityData(kCharacterCath)->entityPosition) {
			if (doPlaySound)
				playCompartmentSoundEvents(object);

			if (!getSoundQueue()->isBuffered(kCharacterCond2))
				getSound()->playSound(kCharacterCond2, (rnd(2)) ? "JAC1000" : "JAC1000A");

			if (doLoadScene)
				getScenes()->loadSceneFromObject(object);

			return true;
		}

		// Direction = down
		if (getEntityData(kCharacterCond2)->direction == kDirectionDown
		 && getEntityData(kCharacterCond2)->entityPosition > getEntityData(kCharacterCath)->entityPosition) {
			if (doPlaySound)
				playCompartmentSoundEvents(object);

			if (!getSoundQueue()->isBuffered(kCharacterCond2))
				getSound()->playSound(kCharacterCond2, (rnd(2)) ? "JAC1000" : "JAC1000A");

			if (doLoadScene)
				getScenes()->loadSceneFromObject(object, true);
		}
	}

	return false;
}

void Action::playCompartmentSoundEvents(ObjectIndex object) const {
	if (getObjects()->get(object).status == kObjectLocation1 || getObjects()->get(object).status == kObjectLocation3 || getEntities()->checkFields2(object)) {
		getSound()->playSoundEvent(kCharacterCath, 13);
	} else {
		getSound()->playSoundEvent(kCharacterCath, 14);
		getSound()->playSoundEvent(kCharacterCath, 15, 3);
	}
}

//////////////////////////////////////////////////////////////////////////
// Cursors
//////////////////////////////////////////////////////////////////////////
CursorStyle Action::getCursor(const SceneHotspot &hotspot) const {
	// Simple cursor style
	if (hotspot.cursor != kCursorProcess)
		return (CursorStyle)hotspot.cursor;

	ObjectIndex object = (ObjectIndex)hotspot.param1;

	switch (hotspot.action) {
	default:
		return kCursorNormal;

	case SceneHotspot::kActionInventory:
		if (!getState()->sceneBackup2 && (HELPERgetEvent(kEventKronosBringFirebird) || getProgress().isEggOpen))
			return kCursorNormal;
		else
			return kCursorBackward;

	case SceneHotspot::kActionKnockOnDoor:
		debugC(2, kLastExpressDebugScenes, "================================= DOOR %03d =================================", object);
		if (object >= kObjectMax)
			return kCursorNormal;
		else
			return (CursorStyle)getObjects()->get(object).windowCursor;

	case SceneHotspot::kAction12:
		debugC(2, kLastExpressDebugScenes, "================================= OBJECT %03d =================================", object);
		if (object >= kObjectMax)
			return kCursorNormal;

		if (getObjects()->get(object).entity)
			return (CursorStyle)getObjects()->get(object).windowCursor;
		else
			return kCursorNormal;

	case SceneHotspot::kActionPickItem:
		debugC(2, kLastExpressDebugScenes, "================================= ITEM %03d =================================", object);
		if (object >= kObjectCompartmentA)
			return kCursorNormal;

		if ((!getInventory()->getSelectedItem() || getInventory()->getSelectedEntry()->floating)
		 && (object != kObject21 || getProgress().eventCorpseMovedFromFloor))
			return kCursorHand;
		else
			return kCursorNormal;

	case SceneHotspot::kActionDropItem:
		debugC(2, kLastExpressDebugScenes, "================================= ITEM %03d =================================", object);
		if (object >= kObjectCompartmentA)
			return kCursorNormal;

		if (getInventory()->getSelectedItem() != (InventoryItem)object)
			return kCursorNormal;

		if (object == kObject20 && hotspot.param2 == 4 && !getProgress().isTrainRunning)
			return kCursorNormal;

		if (object == kObjectHandleInsideBathroom  && hotspot.param2 == 1 && getProgress().field_5C)
			return kCursorNormal;

		return (CursorStyle)getInventory()->getSelectedEntry()->cursor;

	case SceneHotspot::kAction15:
		if (object >= kObjectMax)
			return kCursorNormal;

		if (getProgress().isEqual(object, hotspot.param2))
			return (CursorStyle)hotspot.param3;

		return kCursorNormal;

	case SceneHotspot::kActionEnterCompartment:
		if ((getInventory()->getSelectedItem() != kItemKey || getObjects()->get(kObjectCompartment1).status)
		&& (getObjects()->get(kObjectCompartment1).status != 1 || !getInventory()->hasItem(kItemKey)
		 ||	(getInventory()->getSelectedItem() != kItemFirebird && getInventory()->getSelectedItem() != kItemBriefcase)))
			goto LABEL_KEY;

		return (CursorStyle)getInventory()->get(kItemKey)->cursor; // TODO is that always the same as kCursorKey ?

	case SceneHotspot::kActionGetOutsideTrain:
		if (getProgress().jacket != kJacketGreen)
			return kCursorNormal;

		if ((HELPERgetEvent(kEventCathLookOutsideWindowDay) || HELPERgetEvent(kEventCathLookOutsideWindowNight) || getObjects()->get(kObjectCompartment1).model == kObjectModel1)
			&& getProgress().isTrainRunning
			&& (object != kObjectOutsideAnnaCompartment || (getEntities()->isInsideCompartment(kCharacterRebecca, kCarRedSleeping, kPosition_4840) && getObjects()->get(kObjectOutsideBetweenCompartments).status == 2))
			&& getInventory()->getSelectedItem() != kItemBriefcase && getInventory()->getSelectedItem() != kItemFirebird)
			return kCursorForward;

		return (getObjects()->get(kObjectCompartment1).model < kObjectModel2) ? kCursorNormal : kCursorMagnifier;

	case SceneHotspot::kActionSlip:
		return (getProgress().field_C8 < 1) ? kCursorNormal : kCursorLeft;

	case SceneHotspot::kActionClimbUpTrain:
		if (getProgress().isTrainRunning
			&& (getProgress().chapter == kChapter2 || getProgress().chapter == kChapter3 || getProgress().chapter == kChapter5)
			&& getInventory()->getSelectedItem() != kItemFirebird
			&& getInventory()->getSelectedItem() != kItemBriefcase)
			return kCursorUp;

		return kCursorNormal;

	case SceneHotspot::kActionJumpUpDownTrain:
		if (object != kObjectCompartment1)
			return kCursorNormal;

		return (getObjects()->get(kObjectCeiling).status < kObjectLocation1) ? kCursorHand : kCursorNormal;

	case SceneHotspot::kActionUnbound:
		if (hotspot.param2 != 2)
			return kCursorNormal;

		if (HELPERgetEvent(kEventCathBurnRope) || !HELPERgetEvent(kEventCathStruggleWithBonds2))
			return kCursorNormal;

		return kCursorHand;

	case SceneHotspot::kActionCatchBeetleHS:
		if (!getBeetle()->isLoaded())
			return kCursorNormal;

		if (!getBeetle()->isCatchable())
			return kCursorNormal;

		if (getInventory()->getSelectedItem() == kItemMatchBox && getInventory()->hasItem(kItemMatch))
			return (CursorStyle)getInventory()->get(kItemMatchBox)->cursor;

		return kCursorHandPointer;

	case SceneHotspot::KActionUseWhistle:
		if (object != kObjectCompartment3)
			return kCursorNormal;

		if (getInventory()->getSelectedItem() == kItemWhistle)
			return kCursorWhistle;
		else
			return kCursorNormal;

	case SceneHotspot::kActionOpenBed:
		if (getProgress().chapter < kChapter2)
			return kCursorHand;

		return kCursorNormal;

	case SceneHotspot::kActionDialog:
		if (getSound()->getDialogName((CharacterIndex)object))
			return kCursorHandPointer;

		return kCursorNormal;

	case SceneHotspot::kActionBed:
		if (getProgress().field_18 == 2 && !getProgress().field_E4
			&& (getState()->time > kTimeBedTime
			|| (getProgress().eventMetAugust && getProgress().field_CC
			&& (!getProgress().field_24 || getProgress().field_3C))))
			return kCursorSleep;

		return kCursorNormal;

LABEL_KEY:
	// Handle compartment action
	case SceneHotspot::kActionCompartment:
	case SceneHotspot::kActionExitCompartmentHS:
		debugC(2, kLastExpressDebugScenes, "================================= DOOR %03d =================================", object);
		if (object >= kObjectMax)
			return kCursorNormal;

		if (getInventory()->getSelectedItem() != kItemKey
		|| getObjects()->get(object).entity
		|| getObjects()->get(object).status != 1
		|| !getObjects()->get(object).handleCursor
		|| getEntities()->isInsideCompartments(kCharacterCath)
		|| getEntities()->checkFields2(object))
			return (CursorStyle)getObjects()->get(object).handleCursor;
		else
			return (CursorStyle)getInventory()->get(kItemKey)->cursor;
	}
}

//////////////////////////////////////////////////////////////////////////
// Animation
//////////////////////////////////////////////////////////////////////////

// Play an animation and add delta time to global game time
void Action::playAnimation(EventIndex index, bool debugMode) const {
	if (index >= _animationListSize)
		error("[Action::playAnimation] Invalid event index (value=%i, max=%i)", index, _animationListSize);

	// In debug mode, just show the animation
	if (debugMode) {
		Animation animation;
		if (animation.load(getArchiveMember(Common::String(_animationList[index].filename) + ".nis")))
			animation.play();
		return;
	}

	getFlags()->flag_3 = true;

	// Hide cursor
	_engine->getCursor()->show(false);

	// Show inventory & hourglass
	getInventory()->show();
	getInventory()->showHourGlass();

	if (!getFlags()->mouseRightClick) {

		if (getGlobalTimer()) {
			if (getSoundQueue()->isBuffered("TIMER")) {
				getSoundQueue()->fade("TIMER");
				setGlobalTimer(105);
			}
		}

		bool processSound = false;
		if (index >= kEventCorpseDropFloorOriginal
		 || index == kEventCathWakingUp
		 || index == kEventConcertCough
		 || index == kEventConcertSit
		 || index == kEventConcertLeaveWithBriefcase)
			processSound = true;

		Animation animation;
		if (animation.load(getArchiveMember(Common::String(_animationList[index].filename) + ".nis") , processSound ? Animation::kFlagDefault : Animation::kFlagProcess))
			animation.play();

		if (getSoundQueue()->isBuffered("TIMER"))
			getSoundQueue()->stop("TIMER");
	}

	// Show cursor
	_engine->getCursor()->show(true);

	HELPERgetEvent(index) = 1;

	// Adjust game time
	getState()->timeTicks += _animationList[index].time;
	getState()->time = (TimeValue)(getState()->time + (TimeValue)(_animationList[index].time * getState()->timeDelta));
}

int LogicManager::findCursor(Link *link) {
	int result;

	if (link->cursor != kCursorProcess)
		return link->cursor;

	switch (link->action) {
	case 1:
		if (!_lastSavedNodeIndex && (_gameEvents[kEventKronosBringFirebird] || _gameProgress[kEventAugustBringEgg])) {
			result = kCursorNormal;
		} else {
			result = kCursorBackward;
		}

		break;
	case 5:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else {
			result = _gameObjects[link->param1].cursor;
		}

		break;
	case 6:
	case 31:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else if (_inventorySelectedItemIdx != kItemKey || (_gameObjects[link->param1].character) || _gameObjects[link->param1].door != 1 || !_gameObjects[link->param1].cursor2 || inComp(kCharacterCath) || preventEnterComp(link->param1)) {
			result = _gameObjects[link->param1].cursor2;
		} else {
			result = _gameInventory[kItemKey].cursor;
		}

		break;
	case 12:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else {
			if (_gameObjects[link->param1].character)
				result = _gameObjects[link->param1].cursor;
			else
				result = kCursorNormal;
		}

		break;
	case 13:
		if (link->param1 >= 32) {
			result = kCursorNormal;
		} else if ((!_inventorySelectedItemIdx || _gameInventory[_inventorySelectedItemIdx].manualSelect) && (link->param1 != 21 || _gameProgress[kProgressEventCorpseMovedFromFloor] == 1)) {
			result = kCursorHand;
		} else {
			result = kCursorNormal;
		}

		break;
	case 14:
		if (link->param1 >= 32) {
			result = kCursorNormal;
		} else if (link->param1 != _inventorySelectedItemIdx || link->param1 == 20 && !_gameProgress[kProgressIsTrainRunning] && link->param2 == 4 || link->param1 == 18 && link->param2 == 1 && _gameProgress[kProgressField5C]) {
			result = kCursorNormal;
		} else {
			result = _gameInventory[_inventorySelectedItemIdx].cursor;
		}

		break;
	case 15:
		if (link->param1 >= 128) {
			result = kCursorNormal;
		} else if (_gameProgress[link->param1] == link->param2) {
			result = link->param3;
		} else {
			result = kCursorNormal;
		}

		break;
	case 16:
		if (_inventorySelectedItemIdx == kItemKey && !_gameObjects[kItemMatchBox].door || _gameObjects[kItemMatchBox].door == 1 && cathHasItem(kItemKey) && (_inventorySelectedItemIdx == kItemBriefcase || _inventorySelectedItemIdx == kItemFirebird)) {
			result = _gameInventory[kItemKey].cursor;
		} else {
			if (link->param1 >= 128) {
				result = 0;
			} else if (_inventorySelectedItemIdx != kItemKey ||
				_gameObjects[link->param1].character ||
				_gameObjects[link->param1].door != 1 ||
				!_gameObjects[link->param1].cursor2 ||
				inComp(kCharacterCath) || preventEnterComp(link->param1)) {
				result = _gameObjects[link->param1].cursor2;
			} else {
				result = _gameInventory[kItemKey].cursor;
			}
		}

		break;
	case 18:
		if (_gameProgress[kProgressJacket] == 2) {
			if ((_gameEvents[kEventCathLookOutsideWindowDay] || _gameEvents[kEventCathLookOutsideWindowNight] || getModel(1) == 1) &&
				_gameProgress[kProgressIsTrainRunning] &&
				(link->param1 != 45 || !inComp(kCharacterRebecca, kCarRedSleeping, 4840) && _gameObjects[kObjectOutsideBetweenCompartments].door == 2) &&
				_inventorySelectedItemIdx != kItemBriefcase && _inventorySelectedItemIdx != kItemFirebird) {
				result = kCursorForward;
			} else {
				result = getModel(1) == 1 ? kCursorNormal : kCursorMagnifier;
			}
		} else {
			result = kCursorNormal;
		}

		break;
	case 19:
		result = _gameProgress[kProgressFieldC8] == 0 ? kCursorNormal : kCursorLeft;
		break;
	case 21:
		if (_gameProgress[kProgressIsTrainRunning] && _inventorySelectedItemIdx != kItemBriefcase && _inventorySelectedItemIdx != kItemFirebird &&
			(_gameProgress[kProgressChapter] == 2 || _gameProgress[kProgressChapter] == 3 || _gameProgress[kProgressChapter] == 5)) {
			result = kCursorUp;
		} else {
			result = kCursorNormal;
		}

		break;
	case 23:
		if (link->param1 == 1) {
			result = checkDoor(73) == 0 ? kCursorHand : kCursorNormal;
		} else {
			result = kCursorNormal;
		}

		break;
	case 24:
		if (link->param1 == 2) {
			if (!_gameEvents[kEventCathStruggleWithBonds2] || _gameEvents[kEventCathBurnRope])
				result = kCursorNormal;
			else
				result = kCursorHand;
		} else {
			result = kCursorNormal;
		}

		break;
	case 30:
		if (_engine->_beetle) {
			if (_engine->_beetle->onTable()) {
				if (_inventorySelectedItemIdx == kItemMatchBox && cathHasItem(kItemMatch))
					result = _gameInventory[kItemMatchBox].cursor;
				else
					result = kCursorHandPointer;
			} else {
				result = kCursorNormal;
			}
		} else {
			result = kCursorNormal;
		}

		break;
	case 33:
		if (link->param1 == 3) {
			if (_inventorySelectedItemIdx == kItemWhistle)
				result = _gameInventory[kItemWhistle].cursor;
			else
				result = kCursorNormal;
		} else {
			result = kCursorNormal;
		}

		break;
	case 35:
		result = _gameProgress[kProgressChapter] == 1 ? kCursorHand : kCursorNormal;
		break;
	case 37:
		result = getHintDialog(link->param1) == 0 ? kCursorNormal : kCursorHandPointer;
		break;
	case 40:
		if (_gameProgress[kProgressField18] == 2 && !_gameProgress[kProgressFieldE4] && (_gameTime > 1404000 || _gameProgress[kProgressEventMetAugust] && _gameProgress[kProgressFieldCC] && (!_gameProgress[kProgressField24] || _gameProgress[kProgressField3C]))) {
			result = kCursorSleep;
		} else {
			result = kCursorNormal;
		}

		break;
	default:
		result = kCursorNormal;
		break;
	}

	return result;
}

bool LogicManager::nodeHasItem(int item) {
	switch (_trainData[_trainNodeIndex].car) {
	case kCarKronos:
		if (_trainData[_trainNodeIndex].parameter1 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kCarGreenSleeping:
		if (_trainData[_trainNodeIndex].parameter1 != item && _trainData[_trainNodeIndex].parameter2 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kCarRedSleeping:
		if (_trainData[_trainNodeIndex].parameter2 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kCarRestaurant:
		if (_trainData[_trainNodeIndex].parameter1 != item && _trainData[_trainNodeIndex].parameter2 != item && _trainData[_trainNodeIndex].parameter3 != item) {
			return false;
		} else {
			return true;
		}

		break;
	case kCarLocomotive:
		if (_trainData[_trainNodeIndex].parameter2 != item) {
			return false;
		} else {
			return true;
		}

		break;
	default:
		break;
	}

	return false;
}

void LogicManager::doPreFunction(int *sceneOut) {
	Link *link;
	Link *next;
	Link tmp;	
	uint16 scene;

	if (!*sceneOut || *sceneOut > 2500)
		*sceneOut = 1;

	switch (_trainData[*sceneOut].car) {
	case kCarBaggageRear:
		if (_trainData[*sceneOut].parameter1 < 128) {
			if (_gameObjects[_trainData[*sceneOut].parameter1].door) {
				link = _trainData[*sceneOut].link;
				for (bool found = false; link && !found; link = link->next) {
					if (_gameObjects[_trainData[*sceneOut].parameter1].door == link->location) {
						tmp.copyFrom(link);
						doAction(&tmp);

						if (tmp.scene) {
							*sceneOut = (int)link->scene;
							doPreFunction(sceneOut);
						}

						found = true;
					}
				}
			}
		}

		break;
	case kCarKronos:
		if (_trainData[*sceneOut].parameter1 < 32) {
			if (_gameInventory[_trainData[*sceneOut].parameter1].location) {
				link = _trainData[*sceneOut].link;
				for (bool found = false; link && !found; link = link->next) {
					if (_gameInventory[_trainData[*sceneOut].parameter1].location == link->location) {
						tmp.copyFrom(link);
						doAction(&tmp);

						if (tmp.scene) {
							*sceneOut = (int)link->scene;
							doPreFunction(sceneOut);
						}

						found = true;
					}
				}
			}
		}

		break;
	case kCarGreenSleeping:
		if (_trainData[*sceneOut].parameter1 < 32) {
			if (_trainData[*sceneOut].parameter2 < 32) {
				int locFlag = (_gameInventory[_trainData[*sceneOut].parameter1].location != 0) ? 1 : 0;

				if (_gameInventory[_trainData[*sceneOut].parameter2].location)
					locFlag |= 2;

				if (locFlag != 0) {
					link = _trainData[*sceneOut].link;
					for (bool found = false; link && !found; link = link->next) {
						if (link->location == locFlag) {
							if (_gameInventory[_trainData[*sceneOut].parameter1].location == link->param1 && _gameInventory[_trainData[*sceneOut].parameter2].location == link->param2) {
								tmp.copyFrom(link);
								doAction(&tmp);

								if (tmp.scene) {
									*sceneOut = (int)link->scene;
									doPreFunction(sceneOut);
								}

								found = true;
							}
						}
					}
				}
			}
		}

		break;
	case kCarRedSleeping:
		if (_trainData[*sceneOut].parameter1 < 128) {
			if (_trainData[*sceneOut].parameter2 < 32) {
				int locFlag = (_gameObjects[_trainData[*sceneOut].parameter1].door == 2) ? 1 : 0;

				if (_gameInventory[_trainData[*sceneOut].parameter2].location)
					locFlag |= 2;

				if (locFlag != 0) {
					link = _trainData[*sceneOut].link;
					for (bool found = false; link && !found; link = link->next) {
						if (link->location == locFlag) {
							if (_gameObjects[_trainData[*sceneOut].parameter1].door == link->param1 && _gameInventory[_trainData[*sceneOut].parameter2].location == link->param2) {
								tmp.copyFrom(link);
								doAction(&tmp);

								if (tmp.scene) {
									*sceneOut = (int)link->scene;
									doPreFunction(sceneOut);
								}

								found = true;
							}
						}
					}
				}
			}
		}

		break;
	case kCarRestaurant:
		if (_trainData[*sceneOut].parameter1 < 32 && _trainData[*sceneOut].parameter2 < 32) {
			if (_trainData[*sceneOut].parameter3 < 32) {
				int locFlag = (_gameInventory[_trainData[*sceneOut].parameter1].location != 0) ? 1 : 0;

				if (_gameInventory[_trainData[*sceneOut].parameter2].location)
					locFlag |= 2;

				if (_gameInventory[_trainData[*sceneOut].parameter3].location)
					locFlag |= 4;

				if (locFlag != 0) {
					link = _trainData[*sceneOut].link;
					for (bool found = false; link && !found; link = link->next) {
						if (link->location == locFlag) {
							if (_gameInventory[_trainData[*sceneOut].parameter1].location == link->param1 && _gameInventory[_trainData[*sceneOut].parameter2].location == link->param2 && _gameInventory[_trainData[*sceneOut].parameter3].location == link->param3) {
								tmp.copyFrom(link);
								doAction(&tmp);

								if (tmp.scene) {
									*sceneOut = (int)link->scene;
									doPreFunction(sceneOut);
								}

								found = true;
							}
						}
					}
				}
			}
		}

		break;
	case kCarBaggage:
		if (_trainData[*sceneOut].parameter1 < 128) {
			link = _trainData[*sceneOut].link;
			bool found = false;
			if (link) {
				while (!found) {
					if (_gameObjects[_trainData[*sceneOut].parameter1].model == link->location) {
						tmp.copyFrom(link);
						doAction(&tmp);

						if (tmp.scene) {
							*sceneOut = (int)link->scene;
							doPreFunction(sceneOut);
						}

						found = true;
					}

					link = link->next;
					if (!link) {
						if (!found) {
							tmp.copyFrom(_trainData[*sceneOut].link);
							doAction(&tmp);

							if (tmp.scene) {
								*sceneOut = (int)tmp.scene;
								doPreFunction(sceneOut);
								break;
							}
						}
					}
				}
			} else {
				// This was a nullptr access in the original I think?
				// I'm shielding it for now, it might only happen during
				// non-ordinary situations (e.g. no conductors)
				if (!found && _trainData[*sceneOut].link) {
					tmp.copyFrom(_trainData[*sceneOut].link);
					doAction(&tmp);

					if (tmp.scene) {
						*sceneOut = (int)tmp.scene;
						doPreFunction(sceneOut);
					}
				}
			}
		}

		break;
	case kCarCoalTender:
		if (_trainData[*sceneOut].parameter1 < 16 &&
			(_softBlockedEntitiesBits[_trainData[*sceneOut].parameter1] || _blockedEntitiesBits[_trainData[*sceneOut].parameter1])) {
			if ((!_engine->getOtisManager()->fDirection(_trainNodeIndex) || !_engine->getOtisManager()->fDirection(*sceneOut) || _trainData[_trainNodeIndex].nodePosition.position >= _trainData[*sceneOut].nodePosition.position) &&
				(!_engine->getOtisManager()->rDirection(_trainNodeIndex) || !_engine->getOtisManager()->rDirection(*sceneOut) || _trainData[_trainNodeIndex].nodePosition.position <= _trainData[*sceneOut].nodePosition.position)) {
				next = _trainData[*sceneOut].link->next;
				scene = next->scene;
				*sceneOut = (int)scene;
				doPreFunction(sceneOut);

				break;
			}

			if (whoseBit(_softBlockedEntitiesBits[_trainData[*sceneOut].parameter1]) != 30 &&
				whoseBit(_blockedEntitiesBits[_trainData[*sceneOut].parameter1]) != 30) {
				playDialog(kCharacterCath, "CAT1126A", -1, 0);
			}

			scene = _trainData[*sceneOut].link->scene;
			*sceneOut = (int)scene;
			doPreFunction(sceneOut);
		}

		break;
	case kCarLocomotive:
		if (_trainData[*sceneOut].parameter1 < 32) {
			if (_softBlockedEntitiesBits[_trainData[*sceneOut].parameter1] || _blockedEntitiesBits[_trainData[*sceneOut].parameter1]) {
				if (_engine->getOtisManager()->fDirection(_trainNodeIndex) &&
					_engine->getOtisManager()->fDirection(*sceneOut) &&
					_trainData[_trainNodeIndex].nodePosition.position < _trainData[*sceneOut].nodePosition.position ||
					_engine->getOtisManager()->rDirection(_trainNodeIndex) &&
					_engine->getOtisManager()->rDirection(*sceneOut) &&
					_trainData[_trainNodeIndex].nodePosition.position > _trainData[*sceneOut].nodePosition.position) {

					if (whoseBit(_softBlockedEntitiesBits[_trainData[*sceneOut].parameter1]) != 30 && whoseBit(_blockedEntitiesBits[_trainData[*sceneOut].parameter1]) != 30) {
						playDialog(kCharacterCath, "CAT1126A", -1, 0);
					}

					scene = _trainData[*sceneOut].link->scene;
				} else {
					next = _trainData[*sceneOut].link->next;
					scene = next->scene;
				}

				*sceneOut = (int)scene;
				doPreFunction(sceneOut);
			} else {
				if (_trainData[*sceneOut].parameter2 < 32) {
					if (_gameInventory[_trainData[*sceneOut].parameter2].location) {
						link = _trainData[*sceneOut].link;
						for (bool found = false; link && !found; link = link->next) {
							if (_gameInventory[_trainData[*sceneOut].parameter2].location == link->location) {
								tmp.copyFrom(link);
								doAction(&tmp);

								if (tmp.scene) {
									*sceneOut = (int)link->scene;
									doPreFunction(sceneOut);
								}

								found = true;
							}
						}
					}
				}
			}
		}

		break;
	default:
		break;
	}

	if (whoRunningDialog(kCharacterTableE)) {
		if (_trainData[*sceneOut].car != 132 || _trainData[*sceneOut].parameter1)
			fadeDialog(kCharacterTableE);
	}

	if (_engine->_beetle) {
		if (_trainData[*sceneOut].car != 130)
			_engine->endBeetle();
	}
}

void LogicManager::doPostFunction() {
	Link tmp;

	switch (_trainData[_trainNodeIndex].car) {
	case 128:
	{
		int32 delta = _trainData[_trainNodeIndex].parameter1 + 10;
		_gameTime += delta * _gameTimeTicksDelta;
		_currentGameSessionTicks += delta;
		int32 delayedTicks = _engine->getSoundFrameCounter() + 4 * _trainData[_trainNodeIndex].parameter1;

		if (!_engine->mouseHasRightClicked() && delayedTicks > _engine->getSoundFrameCounter()) {
			do {
				if (_engine->mouseHasRightClicked())
					break;
				_engine->getSoundManager()->soundThread();
				_engine->getSubtitleManager()->subThread();
			} while (delayedTicks > _engine->getSoundFrameCounter());
		}

		tmp.copyFrom(_trainData[_trainNodeIndex].link);
		doAction(&tmp);

		if (_engine->mouseHasRightClicked() && _trainData[tmp.scene].car == 128) {
			do {
				tmp.copyFrom(_trainData[tmp.scene].link);
				doAction(&tmp);

				// TODO: Figure out why this happens when canceling fast-walks mode
				// in between cars...
				if (tmp.action == 0) {
					warning("Possible loop for link with action 0");
					//break;
				}
			} while (_trainData[tmp.scene].car == 128);
		}

		if (getCharacter(kCharacterCath).characterPosition.car == 9 &&
			(getCharacter(kCharacterCath).characterPosition.position == 4 ||
			 getCharacter(kCharacterCath).characterPosition.position == 3)) {

			int characterIdx = 0;
			int charactersRndArray[39];

			memset(charactersRndArray, 0, sizeof(charactersRndArray));

			for (int j = 1; j < 40; j++) {
				if (getCharacter(kCharacterCath).characterPosition.position == 4) {
					if (getCharacter(j).characterPosition.car == 4 && getCharacter(j).characterPosition.position > 9270 || getCharacter(j).characterPosition.car == 5 && getCharacter(j).characterPosition.position < 1540) {
						charactersRndArray[characterIdx] = j;
						characterIdx++;
					}
				} else if (getCharacter(j).characterPosition.car == 3 && getCharacter(j).characterPosition.position > 9270 || getCharacter(j).characterPosition.car == 4 && getCharacter(j).characterPosition.position < 850) {
					charactersRndArray[characterIdx] = j;
					characterIdx++;
				}
			}

			if (characterIdx) {
				if (characterIdx <= 1) {
					playChrExcuseMe(charactersRndArray[0], kCharacterCath, 16);
				} else {
					playChrExcuseMe(charactersRndArray[rnd(characterIdx)], kCharacterCath, 16);
				}
			}
		}

		if (tmp.scene)
			_engine->getGraphicsManager()->stepBG(tmp.scene);

		return;
	}
	case 129:
		if (_gameProgress[kProgressField18] == 2)
			send(kCharacterCath, kCharacterMaster, 190346110, 0);

		return;
	case 130:
		_engine->doBeetle();
		return;
	case 131:
	{
		if (_gameTime < 2418300 && _gameProgress[kProgressField18] != 4) {
			Slot *slot = _engine->getSoundManager()->_soundCache;
			if (slot) {
				do {
					if (slot->hasTag(kSoundTagLink))
						break;

					slot = slot->getNext();
				} while (slot);

				if (slot)
					slot->setFade(0);
			}

			playDialog(kCharacterClerk, "LIB050", 16, 0);

			if (_gameProgress[kProgressChapter] == 1) {
				endGame(0, 0, 62, true);
			} else if (_gameProgress[kProgressChapter] == 4) {
				endGame(0, 0, 64, true);
			} else {
				endGame(0, 0, 63, true);
			}
		}

		return;
	}
	case 132:
		if (!whoRunningDialog(kCharacterTableE)) {
			switch (_trainData[_trainNodeIndex].parameter1) {
			case 1:
				if (dialogRunning("TXT1001"))
					endDialog("TXT1001");

				playDialog(kCharacterTableE, "TXT1001", 16, 0);
				break;
			case 2:
				if (dialogRunning("TXT1001A"))
					endDialog("TXT1001A");

				playDialog(kCharacterTableE, "TXT1001A", 16, 0);
				break;
			case 3:
				if (dialogRunning("TXT1011"))
					endDialog("TXT1011");

				playDialog(kCharacterTableE, "TXT1011", 16, 0);
				break;
			case 4:
				if (dialogRunning("TXT1012"))
					endDialog("TXT1012");

				playDialog(kCharacterTableE, "TXT1012", 16, 0);
				break;
			case 5:
				if (dialogRunning("TXT1013"))
					endDialog("TXT1013");

				playDialog(kCharacterTableE, "TXT1013", 16, 0);
				break;
			case 6:
				if (dialogRunning("TXT1014"))
					endDialog("TXT1014");

				playDialog(kCharacterTableE, "TXT1014", 16, 0);
				break;
			case 7:
				if (dialogRunning("TXT1020"))
					endDialog("TXT1020");

				playDialog(kCharacterTableE, "TXT1020", 16, 0);
				break;
			case 8:
				if (dialogRunning("TXT1030"))
					endDialog("TXT1030");

				playDialog(kCharacterTableE, "TXT1030", 16, 0);
				break;
			case 50:
				playDialog(kCharacterTableE, "END1009B", 16, 0);
				break;
			case 51:
				playDialog(kCharacterTableE, "END1046", 16, 0);
				break;
			case 52:
				playDialog(kCharacterTableE, "END1047", 16, 0);
				break;
			case 53:
				playDialog(kCharacterTableE, "END1112", 16, 0);
				break;
			case 54:
				playDialog(kCharacterTableE, "END1112A", 16, 0);
				break;
			case 55:
				playDialog(kCharacterTableE, "END1503", 16, 0);
				break;
			case 56:
				playDialog(kCharacterTableE, "END1505A", 16, 0);
				break;
			case 57:
				playDialog(kCharacterTableE, "END1505B", 16, 0);
				break;
			case 58:
				playDialog(kCharacterTableE, "END1610", 16, 0);
				break;
			case 59:
				playDialog(kCharacterTableE, "END1612A", 16, 0);
				break;
			case 60:
				playDialog(kCharacterTableE, "END1612C", 16, 0);
				break;
			case 61:
				playDialog(kCharacterTableE, "END1612D", 16, 0);
				break;
			case 62:
				playDialog(kCharacterTableE, "ENDALRM1", 16, 0);
				break;
			case 63:
				playDialog(kCharacterTableE, "ENDALRM2", 16, 0);
				break;
			case 64:
				playDialog(kCharacterTableE, "ENDALRM3", 16, 0);
				break;
			default:
				break;
			}
		}

		return;
	case 133:
		if (_doubleClickFlag) {
			_doubleClickFlag = false;
			_engine->getGraphicsManager()->setMouseDrawable(true);
			mouseStatus();
		}

		break;
	default:
		break;
	}
}

void LogicManager::doAction(Link *link) {
	char filename[8];

	int musId = 0;
	int nisId = 0;
	
	switch (link->action) {
	case kActionInventory:
	{
		if (_useLastSavedNodeIndex) {
			int bumpScene = 0;
			if (_lastSavedNodeIndex) {
				bumpScene = _lastSavedNodeIndex;
				_lastSavedNodeIndex = 0;
				bumpCathNode(bumpScene);
			} else {
				_useLastSavedNodeIndex = 0;

				if (_positions[100 * _trainData[_lastNodeIndex].nodePosition.car + _trainData[_lastNodeIndex].cathDir]) {
					bumpCathNode(getSmartBumpNode(_lastNodeIndex));
				} else {
					bumpCathNode(_lastNodeIndex);
				}
			}

			if (_inventorySelectedItemIdx && (!_gameInventory[_inventorySelectedItemIdx].isSelectable || (bumpScene == 0 && findLargeItem()))) {
				_inventorySelectedItemIdx = findLargeItem();

				if (_inventorySelectedItemIdx) {
					_engine->getGraphicsManager()->drawItem(_gameInventory[_inventorySelectedItemIdx].cursor, 44, 0);
				} else if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}

				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
				return;
			}
		}

		break;
	}
	case kActionSendCathMessage:
		send(kCharacterCath, link->param1, link->param2, 0);
		break;
	case kActionPlaySound:
		Common::sprintf_s(filename, "LIB%03d", link->param1);
		if (link->param2 || !cathRunningDialog(filename))
			queueSFX(kCharacterCath, link->param1, link->param2);

		break;
	case kActionPlayMusic:
		Common::sprintf_s(filename, "MUS%03d", link->param1);
		if (!dialogRunning(filename) && (link->param1 != 50 || _gameProgress[kProgressChapter] == 5))
			playDialog(kCharacterCath, filename, 16, link->param2);

		break;
	case kActionKnock:
		if (link->param1 < 128) {
			if (_gameObjects[link->param1].character) {
				send(kCharacterCath, _gameObjects[link->param1].character, 8, link->param1);
				return;
			}

			if (!cathRunningDialog("LIB012"))
				queueSFX(kCharacterCath, 12, 0);
		}

		break;
	case kActionPlaySounds:
		queueSFX(kCharacterCath, link->param1, 0);
		queueSFX(kCharacterCath, link->param3, link->param2);

		break;
	case kActionPlayAnimation:
		if (!_gameEvents[link->param1]) {
			playNIS(link->param1);

			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionOpenCloseObject:
		if (link->param1 >= 128)
			return;

		setDoor(link->param1, _gameObjects[link->param1].character, link->param2, 255, 255);
		if ((link->param1 < 9 || link->param1 > 16) && (link->param1 < 40 || link->param1 > 47)) {
			if (link->param2) {
				if (link->param2 == 1) {
					queueSFX(kCharacterCath, 24, 0);
					return;
				}

				if (link->param2 != 2)
					return;
			}

			queueSFX(kCharacterCath, 36, 0);
			return;
		}

		if (link->param2) {
			if (link->param2 == 2)
				queueSFX(kCharacterCath, 20, 0);
		} else {
			queueSFX(kCharacterCath, 21, 0);
		}

		break;
	case kActionSetModel:
		if (link->param1 < 128) {
			setModel(link->param1, link->param2);
			if (link->param1 != 112 || dialogRunning("LIB096")) {
				if (link->param1 == 1)
					queueSFX(kCharacterCath, 73, 0);
			} else {
				queueSFX(kCharacterCath, 96, 0);
			}
		}

		break;
	case kActionSetItem:
		if (link->param1 < 32) {
			if (!_gameInventory[link->param1].isPresent) {
				_gameInventory[link->param1].location = link->param2;

				if (link->param1 == kItemCorpse) {
					_gameProgress[kProgressEventCorpseMovedFromFloor] = (_gameInventory[kItemCorpse].location == 3 || _gameInventory[kItemCorpse].location == 4) ? 1 : 0;
				}
			}
		}

		break;
	case kActionPickItem:
		if (link->param1 >= 32)
			return;

		if (!_gameInventory[link->param1].location)
			return;

		if (link->param1 == kItemCorpse) {
			takeTyler(link->scene == 0, link->param2);

			if (link->param2 != 4) {
				_gameInventory[kItemCorpse].isPresent = 1;
				_gameInventory[kItemCorpse].location = 0;
				_inventorySelectedItemIdx = kItemCorpse;
				_engine->getGraphicsManager()->drawItem(_gameInventory[kItemCorpse].cursor, 44, 0);
				_engine->getGraphicsManager()->burstBox(44u, 0, 32, 32);
			}
		} else {
			_gameInventory[link->param1].isPresent = 1;
			_gameInventory[link->param1].location = 0;

			if (link->param1 == kItemGreenJacket) {
				takeJacket(link->scene == 0);
			} else {
				if (link->param1 == kItemScarf) {
					takeScarf(link->scene == 0);
					return;
				}

				if (link->param1 == kItemParchemin && link->param2 == 2) {
					_gameInventory[kItemParchemin].isPresent = 1;
					_gameInventory[kItemParchemin].location = 0;
					_gameInventory[kItem11].location = 1;
					queueSFX(kCharacterCath, 9, 0);
				} else if (link->param1 == kItemBomb) {
					forceJump(kCharacterAbbot, &LogicManager::CONS_Abbot_CatchCath);
				} else if (link->param1 == kItemBriefcase) {
					queueSFX(kCharacterCath, 83, 0);
				}
			}

			if (_gameInventory[link->param1].scene) {
				if (!_useLastSavedNodeIndex) {
					if (!link->scene)
						link->scene = _trainNodeIndex;

					_useLastSavedNodeIndex = 1;
					_lastNodeIndex = link->scene;
				}

				bumpCathNode(_gameInventory[link->param1].scene);
				link->scene = kSceneNone;
			}

			if (_gameInventory[link->param1].isSelectable) {
				_inventorySelectedItemIdx = link->param1;
				_engine->getGraphicsManager()->drawItem(_gameInventory[link->param1].cursor, 44, 0);
				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}
		}

		break;
	case kActionDropItem:
		if (link->param1 >= 32 || !_gameInventory[link->param1].isPresent || !link->param2)
			return;

		if (link->param1 == kItemBriefcase) {
			queueSFX(kCharacterCath, 82, 0);

			if (link->param2 == 2) {
				if (!_gameProgress[kProgressField58]) {
					_engine->getVCR()->writeSavePoint(1, 0, 0);
					_gameProgress[kProgressField58] = 1;
				}

				if (_gameInventory[kItemParchemin].location == 2) {
					_gameInventory[kItemParchemin].isPresent = 1;
					_gameInventory[kItemParchemin].location = 0;
					_gameInventory[kItem11].location = 1;
					queueSFX(kCharacterCath, 9, 0);
				}
			}
		}

		_gameInventory[link->param1].isPresent = 0;
		_gameInventory[link->param1].location = link->param2;
		if (link->param1 == 20)
			dropTyler(link->scene == 0);

		_inventorySelectedItemIdx = 0;

		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
			_engine->getGraphicsManager()->unlockSurface();
		}

		_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
		break;
	
	case kActionLeanOutWindow:
		if (!_gameEvents[kEventCathLookOutsideWindowDay] && !_gameEvents[kEventCathLookOutsideWindowNight] && getModel(1) != 1 || !_gameProgress[kProgressIsTrainRunning] || link->param1 == 45 && (inComp(kCharacterRebecca, kCarRedSleeping, 4840) || _gameObjects[kObjectOutsideBetweenCompartments].door != 2) || _inventorySelectedItemIdx == kItemBriefcase || _inventorySelectedItemIdx == kItemFirebird) {
			if (link->param1 == 9 || link->param1 >= 44 && link->param1 <= 45) {
				if (isNight()) {
					playNIS(kEventCathLookOutsideWindowNight);
				} else {
					playNIS(kEventCathLookOutsideWindowDay);
				}
				cleanNIS();
				link->scene = kSceneNone;
			}

			return;
		}

		switch (link->param1) {
		case 9:
			_gameEvents[kEventCathLookOutsideWindowDay] = 1;

			if (isNight()) {
				playNIS(kEventCathGoOutsideTylerCompartmentNight);
			} else {
				playNIS(kEventCathGoOutsideTylerCompartmentDay);
			}

			_gameProgress[kProgressFieldC8] = 1;
			break;
		case 44:
			_gameEvents[kEventCathLookOutsideWindowDay] = 1;

			if (isNight()) {
				playNIS(kEventCathGoOutsideNight);
			} else {
				playNIS(kEventCathGoOutsideDay);
			}

			_gameProgress[kProgressFieldC8] = 1;
			break;
		case 45:
			_gameEvents[kEventCathLookOutsideWindowDay] = 1;

			if (isNight()) {
				playNIS(kEventCathGetInsideNight);
			} else {
				playNIS(kEventCathGetInsideDay);
			}

			if (!link->scene)
				cleanNIS();

			break;
		}

		break;
	case kActionAlmostFall:
		if (link->param1 == 9) {
			if (isNight()) {
				playNIS(kEventCathSlipTylerCompartmentNight);
			} else {
				playNIS(kEventCathSlipTylerCompartmentDay);
			}

			_gameProgress[kProgressFieldC8] = 0;

			if (link->scene)
				return;
		} else {
			if (link->param1 != 44)
				return;

			if (isNight()) {
				playNIS(kEventCathSlipNight);
			} else {
				playNIS(kEventCathSlipDay);
			}

			_gameProgress[kProgressFieldC8] = 0;

			if (link->scene)
				return;
		}

		cleanNIS();
		break;
	case kActionClimbInWindow:
		switch (link->param1) {
		case 9:
			if (isNight()) {
				playNIS(kEventCathGetInsideTylerCompartmentNight);
			} else {
				playNIS(kEventCathGetInsideTylerCompartmentDay);
			}

			if (link->scene)
				return;

			break;
		case 44:
			if (isNight()) {
				playNIS(kEventCathGetInsideNight);
			} else {
				playNIS(kEventCathGetInsideDay);
			}

			if (link->scene)
				return;

			break;
		case 45:
			playNIS(kEventCathGettingInsideAnnaCompartment);

			if (link->scene)
				return;

			break;
		default:
			return;
		}

		cleanNIS();
		break;
	case kActionClimbLadder:
		if (link->param1 == 1) {
			if (_gameProgress[kProgressChapter] == 2 || _gameProgress[kProgressChapter] == 3) {
				playNIS(kEventCathTopTrainGreenJacket);
			} else if (_gameProgress[kProgressChapter] == 5) {
				playNIS(kEventCathTopTrainNoJacketDay - (_gameProgress[kProgressIsDayTime] == kProgressField0));
			}

			if (link->scene)
				return;
		} else {
			if (link->param1 != 2)
				return;

			if (_gameProgress[kProgressChapter] == 2 || _gameProgress[kProgressChapter] == 3) {
				playNIS(kEventCathClimbUpTrainGreenJacket);
				playNIS(kEventCathTopTrainGreenJacket);
			} else if (_gameProgress[kProgressChapter] == 5) {
				playNIS(kEventCathClimbUpTrainNoJacketDay - (_gameProgress[kProgressIsDayTime] == 0));
				playNIS(kEventCathTopTrainNoJacketDay - (_gameProgress[kProgressIsDayTime] == 0));
			}

			if (link->scene)
				return;
		}

		cleanNIS();
		break;
	case kActionClimbDownTrain:
		if (_gameProgress[kProgressChapter] == 2 || _gameProgress[kProgressChapter] == 3) {
			nisId = kEventCathClimbDownTrainGreenJacket;
		} else if (_gameProgress[kProgressChapter] == 5) {
			if (_gameProgress[kProgressIsDayTime] == 0) {
				nisId = kEventCathClimbDownTrainNoJacketNight;
			} else {
				nisId = kEventCathClimbDownTrainNoJacketDay;
			}
		}

		if (nisId) {
			playNIS(nisId);

			if (nisId == kEventCathClimbDownTrainNoJacketDay)
				queueSFX(kCharacterCath, 37, 0);

			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionKronosSanctum:
		switch (link->param1) {
		case 1:
			send(kCharacterCath, 32, 225056224, 0);
			break;
		case 2:
			send(kCharacterCath, 32, 338494260, 0);
			break;
		case 3:
			if (_inventorySelectedItemIdx == kItemBriefcase) {
				_gameInventory[kItemBriefcase].location = 3;
				_gameInventory[kItemBriefcase].isPresent = 0;
				queueSFX(kCharacterCath, 82, 0);
				_inventorySelectedItemIdx = 0;
				if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}
				_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			}

			if (_gameInventory[kItemBriefcase].location == 3) {
				nisId = kEventCathJumpUpCeiling;
			} else {
				nisId = kEventCathJumpUpCeilingBriefcase;
			}

			break;
		case 4:
			if (_gameProgress[kProgressChapter] == 1)
				send(kCharacterCath, kCharacterKronos, 202621266, 0);
			break;
		default:
			break;
		}

		if (nisId) {
			playNIS(nisId);

			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionEscapeBaggage:
		switch (link->param1) {
		case 1:
			nisId = kEventCathStruggleWithBonds;
			break;
		case 2:
			nisId = kEventCathBurnRope;
			break;
		case 3:
			if (_gameEvents[kEventCathBurnRope]) {
				playNIS(kEventCathRemoveBonds);
				_gameProgress[kProgressField84] = 0;
				bumpCath(kCarBaggageRear, 89, 255);
				link->scene = kSceneNone;
			}

			break;
		case 4:
			if (!_gameEvents[kEventCathStruggleWithBonds2]) {
				playNIS(kEventCathStruggleWithBonds2);
				queueSFX(kCharacterCath, 101, 0);
				dropItem(kItemMatch, 2);
				if (!link->scene)
					cleanNIS();
			}
			break;
		case 5:
			send(kCharacterCath, kCharacterIvo, 192637492, 0);
			break;
		default:
			break;
		}

		if (nisId) {
			playNIS(nisId);
			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionEnterBaggage:
		switch (link->param1) {
		case 1:
			send(kCharacterCath, kCharacterAnna, 272177921, 0);
			break;
		case 2:
			if (!dialogRunning("MUS021"))
				playDialog(kCharacterCath, "MUS021", 16, 0);

			break;
		case 3:
			queueSFX(kCharacterCath, 43, 0);
			if (cathHasItem(kItemKey)) {
				if (!_gameEvents[kEventAnnaBaggageArgument]) {
					forceJump(kCharacterAnna, &LogicManager::CONS_Anna_BaggageFight);
					link->scene = kSceneNone;
				}
			}

			break;
		}

		break;
	case kActionBombPuzzle:
		switch (link->param1) {
		case 1:
			send(kCharacterCath, kCharacterMaster, 158610240, 0);
			break;
		case 2:
			send(kCharacterCath, kCharacterMaster, 225367984, 0);
			_inventorySelectedItemIdx = 0;

			if (_engine->getGraphicsManager()->acquireSurface()) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
				_engine->getGraphicsManager()->unlockSurface();
			}

			_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			link->scene = kSceneNone;

			break;
		case 3:
			send(kCharacterCath, kCharacterMaster, 191001984, 0);
			link->scene = kSceneNone;
			break;
		case 4:
			send(kCharacterCath, kCharacterMaster, 201959744, 0);
			link->scene = kSceneNone;
			break;
		case 5:
			send(kCharacterCath, kCharacterMaster, 169300225, 0);
			break;
		default:
			break;
		}

		break;
	case kActionConductors:
		if (!cathRunningDialog("LIB031"))
			queueSFX(kCharacterCath, 31, 0);

		if (getCharacter(kCharacterCath).characterPosition.car == 3) {
			send(kCharacterCath, kCharacterCond1, 225358684, link->param1);
		} else if (getCharacter(kCharacterCath).characterPosition.car == 4) {
			send(kCharacterCath, kCharacterCond2, 225358684, link->param1);
		}

		break;
	case kActionKronosConcert:
		if (link->param1 == 1) {
			nisId = kEventConcertSit;
		} else if (link->param1 == 2) {
			nisId = kEventConcertCough;
		}

		if (nisId) {
			playNIS(nisId);
			if (!link->scene)
				cleanNIS();
		}

		break;
	case kActionPlayMusic2:
		_gameProgress[kProgressFieldC] = 1;
		queueSFX(kCharacterCath, link->param1, link->param2);
		Common::sprintf_s(filename, "MUS%03d", link->param3);
		if (!dialogRunning(filename))
			playDialog(kCharacterCath, filename, 16, 0);

		break;
	case kActionCatchBeetle:
		if (_engine->_beetle && _engine->_beetle->click()) {
			_engine->endBeetle();
			_gameInventory[kItemBeetle].location = 1;
			send(kCharacterCath, kCharacterClerk, 202613084, 0);
		}

		break;

	case kActionCompartment:
	case kActionExitCompartment:
	case kActionEnterCompartment:
	{
		bool skipFlag = false;

		if (link->action != kActionCompartment) {
			if (link->action == kActionExitCompartment) {
				if (!_gameProgress[kProgressField30] && _gameProgress[kProgressJacket]) {
					_engine->getVCR()->writeSavePoint(1, kCharacterCath, 0);
					_gameProgress[kProgressField30] = 1;
				}

				setModel(1, link->param2);
			}

			if (_gameObjects[kItemMatchBox].door != 1 && _gameObjects[kItemMatchBox].door != 3 && _inventorySelectedItemIdx != kItemKey) {
				if (!_gameProgress[kProgressEventFoundCorpse]) {
					_engine->getVCR()->writeSavePoint(1, kCharacterCath, 0);
					playDialog(kCharacterCath, "LIB014", -1, 0);
					playNIS(kEventCathFindCorpse);
					playDialog(kCharacterCath, "LIB015", -1, 0);
					_gameProgress[kProgressEventFoundCorpse] = 1;
					link->scene = kSceneCompartmentCorpse;

					return;
				}
			} else {
				skipFlag = true;
			}
		}

		if (skipFlag || link->action == kActionCompartment || (link->action != kActionEnterCompartment || _gameInventory[kItemBriefcase].location != 2)) {
			if (link->param1 >= 128)
				return;

			if (_gameObjects[link->param1].character) {
				send(kCharacterCath, _gameObjects[link->param1].character, 9, link->param1);
				link->scene = kSceneNone;
				return;
			}

			if (bumpCathTowardsCond(link->param1, 1, 1)) {
				link->scene = kSceneNone;
				return;
			}

			if (_gameObjects[link->param1].door == 1 || _gameObjects[link->param1].door == 3 || preventEnterComp(link->param1)) {
				if (_gameObjects[link->param1].door != 1 || preventEnterComp(link->param1) || _inventorySelectedItemIdx != 15 && (link->param1 != 1 || !cathHasItem(kItemKey) || _inventorySelectedItemIdx != kItemBriefcase && _inventorySelectedItemIdx != kItemFirebird)) {
					if (!cathRunningDialog("LIB013"))
						queueSFX(kCharacterCath, 13, 0);

					link->scene = kSceneNone;
					return;
				}

				queueSFX(kCharacterCath, 32, 0);

				if (link->param1 != 0 && link->param1 <= 3 || link->param1 >= 32 && link->param1 <= 37)
					setDoor(link->param1, kCharacterCath, 0, 10, 9);

				queueSFX(kCharacterCath, 15, 22);
				_inventorySelectedItemIdx = 0;
				if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}
			} else {
				if (link->action != 16 || _inventorySelectedItemIdx != kItemKey) {
					if (link->param1 == 109) {
						queueSFX(kCharacterCath, 26, 0);
					} else {
						queueSFX(kCharacterCath, 14, 0);
						queueSFX(kCharacterCath, 15, 22);
					}

					return;
				}

				setDoor(1, kCharacterCath, 1, 10, 9);
				queueSFX(kCharacterCath, 16, 0);
				_inventorySelectedItemIdx = 0;
				link->scene = kSceneNone;

				if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 44, 0, 32, 32);
					_engine->getGraphicsManager()->unlockSurface();
				}
			}

			_engine->getGraphicsManager()->burstBox(44, 0, 32, 32);
			return;
		}

		queueSFX(kCharacterCath, 14, 0);
		queueSFX(kCharacterCath, 15, 22);

		if (_gameProgress[kProgressField78] && !dialogRunning("MUS003")) {
			playDialog(kCharacterCath, "MUS003", 16, 0);
			_gameProgress[kProgressField78] = 0;
		}

		bumpCath(kCarGreenSleeping, 77, 255);
		link->scene = kSceneNone;

		break;
	}

	case kActionOutsideTrain:
		switch (link->param1) {
		case 1:
			send(kCharacterCath, kCharacterSalko, 167992577, 0);
			break;
		case 2:
			send(kCharacterCath, kCharacterVesna, 202884544, 0);
			break;
		case 3:
			if (_gameProgress[kProgressChapter] == 5) {
				send(kCharacterCath, kCharacterAbbot, 168646401, 0);
				send(kCharacterCath, kCharacterMilos, 168646401, 0);
			} else {
				send(kCharacterCath, kCharacterClerk, 203339360, 0);
			}

			link->scene = kSceneNone;
			break;
		case 4:
			send(kCharacterCath, kCharacterMilos, 169773228, 0);
			break;
		case 5:
			send(kCharacterCath, kCharacterVesna, 167992577, 0);
			break;
		case 6:
			send(kCharacterCath, kCharacterAugust, 203078272, 0);
			break;
		default:
			break;
		}

		break;
	case kActionFirebirdPuzzle:
		if (_gameEvents[kEventKronosBringFirebird]) {
			switch (link->param1) {
			case 1:
				send(kCharacterCath, kCharacterAnna, 205294778, 0);
				break;
			case 2:
				send(kCharacterCath, kCharacterAnna, 224309120, 0);
				break;
			case 3:
				send(kCharacterCath, kCharacterAnna, 270751616, 0);
				break;
			}
		} else {
			switch (link->param1) {
			case 1:
			{
				if (inComp(kCharacterCath, kCarGreenSleeping, 8200)) {
					nisId = kEventCathOpenEgg;
					Link *lnk = _trainData[link->scene].link;
					if (lnk)
						link->scene = lnk->scene;
				} else {
					nisId = kEventCathOpenEggNoBackground;
				}

				_gameProgress[kProgressIsEggOpen] = 1;
				break;
			}
			case 2:
				if (!inComp(kCharacterCath, kCarGreenSleeping, 8200)) {
					nisId = kEventCathCloseEggNoBackground;
				} else {
					nisId = kEventCathCloseEgg;
				}

				_gameProgress[kProgressIsEggOpen] = 0;
				break;
			case 3:
				if (!inComp(kCharacterCath, kCarGreenSleeping, 8200)) {
					nisId = kEventCathUseWhistleOpenEggNoBackground;
				} else {
					nisId = kEventCathUseWhistleOpenEgg;
				}

				break;
			}

			if (nisId) {
				playNIS(nisId);

				if (!link->scene)
					cleanNIS();
			}
		}

		break;
	case kActionOpenMatchBox:
		if (_gameInventory[kItemMatch].location && !_gameInventory[kItemMatch].isPresent) {
			_gameInventory[kItemMatch].isPresent = 1;
			_gameInventory[kItemMatch].location = 0;
			queueSFX(kCharacterCath, 102, 0);
		}

		break;
	case kActionOpenBed:
		queueSFX(kCharacterCath, 59, 0);
		break;
	case kActionHintDialog:
		if (dialogRunning(getHintDialog(link->param1))) {
			endDialog(getHintDialog(link->param1));
		}

		playDialog(kCharacterTableE, getHintDialog(link->param1), 16, 0);

		break;
	case kActionMusicEggBox:
		queueSFX(kCharacterCath, 43, 0);
		if (_gameProgress[kProgressField7C] && !dialogRunning("MUS003")) {
			playDialog(kCharacterCath, "MUS003", 16, 0);
			_gameProgress[kProgressField7C] = 0;
		}

		break;
	case kActionPlayMusic3:
		queueSFX(kCharacterCath, 24, 0);
		if (_gameProgress[kProgressField80] && !dialogRunning("MUS003")) {
			playDialog(kCharacterCath, "MUS003", 16, 0);
			_gameProgress[kProgressField80] = 0;
		}

		break;
	case kActionKnockInside:
	case kActionBed:
		if (link->action == kActionBed) {
			queueSFX(kCharacterCath, 85, 0);
		}

		if (link->param1 < 128) {
			if (_gameObjects[link->param1].character)
				send(kCharacterCath, _gameObjects[link->param1].character, 8, link->param1);
		}

		return;
	case kActionPlayMusicChapter:
		switch (_gameProgress[kProgressChapter]) {
		case 1:
			musId = link->param1;
			break;
		case 2:
		case 3:
			musId = link->param2;
			break;
		case 4:
		case 5:
			musId = link->param3;
			break;
		}

		if (musId) {
			Common::sprintf_s(filename, "MUS%03d", musId);
			if (!dialogRunning(filename))
				playDialog(kCharacterCath, filename, 16, 0);
		}

		break;
	case kActionPlayMusicChapterSetupTrain:
		switch (_gameProgress[kProgressChapter]) {
		case 1:
			musId = 1;
			break;
		case 2:
		case 3:
			musId = 2;
			break;
		case 4:
		case 5:
			musId = 4;
			break;
		}

		Common::sprintf_s(filename, "MUS%03d", link->param1);

		if (!dialogRunning(filename) && (link->param3 & musId) != 0) {
			playDialog(kCharacterCath, filename, 16, 0);

			fedEx(kCharacterCath, kCharacterClerk, 203863200, filename);
			send(kCharacterCath, kCharacterClerk, 222746496, link->param2);
		}

		break;
	case kActionEasterEggs:
		if (link->param1 == 1) {
			send(kCharacterCath, kCharacterRebecca, 205034665, 0);
		} else if (link->param1 == 2) {
			send(kCharacterCath, kCharacterMaster, 225358684, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::takeTyler(bool doCleanNIS, int8 bedPosition) {
	if (!_gameProgress[kProgressJacket])
		_gameProgress[kProgressJacket] = 1;

	if (_gameInventory[kItemCorpse].location == 1) {
		if (bedPosition == 4) {
			if (_gameProgress[kProgressJacket])
				playNIS(kEventCorpsePickFloorOpenedBedOriginal);

			_gameInventory[kItemCorpse].location = 5;
		} else if (_gameProgress[kProgressJacket] == 2) {
			playNIS(kEventCorpsePickFloorGreen);
		} else {
			playNIS(kEventCorpsePickFloorOriginal);
		}
	} else if (_gameInventory[kItemCorpse].location == 2) {
		if (_gameProgress[kProgressJacket] == 2) {
			playNIS(kEventCorpsePickBedGreen);
		} else {
			playNIS(kEventCorpsePickBedOriginal);
		}
	}

	if (doCleanNIS)
		cleanNIS();
}

void LogicManager::dropTyler(bool doCleanNIS) {
	switch (_gameInventory[kItemCorpse].location) {
	case 1:
		if (_gameProgress[kProgressJacket] == 2) {
			playNIS(kEventCorpseDropFloorGreen);
		} else {
			playNIS(kEventCorpseDropFloorOriginal);
		}

		break;
	case 2:
		if (_gameProgress[kProgressJacket] == 2) {
			playNIS(kEventCorpseDropBedGreen);
		} else {
			playNIS(kEventCorpseDropBedOriginal);
		}

		break;
	case 4:
		_gameInventory[kItemCorpse].location = 0;
		_gameProgress[kProgressEventCorpseThrown] = 1;

		if (_gameTime <= 1138500) {
			if (_gameProgress[kProgressJacket] == 2) {
				playNIS(kEventCorpseDropWindowGreen);
			} else {
				playNIS(kEventCorpseDropWindowOriginal);
			}

			_gameProgress[kProgressField24] = 1;
		} else {
			playNIS(kEventCorpseDropBridge);
		}

		_gameProgress[kProgressEventCorpseMovedFromFloor] = 1;

		break;
	}

	if (doCleanNIS)
		cleanNIS();
}

void LogicManager::takeJacket(bool doCleanNIS) {
	_gameProgress[kProgressJacket] = 2;
	_gameInventory[kItemMatchBox].isPresent = 1;
	_gameInventory[kItemMatchBox].location = 0;
	setDoor(9, kCharacterCath, 2, 255, 255);
	playNIS(kEventPickGreenJacket);
	_gameProgress[kProgressPortrait] = 34;
	_engine->getGraphicsManager()->drawItemDim(_gameProgress[kProgressPortrait], 0, 0, 1);
	_engine->getGraphicsManager()->burstBox(0, 0, 32, 32);

	if (doCleanNIS)
		cleanNIS();
}

void LogicManager::takeScarf(bool doCleanNIS) {
	if (_gameProgress[kProgressJacket] == 2) {
		playNIS(kEventPickScarfGreen);
	} else {
		playNIS(kEventPickScarfOriginal);
	}

	if (doCleanNIS)
		cleanNIS();
}

const char *LogicManager::getHintDialog(int character) {
	if (whoRunningDialog(kCharacterTableE))
		return nullptr;

	switch (character) {
	case kCharacterAnna:
		if (_gameEvents[kEventAnnaDialogGoToJerusalem]) {
			return "XANN12";
		} else if (_gameEvents[kEventLocomotiveRestartTrain]) {
			return "XANN11";
		} else if (_gameEvents[kEventAnnaBaggageTies] ||
				   _gameEvents[kEventAnnaBaggageTies2] ||
				   _gameEvents[kEventAnnaBaggageTies3] ||
				   _gameEvents[kEventAnnaBaggageTies4]) {
			return "XANN10";
		} else if (_gameEvents[kEventAnnaTired] ||
				   _gameEvents[kEventAnnaTiredKiss]) {
			return "XANN9";
		} else if (_gameEvents[kEventAnnaBaggageArgument]) {
			return "XANN8";
		} else if (_gameEvents[kEventKronosVisit]) {
			return "XANN7";
		} else if (_gameEvents[kEventAbbotIntroduction]) {
			return "XANN6A";
		} else if (_gameEvents[kEventVassiliSeizure]) {
			return "XANN6";
		} else if (_gameEvents[kEventAugustPresentAnna] ||
				   _gameEvents[kEventAugustPresentAnnaFirstIntroduction]) {
			return "XANN5";
		} else if (_gameProgress[kProgressField60]) {
			return "XANN4";
		} else if (_gameEvents[kEventAnnaGiveScarf] ||
				   _gameEvents[kEventAnnaGiveScarfDiner] ||
				   _gameEvents[kEventAnnaGiveScarfSalon] ||
				   _gameEvents[kEventAnnaGiveScarfMonogram] ||
				   _gameEvents[kEventAnnaGiveScarfDinerMonogram] ||
				   _gameEvents[kEventAnnaGiveScarfSalonMonogram]) {
			return "XANN3";
		} else if (_gameEvents[kEventDinerMindJoin]) {
			return "XANN2";
		} else if (_gameEvents[kEventGotALight] ||
				   _gameEvents[kEventGotALightD]) {
			return "XANN1";
		}

		break;
	case kCharacterAugust:
		if (_gameEvents[kEventAugustTalkCigar]) {
			return "XAUG6";
		} else if (_gameEvents[kEventAugustBringBriefcase]) {
			return "XAUG5";
		} else if (_gameEvents[kEventAugustMerchandise]) {
			if (_gameTime <= 2200500) {
				return "XAUG4";
			} else {
				return "XAUG4A";
			}
		} else if (_gameEvents[kEventDinerAugust] ||
				   _gameEvents[kEventDinerAugustAlexeiBackground] ||
				   _gameEvents[kEventMeetAugustTylerCompartment] ||
				   _gameEvents[kEventMeetAugustHisCompartment] ||
				   _gameEvents[kEventMeetAugustTylerCompartmentBed] ||
				   _gameEvents[kEventMeetAugustHisCompartmentBed]) {
			return "XAUG3";
		} else if (_gameEvents[kEventAugustPresentAnnaFirstIntroduction]) {
			return "XAUG2";
		} else if (_gameProgress[kProgressEventMertensAugustWaiting]) {
			return "XAUG1";
		}

		break;
	case kCharacterTatiana:
		if (_gameEvents[kEventTatianaTylerCompartment]) {
			return "XTAT6";
		} else if (_gameEvents[kEventTatianaCompartmentStealEgg]) {
			return "XTAT5";
		} else if (_gameEvents[kEventTatianaGivePoem]) {
			return "XTAT3";
		} else if (_gameProgress[kProgressField64]) {
			return "XTAT1";
		}

		break;
	case kCharacterVassili:
		if (_gameEvents[kEventCathFreePassengers]) {
			return "XVAS4";
		} else if (_gameEvents[kEventVassiliCompartmentStealEgg]) {
			return "XVAS3";
		} else if (_gameEvents[kEventAbbotIntroduction]) {
			return "XVAS2";
		} else if (_gameEvents[kEventVassiliSeizure]) {
			return "XVAS1A";
		} else if (_gameProgress[kProgressField64]) {
			return "XVAS1";
		}

		break;
	case kCharacterAlexei:
		if (_gameProgress[kProgressField88]) {
			return "XALX6";
		} else if (_gameProgress[kProgressField8C]) {
			return "XALX5";
		} else if (_gameProgress[kProgressField90]) {
			return "XALX4A";
		} else if (_gameProgress[kProgressField68]) {
			return "XALX4";
		} else if (_gameEvents[kEventAlexeiSalonPoem]) {
			return "XALX3";
		} else if (_gameEvents[kEventAlexeiSalonVassili]) {
			return "XALX2";
		} else if (_gameEvents[kEventAlexeiDiner] ||
				   _gameEvents[kEventAlexeiDinerOriginalJacket]) {
			return "XALX1";
		}

		break;
	case kCharacterAbbot:
		if (_gameEvents[kEventAbbotDrinkDefuse]) {
			return "XABB4";
		} else if (_gameEvents[kEventAbbotInvitationDrink] ||
				   _gameEvents[kEventDefuseBomb]) {
			return "XABB3";
		} else if (_gameEvents[kEventAbbotWrongCompartment] ||
				   _gameEvents[kEventAbbotWrongCompartmentBed]) {
			return "XABB2";
		} else if (_gameEvents[kEventAbbotIntroduction]) {
			return "XABB1";
		}

		break;
	case kCharacterMilos:
		if (_gameEvents[kEventLocomotiveMilosDay] || _gameEvents[kEventLocomotiveMilosNight]) {
			return "XMIL5";
		} else if (_gameEvents[kEventMilosCompartmentVisitTyler] &&
				  (_gameProgress[kProgressChapter] == 3 ||
				   _gameProgress[kProgressChapter] == 4)) {
			return "XMIL4";
		} else if (_gameEvents[kEventMilosCorridorThanks] ||
				   _gameProgress[kProgressChapter] == 5) {
			return "XMIL3";
		} else if (_gameEvents[kEventMilosCompartmentVisitAugust]) {
			return "XMIL2";
		} else if (_gameEvents[kEventMilosTylerCompartmentDefeat]) {
			return "XMIL1";
		}

		break;
	case kCharacterVesna:
		if (_gameProgress[kProgressField94]) {
			return "XVES2";
		} else if (_gameProgress[kProgressField98]) {
			return "XVES1";
		}

		break;
	case kCharacterKronos:
		if (_gameEvents[kEventKronosReturnBriefcase])
			return "XKRO6";
		if (_gameEvents[kEventKronosBringEggCeiling] ||
			_gameEvents[kEventKronosBringEgg]) {
			return "XKRO5";
		} else {
			if (_gameEvents[kEventKronosConversation] ||
				_gameEvents[kEventKronosConversationFirebird]) {
				if (_gameInventory[kItemFirebird].location != 6 &&
					_gameInventory[kItemFirebird].location != 5 &&
					_gameInventory[kItemFirebird].location != 2 &&
					_gameInventory[kItemFirebird].location != 1)
					return "XKRO4A";
			}

			if (_gameEvents[kEventKronosConversationFirebird])
				return "XKRO4";

			if (_gameEvents[kEventMilosCompartmentVisitAugust]) {
				if (_gameEvents[kEventKronosConversation])
					return "XKRO3";
			} else if (_gameEvents[kEventKronosConversation]) {
				return "XKRO2";
			}

			if (_gameProgress[kProgressEventMertensChronosInvitation]) {
				return "XKRO1";
			}
		}

		break;
	case kCharacterFrancois:
		if (_gameProgress[kProgressField9C]) {
			return "XFRA3";
		} else if (_gameProgress[kProgressFieldA0] ||
				   _gameEvents[kEventFrancoisWhistle] ||
				   _gameEvents[kEventFrancoisWhistleD] ||
				   _gameEvents[kEventFrancoisWhistleNight] ||
				   _gameEvents[kEventFrancoisWhistleNightD]) {
			return "XFRA2";
		} else if (_gameTime <= 1075500) {
			return "XFRA1";
		}

		break;
	case kCharacterMadame:
		if (_gameProgress[kProgressFieldA4]) {
			return "XMME4";
		} else if (_gameProgress[kProgressFieldA8]) {
			return "XMME3";
		} else if (_gameProgress[kProgressFieldA0]) {
			return "XMME2";
		} else  if (_gameProgress[kProgressFieldAC]) {
			return "XMME1";
		}

		break;
	case kCharacterMonsieur:
		if (_gameProgress[kProgressEventMetBoutarel]) {
			return "XMRB1";
		}

		break;
	case kCharacterRebecca:
		if (_gameProgress[kProgressFieldB4]) {
			return "XREB1A";
		} else if (_gameProgress[kProgressFieldB8]) {
			return "XREB1";
		}

		break;
	case kCharacterSophie:
		if (_gameProgress[kProgressFieldB0]) {
			return "XSOP2";
		} else if (_gameProgress[kProgressFieldBC]) {
			return "XSOP1B";
		} else if (_gameProgress[kProgressFieldB4]) {
			return "XSOP1A";
		} else if (!_gameProgress[kProgressFieldB8]) {
			return "XSOP1";
		}

		break;
	case kCharacterMahmud:
		if (_gameProgress[kProgressFieldC4]) {
			return "XMAH1";
		}

		break;
	case kCharacterYasmin:
		if (_gameProgress[kProgressEventMetYasmin]) {
			return "XHAR2";
		}

		break;
	case kCharacterHadija:
		if (_gameProgress[kProgressEventMetHadija]) {
			return "XHAR1";
		}

		break;
	case kCharacterAlouan:
		if (_gameProgress[kProgressFieldDC]) {
			return "XHAR3";
		}

		break;
	case kCharacterPolice:
		if (_gameProgress[kProgressFieldE0]) {
			return "XHAR4";
		}

		break;
	case kCharacterMaster:
		if (_gameEvents[kEventCathDream] || _gameEvents[kEventCathWakingUp]) {
			return "XTYL3";
		} else {
			return "XTYL1";
		}

		break;
	default:
		break;
	}

	return nullptr;
}

} // End of namespace LastExpress
