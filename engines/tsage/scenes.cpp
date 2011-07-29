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

#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/ringworld_logic.h"
#include "tsage/tsage.h"
#include "tsage/saveload.h"

namespace tSage {

SceneManager::SceneManager() {
	_scene = NULL;
	_hasPalette = false;
	_sceneNumber = -1;
	_nextSceneNumber = -1;
	_previousScene = 0;
	_fadeMode = FADEMODE_GRADUAL;
	_scrollerRect = Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_saver->addListener(this);
	_objectCount = 0;
}

SceneManager::~SceneManager() {
	delete _scene;
}

void SceneManager::setNewScene(int sceneNumber) {
	warning("SetNewScene(%d)", sceneNumber);
	_nextSceneNumber = sceneNumber;
}

void SceneManager::checkScene() {
	if (_nextSceneNumber != -1) {
		sceneChange();
		_nextSceneNumber = -1;
	}

	_globals->dispatchSounds();
}

void SceneManager::sceneChange() {
	int activeScreenNumber = 0;

	// Handle removing the scene
	if (_scene) {
		activeScreenNumber = _scene->_activeScreenNumber;
		_scene->remove();
	}

	// Clear the scene objects
	SynchronizedList<SceneObject *>::iterator io = _globals->_sceneObjects->begin();
	while (io != _globals->_sceneObjects->end()) {
		SceneObject *sceneObj = *io;
		++io;
		sceneObj->removeObject();
	}

	// Clear the secondary scene object list
	io = _globals->_sceneManager._altSceneObjects.begin();
	while (io != _globals->_sceneManager._altSceneObjects.end()) {
		SceneObject *sceneObj = *io;
		++io;
		sceneObj->removeObject();
	}

	// Clear the hotspot list
	SynchronizedList<SceneItem *>::iterator ii = _globals->_sceneItems.begin();
	while (ii != _globals->_sceneItems.end()) {
		SceneItem *sceneItem = *ii;
		++ii;
		sceneItem->remove();
	}

	// TODO: Clear _list_45BAA list

	// If there is an active scene, deactivate it
	if (_scene) {
		_previousScene = _sceneNumber;

		delete _scene;
		_scene = NULL;
		_sceneNumber = -1;
	}

	// Set the next scene to be active
	_sceneNumber = _nextSceneNumber;

	// Free any regions
	disposeRegions();

	// Ensure that the same number of objects are registered now as when the scene started
	if (_objectCount > 0) {
		assert(_objectCount == _saver->getObjectCount());
	}
	_objectCount = _saver->getObjectCount();
	_globals->_sceneHandler._delayTicks = 2;

	// Instantiate and set the new scene
	_scene = getNewScene();

	if (!_saver->getMacroRestoreFlag())
		_scene->postInit();
	else
		_scene->loadScene(activeScreenNumber);
}

Scene *SceneManager::getNewScene() {
	return _globals->_game->createScene(_nextSceneNumber);
}

void SceneManager::fadeInIfNecessary() {
	if (_hasPalette) {
		uint32 adjustData = 0;
		for (int percent = 0; percent < 100; percent += 5) {
			if (_globals->_sceneManager._fadeMode == FADEMODE_IMMEDIATE)
				percent = 100;

			_globals->_scenePalette.fade((const byte *)&adjustData, false, percent);
			g_system->updateScreen();
			g_system->delayMillis(10);
		}

		_globals->_scenePalette.refresh();
		_hasPalette = false;
	}
}

void SceneManager::changeScene(int newSceneNumber) {
	warning("changeScene(%d)", newSceneNumber);
	// Fade out the scene
	ScenePalette scenePalette;
	uint32 adjustData = 0;
	_globals->_scenePalette.clearListeners();
	scenePalette.getPalette();

	for (int percent = 100; percent >= 0; percent -= 5) {
		scenePalette.fade((byte *)&adjustData, false, percent);
		g_system->delayMillis(10);
	}

	// Stop any objects that were animating
	SynchronizedList<SceneObject *>::iterator i;
	for (i = _globals->_sceneObjects->begin(); i != _globals->_sceneObjects->end(); ++i) {
		SceneObject *sceneObj = *i;
		Common::Point pt(0, 0);
		sceneObj->addMover(NULL, &pt);
		sceneObj->setObjectWrapper(NULL);
		sceneObj->animate(ANIM_MODE_NONE, 0);

		sceneObj->_flags &= !OBJFLAG_PANES;
	}

	// Blank out the screen
	_globals->_screenSurface.fillRect(_globals->_screenSurface.getBounds(), 0);

	// Set the new scene to be loaded
	setNewScene(newSceneNumber);
}

void SceneManager::setup() {
	_saver->addLoadNotifier(SceneManager::loadNotifier);
	setBackSurface();
}

void SceneManager::setBackSurface() {
	int size = _globals->_sceneManager._scene->_backgroundBounds.width() *
		_globals->_sceneManager._scene->_backgroundBounds.height();

	if (size > 96000) {
		if (_globals->_sceneManager._scene->_backgroundBounds.width() <= SCREEN_WIDTH) {
			// Standard size creation
			_globals->_sceneManager._scene->_backSurface.create(SCREEN_WIDTH, SCREEN_HEIGHT * 3 / 2);
			_globals->_sceneManager._scrollerRect = Rect(0, 30, SCREEN_WIDTH, SCREEN_HEIGHT - 30);
		} else {
			// Wide screen needs extra space to allow for scrolling
			_globals->_sceneManager._scene->_backSurface.create(SCREEN_WIDTH * 3 / 2, SCREEN_HEIGHT);
			_globals->_sceneManager._scrollerRect = Rect(80, 0, SCREEN_WIDTH - 80, SCREEN_HEIGHT);
		}
	} else {
		_globals->_sceneManager._scene->_backSurface.create(
			_globals->_sceneManager._scene->_backgroundBounds.width(),
			_globals->_sceneManager._scene->_backgroundBounds.height()
		);
		_globals->_sceneManager._scrollerRect = Rect(80, 20, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 20);
	}
}

void SceneManager::saveListener(int saveMode) {
}

void SceneManager::loadNotifier(bool postFlag) {
	if (postFlag) {
		if (_globals->_sceneManager._scene->_activeScreenNumber != -1)
			_globals->_sceneManager._scene->loadSceneData(_globals->_sceneManager._scene->_activeScreenNumber);
		_globals->_sceneManager._hasPalette = true;
	}
}

void SceneManager::setBgOffset(const Common::Point &pt, int loadCount) {
	_sceneBgOffset = pt;
	_sceneLoadCount = loadCount;
}

void SceneManager::listenerSynchronize(Serializer &s) {
	s.validate("SceneManager");

	if (s.isLoading() && !_globals->_sceneManager._scene)
		// Loading a savegame straight from the launcher, so instantiate a blank placeholder scene
		// in order for the savegame loading to work correctly
		_globals->_sceneManager._scene = new Scene();

	_altSceneObjects.synchronize(s);
	s.syncAsSint32LE(_sceneNumber);
	s.syncAsUint16LE(_globals->_sceneManager._scene->_activeScreenNumber);

	if (s.isLoading()) {
		changeScene(_sceneNumber);
		
		if (_nextSceneNumber != -1) {
			sceneChange();
			_nextSceneNumber = -1;
		}
	}

	_globals->_sceneManager._scrollerRect.synchronize(s);
	SYNC_POINTER(_globals->_scrollFollower);
	s.syncAsSint16LE(_loadMode);
}

/*--------------------------------------------------------------------------*/

Scene::Scene() : _sceneBounds(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
			_backgroundBounds(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
	_sceneMode = 0;
	_oldSceneBounds = Rect(4000, 4000, 4100, 4100);
	Common::set_to(&_zoomPercents[0], &_zoomPercents[256], 0);
}

Scene::~Scene() {
}

void Scene::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		StripCallback::synchronize(s);

	s.syncAsSint32LE(_field12);
	s.syncAsSint32LE(_screenNumber);
	s.syncAsSint32LE(_activeScreenNumber);
	s.syncAsSint32LE(_sceneMode);
	_backgroundBounds.synchronize(s);
	_sceneBounds.synchronize(s);
	_oldSceneBounds.synchronize(s);
	s.syncAsSint16LE(_fieldA);
	s.syncAsSint16LE(_fieldE);

	for (int i = 0; i < 256; ++i)
		s.syncAsUint16LE(_enabledSections[i]);
	for (int i = 0; i < 256; ++i)
		s.syncAsSint16LE(_zoomPercents[i]);
}

void Scene::postInit(SceneObjectList *OwnerList) {
	_action = NULL;
	_field12 = 0;
	_sceneMode = 0;
}

void Scene::process(Event &event) {
	if (_action)
		_action->process(event);
}

void Scene::dispatch() {
	if (_action)
		_action->dispatch();
}

void Scene::loadScene(int sceneNum) {
	warning("loadScene(%d)", sceneNum);
	_screenNumber = sceneNum;
	if (_globals->_scenePalette.loadPalette(sceneNum))
		_globals->_sceneManager._hasPalette = true;

	loadSceneData(sceneNum);
}

void Scene::loadSceneData(int sceneNum) {
	_activeScreenNumber = sceneNum;

	// Get the basic scene size
	byte *data = _resourceManager->getResource(RES_BITMAP, sceneNum, 9999);
	_backgroundBounds = Rect(0, 0, READ_LE_UINT16(data), READ_LE_UINT16(data + 2));
	_globals->_sceneManager._scene->_sceneBounds.contain(_backgroundBounds);
	DEALLOCATE(data);

	// Set up a surface for storing the scene background
	SceneManager::setBackSurface();

	// Load the data lists for the scene
	_globals->_walkRegions.load(sceneNum);

	// Load the item regions of the scene
	_globals->_sceneRegions.load(sceneNum);

	// Load the priority regions
	_priorities.load(sceneNum);

	// Initialize the section enabled list
	Common::set_to(&_enabledSections[0], &_enabledSections[16 * 16], 0xffff);

	_globals->_sceneOffset.x = (_sceneBounds.left / 160) * 160;
	_globals->_sceneOffset.y = (_sceneBounds.top / 100) * 100;
	_globals->_paneRefreshFlag[0] = 1;
	_globals->_paneRefreshFlag[1] = 1;
	_globals->_sceneManager._loadMode = 1;
	_globals->_sceneManager._sceneLoadCount = 0;
	_globals->_sceneManager._sceneBgOffset = Common::Point(0, 0);

	// Load the background for the scene
	loadBackground(0, 0);
}

void Scene::loadBackground(int xAmount, int yAmount) {
	// Adjust the scene bounds by the passed scroll amounts
	_sceneBounds.translate(xAmount, yAmount);
	_sceneBounds.contain(_backgroundBounds);
	_sceneBounds.left &= ~3;
	_sceneBounds.right &= ~3;
	_globals->_sceneOffset.x &= ~3;

	if ((_sceneBounds.top != _oldSceneBounds.top) || (_sceneBounds.left != _oldSceneBounds.left)) {
		if (_globals->_sceneManager._loadMode == 0) {
			_globals->_paneRefreshFlag[0] = 2;
			_globals->_paneRefreshFlag[1] = 2;
			_globals->_sceneManager._loadMode = 2;
		}
		_oldSceneBounds = _sceneBounds;
	}

	_globals->_sceneOffset.x = (_sceneBounds.left / 160) * 160;
	_globals->_sceneOffset.y = (_sceneBounds.top / 100) * 100;

	if ((_backgroundBounds.width() / 160) == 3)
		_globals->_sceneOffset.x = 0;
	if ((_backgroundBounds.height() / 100) == 3)
		_globals->_sceneOffset.y = 0;

	if ((_globals->_sceneOffset.x != _globals->_prevSceneOffset.x) ||
		(_globals->_sceneOffset.y != _globals->_prevSceneOffset.y)) {
		// Change has happend, so refresh background
		_globals->_prevSceneOffset = _globals->_sceneOffset;
		refreshBackground(xAmount, yAmount);
	}
}

void Scene::refreshBackground(int xAmount, int yAmount) {
	if (_globals->_sceneManager._scene->_activeScreenNumber == -1)
		return;

	// Set the quadrant ranges
	int xHalfCount = MIN(_backSurface.getBounds().width() / 160, _backgroundBounds.width() / 160);
	int yHalfCount = MIN(_backSurface.getBounds().height() / 100, _backgroundBounds.height() / 100);
	int xHalfOffset = (_backgroundBounds.width() / 160) == 3 ? 0 : _sceneBounds.left / 160;
	int yHalfOffset = (_backgroundBounds.height() / 100) == 3 ? 0 : _sceneBounds.top / 100;

	// Set the limits and increment amounts
	int xInc = (xAmount < 0) ? -1 : 1;
	int xSectionStart = (xAmount < 0) ? 15 : 0;
	int xSectionEnd = (xAmount < 0) ? -1 : 16;
	int yInc = (yAmount < 0) ? -1 : 1;
	int ySectionStart = (yAmount < 0) ? 15 : 0;
	int ySectionEnd = (yAmount < 0) ? -1 : 16;
	bool changedFlag = false;

	for (int yp = ySectionStart; yp != ySectionEnd; yp += yInc) {
		for (int xp = xSectionStart; xp != xSectionEnd; xp += xInc) {
			if ((yp < yHalfOffset) || (yp >= (yHalfOffset + yHalfCount)) ||
				(xp < xHalfOffset) || (xp >= (xHalfOffset + xHalfCount))) {
				// Flag section as enabled
				_enabledSections[xp * 16 + yp] = 0xffff;
			} else {
				// Check if the section is already loaded
				if ((_enabledSections[xp * 16 + yp] == 0xffff) || ((xAmount == 0) && (yAmount == 0))) {
					// Chunk isn't loaded, so load it in
					Graphics::Surface s = _backSurface.lockSurface();
					GfxSurface::loadScreenSection(s, xp - xHalfOffset, yp - yHalfOffset, xp, yp);
					_backSurface.unlockSurface();
					changedFlag = true;
				} else {
					int yv = (_enabledSections[xp * 16 + yp] == ((xp - xHalfOffset) << 4)) ? 0 : 1;
					if (yv | (yp - yHalfOffset)) {
						// Copy an existing 160x100 screen section previously loaded
						int xSectionDest = xp - xHalfOffset;
						int ySectionDest = yp - yHalfOffset;
						int xSectionSrc = _enabledSections[xp * 16 + yp] >> 4;
						int ySectionSrc = _enabledSections[xp * 16 + yp] & 0xf;

						Rect srcBounds(xSectionSrc * 160, ySectionSrc * 100,
								(xSectionSrc + 1) * 160, (ySectionSrc + 1) * 100);
						Rect destBounds(xSectionDest * 160, ySectionDest * 100,
								(xSectionDest + 1) * 160, (ySectionDest + 1) * 100);

						_backSurface.copyFrom(_backSurface, srcBounds, destBounds);
					}
				}

				_enabledSections[xp * 16 + yp] =
					((xp - xHalfOffset) << 4) | (yp - yHalfOffset);
			}
		}
	}

	if (changedFlag) {
		drawAltObjects();
	}
}

void Scene::drawAltObjects() {
	Common::Array<SceneObject *> objList;

	// Initial loop to set the priority for entries in the list
	for (SynchronizedList<SceneObject *>::iterator i = _globals->_sceneManager._altSceneObjects.begin();
		i != _globals->_sceneManager._altSceneObjects.end(); ++i) {
		SceneObject *obj = *i;
		objList.push_back(obj);

		// Handle updating object priority
		if (!(obj->_flags & OBJFLAG_FIXED_PRIORITY)) {
			obj->_priority = MIN((int)obj->_position.y - 1,
				(int)_globals->_sceneManager._scene->_backgroundBounds.bottom);
		}
	}

	// Sort the list by priority
	_globals->_sceneManager._altSceneObjects.sortList(objList);

	// Drawing loop
	for (uint objIndex = 0; objIndex < objList.size(); ++objIndex) {
		SceneObject *obj = objList[objIndex];

		obj->reposition();
		obj->draw();
	}
}

void Scene::setZoomPercents(int yStart, int minPercent, int yEnd, int maxPercent) {
	int currDiff = 0;
	int v = 0;
	while (v < yStart)
		_zoomPercents[v++] = minPercent;

	int diff1 = ABS(maxPercent - minPercent);
	int diff2 = ABS(yEnd - yStart);
	int remainingDiff = MAX(diff1, diff2);

	while (remainingDiff-- != 0) {
		_zoomPercents[v] = minPercent;
		if (diff2 <= diff1) {
			++minPercent;
			currDiff += diff2;
			if (currDiff >= diff1) {
				currDiff -= diff1;
				++v;
			}
		} else {
			++v;
			currDiff += diff1;
			if (currDiff >= diff2) {
				currDiff -= diff2;
				++minPercent;
			}
		}
	}

	while (yEnd < 256)
		_zoomPercents[yEnd++] = minPercent;
}

/*--------------------------------------------------------------------------*/

void Game::execute() {
	// Main game loop
	bool activeFlag = false;
	do {
		// Process all currently atcive game handlers
		activeFlag = false;
		for (SynchronizedList<GameHandler *>::iterator i = _handlers.begin(); i != _handlers.end(); ++i) {
			GameHandler *gh = *i;
			if (gh->_lockCtr.getCtr() == 0) {
				gh->execute();
				activeFlag = true;
			}
		}
	} while (activeFlag && !_vm->shouldQuit());
}

} // End of namespace tSage
