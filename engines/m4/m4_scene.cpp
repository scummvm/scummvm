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
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"

#include "m4/dialogs.h"
#include "m4/globals.h"
#include "m4/scene.h"
#include "m4/events.h"
#include "m4/graphics.h"
#include "m4/rails.h"
#include "m4/font.h"
#include "m4/m4_views.h"
#include "m4/mads_views.h"
#include "m4/compression.h"

namespace M4 {

M4Scene::M4Scene(M4Engine *vm): Scene(vm) {
	_vm = vm;
	_sceneSprites = NULL;
	_interfaceSurface = new M4InterfaceView(vm);
}

M4Scene::~M4Scene() {
	delete _sceneSprites;
}

void M4Scene::loadSceneSprites(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	sprintf(filename, "%i.ssb", sceneNumber);

	Common::SeekableReadStream *sceneS = _vm->res()->get(filename);
	_sceneSprites = new SpriteAsset(_vm, sceneS, sceneS->size(), filename);
	_vm->res()->toss(filename);

	printf("Scene has %d sprites, each one having %d colors\n", _sceneSprites->getCount(), _sceneSprites->getColorCount());
}

void M4Scene::loadScene(int sceneNumber) {
	Scene::loadScene(sceneNumber);

	_backgroundSurface->loadBackground(sceneNumber);
	_palData = NULL;

	if (_vm->getGameType() == GType_Burger &&
			sceneNumber != TITLE_SCENE_BURGER && sceneNumber != MAINMENU_SCENE_BURGER)
		setStatusText("");

	// Load scene def file (*.CHK)
	loadSceneResources(sceneNumber);
	loadSceneInverseColorTable(sceneNumber);

	// TODO: set walker scaling
	// TODO: destroy woodscript buffer

	// Load scene walk path file (*.COD/*.WW?)
	loadSceneCodes(sceneNumber);

	// Load inverse color table file (*.IPL)
	loadSceneInverseColorTable(sceneNumber);

	if (_vm->getGameType() != GType_Burger) {
		// Load scene sprites file (*.SSB)
		loadSceneSprites(sceneNumber);

		// Load scene sprite codes file (*.SSC)
		loadSceneSpriteCodes(sceneNumber);
	}


	if (sceneNumber != TITLE_SCENE_BURGER && sceneNumber != MAINMENU_SCENE_BURGER) {
		_m4Vm->scene()->getInterface()->show();
		showSprites();
	}

	// Purge resources
	_vm->res()->purge();
}

void M4Scene::loadSceneCodes(int sceneNumber, int index) {
	char filename[kM4MaxFilenameSize];
	Common::SeekableReadStream *sceneS;

	sprintf(filename, "%i.cod", sceneNumber);
	sceneS = _vm->res()->openFile(filename);
	_walkSurface->loadCodesM4(sceneS);
	_vm->res()->toss(filename);
}

void M4Scene::show() {
	Scene::show();
	_vm->_viewManager->addView(_interfaceSurface);
}

void M4Scene::checkHotspotAtMousePos(int x, int y) {
	if (_vm->getGameType() == GType_Riddle)
		return;

	// TODO: loads of things to do here, only the mouse cursor and the status
	// text is changed for now

	// Only scene hotspots are checked for now, not parallax/props, as the
	// latter ones are not used by Orion Burger
	HotSpot *currentHotSpot = _sceneResources.hotspots->findByXY(x, y);
	if (currentHotSpot != NULL && currentHotSpot->getActive()) {
		if (_vm->_mouse->getCursorNum() != CURSOR_LOOK &&
			_vm->_mouse->getCursorNum() != CURSOR_TAKE &&
			_vm->_mouse->getCursorNum() != CURSOR_USE &&
			_m4Vm->scene()->getInterface()->_inventory.getSelectedIndex() == -1) {
			_vm->_mouse->setCursorNum(currentHotSpot->getCursor());
		}
		_m4Vm->scene()->getInterface()->setStatusText(currentHotSpot->getPrep());
	} else {
		if (_vm->_mouse->getCursorNum() != CURSOR_LOOK &&
			_vm->_mouse->getCursorNum() != CURSOR_TAKE &&
			_vm->_mouse->getCursorNum() != CURSOR_USE &&
			_m4Vm->scene()->getInterface()->_inventory.getSelectedIndex() == -1) {
			_vm->_mouse->setCursorNum(0);
		} else {

		}
	}
}

void M4Scene::leftClick(int x, int y) {
	if (_vm->getGameType() == GType_Burger) {
		// Place a Wilbur sprite with the correct facing
		HotSpot	*currentHotSpot = _sceneResources.hotspots->findByXY(x, y);
		if (currentHotSpot != NULL && currentHotSpot->getActive()) {
			update();
			_vm->_actor->setWalkerDirection(currentHotSpot->getFacing());
			/*
			int posX = currentHotSpot->getFeetX();
			int posY = currentHotSpot->getFeetY() -
					   scaleValue(_vm->_actor->getWalkerHeight(), _vm->_actor->getWalkerScaling(), 0);
			//_vm->_actor->placeWalkerSpriteAt(0, posX, posY);
			*/

			// Player said.... (for scene scripts)
			printf("Player said: %s %s\n", currentHotSpot->getVerb(), currentHotSpot->getVocab());

			// FIXME: This should be moved somewhere else, and is incomplete
			if (_m4Vm->scene()->getInterface()->_inventory.getSelectedIndex() == -1) {
				if (_vm->_mouse->getVerb() == NULL) {
					strcpy(_vm->_player->verb, currentHotSpot->getVerb());
				} else {
					strcpy(_vm->_player->verb, _vm->_mouse->getVerb());
				}
			} else {
				strcpy(_vm->_player->verb, _m4Vm->scene()->getInterface()->_inventory.getSelectedObjectName());
			}
			strcpy(_vm->_player->noun, currentHotSpot->getVocab());
			strcpy(_vm->_player->object, "");
			_vm->_player->commandReady = true;

			printf("## Player said: %s %s\n", _vm->_player->verb, _vm->_player->noun);

		}
	}
}

void M4Scene::rightClick(int x, int y) {
	if (_vm->getGameType() == GType_Burger) {
		nextCommonCursor();
		_m4Vm->scene()->getInterface()->_inventory.clearSelected();
	}
}

void M4Scene::setAction(int action, int objectId) {
}

void M4Scene::setStatusText(const char *text) {
	getInterface()->setStatusText(text);
}

void M4Scene::update() {

}

void M4Scene::nextCommonCursor() {
	int cursorIndex = _vm->_mouse->getCursorNum();

	switch (cursorIndex) {
	case CURSOR_ARROW:
		cursorIndex = CURSOR_LOOK;
		break;
	case CURSOR_LOOK:
		cursorIndex = CURSOR_TAKE;
		break;
	case CURSOR_TAKE:
		cursorIndex = CURSOR_USE;
		break;
	case CURSOR_USE:
		cursorIndex = CURSOR_ARROW;
		break;
	default:
		cursorIndex = CURSOR_ARROW;
	}

	_vm->_mouse->setCursorNum(cursorIndex);
}

} // End of namespace M4
