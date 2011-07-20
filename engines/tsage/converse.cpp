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

#include "common/str-array.h"

#include "tsage/tsage.h"
#include "tsage/globals.h"
#include "tsage/staticres.h"

namespace tSage {

#define STRIP_WORD_DELAY 30


SequenceManager::SequenceManager() : Action() {
	Common::set_to(&_objectList[0], &_objectList[6], (SceneObject *)NULL);
	_sequenceData.clear();
	_field24 = 0;
	_sequenceOffset = 0;
	_resNum = 0;
	_field26 = 0;
	_objectIndex = 0;
	_keepActive = false;
	setup();
}

void SequenceManager::setup() {
	_sequenceOffset = 0;
	_objectIndex = 0;
	_sceneObject = _objectList[0];
}

void SequenceManager::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		Action::synchronize(s);

	s.syncAsSint32LE(_resNum);
	s.syncAsSint32LE(_sequenceOffset);
	s.syncAsByte(_keepActive);
	s.syncAsSint32LE(_field24);
	s.syncAsSint32LE(_field26);

	s.syncAsSint32LE(_objectIndex);
	SYNC_POINTER(_sceneObject);
	for (int i = 0; i < 6; ++i)
		SYNC_POINTER(_objectList[i]);

	int seqSize = _sequenceData.size();
	s.syncAsUint32LE(seqSize);
	if (s.isLoading())
		_sequenceData.resize(seqSize);
	if (seqSize > 0)
		s.syncBytes(&_sequenceData[0], seqSize);
}

void SequenceManager::remove() {
	if ((!_sequenceData.empty()) && !_keepActive) {
		_sequenceData.clear();
	}

	if (_globals->_sceneObjects->contains(&_sceneText))
		_sceneText.remove();

	Common::set_to(&_objectList[0], &_objectList[6], (SceneObject *)NULL);
	Action::remove();
}

void SequenceManager::signal() {
	if (_globals->_sceneObjects->contains(&_sceneText))
		_sceneText.hide();

	bool continueFlag = true;
	while (continueFlag) {
		if (_sequenceOffset >=_sequenceData.size()) {
			// Reached the end of the sequence
			if (!_keepActive)
				remove();
			break;
		}

		uint16 idx = static_cast<uint16>(getNextValue() - 32000);
		if (idx > 34)
			continue;

		int16 v1, v2, v3;
		switch (idx) {
		case 0:
			// Stop sequence
			continueFlag = false;
			break;
		case 1:
			_sceneObject->animate(ANIM_MODE_1, NULL);
			break;
		case 2:
			_sceneObject->animate(ANIM_MODE_2, NULL);
			break;
		case 3:
			_sceneObject->animate(ANIM_MODE_3);
			break;
		case 4:
			v1 = getNextValue();
			v2 = getNextValue();
			_sceneObject->animate(ANIM_MODE_8, v1, v2 ? this : NULL);
			break;
		case 5:
			v1 = getNextValue();
			v2 = getNextValue();
			_sceneObject->animate(ANIM_MODE_7, v1, v2 ? this : NULL);
			break;
		case 6:
			v2 = getNextValue();
			_sceneObject->animate(ANIM_MODE_5, v2 ? this : NULL);
			break;
		case 7:
			v2 = getNextValue();
			_sceneObject->animate(ANIM_MODE_6, v2 ? this : NULL);
			break;
		case 8:
			v1 = getNextValue();
			v3 = getNextValue();
			v2 = getNextValue();
			_sceneObject->animate(ANIM_MODE_4, v1, v3, v2 ? this : NULL);
			break;
		case 9:
			v1 = getNextValue();
			v3 = getNextValue();
			v2 = getNextValue();
			_globals->_sceneManager._scene->_sceneBounds.moveTo(v3, v2);
			_globals->_sceneManager._scene->loadScene(v1);
			break;
		case 10: {
			int resNum= getNextValue();
			int lineNum = getNextValue();
			int color = getNextValue();
			int xp = getNextValue();
			int yp = getNextValue();
			int width = getNextValue();
			setMessage(resNum, lineNum, color, Common::Point(xp, yp), width);
			break;
		}
		case 11:
			v1 = getNextValue();
			v2 = getNextValue();
			setAction(globalManager(), v2 ? this : NULL, v1, _objectList[0], _objectList[1], _objectList[2], _objectList[3], NULL);
			break;
		case 12:
			v1 = getNextValue();
			setDelay(v1);
			break;
		case 13: {
			v1 = getNextValue();
			v3 = getNextValue();
			v2 = getNextValue();
			NpcMover *mover = new NpcMover();
			Common::Point destPos(v1, v3);
			_sceneObject->addMover(mover, &destPos, v2 ? this : NULL);
			break;
		}
		case 14:
			v1 = getNextValue();
			_sceneObject->_numFrames = v1;
			break;
		case 15:
			v1 = getNextValue();
			_sceneObject->_moveRate = v1;
			break;
		case 16:
			v1 = getNextValue();
			v2 = getNextValue();
			_sceneObject->_moveDiff = Common::Point(v1, v2);
			break;
		case 17:
			_sceneObject->hide();
			break;
		case 18:
			_sceneObject->show();
			break;
		case 19:
			v1 = getNextValue();
			_sceneObject->setVisage(v1);
			break;
		case 20:
			v1 = getNextValue();
			_sceneObject->setStrip(v1);
			break;
		case 21:
			v1 = getNextValue();
			_sceneObject->setFrame(v1);
			break;
		case 22:
			v1 = getNextValue();
			_sceneObject->fixPriority(v1);
			break;
		case 23:
			v1 = getNextValue();
			_sceneObject->changeZoom(v1);
			break;
		case 24:
			v1 = getNextValue();
			v2 = getNextValue();
			v3 = getNextValue();
			_sceneObject->setPosition(Common::Point(v1, v2), v3);
			break;
		case 25: {
			int yStart = getNextValue();
			int minPercent = getNextValue();
			int yEnd = getNextValue();
			int maxPercent = getNextValue();
			_globals->_sceneManager._scene->setZoomPercents(yStart, minPercent, yEnd, maxPercent);
			break;
		}
		case 26:
			v1 = getNextValue();
			v2 = getNextValue();
			_soundHandler.play(v1, v2 ? this : NULL, 127);
			break;
		case 27: {
			v1 = getNextValue();
			v3 = getNextValue();
			v2 = getNextValue();
			PlayerMover *mover = new PlayerMover();
			Common::Point destPos(v1, v3);
			_sceneObject->addMover(mover, &destPos, v2 ? this : NULL);
			break;
		}
		case 28:
			_objectIndex = getNextValue();
			assert((_objectIndex >= 0) && (_objectIndex < 6));
			_sceneObject = _objectList[_objectIndex];
			assert(_sceneObject);
			break;
		case 29:
			_sceneObject->animate(ANIM_MODE_NONE);
			break;
		case 30:
			v1 = getNextValue();
			_globals->_scrollFollower = (v1 == -1) ? NULL : _objectList[v1];
			break;
		case 31:
			_sceneObject->setObjectWrapper(new SceneObjectWrapper());
			break;
		case 32:
			_sceneObject->setObjectWrapper(NULL);
			break;
		case 33:
			v1 = getNextValue();
			if (_keepActive)
				setDelay(1);
			else {
				_sceneText.remove();
				_globals->_sceneManager._scene->_stripManager.start(v1, this);
			}
			break;
		case 34: {
			v1 = getNextValue();
			v2 = getNextValue();
			int objIndex1 = getNextValue() - 1;
			int objIndex2 = getNextValue() - 1;
			int objIndex3 = getNextValue() - 1;
			int objIndex4 = getNextValue() - 1;
			int objIndex5 = getNextValue() - 1;
			int objIndex6 = getNextValue() - 1;

			setAction(globalManager(), v2 ? this : NULL, v1, _objectList[objIndex1], _objectList[objIndex2],
				_objectList[objIndex3], _objectList[objIndex4], _objectList[objIndex5], _objectList[objIndex6], NULL);
			break;
		}
		default:
			error("SequenceManager::signal - Unknown action %d at offset %xh", idx, _sequenceOffset - 2);
			break;
		}
	}
}

void SequenceManager::process(Event &event) {
	if (((event.eventType == EVENT_BUTTON_DOWN) || (event.eventType == EVENT_KEYPRESS)) &&
		!event.handled && _globals->_sceneObjects->contains(&_sceneText)) {
		// Remove the text item
		_sceneText.remove();
		setDelay(2);
		event.handled = true;
	} else {
		Action::process(event);
	}
}


void SequenceManager::attached(EventHandler *newOwner, EventHandler *endHandler, va_list va) {
	// Get the sequence number to use
	_resNum = va_arg(va, int);

	byte *seqData = _resourceManager->getResource(RES_SEQUENCE, _resNum, 0);
	uint seqSize = _vm->_memoryManager.getSize(seqData);

	_sequenceData.resize(seqSize);
	Common::copy(seqData, seqData + seqSize, &_sequenceData[0]);

	DEALLOCATE(seqData);

	Common::set_to(&_objectList[0], &_objectList[6], (SceneObject *)NULL);
	for (int idx = 0; idx < 6; ++idx) {
		_objectList[idx] = va_arg(va, SceneObject *);
		if (!_objectList[idx])
			break;
	}

	setup();
	Action::attached(newOwner, endHandler, va);
}

/**
 * Returns the next Id in the sequence
 */
uint16 SequenceManager::getNextValue() {
	uint16 result = READ_LE_UINT16(&_sequenceData[0] + _sequenceOffset);
	_sequenceOffset += 2;
	return result;
}

void SequenceManager::setMessage(int resNum, int lineNum, int color, const Common::Point &pt, int width) {
	_sceneText._color1 = color;
	_sceneText._color2 = 0;
	_sceneText._color3 = 0;
	_sceneText._fontNumber = 2;
	_sceneText._width = width;

	// Get the display message
	Common::String msg = _resourceManager->getMessage(resNum, lineNum);

	// Set the text message
	_sceneText.setup(msg);

	// Move the text to the correct position
	Rect textRect = _sceneText._bounds;
	Rect sceneBounds = _globals->_sceneManager._scene->_sceneBounds;
	sceneBounds.collapse(4, 2);
	textRect.moveTo(pt);
	textRect.contain(sceneBounds);

	_sceneText.setPosition(Common::Point(textRect.left, textRect.top));

	// Draw the text
	_sceneText.fixPriority(255);
	_sceneText.show();

	// Set the delay based on the number of words
	int numWords = 0;
	const char *msgP = msg.c_str();
	while (*msgP) {
		if (*msgP++ == ' ')
			++numWords;
	}

	setDelay(numWords * 18 + 120);
}

SequenceManager *SequenceManager::globalManager() {
	return &_globals->_sequenceManager;
}

/*--------------------------------------------------------------------------*/

ConversationChoiceDialog::ConversationChoiceDialog() {
	_stdColor = 23;
	_highlightColor = _globals->_scenePalette._colors.background;
	_fontNumber = 1;
}

int ConversationChoiceDialog::execute(const Common::StringArray &choiceList) {
	_gfxManager._font.setFontNumber(_fontNumber);

	_bounds = Rect(20, 0, 20, 0);
	_choiceList.clear();

	// Set up the list of choices
	int yp = 0;
	for (uint idx = 0; idx < choiceList.size(); ++idx) {
		Rect tempRect;
		_gfxManager._font.getStringBounds(choiceList[idx].c_str(), tempRect, 265);
		tempRect.moveTo(25, yp + 10);

		_choiceList.push_back(ChoiceEntry(choiceList[idx], tempRect));
		yp += tempRect.height() + 5;
		_bounds.extend(tempRect);
	}
	_selectedIndex = _choiceList.size();

	// Set the position for the dialog
	_bounds.bottom -= 10;
	yp = 180 - _bounds.height();
	_bounds.translate(0, yp);
	_bounds.right = _bounds.left + 280;

	// Draw the dialog
	draw();
	_globals->_events.showCursor();

	// Event handling loop
	Event event;
	while (!_vm->shouldQuit()) {
		while (!_globals->_events.getEvent(event, EVENT_KEYPRESS | EVENT_BUTTON_DOWN | EVENT_MOUSE_MOVE) &&
				!_vm->shouldQuit()) {
			g_system->delayMillis(10);
			g_system->updateScreen();
		}
		if (_vm->shouldQuit())
			break;

		if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode >= Common::KEYCODE_1) &&
			(event.kbd.keycode <= (Common::KEYCODE_0 + (int)_choiceList.size()))) {
			// Selected an option by number
			_selectedIndex = event.kbd.keycode - Common::KEYCODE_1;
			break;
		} else if ((_selectedIndex != _choiceList.size()) && ((event.eventType == EVENT_BUTTON_DOWN) ||
					(event.eventType == EVENT_BUTTON_UP))) {
			// Item selected
			break;
		} else {
			// Check if any item is highlighted
			event.mousePos.x -= _gfxManager._bounds.left;
			event.mousePos.y -= _gfxManager._bounds.top;

			uint idx = 0;
			while ((idx < _choiceList.size()) && !_choiceList[idx]._bounds.contains(event.mousePos.x, event.mousePos.y))
				++idx;

			if (idx != _selectedIndex) {
				if (_selectedIndex != _choiceList.size()) {
					// De-highlight previously selected item
					_gfxManager._font._colors.foreground = _stdColor;
					_gfxManager._font.writeLines(_choiceList[_selectedIndex]._msg.c_str(),
						_choiceList[_selectedIndex]._bounds, ALIGN_LEFT);
				}

				_selectedIndex = idx;

				if (_selectedIndex != _choiceList.size()) {
					// Highlight the new item
					_gfxManager._font._colors.foreground = _highlightColor;
					_gfxManager._font.writeLines(_choiceList[idx]._msg.c_str(), _choiceList[idx]._bounds, ALIGN_LEFT);
				}

			}
		}
	}

	// Remove the dialog
	remove();

	return _selectedIndex;
}

void ConversationChoiceDialog::draw() {
	// Make a backup copy of the area the dialog will occupy
	Rect tempRect = _bounds;
	tempRect.collapse(-10, -10);
	_savedArea = Surface_getArea(_globals->_gfxManagerInstance.getSurface(), tempRect);

	// Fill in the contents of the entire dialog
	_gfxManager._bounds = Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	drawFrame();

	_gfxManager._bounds = tempRect;
	_gfxManager._font._colors.foreground = _stdColor;
	_gfxManager.activate();

	// Loop through writing the conversation choices
	for (uint idx = 0; idx < _choiceList.size(); ++idx) {
		Common::String strNum = Common::String::format("%d", idx + 1);

		// Write the choice number
		_gfxManager._font.setPosition(13, _choiceList[idx]._bounds.top);
		_gfxManager._font.writeString(strNum.c_str());

		_gfxManager._font.writeLines(_choiceList[idx]._msg.c_str(), _choiceList[idx]._bounds, ALIGN_LEFT);
	}

	_gfxManager.deactivate();
}

/*--------------------------------------------------------------------------*/

void Obj44::load(const byte *dataP) {
	_id = READ_LE_UINT16(dataP);
	for (int idx = 0; idx < OBJ44_LIST_SIZE; ++idx)
		_field2[idx] = READ_LE_UINT16(dataP + 2 + idx * 2);

	const byte *listP = dataP + 0x10;
	for (int idx = 0; idx < OBJ44_LIST_SIZE; ++idx, listP += 10) {
		_list[idx]._id = READ_LE_UINT16(listP);
		_list[idx]._scriptOffset = READ_LE_UINT16(listP + 2);
	}

	_speakerOffset = READ_LE_UINT16(dataP + 0x42);
}

void Obj44::synchronize(Serializer &s) {
	s.syncAsSint32LE(_id);
	for (int idx = 0; idx < OBJ44_LIST_SIZE; ++idx)
		s.syncAsSint32LE(_field2[idx]);
	for (int idx = 0; idx < OBJ44_LIST_SIZE; ++idx)
		_list[idx].synchronize(s);
	s.syncAsUint32LE(_speakerOffset);
}

/*--------------------------------------------------------------------------*/

StripManager::StripManager() {
	_callbackObject = NULL;
	_activeSpeaker = NULL;
	reset();
}

StripManager::~StripManager() {
}

void StripManager::start(int stripNum, EventHandler *owner, StripCallback *callback) {
	reset();

	_stripNum = stripNum;
	_callbackObject = callback;
	_sceneNumber = _globals->_sceneManager._scene->_screenNumber;
	_sceneBounds = _globals->_sceneManager._scene->_sceneBounds;
	_script.clear();

	assert(owner);
	owner->setAction(this, owner);
}

void StripManager::reset() {
	_actionIndex = 0;
	_delayFrames = 0;
	_owner = NULL;
	_endHandler = NULL;
	_field2E6 = false;
	_stripNum = -1;
	_obj44Index = 0;
	_field2E8 = 0;
	_field20 = 0;
	_activeSpeaker = NULL;
	_textShown = false;
	_callbackObject = NULL;

	_obj44List.clear();
	if (!_script.empty()) {
		_script.clear();
	}
}

void StripManager::load() {
	// Get the script
	byte *script = _resourceManager->getResource(RES_STRIP, _stripNum, 2);
	uint scriptSize = _vm->_memoryManager.getSize(script);

	_script.resize(scriptSize);
	Common::copy(script, script + scriptSize, &_script[0]);

	DEALLOCATE(script);

	// Get the object list
	byte *obj44List = _resourceManager->getResource(RES_STRIP, _stripNum, 1);
	int dataSize = _vm->_memoryManager.getSize(obj44List);
	assert((dataSize % 0x44) == 0);

	byte *dataP = obj44List;
	for (int idx = 0; idx < (dataSize / 0x44); ++idx, dataP += 0x44) {
		Obj44 obj;
		obj.load(dataP);
		_obj44List.push_back(obj);
	}

	DEALLOCATE(obj44List);
}

void StripManager::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		Action::synchronize(s);

	s.syncAsSint32LE(_stripNum);
	s.syncAsSint32LE(_obj44Index);
	s.syncAsSint32LE(_field20);
	s.syncAsSint32LE(_sceneNumber);
	_sceneBounds.synchronize(s);
	SYNC_POINTER(_activeSpeaker);
	s.syncAsByte(_textShown);
	s.syncAsByte(_field2E6);
	s.syncAsSint32LE(_field2E8);

	// Synchronize the item list
	int arrSize = _obj44List.size();
	s.syncAsUint16LE(arrSize);
	if (s.isLoading())
		_obj44List.resize(arrSize);
	for (int i = 0; i < arrSize; ++i)
		_obj44List[i].synchronize(s);

	// Synhcronise script data
	int scriptSize = _script.size();
	s.syncAsUint16LE(scriptSize);
	if (s.isLoading())
		_script.resize(scriptSize);
	if (scriptSize > 0)
		s.syncBytes(&_script[0], scriptSize);

	// Add speaker list
	arrSize = _speakerList.size();
	s.syncAsUint16LE(arrSize);
	if (s.isLoading())
		_speakerList.resize(arrSize);
	for (int i = 0; i < arrSize; ++i)
		SYNC_POINTER(_speakerList[i]);

	SYNC_POINTER(_callbackObject);
}

void StripManager::remove() {
	if (_textShown) {
		if (_activeSpeaker)
			_activeSpeaker->removeText();
		_textShown = false;
	}

	if (_activeSpeaker)
		_activeSpeaker->remove();

	if (_sceneNumber != _globals->_sceneManager._scene->_screenNumber) {
		_globals->_sceneManager._scene->_sceneBounds = _sceneBounds;
		_globals->_sceneManager._scene->loadScene(_sceneNumber);
	}

	Action::remove();
}

void StripManager::signal() {
	if (_textShown) {
		_activeSpeaker->removeText();
		_textShown = false;
	}

	if (_obj44Index < 0) {
		EventHandler *owner = _endHandler;
		int stripNum = ABS(_obj44Index);
		remove();

		start(stripNum, owner);
		return;
	} else if (_obj44Index == 10000) {
		// Reached end of strip
		remove();
		return;
	}

	// Run strip

	if (_obj44List.size() == 0)
		// Load the data for the strip
		load();

	Obj44 &obj44 = _obj44List[_obj44Index];
	_field2E8 = obj44._id;
	Common::StringArray choiceList;

	// Build up a list of script entries
	int idx;
	for (idx = 0; idx < OBJ44_LIST_SIZE; ++idx) {
		if (!obj44._list[idx]._id)
			break;

		// Get the next one
		choiceList.push_back((const char *)&_script[0] + obj44._list[idx]._scriptOffset);
	}

	int strIndex = 0;
	if (choiceList.size() > 1)
		// Get the user to select a conversation option
		strIndex = _choiceDialog.execute(choiceList);

	if ((choiceList.size() != 1) && !_field2E6)
		_delayFrames = 1;
	else {
		Speaker *speakerP = getSpeaker((const char *)&_script[0] + obj44._speakerOffset);
		if (!speakerP)
			error("Speaker not found.  Screenplay: %s %d", (const char *)&_script[0] + obj44._speakerOffset, _stripNum);

		if (speakerP != _activeSpeaker) {
			if (_activeSpeaker)
				_activeSpeaker->remove();
			_activeSpeaker = speakerP;

			if ((_activeSpeaker->_newSceneNumber == -1) && (_globals->_sceneManager._scene->_screenNumber != _sceneNumber)) {
				_globals->_sceneManager._scene->_sceneBounds = _sceneBounds;
				_globals->_sceneManager._scene->loadScene(_sceneNumber);
			}

			_activeSpeaker->proc12(this);
		}

		if (_callbackObject) {
			for (idx = 0; idx < OBJ44_LIST_SIZE; ++idx) {
				if (!obj44._field2[idx])
					break;

				_callbackObject->stripCallback(obj44._field2[idx]);
			}
		}

		_textShown = true;
		_activeSpeaker->setText(choiceList[strIndex]);
	}

	_obj44Index = getNewIndex(obj44._list[strIndex]._id);
	if (_obj44Index == 10001) {
		MessageDialog::show("Strip Failure: Node not found", OK_BTN_STRING);
		_obj44Index = 0;
	}
}

void StripManager::process(Event &event) {
	Action::process(event);
	if (event.handled)
		return;

	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
		if (_obj44Index != 10000) {
			int currIndex = _obj44Index;
			while (!_obj44List[_obj44Index]._list[1]._id) {
				_obj44Index = getNewIndex(_obj44List[_obj44Index]._list[0]._id);
				if ((_obj44Index < 0) || (_obj44Index == 10000))
					break;
				currIndex = _obj44Index;
			}

			_field2E8 = _obj44List[currIndex]._id;
		}

		// Signal the end of the strip
		_delayFrames = 0;
		event.handled = true;
		signal();
	} else if (event.eventType & (EVENT_BUTTON_DOWN | EVENT_KEYPRESS)) {
		// Move to next sequence in the strip
		_delayFrames = 0;
		event.handled = true;
		signal();
	}
}

void StripManager::addSpeaker(Speaker *speaker) {
	assert(_speakerList.size() < 100);
	_speakerList.push_back(speaker);
}

Speaker *StripManager::getSpeaker(const char *speakerName) {
	for (uint idx = 0; idx < _speakerList.size(); ++idx) {
		if (!strcmp(_speakerList[idx]->_speakerName.c_str(), speakerName))
			return _speakerList[idx];
	}

	return NULL;
}

int StripManager::getNewIndex(int id) {
	if (id == 10000)
		return id;

	for (uint idx = 0; idx < _obj44List.size(); ++idx) {
		if (_obj44List[idx]._id == id) {
			return (id == 0) ? 10001 : idx;
		}
	}

	return 10001;
}

/*--------------------------------------------------------------------------*/

Speaker::Speaker() : EventHandler() {
	_newSceneNumber = -1;
	_hideObjects = true;
	_field18 = 0;
	_textWidth = 140;
	_textPos = Common::Point(10, 20);
	_fontNumber = 2;
	_textMode = ALIGN_LEFT;
	_color1 = _color2 = _color3 = _globals->_scenePalette._colors.foreground;
	_action = NULL;
	_speakerName = "SPEAKER";
}

void Speaker::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		EventHandler::synchronize(s);

	_fieldA.synchronize(s);
	SYNC_POINTER(_field18);
	s.syncString(_speakerName);
	s.syncAsSint32LE(_newSceneNumber);
	s.syncAsSint32LE(_oldSceneNumber);
	_sceneBounds.synchronize(s);
	s.syncAsSint32LE(_textWidth);
	s.syncAsSint16LE(_textPos.x); s.syncAsSint16LE(_textPos.y);
	s.syncAsSint32LE(_fontNumber);
	SYNC_ENUM(_textMode, TextAlign);
	s.syncAsSint16LE(_color1);
	s.syncAsSint16LE(_color2);
	s.syncAsSint16LE(_color3);
	s.syncAsByte(_hideObjects);
}

void Speaker::remove() {
	if (_hideObjects)
		SceneObjectList::deactivate();
}

void Speaker::proc12(Action *action) {
	_action = action;
	if (_newSceneNumber != -1) {
		_oldSceneNumber = _globals->_sceneManager._sceneNumber;
		_sceneBounds = _globals->_sceneManager._scene->_sceneBounds;
		_globals->_sceneManager._scene->loadScene(_newSceneNumber);
	}

	if (_hideObjects)
		// Activate the object list for display
		_objectList.activate();

	// Draw the speaker objects without any fading
	FadeMode fadeMode = _globals->_sceneManager._fadeMode;
	_globals->_sceneManager._fadeMode = FADEMODE_IMMEDIATE;
	_globals->_sceneObjects->draw();
	_globals->_sceneManager._fadeMode = fadeMode;
}

void Speaker::setText(const Common::String &msg) {
	_globals->_sceneObjects->draw();

	_sceneText._color1 = _color1;
	_sceneText._color2 = _color2;
	_sceneText._color3 = _color3;
	_sceneText._width = _textWidth;
	_sceneText._fontNumber = _fontNumber;
	_sceneText._textMode = _textMode;
	_sceneText.setup(msg);
	_sceneText.setPosition(_textPos);
	_sceneText.fixPriority(256);

	// Count the number of words (by spaces) in the string
	const char *msgP = msg.c_str();
	int spaceCount = 0;
	while (*msgP) {
		if (*msgP++ == ' ')
			++spaceCount;
	}

	int numFrames = spaceCount * STRIP_WORD_DELAY + 120;
	if (_action)
		_action->setDelay(numFrames);
}

void Speaker::removeText() {
	_sceneText.remove();
}

/*--------------------------------------------------------------------------*/

SpeakerGameText::SpeakerGameText() : Speaker() {
	_speakerName = "GAMETEXT";
	_textPos = Common::Point(40, 40);
	_textMode = ALIGN_CENTER;
	_color1 = 7;
	_textWidth = 230;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

ScreenSpeaker::ScreenSpeaker() : Speaker() {
	_npc = NULL;
	_textMode = ALIGN_CENTER;
}

void ScreenSpeaker::setText(const Common::String &msg) {
	GfxManager gfxMan;
	gfxMan.activate();
	gfxMan._font.setFontNumber(_fontNumber);
	Rect textRect;

	_globals->gfxManager().getStringBounds(msg.c_str(), textRect, _textWidth);
	if (_npc) {
		textRect.center(_npc->_position.x, _npc->_bounds.top - (textRect.height() / 2 + 10));
	} else {
		textRect.center(_globals->_sceneManager._scene->_sceneBounds.left +
			(_globals->_sceneManager._scene->_sceneBounds.width() / 2),
			_globals->_sceneManager._scene->_sceneBounds.top);
	}

	Rect rect2 = _globals->_sceneManager._scene->_sceneBounds;
	rect2.collapse(10, 6);
	textRect.contain(rect2);

	_textPos.x = textRect.left;
	_textPos.y = textRect.top;
	Speaker::setText(msg);

	gfxMan.deactivate();
}

/*--------------------------------------------------------------------------*/

void SpeakerAction::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(_globals->_randomSource.getRandomNumber(60) + 60);
		break;
	case 1:
		static_cast<SceneObject *>(_owner)->setFrame(1);
		static_cast<SceneObject *>(_owner)->animate(ANIM_MODE_5, this, NULL);
		break;
	case 2:
		setDelay(_globals->_randomSource.getRandomNumber(10));
		_actionIndex = 0;
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void AnimatedSpeaker::removeText() {
	Speaker::removeText();
	_object1.remove();
	_object2.remove();

	_objectList.draw();
}

} // end of namespace tSage
