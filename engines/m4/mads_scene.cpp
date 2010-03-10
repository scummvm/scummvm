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

MadsScene::MadsScene(MadsEngine *vm): _sceneResources(), Scene(vm, &_sceneResources) {
	_vm = vm;

	strcpy(_statusText, "");
	_interfaceSurface = new MadsInterfaceView(vm);
	_spriteSlotsStart = 0;
	for (int i = 0; i < 3; ++i)
		actionNouns[i] = 0;
}

/**
 * Secondary scene loading code
 */
void MadsScene::loadScene2(const char *aaName) {
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


	// Text display 
	_textDisplay.draw(this);

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

MadsAction::MadsAction() {
	clear();
}

void MadsAction::clear() {
	_actionMode = ACTMODE_NONE;
	_articleNumber = 0;
	_lookFlag = false;
	_statusText[0] = '\0';
	_selectedRow = -1;
	_currentHotspot = -1;
	//word_86F3A/word_86F4C
	_currentAction = kVerbNone;
	_objectNameId = -1;
	_objectDescId = -1;
	//word_83334
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
					int hotspotCount = _madsVm->scene()->getSceneResources().hotspotCount;

					if (_currentHotspot < hotspotCount) {
						// Get the verb Id from the hotspot
						verbId = 0;//selected hotspot
					} else {
						// Get the verb Id from the scene object
						verbId = 0;//Scene_object[_currentHotspot - _hotspotCount].verbId;
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

				//loc_21CE2
			}
		}
	}

	//word_83334 = -1;
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

MadsScreenText::MadsScreenText() {
	for (int i = 0; i < TEXT_DISPLAY_SIZE; ++i)
		_textDisplay[i].active = false;
	for (int i = 0; i < TIMED_TEXT_SIZE; ++i)
		_timedText[i].flags = 0;
	_abortTimedText = false;
}

/**
 * Adds the specified string to the list of on-screen display text
 */
int MadsScreenText::add(const Common::Point &destPos, uint fontColours, int widthAdjust, const char *msg, Font *font) {
	// Find a free slot
	int idx = 0;
	while ((idx < TEXT_DISPLAY_SIZE) && _textDisplay[idx].active)
		++idx;
	if (idx == TEXT_DISPLAY_SIZE)
		error("Ran out of text display slots");

	// Set up the entry values
	_textDisplay[idx].active = true;
	_textDisplay[idx].active2 = 1;
	_textDisplay[idx].bounds.left = destPos.x;
	_textDisplay[idx].bounds.top = destPos.y;
	_textDisplay[idx].bounds.setWidth(font->getWidth(msg, widthAdjust));
	_textDisplay[idx].bounds.setHeight(font->getHeight());
	_textDisplay[idx].font = font;
	strncpy(_textDisplay[idx].message, msg, 100);
	_textDisplay[idx].message[99] = '\0';
	_textDisplay[idx].colour1 = fontColours & 0xff;
	_textDisplay[idx].colour2 = fontColours >> 8;
	_textDisplay[idx].spacing = widthAdjust;

	return idx;
}

/**
 * Adds a new entry to the timed on-screen text display list
 */
int MadsScreenText::addTimed(const Common::Point &destPos, uint fontColours, uint flags, int vUnknown, uint32 timeout, const char *message) {
	// Find a free slot
	int idx = 0;
	while ((idx < TIMED_TEXT_SIZE) && ((_timedText[idx].flags & TEXTFLAG_ACTIVE) != 0))
		++idx;
	if (idx == TIMED_TEXT_SIZE) {
		if (vUnknown == 0)
			return -1;

		error("Ran out of timed text display slots");
	}

	// Set up the entry values
	_timedText[idx].flags = flags | TEXTFLAG_ACTIVE;
	strcpy(_timedText[idx].message, message);
	_timedText[idx].colour1 = fontColours & 0xff;
	_timedText[idx].colour2 = fontColours >> 8;
	_timedText[idx].position.x = destPos.x;
	_timedText[idx].position.y = destPos.y;
	_timedText[idx].textDisplayIndex = -1;
	_timedText[idx].timeout = timeout;
	_timedText[idx].frameTimer = g_system->getMillis();
	_timedText[idx].field_1C = vUnknown;
	_timedText[idx].field_1D = 0; /* word_84206 */

	// Copy the current action noun list
	for (int i = 0; i < 3; ++i)
		_timedText[idx].actionNouns[i] = _madsVm->scene()->actionNouns[i];

	if (flags & TEXTFLAG_2) {
		warning("word_844b8 and dword_845a0 not yet implemented");
	}

	return idx;
}

/**
 * Draws any text display entries to the screen
 */
void MadsScreenText::draw(M4Surface *surface) {
	for (int idx = 0; idx < TEXT_DISPLAY_SIZE; ++idx) {
		if (_textDisplay[idx].active && (_textDisplay[idx].active2 >= 0)) {
			_textDisplay[idx].font->setColours(_textDisplay[idx].colour1, 0xFF,
				(_textDisplay[idx].colour2 == 0) ? _textDisplay[idx].colour1 : _textDisplay[idx].colour2);
			_textDisplay[idx].font->writeString(surface, _textDisplay[idx].message, 
				_textDisplay[idx].bounds.left, _textDisplay[idx].bounds.top, _textDisplay[idx].bounds.width(),
				_textDisplay[idx].spacing);
		}
	}

	// Clear up any now inactive text display entries
	for (int idx = 0; idx < TEXT_DISPLAY_SIZE; ++idx) {
		if (_textDisplay[idx].active2 < 0) {
			_textDisplay[idx].active = false;
			_textDisplay[idx].active2 = 0;
		}
	}
}

void MadsScreenText::timedDisplay() {
	for (int idx = 0; !_abortTimedText && (idx < TEXT_DISPLAY_SIZE); ++idx) {
		if (((_timedText[idx].flags & TEXTFLAG_ACTIVE) != 0) && 
			(_timedText[idx].frameTimer <= g_system->getMillis()))
			// Add the specified entry
			addTimedText(&_timedText[idx]);
	}
}

void MadsScreenText::addTimedText(TimedText *entry) {
	if ((entry->flags & TEXTFLAG_40) != 0) {
		this->setActive2(entry->textDisplayIndex);
		entry->flags &= 0x7F;
		return;
	}

	if ((entry->flags & TEXTFLAG_8) == 0) 
		// FIXME: Adjust timeouts for ScumVM's milli counter
		entry->timeout -= 3;

	if ((entry->flags & TEXTFLAG_4) != 0) {
		Text4A &rec = _text4A[entry->unk4AIndex];
		if ((rec.field25 != 0) || (rec.active == 0))
			entry->timeout = 0;
	}

	if ((entry->timeout == 0) && !_abortTimedText) {
		entry->flags |= TEXTFLAG_40;

		if (entry->field_1C) {
			_abortTimedText = entry->field_1C;
			//word_84208 = entry->field_1D;
			
			if (entry->field_1D != 1) {
				// Restore the action list
				for (int i = 0; i < 3; ++i)
					_madsVm->scene()->actionNouns[i] = entry->actionNouns[i]; 
			}
		}
	}

	// TODO: code from 'loc_244ec' onwards
}

} // End of namespace M4
