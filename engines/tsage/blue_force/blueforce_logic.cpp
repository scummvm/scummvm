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
#include "tsage/blue_force/blueforce_scenes0.h"
#include "tsage/blue_force/blueforce_scenes1.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

void BlueForceGame::start() {
	// Start the game
	_globals->_sceneManager.changeScene(50);

	_globals->_events.setCursor(CURSOR_WALK);
}

Scene *BlueForceGame::createScene(int sceneNumber) {
	switch (sceneNumber) {
	/* Scene Group #0 */
	case 20:
		// Tsunami Title Screen
		return new Scene20();
	case 50:
		return new Scene50();
	case 60:
		error("Scene group 0 not implemented");
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
	case 190:
		error("Scene group 1 not implemented");
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
	case 315:
	case 325:
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

void Timer::set(uint32 delay, Action *action) {
	assert(delay != 0);

	_endFrame = BF_GLOBALS._sceneHandler->getFrameDifference() + delay;
	_endAction = action;

	((SceneExt *)BF_GLOBALS._sceneManager._scene)->addTimer(this);
}

/*--------------------------------------------------------------------------*/

void SceneItemType1::process(Event &event) {
	if (_action)
		_action->process(event);
}

void SceneItemType1::startMove(SceneObject *sceneObj, va_list va) {
	warning("TODO: sub_1621C");
}

/*--------------------------------------------------------------------------*/

void SceneItemType2::startMove(SceneObject *sceneObj, va_list va) {
}

/*--------------------------------------------------------------------------*/

SceneExt::SceneExt(): Scene() {
	warning("TODO: dword_503AA/dword_503AE/dword_53030");

	_field372 = 0;
	_field37A = 0;
	_eventHandler = NULL;
}

void SceneExt::postInit(SceneObjectList *OwnerList) {
	Scene::postInit(OwnerList);
	if (BF_GLOBALS._v4CEA2) {
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
		if ((--_field37A == 0) && BF_GLOBALS._v4CEA2) {
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

/*--------------------------------------------------------------------------*/

GameScene::GameScene() {

}

void GameScene::postInit(SceneObjectList *OwnerList) {
	_field794 = 0;
	_field412 = 1;
	SceneExt::postInit(OwnerList);
}

void GameScene::remove() {
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
	GLOBALS._scenePalette.loadPalette(2);
	GLOBALS._scenePalette.refresh();
}

void SceneHandlerExt::process(Event &event) {
	SceneHandler::process(event);

	// TODO: All the new stuff from Blue Force
}

/*--------------------------------------------------------------------------*/

BlueForceInvObjectList::BlueForceInvObjectList():
		_business_card(9, 4, 2, 0),
		_lauras_sweater(9, 4, 3, 0),
		_handcuffs(9, 1, 4, 0),
		_magnum(9, 1, 5, 0),
		_ticket_book(9, 1, 6, 0),
		_miranda_card(9, 1, 7, 0),
		_forest_follet(9, 1, 8, 0),
		_bradford_id(9, 1, 9, 0),
		_baseball_card(9, 1, 10, 0),
		_slip_bradford(9, 1, 11, 0),
		_flare(9, 1, 12, 0),
		_rap_sheet(9, 1, 13, 0),
		_cartridges(9, 1, 14, 0),
		_rifle(9, 1, 15, 0),
		_wig(9, 1, 16, 0),
		_frankies_id(9, 1, 17, 0),
		_tyrones_id(9, 1, 18, 0),
		_pistol22(9, 1, 19, 0),
		_unused(1, 1, 1, 0),
		_slip_frankie(9, 2, 1, 0),
		_slip_tyrone(9, 2, 2, 0),
		_atf_teletype(9, 2, 3, 0),
		_da_note(9, 2, 4, 0),
		_blueprints(9, 2, 5, 0),
		_planter_key(9, 2, 6, 0),
		_center_punch(9, 2, 7, 0),
		_tranquilizer(9, 2, 8, 0),
		_boat_hook(9, 2, 9, 0),
		_oily_rags(9, 2, 10, 0),
		_fuel_jar(9, 2, 11, 0),
		_screwdriver(9, 2, 12, 0),
		_floppy_disk1(9, 2, 13, 0),
		_floppy_disk2(9, 2, 14, 0),
		_driftwood(9, 2, 15, 0),
		_crate_piece1(9, 2, 16, 0),
		_crate_piece2(9, 2, 17, 0),
		_shoebox(9, 2, 18, 0),
		_badge(9, 2, 19, 0),
		_unused2(1, 1, 1, 0),
		_rental_coupons(9, 3, 1, 0),
		_nickel(9, 3, 2, 0),
		_calendar(9, 3, 3, 0),
		_dixon_note(9, 3, 4, 0),
		_cobb_mugshot(9, 3, 5, 0),
		_murder_article(9, 3, 6, 0),
		_microfiche(9, 3, 7, 0),
		_future_wave_keys(9, 3, 8, 0),
		_rental_boat_keys(9, 3, 9, 0),
		_napkin(9, 3, 10, 0),
		_cobb_printout(9, 3, 11, 0),
		_fishing_net(9, 3, 12, 0),
		_id(9, 3, 13, 0),
		_rounds_9mm(9, 3, 14, 0),
		_dates_note(9, 3, 15, 0),
		_hand_grenade(9, 3, 16, 0),
		_cord_110(9, 3, 17, 0),
		_cord_110_plug(9, 3, 18, 0),
		_cord_220(9, 3, 19, 0),
		_unused3(1, 1, 1, 0),
		_cord_220_plug(9, 4, 1, 0),
		_official_document(9, 4, 2, 0),
		_red_sweater(9, 4, 3, 0),
		_jackknife(9, 4, 4, 0),
		_whistle(9, 4, 5, 0),
		_gun(9, 1, 2, 0),
		_alley_cat_key(9, 4, 7, 0) {

	// Add the items to the list
	_itemList.push_back(&_business_card);
	_itemList.push_back(&_lauras_sweater);
	_itemList.push_back(&_handcuffs);
	_itemList.push_back(&_magnum);
	_itemList.push_back(&_ticket_book);
	_itemList.push_back(&_miranda_card);
	_itemList.push_back(&_forest_follet);
	_itemList.push_back(&_bradford_id);
	_itemList.push_back(&_baseball_card);
	_itemList.push_back(&_slip_bradford);
	_itemList.push_back(&_flare);
	_itemList.push_back(&_rap_sheet);
	_itemList.push_back(&_cartridges);
	_itemList.push_back(&_rifle);
	_itemList.push_back(&_wig);
	_itemList.push_back(&_frankies_id);
	_itemList.push_back(&_tyrones_id);
	_itemList.push_back(&_pistol22);
	_itemList.push_back(&_unused);
	_itemList.push_back(&_slip_frankie);
	_itemList.push_back(&_slip_tyrone);
	_itemList.push_back(&_atf_teletype);
	_itemList.push_back(&_da_note);
	_itemList.push_back(&_blueprints);
	_itemList.push_back(&_planter_key);
	_itemList.push_back(&_center_punch);
	_itemList.push_back(&_tranquilizer);
	_itemList.push_back(&_boat_hook);
	_itemList.push_back(&_oily_rags);
	_itemList.push_back(&_fuel_jar);
	_itemList.push_back(&_screwdriver);
	_itemList.push_back(&_floppy_disk1);
	_itemList.push_back(&_floppy_disk2);
	_itemList.push_back(&_driftwood);
	_itemList.push_back(&_crate_piece1);
	_itemList.push_back(&_crate_piece2);
	_itemList.push_back(&_shoebox);
	_itemList.push_back(&_badge);
	_itemList.push_back(&_unused2);
	_itemList.push_back(&_rental_coupons);
	_itemList.push_back(&_nickel);
	_itemList.push_back(&_calendar);
	_itemList.push_back(&_dixon_note);
	_itemList.push_back(&_cobb_mugshot);
	_itemList.push_back(&_murder_article);
	_itemList.push_back(&_microfiche);
	_itemList.push_back(&_future_wave_keys);
	_itemList.push_back(&_rental_boat_keys);
	_itemList.push_back(&_napkin);
	_itemList.push_back(&_cobb_printout);
	_itemList.push_back(&_fishing_net);
	_itemList.push_back(&_id);
	_itemList.push_back(&_rounds_9mm);
	_itemList.push_back(&_dates_note);
	_itemList.push_back(&_hand_grenade);
	_itemList.push_back(&_cord_110);
	_itemList.push_back(&_cord_110_plug);
	_itemList.push_back(&_cord_220);
	_itemList.push_back(&_unused3);
	_itemList.push_back(&_cord_220_plug);
	_itemList.push_back(&_official_document);
	_itemList.push_back(&_red_sweater);
	_itemList.push_back(&_jackknife);
	_itemList.push_back(&_whistle);
	_itemList.push_back(&_gun);
	_itemList.push_back(&_alley_cat_key);
}


} // End of namespace BlueForce

} // End of namespace TsAGE
