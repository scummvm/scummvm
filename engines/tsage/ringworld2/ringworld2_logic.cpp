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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_logic.h"
#include "tsage/ringworld2/ringworld2_dialogs.h"
#include "tsage/ringworld2/ringworld2_scenes0.h"
#include "tsage/ringworld2/ringworld2_scenes2.h"
#include "tsage/ringworld2/ringworld2_scenes3.h"

namespace TsAGE {

namespace Ringworld2 {

Scene *Ringworld2Game::createScene(int sceneNumber) {
	switch (sceneNumber) {
	/* Scene group #0 */
	case 50: 
		// Waking up cutscene
		return new Scene50();
	case 100: 
		// Quinn's room
		return new Scene100();
	case 125: 
		// Computer console
		return new Scene125();
	case 150: 
		// Empty Bedroom #1
		return new Scene150();
	case 160:
		error("Missing scene %d from group 0", sceneNumber);
	case 175:
		// Empty Bedroom #2
		return new Scene175();
	case 180:
		error("Missing scene %d from group 0", sceneNumber);
	case 200:
		// Deck #2 - By Lift
		return new Scene200();
	case 205:
		error("Missing scene %d from group 0", sceneNumber);
	case 250:
		// Lift
		return new Scene250();
	case 300:
		// Bridge
		return new Scene300();
	case 325:
		error("Missing scene %d from group 0", sceneNumber);
	case 400:
		// Science Lab
		return new Scene400();
	case 500:
	case 525:
	case 600:
	case 700:
		error("Missing scene %d from group 0", sceneNumber);
	case 800:
		// Sick bay
		return new Scene800();
	case 825:
		// Autodoc
		return new Scene825();
	case 850:
		// Deck #5 - By Lift
		return new Scene850();
	case 900:
		error("Missing scene %d from group 0", sceneNumber);
	/* Scene group #1 */
	//
	case 1000:
	case 1010:
	case 1020:
	case 1100:
	case 1200:
	case 1330:
	case 1500:
	case 1525:
	case 1530:
	case 1550:
	case 1575:
	case 1580:
	case 1625:
	case 1700:
	case 1750:
	case 1800:
	case 1850:
	case 1875:
	case 1900:
	case 1925:
	case 1945:
	case 1950:
		error("Missing scene %d from group 1", sceneNumber);
	/* Scene group #2 */
	//
	case 2000:
		// Ice Maze
		return new Scene2000();
	case 2350:
		// Ice Maze: Balloon Launch Platform
		return new Scene2350();
	case 2400:
		// Ice Maze: Large empty room
		return new Scene2400();
	case 2425:
		// Ice Maze: 
		return new Scene2425();
	case 2430:
		// Ice Maze: Bedroom
		return new Scene2430();
	case 2435:
		// Ice Maze: Throne room
		return new Scene2435();
	case 2440:
		// Ice Maze: Another bedroom
		return new Scene2440();
	case 2445:
		// Ice Maze:
		return new Scene2445();
	case 2450:
		// Ice Maze: Another bedroom
		return new Scene2450();
	case 2455:
		// Ice Maze: Inside crevasse
		return new Scene2455();
	case 2500:
		// Ice Maze: Large Cave
		return new Scene2500();
	case 2525:
		// Ice Maze: Furnace room
		return new Scene2525();
	case 2530:
		// Ice Maze: Well
		return new Scene2530();
	case 2535:
		// Ice Maze: Tannery
		return new Scene2535();
	case 2600:
		// Ice Maze: Exit
		return new Scene2600();
	case 2700:
		// Forest Maze
		return new Scene2700();
	case 2750:
		// Forest Maze
		return new Scene2750();
	case 2800:
		// Exiting Forest
		return new Scene2800();
	case 2900:
		error("Missing scene %d from group 2", sceneNumber);
	/* Scene group #3 */
	//
	case 3100:
		return new Scene3100();
	case 3125:
		// Ghouls dormitory
		return new Scene3125();
	case 3150:
		// Jail
		return new Scene3150();
	case 3175:
		// Autopsy room
		return new Scene3175();
	case 3200:
		// Cutscene : Guards - Discussion
		return new Scene3200();
	case 3210:
		// Cutscene : Captain and Private - Discussion
		return new Scene3210();
	case 3220:
		// Cutscene : Guards in cargo zone
		return new Scene3220();
	case 3230:
		// Cutscene : Guards on duty
		return new Scene3230();
	case 3240:
		// Cutscene : Teal monolog
		return new Scene3240();
	case 3245:
		// Cutscene : Discussions with Dr. Tomko
		return new Scene3245();
	case 3250:
		// Room with large stasis field negator
		return new Scene3250();
	case 3255:
		return new Scene3255();
	case 3260:
		// Computer room
		return new Scene3260();
	case 3275:
	case 3350:
	case 3375:
	case 3385:
	case 3395:
	case 3400:
	case 3500:
	case 3600:
	case 3700:
	case 3800:
	case 3900:
		error("Missing scene %d from group 3", sceneNumber);
	default:
		error("Unknown scene number - %d", sceneNumber);
		break;
	}
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool Ringworld2Game::canLoadGameStateCurrently() {
	// Don't allow a game to be loaded if a dialog is active
	return g_globals->_gfxManagers.size() == 1;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool Ringworld2Game::canSaveGameStateCurrently() {
	// Don't allow a game to be saved if a dialog is active
	return g_globals->_gfxManagers.size() == 1;
}

/*--------------------------------------------------------------------------*/

SceneExt::SceneExt(): Scene() {
	_stripManager._onBegin = SceneExt::startStrip;
	_stripManager._onEnd = SceneExt::endStrip;

	_field372 = _field37A = 0;
	_savedPlayerEnabled = false;
	_savedUiEnabled = false;
	_savedCanWalk = false;
	_focusObject = NULL;
}

void SceneExt::postInit(SceneObjectList *OwnerList) {
	Scene::postInit(OwnerList);

	// Exclude the bottom area of the screen to allow room for the UI
	T2_GLOBALS._interfaceY = UI_INTERFACE_Y;

	// Initialise fields
	_action = NULL;
	_field12 = 0;
	_sceneMode = 0;
}

void SceneExt::remove() {
	_sceneAreas.clear();
	Scene::remove();
}

void SceneExt::process(Event &event) {
	if (!event.handled)
		Scene::process(event);
}

void SceneExt::dispatch() {
/*
	_timerList.dispatch();

	if (_field37A) {
		if ((--_field37A == 0) && R2_GLOBALS._dayNumber) {
			if (R2_GLOBALS._uiElements._active && R2_GLOBALS._player._enabled) {
				R2_GLOBALS._uiElements.show();
			}

			_field37A = 0;
		}
	}
*/
	Scene::dispatch();
}

void SceneExt::loadScene(int sceneNum) {
	Scene::loadScene(sceneNum);

	_v51C34.top = 0;
	_v51C34.bottom = 300;

	int prevScene = R2_GLOBALS._sceneManager._previousScene;
	int sceneNumber = R2_GLOBALS._sceneManager._sceneNumber;

	if (((prevScene == -1) && (sceneNumber != 180) && (sceneNumber != 205) && (sceneNumber != 50)) ||
			(sceneNumber == 50) || ((prevScene == 205) && (sceneNumber == 100)) ||
			((prevScene == 180) && (sceneNumber == 100))) {
		// TODO: sub_17875
		R2_GLOBALS._v58CE2 = 1;
		R2_GLOBALS._uiElements.show();
	} else {
		// Update the user interface
		R2_GLOBALS._uiElements.updateInventory();
	}
}

bool SceneExt::display(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_CROSSHAIRS:
		return false;
	case CURSOR_LOOK:
		SceneItem::display2(1, R2_GLOBALS._randomSource.getRandomNumber(4));
		break;
	case CURSOR_USE:
		SceneItem::display2(1, R2_GLOBALS._randomSource.getRandomNumber(4) + 5);
		break;
	case CURSOR_TALK:
		SceneItem::display2(1, R2_GLOBALS._randomSource.getRandomNumber(4) + 10);
		break;
	case R2_NEGATOR_GUN:
		if (R2_GLOBALS.getFlag(1))
			SceneItem::display2(2, action);
		else
			SceneItem::display2(5, 0);
		break;
	case R2_7:
		if ((R2_GLOBALS._v565F1[1] == 2) || ((R2_GLOBALS._v565F1[1] == 1) &&
				(R2_GLOBALS._v565F1[2] == 2) && (R2_GLOBALS._sceneManager._previousScene == 300))) {
			R2_GLOBALS._sound4.stop();
			R2_GLOBALS._sound3.play(46);
			SceneItem::display2(5, 15);
		} else {
			R2_GLOBALS._sound3.play(43, 0);
			SceneItem::display2(2, 0);
		}

		R2_GLOBALS._sound4.play(45);
		break;
	case R2_9:
	case R2_39:
		R2_GLOBALS._sound3.play(44);
		SceneItem::display2(2, action);
		R2_GLOBALS._sound3.stop();
		break;
	case R2_44:
		R2_GLOBALS._sound3.play(99);
		SceneItem::display2(2, action);
		break;
	default:
		SceneItem::display2(2, action);
		break;
	}

	event.handled = true;
	return true;
}

void SceneExt::fadeOut() {
	uint32 black = 0;
	R2_GLOBALS._scenePalette.fade((const byte *)&black, false, 100);
}

void SceneExt::startStrip() {
	SceneExt *scene = (SceneExt *)R2_GLOBALS._sceneManager._scene;
	scene->_field372 = 1;
	scene->_savedPlayerEnabled = R2_GLOBALS._player._enabled;

	if (scene->_savedPlayerEnabled) {
		scene->_savedUiEnabled = R2_GLOBALS._player._uiEnabled;
		scene->_savedCanWalk = R2_GLOBALS._player._canWalk;
		R2_GLOBALS._player.disableControl();
/*
		if (!R2_GLOBALS._v50696 && R2_GLOBALS._uiElements._active)
			R2_GLOBALS._uiElements.hide();
*/
	}
}

void SceneExt::endStrip() {
	SceneExt *scene = (SceneExt *)R2_GLOBALS._sceneManager._scene;
	scene->_field372 = 0;

	if (scene->_savedPlayerEnabled) {
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._uiEnabled = scene->_savedUiEnabled;
		R2_GLOBALS._player._canWalk = scene->_savedCanWalk;
/*
		if (!R2_GLOBALS._v50696 && R2_GLOBALS._uiElements._active)
			R2_GLOBALS._uiElements.show();
*/
	}
}

void SceneExt::clearScreen() {
	R2_GLOBALS._screenSurface.fillRect(R2_GLOBALS._screenSurface.getBounds(), 0);
}

void SceneExt::refreshBackground(int xAmount, int yAmount) {
	switch (_activeScreenNumber) {
	case 700:
	case 1020:
	case 1100:
	case 1700:
	case 2600:
	case 2950:
	case 3100:
	case 3101:
	case 3275:
	case 3600:
		// Use traditional style sectioned screen loading
		Scene::refreshBackground(xAmount, yAmount);
		return;
	default:
		// Break out to new style screen loading
		break;
	}

	/* New style background loading */

	// Get the screen data
	byte *dataP = g_resourceManager->getResource(RT18, _activeScreenNumber, 0);
	int screenSize = g_vm->_memoryManager.getSize(dataP);

	// Lock the background for update
	Graphics::Surface s = _backSurface.lockSurface();
	assert(screenSize == (s.w * s.h));

	// Copy the data
	byte *destP = (byte *)s.getBasePtr(0, 0);
	Common::copy(dataP, dataP + (s.w * s.h), destP);
	_backSurface.unlockSurface();

	// Free the resource data
	DEALLOCATE(dataP);
}

/**
 * Saves the current player position and view in the details for the specified character index
 */
void SceneExt::saveCharacter(int characterIndex) {
	R2_GLOBALS._player._characterPos[characterIndex] = R2_GLOBALS._player._position;
	R2_GLOBALS._player._characterStrip[characterIndex] = R2_GLOBALS._player._strip;
	R2_GLOBALS._player._characterFrame[characterIndex] = R2_GLOBALS._player._frame;
}

/*--------------------------------------------------------------------------*/

void SceneHandlerExt::postInit(SceneObjectList *OwnerList) {
	SceneHandler::postInit(OwnerList);
}

void SceneHandlerExt::process(Event &event) {
	if (T2_GLOBALS._uiElements._active) {
		T2_GLOBALS._uiElements.process(event);
		if (event.handled)
			return;
	}

	SceneExt *scene = static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene);
	if (scene && R2_GLOBALS._player._uiEnabled) {
		// Handle any scene areas that have been registered
		SynchronizedList<SceneArea *>::iterator saIter;
		for (saIter = scene->_sceneAreas.begin(); saIter != scene->_sceneAreas.end() && !event.handled; ++saIter) {
			(*saIter)->process(event);
		}
	}

	if (!event.handled)
		SceneHandler::process(event);
}

/*--------------------------------------------------------------------------*/

DisplayHotspot::DisplayHotspot(int regionId, ...) {
	_sceneRegionId = regionId;

	// Load up the actions
	va_list va;
	va_start(va, regionId);

	int param = va_arg(va, int);
	while (param != LIST_END) {
		_actions.push_back(param);
		param = va_arg(va, int);
	}

	va_end(va);
}

bool DisplayHotspot::performAction(int action) {
	for (uint i = 0; i < _actions.size(); i += 3) {
		if (_actions[i] == action) {
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
			return true;
		}
	}

	return false;
}

/*--------------------------------------------------------------------------*/

DisplayObject::DisplayObject(int firstAction, ...) {
	// Load up the actions
	va_list va;
	va_start(va, firstAction);

	int param = firstAction;
	while (param != LIST_END) {
		_actions.push_back(param);
		param = va_arg(va, int);
	}

	va_end(va);
}

bool DisplayObject::performAction(int action) {
	for (uint i = 0; i < _actions.size(); i += 3) {
		if (_actions[i] == action) {
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
			return true;
		}
	}

	return false;
}

/*--------------------------------------------------------------------------*/

Ringworld2InvObjectList::Ringworld2InvObjectList():
		_none(1, 1),
		_inv1(1, 2),
		_inv2(1, 3),
		_negatorGun(1, 4),
		_steppingDisks(1, 5),
		_inv5(1, 6),
		_inv6(1, 7),
		_inv7(1, 8),
		_inv8(1, 9),
		_inv9(1, 10),
		_inv10(1, 11),
		_inv11(1, 12),
		_inv12(1, 13),
		_inv13(1, 14),
		_inv14(1, 15),
		_inv15(1, 16),
		_inv16(1, 17),
		_inv17(2, 2),
		_inv18(2, 3),
		_inv19(2, 4),
		_inv20(2, 5),
		_inv21(2, 5),
		_inv22(2, 6),
		_inv23(2, 7),
		_inv24(2, 8),
		_inv25(2, 9),
		_inv26(2, 10),
		_inv27(2, 11),
		_inv28(2, 12),
		_inv29(2, 13),
		_inv30(2, 14),
		_inv31(2, 15),
		_inv32(2, 16),
		_inv33(3, 2),
		_inv34(3, 3),
		_inv35(3, 4),
		_inv36(3, 5),
		_inv37(3, 6),
		_inv38(3, 7),
		_inv39(1, 10),
		_inv40(3, 8),
		_inv41(3, 9),
		_inv42(3, 10),
		_inv43(3, 11),
		_inv44(3, 12),
		_inv45(3, 13),
		_inv46(3, 17),
		_inv47(3, 14),
		_inv48(3, 14),
		_inv49(3, 15),
		_inv50(3, 15),
		_inv51(3, 17),
		_inv52(4, 2) {

	// Add the items to the list
	_itemList.push_back(&_none);
	_itemList.push_back(&_inv1);
	_itemList.push_back(&_inv2);
	_itemList.push_back(&_negatorGun);
	_itemList.push_back(&_steppingDisks);
	_itemList.push_back(&_inv5);
	_itemList.push_back(&_inv6);
	_itemList.push_back(&_inv7);
	_itemList.push_back(&_inv8);
	_itemList.push_back(&_inv9);
	_itemList.push_back(&_inv10);
	_itemList.push_back(&_inv11);
	_itemList.push_back(&_inv12);
	_itemList.push_back(&_inv13);
	_itemList.push_back(&_inv14);
	_itemList.push_back(&_inv15);
	_itemList.push_back(&_inv16);
	_itemList.push_back(&_inv17);
	_itemList.push_back(&_inv18);
	_itemList.push_back(&_inv19);
	_itemList.push_back(&_inv20);
	_itemList.push_back(&_inv21);
	_itemList.push_back(&_inv22);
	_itemList.push_back(&_inv23);
	_itemList.push_back(&_inv24);
	_itemList.push_back(&_inv25);
	_itemList.push_back(&_inv26);
	_itemList.push_back(&_inv27);
	_itemList.push_back(&_inv28);
	_itemList.push_back(&_inv29);
	_itemList.push_back(&_inv30);
	_itemList.push_back(&_inv31);
	_itemList.push_back(&_inv32);
	_itemList.push_back(&_inv33);
	_itemList.push_back(&_inv34);
	_itemList.push_back(&_inv35);
	_itemList.push_back(&_inv36);
	_itemList.push_back(&_inv37);
	_itemList.push_back(&_inv38);
	_itemList.push_back(&_inv39);
	_itemList.push_back(&_inv40);
	_itemList.push_back(&_inv41);
	_itemList.push_back(&_inv42);
	_itemList.push_back(&_inv43);
	_itemList.push_back(&_inv44);
	_itemList.push_back(&_inv45);
	_itemList.push_back(&_inv46);
	_itemList.push_back(&_inv47);
	_itemList.push_back(&_inv48);
	_itemList.push_back(&_inv49);
	_itemList.push_back(&_inv50);
	_itemList.push_back(&_inv51);
	_itemList.push_back(&_inv52);

	_selectedItem = NULL;
}

void Ringworld2InvObjectList::reset() {
	// Reset all object scene numbers
	SynchronizedList<InvObject *>::iterator i;
	for (i = _itemList.begin(); i != _itemList.end(); ++i) {
		(*i)->_sceneNumber = 0;
	}

	// Set up default inventory
	setObjectScene(R2_OPTO_DISK, 800);
	setObjectScene(R2_READER, 400);
	setObjectScene(R2_NEGATOR_GUN, 100);
	setObjectScene(R2_STEPPING_DISKS, 100);
	setObjectScene(R2_ATTRACTOR_UNIT, 400);
	setObjectScene(R2_SENSOR_PROBE, 400);
	setObjectScene(R2_7, 500);
	setObjectScene(R2_8, 700);
	setObjectScene(R2_9, 800);
	setObjectScene(R2_10, 100);
	setObjectScene(R2_11, 400);
	setObjectScene(R2_12, 500);
	setObjectScene(R2_13, 1550);
	setObjectScene(R2_OPTICAL_FIBRE, 850);
	setObjectScene(R2_CLAMP, 850);
	setObjectScene(R2_16, 0);
	setObjectScene(R2_17, 1550);
	setObjectScene(R2_18, 1550);
	setObjectScene(R2_19, 1550);
	setObjectScene(R2_20, 500);
	setObjectScene(R2_21, 500);
	setObjectScene(R2_22, 1550);
	setObjectScene(R2_23, 1580);
	setObjectScene(R2_24, 9999);
	setObjectScene(R2_25, 1550);
	setObjectScene(R2_26, 1550);
	setObjectScene(R2_27, 1580);
	setObjectScene(R2_28, 1550);
	setObjectScene(R2_29, 2525);
	setObjectScene(R2_30, 2440);
	setObjectScene(R2_31, 2455);
	setObjectScene(R2_32, 2535);
	setObjectScene(R2_33, 2530);
	setObjectScene(R2_34, 1950);
	setObjectScene(R2_35, 1950);
	setObjectScene(R2_36, 9999);
	setObjectScene(R2_37, 2430);
	setObjectScene(R2_38, 9999);
	setObjectScene(R2_39, 2);
	setObjectScene(R2_40, 9999);
	setObjectScene(R2_41, 3150);
	setObjectScene(R2_42, 0);
	setObjectScene(R2_43, 3260);
	setObjectScene(R2_44, 2);
	setObjectScene(R2_45, 1550);
	setObjectScene(R2_46, 0);
	setObjectScene(R2_47, 3150);
	setObjectScene(R2_48, 2435);
	setObjectScene(R2_49, 2440);
	setObjectScene(R2_50, 2435);
	setObjectScene(R2_51, 1580);
	setObjectScene(R2_52, 3260);
}

void Ringworld2InvObjectList::setObjectScene(int objectNum, int sceneNumber) {
	// Find the appropriate object
	int num = objectNum;
	SynchronizedList<InvObject *>::iterator i = _itemList.begin();
	while (num-- > 0) ++i;
	(*i)->_sceneNumber = sceneNumber;

	// If the item is the currently active one, default back to the use cursor
	if (R2_GLOBALS._events.getCursor() == objectNum)
		R2_GLOBALS._events.setCursor(CURSOR_USE);

	// Update the user interface if necessary
	T2_GLOBALS._uiElements.updateInventory();
}

/*--------------------------------------------------------------------------*/

void Ringworld2Game::start() {
	int slot = -1;

	if (ConfMan.hasKey("save_slot")) {
		slot = ConfMan.getInt("save_slot");
		Common::String file = g_vm->generateSaveName(slot);
		Common::InSaveFile *in = g_vm->_system->getSavefileManager()->openForLoading(file);
		if (in)
			delete in;
		else
			slot = -1;
	}

	if (slot >= 0)
		R2_GLOBALS._sceneHandler->_loadGameSlot = slot;
	else {
		// Switch to the first game scene
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		R2_GLOBALS._uiElements._active = true;
		R2_GLOBALS._sceneManager.setNewScene(100);
	}

	g_globals->_events.showCursor();
}

void Ringworld2Game::restart() {
	g_globals->_scenePalette.clearListeners();
	g_globals->_soundHandler.stop();

	// Change to the first game scene
	g_globals->_sceneManager.changeScene(100);
}

void Ringworld2Game::endGame(int resNum, int lineNum) {
	g_globals->_events.setCursor(CURSOR_WALK);
	Common::String msg = g_resourceManager->getMessage(resNum, lineNum);
	bool savesExist = g_saver->savegamesExist();

	if (!savesExist) {
		// No savegames exist, so prompt the user to restart or quit
		if (MessageDialog::show(msg, QUIT_BTN_STRING, RESTART_BTN_STRING) == 0)
			g_vm->quitGame();
		else
			restart();
	} else {
		// Savegames exist, so prompt for Restore/Restart
		bool breakFlag;
		do {
			if (g_vm->shouldQuit()) {
				breakFlag = true;
			} else if (MessageDialog::show(msg, RESTART_BTN_STRING, RESTORE_BTN_STRING) == 0) {
				restart();
				breakFlag = true;
			} else {
				handleSaveLoad(false, g_globals->_sceneHandler->_loadGameSlot, g_globals->_sceneHandler->_saveName);
				breakFlag = g_globals->_sceneHandler->_loadGameSlot >= 0;
			}
		} while (!breakFlag);
	}

	g_globals->_events.setCursorFromFlag();
}

void Ringworld2Game::processEvent(Event &event) {
	if (event.eventType == EVENT_KEYPRESS) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_F1:
			// F1 - Help
			HelpDialog::show();
			break;

		case Common::KEYCODE_F2:
			// F2 - Sound Options
			SoundDialog::execute();
			break;

		case Common::KEYCODE_F3:
			// F3 - Quit
			quitGame();
			event.handled = false;
			break;

		case Common::KEYCODE_F4:
			// F4 - Restart
			restartGame();
			g_globals->_events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F7:
			// F7 - Restore
			restoreGame();
			g_globals->_events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F8:
			// F8 - Credits
			warning("TODO: Show Credits");
			break;

		case Common::KEYCODE_F10:
			// F10 - Pause
			GfxDialog::setPalette();
			MessageDialog::show(GAME_PAUSED_MSG, OK_BTN_STRING);
			g_globals->_events.setCursorFromFlag();
			break;

		default:
			break;
		}
	}
}

void Ringworld2Game::rightClick() {
	RightClickDialog *dlg = new RightClickDialog();
	dlg->execute();
	delete dlg;
}

/*--------------------------------------------------------------------------*/

NamedHotspot::NamedHotspot() : SceneHotspot() {
	_resNum = 0;
	_lookLineNum = _useLineNum = _talkLineNum = -1;
}

bool NamedHotspot::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_WALK:
		// Nothing
		return false;
	case CURSOR_LOOK:
		if (_lookLineNum == -1)
			return SceneHotspot::startAction(action, event);

		SceneItem::display2(_resNum, _lookLineNum);
		return true;
	case CURSOR_USE:
		if (_useLineNum == -1)
			return SceneHotspot::startAction(action, event);

		SceneItem::display2(_resNum, _useLineNum);
		return true;
	case CURSOR_TALK:
		if (_talkLineNum == -1)
			return SceneHotspot::startAction(action, event);

		SceneItem::display2(_resNum, _talkLineNum);
		return true;
	default:
		return SceneHotspot::startAction(action, event);
	}
}

void SceneActor::postInit(SceneObjectList *OwnerList) {
	_lookLineNum = _talkLineNum = _useLineNum = -1;
	SceneObject::postInit();
}

void SceneActor::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	s.syncAsSint16LE(_resNum);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_talkLineNum);
	s.syncAsSint16LE(_useLineNum);
}

bool SceneActor::startAction(CursorType action, Event &event) {
	bool handled = true;

	switch (action) {
	case CURSOR_LOOK:
		if (_lookLineNum == -1)
			handled = false;
		else
			SceneItem::display2(_resNum, _lookLineNum);
		break;
	case CURSOR_USE:
		if (_useLineNum == -1)
			handled = false;
		else
			SceneItem::display2(_resNum, _useLineNum);
		break;
	case CURSOR_TALK:
		if (_talkLineNum == -1)
			handled = false;
		else
			SceneItem::display2(_resNum, _talkLineNum);
		break;
	default:
		handled = false;
		break;
	}

	if (!handled)
		handled = ((SceneExt *)R2_GLOBALS._sceneManager._scene)->display(action, event);
	return handled;
}

void SceneActor::setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode, SceneItem *item) {
	_resNum = resNum;
	_lookLineNum = lookLineNum;
	_talkLineNum = talkLineNum;
	_useLineNum = useLineNum;

	switch (mode) {
	case 2:
		g_globals->_sceneItems.push_front(this);
		break;
	case 4:
		g_globals->_sceneItems.addBefore(item, this);
		break;
	case 5:
		g_globals->_sceneItems.addAfter(item, this);
		break;
	default:
		g_globals->_sceneItems.push_back(this);
		break;
	}
}

void SceneActor::setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum) {
	_resNum = resNum;
	_lookLineNum = lookLineNum;
	_talkLineNum = talkLineNum;
	_useLineNum = useLineNum;
}

/*--------------------------------------------------------------------------*/

SceneArea::SceneArea(): EventHandler() {
	_enabled = true;
	_insideArea = false;
	_savedCursorNum = CURSOR_NONE;
	_cursorState = 0;
}

void SceneArea::synchronize(Serializer &s) {
	EventHandler::synchronize(s);

	_bounds.synchronize(s);
	s.syncAsSint16LE(_enabled);
	s.syncAsSint16LE(_insideArea);
	s.syncAsSint16LE(_cursorNum);
	s.syncAsSint16LE(_savedCursorNum);
	s.syncAsSint16LE(_cursorState);
}

void SceneArea::remove() {
	static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene)->_sceneAreas.remove(this);
}

void SceneArea::process(Event &event) {
	if (!R2_GLOBALS._insetUp && _enabled && R2_GLOBALS._events.isCursorVisible()) {
		CursorType cursor = R2_GLOBALS._events.getCursor();

		if (_bounds.contains(event.mousePos)) {
			// Cursor moving in bounded area
			if (cursor != _cursorNum) {
				_savedCursorNum = cursor;
				_cursorState = 0;
				R2_GLOBALS._events.setCursor(_cursorNum);
			}
			_insideArea = true;
		} else if ((event.mousePos.y < 171) && _insideArea && (_cursorNum == cursor) &&
				(_savedCursorNum != CURSOR_NONE)) {
			// Cursor moved outside bounded area
			R2_GLOBALS._events.setCursor(_savedCursorNum);
		}
	}
}

void SceneArea::setDetails(const Rect &bounds, CursorType cursor) {
	_bounds = bounds;
	_cursorNum = cursor;

	static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene)->_sceneAreas.push_front(this);
}

/*--------------------------------------------------------------------------*/

SceneExit::SceneExit(): SceneArea() {
	_moving = false;
	_destPos = Common::Point(-1, -1);
}

void SceneExit::synchronize(Serializer &s) {
	SceneArea::synchronize(s);

	s.syncAsSint16LE(_moving);
	s.syncAsSint16LE(_destPos.x);
	s.syncAsSint16LE(_destPos.y);
}

void SceneExit::setDetails(const Rect &bounds, CursorType cursor, int sceneNumber) {
	_sceneNumber = sceneNumber;
	SceneArea::setDetails(bounds, cursor);
}

void SceneExit::changeScene() {
	R2_GLOBALS._sceneManager.setNewScene(_sceneNumber);
}

void SceneExit::process(Event &event) {
	if (!R2_GLOBALS._insetUp) {
		SceneArea::process(event);

		if (_enabled) {
			if (event.eventType == EVENT_BUTTON_DOWN) {
				if (!_bounds.contains(event.mousePos))
					_moving = false;
				else if (!R2_GLOBALS._player._canWalk) {
					_moving = false;
					changeScene();
					event.handled = true;
				} else {
					Common::Point dest((_destPos.x == -1) ? event.mousePos.x : _destPos.x,
						(_destPos.y == -1) ? event.mousePos.y : _destPos.y);
					ADD_PLAYER_MOVER(dest.x, dest.y);

					_moving = true;
					event.handled = true;
				}
			}

			if (_moving && (_bounds.contains(R2_GLOBALS._player._position) || (R2_GLOBALS._player._position == _destPos)))
				changeScene();
		}
	}
}

} // End of namespace Ringworld2

} // End of namespace TsAGE
