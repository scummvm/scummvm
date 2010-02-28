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
#include "m4/staticres.h"

namespace M4 {

Scene::Scene(MadsM4Engine *vm): View(vm, Common::Rect(0, 0, vm->_screen->width(), vm->_screen->height())) {
	_screenType = VIEWID_SCENE;

	_sceneResources.hotspots = new HotSpotList();
	_sceneResources.parallax = new HotSpotList();
	_sceneResources.props = new HotSpotList();
	_backgroundSurface = new M4Surface();
	_walkSurface = new M4Surface();
	_palData = NULL;
	_interfacePal = NULL;
	_interfaceSurface = NULL;
	_inverseColorTable = NULL;
	_vm->_rails->setCodeSurface(_walkSurface);
}

Scene::~Scene() {
	leaveScene();
}

void Scene::loadScene(int sceneNumber) {
	_previousScene = _currentScene;
	_currentScene = sceneNumber;

	// Load scene background and set palette
	if (_palData) {
		_vm->_palette->deleteRange(_palData);
		delete _palData;
	}

	if (_interfacePal) {
		_vm->_palette->deleteRange(_interfacePal);
		delete _interfacePal;
	}
}

void Scene::leaveScene() {
	delete _palData;
	delete _interfacePal;
	delete[] _inverseColorTable;
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

void Scene::loadSceneResources(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	int i = 0, x = 0, y = 0;
	sprintf(filename, "%i.chk", sceneNumber);

	Common::SeekableReadStream *sceneS = _vm->res()->get(filename);

	if (sceneS != NULL) {
		sceneS->read(_sceneResources.artBase, MAX_CHK_FILENAME_SIZE);
		sceneS->read(_sceneResources.pictureBase, MAX_CHK_FILENAME_SIZE);
		_sceneResources.hotspotCount = sceneS->readUint32LE();
		_sceneResources.parallaxCount = sceneS->readUint32LE();
		_sceneResources.propsCount = sceneS->readUint32LE();
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
		_sceneResources.props->clear();

		_sceneResources.hotspots->loadHotSpots(sceneS, _sceneResources.hotspotCount);
		_sceneResources.parallax->loadHotSpots(sceneS, _sceneResources.parallaxCount);
		_sceneResources.props->loadHotSpots(sceneS, _sceneResources.propsCount);

		// Note that toss() deletes the MemoryReadStream
		_vm->res()->toss(filename);
	}
}

void Scene::loadSceneHotSpotsMads(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	sprintf(filename, "rm%i.hh", sceneNumber);
	MadsPack hotSpotData(filename, _vm);
	Common::SeekableReadStream *hotspotStream = hotSpotData.getItemStream(0);

	int hotspotCount = hotspotStream->readUint16LE();
	delete hotspotStream;

	_sceneResources.hotspotCount = hotspotCount;

	hotspotStream = hotSpotData.getItemStream(1);

	// Clear current hotspot lists
	_sceneResources.hotspots->clear();

	_sceneResources.hotspots->loadHotSpots(hotspotStream, _sceneResources.hotspotCount);

	delete hotspotStream;
}

void Scene::loadSceneInverseColorTable(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	Common::SeekableReadStream *iplS;

	if (_vm->isM4()) {
		sprintf(filename, "%i.ipl", sceneNumber);
		iplS = _vm->res()->openFile(filename);
		delete[] _inverseColorTable;
		_inverseColorTable = new byte[iplS->size()];
		iplS->read(_inverseColorTable, iplS->size());
		_vm->res()->toss(filename);
	} else {
		// TODO?
		return;
	}

}

void Scene::loadSceneSpriteCodes(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	sprintf(filename, "%i.ssc", sceneNumber);

	Common::SeekableReadStream *sceneS = _vm->res()->get(filename);

	// TODO

	if (sceneS != NULL) {
		SpriteAsset* _sceneSpriteCodes = new SpriteAsset(_vm, sceneS, sceneS->size(), filename);
		int colorCount = _sceneSpriteCodes->getColorCount();
//			RGB8* spritePalette = _sceneSpriteCodes->getPalette();
		//_vm->_palette->setPalette(spritePalette, 0, colorCount);

		printf("Scene has %d sprite codes, each one having %d colors\n", _sceneSpriteCodes->getCount(), colorCount);

		// Note that toss() deletes the MemoryReadStream
		_vm->res()->toss(filename);
	}
}

void Scene::showSprites() {
	// TODO: This is all experimental code, it needs heavy restructuring
	// and cleanup

	// taken from set_walker_scaling() in adv_walk.cpp. A proper implementation will need
	// to store these in global variables
	int minScaling = FixedDiv(_sceneResources.backScale << 16, 100 << 16);
	int maxScaling = FixedDiv(_sceneResources.frontScale << 16, 100 << 16);
	int scaler;

	_vm->_actor->setWalkerDirection(kFacingSouthEast);
	//_vm->_actor->setWalkerPalette();

	// taken from set_walker_scaling() in adv_walk.cpp
	if (_sceneResources.frontY == _sceneResources.backY)
		scaler = 0;
	else
		scaler = FixedDiv(maxScaling - minScaling,
				 (_sceneResources.frontY << 16) - (_sceneResources.backY << 16));

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
	for (i = 0; i < _sceneResources.hotspotCount; i++) {
		currentHotSpot = _sceneResources.hotspots->get(i);
		_backgroundSurface->frameRect(currentHotSpot->getRect(), _vm->_palette->GREEN);
	}
	if (_vm->isM4()) {
		// parallax (yellow)
		for (i = 0; i < _sceneResources.parallaxCount; i++) {
			currentHotSpot = _sceneResources.parallax->get(i);
			_backgroundSurface->frameRect(currentHotSpot->getRect(), _vm->_palette->YELLOW);
		}
		// props (red)
		for (i = 0; i < _sceneResources.propsCount; i++) {
			currentHotSpot = _sceneResources.props->get(i);
			_backgroundSurface->frameRect(currentHotSpot->getRect(), _vm->_palette->RED);
		}
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

		byte colors[256 * 4];
		memset(colors, 0, sizeof(colors));
		colors[255 * 4 + 0] = 255;
		colors[255 * 4 + 1] = 255;
		colors[255 * 4 + 2] = 255;
		_vm->_palette->setPalette(colors, 0, 256);
	} else {
		// For MADS, simply copy the walk data to the background, in whatever current palette is active
		_walkSurface->copyTo(_backgroundSurface);
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
		checkHotspotAtMousePos(x, y);
		break;
	default:
		return false;
	}

	return true;
}

enum boxSprites {
	topLeft = 0,
	topRight = 1,
	bottomLeft = 2,
	bottomRight = 3,
	left = 4,
	right = 5,
	top = 6,
	bottom = 7,
	topMiddle = 8,
	filler1 = 9,
	filler2 = 10
	// TODO: finish this
};

// TODO: calculate width and height, show text, show face if it exists
// TODO: this has been tested with Dragonsphere only, there are some differences
// in the sprites used in Phantom
void Scene::showMADSV2TextBox(char *text, int x, int y, char *faceName) {
	int repeatX = 40;	// FIXME: this is hardcoded
	int repeatY = 30;	// FIXME: this is hardcoded
	int curX = x, curY = y;
	int topRightX = x;	// TODO: this is probably not needed
	Common::SeekableReadStream *data = _vm->res()->get("box.ss");
	SpriteAsset *boxSprites = new SpriteAsset(_vm, data, data->size(), "box.ss");
	_vm->res()->toss("box.ss");

	RGBList *palData = new RGBList(boxSprites->getColorCount(), boxSprites->getPalette(), true);
	_vm->_palette->addRange(palData);

	for (int i = 0; i < boxSprites->getCount(); i++)
		boxSprites->getFrame(i)->translate(palData);		// sprite pixel translation

	// Top left corner
	boxSprites->getFrame(topLeft)->copyTo(_backgroundSurface, x, curY);
	curX += boxSprites->getFrame(topLeft)->width();

	// Top line
	for (int i = 0; i < repeatX; i++) {
		boxSprites->getFrame(top)->copyTo(_backgroundSurface, curX, curY + 3);
		curX += boxSprites->getFrame(top)->width();
	}

	// Top right corner
	boxSprites->getFrame(topRight)->copyTo(_backgroundSurface, curX, curY);
	topRightX = curX;

	// Top middle
	// FIXME: the transparent color for this is also the black border color
	boxSprites->getFrame(topMiddle)->copyTo(_backgroundSurface,
											x + (curX - x) / 2 - boxSprites->getFrame(topMiddle)->width() / 2,
											curY - 5, 167);
	curX = x;
	curY += boxSprites->getFrame(topLeft)->height();

	// -----------------------------------------------------------------------------------------------

	// Draw contents
	for (int i = 0; i < repeatY; i++) {
		for (int j = 0; j < repeatX; j++) {
			if (j == 0) {
				boxSprites->getFrame(left)->copyTo(_backgroundSurface, curX + 3, curY);
				curX += boxSprites->getFrame(left)->width();
			} else if (j == repeatX - 1) {
				curX = topRightX - 2;
				boxSprites->getFrame(right)->copyTo(_backgroundSurface, curX + 3, curY + 1);
			} else {
				// TODO: the background of the contents follows a pattern which is not understood yet
				if (j % 2 == 0) {
					boxSprites->getFrame(filler1)->copyTo(_backgroundSurface, curX + 3, curY);
					curX += boxSprites->getFrame(filler1)->width();
				} else {
					boxSprites->getFrame(filler2)->copyTo(_backgroundSurface, curX + 3, curY);
					curX += boxSprites->getFrame(filler2)->width();
				}
			}
		}	// for j
		curX = x;
		curY += boxSprites->getFrame(left)->height();
	}	// for i

	// -----------------------------------------------------------------------------------------------
	curX = x;

	// Bottom left corner
	boxSprites->getFrame(bottomLeft)->copyTo(_backgroundSurface, curX, curY);
	curX += boxSprites->getFrame(bottomLeft)->width();

	// Bottom line
	for (int i = 0; i < repeatX; i++) {
		boxSprites->getFrame(bottom)->copyTo(_backgroundSurface, curX, curY + 1);
		curX += boxSprites->getFrame(bottom)->width();
	}

	// Bottom right corner
	boxSprites->getFrame(bottomRight)->copyTo(_backgroundSurface, curX, curY + 1);
}

/*--------------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------------*/

MadsScene::MadsScene(MadsEngine *vm): Scene(vm) {
	_vm = vm;

	strcpy(_statusText, "");
	_interfaceSurface = new MadsInterfaceView(vm);
	_currentAction = kVerbNone;
	_spriteSlotsStart = 0;
}

/**
 * Secondary scene loading code
 */
void MadsScene::loadScene2(const char *aaName) {
	// Load up the properties for the scene
	_sceneInfo.load(_currentScene);

	// Load scene walk paths
	loadSceneCodes(_currentScene);
}

/**
 * Existing ScummVM code that needs to be eventually replaced with MADS code
 */
void MadsScene::loadSceneTemporary() {
	/* Existing code that eventually needs to be replaced with the proper MADS code */
	// Set system palette entries
	_vm->_palette->blockRange(0, 7);
	RGB8 sysColors[3] = { {0x1f<<2, 0x2d<<2, 0x31<<2, 0}, {0x24<<2, 0x37<<2, 0x3a<<2, 0},
		{0x00<<2, 0x10<<2, 0x16<<2, 0}};
	_vm->_palette->setPalette(&sysColors[0], 4, 3);

	_backgroundSurface->loadBackground(_currentScene, &_palData);
	_vm->_palette->addRange(_palData);
	_backgroundSurface->translate(_palData);

	if (_currentScene < 900) {
		/*_backgroundSurface->fillRect(Common::Rect(0, MADS_SURFACE_HEIGHT,
			_backgroundSurface->width(), _backgroundSurface->height()),
			_vm->_palette->BLACK);*/
		// TODO: interface palette
		_interfaceSurface->madsloadInterface(0, &_interfacePal);
		_vm->_palette->addRange(_interfacePal);
		_interfaceSurface->translate(_interfacePal);
		_backgroundSurface->copyFrom(_interfaceSurface, Common::Rect(0, 0, 320, 44), 0, 200 - 44);

		_interfaceSurface->initialise();
	}

	// Don't load other screen resources for system screens
	if (_currentScene >= 900)
		return;

	loadSceneHotSpotsMads(_currentScene);

	// TODO: set walker scaling
	// TODO: destroy woodscript buffer

	// Load inverse color table file (*.IPL)
	loadSceneInverseColorTable(_currentScene);
}

void MadsScene::loadScene(int sceneNumber) {
	// Close the menu if it's active
	View *mainMenu = _vm->_viewManager->getView(VIEWID_MAINMENU);
	if (mainMenu != NULL) {
		_vm->_viewManager->deleteView(mainMenu);
	}

	// Handle common scene setting
	Scene::loadScene(sceneNumber);

	// Signal the script engine what scene is to be active
	_sceneLogic.selectScene(sceneNumber);
	_sceneLogic.setupScene();

	// Add the scene if necessary to the list of scenes that have been visited
	_vm->globals()->addVisitedScene(sceneNumber);

	// Secondary scene load routine
	loadScene2("*I0.AA");

	// Do any scene specific setup
	_sceneLogic.enterScene();

	// Existing ScummVM code that needs to be eventually replaced with MADS code
	loadSceneTemporary();

	// Purge resources
	_vm->res()->purge();
}

void MadsScene::leaveScene() {
	_sceneResources.hotspots->clear();
	_sceneResources.parallax->clear();
	_sceneResources.props->clear();

	delete _sceneResources.hotspots;
	delete _sceneResources.parallax;
	delete _sceneResources.props;

	// Delete the sprites
	for (uint i = 0; i <_sceneSprites.size(); ++i) delete _sceneSprites[i];
	_sceneSprites.clear();

	delete _backgroundSurface;
	delete _walkSurface;

	Scene::leaveScene();
}

void MadsScene::show() {
	Scene::show();
	_vm->_viewManager->addView(_interfaceSurface);
}

void MadsScene::loadSceneCodes(int sceneNumber, int index) {
	char filename[kM4MaxFilenameSize];
	Common::SeekableReadStream *sceneS;

	if (_vm->getGameType() == GType_Phantom || _vm->getGameType() == GType_DragonSphere) {
		sprintf(filename, "rm%i.ww%i", sceneNumber, index);
		MadsPack walkData(filename, _vm);
		sceneS = walkData.getItemStream(0);
		_walkSurface->loadCodesMads(sceneS);
		_vm->res()->toss(filename);
	} else if (_vm->getGameType() == GType_RexNebular) {
		// For Rex Nebular, the walk areas are part of the scene info
		byte *destP = _walkSurface->getBasePtr(0, 0);
		const byte *srcP = _sceneInfo.walkData;
		byte runLength;
		while ((runLength = *srcP++) != 0) {
			Common::set_to(destP, destP + runLength, *srcP++);
			destP += runLength;
		}
	}
}

void MadsScene::checkHotspotAtMousePos(int x, int y) {
	HotSpot *currentHotSpot = _sceneResources.hotspots->findByXY(x, y);
	if (currentHotSpot != NULL) {
		_vm->_mouse->setCursorNum(currentHotSpot->getCursor());

		// This is the "easy" interface, which updates the status text when the mouse is moved
		// TODO: toggle this code for easy/normal interface mode
		char statusText[50];
		int verbId = _currentAction;
		if (verbId == kVerbNone)
			verbId = currentHotSpot->getVerbID();
		if (verbId == kVerbNone)
			verbId = kVerbWalkTo;

		sprintf(statusText, "%s %s\n", _madsVm->globals()->getVocab(verbId), currentHotSpot->getVocab());

		statusText[0] = toupper(statusText[0]);	// capitalize first letter
		setStatusText(statusText);
	} else {
		_vm->_mouse->setCursorNum(0);
		setStatusText("");
	}
}

void MadsScene::leftClick(int x, int y) {
	HotSpot *currentHotSpot = _sceneResources.hotspots->findByXY(x, y);
	if (currentHotSpot != NULL) {
		char statusText[50];
		if (currentHotSpot->getVerbID() != 0) {
			sprintf(statusText, "%s %s\n", currentHotSpot->getVerb(), currentHotSpot->getVocab());
		} else {
			sprintf(statusText, "%s %s\n", _madsVm->globals()->getVocab(kVerbWalkTo), currentHotSpot->getVocab());
		}

		statusText[0] = toupper(statusText[0]);	// capitalize first letter
		setStatusText(statusText);
	}
}

void MadsScene::rightClick(int x, int y) {
	// ***DEBUG*** - sample dialog display
	int idx = 3; //_madsVm->_globals->messageIndexOf(0x277a);
	const char *msg = _madsVm->globals()->loadMessage(idx);
	Dialog *dlg = new Dialog(_vm, msg, "TEST DIALOG");
	_vm->_viewManager->addView(dlg);
	_vm->_viewManager->moveToFront(dlg);
}

void MadsScene::setAction(int action, int objectId) {
	VALIDATE_MADS;
	char statusText[50];

	// TODO: Actually executing actions directly for objects. Also, some object actions are special in that
	// a second object can be selected, as in 'use gun to shoot person', with requires a target

	// Set up the new action
	strcpy(statusText, _madsVm->globals()->getVocab(action));
	statusText[0] = toupper(statusText[0]);	// capitalize first letter

	if (objectId != -1) {
		MadsObject *obj = _madsVm->globals()->getObject(objectId);
		sprintf(statusText + strlen(statusText), " %s", _madsVm->globals()->getVocab(obj->descId));
	} else {
		_currentAction = action;
	}

	setStatusText(statusText);
}

void MadsScene::setStatusText(const char *text) {
	strcpy(_statusText, text);
}

/**
 * Draws all the elements of the scene
 */
void MadsScene::drawElements() {
	
	// Display animations
	for (int idx = 0; idx < _spriteSlotsStart; ++idx) {
		
	}


	// Text display loop
	for (int idx = 0; idx < TEXT_DISPLAY_SIZE; ++idx) {
		if (_textDisplay[idx].active && (_textDisplay[idx].field_A >= 0)) {
			_textDisplay[idx].font->setColours(0xFF, (_textDisplay[idx].colour2 == 0) ? 
				_textDisplay[idx].colour1 : _textDisplay[idx].colour2, _textDisplay[idx].colour1);
			_textDisplay[idx].font->writeString(this, _textDisplay[idx].message, 
				_textDisplay[idx].bounds.left, _textDisplay[idx].bounds.top, 
				width() - _textDisplay[idx].bounds.left, _textDisplay[idx].spacing);
		}
	}

	// Copy the user interface surface onto the surface
	_interfaceSurface->copyTo(this, 0, this->height() - _interfaceSurface->height());

/*
	// At this point, in the original engine, the dirty/changed areas were copied to the screen. At the moment,
	// the current M4 engine framework doesn't support dirty areas, but this is being kept in case that ever changes
	for (int idx = 0; idx < DIRTY_AREA_SIZE; ++idx) {
		if (_dirtyAreas[idx].active && _dirtyAreas[idx].active2 && 
			(_dirtyAreas[idx].bounds.width() > 0) && (_dirtyAreas[idx].bounds.height() > 0)) {
			// Copy changed area to screen

		}
	}
*/

	// Some kind of copying over of slot entries
	for (int idx = 0, idx2 = 0; idx < _spriteSlotsStart; ++idx) {
		if (_spriteSlots[idx].spriteId >= 0) {
			if (idx != idx2) {
				// Copy over the slot entry
				_spriteSlots[idx2] = _spriteSlots[idx];
			}
			++idx2;
		}
	}

	// Clear up any now inactive text display entries
	for (int idx = 0; idx < TEXT_DISPLAY_SIZE; ++idx) {
		if (_textDisplay[idx].field_A < 0) {
			_textDisplay[idx].active = false;
			_textDisplay[idx].field_A = 0;
		}
	}
}


void MadsScene::update() {
	// Copy the bare scene in
	_backgroundSurface->copyTo(this);

	// Draw all the various elements
	drawElements();

	// Handle display of any status text
	if (_statusText[0]) {
		// Text colors are inverted in Dragonsphere
		if (_vm->getGameType() == GType_DragonSphere)
			_vm->_font->setColors(_vm->_palette->BLACK, _vm->_palette->WHITE, _vm->_palette->BLACK);
		else
			_vm->_font->setColors(_vm->_palette->WHITE, _vm->_palette->BLACK, _vm->_palette->BLACK);

		_vm->_font->setFont(FONT_MAIN_MADS);
		_vm->_font->writeString(this, _statusText, (width() - _vm->_font->getWidth(_statusText)) / 2, 142, 0);
	}

	//***DEBUG***
	_sceneSprites[0]->getFrame(1)->copyTo(this, 120, 90, 0);
}

int MadsScene::loadSceneSpriteSet(const char *setName) {
	char resName[100];
	strcpy(resName, setName);

	// Append a '.SS' if it doesn't alreayd have an extension
	if (!strchr(resName, '.'))
		strcat(resName, ".SS");

	Common::SeekableReadStream *data = _vm->res()->get(resName);
	SpriteAsset *spriteSet = new SpriteAsset(_vm, data, data->size(), resName);
	spriteSet->translate(_vm->_palette);
	_vm->res()->toss(resName);

	_sceneSprites.push_back(spriteSet);
	return _sceneSprites.size() - 1;
}

void MadsScene::loadPlayerSprites(const char *prefix) {
	const char suffixList[8] = { '8', '9', '6', '3', '2', '7', '4', '1' };
	char setName[80];

	strcpy(setName, "*");
	strcat(setName, prefix);
	strcat(setName, "_0.SS");
	char *digitP = strchr(setName, '_') + 1;

	for (int idx = 0; idx < 8; ++idx) {
		*digitP = suffixList[idx];

		if (_vm->res()->resourceExists(setName)) {
			loadSceneSpriteSet(setName);
			return;
		}
	}

	error("Couldn't find player sprites");
}

/*--------------------------------------------------------------------------*/

void MadsSceneInfo::load(int sceneId) {
	const char *sceneInfoStr = MADSResourceManager::getResourceName(RESPREFIX_RM, sceneId, ".DAT");
	Common::SeekableReadStream *rawStream = _vm->_resourceManager->get(sceneInfoStr);
	MadsPack sceneInfo(rawStream);

	// Basic scene info
	Common::SeekableReadStream *stream = sceneInfo.getItemStream(0);

	int resSceneId = stream->readUint16LE();
	assert(resSceneId == sceneId);

	artFileNum = stream->readUint16LE();
	field_4 = stream->readUint16LE();
	width = stream->readUint16LE();
	height = stream->readUint16LE();
	assert((width == 320) && (height == 156));
	
	stream->skip(24);

	objectCount = stream->readUint16LE();

	stream->skip(40);

	for (int i = 0; i < objectCount; ++i) {
		objects[i].load(stream);
	}

	// For Rex Nebular, read in the scene's compressed walk surface information
	if (_vm->getGameType() == GType_RexNebular) {
		delete walkData;

		stream = sceneInfo.getItemStream(1);
		walkData = (byte *)malloc(stream->size());
		stream->read(walkData, stream->size());
	}

	_vm->_resourceManager->toss(sceneInfoStr);
}

} // End of namespace M4
