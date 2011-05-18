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
 */

#include "common/system.h"
#include "common/textconsole.h"

#include "m4/m4_scene.h"
#include "m4/dialogs.h"
#include "m4/globals.h"
#include "m4/events.h"
#include "m4/graphics.h"
#include "m4/rails.h"
#include "m4/font.h"
#include "m4/m4_views.h"
#include "m4/mads_views.h"
#include "m4/compression.h"

namespace M4 {

M4Scene::M4Scene(M4Engine *vm): _sceneResources(), Scene(vm, &_sceneResources) {
	_vm = vm;
	_sceneSprites = NULL;
	_inverseColorTable = NULL;

	_sceneResources.hotspots = new HotSpotList();
	_sceneResources.parallax = new HotSpotList();
	_sceneResources.dynamicHotspots = new HotSpotList();
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

	debugCN(kDebugGraphics, "Scene has %d sprites, each one having %d colors\n", _sceneSprites->getCount(), _sceneSprites->getColorCount());
}

void M4Scene::loadSceneResources(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	int i = 0, x = 0, y = 0;
	sprintf(filename, "%i.chk", sceneNumber);

	Common::SeekableReadStream *sceneS = _vm->res()->get(filename);

	if (sceneS != NULL) {
		sceneS->read(_sceneResources.artBase, MAX_CHK_FILENAME_SIZE);
		sceneS->read(_sceneResources.pictureBase, MAX_CHK_FILENAME_SIZE);
		int hotspotCount = sceneS->readUint32LE();
		_sceneResources.parallaxCount = sceneS->readUint32LE();
		int dynHotspotCount = sceneS->readUint32LE();
		_sceneResources.frontY = sceneS->readUint32LE();
		_sceneResources.backY = sceneS->readUint32LE();
		_sceneResources.frontScale = sceneS->readUint32LE();
		_sceneResources.backScale = sceneS->readUint32LE();
		for (i = 0; i < 16; i++)
			_sceneResources.depthTable[i] = sceneS->readUint16LE();
		_sceneResources.railNodeCount = sceneS->readUint32LE();

		// Clear rails from previous scene
		_vm->_rails->clearRails();

		for (i = 0; i < _sceneResources.railNodeCount; i++) {
			x = sceneS->readUint32LE();
			y = sceneS->readUint32LE();
			if (_vm->_rails->addRailNode(x, y, true) < 0) {
				warning("Too many rail nodes defined for scene");
			}
		}

		// Clear current hotspot lists
		_sceneResources.hotspots->clear();
		_sceneResources.parallax->clear();
		_sceneResources.dynamicHotspots->clear();

		_sceneResources.hotspots->loadHotSpots(sceneS, hotspotCount);
		_sceneResources.parallax->loadHotSpots(sceneS, _sceneResources.parallaxCount);
		_sceneResources.dynamicHotspots->loadHotSpots(sceneS, dynHotspotCount);

		// Note that toss() deletes the MemoryReadStream
		_vm->res()->toss(filename);
	}
}

void M4Scene::loadSceneInverseColorTable(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	Common::SeekableReadStream *iplS;

	sprintf(filename, "%i.ipl", sceneNumber);
	iplS = _vm->res()->openFile(filename);
	delete[] _inverseColorTable;
	_inverseColorTable = new byte[iplS->size()];
	iplS->read(_inverseColorTable, iplS->size());
	_vm->res()->toss(filename);
}

void M4Scene::loadSceneSpriteCodes(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	sprintf(filename, "%i.ssc", sceneNumber);

	Common::SeekableReadStream *sceneS = _vm->res()->get(filename);

	// TODO

	if (sceneS != NULL) {
		SpriteAsset* _sceneSpriteCodes = new SpriteAsset(_vm, sceneS, sceneS->size(), filename);
		int colorCount = _sceneSpriteCodes->getColorCount();
//			RGB8* spritePalette = _sceneSpriteCodes->getPalette();
		//_vm->_palette->setPalette(spritePalette, 0, colorCount);

		debugCN(kDebugGraphics, "Scene has %d sprite codes, each one having %d colors\n", _sceneSpriteCodes->getCount(), colorCount);

		// Note that toss() deletes the MemoryReadStream
		_vm->res()->toss(filename);
	}
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

void M4Scene::leaveScene() {
	_sceneResources.parallax->clear();

	delete _sceneResources.parallax;
	delete[] _inverseColorTable;

	Scene::leaveScene();
}

void M4Scene::mouseMove(int x, int y) {
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
			debugCN(kDebugGraphics, "Player said: %s %s\n", currentHotSpot->getVerb(), currentHotSpot->getVocab());

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

			debugCN(kDebugGraphics, "## Player said: %s %s\n", _vm->_player->verb, _vm->_player->noun);

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

void M4Scene::showHotSpots() {
	Scene::showHotSpots();

	// parallax (yellow)
	for (int i = 0; i < _sceneResources.parallaxCount; i++) {
		HotSpot *currentHotSpot = _sceneResources.parallax->get(i);
		_backgroundSurface->frameRect(currentHotSpot->getRect(), _vm->_palette->YELLOW);
	}
}

} // End of namespace M4
