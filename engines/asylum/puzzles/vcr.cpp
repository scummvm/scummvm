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
 * $URL$
 * $Id$
 *
 */

#include "asylum/puzzles/vcr.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/config.h"
#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/video.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"
#include "asylum/shared.h"

namespace Asylum {

BlowUpPuzzleVCR::BlowUpPuzzleVCR(AsylumEngine *engine): BlowUpPuzzle(engine) {
	// reset all states
	memset(&_jacksState,   0, sizeof(_jacksState));
	memset(&_holesState,   0, sizeof(_holesState));
	memset(&_buttonsState, 0, sizeof(_buttonsState));

	_tvScreenAnimIdx = 0;
	_isAccomplished  = false;
}

BlowUpPuzzleVCR::~BlowUpPuzzleVCR() {
	delete _cursor;
	delete _bgResource;
}

void BlowUpPuzzleVCR::open() {
	_active = true;
	getScene()->deactivate();

	getSound()->stopAll();

	// Load the graphics palette
	getScreen()->setPalette(getWorld()->graphicResourceIds[29]);

	// show blow up puzzle BG
	getScreen()->draw(getWorld()->graphicResourceIds[0], 0, 0, 0, 0);

	// Set mouse cursor
	_cursor->set(getWorld()->graphicResourceIds[28], 0, 2);
	_cursor->show();

	_leftClickUp    = false;
	_leftClickDown  = false;
	_rightClickDown = false;
}

void BlowUpPuzzleVCR::close() {
	_active = false;
	getScene()->activate();
}

bool BlowUpPuzzleVCR::handleEvent(const AsylumEvent &ev) {
	switch (ev.type) {
	case Common::EVENT_MOUSEMOVE:
		//_cursor->move(ev.mouse.x, ev.mouse.y);
		break;
	case Common::EVENT_LBUTTONUP:
		_leftClickUp = true;
		break;
	case Common::EVENT_LBUTTONDOWN:
		_leftClickDown = true;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_rightClickDown = true;
		break;
	default:
		break;
	}

	if (_leftClickUp || _leftClickDown)
		update();

	return true;
}

void BlowUpPuzzle::playSound(ResourceId resourceId, bool loop) {
	getSound()->playSound(resourceId, loop, Config.sfxVolume, 0);
}

int BlowUpPuzzleVCR::inPolyRegion(int x, int y, int polyIdx) const {
	return  x >= BlowUpPuzzleVCRPolies[polyIdx].left && x <= BlowUpPuzzleVCRPolies[polyIdx].right &&
	        y >= BlowUpPuzzleVCRPolies[polyIdx].top  && y <= BlowUpPuzzleVCRPolies[polyIdx].bottom;
}

void BlowUpPuzzleVCR::update() {
	getScreen()->clearGraphicsInQueue();

	if (_rightClickDown) { // quits BlowUp Puzzle
		_rightClickDown = false;
		close();
		getSound()->stopAll();
	}

	if (_leftClickDown) {
		_leftClickDown = false;
		handleMouseDown();
	}

	if (_leftClickUp) {
		_leftClickUp = false;
		handleMouseUp();
	}

	updateCursorInPolyRegion();

	updateBlackJack();
	updateRedJack();
	updateYellowJack();

	updatePowerButton();
	updateRewindButton();
	updatePlayButton();
	updateStopButton();

	if (_buttonsState[kPower] == kON) {
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[22], _tvScreenAnimIdx, Common::Point(0, 37), 0, 0, 1);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[23], _tvScreenAnimIdx++, Common::Point(238, 22), 0, 0, 1);
		_tvScreenAnimIdx %= 6;
	}

	if (_isAccomplished) {
		getScreen()->drawGraphicsInQueue();

		int16 barSize = 0;
		do {
			getScreen()->drawWideScreenBars(barSize);
			barSize += 4;
		} while (barSize < 84);

		// TODO: fade palette to gray

		getVideo()->playVideo(2);

		_isAccomplished = false;
		close();
	} else {
		getScreen()->drawGraphicsInQueue();
	}
}

GraphicQueueItem BlowUpPuzzleVCR::getGraphicJackItem(int32 index) {
	GraphicQueueItem jackItemOnHand;

	int jackY = _cursor->position().y;
	if (_cursor->position().y < 356) {
		jackY = 356;
	}

	jackItemOnHand.resourceId = getWorld()->graphicResourceIds[index];
	jackItemOnHand.frameIndex = 0;
	jackItemOnHand.point = Common::Point(_cursor->position().x - 114, jackY - 14);
	jackItemOnHand.priority = 1;

	return jackItemOnHand;
}

GraphicQueueItem BlowUpPuzzleVCR::getGraphicShadowItem() {
	GraphicQueueItem shadowItem;

	int shadowY = (_cursor->position().y - 356) / 4;
	if (_cursor->position().y < 356) {
		shadowY = 0;
	}
	shadowItem.resourceId = getWorld()->graphicResourceIds[30];
	shadowItem.frameIndex = 0;
	shadowItem.point = Common::Point(_cursor->position().x - shadowY, 450);
	shadowItem.priority = 2;

	return shadowItem;
}

void BlowUpPuzzleVCR::updateJack(Jack jack, const VCRDrawInfo &onTable, const VCRDrawInfo &pluggedOnRed, const VCRDrawInfo &pluggedOnYellow, const VCRDrawInfo &pluggedOnBlack, int32 resourceOnHandIndex) {
	GraphicQueueItem item;

	switch (_jacksState[jack]) {
	case kOnTable:
		item.resourceId = getWorld()->graphicResourceIds[onTable.resourceId];
		item.frameIndex = 0;
		item.point = onTable.point;
		item.priority = 3;
		break;

	case kPluggedOnRed:
		item.resourceId = getWorld()->graphicResourceIds[pluggedOnRed.resourceId];
		item.frameIndex = 0;
		item.point = Common::Point(329, 407);
		item.priority = 3;
		break;

	case kPluggedOnYellow:
		item.resourceId = getWorld()->graphicResourceIds[pluggedOnYellow.resourceId];
		item.frameIndex = 0;
		item.point = Common::Point(402, 413);
		item.priority = 3;
		break;

	case kPluggedOnBlack:
		item.resourceId = getWorld()->graphicResourceIds[pluggedOnBlack.resourceId];
		item.frameIndex = 0;
		item.point = Common::Point(477, 418);
		item.priority = 3;
		break;

	case kOnHand: {
		GraphicQueueItem jackItemOnHand = getGraphicJackItem(resourceOnHandIndex);
		getScreen()->addGraphicToQueue(jackItemOnHand);

		item = getGraphicShadowItem();
		}
		break;

	default:
		item.resourceId = kResourceNone;
		break;
	}

	if (item.resourceId != 0)
		getScreen()->addGraphicToQueue(item);
}

void BlowUpPuzzleVCR::updateBlackJack() {
	VCRDrawInfo onTable;
	onTable.resourceId = 1;
	onTable.point = Common::Point(0, 411);

	VCRDrawInfo pluggedOnRed;
	pluggedOnRed.resourceId = 5;

	VCRDrawInfo pluggedOnYellow;
	pluggedOnYellow.resourceId = 8;

	VCRDrawInfo pluggedOnBlack;
	pluggedOnBlack.resourceId = 11;

	updateJack(kBlack, onTable, pluggedOnRed, pluggedOnYellow, pluggedOnBlack, 27);
}

void BlowUpPuzzleVCR::updateRedJack() {
	VCRDrawInfo onTable;
	onTable.resourceId = 2;
	onTable.point = Common::Point(76, 428);

	VCRDrawInfo pluggedOnRed;
	pluggedOnRed.resourceId = 4;

	VCRDrawInfo pluggedOnYellow;
	pluggedOnYellow.resourceId = 7;

	VCRDrawInfo pluggedOnBlack;
	pluggedOnBlack.resourceId = 10;

	updateJack(kRed, onTable, pluggedOnRed, pluggedOnYellow, pluggedOnBlack, 25);
}

void BlowUpPuzzleVCR::updateYellowJack() {
	VCRDrawInfo onTable;
	onTable.resourceId = 3;
	onTable.point = Common::Point(187, 439);

	VCRDrawInfo pluggedOnRed;
	pluggedOnRed.resourceId = 6;

	VCRDrawInfo pluggedOnYellow;
	pluggedOnYellow.resourceId = 9;

	VCRDrawInfo pluggedOnBlack;
	pluggedOnBlack.resourceId = 12;

	updateJack(kYellow, onTable, pluggedOnRed, pluggedOnYellow, pluggedOnBlack, 26);
}


// common function to set and unset the jack on holes for each type of jack
int BlowUpPuzzleVCR::setJackOnHole(int jackType, JackState plugged) {
	if (!_holesState[plugged-1]) {
		if (_jacksState[jackType-1] == kOnHand) {
			_jacksState[jackType-1] = plugged;
			_holesState[plugged-1] = jackType; // set jack on red
			playSound(getWorld()->graphicResourceIds[44]);
		}
	} else if (jackType == 0) {
		jackType = _holesState[plugged-1];
		_jacksState[jackType-1] = kOnHand;
		_holesState[plugged-1] = 0;
		playSound(getWorld()->graphicResourceIds[43]);
		return 0;
	}
	return 1;
}

void BlowUpPuzzleVCR::updateButton(Button button, const VCRDrawInfo &btON, const VCRDrawInfo &btDown) {
	GraphicQueueItem item;

	switch (_buttonsState[button]) {
	case kON:
		item.resourceId = getWorld()->graphicResourceIds[btON.resourceId];
		item.frameIndex = 0;
		item.point = btON.point;
		item.priority = 3;
		break;
	case kDownON:
	case kDownOFF:
		item.resourceId = getWorld()->graphicResourceIds[btDown.resourceId];
		item.frameIndex = 0;
		item.point = btDown.point;
		item.priority = 3;
		break;
	default:
		item.resourceId = kResourceNone;
		break;
	}

	if (item.resourceId != 0)
		getScreen()->addGraphicToQueue(item);

}

void BlowUpPuzzleVCR::updatePowerButton() {
	VCRDrawInfo btON;
	btON.resourceId = 17;
	btON.point = Common::Point(512, 347);

	VCRDrawInfo btDown;
	btDown.resourceId = 21;
	btDown.point = Common::Point(506, 343);

	updateButton(kPower, btON, btDown);
}

void BlowUpPuzzleVCR::updateRewindButton() {
	VCRDrawInfo btON;
	btON.resourceId = 14;
	btON.point = Common::Point(248, 347);

	VCRDrawInfo btDown;
	btDown.resourceId = 18;
	btDown.point = Common::Point(245, 344);

	updateButton(kRewind, btON, btDown);
}

void BlowUpPuzzleVCR::updatePlayButton() {
	VCRDrawInfo btON;
	btON.resourceId = 16;
	btON.point = Common::Point(401, 359);

	VCRDrawInfo btDown;
	btDown.resourceId = 20;
	btDown.point = Common::Point(391, 355);

	updateButton(kPlay, btON, btDown);
}

void BlowUpPuzzleVCR::updateStopButton() {
	VCRDrawInfo btON;
	btON.resourceId = 15;
	btON.point = Common::Point(330, 354);

	VCRDrawInfo btDown;
	btDown.resourceId = 19;
	btDown.point = Common::Point(326, 350);

	updateButton(kStop, btON, btDown);
}


void BlowUpPuzzleVCR::updateCursorInPolyRegion() {
	int showCursor = 0;

	if (_jacksState[kBlack] == kOnHand) {
		showCursor = kBlack + 1;
	} else if (_jacksState[kRed] == kOnHand) {
		showCursor = kRed + 1;
	} else {
		showCursor = ((_jacksState[kYellow] != kOnHand) - 1) & 3;
	}

	if (!showCursor) {
		if (inPolyRegion(_cursor->position().x, _cursor->position().y, kRewindButton)
		 || inPolyRegion(_cursor->position().x, _cursor->position().y, kStopButton)
		 || inPolyRegion(_cursor->position().x, _cursor->position().y, kPlayButton)
		 || inPolyRegion(_cursor->position().x, _cursor->position().y, kPowerButton)
		 || inPolyRegion(_cursor->position().x, _cursor->position().y, kBlackJack)
		 || inPolyRegion(_cursor->position().x, _cursor->position().y, kRedJack)
		 || inPolyRegion(_cursor->position().x, _cursor->position().y, kYellowJack)) {
			_cursor->animate();
		} else {
			if ((inPolyRegion(_cursor->position().x, _cursor->position().y, kRedHole) && _holesState[kOnTable])
			 || (inPolyRegion(_cursor->position().x, _cursor->position().y, kYellowHole) && _holesState[kPluggedOnRed])
			 || (inPolyRegion(_cursor->position().x, _cursor->position().y, kBlackHole) && _holesState[kPluggedOnYellow])) {
				if (_cursor->currentFrame != 2) { // reset cursor
					_cursor->show();
					_cursor->set(MAKE_RESOURCE(kResourcePackShared, 2), 0, 2);
					_cursor->animate();
				}
			} else {
				if (_cursor->currentFrame != 0) { // reset cursor
					_cursor->show();
					_cursor->set(MAKE_RESOURCE(kResourcePackShared, 0), 0, 2);
					_cursor->animate();
				}
			}
		}
	} else {
		_cursor->hide();
	}
}

void BlowUpPuzzleVCR::handleMouseDown() {

	if (_isAccomplished)
		return;

	int jackType = 0;
	if (_jacksState[kBlack] == kOnHand) {
		jackType = kBlack + 1;
	} else if (_jacksState[kRed] == kOnHand) {
		jackType = kRed + 1;
	} else {
		jackType = ((_jacksState[kYellow] != kOnHand) - 1) & 3;
	}

	// Plug-in jacks
	if (inPolyRegion(_cursor->position().x, _cursor->position().y, kRedHole)) {
		if (!setJackOnHole(jackType, kPluggedOnRed)) {
			return;
		}
	}
	if (inPolyRegion(_cursor->position().x, _cursor->position().y, kYellowHole)) {
		if (!setJackOnHole(jackType, kPluggedOnYellow)) {
			return;
		}
	}
	if (inPolyRegion(_cursor->position().x, _cursor->position().y, kBlackHole)) {
		if (!setJackOnHole(jackType, kPluggedOnBlack)) {
			// TODO: this will probably segfault!
			error("[BlowUpPuzzleVCR::handleMouseDown] Not implemented!");
			/*if (_holesState[kBlackHole] != kPluggedOnYellow && _buttonsState[kPower] == 1) {
				_buttonsState[kPower]  = kOFF;
				_buttonsState[kStop]   = kOFF;
				_buttonsState[kPlay]   = kOFF;
				_buttonsState[kRewind] = kOFF;
			}
			return;*/
		}
	}

	// Put jacks on table --
	if (jackType) {
		if (_cursor->position().x >= (int32)BlowUpPuzzleVCRPolies[kBlackJack].left && _cursor->position().x <= (int32)BlowUpPuzzleVCRPolies[kYellowJack].right
		 && _cursor->position().y >= (int32)BlowUpPuzzleVCRPolies[kBlackJack].top  && _cursor->position().y <= (int32)BlowUpPuzzleVCRPolies[kYellowJack].bottom) {

			_jacksState[jackType-1] = kOnTable;
			playSound(getWorld()->graphicResourceIds[50]);
			_cursor->show();
		}
		return;
	}

	// Get Jacks from Table
	if (inPolyRegion(_cursor->position().x, _cursor->position().y, kBlackJack)) {
		_jacksState[kBlack] = kOnHand;
	} else if (inPolyRegion(_cursor->position().x, _cursor->position().y, kRedJack)) {
		_jacksState[kRed] = kOnHand;
	} else if (inPolyRegion(_cursor->position().x, _cursor->position().y, kYellowJack)) {
		_jacksState[kYellow] = kOnHand;
	}

	// VCR button regions
	if (inPolyRegion(_cursor->position().x, _cursor->position().y, kRewindButton)) {
		playSound(getWorld()->graphicResourceIds[39]);
		if (!_buttonsState[kRewind]) {
			_buttonsState[kRewind] = kDownON;
			return;
		}
		if (_buttonsState[kRewind] == kON) {
			_buttonsState[kRewind] = kDownOFF;
			return;
		}
	} else if (inPolyRegion(_cursor->position().x, _cursor->position().y, kPlayButton)) {
		playSound(getWorld()->graphicResourceIds[39]);
		if (!_buttonsState[kPlay]) {
			_buttonsState[kPlay] = kDownON;
			return;
		}
		if (_buttonsState[kPlay] == kON) {
			_buttonsState[kPlay] = kDownOFF;
			return;
		}
	} else if (inPolyRegion(_cursor->position().x, _cursor->position().y, kStopButton)) {
		playSound(getWorld()->graphicResourceIds[39]);
		if (_buttonsState[kStop]) {
			if (_buttonsState[kStop] == kON) {
				_buttonsState[kStop] = kDownOFF;
				return;
			}
		} else {
			_buttonsState[kStop] = kDownON;
			return;
		}
	} else if (inPolyRegion(_cursor->position().x, _cursor->position().y, kPowerButton)) {
		playSound(getWorld()->graphicResourceIds[39]);

		if (!_buttonsState[kPower] && _holesState[kPluggedOnYellow] == kRed && _holesState[kOnTable] && _holesState[kPluggedOnRed]) {
			_buttonsState[kPower] = kDownON;
		} else {
			_buttonsState[kPower] = kDownOFF;
		}
	}
}

void BlowUpPuzzleVCR::handleMouseUp() {
	if (_isAccomplished)
		return;

	if (_buttonsState[kPower] == kDownON) {
		playSound(getWorld()->graphicResourceIds[47], true);
		_buttonsState[kPower]  = kON;
		_buttonsState[kStop]   = kON;
		_buttonsState[kPlay]   = kON;
		_buttonsState[kRewind] = kON;
	} else if (_buttonsState[kPower] == kDownOFF) {
		_buttonsState[kPower]  = kOFF;
		_buttonsState[kStop]   = kOFF;
		_buttonsState[kPlay]   = kOFF;
		_buttonsState[kRewind] = kOFF;
		getSound()->stopAll();
	}

	if (_buttonsState[kRewind] == kDownOFF) {
		_buttonsState[kRewind] = kON;
		playSound(getWorld()->graphicResourceIds[46]);
	} else if (_buttonsState[kRewind] == kDownON) {
		_buttonsState[kRewind] = kOFF;
	}

	if (_buttonsState[kPlay] == kDownOFF) {
		_buttonsState[kPlay] = kON;
		if (_holesState[kOnTable] == kYellow && _holesState[kPluggedOnRed] == kYellow + 1 /*FIXME this is not a proper value */ && _holesState[kPluggedOnYellow] == kRed) {
			_vm->setGameFlag(kGameFlagSolveVCRBlowUpPuzzle);
			_isAccomplished = true;
		}
	} else if (_buttonsState[kPlay] == kDownON) {
		_buttonsState[kPlay] = kOFF;
	}

	if (_buttonsState[kStop] == kDownOFF) {
		_buttonsState[kStop] = kON;
		return;
	}
	if (_buttonsState[kStop] == kDownON) {
		_buttonsState[kStop] = kOFF;
	}
}

} // end of namespace Asylum
