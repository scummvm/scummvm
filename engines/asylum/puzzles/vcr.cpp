/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/puzzles/vcr.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"
#include "asylum/views/video.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

namespace Asylum {

PuzzleVCR::PuzzleVCR(AsylumEngine *engine): Puzzle(engine) {
	// reset all states
	memset(&_jacksState,   0, sizeof(_jacksState));
	memset(&_holesState,   0, sizeof(_holesState));
	memset(&_buttonsState, 0, sizeof(_buttonsState));

	_tvScreenFrameIndex = 0;
	_isAccomplished  = false;
}

PuzzleVCR::~PuzzleVCR() {
}

void PuzzleVCR::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsUint32LE(_buttonsState[kPowerButton]);
	s.syncAsUint32LE(_buttonsState[kRewindButton]);
	s.syncAsUint32LE(_buttonsState[kPlayButton]);
	s.syncAsUint32LE(_buttonsState[kStopButton]);

	s.syncAsUint32LE(_holesState[kBlack]);
	s.syncAsUint32LE(_holesState[kRed]);
	s.syncAsUint32LE(_holesState[kYellow]);


	s.syncAsUint32LE(_jacksState[kBlack]);
	s.syncAsUint32LE(_jacksState[kRed]);
	s.syncAsUint32LE(_jacksState[kYellow]);

	s.syncAsUint32LE(_tvScreenFrameIndex);
	s.syncAsUint32LE(_isAccomplished);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleVCR::init(const AsylumEvent &)  {
	// Load the graphics palette
	getScreen()->setPalette(getWorld()->graphicResourceIds[29]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[29]);

	if (_jacksState[kBlack] == kOnHand || _jacksState[kRed] == kOnHand || _jacksState[kYellow] == kOnHand) {
		getCursor()->hide();
		getSharedData()->setFlag(kFlag1, true);
	} else {
		getCursor()->set(getWorld()->graphicResourceIds[28]);
	}

	return true;
}

bool PuzzleVCR::key(const AsylumEvent &evt) {
	getSound()->stop(getWorld()->graphicResourceIds[47]);
	getScreen()->clearGraphicsInQueue();
	getScreen()->clear();

	_vm->switchEventHandler(getScene());

	return true;
}

bool PuzzleVCR::mouseLeftDown(const AsylumEvent &evt) {
	if (_isAccomplished)
		return true;

	//////////////////////////////////////////////////////////////////////////
	// Plug-in jacks
	//////////////////////////////////////////////////////////////////////////
	JackState state = kPluggedOnRed;
	if (_jacksState[kBlack] != kOnHand) {
		if (_jacksState[kRed] == kOnHand)
			state = kPluggedOnYellow;
		else
			state = (_jacksState[kYellow] != kOnHand) ? kOnTable : kPluggedOnBlack;
	}

	if (inPolygon(evt.mouse, kRedHole)) {
		setJackOnHole(kBlack, state, kPluggedOnRed);
	} else if (inPolygon(evt.mouse, kYellowHole)) {
		setJackOnHole(kRed, state, kPluggedOnYellow);
	} else if (inPolygon(evt.mouse, kBlackHole)) {
		setJackOnHole(kYellow, state, kPluggedOnBlack);

		if (_holesState[kYellow] != kPluggedOnYellow && _buttonsState[kPowerButton] == kON) {
			_buttonsState[kStopButton] = kOFF;
			_buttonsState[kPlayButton] = kOFF;
			_buttonsState[kRewindButton] = kOFF;
			_buttonsState[kPowerButton] = kOFF;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Put jacks on table
	//////////////////////////////////////////////////////////////////////////
	Color jack = getJackOnHand();
	if (jack != kNone) {
		if (evt.mouse.x >= (int32)puzzleVCRPolygons[kBlackJack][0] && evt.mouse.x <= (int32)puzzleVCRPolygons[kYellowJack][2]
		 && evt.mouse.y >= (int32)puzzleVCRPolygons[kBlackJack][1] && evt.mouse.y <= (int32)puzzleVCRPolygons[kYellowJack][3]) {

			_jacksState[jack] = kOnTable;
			getSound()->playSound(getWorld()->graphicResourceIds[50]);
			getCursor()->show();
			getSharedData()->setFlag(kFlag1, false);
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Get Jacks from Table
	//////////////////////////////////////////////////////////////////////////
	getCursor()->show();
	getSharedData()->setFlag(kFlag1, false);


	if (inPolygon(evt.mouse, kBlackJack))
		pickJack(kBlack);
	else if (inPolygon(evt.mouse, kRedJack))
		pickJack(kRed);
	else if (inPolygon(evt.mouse, kYellowJack))
		pickJack(kYellow);

	//////////////////////////////////////////////////////////////////////////
	// VCR button regions
	//////////////////////////////////////////////////////////////////////////
	if (inPolygon(evt.mouse, kRewindButton)) {
		getSound()->playSound(getWorld()->graphicResourceIds[39]);

		if (!_buttonsState[kRewindButton])
			_buttonsState[kRewindButton] = kDownON;
		else if (_buttonsState[kRewindButton] == kON)
			_buttonsState[kRewindButton] = kDownOFF;

	} else if (inPolygon(evt.mouse, kPlayButton)) {
		getSound()->playSound(getWorld()->graphicResourceIds[39]);

		if (!_buttonsState[kPlayButton])
			_buttonsState[kPlayButton] = kDownON;
		else if (_buttonsState[kPlayButton] == kON)
			_buttonsState[kPlayButton] = kDownOFF;

	} else if (inPolygon(evt.mouse, kStopButton)) {
		getSound()->playSound(getWorld()->graphicResourceIds[39]);

		if (!_buttonsState[kStopButton])
			_buttonsState[kStopButton] = kDownON;
		else if (_buttonsState[kStopButton] == kON)
			_buttonsState[kStopButton] = kDownOFF;

	} else if (inPolygon(evt.mouse, kPowerButton)) {
		getSound()->playSound(getWorld()->graphicResourceIds[39]);

		if (!_buttonsState[kPowerButton] && _holesState[kYellow] == kPluggedOnRed) {
			_buttonsState[kPowerButton] = kDownON;
		} else {
			_buttonsState[kPowerButton] = kDownOFF;
		}
	}

	return true;
}

bool PuzzleVCR::mouseLeftUp(const AsylumEvent &) {
	if (_isAccomplished)
		return true;

	//////////////////////////////////////////////////////////////////////////
	// Power
	//////////////////////////////////////////////////////////////////////////
	if (_buttonsState[kPowerButton] == kDownON) {
		if (!getSound()->isPlaying(getWorld()->graphicResourceIds[47]))
			getSound()->playSound(getWorld()->graphicResourceIds[47], true);

		_buttonsState[kPowerButton]  = kON;
		_buttonsState[kStopButton]   = kON;
		_buttonsState[kPlayButton]   = kON;
		_buttonsState[kRewindButton] = kON;
	} else if (_buttonsState[kPowerButton] == kDownOFF) {
		_buttonsState[kPowerButton]  = kOFF;
		_buttonsState[kStopButton]   = kOFF;
		_buttonsState[kPlayButton]   = kOFF;
		_buttonsState[kRewindButton] = kOFF;

		getSound()->stop(getWorld()->graphicResourceIds[47]);
	}

	//////////////////////////////////////////////////////////////////////////
	// Rewind
	//////////////////////////////////////////////////////////////////////////
	if (_buttonsState[kRewindButton] == kDownOFF) {
		getSound()->playSound(getWorld()->graphicResourceIds[46]);
		_buttonsState[kRewindButton] = kON;
	} else if (_buttonsState[kRewindButton] == kDownON) {
		_buttonsState[kRewindButton] = kOFF;
	}

	//////////////////////////////////////////////////////////////////////////
	// Play
	//////////////////////////////////////////////////////////////////////////
	if (_buttonsState[kPlayButton] == kDownOFF) {
		_buttonsState[kPlayButton] = kON;

		if (_holesState[kBlack] == kPluggedOnYellow
		 && _holesState[kRed] == kPluggedOnBlack
		 && _holesState[kYellow] == kPluggedOnRed) {
			getCursor()->hide();
			_vm->setGameFlag(kGameFlagSolveVCRPuzzle);
			_isAccomplished = true;
		}
	} else if (_buttonsState[kPlayButton] == kDownON) {
		_buttonsState[kPlayButton] = kOFF;
	}

	//////////////////////////////////////////////////////////////////////////
	// Stop
	//////////////////////////////////////////////////////////////////////////
	if (_buttonsState[kStopButton] == kDownOFF) {
		_buttonsState[kStopButton] = kON;
	} else if (_buttonsState[kStopButton] == kDownON) {
		_buttonsState[kStopButton] = kOFF;
	}

	return true;
}

bool PuzzleVCR::mouseRightDown(const AsylumEvent &) {
	getScreen()->clearGraphicsInQueue();
	getScreen()->clear();

	getSound()->stop(getWorld()->graphicResourceIds[47]);

	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Drawing
//////////////////////////////////////////////////////////////////////////
void PuzzleVCR::updateScreen() {
	// Draw background
	getScreen()->clearGraphicsInQueue();
	getScreen()->fillRect(0, 0, 640, 480, 252);
	getScreen()->draw(getWorld()->graphicResourceIds[0], 0, Common::Point(0, 0), kDrawFlagNone, true);

	updateBlackJack();
	updateRedJack();
	updateYellowJack();

	updatePowerButton();
	updateRewindButton();
	updatePlayButton();
	updateStopButton();

	if (_buttonsState[kPowerButton] == kON) {
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[22], _tvScreenFrameIndex, Common::Point(0, 37), kDrawFlagNone, 0, 1);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[23], _tvScreenFrameIndex++, Common::Point(238, 22), kDrawFlagNone, 0, 1);

		_tvScreenFrameIndex %= GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[22]);
	}

	if (_isAccomplished) {
		getCursor()->show();
		getScreen()->draw(getWorld()->graphicResourceIds[0]);

		getScreen()->clearDefaultColor();
		for (int16 barSize = 0; barSize < 84; barSize += 4) {
			getScreen()->drawWideScreenBars(barSize);
			_vm->_system->updateScreen();
		}

		// Palette fade
		getScreen()->paletteFade(0, 25, 10);
		getScreen()->clear();

		getScene()->updateScreen();
		getScreen()->drawWideScreenBars(82);
		getSound()->stop(getWorld()->graphicResourceIds[47]);
		getSound()->playMusic(kResourceNone, 0);
		getScreen()->clear();

		getVideo()->play(2, getScene());

		if (getWorld()->musicCurrentResourceIndex != kMusicStopped)
			getSound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, getWorld()->musicCurrentResourceIndex));

		getScreen()->paletteFade(0, 2, 1);
		getScreen()->clear();

		// setupPalette();
		getScreen()->setupPalette(nullptr, 0, 0);

		int paletteId = _vm->checkGameVersion("Demo") ? 20 : 28;
		getScreen()->setPalette(MAKE_RESOURCE(kResourcePackTowerCells, paletteId));
		getScreen()->setGammaLevel(MAKE_RESOURCE(kResourcePackTowerCells, paletteId));
	}
}

void PuzzleVCR::updateCursor() {
	Color jack = getJackOnHand();
	Common::Point mousePos = getCursor()->position();

	if (mousePos.x)
		mousePos.x = 465;

	if (jack == kNone) {
		if (inPolygon(mousePos, kRewindButton)
		 || inPolygon(mousePos, kStopButton)
		 || inPolygon(mousePos, kPlayButton)
		 || inPolygon(mousePos, kPowerButton)
		 || inPolygon(mousePos, kBlackJack)
		 || inPolygon(mousePos, kRedJack)
		 || inPolygon(mousePos, kYellowJack)) {
			if (getCursor()->getAnimation() != kCursorAnimationMirror)
				getCursor()->set(getWorld()->graphicResourceIds[28]);
		} else if ((inPolygon(mousePos, kRedHole) && _holesState[kBlack])
		        || (inPolygon(mousePos, kYellowHole) && _holesState[kRed])
		        || (inPolygon(mousePos, kBlackHole) && _holesState[kYellow])) {
			if (getCursor()->getAnimation() != kCursorAnimationMirror)
				getCursor()->set(getWorld()->graphicResourceIds[28]);
		} else {
			if (getCursor()->getAnimation())
				getCursor()->set(getWorld()->graphicResourceIds[28], kCursorAnimationNone);
		}
	} else {
		getCursor()->hide();
	}
}

//////////////////////////////////////////////////////////////////////////
// Update Jack
//////////////////////////////////////////////////////////////////////////
void PuzzleVCR::updateJack(Color jack, const VCRDrawInfo &onTable, const VCRDrawInfo &pluggedOnRed, const VCRDrawInfo &pluggedOnYellow, const VCRDrawInfo &pluggedOnBlack, int32 resourceOnHandIndex) {
	GraphicQueueItem item;
	Common::Point mousePos = getCursor()->position();

	switch (_jacksState[jack]) {
	default:
		return;

	case kOnTable:
		item.resourceId = getWorld()->graphicResourceIds[onTable.resourceId];
		item.source = onTable.point;
		item.priority = 3;
		break;

	case kPluggedOnRed:
		item.resourceId = getWorld()->graphicResourceIds[pluggedOnRed.resourceId];
		item.source = Common::Point(329, 407);
		item.priority = 3;
		break;

	case kPluggedOnYellow:
		item.resourceId = getWorld()->graphicResourceIds[pluggedOnYellow.resourceId];
		item.source = Common::Point(402, 413);
		item.priority = 3;
		break;

	case kPluggedOnBlack:
		item.resourceId = getWorld()->graphicResourceIds[pluggedOnBlack.resourceId];
		item.source = Common::Point(477, 418);
		item.priority = 3;
		break;

	case kOnHand:
		// Jack
		item.resourceId = getWorld()->graphicResourceIds[resourceOnHandIndex];
		item.source =  Common::Point(mousePos.x - 114, mousePos.y < 356 ? 342 : mousePos.y - 14);
		item.priority = 1;
		getScreen()->addGraphicToQueue(item);

		// Shadow
		item.resourceId = getWorld()->graphicResourceIds[30];
		item.source = Common::Point(mousePos.x - (mousePos.y < 356 ? 0 : (mousePos.y - 356) / 4), 450);
		item.priority = 2;
		break;
	}

	getScreen()->addGraphicToQueue(item);
}

void PuzzleVCR::updateBlackJack() {
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

void PuzzleVCR::updateRedJack() {
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

void PuzzleVCR::updateYellowJack() {
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

//////////////////////////////////////////////////////////////////////////
// Update Button
//////////////////////////////////////////////////////////////////////////
void PuzzleVCR::updateButton(VCRRegions button, const VCRDrawInfo &btON, const VCRDrawInfo &btDown) {
	GraphicQueueItem item;

	switch (_buttonsState[button]) {
	default:
		return;

	case kON:
		item.resourceId = getWorld()->graphicResourceIds[btON.resourceId];
		item.source = btON.point;
		item.priority = 3;
		break;

	case kDownON:
	case kDownOFF:
		item.resourceId = getWorld()->graphicResourceIds[btDown.resourceId];
		item.source = btDown.point;
		item.priority = 3;
		break;
	}

	getScreen()->addGraphicToQueue(item);
}

void PuzzleVCR::updatePowerButton() {
	VCRDrawInfo btON;
	btON.resourceId = 17;
	btON.point = Common::Point(512, 347);

	VCRDrawInfo btDown;
	btDown.resourceId = 21;
	btDown.point = Common::Point(506, 343);

	updateButton(kPowerButton, btON, btDown);
}

void PuzzleVCR::updateRewindButton() {
	VCRDrawInfo btON;
	btON.resourceId = 14;
	btON.point = Common::Point(248, 347);

	VCRDrawInfo btDown;
	btDown.resourceId = 18;
	btDown.point = Common::Point(245, 344);

	updateButton(kRewindButton, btON, btDown);
}

void PuzzleVCR::updatePlayButton() {
	VCRDrawInfo btON;
	btON.resourceId = 16;
	btON.point = Common::Point(401, 359);

	VCRDrawInfo btDown;
	btDown.resourceId = 20;
	btDown.point = Common::Point(391, 355);

	updateButton(kPlayButton, btON, btDown);
}

void PuzzleVCR::updateStopButton() {
	VCRDrawInfo btON;
	btON.resourceId = 15;
	btON.point = Common::Point(330, 354);

	VCRDrawInfo btDown;
	btDown.resourceId = 19;
	btDown.point = Common::Point(326, 350);

	updateButton(kStopButton, btON, btDown);
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
int PuzzleVCR::inPolygon(const Common::Point &point, int polygonIndex) const {
	return _vm->rectContains(&puzzleVCRPolygons[polygonIndex], point);
}

PuzzleVCR::Color PuzzleVCR::getJackOnHand() const {
	Color jack = kNone;
	if (_jacksState[kBlack] == kOnHand)
		jack = kBlack;
	else if (_jacksState[kRed] == kOnHand)
		jack = kRed;
	else if (_jacksState[kYellow] == kOnHand)
		jack = kYellow;

	return jack;
}

void PuzzleVCR::setJackOnHole(Color hole, JackState state, JackState newState) {
	bool isYellow = (hole == kYellow);

	if (_holesState[hole]) {
		if (isYellow)
			getSound()->stop(getWorld()->graphicResourceIds[47]);

		_jacksState[_holesState[hole] - 1] = kOnHand;
		_holesState[hole] = kOnTable;

		if (state != kOnTable) {
			getSound()->playSound(getWorld()->graphicResourceIds[44]);
			_holesState[hole] = state;

			if (isYellow) {
				if (state != kPluggedOnYellow && _buttonsState[kPowerButton] == kON)
					getSound()->stop(getWorld()->graphicResourceIds[47]);
			}

			_jacksState[state - 1] = newState;
		} else {
			getSound()->playSound(getWorld()->graphicResourceIds[43]);
			getCursor()->hide();
			getSharedData()->setFlag(kFlag1, true);
		}
	} else if (state != kOnTable) {
		getSound()->playSound(getWorld()->graphicResourceIds[44]);
		_holesState[hole] = state;

		if (isYellow) {
			if (state != kPluggedOnYellow && _buttonsState[kPowerButton] == kON)
				getSound()->stop(getWorld()->graphicResourceIds[47]);
		}

		_jacksState[state - 1] = newState;
	}
}

void PuzzleVCR::pickJack(Color jack) {
	getCursor()->hide();
	getSharedData()->setFlag(kFlag1, true);
	_jacksState[jack] = kOnHand;
}

} // end of namespace Asylum
