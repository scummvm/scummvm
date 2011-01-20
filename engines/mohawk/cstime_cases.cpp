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

#include "mohawk/cstime_cases.h"
#include "mohawk/cstime_ui.h"

namespace Mohawk {

CSTimeCase1::CSTimeCase1(MohawkEngine_CSTime *vm) : CSTimeCase(vm, 1) {
}

CSTimeCase1::~CSTimeCase1() {
}

bool CSTimeCase1::checkConvCondition(uint16 conditionId) {
	const Common::Array<CSTimeHotspot> &hotspots = getCurrScene()->getHotspots();

	bool gotTorch = _vm->_haveInvItem[1];

	// These are all for conversations in the first scene (with the boatman).
	switch (conditionId) {
	case 0:
		// Got the torch?
		return (gotTorch);
	case 1:
		// Is the bag still on land?
		return (hotspots[5].state == 1);
	case 2:
		// Is the bag on the boat, but player hasn't taken the torch?
		return (hotspots[5].state != 1 && !gotTorch);
	}

	return false;
}

bool CSTimeCase1::checkAmbientCondition(uint16 charId, uint16 ambientId) {
	return true;
}

bool CSTimeCase1::checkObjectCondition(uint16 objectId) {
	const Common::Array<CSTimeHotspot> &hotspots = getCurrScene()->getHotspots();

	switch (_currScene) {
	case 1:
		switch (objectId) {
		case 1:
			// Hide bag on boat if it's not there.
			return (hotspots[5].state == 1);
		case 2:
			// Hide bag on land if it's not there.
			return (hotspots[5].state != 1);
		case 3:
			// Hide torch if it's been picked up.
			return (hotspots[4].state == 1);
		}
		break;

	case 2:
		// The first note piece.
		return !_vm->getInterface()->getCarmenNote()->havePiece(0);

	case 3:
		// The features representing different stages in the body sequence.
		if (objectId == 6 && _vm->_caseVariable[3] != 0)
			return false;
		else if (objectId == 7 && _vm->_caseVariable[3] != 1)
			return false;
		else if (objectId == 8 && _vm->_caseVariable[3] != 2)
			return false;
		break;

	case 4:
		// The second note piece?
		if (objectId == 0)
			return (hotspots[0].state > 0);
		break;

	case 5:
		// The third note piece.
		if (objectId == 1)
			return !_vm->getInterface()->getCarmenNote()->havePiece(2);
	}

	return true;
}

void CSTimeCase1::selectHelpStrings() {
	if (_currScene == 1) {
		if (_vm->_haveInvItem[1]) {
			// Got the torch, ready to leave.
			// FIXME
		} else {
			// Still don't have the torch.
			// FIXME
		}
	} else {
		// FIXME
	}

	// FIXME
}

void CSTimeCase1::handleConditionalEvent(const CSTimeEvent &event) {
	CSTimeEvent newEvent;

	switch (event.param2) {
	case 0:
		// Trying to enter the first room of the tomb.
		if (!_conversations[1]->getAsked(2, 0)) {
			// We need a plan first.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 12352));
		} else if (!_vm->getInterface()->getCarmenNote()->havePiece(0)) {
			// Shouldn't we take a look at that note?
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 10355));
		} else {
			// Onward!
			_vm->addEvent(CSTimeEvent(kCSTimeEventNewScene, event.param1, 3));
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, 12551));
		}
		break;

	case 1:
		// Poking at the jars. The response depends on whether the hieroglyphs on the tomb wall
		// have been seen yet or not.
		_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(),
			_vm->_caseVariable[2] ? 14304 : 14303));
		break;

	case 2:
		// FIXME: Leaving the mummy-preparing room?
		error("cond event 2");
		break;

	case 3:
		// FIXME: Body sequence stuff.
		error("cond event 3");
		break;

	case 4:
		// Woven bag dragged.
		if (_conversations[0]->getAsked(2, 1)) {
			// We were asked to move it.
			if (event.param1 == 5) {
				// Yay, the player got it on the boat!
				// Congratulate the player and enable the torch.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 2, 10551));
				getCurrScene()->getHotspot(4).invObjId = 1;
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventEnableHotspot, 2, 6));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 2, 2));
			} else {
				assert(event.param1 < 7);
				// It didn't get dropped onto the boat, so we complain about it.
				newEvent.type = kCSTimeEventCharStartFlapping;
				// Does the Good Guide complain (if we gave it to her, or put it in the inventory)?
				newEvent.param1 = (event.param1 == 1 || event.param1 == 6) ? getCurrScene()->getHelperId() : 2;
				// Which string?
				static const uint16 strings[7] = { 30201, 30103, 30202, 30203, 30203, 0, 10352};
				newEvent.param2 = strings[event.param1];
				_vm->insertEventAtFront(newEvent);
			}
		} else {
			// We're just randomly moving the woven bag!
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 10351));

			if (event.param1 == 5) {
				// This went onto the boat hotspot, so the bag was removed; put it back.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventEnableHotspot, 0xffff, 5));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, 1));
			}
		}
		break;

	case 5:
		// We're ready to shove off!
		_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 10356));
		break;

	case 6:
		// Trying to leave the first scene by walking.
		if (_vm->_haveInvItem[1]) {
			// If you have the torch, the Good Guide prods you to use the boat.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 10305));
		} else {
			// Otherwise, the boatman tells you that you can't leave yet.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 2, 10506));
		}
		break;

	case 7:
		// Clicking on the woven bag.
		if (_conversations[0]->getAsked(2, 0)) {
			// If we were asked to move it, the Good Guide prods us to try dragging.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 10306));
		} else {
			// Otherwise, the boatman tells us what it is.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 2, 10502));
		}
		break;

	case 8:
		// One-time-only reminder that you can re-ask questions.
		if (_vm->_caseVariable[7])
			break;
		_vm->_caseVariable[7] = 1;
		_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 0, 12359));
		break;

	case 9:
		// Trying to give the torch to the Good Guide; you get a different message
		// depending on whether it's already in your inventory or not.
		_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 0, _vm->_haveInvItem[1] ? 9906 : 30119));
		break;

	default:
		error("unknown Conditional Event type %d for case 1", event.param2);
	}
}

} // End of namespace Mohawk
