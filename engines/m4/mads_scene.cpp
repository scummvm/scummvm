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

#include "m4/mads_scene.h"
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
#include "m4/animation.h"

namespace M4 {

static const int INV_ANIM_FRAME_SPEED = 2;
static const int INVENTORY_X = 160;
static const int INVENTORY_Y = 159;
static const int SCROLLER_DELAY = 200;

//--------------------------------------------------------------------------

void SceneNode::load(Common::SeekableReadStream *stream) {
	// Get the next data block
	pt.x = stream->readUint16LE();
	pt.y = stream->readUint16LE();

	for (int i = 0; i < MAX_ROUTE_NODES; ++i)
		indexes[i] = stream->readUint16LE();
}

//--------------------------------------------------------------------------

MadsScene::MadsScene(MadsEngine *vm): _sceneResources(), Scene(vm, &_sceneResources), MadsView(this) {
	_vm = vm;
	_activeAnimation = NULL;

	MadsView::_bgSurface = Scene::_backgroundSurface;
	MadsView::_depthSurface = Scene::_walkSurface;
	_interfaceSurface = new MadsInterfaceView(vm);
	_showMousePos = false;
	_mouseMsgIndex = -1;
	_previousScene = -1;
}

MadsScene::~MadsScene() {
	delete _activeAnimation;
	_activeAnimation = NULL;
	leaveScene();
	_vm->_viewManager->deleteView(_interfaceSurface);
}

/**
 * Secondary scene loading code
 */
void MadsScene::loadScene2(const char *aaName, int sceneNumber) {
	// TODO: Completely finish
	_madsVm->globals()->previousScene = _madsVm->globals()->sceneNumber;
	_madsVm->globals()->sceneNumber = sceneNumber;

	_spriteSlots.clear();
	_sequenceList.clear();
	_kernelMessages.clear();

	// Load up the properties for the scene
	_sceneResources.load(_currentScene, NULL,  0/*word_83546*/, _walkSurface, _backgroundSurface);

	// Load scene walk paths
	loadSceneCodes(_currentScene);

	// Initialize the scene animation
	uint16 flags = 0x4100;
	if (_madsVm->globals()->_config.textWindowStill)
		flags |= 0x200;

	_sceneAnimation->initialize(aaName, flags, _interfaceSurface, NULL);
}

/**
 * Existing ScummVM code that needs to be eventually replaced with MADS code
 */
void MadsScene::loadSceneTemporary() {
	/* Existing code that eventually needs to be replaced with the proper MADS code */
	// Set system palette entries
	_vm->_palette->blockRange(0, 18);
	RGB8 sysColors[3] = { {0x1f<<2, 0x2d<<2, 0x31<<2}, {0x24<<2, 0x37<<2, 0x3a<<2},
		{0x00<<2, 0x10<<2, 0x16<<2}};
	_vm->_palette->setPalette(&sysColors[0], 4, 3);

	_interfaceSurface->initialize();

	loadSceneHotspots(_currentScene);

	_action.clear();
}

void MadsScene::loadScene(int sceneNumber) {
	// Close the menu if it's active
	View *mainMenu = _vm->_viewManager->getView(VIEWID_MAINMENU);
	if (mainMenu != NULL) {
		_vm->_viewManager->deleteView(mainMenu);
	}

	// Handle common scene setting
	Scene::loadScene(sceneNumber);
	_madsVm->globals()->_nextSceneId = sceneNumber;

	// Existing ScummVM code that needs to be eventually replaced with MADS code
	loadSceneTemporary();

	_madsVm->_player._spritesChanged = true;
	_madsVm->globals()->clearQuotes();
	_dynamicHotspots.reset();

	// Signal the script engine what scene is to be active
	_sceneLogic.selectScene(sceneNumber);

	// Add the scene if necessary to the list of scenes that have been visited
	_vm->globals()->addVisitedScene(sceneNumber);

	if (_vm->getGameType() == GType_RexNebular)
		_sceneLogic.setupScene();

	// TODO: Unknown code

	// Secondary scene load routine
	if (_vm->getGameType() == GType_RexNebular)
		// Secondary scene load routine
		loadScene2("*I0.AA", sceneNumber);

	_madsVm->_player.loadSprites(NULL);

	switch (_madsVm->globals()->_config.screenFades) {
	case 0:
		_abortTimers2 = 2;
		break;
	case 2:
		_abortTimers2 = 21;
		break;
	default:
		_abortTimers2 = 20;
		break;
	}
	_abortTimers = 0;
	_abortTimersMode2 = ABORTMODE_1;


	// Do any scene specific setup
	if (_vm->getGameType() == GType_RexNebular)
		_sceneLogic.doEnterScene();

	// Miscellaneous player setup
	_madsVm->_player._destPos = _madsVm->_player._destPos;
	_madsVm->_player._newDirection = _madsVm->_player._direction;
	_madsVm->_player.setupFrame();
	_madsVm->_player.updateFrame();

	// Purge resources
	_vm->res()->purge();
}

void MadsScene::loadSceneHotspots(int sceneNumber) {
	char filename[kM4MaxFilenameSize];
	sprintf(filename, "rm%i.hh", sceneNumber);
	MadsPack hotSpotData(filename, _vm);
	Common::SeekableReadStream *hotspotStream = hotSpotData.getItemStream(0);

	int hotspotCount = hotspotStream->readUint16LE();
	delete hotspotStream;

	hotspotStream = hotSpotData.getItemStream(1);

	// Clear current hotspot lists
	_sceneResources.hotspots->clear();
	_sceneResources.hotspots->loadHotSpots(hotspotStream, hotspotCount);

	delete hotspotStream;
}

void MadsScene::leaveScene() {
	_sceneResources.hotspots->clear();
	_sceneResources.dynamicHotspots->clear();

	delete _sceneResources.hotspots;
	delete _sceneResources.dynamicHotspots;
	delete _walkSurface;

	if (_activeAnimation) {
		delete _activeAnimation;
		_activeAnimation = NULL;
	}

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
	}
}

void MadsScene::mouseMove(int x, int y) {
	HotSpot *currentHotSpot = _sceneResources.hotspots->findByXY(x, y);
	if (currentHotSpot != NULL) {
		_vm->_mouse->setCursorNum(currentHotSpot->getCursor());

		_action._selectedRow = -1;
		_action._actionMode = ACTMODE_NONE;
		_action._actionMode2 = ACTMODE2_4;
		_action._hotspotId = currentHotSpot->getIndex();

	} else {
		_vm->_mouse->setCursorNum(0);
	}
}

void MadsScene::leftClick(int x, int y) {
	// TODO: figure out the rest of Scene_leftClick, and implements relevant parts in the interface class
	_action._v86F4C = -1;
	_action._v86F4E = 0;
	_customDest = _madsVm->_mouse->currentPos();
	_action._selectedAction = -1;
	_action._v86F4A = true;
}

void MadsScene::rightClick(int x, int y) {
	if (_vm->getGameType() == GType_RexNebular) {
		// ***DEBUG*** - sample dialog display
		int idx = 3; //_madsVm->_globals->messageIndexOf(0x277a);
		const char *msg = _madsVm->globals()->loadMessage(idx);
		Dialog *dlg = new Dialog(_vm, msg, "TEST DIALOG");
		_vm->_viewManager->addView(dlg);
		_vm->_viewManager->moveToFront(dlg);
	}
}

void MadsScene::setAction(int action, int objectId) {
	VALIDATE_MADS;

	error("todo");
	// TODO: Actually executing actions directly for objects. Also, some object actions are special in that
	// a second object can be selected, as in 'use gun to shoot person', with requires a target
/*
	// Set up the new action
	strcpy(statusText, _madsVm->globals()->getVocab(action));
	statusText[0] = toupper(statusText[0]);	// capitalize first letter

	if (objectId != -1) {
		MadsObject *obj = _madsVm->globals()->getObject(objectId);
		sprintf(statusText + strlen(statusText), " %s", _madsVm->globals()->getVocab(obj->descId));
	} else {
		_currentAction = action;
	}
*/
//	setStatusText(statusText);
}

/**
 * Draws all the elements of the scene
 */
void MadsScene::drawElements() {
	refresh();

	// Copy the user interface surface onto the surface
	_interfaceSurface->copyTo(this, 0, this->height() - _interfaceSurface->height());
}


void MadsScene::update() {
	// Draw all the various elements
	drawElements();

	_action.set();
}

void MadsScene::updateState() {
	if (!_abortTimers && !_madsVm->_player._unk3) {
		if (_dynamicHotspots._changed)
			_dynamicHotspots.refresh();

//		int v = (_madsVm->_player._stepEnabled && !_action._startWalkFlag && !_abortTimers2) ? 1 : 0;
//		_screenObjects.check(v, false);
	}

	// Handle starting off any selected action
	bool doPreAction = false;
	if ((_action._selectedAction != 0) && _madsVm->_player._stepEnabled &&
			!_action._startWalkFlag && !_abortTimers && !_madsVm->_player._unk3) {
		// Start the action
		_action.startAction();

		if (_action._action.verbId == kVerbLookAt) {
			_action._action.verbId = kVerbLook;
			_action._savedFields.selectedRow = 0;
		}
		doPreAction = true;
	}
	if (doPreAction || ((_abortTimers != 0) && (_abortTimersMode == ABORTMODE_2)))
		doPreactions();

	checkStartWalk();

	if (_action._inProgress && !_madsVm->_player._moving && !_action._startWalkFlag &&
		(_madsVm->_player._newDirection == _madsVm->_player._direction)) {
		// Reached the end of action movement, so ready to actually do action
			doAction();
	} else if ((_abortTimers != 0) && (_abortTimersMode == ABORTMODE_0))
		// Do an action designated by scripts
		doAction();

	bool freeFlag = false;
	if (_currentScene != _nextScene)
		freeFlag = true;
	else {
		doSceneStep();

		if (_currentScene != _nextScene)
			freeFlag = true;
		else {
			// Update the player
			_madsVm->_player.nextFrame();

			// Handle updating the animation
			if (!_abortTimers && (_activeAnimation))
				_activeAnimation->update();

			// Handle refreshing the mouse position display
			if (_mouseMsgIndex != -1)
				_madsVm->scene()->_kernelMessages.remove(_mouseMsgIndex);
			if (_showMousePos) {
				char buffer[20];
				sprintf(buffer, "(%d,%d)", _madsVm->_mouse->currentPos().x, _madsVm->_mouse->currentPos().y);

				_mouseMsgIndex = _madsVm->scene()->_kernelMessages.add(Common::Point(5, 5), 0x203, 0, 0, 1, buffer);
			}
		}
	}

	if (_madsVm->globals()->_config.easyMouse)
		_action.refresh();

	if ((_activeAnimation) && !_abortTimers) {
		_activeAnimation->update();
		if (((MadsAnimation *) _activeAnimation)->freeFlag() || freeFlag) {
			delete _activeAnimation;
			_activeAnimation = NULL;
		}
	}

	MadsView::update();

	// Remove the animation if it's been completed
	if ((_activeAnimation) && ((MadsAnimation *)_activeAnimation)->freeFlag())
		freeAnimation();

	if ((_action._selectedAction != 0) || !_madsVm->_player._stepEnabled) {
		_action.clear();
		_action._selectedAction = 0;
	}
}

void MadsScene::checkStartWalk() {
	if (_action._startWalkFlag && _action._walkFlag) {
		_madsVm->_player.setDest(_destPos.x, _destPos.y, _destFacing);
		_action._startWalkFlag = false;
	}
}

void MadsScene::doPreactions() {
	if ((_screenObjects._v832EC == 0) || (_screenObjects._v832EC == 2)) {
		_abortTimersMode2 = ABORTMODE_2;
		_action.checkAction();

		_sceneLogic.doPreactions();

		if (_abortTimersMode == ABORTMODE_2)
			_abortTimers = 0;
	}
}

void MadsScene::doSceneStep() {
	// Step through the scene
	_sceneLogic.doSceneStep();

	_madsVm->_player.step();
	_madsVm->_player._unk3 = 0;

	if (_abortTimersMode == ABORTMODE_1)
		_abortTimers = 0;
}

void MadsScene::doAction() {
	AbortTimerMode mode = ABORTMODE_0;
	_abortTimersMode2 = mode;

	if ((_action._inProgress || (_abortTimers != 0)) && !_action._v8453A) {
		_sceneLogic.doAction();
		mode = _action._inProgress ? ABORTMODE_0 : ABORTMODE_1;
	}

	if (_screenObjects._v832EC)
		_action._inProgress = false;
	else {
		if ((_action._inProgress || (_abortTimers != 0)) && (mode == ABORTMODE_0) && (_action._v8453A == mode)) {
			// TODO: sound_fn_p();
			mode = _action._inProgress ? ABORTMODE_0 : ABORTMODE_1;

		}

		if ((_action._inProgress || (_abortTimers != 0)) && (mode == ABORTMODE_0) && (_action._v8453A == mode)) {
			// Perform a core scene-indepedant action on an object
			// object_do_action
		}
	}

}


/**
 * Does extra work at cleaning up the animation, and then deletes it
 */
void MadsScene::freeAnimation() {
	if (!_activeAnimation)
		return;

	MadsAnimation *anim = (MadsAnimation *)_activeAnimation;
	if (anim->freeFlag()) {
		_madsVm->scene()->_spriteSlots.clear();
		_madsVm->scene()->_spriteSlots.fullRefresh();
		_madsVm->scene()->_sequenceList.scan();
	}

	if (_madsVm->_player._visible) {
		_madsVm->_player._forceRefresh = true;
		_madsVm->_player.update();
	}

	delete _activeAnimation;
	_activeAnimation = NULL;
}


int MadsScene::loadSceneSpriteSet(const char *setName) {
	char resName[100];
	strcpy(resName, setName);

	// Append a '.SS' if it doesn't alreayd have an extension
	if (!strchr(resName, '.'))
		strcat(resName, ".SS");

	return _spriteSlots.addSprites(resName);
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
void MadsScene::showMADSV2TextBox(char *text, int x, int y, char *faceName) {
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

void MadsScene::loadAnimation(const Common::String &animName, int abortTimers) {
	if (_activeAnimation)
		error("Multiple active animations are not allowed");

	MadsAnimation *anim = new MadsAnimation(_vm, this);
	anim->load(animName.c_str(), abortTimers);
	_activeAnimation = anim;
}

bool MadsScene::getDepthHighBit(const Common::Point &pt) {
	const byte *p = _depthSurface->getBasePtr(pt.x, pt.y);
	if (_sceneResources._depthStyle == 2)
		return ((*p << 4) & 0x80) != 0;

	return (*p & 0x80) != 0;
}

bool MadsScene::getDepthHighBits(const Common::Point &pt) {
	if (_sceneResources._depthStyle == 2)
		return 0;

	const byte *p = _depthSurface->getBasePtr(pt.x, pt.y);
	return (*p & 0x70) >> 4;
}

/*--------------------------------------------------------------------------*/

void MadsSceneResources::load(int sceneNumber, const char *resName, int v0, M4Surface *depthSurface, M4Surface *surface) {
	char buffer1[80];
	const char *sceneName;

	// TODO: Initialize spriteSet / xp_list

	if (sceneNumber > 0) {
		sceneName = MADSResourceManager::getResourceName(RESPREFIX_RM, sceneNumber, ".DAT");
	} else {
		strcpy(buffer1, "*");
		strcat(buffer1, resName);
		sceneName = buffer1; // TODO: Check whether this needs to be converted to 'HAG form'
	}

	Common::SeekableReadStream *rawStream = _vm->_resourceManager->get(sceneName);
	MadsPack sceneInfo(rawStream);

	// Chunk 0:
	// Basic scene info
	Common::SeekableReadStream *stream = sceneInfo.getItemStream(0);

	if (_vm->getGameType() == GType_RexNebular) {
		int resSceneId = stream->readUint16LE();
		assert(resSceneId == sceneNumber);
	} else {
		char roomFilename[10];
		char roomFilenameExpected[10];
		sprintf(roomFilenameExpected, "*RM%d", sceneNumber);

		stream->read(roomFilename, 6);
		roomFilename[6] = 0;
		assert(!strcmp(roomFilename, roomFilenameExpected));
	}

	// TODO: The following is wrong for Phantom/Dragon
	_artFileNum = stream->readUint16LE();
	_depthStyle = stream->readUint16LE();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();

	stream->skip(24);

	int nodeCount = stream->readUint16LE();
	_yBandsEnd = stream->readUint16LE();
	_yBandsStart = stream->readUint16LE();
	_maxScale = stream->readSint16LE();
	_minScale = stream->readSint16LE();
	for (int i = 0; i < DEPTH_BANDS_SIZE; ++i)
		_depthBands[i] = stream->readUint16LE();
	stream->skip(2);

	// Load in any scene objects
	for (int i = 0; i < nodeCount; ++i) {
		SceneNode rec;
		rec.load(stream);
		_nodes.push_back(rec);
	}
	for (int i = 0; i < 20 - nodeCount; ++i)
		stream->skip(48);

	// Add two extra nodes in that will be used for player movement
	for (int i = 0; i < 2; ++i) {
		SceneNode rec;
		_nodes.push_back(rec);
	}

	int setCount = stream->readUint16LE();
	stream->readUint16LE();
	for (int i = 0; i < setCount; ++i) {
		char buffer2[64];
		Common::String s(buffer2, 64);
		_setNames.push_back(s);
	}

	delete stream;

	// Initialize a copy of the surfaces if they weren't provided
	bool dsFlag = false, ssFlag = false;
	if (!surface) {
		surface = new M4Surface(_width, _height);
		ssFlag = true;
	} else if ((_width != surface->width()) || (_height != surface->height()))
		surface->setSize(_width, _height);

	if (!depthSurface) {
		depthSurface = new M4Surface(_width, _height);
		dsFlag = true;
	} else if ((_width != depthSurface->width()) || (_height != depthSurface->height()))
		depthSurface->setSize(_width, _height);


	// For Rex Nebular, read in the scene's compressed walk surface information
	if (_vm->getGameType() == GType_RexNebular) {
		assert(depthSurface);
		stream = sceneInfo.getItemStream(1);
		byte *walkData = (byte *)malloc(stream->size());
		stream->read(walkData, stream->size());

		// For Rex Nebular, the walk areas are part of the scene info
		byte *destP = depthSurface->getBasePtr(0, 0);
		const byte *srcP = walkData;
		byte runLength;

		// Run length encoded depth data
		while ((runLength = *srcP++) != 0) {
			if (_depthStyle == 2) {
				// 2-bit depth pixels
				byte byteVal = *srcP++;
				for (int byteCtr = 0; byteCtr < runLength; ++byteCtr) {
					byte v = byteVal;
					for (int bitCtr = 0; bitCtr < 4; ++bitCtr, v >>= 2)
						*destP++ = (((v & 1) + 1) << 3) - 1;
				}
			} else {
				// 8-bit depth pixels
				Common::set_to(destP, destP + runLength, *srcP++);
				destP += runLength;
			}
		}

		free(walkData);
		delete stream;
	}

	_vm->_resourceManager->toss(sceneName);

	// Load the surface artwork
	surface->loadBackground(_artFileNum);

	// Final cleanup
	if (ssFlag)
		delete surface;
	if (dsFlag)
		delete depthSurface;
}

void MadsSceneResources::setRouteNode(int nodeIndex, const Common::Point &pt, M4Surface *depthSurface) {
	int flags, hypotenuse;

	_nodes[nodeIndex].pt = pt;

	// Recalculate inter-node lengths
	for (uint idx = 0; idx < _nodes.size(); ++idx) {
		int entry;
		if (idx == (uint)nodeIndex) {
			entry = 0x3FFF;
		} else {
			// Process the node
			flags = getRouteFlags(pt, _nodes[idx].pt, depthSurface);

			int xDiff = ABS(_nodes[idx].pt.x - pt.x);
			int yDiff = ABS(_nodes[idx].pt.y - pt.y);
			hypotenuse = SqrtF16(xDiff * xDiff + yDiff * yDiff);

			if (hypotenuse >= 0x3FFF)
				// Shouldn't ever be this large
				hypotenuse = 0x3FFF;

			entry = hypotenuse | flags;
			_nodes[idx].indexes[nodeIndex] = entry;
			_nodes[nodeIndex].indexes[idx] = entry;
		}
	}
}

int MadsSceneResources::getRouteFlags(const Common::Point &src, const Common::Point &dest, M4Surface *depthSurface) {
	int result = 0x8000;
	bool flag = false;

	int xDiff = ABS(dest.x - src.x);
	int yDiff = ABS(dest.y - src.y);
	int xDirection = dest.x >= src.x ? 1 : -1;
	int yDirection = dest.y >= src.y ? depthSurface->width() : -depthSurface->width();
	int majorDiff = 0;
	if (dest.x < src.x)
		majorDiff = MAX(xDiff, yDiff);
	++xDiff;
	++yDiff;

	byte *srcP = depthSurface->getBasePtr(src.x, src.y);

	int totalCtr = majorDiff;
	for (int xCtr = 0; xCtr < xDiff; ++xCtr, srcP += xDirection) {
		totalCtr += yDiff;

		if ((*srcP & 0x80) == 0)
			flag = false;
		else if (!flag) {
			flag = true;
			result -= 0x4000;
			if (result == 0)
				break;
		}

		while (totalCtr >= xDiff) {
			totalCtr -= xDiff;

			if ((*srcP & 0x80) == 0)
				flag = false;
			else if (!flag) {
				flag = true;
				result -= 0x4000;
				if (result == 0)
					break;
			}

			srcP += yDirection;
		}
		if (result == 0)
			break;
	}

	return result;
}

/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 * MadsInterfaceView handles the user interface section at the bottom of
 * game screens in MADS games
 *--------------------------------------------------------------------------
 */

MadsInterfaceView::MadsInterfaceView(MadsM4Engine *vm): GameInterfaceView(vm,
		Common::Rect(0, MADS_SURFACE_HEIGHT, vm->_screen->width(), vm->_screen->height())) {
	_screenType = VIEWID_INTERFACE;
	_highlightedElement = -1;
	_topIndex = 0;
	_selectedObject = -1;
	_cheatKeyCtr = 0;

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
		_vm->_font->current()->setColors(4, 4, 0xff);
		break;
	case ITEM_HIGHLIGHTED:
		_vm->_font->current()->setColors(5, 5, 0xff);
		break;
	case ITEM_SELECTED:
		_vm->_font->current()->setColors(6, 6, 0xff);
		break;
	}
}

void MadsInterfaceView::initialize() {
	// Build up the inventory list
	_inventoryList.clear();

	for (uint i = 0; i < _madsVm->globals()->getObjectsSize(); ++i) {
		MadsObject *obj = _madsVm->globals()->getObject(i);
		if (obj->_roomNumber == PLAYER_INVENTORY)
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
		_madsVm->globals()->getObject(objectNumber)->_roomNumber = PLAYER_INVENTORY;
		_inventoryList.push_back(objectNumber);
	}

	setSelectedObject(objectNumber);
}

void MadsInterfaceView::onRefresh(RectList *rects, M4Surface *destSurface) {
	_vm->_font->setFont(FONT_INTERFACE_MADS);
	char buffer[100];

	// Check to see if any dialog is currently active
	bool dialogVisible = _vm->_viewManager->getView(LAYER_DIALOG) != NULL;

	// Highlighting logic for action list
	int actionIndex = 0;
	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 5; ++y, ++actionIndex) {
			// Determine the font color depending on whether an item is selected. Note that the first action,
			// 'Look', is always 'selected', even when another action is clicked on
			setFontMode((_highlightedElement == actionIndex) ? ITEM_HIGHLIGHTED :
				((actionIndex == 0) ? ITEM_SELECTED : ITEM_NORMAL));

			// Get the verb action and capitalise it
			const char *verbStr = _madsVm->globals()->getVocab(kVerbLook + actionIndex);
			strcpy(buffer, verbStr);
			if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

			// Display the verb
			const Common::Rect r(_screenObjects[actionIndex]);
			_vm->_font->current()->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
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

		const char *descStr = _madsVm->globals()->getVocab(_madsVm->globals()->getObject(
			_inventoryList[_topIndex + i])->_descId);
		strcpy(buffer, descStr);
		if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

		const Common::Rect r(_screenObjects[INVLIST_START + i]);

		// Set the highlighting of the inventory item
		if (_highlightedElement == (int)(INVLIST_START + i)) setFontMode(ITEM_HIGHLIGHTED);
		else if (_selectedObject == _inventoryList[_topIndex + i]) setFontMode(ITEM_SELECTED);
		else setFontMode(ITEM_NORMAL);

		// Write out it's description
		_vm->_font->current()->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
	}

	// Handle the display of any currently selected object
	if (_objectSprites) {
		// Display object sprite. Note that the frame number isn't used directly, because it would result
		// in too fast an animation
		M4Sprite *spr = _objectSprites->getFrame(_objectFrameNumber / INV_ANIM_FRAME_SPEED);
		spr->copyTo(destSurface, INVENTORY_X, INVENTORY_Y, TRANSPARENT_COLOR_INDEX);

		if (!_madsVm->globals()->_config.invObjectsStill && !dialogVisible) {
			// If objects need to be animated, move to the next frame
			if (++_objectFrameNumber >= (_objectSprites->getCount() * INV_ANIM_FRAME_SPEED))
				_objectFrameNumber = 0;
		}

		// List the vocab actions for the currently selected object
		MadsObject *obj = _madsVm->globals()->getObject(_selectedObject);
		int yIndex = MIN(_highlightedElement - VOCAB_START, obj->_vocabCount - 1);

		for (int i = 0; i < obj->_vocabCount; ++i) {
			const Common::Rect r(_screenObjects[VOCAB_START + i]);

			// Get the vocab description and capitalise it
			const char *descStr = _madsVm->globals()->getVocab(obj->_vocabList[i].vocabId);
			strcpy(buffer, descStr);
			if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

			// Set the highlighting and display the entry
			setFontMode((i == yIndex) ? ITEM_HIGHLIGHTED : ITEM_NORMAL);
			_vm->_font->current()->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
		}
	}
}

bool MadsInterfaceView::onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents) {
	MadsAction &act = _madsVm->scene()->_action;

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
		{
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
				int verbId = kVerbLook + (_highlightedElement - ACTIONS_START);
				warning("Selected action #%d", verbId);

			} else if ((_highlightedElement >= VOCAB_START) && (_highlightedElement < (VOCAB_START + 5))) {
				// A vocab action was selected
				MadsObject *obj = _madsVm->globals()->getObject(_selectedObject);
				int vocabIndex = MIN(_highlightedElement - VOCAB_START, obj->_vocabCount - 1);
				if (vocabIndex >= 0) {
					act._actionMode = ACTMODE_OBJECT;
					act._actionMode2 = ACTMODE2_2;
					act._flags1 = obj->_vocabList[1].flags1;
					act._flags2 = obj->_vocabList[1].flags2;

					act._action.verbId = _selectedObject;
					act._articleNumber = act._flags2;
				}
			}
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

	case KEVENT_KEY:
		if (_cheatKeyCtr == CHEAT_SEQUENCE_MAX)
			handleCheatKey(param1);
		handleKeypress(param1);
		return true;

	default:
		break;
	}

	return false;
}

bool MadsInterfaceView::handleCheatKey(int32 keycode) {
	switch (keycode) {
	case Common::KEYCODE_SPACE:
		// TODO: Move player to current destination
		return true;

	case Common::KEYCODE_c | (Common::KBD_CTRL << 24):
		// Toggle display of mouse position
		_madsVm->scene()->_showMousePos = !_madsVm->scene()->_showMousePos;
		break;

	case Common::KEYCODE_t | (Common::KBD_ALT << 24): {
		// Teleport to room
		//Scene *sceneView = (Scene *)vm->_viewManager->getView(VIEWID_SCENE);
		// TODO: Implement teleport code

		return true;
	}

	default:
		break;
	}

	return false;
}

const char *CHEAT_SEQUENCE = "widepipe";

bool MadsInterfaceView::handleKeypress(int32 keycode) {
	int flags = keycode >> 24;
	int kc = keycode & 0xffff;

	// Capitalise the letter if necessary
	if (_cheatKeyCtr < CHEAT_SEQUENCE_MAX) {
		if ((flags & Common::KBD_CTRL) && (kc == CHEAT_SEQUENCE[_cheatKeyCtr])) {
			++_cheatKeyCtr;
			if (_cheatKeyCtr == CHEAT_SEQUENCE_MAX)
				Dialog::display(_vm, 22, cheatingEnabledDesc);
			return true;
		} else {
			_cheatKeyCtr = 0;
		}
	}

	// Handle the various keys
	if ((keycode == Common::KEYCODE_ESCAPE) || (keycode == Common::KEYCODE_F1)) {
		// Game menu
		_madsVm->globals()->dialogType = DIALOG_GAME_MENU;
		leaveScene();
		return false;
	} else if (flags & Common::KBD_CTRL) {
		// Handling of the different control key combinations
		switch (kc) {
		case Common::KEYCODE_i:
			// Mouse to inventory
			warning("TODO: Mouse to inventory");
			break;

		case Common::KEYCODE_k:
			// Toggle hotspots
			warning("TODO: Toggle hotspots");
			break;

		case Common::KEYCODE_p:
			// Player stats
			warning("TODO: Player stats");
			break;

		case Common::KEYCODE_q:
			// Quit game
			break;

		case Common::KEYCODE_s:
			// Activate sound
			warning("TODO: Activate sound");
			break;

		case Common::KEYCODE_t:
			// Rotate player - This was Ctrl-U in the original, but in ScummVM Ctrl-U is a global mute key
			_madsVm->_player._newDirection = _madsVm->_player._directionListIndexes[_madsVm->_player._newDirection + 10];
			break;

		case Common::KEYCODE_v: {
			// Release version
			Dialog *dlg = new Dialog(_vm, GameReleaseInfoStr, GameReleaseTitleStr);
			_vm->_viewManager->addView(dlg);
			_vm->_viewManager->moveToFront(dlg);
			return false;
		}

		default:
			break;
		}
	} else if ((flags & Common::KBD_ALT) && (kc == Common::KEYCODE_q)) {
		// Quit Game

	} else {
		// Standard keypresses
		switch (kc) {
			case Common::KEYCODE_F2:
				// Save game
				_madsVm->globals()->dialogType = DIALOG_SAVE;
				leaveScene();
				break;
			case Common::KEYCODE_F3:
				// Restore game
				_madsVm->globals()->dialogType = DIALOG_RESTORE;
				leaveScene();
				break;
		}
	}
//DIALOG_OPTIONS
	return false;
}

void MadsInterfaceView::leaveScene() {
	// Close the scene
	View *view = _madsVm->_viewManager->getView(VIEWID_SCENE);
	_madsVm->_viewManager->deleteView(view);
}

//--------------------------------------------------------------------------

int getActiveAnimationBool() {
	return (_madsVm->scene()->activeAnimation()) ? 1 : 0;
}

int getAnimationCurrentFrame() {
	Animation *anim = _madsVm->scene()->activeAnimation();
	return anim ? anim->getCurrentFrame() : 0;
}


} // End of namespace M4
