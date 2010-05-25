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

namespace M4 {

MadsScene::MadsScene(MadsEngine *vm): _sceneResources(), Scene(vm, &_sceneResources), MadsView(this) {
	_vm = vm;

	MadsView::_bgSurface = Scene::_backgroundSurface;
	MadsView::_depthSurface = Scene::_walkSurface;
	_interfaceSurface = new MadsInterfaceView(vm);
	for (int i = 0; i < 3; ++i)
		actionNouns[i] = 0;
}

MadsScene::~MadsScene() {
	leaveScene();
	_vm->_viewManager->deleteView(_interfaceSurface);
}

/**
 * Secondary scene loading code
 */
void MadsScene::loadScene2(const char *aaName) {
	// TODO: Completely finish

	_spriteSlots.clear();
	_sequenceList.clear();
	_kernelMessages.clear();

	// Load up the properties for the scene
	_sceneResources.load(_currentScene);

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
		_interfaceSurface->madsloadInterface(0, &_interfacePal);
		_vm->_palette->addRange(_interfacePal);
		_interfaceSurface->translate(_interfacePal);
		_backgroundSurface->copyFrom(_interfaceSurface, Common::Rect(0, 0, 320, 44), 0, 200 - 44);

		_interfaceSurface->initialise();
	}

	// Don't load other screen resources for system screens
	if (_currentScene >= 900)
		return;

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

	_madsVm->globals()->previousScene = _madsVm->globals()->sceneNumber;
	_madsVm->globals()->sceneNumber = sceneNumber;

	// Existing ScummVM code that needs to be eventually replaced with MADS code
	loadSceneTemporary();

	// Signal the script engine what scene is to be active
	_sceneLogic.selectScene(sceneNumber);
	_sceneLogic.setupScene();

	// Add the scene if necessary to the list of scenes that have been visited
	_vm->globals()->addVisitedScene(sceneNumber);

	// Secondary scene load routine
	loadScene2("*I0.AA");

	// Do any scene specific setup
	_sceneLogic.enterScene();

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

	_sceneResources.hotspotCount = hotspotCount;

	hotspotStream = hotSpotData.getItemStream(1);

	// Clear current hotspot lists
	_sceneResources.hotspots->clear();

	_sceneResources.hotspots->loadHotSpots(hotspotStream, _sceneResources.hotspotCount);

	delete hotspotStream;
}

void MadsScene::leaveScene() {
	_sceneResources.hotspots->clear();
	_sceneResources.props->clear();

	delete _sceneResources.hotspots;
	delete _sceneResources.props;
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
		const byte *srcP = _sceneResources.walkData;
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
		int verbId = 0;//***DEBUG****_currentAction;
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
	setStatusText(statusText);
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
	const char *sStatusText = _action.statusText();

	// Handle display of any status text
	if (sStatusText[0]) {
		// Text colors are inverted in Dragonsphere
		if (_vm->getGameType() == GType_DragonSphere)
			_vm->_font->current()->setColours(_vm->_palette->BLACK, _vm->_palette->WHITE, _vm->_palette->BLACK);
		else
			_vm->_font->current()->setColours(_vm->_palette->WHITE, _vm->_palette->BLACK, _vm->_palette->BLACK);

		_vm->_font->setFont(FONT_MAIN_MADS);
		_vm->_font->current()->writeString(this, sStatusText, (width() - _vm->_font->current()->getWidth(sStatusText)) / 2, 142, 0);
	}

	//***DEBUG***
	_spriteSlots.getSprite(0).getFrame(1)->copyTo(this, 120, 90, 0);
}

void MadsScene::updateState() {
	_sceneLogic.sceneStep();
	_sequenceList.tick();
	_kernelMessages.update();
}

int MadsScene::loadSceneSpriteSet(const char *setName) {
	char resName[100];
	strcpy(resName, setName);

	// Append a '.SS' if it doesn't alreayd have an extension
	if (!strchr(resName, '.'))
		strcat(resName, ".SS");

	return _spriteSlots.addSprites(resName);
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

/*--------------------------------------------------------------------------*/

MadsAction::MadsAction() {
	clear();
}

void MadsAction::clear() {
	_actionMode = ACTMODE_NONE;
	_actionMode2 = ACTMODE2_0;
	_word_86F42 = 0;
	_word_86F4E = 0;
	_articleNumber = 0;
	_lookFlag = false;
	_word_86F4A = 0;
	_statusText[0] = '\0';
	_selectedRow = -1;
	_currentHotspot = -1;
	_word_86F3A = -1;
	_word_86F4C = -1;
	//word_86F3A/word_86F4C
	_currentAction = kVerbNone;
	_objectNameId = -1;
	_objectDescId = -1;
	_word_83334 = -1;
}

void MadsAction::appendVocab(int vocabId, bool capitalise) {
	char *s = _statusText + strlen(_statusText);
	const char *vocabStr = _madsVm->globals()->getVocab(vocabId);
	strcpy(s, vocabStr);
	if (capitalise)
		*s = toupper(*s);

	strcat(s, " ");
}

void MadsAction::set() {
	int hotspotCount = _madsVm->scene()->getSceneResources().hotspotCount;
	bool flag = false;
	_currentAction = -1;
	_objectNameId = -1;
	_objectDescId = -1;

	if (_actionMode == ACTMODE_TALK) {
		// Handle showing the conversation selection. Rex at least doesn't actually seem to use this
		if (_selectedRow >= 0) {
			const char *desc = _madsVm->_converse[_selectedRow].desc;
			if (desc)
				strcpy(_statusText, desc);
		}
	} else if (_lookFlag && (_selectedRow == 0)) {
		// Two 'look' actions in succession, so the action becomes 'Look around'
		strcpy(_statusText, lookAroundStr);
	} else {
		if ((_actionMode == ACTMODE_OBJECT) && (_selectedRow >= 0) && (_flags1 == 2) && (_flags2 == 0)) {
			// Use/to action
			int selectedObject = _madsVm->scene()->getInterface()->getSelectedObject();
			MadsObject *objEntry = _madsVm->globals()->getObject(selectedObject);
			
			_objectNameId = objEntry->descId;
			_currentAction = objEntry->vocabList[_selectedRow].vocabId;

			// Set up the status text stirng
			strcpy(_statusText, useStr);
			appendVocab(_objectNameId);
			strcpy(_statusText, toStr);
			appendVocab(_currentAction);
		} else {
			// Handling for if an action has been selected
			if (_selectedRow >= 0) {
				if (_actionMode == ACTMODE_VERB) {
					// Standard verb action
					_currentAction = verbList[_selectedRow].verb;
				} else {
					// Selected action on an inventory object
					int selectedObject = _madsVm->scene()->getInterface()->getSelectedObject();
					MadsObject *objEntry = _madsVm->globals()->getObject(selectedObject);

					_currentAction = objEntry->vocabList[_selectedRow].vocabId;
				}

				appendVocab(_currentAction, true);

				if (_currentAction == kVerbLook) {
					// Add in the word 'add'
					strcat(_statusText, atStr);
					strcat(_statusText, " ");
				}
			}

			// Handling for if a hotspot has been selected/highlighted
			if ((_currentHotspot >= 0) && (_selectedRow >= 0) && (_articleNumber > 0) && (_flags1 == 2)) {
				flag = true;

				strcat(_statusText, englishMADSArticleList[_articleNumber]);
				strcat(_statusText, " ");
			}

			if (_currentHotspot >= 0) {
				if (_selectedRow < 0) {
					int verbId;

					if (_currentHotspot < hotspotCount) {
						// Get the verb Id from the hotspot
						verbId = (*_madsVm->scene()->getSceneResources().hotspots)[_currentHotspot].getVerbID();
					} else {
						// Get the verb Id from the scene object
						verbId = (*_madsVm->scene()->getSceneResources().props)[_currentHotspot - hotspotCount].getVerbID();
					}

					if (verbId > 0) {
						// Set the specified action
						_currentAction = verbId;
						appendVocab(_currentAction, true);
					} else {
						// Default to a standard 'walk to'
						_currentAction = kVerbWalkTo;
						strcat(_statusText, walkToStr);
					}
				}

				if ((_actionMode2 == ACTMODE2_2) || (_actionMode2 == ACTMODE2_5)) {
					// Get name from given inventory object
					int objectId = _madsVm->scene()->getInterface()->getInventoryObject(_currentHotspot);
					_objectNameId = _madsVm->globals()->getObject(objectId)->descId;
				} else if (_currentHotspot < hotspotCount) {
					// Get name from scene hotspot
					_objectNameId = (*_madsVm->scene()->getSceneResources().hotspots)[_currentHotspot].getVocabID();
				} else {
					// Get name from temporary scene hotspot
					_objectNameId = (*_madsVm->scene()->getSceneResources().props)[_currentHotspot].getVocabID();
				}
			}
		}

		if ((_currentHotspot >= 0) && (_articleNumber > 0) && !flag) {
			if (_articleNumber == -1) {
				if (_word_86F3A >= 0) {
					int articleNum = 0;

					if ((_word_86F42 == 2) || (_word_86F42 == 5)) {
						int objectId = _madsVm->scene()->getInterface()->getInventoryObject(_currentHotspot);
						articleNum = _madsVm->globals()->getObject(objectId)->article;
					} else if (_word_86F3A < hotspotCount) {
						articleNum = (*_madsVm->scene()->getSceneResources().hotspots)[_currentHotspot].getArticle();
					} else {

					}
				}

			} else if ((_articleNumber == kVerbLook) || (_vm->getGameType() != GType_RexNebular) ||
				(strcmp(_madsVm->globals()->getVocab(_objectDescId), fenceStr) != 0)) {
				// Write out the article
				strcat(_statusText, englishMADSArticleList[_articleNumber]);
			} else {
				// Special case for a 'fence' entry in Rex Nebular
				strcat(_statusText, overStr);
			}

			strcat(_statusText, " ");
		}

		// Append object description if necessary
		if (_word_86F3A >= 0)
			appendVocab(_objectDescId);

		// Remove any trailing space character
		int statusLen = strlen(_statusText);
		if ((statusLen > 0) && (_statusText[statusLen - 1] == ' '))
			_statusText[statusLen - 1] = '\0';
	}

	_word_83334 = -1;
}

/*--------------------------------------------------------------------------*/

void MadsSceneResources::load(int sId) {
	const char *sceneInfoStr = MADSResourceManager::getResourceName(RESPREFIX_RM, sId, ".DAT");
	Common::SeekableReadStream *rawStream = _vm->_resourceManager->get(sceneInfoStr);
	MadsPack sceneInfo(rawStream);

	// Basic scene info
	Common::SeekableReadStream *stream = sceneInfo.getItemStream(0);

	int resSceneId = stream->readUint16LE();
	assert(resSceneId == sId);

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

/*--------------------------------------------------------------------------*/

} // End of namespace M4
