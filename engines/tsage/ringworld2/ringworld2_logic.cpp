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

namespace TsAGE {

namespace Ringworld2 {

Scene *Ringworld2Game::createScene(int sceneNumber) {
	switch (sceneNumber) {
	/* Scene group #0 */
	// Quinn's room
	case 100: return new Scene100();

	// Computer console
	case 125: return new Scene125();

	default:
		error("Unknown scene number - %d", sceneNumber);
		break;
	}
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool Ringworld2Game::canLoadGameStateCurrently() {
	return true;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool Ringworld2Game::canSaveGameStateCurrently() {
	return true;
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
}

void SceneExt::remove() {
/*
	R2_GLOBALS._uiElements.hide();
	R2_GLOBALS._uiElements.resetClear();

	if (_action) {
		if (_action->_endHandler)
			_action->_endHandler = NULL;
		_action->remove();
	}
	
	_focusObject = NULL;
*/
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
}

bool SceneExt::display(CursorType action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(9000, R2_GLOBALS._randomSource.getRandomNumber(2));
		break;
	case CURSOR_USE:
		SceneItem::display2(9000, R2_GLOBALS._randomSource.getRandomNumber(2) + 6);
		break;
	case CURSOR_TALK:
		SceneItem::display2(9000, R2_GLOBALS._randomSource.getRandomNumber(2) + 3);
		break;
	default:
		return false;
	}

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
	setObjectScene(R2_2, 400);
	setObjectScene(R2_NEGATOR_GUN, 100);
	setObjectScene(R2_STEPPING_DISKS, 100);
	setObjectScene(R2_5, 400);
	setObjectScene(R2_6, 400);
	setObjectScene(R2_7, 500);
	setObjectScene(R2_8, 700);
	setObjectScene(R2_9, 800);
	setObjectScene(R2_10, 100);
	setObjectScene(R2_11, 400);
	setObjectScene(R2_12, 500);
	setObjectScene(R2_13, 1550);
	setObjectScene(R2_14, 850);
	setObjectScene(R2_15, 850);
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
//			MessageDialog::show(HELP_MSG, OK_BTN_STRING);
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

void NamedHotspot::setDetails(int ys, int xs, int ye, int xe, const int resnum, const int lookLineNum, const int useLineNum) {
	setBounds(ys, xe, ye, xs);
	_resNum = resnum;
	_lookLineNum = lookLineNum;
	_useLineNum = useLineNum;
	_talkLineNum = -1;
	g_globals->_sceneItems.addItems(this, NULL);
}

void NamedHotspot::setDetails(const Rect &bounds, int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode, SceneItem *item) {
	setBounds(bounds);
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

void NamedHotspot::setDetails(int sceneRegionId, int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode) {
	_sceneRegionId = sceneRegionId;
	_resNum = resNum;
	_lookLineNum = lookLineNum;
	_talkLineNum = talkLineNum;
	_useLineNum = useLineNum;

	// Handle adding hotspot to scene items list as necessary
	switch (mode) {
	case 2:
		GLOBALS._sceneItems.push_front(this);
		break;
	case 3:
		break;
	default:
		GLOBALS._sceneItems.push_back(this);
		break;
	}
}

void NamedHotspot::synchronize(Serializer &s) {
	SceneHotspot::synchronize(s);
	s.syncAsSint16LE(_resNum);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_useLineNum);

	if (g_vm->getGameID() == GType_BlueForce)
		s.syncAsSint16LE(_talkLineNum);
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
		handled = ((SceneExt *)R2_GLOBALS._sceneManager._scene)->display(action);
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


} // End of namespace Ringworld2

} // End of namespace TsAGE
