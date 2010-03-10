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

Scene::Scene(MadsM4Engine *vm, SceneResources *res): View(vm, Common::Rect(0, 0, vm->_screen->width(), 
			vm->_screen->height())), _sceneResources(res) {
	_screenType = VIEWID_SCENE;

	_sceneResources->hotspots = new HotSpotList();
	_sceneResources->parallax = new HotSpotList();
	_sceneResources->props = new HotSpotList();
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
		sceneS->read(_sceneResources->artBase, MAX_CHK_FILENAME_SIZE);
		sceneS->read(_sceneResources->pictureBase, MAX_CHK_FILENAME_SIZE);
		_sceneResources->hotspotCount = sceneS->readUint32LE();
		_sceneResources->parallaxCount = sceneS->readUint32LE();
		_sceneResources->propsCount = sceneS->readUint32LE();
		_sceneResources->frontY = sceneS->readUint32LE();
		_sceneResources->backY = sceneS->readUint32LE();
		_sceneResources->frontScale = sceneS->readUint32LE();
		_sceneResources->backScale = sceneS->readUint32LE();
		for (i = 0; i < 16; i++)
			_sceneResources->depthTable[i] = sceneS->readUint16LE();
		_sceneResources->railNodeCount = sceneS->readUint32LE();

		// Clear rails from previous scene
		_vm->_rails->clearRails();

		for (i = 0; i < _sceneResources->railNodeCount; i++) {
			x = sceneS->readUint32LE();
			y = sceneS->readUint32LE();
			if (_vm->_rails->addRailNode(x, y, true) < 0) {
				warning("Too many rail nodes defined for scene");
			}
		}

		// Clear current hotspot lists
		_sceneResources->hotspots->clear();
		_sceneResources->parallax->clear();
		_sceneResources->props->clear();

		_sceneResources->hotspots->loadHotSpots(sceneS, _sceneResources->hotspotCount);
		_sceneResources->parallax->loadHotSpots(sceneS, _sceneResources->parallaxCount);
		_sceneResources->props->loadHotSpots(sceneS, _sceneResources->propsCount);

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

	_sceneResources->hotspotCount = hotspotCount;

	hotspotStream = hotSpotData.getItemStream(1);

	// Clear current hotspot lists
	_sceneResources->hotspots->clear();

	_sceneResources->hotspots->loadHotSpots(hotspotStream, _sceneResources->hotspotCount);

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
	for (i = 0; i < _sceneResources->hotspotCount; i++) {
		currentHotSpot = _sceneResources->hotspots->get(i);
		_backgroundSurface->frameRect(currentHotSpot->getRect(), _vm->_palette->GREEN);
	}
	if (_vm->isM4()) {
		// parallax (yellow)
		for (i = 0; i < _sceneResources->parallaxCount; i++) {
			currentHotSpot = _sceneResources->parallax->get(i);
			_backgroundSurface->frameRect(currentHotSpot->getRect(), _vm->_palette->YELLOW);
		}
		// props (red)
		for (i = 0; i < _sceneResources->propsCount; i++) {
			currentHotSpot = _sceneResources->props->get(i);
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

} // End of namespace M4
