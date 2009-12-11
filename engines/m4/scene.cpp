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
#include "m4/compression.h"

namespace M4 {

static const int INV_ANIM_FRAME_SPEED = 8;
static const int INVENTORY_X = 160;
static const int INVENTORY_Y = 159;
static const int SCROLLER_DELAY = 200;


Scene::Scene(M4Engine *vm): View(vm, Common::Rect(0, 0, vm->_screen->width(), vm->_screen->height())) {
	_screenType = VIEWID_SCENE;

	_sceneResources.hotspots = new HotSpotList();
	_sceneResources.parallax = new HotSpotList();
	_sceneResources.props = new HotSpotList();
	_backgroundSurface = new M4Surface();
	_codeSurface = new M4Surface();
	_madsInterfaceSurface = new MadsInterfaceView(vm);
	_sceneSprites = NULL;
	_palData = NULL;
	_interfacePal = NULL;
	_inverseColorTable = NULL;
	strcpy(_statusText, "");
	_vm->_rails->setCodeSurface(_codeSurface);
	_currentAction = kVerbNone;
}

Scene::~Scene() {
	_sceneResources.hotspots->clear();
	_sceneResources.parallax->clear();
	_sceneResources.props->clear();

	delete _sceneResources.hotspots;
	delete _sceneResources.parallax;
	delete _sceneResources.props;

	delete _backgroundSurface;
	delete _codeSurface;

	if (_sceneSprites)
		delete _sceneSprites;

	_vm->_palette->deleteAllRanges();

	if (_palData)
		delete _palData;

	if (_interfacePal)
		delete _interfacePal;

	if (_inverseColorTable)
		delete[] _inverseColorTable;

}

void Scene::loadScene(int sceneNumber) {
	_currentScene = sceneNumber;

	// Close the menu if it's active
	if (!_vm->isM4()) {
		View *mainMenu = _vm->_viewManager->getView(VIEWID_MAINMENU);
		if (mainMenu != NULL) {
			_vm->_viewManager->deleteView(mainMenu);
		}
	}

	// TODO: Check if we were loading a game


	// Load scene background and set palette
	if (_palData) {
		_vm->_palette->deleteRange(_palData);
		delete _palData;
	}

	if (_interfacePal) {
		_vm->_palette->deleteRange(_interfacePal);
		delete _interfacePal;
	}

	if (_vm->isM4()) {
		_backgroundSurface->loadBackground(sceneNumber);
		_palData = NULL;
	} else {
		// Set system palette entries
		_vm->_palette->blockRange(0, 7);
		RGB8 sysColors[3] = { {0x1f<<2, 0x2d<<2, 0x31<<2, 0}, {0x24<<2, 0x37<<2, 0x3a<<2, 0}, 
			{0x00<<2, 0x10<<2, 0x16<<2, 0}};
		_vm->_palette->setPalette(&sysColors[0], 4, 3);

		_backgroundSurface->loadBackground(sceneNumber, &_palData);
		_vm->_palette->addRange(_palData);
		_backgroundSurface->translate(_palData);

		if (sceneNumber < 900) {
			/*_backgroundSurface->fillRect(Common::Rect(0, MADS_SURFACE_HEIGHT,
				_backgroundSurface->width(), _backgroundSurface->height()),
				_vm->_palette->BLACK);*/
			// TODO: interface palette
			_madsInterfaceSurface->madsloadInterface(0, &_interfacePal);
			_vm->_palette->addRange(_interfacePal);
			_madsInterfaceSurface->translate(_interfacePal);
			_backgroundSurface->copyFrom(_madsInterfaceSurface, Common::Rect(0, 0, 320, 44), 0, 200 - 44);

			_madsInterfaceSurface->initialise();
		}
	}

	if (_vm->getGameType() == GType_Burger &&
		sceneNumber != TITLE_SCENE_BURGER && sceneNumber != MAINMENU_SCENE_BURGER)
		_vm->_interfaceView->setStatusText("");

	// Load scene def file (*.CHK)
	if (_vm->isM4()) {
		loadSceneResources(sceneNumber);
		loadSceneInverseColorTable(sceneNumber);
	} else {
		// Don't load other screen resources for system screens
		if (sceneNumber >= 900)
			return;

		loadSceneHotSpotsMads(sceneNumber);
	}

	// TODO: set walker scaling
	// TODO: destroy woodscript buffer

	// Load scene walk path file (*.COD/*.WW?)
	loadSceneCodes(sceneNumber);

	// Load inverse color table file (*.IPL)
	loadSceneInverseColorTable(sceneNumber);

	if (_vm->isM4()) {

		if (_vm->getGameType() != GType_Burger) {
			// Load scene sprites file (*.SSB)
			loadSceneSprites(sceneNumber);

			// Load scene sprite codes file (*.SSC)
			loadSceneSpriteCodes(sceneNumber);
		}


		if (sceneNumber != TITLE_SCENE_BURGER && sceneNumber != MAINMENU_SCENE_BURGER) {
			_vm->_interfaceView->show();
			showSprites();
		}
	}

	// Purge resources
	_vm->res()->purge();
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

void Scene::loadSceneCodes(int sceneNumber, int index) {
	char filename[kM4MaxFilenameSize];
	Common::SeekableReadStream *sceneS;

	if (_vm->isM4()) {
		sprintf(filename, "%i.cod", sceneNumber);
		sceneS = _vm->res()->openFile(filename);
		_codeSurface->loadCodesM4(sceneS);
		_vm->res()->toss(filename);
	} else if (_vm->getGameType() == GType_Phantom || _vm->getGameType() == GType_DragonSphere) {
		sprintf(filename, "rm%i.ww%i", sceneNumber, index);
		MadsPack walkData(filename, _vm);
		sceneS = walkData.getItemStream(0);
		_codeSurface->loadCodesMads(sceneS);
		_vm->res()->toss(filename);
	} else if (_vm->getGameType() == GType_RexNebular) {
		// TODO
		return;
	}
}

void Scene::loadSceneInverseColorTable(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	Common::SeekableReadStream *iplS;

	if (_vm->isM4()) {
		sprintf(filename, "%i.ipl", sceneNumber);
		iplS = _vm->res()->openFile(filename);
		if (_inverseColorTable)
			delete[] _inverseColorTable;
		_inverseColorTable = new byte[iplS->size()];
		iplS->read(_inverseColorTable, iplS->size());
		_vm->res()->toss(filename);
	} else {
		// TODO?
		return;
	}

}


void Scene::loadSceneSprites(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	sprintf(filename, "%i.ssb", sceneNumber);

	Common::SeekableReadStream *sceneS = _vm->res()->get(filename);
	_sceneSprites = new SpriteAsset(_vm, sceneS, sceneS->size(), filename);
	_vm->res()->toss(filename);

	printf("Scene has %d sprites, each one having %d colors\n", _sceneSprites->getCount(), _sceneSprites->getColorCount());
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

void Scene::checkHotspotAtMousePos(int x, int y) {
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
			_vm->_interfaceView->_inventory.getSelectedIndex() == -1) {
			_vm->_mouse->setCursorNum(currentHotSpot->getCursor());
		}
		_vm->_interfaceView->setStatusText(currentHotSpot->getPrep());
	} else {
		if (_vm->_mouse->getCursorNum() != CURSOR_LOOK &&
			_vm->_mouse->getCursorNum() != CURSOR_TAKE &&
			_vm->_mouse->getCursorNum() != CURSOR_USE &&
			_vm->_interfaceView->_inventory.getSelectedIndex() == -1) {
			_vm->_mouse->setCursorNum(0);
		} else {

		}
	}
}

void Scene::checkHotspotAtMousePosMads(int x, int y) {
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

		sprintf(statusText, "%s %s\n", _vm->_globals->getVocab(verbId), currentHotSpot->getVocab());

		statusText[0] = toupper(statusText[0]);	// capitalize first letter
		setMADSStatusText(statusText);
	} else {
		_vm->_mouse->setCursorNum(0);
		setMADSStatusText("");
	}
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

// Test function, shows all scene codes
void Scene::showCodes() {
	uint8 *pixelData = _codeSurface->getBasePtr(0, 0);
	for (int i = 0; i < _codeSurface->width() * _codeSurface->height(); i++)
		if (pixelData[i] & 0x10)
			pixelData[i] = 0xFF;
		else
			pixelData[i] = 0;

	byte colors[256 * 4];
	memset(colors, 0, sizeof(colors));
	colors[255 * 4 + 0] = 255;
	colors[255 * 4 + 1] = 255;
	colors[255 * 4 + 2] = 255;
	_vm->_palette->setPalette(colors, 0, 256);

	_backgroundSurface->copyFrom(_codeSurface, Common::Rect(0, 0, 640, 480), 0, 0);
	//_system->copyRectToScreen(codes->getBasePtr(0, 0), codes->w, 0, 0, codes->w, codes->h);
}

void Scene::playIntro() {

}

void Scene::show() {
	_vm->_viewManager->addView(this);
	_vm->_viewManager->addView(_madsInterfaceSurface);
}

void Scene::update() {
	// TODO: Needs a proper implementation
	// NOTE: Don't copy the background when in M4 mode or WoodScript anims won't be shown
	if (!_vm->isM4()) {
		_backgroundSurface->copyTo(this);

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

		_madsInterfaceSurface->copyTo(this, 0, this->height() - _madsInterfaceSurface->height());
	}
}

void Scene::onRefresh(RectList *rects, M4Surface *destSurface) {
	update();
	View::onRefresh(rects, destSurface);
}

bool Scene::onEvent(M4EventType eventType, int param1, int x, int y, bool &captureEvents) {
	//if (_vm->getGameType() != GType_Burger)
	//	return false;

	// If the game is currently paused, don't do any scene processing
	if (_vm->_kernel->paused)
		return false;

	switch (eventType) {
	case MEVENT_LEFT_CLICK:
		{
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
					if (_vm->_interfaceView->_inventory.getSelectedIndex() == -1) {
						if (_vm->_mouse->getVerb() == NULL) {
							strcpy(_vm->_player->verb, currentHotSpot->getVerb());
						} else {
							strcpy(_vm->_player->verb, _vm->_mouse->getVerb());
						}
					} else {
						strcpy(_vm->_player->verb, _vm->_interfaceView->_inventory.getSelectedObjectName());
					}
					strcpy(_vm->_player->noun, currentHotSpot->getVocab());
					strcpy(_vm->_player->object, "");
					_vm->_player->commandReady = true;

					printf("## Player said: %s %s\n", _vm->_player->verb, _vm->_player->noun);

				}
			}

			if (!_vm->isM4()) {
				HotSpot *currentHotSpot = _sceneResources.hotspots->findByXY(x, y);
				if (currentHotSpot != NULL) {
					char statusText[50];
					if (currentHotSpot->getVerbID() != 0) {
						sprintf(statusText, "%s %s\n", currentHotSpot->getVerb(), currentHotSpot->getVocab());
					} else {
						sprintf(statusText, "%s %s\n", _vm->_globals->getVocab(kVerbWalkTo), currentHotSpot->getVocab());
					}

					statusText[0] = toupper(statusText[0]);	// capitalize first letter
					setMADSStatusText(statusText);
				}
			}
		}
		break;
	case MEVENT_RIGHT_CLICK:
		if (_vm->getGameType() == GType_Burger) {
			nextCommonCursor();
			_vm->_interfaceView->_inventory.clearSelected();
		} else {
			// ***DEBUG*** - sample dialog display
			const char *msg = _vm->_globals->loadMessage(10);
			Dialog *dlg = new Dialog(_vm, msg);
			_vm->_viewManager->addView(dlg);
			_vm->_viewManager->moveToFront(dlg);
		}
		break;
	case MEVENT_MOVE:
		if (_vm->isM4())
			checkHotspotAtMousePos(x, y);
		else
			checkHotspotAtMousePosMads(x, y);
		break;
	default:
		return false;
	}

	return true;
}

void Scene::nextCommonCursor() {
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

void Scene::setAction(int action, int objectId) {
	VALIDATE_MADS;
	char statusText[50];

	// TODO: Actually executing actions directly for objects. Also, some object actions are special in that
	// a second object can be selected, as in 'use gun to shoot person', with requires a target

	// Set up the new action
	strcpy(statusText, _vm->_globals->getVocab(action));
	statusText[0] = toupper(statusText[0]);	// capitalize first letter
	
	if (objectId != -1) {
		MadsObject *obj = _vm->_globals->getObject(objectId);
		sprintf(statusText + strlen(statusText), " %s", _vm->_globals->getVocab(obj->descId));
	} else {
		_currentAction = action;
	}

	setMADSStatusText(statusText);
}


/*--------------------------------------------------------------------------
 * MadsInterfaceView handles the user interface section at the bottom of
 * game screens in MADS games
 *--------------------------------------------------------------------------
 */

MadsInterfaceView::MadsInterfaceView(M4Engine *vm): View(vm, Common::Rect(0, MADS_SURFACE_HEIGHT,
														 vm->_screen->width(), vm->_screen->height())) {
	_screenType = VIEWID_INTERFACE;
	_highlightedElement = -1;
	_topIndex = 0;
	_selectedObject = -1;

	_objectSprites = NULL;
	_objectPalData = NULL;

	/* Set up the rect list for screen elements */
	// Actions
	for (int i = 0; i < 10; ++i)
		_screenObjects.addRect((i / 5) * 32 + 1, (i % 5) * 8 + MADS_SURFACE_HEIGHT + 2,
			((i / 5) + 1) * 32 + 3, ((i % 5) + 1) * 8 + MADS_SURFACE_HEIGHT + 2);
	
	// Scroller elements (up arrow, scroller, down arrow)
	_screenObjects.addRect(73, 160, 82, 167);
	_screenObjects.addRect(73, 168, 82, 190);
	_screenObjects.addRect(73, 191, 82, 198);

	// Inventory object names
	for (int i = 0; i < 5; ++i)
		_screenObjects.addRect(89, 158 + i * 8, 160, 166 + i * 8);
	
	// Full rectangle area for all vocab actions
	for (int i = 0; i < 5; ++i)
		_screenObjects.addRect(239, 158 + i * 8, 320, 166 + i * 8);
}

MadsInterfaceView::~MadsInterfaceView() {
	delete _objectSprites;
}

void MadsInterfaceView::setFontMode(InterfaceFontMode newMode) {
	switch (newMode) {
	case ITEM_NORMAL:
		_vm->_font->setColors(4, 4, 0xff);
		break;
	case ITEM_HIGHLIGHTED:
		_vm->_font->setColors(5, 5, 0xff);
		break;
	case ITEM_SELECTED:
		_vm->_font->setColors(6, 6, 0xff);
		break;
	}
}

void MadsInterfaceView::initialise() {
	// Build up the inventory list
	_inventoryList.clear();

	for (uint i = 0; i < _vm->_globals->getObjectsSize(); ++i) {
		MadsObject *obj = _vm->_globals->getObject(i);
		if (obj->roomNumber == PLAYER_INVENTORY)
			_inventoryList.push_back(i);
	}

	// If the inventory has at least one object, select it
	if (_inventoryList.size() > 0)
		setSelectedObject(_inventoryList[0]);
}

void MadsInterfaceView::setSelectedObject(int objectNumber) {
	char resName[80];

	// Load inventory resource
	if (_objectSprites) {
		_vm->_palette->deleteRange(_objectPalData);
		delete _objectSprites;
	}

	// Check to make sure the object is in the inventory, and also visible on-screen
	int idx = _inventoryList.indexOf(objectNumber);
	if (idx == -1) {
		// Object wasn't found, so return
		_selectedObject = -1;
		return;
	}

	// Found the object
	if (idx < _topIndex)
		_topIndex = idx;
	else if (idx >= (_topIndex + 5))
		_topIndex = MAX(0, idx - 4);
	
	_selectedObject = objectNumber;
	sprintf(resName, "*OB%.3dI.SS", objectNumber);

	Common::SeekableReadStream *data = _vm->res()->get(resName);
	_objectSprites = new SpriteAsset(_vm, data, data->size(), resName);
	_vm->res()->toss(resName);

	// Slot it into available palette space
	_objectPalData = _objectSprites->getRgbList();
	_vm->_palette->addRange(_objectPalData);
	_objectSprites->translate(_objectPalData, true);

	_objectFrameNumber = 0;
}

void MadsInterfaceView::addObjectToInventory(int objectNumber) {
	if (_inventoryList.indexOf(objectNumber) == -1) {
		_vm->_globals->getObject(objectNumber)->roomNumber = PLAYER_INVENTORY;
		_inventoryList.push_back(objectNumber);
	}

	setSelectedObject(objectNumber);
}

void MadsInterfaceView::onRefresh(RectList *rects, M4Surface *destSurface) {
	_vm->_font->setFont(FONT_INTERFACE_MADS);
	char buffer[100];

	// Highlighting logic for action list
	int actionIndex = 0;
	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 5; ++y, ++actionIndex) {
			// Determine the font colour depending on whether an item is selected. Note that the first action,
			// 'Look', is always 'selected', even when another action is clicked on
			setFontMode((_highlightedElement == actionIndex) ? ITEM_HIGHLIGHTED : 
				((actionIndex == 0) ? ITEM_SELECTED : ITEM_NORMAL));

			// Get the verb action and capitalise it
			const char *verbStr = _vm->_globals->getVocab(kVerbLook + actionIndex);
			strcpy(buffer, verbStr);
			if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

			// Display the verb
			const Common::Rect r(_screenObjects[actionIndex]);
			_vm->_font->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
		}
	}

	// Check for highlighting of the scrollbar controls
	if ((_highlightedElement == SCROLL_UP) || (_highlightedElement == SCROLL_SCROLLER) || (_highlightedElement == SCROLL_DOWN)) {
		// Highlight the control's borders
		const Common::Rect r(_screenObjects[_highlightedElement]);
		destSurface->frameRect(r, 5);
	}

	// Draw the horizontal line in the scroller representing the current top selected 
	const Common::Rect scroller(_screenObjects[SCROLL_SCROLLER]);
	int yP = (_inventoryList.size() < 2) ? 0 : (scroller.height() - 5) * _topIndex / (_inventoryList.size() - 1);
	destSurface->setColor(4);
	destSurface->hLine(scroller.left + 2, scroller.right - 3, scroller.top + 2 + yP);

	// List inventory items
	for (uint i = 0; i < 5; ++i) {
		if ((_topIndex + i) >= _inventoryList.size())
			break;

		const char *descStr = _vm->_globals->getVocab(_vm->_globals->getObject(
			_inventoryList[_topIndex + i])->descId);
		strcpy(buffer, descStr);
		if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

		const Common::Rect r(_screenObjects[INVLIST_START + i]);
	
		// Set the highlighting of the inventory item
		if (_highlightedElement == (int)(INVLIST_START + i)) setFontMode(ITEM_HIGHLIGHTED);
		else if (_selectedObject == _inventoryList[_topIndex + i]) setFontMode(ITEM_SELECTED);
		else setFontMode(ITEM_NORMAL);

		// Write out it's description
		_vm->_font->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
	}

	// Handle the display of any currently selected object
	if (_objectSprites) {
		// Display object sprite. Note that the frame number isn't used directly, because it would result
		// in too fast an animation
		M4Sprite *spr = _objectSprites->getFrame(_objectFrameNumber / INV_ANIM_FRAME_SPEED);
		spr->copyTo(destSurface, INVENTORY_X, INVENTORY_Y, 0);

		if (!_vm->_globals->invObjectsStill) {
			// If objetcs are to animated, move to the next frame
			if (++_objectFrameNumber >= (_objectSprites->getCount() * INV_ANIM_FRAME_SPEED))
				_objectFrameNumber = 0;
		}

		// List the vocab actions for the currently selected object
		MadsObject *obj = _vm->_globals->getObject(_selectedObject);
		int yIndex = MIN(_highlightedElement - VOCAB_START, obj->vocabCount - 1);

		for (int i = 0; i < obj->vocabCount; ++i) {
			const Common::Rect r(_screenObjects[VOCAB_START + i]);

			// Get the vocab description and capitalise it
			const char *descStr = _vm->_globals->getVocab(obj->vocabList[i].vocabId);
			strcpy(buffer, descStr);
			if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';
		
			// Set the highlighting and display the entry
			setFontMode((i == yIndex) ? ITEM_HIGHLIGHTED : ITEM_NORMAL);
			_vm->_font->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
		}
	}
}

bool MadsInterfaceView::onEvent(M4EventType eventType, int param1, int x, int y, bool &captureEvents) {
	// If the mouse isn't being held down, then reset the repeated scroll timer
	if (eventType != MEVENT_LEFT_HOLD)
		_nextScrollerTicks = 0;

	// Handle various event types
	switch (eventType) {
	case MEVENT_MOVE:
		// If the cursor isn't in "wait mode", don't do any processing
		if (_vm->_mouse->getCursorNum() == CURSOR_WAIT)
			return true;

		// Ensure the cursor is the standard arrow
		_vm->_mouse->setCursorNum(CURSOR_ARROW);

		// Check if any interface element is currently highlighted
		_highlightedElement = _screenObjects.find(Common::Point(x, y));

		return true;
	
	case MEVENT_LEFT_CLICK:
		// Left mouse click
		// Check if an inventory object was selected
		if ((_highlightedElement >= INVLIST_START) && (_highlightedElement < (INVLIST_START + 5))) {
			// Ensure there is an inventory item listed in that cell
			uint idx = _highlightedElement - INVLIST_START;
			if ((_topIndex + idx) < _inventoryList.size()) {
				// Set the selected object
				setSelectedObject(_inventoryList[_topIndex + idx]);
			}
		} else if ((_highlightedElement >= ACTIONS_START) && (_highlightedElement < (ACTIONS_START + 10))) {
			// A standard action was selected
			_vm->_scene->setAction(kVerbLook + (_highlightedElement - ACTIONS_START));
		} else if ((_highlightedElement >= VOCAB_START) && (_highlightedElement < (VOCAB_START + 5))) {
			// A vocab action was selected
			MadsObject *obj = _vm->_globals->getObject(_selectedObject);
			int vocabIndex = MIN(_highlightedElement - VOCAB_START, obj->vocabCount - 1);
			if (vocabIndex >= 0)
				_vm->_scene->setAction(obj->vocabList[vocabIndex].vocabId, _selectedObject);
		}

		return true;

	case MEVENT_LEFT_HOLD:
		// Left mouse hold
		// Handle the scroller - the up/down buttons allow for multiple actions whilst the mouse is held down
		if ((_highlightedElement == SCROLL_UP) || (_highlightedElement == SCROLL_DOWN)) {
			if ((_nextScrollerTicks == 0) || (g_system->getMillis() >= _nextScrollerTicks)) {
				// Handle scroll up/down action
				_nextScrollerTicks = g_system->getMillis() + SCROLLER_DELAY;

				if ((_highlightedElement == SCROLL_UP) && (_topIndex > 0))
					--_topIndex;
				if ((_highlightedElement == SCROLL_DOWN) && (_topIndex < (int)(_inventoryList.size() - 1)))
					++_topIndex;
			}
		}
		return true;

	case MEVENT_LEFT_DRAG:
		// Left mouse drag
		// Handle the the the scroller area that can be dragged to adjust the top displayed index
		if (_highlightedElement == SCROLL_SCROLLER) {
			// Calculate the new top index based on the Y position
			const Common::Rect r(_screenObjects[SCROLL_SCROLLER]);
			_topIndex = CLIP((int)(_inventoryList.size() - 1) * (y - r.top - 2) / (r.height() - 5),
				0, (int)_inventoryList.size() - 1);
		}
		return true;

	default:
		break;
	}

	return false;
}

} // End of namespace M4
