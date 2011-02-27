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
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/scenes.cpp $
 * $Id: scenes.cpp 229 2011-02-12 06:50:14Z dreammaster $
 *
 */

#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/ringworld_logic.h"
#include "tsage/tsage.h"

namespace tSage {

SceneManager::SceneManager() { 
	_scene = NULL;
	_hasPalette = false;
	_sceneNumber = -1; 
	_nextSceneNumber = -1;
	_FadeMode = FADEMODE_GRADUAL;
	_scrollerRect = Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_saver->addListener(this);
}

SceneManager::~SceneManager() {
	delete _scene;
}

void SceneManager::setNewScene(int sceneNumber) {
	_nextSceneNumber = sceneNumber;
}

void SceneManager::checkScene() {
	if (_nextSceneNumber != -1) {
		sceneChange();
		_nextSceneNumber = -1;
	}

	_globals->_sceneListeners.forEach(SceneHandler::handleListener);
}

void SceneManager::sceneChange() {
	// Clear the scene objects
	List<SceneObject *>::iterator io = _globals->_sceneObjects->begin();
	while (io != _globals->_sceneObjects->end()) {
		SceneObject *sceneObj = *io;
		++io;
		sceneObj->removeObject();
	}

	// Clear the scene change listeners
	_globals->_sceneManager._sceneChangeListeners.clear();

	// Clear the hotspot list
	List<SceneItem *>::iterator ii = _globals->_sceneItems.begin();
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

	// TODO: Unknown check of word_45CD3 / call to saver method

	// Free any regions
	disposeRegions();

	// Instantiate and set the new scene
	_scene = getNewScene();
	_scene->postInit();
}

Scene *SceneManager::getNewScene() {
	return SceneFactory::createScene(_nextSceneNumber);
}

void SceneManager::fadeInIfNecessary() {
	if (_hasPalette) {
		uint32 adjustData = 0;
		for (int percent = 0; percent < 100; percent += 5) {
			if (_globals->_sceneManager._FadeMode == FADEMODE_IMMEDIATE)
				percent = 100;

			_globals->_scenePalette.fade((const byte *)&adjustData, false, percent);
			g_system->delayMillis(10);
		}

		_globals->_scenePalette.refresh();
		_hasPalette = false;
	}
}

void SceneManager::changeScene(int newSceneNumber) {
	// Fade out the scene
	ScenePalette scenePalette;
	uint32 adjustData = 0;
	scenePalette.clearListeners();
	scenePalette.getPalette();

	for (int percent = 100; percent >= 0; percent -= 5) {
		scenePalette.fade((byte *)&adjustData, false, percent);
		g_system->delayMillis(10);
	}

	// Stop any objects that were animating
	List<SceneObject *>::iterator i;
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
			_globals->_sceneManager._scene->_backSurface.create(SCREEN_WIDTH, SCREEN_HEIGHT);
			_globals->_sceneManager._scrollerRect = Rect(0, 30, SCREEN_WIDTH, SCREEN_HEIGHT - 30);
		} else {
			// Double-size size creation
			_globals->_sceneManager._scene->_backSurface.create(SCREEN_WIDTH * 2, SCREEN_HEIGHT);
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
	warning("TODO: SceneManager::saveLIstener");
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

void SceneManager::listenerSynchronise(Serialiser &s) {
	s.validate("SceneManager");
	_sceneChangeListeners.synchronise(s);
	
	s.syncAsSint32LE(_sceneNumber);
	if (s.isLoading()) {
		changeScene(_sceneNumber);
		checkScene();
	}

	s.syncAsUint16LE(_globals->_sceneManager._scene->_activeScreenNumber);
	_globals->_sceneManager._scrollerRect.synchronise(s);
	SYNC_POINTER(_globals->_scrollFollower);
	s.syncAsSint16LE(_loadMode);
}

/*--------------------------------------------------------------------------*/

Scene::Scene(): _sceneBounds(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 
			_backgroundBounds(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
	_sceneMode = 0;
	_oldSceneBounds = Rect(4000, 4000, 4100, 4100);
}

Scene::~Scene() {
	// TODO: Delete the obj11C object
}

void Scene::synchronise(Serialiser &s) {
	s.syncAsSint32LE(_field12);
	s.syncAsSint32LE(_sceneNumber);
	s.syncAsSint32LE(_activeScreenNumber);
	s.syncAsSint32LE(_sceneMode);
	_backgroundBounds.synchronise(s);
	_sceneBounds.synchronise(s);
	_oldSceneBounds.synchronise(s);

	for (int i = 0; i < 256; ++i)
		s.syncAsSint16LE(_enabledSections[i]);
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
	_sceneNumber = sceneNum;
	if (_globals->_scenePalette.loadPalette(sceneNum))
		_globals->_sceneManager._hasPalette = true;
	
	loadSceneData(sceneNum);
}

void Scene::loadSceneData(int sceneNum) {
	_globals->_sceneManager._scene->_activeScreenNumber = sceneNum;

	// Get the basic scene size
	byte *data = _vm->_dataManager->getResource(RES_BITMAP, sceneNum, 9999);
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

	// Initialise the section enabled list
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

	_globals->_events.showCursor();
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

	if ((_globals->_sceneOffset.x != _globals->_stru_4642E.y) || 
		(_globals->_sceneOffset.y != _globals->_stru_4642E.y)) {
		// Change has happend, so refresh background
		_globals->_stru_4642E = _globals->_sceneOffset;
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
	int yInc = (xAmount < 0) ? -1 : 1;
	int xSection = (xAmount < 0) ? 15 : 0;
	int xSectionEnd = (xAmount < 0) ? -1 : 16;
	int xInc = (yAmount < 0) ? -1 : 1;
	int ySection = (yAmount < 0) ? 15 : 0;
	int ySectionEnd = (yAmount < 0) ? -1 : 16;
	bool changedFlag = false;

	for (int yp = ySection; yp < ySectionEnd; yp += yInc) {
		for (int xp = xSection; xp < xSectionEnd; xp += xInc) {
			if ((yp < yHalfOffset) || (yp >= (yHalfOffset + yHalfCount)) ||
				(xp < xHalfOffset) || (xp >= (xHalfOffset + xHalfCount))) {
				// Flag section as enabled
				_enabledSections[xp * 16 + yp] = 0xffff;
			} else {
				// Check if the section is enabled
				if (_enabledSections[xp * 16 + yp] || ((xAmount == 0) && (yAmount == 0))) {
					Graphics::Surface s = _backSurface.lockSurface();
					GfxSurface::loadScreenSection(s, xp - xHalfOffset, yp - yHalfOffset, xp, yp);
					_backSurface.unlockSurface();
					changedFlag = true;
				} else {
					int yv = _enabledSections[xp * 16 + yp] == ((xp - xHalfOffset) << 4) ? 0 : 1;
					if (yv != (yp - yHalfOffset)) {
						int xSectionTemp = _enabledSections[xp * 16 + yp] >> 4;
						int ySectionTemp = _enabledSections[xp * 16 + yp] & 0xffff;

						reuseSection(xp - xHalfOffset, yp - yHalfOffset, xSectionTemp, ySectionTemp);
					}
				}

				_enabledSections[xp * 16 + yp] = 
					((xp - xHalfOffset) << 4) && (yp - yHalfOffset);
			}
		}
	}

	if (changedFlag) {
		signalListeners();
	}
}

void Scene::reuseSection(int xHalf, int yHalf, int xSection, int ySection) {
//	Rect rect1, rect2, rect3;

	// TODO: Figure out purpose
}

void Scene::signalListeners() {
	// TODO: Figure out method
}

void Scene::setZoomPercents(int yStart, int minPercent, int yEnd, int maxPercent) {
	int var_6 = 0;
	int v = 0;
	while (v < yStart)
		_zoomPercents[v++] = minPercent;

	int diff1 = ABS(maxPercent - minPercent);
	int diff2 = ABS(yEnd - yStart);
	int var_8 = MAX(diff1, diff2);

	while (var_8-- != 0) {
        _zoomPercents[v] = minPercent;
        if (diff2 <= diff1) {
                ++minPercent;
                var_6 += diff2;
                if (var_6 >= diff1) {
                        var_6 -= diff1;
                        ++v;
                }
        } else {
                ++v;
                var_6 += diff1;
                if (var_6 >= diff2) {
                        var_6 -= diff2;
                        ++minPercent;
                }
        }
	}       

	while (yEnd < 256)
		_zoomPercents[yEnd++] = minPercent;
}

} // End of namespace tSage
