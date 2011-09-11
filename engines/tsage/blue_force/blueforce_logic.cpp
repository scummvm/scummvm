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

#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/blue_force/blueforce_scenes0.h"
#include "tsage/blue_force/blueforce_scenes1.h"
#include "tsage/blue_force/blueforce_scenes3.h"
#include "tsage/blue_force/blueforce_scenes6.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/graphics.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

void BlueForceGame::start() {
	// Start the game
	_globals->_sceneManager.changeScene(300);

	_globals->_events.setCursor(CURSOR_WALK);
}

Scene *BlueForceGame::createScene(int sceneNumber) {
	switch (sceneNumber) {
	/* Scene Group #0 */
	case 20:
		// Tsunami Title Screen
		return new Scene20();
	case 50:
		// Map screen
		return new Scene50();
	case 60:
		// Motorcycle
		return new Scene60();
	/* Scene Group #1 */
	case 100:
		// Tsnunami Title Screen #2
		return new Scene100();
	case 109:
		// Introduction Bar Room
		return new Scene109();
	case 110:

	case 114:
	case 115:
	case 125:
	case 140:
	case 150:
	case 160:
	case 180:
		error("Scene group 1 not implemented");
	case 190:
		// Front of Police Station
		return new Scene190();
	case 200:
	case 210:
	case 220:
	case 225:
	case 265:
	case 270:
	case 271:
	case 280:
		error("Scene group 2 not implemented");
	case 300:
		// Outside Police Station
		return new Scene300();
	case 315:
		// Inside Police Station
		return new Scene315();
	case 325:
		// Police Station Conference Room
		return new Scene325();
	case 330:
	case 340:
	case 342:
	case 350:
	case 355:
	case 360:
	case 370:
	case 380:
	case 385:
	case 390:
		error("Scene group 3 not implemented");
	case 410:
	case 415:
	case 440:
	case 450:
		error("Scene group 4 not implemented");
	case 550:
	case 551:
	case 560:
	case 570:
	case 580:
	case 590:
		error("Scene group 5 not implemented");
	case 600:
	case 620:
	case 666:
		// Death scene
		return new Scene666();
	case 690:
		error("Scene group 6 not implemented");
	case 710:
		error("Scene group 7 not implemented");
	case 800:
	case 810:
	case 820:
	case 830:
	case 840:
	case 850:
	case 860:
	case 870:
	case 880:
		error("Scene group 8 not implemented");
	case 900:
	case 910:
	case 920:
	case 930:
	case 935:
	case 940:
		error("Scene group 9 not implemented");
	default:
		error("Unknown scene number - %d", sceneNumber);
		break;
	}
}

void BlueForceGame::rightClick() {
	RightClickDialog *dlg = new RightClickDialog();
	dlg->execute();
	delete dlg;
}

void BlueForceGame::processEvent(Event &event) {
	if (event.eventType == EVENT_KEYPRESS) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_F1:
			// F1 - Help
			MessageDialog::show(HELP_MSG, OK_BTN_STRING);
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
			_globals->_events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F7:
			// F7 - Restore
			restoreGame();
			_globals->_events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F10:
			// F10 - Pause
			GfxDialog::setPalette();
			MessageDialog::show(GAME_PAUSED_MSG, OK_BTN_STRING);
			_globals->_events.setCursorFromFlag();
			break;

		default:
			break;
		}
	}
}

/*--------------------------------------------------------------------------*/

AObjectArray::AObjectArray(): EventHandler() {
	_inUse = false;
	clear();
}

void AObjectArray::clear() {
	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i)
		_objList[i] = NULL;
}

void AObjectArray::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i)
		SYNC_POINTER(_objList[i]);	
}

void AObjectArray::process(Event &event) {
	if (_inUse)
		error("Array error");
	_inUse = true;

	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i) {
		if (_objList[i])
			_objList[i]->process(event);
	}

	_inUse = false;
}

void AObjectArray::dispatch() {
	if (_inUse)
		error("Array error");
	_inUse = true;

	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i) {
		if (_objList[i])
			_objList[i]->dispatch();
	}

	_inUse = false;
}

int AObjectArray::getNewIndex() {
	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i) {
		if (!_objList[i])
			return i;
	}
	error("AObjectArray too full.");
}

void AObjectArray::add(EventHandler *obj) {
	int idx = getNewIndex();
	_objList[idx] = obj;
}

void AObjectArray::remove(EventHandler *obj) {
	for (int i = 0; i < OBJ_ARRAY_SIZE; ++i) {
		if (_objList[i] == obj) {
			_objList[i] = NULL;
			return;
		}
	}
}

/*--------------------------------------------------------------------------*/

Timer::Timer() {
	_endFrame = 0;
	_endAction = NULL;
	_tickAction = NULL;
}

void Timer::remove() {
	_endFrame = 0;
	_endAction = NULL;

	((Scene100 *)BF_GLOBALS._sceneManager._scene)->removeTimer(this);
}

void Timer::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	SYNC_POINTER(_tickAction);
	SYNC_POINTER(_endAction);
	s.syncAsUint32LE(_endFrame);
}

void Timer::signal() {
	assert(_endAction);
	Action *action = _endAction;
	remove();
	action->signal();
}

void Timer::dispatch() {
	if (_tickAction)
		_tickAction->dispatch();

	if (_endFrame) {
		uint32 frameNumber = BF_GLOBALS._events.getFrameNumber();
		if (frameNumber > _endFrame)
			// Timer has expired
			signal();
	}
}

void Timer::set(uint32 delay, Action *endAction) {
	assert(delay != 0);

	_endFrame = BF_GLOBALS._sceneHandler->getFrameDifference() + delay;
	_endAction = endAction;

	((SceneExt *)BF_GLOBALS._sceneManager._scene)->addTimer(this);
}

/*--------------------------------------------------------------------------*/

TimerExt::TimerExt(): Timer() {
	_action = NULL;
}

void TimerExt::set(uint32 delay, Action *endAction, Action *newAction) {
	_newAction = newAction;
	Timer::set(delay, endAction);
}

void TimerExt::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	SYNC_POINTER(_action);
}

void TimerExt::remove() {
	_action = NULL;
	remove();
}

void TimerExt::signal() {
	Action *endAction = _endAction;
	Action *newAction = _newAction;
	remove();

	// If the end action doesn't have an action anymore, set it to the specified new action
	assert(endAction);
	if (!endAction->_action)
		endAction->setAction(newAction);
}

void TimerExt::dispatch() {

}

/*--------------------------------------------------------------------------*/

void SceneItemType2::startMove(SceneObject *sceneObj, va_list va) {
}

/*--------------------------------------------------------------------------*/

void NamedObject::postInit(SceneObjectList *OwnerList) {
	_lookLineNum = _talkLineNum = _useLineNum = -1;
	SceneObject::postInit();
}

void NamedObject::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	s.syncAsSint16LE(_resNum);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_talkLineNum);
	s.syncAsSint16LE(_useLineNum);
}

bool NamedObject::startAction(CursorType action, Event &event) {
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
		((SceneExt *)BF_GLOBALS._sceneManager._scene)->display(action);
	return handled;
}

void NamedObject::setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode, SceneItem *item) {
	_resNum = resNum;
	_lookLineNum = lookLineNum;
	_talkLineNum = talkLineNum;
	_useLineNum = useLineNum;

	switch (mode) {
	case 2:
		_globals->_sceneItems.push_front(this);
		break;
	case 4:
		_globals->_sceneItems.addBefore(item, this);
		break;
	case 5:
		_globals->_sceneItems.addAfter(item, this);
		break;
	default:
		_globals->_sceneItems.push_back(this);
		break;
	}
}


/*--------------------------------------------------------------------------*/

CountdownObject::CountdownObject(): NamedObject() {
	_countDown = 0;
}

void CountdownObject::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	s.syncAsSint16LE(_countDown);
}

void CountdownObject::dispatch() {
	int frameNum = _frame;
	SceneObject::dispatch();

	if ((frameNum != _frame) && (_countDown > 0)) {
		if (--_countDown == 0) {
			animate(ANIM_MODE_NONE, 0);
			_frame = 1;
		}
	}
}

void CountdownObject::fixCountdown(int mode, ...) {
	if (mode == 8) {
		va_list va;
		va_start(va, mode);

		_countDown = va_arg(va, int);
		animate(ANIM_MODE_8, _countDown, NULL);
		va_end(va);
	}
}

/*--------------------------------------------------------------------------*/

FollowerObject::FollowerObject(): NamedObject() {
	_object = NULL;
}

void FollowerObject::synchronize(Serializer &s) {
	NamedObject::synchronize(s);
	SYNC_POINTER(_object);
}

void FollowerObject::remove() {
	NamedObject::remove();
	_object = NULL;
}

void FollowerObject::dispatch() {
	SceneObject::dispatch();
	assert(_object);

	if ((_object->_flags & OBJFLAG_HIDE) || ((_object->_visage != 307) &&
		((_object->_visage != 308) || (_object->_strip != 1)))) {
		hide();
	} else if ((_object->_visage != 308) || (_object->_strip != 1)) {
		show();
		setStrip(_object->_strip);
		setPosition(_object->_position, _object->_yDiff);
	}
}

void FollowerObject::reposition() {
	assert(_object);
	setStrip(_object->_strip);
	setPosition(_object->_position, _object->_yDiff);
	reposition();
}

void FollowerObject::setup(SceneObject *object, int visage, int frameNum, int yDiff) {
	SceneObject::postInit();
	_object = object;
	_yDiff = yDiff;
	setVisage(visage);
	setFrame(frameNum);

	dispatch();
}

/*--------------------------------------------------------------------------*/

SceneExt::SceneExt(): Scene() {
	_stripManager._onBegin = SceneExt::startStrip;
	_stripManager._onEnd = SceneExt::endStrip;

	_field372 = _field37A = 0;
	_savedPlayerEnabled = false;
	_savedUiEnabled = false;
	_savedCanWalk = false;
	_eventHandler = NULL;
	_cursorVisage.setVisage(1, 8);
}

void SceneExt::postInit(SceneObjectList *OwnerList) {
	Scene::postInit(OwnerList);
	if (BF_GLOBALS._dayNumber) {
		// Blank out the bottom portion of the screen
		BF_GLOBALS._interfaceY = BF_INTERFACE_Y;

		Rect r(0, BF_INTERFACE_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
		BF_GLOBALS.gfxManager().getSurface().fillRect(r, 0);
	}
}

void SceneExt::process(Event &event) {
	_objArray2.process(event);
	if (!event.handled)
		Scene::process(event);
}

void SceneExt::dispatch() {
	_timerList.dispatch();

	if (_field37A) {
		if ((--_field37A == 0) && BF_GLOBALS._dayNumber) {
			if (BF_GLOBALS._v4E238 && (BF_GLOBALS._v4CF9E == 1)) {
				warning("sub_1B052");
			}
			
			_field37A = 0;
		}
	}

	Scene::dispatch();
}

void SceneExt::loadScene(int sceneNum) {
	Scene::loadScene(sceneNum);
	
	_v51C34.top = 0;
	_v51C34.bottom = 300;
}

void SceneExt::checkGun() {
	// Remove a bullet from the currently loaded clip
	if (BF_GLOBALS.getFlag(fLoadedSpare) && (BF_GLOBALS._clip2Bullets > 0)) {
		if (--BF_GLOBALS._clip2Bullets == 0)
			BF_GLOBALS.clearFlag(fGunLoaded);
	} else {
		if (BF_GLOBALS._clip1Bullets > 0)
			--BF_GLOBALS._clip1Bullets;

		if (!BF_GLOBALS._clip1Bullets)
			BF_GLOBALS.clearFlag(fGunLoaded);
	}

	BF_GLOBALS._sound3.play(4);
}

bool SceneExt::display(CursorType action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(9000, BF_GLOBALS._randomSource.getRandomNumber(2));
		break;
	case CURSOR_USE:
		SceneItem::display2(9000, BF_GLOBALS._randomSource.getRandomNumber(2) + 6);
		break;
	case CURSOR_TALK:
		SceneItem::display2(9000, BF_GLOBALS._randomSource.getRandomNumber(2) + 3);
		break;
	case INV_COLT45:
		gunDisplay();
		break;
	default:
		if (action < BF_LAST_INVENT)
			SceneItem::display2(9002, (int)action);
		else
			return false;
		break;
	}

	return true;
}

void SceneExt::fadeOut() {
	uint32 black = 0;
	BF_GLOBALS._scenePalette.fade((const byte *)&black, false, 100);
}

void SceneExt::gunDisplay() {
	if (!BF_GLOBALS.getFlag(gunDrawn)) {
		// Gun not drawn
		SceneItem::display2(1, BF_GLOBALS.getFlag(fCanDrawGun) ? 0 : 4);
	} else if (!BF_GLOBALS.getFlag(fGunLoaded)) {
		// Gun not loaded
		SceneItem::display2(1, 1);
	} else if (!BF_GLOBALS.getHasBullets()) {
		// Out of ammunition
		SceneItem::display2(1, 2);
	} else {
		// Check scene for whether gun can fire
		checkGun();
	}
}

void SceneExt::startStrip() {
	SceneExt *scene = (SceneExt *)BF_GLOBALS._sceneManager._scene;
	scene->_field372 = 1;
	scene->_savedPlayerEnabled = BF_GLOBALS._player._enabled;
	
	if (scene->_savedPlayerEnabled) {
		scene->_savedUiEnabled = BF_GLOBALS._player._uiEnabled;
		scene->_savedCanWalk = BF_GLOBALS._player._canWalk;
		BF_GLOBALS._player.disableControl();

		if (!BF_GLOBALS._v50696 && BF_GLOBALS._uiElements._active)
			BF_GLOBALS._uiElements.hide();
	}
}

void SceneExt::endStrip() {
	SceneExt *scene = (SceneExt *)BF_GLOBALS._sceneManager._scene;
	scene->_field372 = 0;

	if (scene->_savedPlayerEnabled) {
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player._uiEnabled = scene->_savedUiEnabled;
		BF_GLOBALS._player._canWalk = scene->_savedCanWalk;

		if (!BF_GLOBALS._v50696 && BF_GLOBALS._uiElements._active)
			BF_GLOBALS._uiElements.show();
	}
}

/*--------------------------------------------------------------------------*/

GroupedScene::GroupedScene() {

}

void GroupedScene::postInit(SceneObjectList *OwnerList) {
	_field794 = 0;
	_field412 = 1;
	SceneExt::postInit(OwnerList);
}

void GroupedScene::remove() {
	SceneExt::remove();
	if (_field794 == 1) {
		for (SynchronizedList<SceneObject *>::iterator i = BF_GLOBALS._sceneObjects->begin();
				i != BF_GLOBALS._sceneObjects->end(); ++i)
			(*i)->remove();
		
		BF_GLOBALS._sceneObjects->draw();
		BF_GLOBALS._scenePalette.loadPalette(2);
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._v51C42 = 1;
	}

	BF_GLOBALS._scenePalette._field412 = 1;
}

/*--------------------------------------------------------------------------*/

void SceneHandlerExt::postInit(SceneObjectList *OwnerList) {
	SceneHandler::postInit(OwnerList);

	// Load the low end palette data
	BF_GLOBALS._scenePalette.loadPalette(2);
	BF_GLOBALS._scenePalette.refresh();
}

void SceneHandlerExt::process(Event &event) {
	if (BF_GLOBALS._uiElements._active) {
		BF_GLOBALS._uiElements.process(event);
		if (event.handled)
			return;
	}

	// If the strip proxy is currently being controlled by the strip manager, 
	// then pass all events to it first
	if (BF_GLOBALS._stripProxy._action) {
		BF_GLOBALS._stripProxy._action->process(event);
		if (event.handled)
			return;
	}

	SceneHandler::process(event);
}

void SceneHandlerExt::playerAction(Event &event) {
	if (BF_GLOBALS._events.getCursor() == INV_DOG_WHISTLE) {
		SceneItem::display2(1, 6);
		event.handled = true;
	}
}

void SceneHandlerExt::processEnd(Event &event) {
	// Check for a fallback text display for the given cursor/item being used in the scene
	if (!event.handled && BF_GLOBALS._sceneManager._scene) {
		CursorType cursor = BF_GLOBALS._events.getCursor();
		if (((SceneExt *)BF_GLOBALS._sceneManager._scene)->display(cursor))
			event.handled = true;
	}
}

/*--------------------------------------------------------------------------*/

BlueForceInvObjectList::BlueForceInvObjectList():
		_none(9, 5, 1),
		_colt45(9, 1, 1),
		_ammoClip(9, 4, 2),
		_spareClip(9, 4, 3),
		_handcuffs(9, 1, 4),
		_greensGun(9, 1, 5),
		_ticketBook(9, 1, 6),
		_mirandaCard(9, 1, 7),
		_forestRap(9, 1, 8),
		_greenId(9, 1, 9),
		_baseballCard(9, 1, 10),
		_bookingGreen(9, 1, 11),
		_flare(9, 1, 12),
		_cobbRap(9, 1, 13),
		_bullet22(9, 1, 14),
		_autoRifle(9, 1, 15),
		_wig(9, 1, 16),
		_frankieId(9, 1, 17),
		_tyroneId(9, 1, 18),
		_snub22(9, 1, 19),
		_bug(1, 1, 1),
		_bookingFrankie(9, 2, 1),
		_bookingGang(9, 2, 2),
		_fbiTeletype(9, 2, 3),
		_daNote(9, 2, 4),
		_printOut(9, 2, 5),
		_warehouseKeys(9, 2, 6),
		_centerPunch(9, 2, 7),
		_tranqGun(9, 2, 8),
		_hook(9, 2, 9),
		_rags(9, 2, 10),
		_jar(9, 2, 11),
		_screwdriver(9, 2, 12),
		_dFloppy(9, 2, 13),
		_blankDisk(9, 2, 14),
		_stick(9, 2, 15),
		_crate1(9, 2, 16),
		_crate2(9, 2, 17),
		_shoebox(9, 2, 18),
		_badge(9, 2, 19),
		_bug2(1, 1, 1),
		_rentalCoupon(9, 3, 1),
		_nickel(9, 3, 2),
		_lyleCard(9, 3, 3),
		_carterNote(9, 3, 4),
		_mugshot(9, 3, 5),
		_clipping(9, 3, 6),
		_microfilm(9, 3, 7),
		_waveKeys(9, 3, 8),
		_rentalKeys(9, 3, 9),
		_napkin(9, 3, 10),
		_dmvPrintout(9, 3, 11),
		_fishingNet(9, 3, 12),
		_id(9, 3, 13),
		_bullets9mm(9, 3, 14),
		_schedule(9, 3, 15),
		_grenades(9, 3, 16),
		_yellowCord(9, 3, 17),
		_halfYellowCord(9, 3, 18),
		_blackCord(9, 3, 19),
		_bug3(1, 1, 1),
		_halfBlackCord(9, 4, 1),
		_warrant(9, 4, 2),
		_jacket(9, 4, 3),
		_greensKnife(9, 4, 4),
		_dogWhistle(9, 4, 5),
		_ammoBelt(9, 1, 2),
		_lastInvent(9, 4, 7) {

	// Add the items to the list
	_itemList.push_back(&_none);
	_itemList.push_back(&_colt45);
	_itemList.push_back(&_ammoClip);
	_itemList.push_back(&_spareClip);
	_itemList.push_back(&_handcuffs);
	_itemList.push_back(&_greensGun);
	_itemList.push_back(&_ticketBook);
	_itemList.push_back(&_mirandaCard);
	_itemList.push_back(&_forestRap);
	_itemList.push_back(&_greenId);
	_itemList.push_back(&_baseballCard);
	_itemList.push_back(&_bookingGreen);
	_itemList.push_back(&_flare);
	_itemList.push_back(&_cobbRap);
	_itemList.push_back(&_bullet22);
	_itemList.push_back(&_autoRifle);
	_itemList.push_back(&_wig);
	_itemList.push_back(&_frankieId);
	_itemList.push_back(&_tyroneId);
	_itemList.push_back(&_snub22);
	_itemList.push_back(&_bug);
	_itemList.push_back(&_bookingFrankie);
	_itemList.push_back(&_bookingGang);
	_itemList.push_back(&_fbiTeletype);
	_itemList.push_back(&_daNote);
	_itemList.push_back(&_printOut);
	_itemList.push_back(&_warehouseKeys);
	_itemList.push_back(&_centerPunch);
	_itemList.push_back(&_tranqGun);
	_itemList.push_back(&_hook);
	_itemList.push_back(&_rags);
	_itemList.push_back(&_jar);
	_itemList.push_back(&_screwdriver);
	_itemList.push_back(&_dFloppy);
	_itemList.push_back(&_blankDisk);
	_itemList.push_back(&_stick);
	_itemList.push_back(&_crate1);
	_itemList.push_back(&_crate2);
	_itemList.push_back(&_shoebox);
	_itemList.push_back(&_badge);
	_itemList.push_back(&_bug2);
	_itemList.push_back(&_rentalCoupon);
	_itemList.push_back(&_nickel);
	_itemList.push_back(&_lyleCard);
	_itemList.push_back(&_carterNote);
	_itemList.push_back(&_mugshot);
	_itemList.push_back(&_clipping);
	_itemList.push_back(&_microfilm);
	_itemList.push_back(&_waveKeys);
	_itemList.push_back(&_rentalKeys);
	_itemList.push_back(&_napkin);
	_itemList.push_back(&_dmvPrintout);
	_itemList.push_back(&_fishingNet);
	_itemList.push_back(&_id);
	_itemList.push_back(&_bullets9mm);
	_itemList.push_back(&_schedule);
	_itemList.push_back(&_grenades);
	_itemList.push_back(&_yellowCord);
	_itemList.push_back(&_halfYellowCord);
	_itemList.push_back(&_blackCord);
	_itemList.push_back(&_bug3);
	_itemList.push_back(&_halfBlackCord);
	_itemList.push_back(&_warrant);
	_itemList.push_back(&_jacket);
	_itemList.push_back(&_greensKnife);
	_itemList.push_back(&_dogWhistle);
	_itemList.push_back(&_ammoBelt);
	_itemList.push_back(&_lastInvent);
}

void BlueForceInvObjectList::reset() {
	// Reset all object scene numbers
	SynchronizedList<InvObject *>::iterator i;
	for (i = _itemList.begin(); i != _itemList.end(); ++i) {
		(*i)->_sceneNumber = 0;
	}

	// Set up default inventory
	setObjectScene(INV_COLT45, 1);
	setObjectScene(INV_HANDCUFFS, 1);
	setObjectScene(INV_AMMO_BELT, 1);
	setObjectScene(INV_ID, 1);

	// Set default room for other objects
	setObjectScene(INV_TICKET_BOOK, 60);
	setObjectScene(INV_MIRANDA_CARD, 60);
	setObjectScene(INV_FOREST_RAP, 320);
	setObjectScene(INV_GREEN_ID, 370);
	setObjectScene(INV_BASEBALL_CARD, 840);
	setObjectScene(INV_BOOKING_GREEN, 390);
	setObjectScene(INV_FLARE, 355);
	setObjectScene(INV_COBB_RAP, 810);
	setObjectScene(INV_22_BULLET, 415);
	setObjectScene(INV_AUTO_RIFLE, 415);
	setObjectScene(INV_WIG, 415);
	setObjectScene(INV_FRANKIE_ID, 410);
	setObjectScene(INV_TYRONE_ID, 410);
	setObjectScene(INV_22_SNUB, 410);
	setObjectScene(INV_FBI_TELETYPE, 320);
	setObjectScene(INV_DA_NOTE, 320);
	setObjectScene(INV_PRINT_OUT, 570);
	setObjectScene(INV_WHAREHOUSE_KEYS, 360);
	setObjectScene(INV_CENTER_PUNCH, 0);
	setObjectScene(INV_TRANQ_GUN, 830);
	setObjectScene(INV_HOOK, 350);
	setObjectScene(INV_RAGS, 870);
	setObjectScene(INV_JAR, 870);
	setObjectScene(INV_SCREWDRIVER, 355);
	setObjectScene(INV_D_FLOPPY, 570);
	setObjectScene(INV_BLANK_DISK, 560);
	setObjectScene(INV_STICK, 710);
	setObjectScene(INV_CRATE1, 710);
	setObjectScene(INV_CRATE2, 870);
	setObjectScene(INV_SHOEBOX, 270);
	setObjectScene(INV_BADGE, 560);
	setObjectScene(INV_RENTAL_COUPON, 0);
	setObjectScene(INV_NICKEL, 560);
	setObjectScene(INV_LYLE_CARD, 270);
	setObjectScene(INV_CARTER_NOTE, 830);
	setObjectScene(INV_MUG_SHOT, 810);
	setObjectScene(INV_CLIPPING, 810);
	setObjectScene(INV_MICROFILM, 810);
	setObjectScene(INV_WAVE_KEYS, 840);
	setObjectScene(INV_RENTAL_KEYS, 840);
	setObjectScene(INV_NAPKIN, 115);
	setObjectScene(INV_DMV_PRINTOUT, 810);
	setObjectScene(INV_FISHING_NET, 830);
	setObjectScene(INV_9MM_BULLETS, 930);
	setObjectScene(INV_SCHEDULE, 930);
	setObjectScene(INV_GRENADES, 355);
	setObjectScene(INV_GREENS_KNIFE, 370);
	setObjectScene(INV_JACKET, 880);
	setObjectScene(INV_DOG_WHISTLE, 880);
	setObjectScene(INV_YELLOW_CORD, 910);
	setObjectScene(INV_BLACK_CORD, 910);
}

void BlueForceInvObjectList::setObjectScene(int objectNum, int sceneNumber) {
	// Find the appropriate object
	int num = objectNum;
	SynchronizedList<InvObject *>::iterator i = _itemList.begin(); 
	while (num-- > 0) ++i;
	(*i)->_sceneNumber = sceneNumber;
	
	// If the item is the currently active one, default back to the use cursor
	if (BF_GLOBALS._events.getCursor() == objectNum)
		BF_GLOBALS._events.setCursor(CURSOR_USE);

	// Update the user interface if necessary
	BF_GLOBALS._uiElements.updateInventory();
}

/*--------------------------------------------------------------------------*/

} // End of namespace BlueForce

} // End of namespace TsAGE
