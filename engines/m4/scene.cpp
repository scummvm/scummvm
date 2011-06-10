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
#include "m4/staticres.h"

namespace M4 {

Scene::Scene(MadsM4Engine *vm, SceneResources *res): View(vm, Common::Rect(0, 0, vm->_screen->width(),
			vm->_screen->height())), _sceneResources(res) {
	_screenType = VIEWID_SCENE;

	_sceneResources->hotspots = new HotSpotList();
	_sceneResources->dynamicHotspots = new HotSpotList();
	_backgroundSurface = new M4Surface();
	_walkSurface = new M4Surface();
	_palData = NULL;
	_interfacePal = NULL;
	_interfaceSurface = NULL;
	_vm->_rails->setCodeSurface(_walkSurface);
	_currentScene = -1;
}

Scene::~Scene() {
	leaveScene();
	_vm->_scene = NULL;
}

void Scene::loadScene(int sceneNumber) {
	_previousScene = _currentScene;
	_currentScene = sceneNumber;
	_nextScene = sceneNumber;
}

void Scene::leaveScene() {
	if (_palData) {
		_vm->_palette->deleteRange(_palData);
		delete _palData;
		_palData = NULL;
	}
	if (_interfacePal) {
		_vm->_palette->deleteRange(_interfacePal);
		delete _interfacePal;
		_interfacePal = NULL;
	}
}

void Scene::show() {
	_vm->_viewManager->addView(this);
}

void Scene::showInterface() {
	_vm->_viewManager->addView(_interfaceSurface);
}

void Scene::hideInterface() {
	_vm->_viewManager->deleteView(_interfaceSurface);
}

void Scene::showSprites() {
	// TODO: This is all experimental code, it needs heavy restructuring
	// and cleanup

	// taken from set_walker_scaling() in adv_walk.cpp. A proper implementation will need
	// to store these in global variables
	int minScaling = FixedDiv(_sceneResources->backScale << 16, 100 << 16);
	int maxScaling = FixedDiv(_sceneResources->frontScale << 16, 100 << 16);
	int scaler;

	_vm->_actor->setWalkerDirection(kFacingSouthEast);
	//_vm->_actor->setWalkerPalette();

	// taken from set_walker_scaling() in adv_walk.cpp
	if (_sceneResources->frontY == _sceneResources->backY)
		scaler = 0;
	else
		scaler = FixedDiv(maxScaling - minScaling,
				 (_sceneResources->frontY << 16) - (_sceneResources->backY << 16));

	// FIXME: For now, we (incorrectly) scale the walker to 50% of the scene's max scaling
	_vm->_actor->setWalkerScaling(scaler / 2);
	// Test code to display the protagonist
	_vm->_actor->placeWalkerSpriteAt(0, 320, 200);

	// Test code to display scene sprites
	// TODO
}

// Test function, shows all scene hotspots
void Scene::showHotSpots() {
	int i = 0;
	HotSpot *currentHotSpot;

	// hotspots (green)
	for (i = 0; i < _sceneResources->hotspots->size(); i++) {
		currentHotSpot = _sceneResources->hotspots->get(i);
		_backgroundSurface->frameRect(currentHotSpot->getRect(), _vm->_palette->GREEN);
	}

	// Dynamic hotspots (red)
	for (i = 0; i < _sceneResources->dynamicHotspots->size(); i++) {
		currentHotSpot = _sceneResources->dynamicHotspots->get(i);
		_backgroundSurface->frameRect(currentHotSpot->getRect(), _vm->_palette->RED);
	}
}

/**
 * Debug function that shows the walkable areas by copying them over the current background surface
 */
void Scene::showCodes() {
	if (_vm->isM4()) {
		// Show the walk areas for the M4 engine in black and white
		const byte *srcP = (const byte *)_walkSurface->getBasePtr(0, 0);
		byte *destP = _backgroundSurface->getBasePtr(0, 0);

		for (int i = 0; i < _walkSurface->width() * _walkSurface->height(); i++)
			destP[i] = (srcP[i] & 0x10) ? 0xFF : 0;

		byte colors[256 * 3];
		memset(colors, 0, sizeof(colors));
		colors[255 * 3 + 0] = 255;
		colors[255 * 3 + 1] = 255;
		colors[255 * 3 + 2] = 255;
		_vm->_palette->setPalette(colors, 0, 256);
	} else {
		// MADS handling

		// copy the walk data to the background, in whatever current palette is active
		_walkSurface->copyTo(_backgroundSurface);

		// Show all the scene's walk nodes
		SceneNodeList &nodeList = _madsVm->scene()->getSceneResources()._nodes;
		_backgroundSurface->setColor(_madsVm->_palette->WHITE);
		for (uint i = 0; i < nodeList.size() - 2; ++i) {
			// Draw a little cross at the node's position
			_backgroundSurface->hLine(nodeList[i].pt.x - 2, nodeList[i].pt.x + 2, nodeList[i].pt.y);
			_backgroundSurface->vLine(nodeList[i].pt.x, nodeList[i].pt.y - 2, nodeList[i].pt.y + 2);
		}

		((MadsScene *)this)->_spriteSlots.fullRefresh();
	}
}

void Scene::playIntro() {

}

void Scene::onRefresh(RectList *rects, M4Surface *destSurface) {
	update();
	View::onRefresh(rects, destSurface);
}

bool Scene::onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents) {
	//if (_vm->getGameType() != GType_Burger)
	//	return false;

	// If the game is currently paused, don't do any scene processing
	if (_vm->_kernel->paused)
		return false;

	switch (eventType) {
	case MEVENT_LEFT_CLICK:
		leftClick(x, y);
		break;
	case MEVENT_RIGHT_CLICK:
		rightClick(x, y);
		break;
	case MEVENT_MOVE:
		mouseMove(x, y);
		break;
	default:
		return false;
	}

	return true;
}

} // End of namespace M4
