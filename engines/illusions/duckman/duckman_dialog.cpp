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

#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/duckman_dialog.h"
#include "illusions/duckman/scriptopcodes_duckman.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/cursor.h"
#include "illusions/dictionary.h"
#include "illusions/resources/fontresource.h"
#include "illusions/graphics.h"
#include "illusions/input.h"
#include "illusions/resources/actorresource.h"
#include "illusions/resources/backgroundresource.h"
#include "illusions/resources/midiresource.h"
#include "illusions/resources/scriptresource.h"
#include "illusions/resources/soundresource.h"
#include "illusions/resources/talkresource.h"
#include "illusions/resourcesystem.h"
#include "illusions/screen.h"
#include "illusions/screentext.h"
#include "illusions/scriptstack.h"
#include "illusions/sound.h"
#include "illusions/specialcode.h"
#include "illusions/textdrawer.h"
#include "illusions/thread.h"
#include "illusions/time.h"
#include "illusions/updatefunctions.h"

#include "illusions/threads/abortablethread.h"
#include "illusions/threads/causethread_duckman.h"
#include "illusions/threads/scriptthread.h"
#include "illusions/threads/talkthread_duckman.h"
#include "illusions/threads/timerthread.h"

#include "engines/util.h"

namespace Illusions {

// DuckmanDialogSystem

DuckmanDialogSystem::DuckmanDialogSystem(IllusionsEngine_Duckman *vm)
	: _vm(vm) {
}

DuckmanDialogSystem::~DuckmanDialogSystem() {
}

void DuckmanDialogSystem::addDialogItem(int16 choiceJumpOffs, uint32 sequenceId) {
	DialogItem dialogItem;
	dialogItem._choiceJumpOffs = choiceJumpOffs;
	dialogItem._sequenceId = sequenceId;
	_dialogItems.push_back(dialogItem);
}

void DuckmanDialogSystem::startDialog(int16 *choiceOfsPtr, uint32 actorTypeId, uint32 callerThreadId) {
	static const uint32 kDialogSequenceIds[] = {
		0,
		0x6049C, 0x6049C, 0x6047A, 0x6049D,
		0x60479, 0x6049E, 0x6049F, 0x60468
	};
	if (_dialogItems.size() == 1) {
		*choiceOfsPtr = _dialogItems[0]._choiceJumpOffs;
		_vm->notifyThreadId(callerThreadId);
	} else {
		if (!_vm->_cursor._control) {
			Common::Point pos = _vm->getNamedPointPosition(0x70001);
			_vm->_controls->placeActor(0x50001, pos, 0x60001, Illusions::CURSOR_OBJECT_ID, 0);
			_vm->_cursor._control = _vm->_dict->getObjectControl(Illusions::CURSOR_OBJECT_ID);
		}
		_vm->_cursor._control->appearActor();
		_vm->setCursorActorIndex(6, 1, 0);

		_vm->_cursor._gameState = 3;
		_vm->_cursor._notifyThreadId30 = callerThreadId;
		_vm->_cursor._dialogItemsCount = 0;
		_vm->_cursor._overlappedObjectId = 0;
		_vm->_cursor._op113_choiceOfsPtr = choiceOfsPtr;
		_vm->_cursor._currOverlappedControl = 0;

		/* TODO?
		if (!_vm->_input->getCursorMouseMode())
			_vm->_input->setMousePos((Point)0xBC0014);
		*/

		_vm->_cursor._dialogItemsCount = _dialogItems.size();
		Common::Point placePt(20, 188);

		for (uint i = 1; i <= _dialogItems.size(); ++i) {
			DialogItem &dialogItem = _dialogItems[_dialogItems.size() - i];
			_vm->_controls->placeDialogItem(i + 1, actorTypeId, dialogItem._sequenceId, placePt, dialogItem._choiceJumpOffs);
			placePt.x += 40;
		}

		Common::Point placePt2 = _vm->getNamedPointPosition(0x700C3);
		_vm->_controls->placeActor(0x5006E, placePt2, kDialogSequenceIds[_dialogItems.size()], 0x40148, 0);
		Control *control = _vm->_dict->getObjectControl(0x40148);
		control->_flags |= 8;
		_vm->playSoundEffect(8);
	}

	_dialogItems.clear();

}

void DuckmanDialogSystem::updateDialogState() {
	Common::Point mousePos = _vm->_input->getCursorPosition();
	// TODO Handle keyboard input
	_vm->_cursor._control->_actor->_position = mousePos;
	mousePos = _vm->convertMousePos(mousePos);

	Control *currOverlappedControl = _vm->_cursor._currOverlappedControl;
	Control *newOverlappedControl;

	if (_vm->_controls->getDialogItemAtPos(_vm->_cursor._control, mousePos, &newOverlappedControl)) {
		if (currOverlappedControl != newOverlappedControl) {
			newOverlappedControl->setActorIndex(2);
			newOverlappedControl->startSequenceActor(newOverlappedControl->_actor->_sequenceId, 2, 0);
			if (currOverlappedControl) {
				currOverlappedControl->setActorIndex(1);
				currOverlappedControl->startSequenceActor(currOverlappedControl->_actor->_sequenceId, 2, 0);
			}
			_vm->playSoundEffect(10);
			_vm->startCursorSequence();
			_vm->setCursorActorIndex(6, 2, 0);
			_vm->_cursor._currOverlappedControl = newOverlappedControl;
			_vm->_cursor._overlappedObjectId = newOverlappedControl->_objectId;
		}
	} else if (currOverlappedControl) {
		currOverlappedControl->setActorIndex(1);
		currOverlappedControl->startSequenceActor(currOverlappedControl->_actor->_sequenceId, 2, 0);
		_vm->playSoundEffect(10);
		_vm->_cursor._currOverlappedControl = 0;
		_vm->_cursor._overlappedObjectId = 0;
		_vm->startCursorSequence();
		_vm->setCursorActorIndex(6, 1, 0);
	}

	if (_vm->_input->pollEvent(kEventLeftClick)) {
		if (_vm->_cursor._currOverlappedControl) {
			_vm->playSoundEffect(9);
			*_vm->_cursor._op113_choiceOfsPtr = _vm->_cursor._currOverlappedControl->_actor->_choiceJumpOffs;
			_vm->_controls->destroyDialogItems();
			Control *control = _vm->_dict->getObjectControl(0x40148);
			_vm->_controls->destroyControl(control);
			_vm->notifyThreadId(_vm->_cursor._notifyThreadId30);
			_vm->_cursor._notifyThreadId30 = 0;
			_vm->_cursor._gameState = 2;
			_vm->_cursor._dialogItemsCount = 0;
			_vm->_cursor._overlappedObjectId = 0;
			_vm->_cursor._op113_choiceOfsPtr = 0;
			_vm->_cursor._control->disappearActor();
		}
	}

}

} // End of namespace Illusions
