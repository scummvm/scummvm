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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/ui/kia_section_crimes.h"

#include "bladerunner/actor_clues.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/crimes_database.h"
#include "bladerunner/font.h"
#include "bladerunner/game_flags.h"
#include "bladerunner/game_info.h"
#include "bladerunner/shape.h"
#include "bladerunner/script/kia_script.h"
#include "bladerunner/suspects_database.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/kia_log.h"
#include "bladerunner/ui/kia_section_suspects.h"
#include "bladerunner/ui/kia_shapes.h"
#include "bladerunner/ui/ui_container.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/ui/ui_scroll_box.h"

#include "graphics/surface.h"

namespace BladeRunner {

KIASectionCrimes::KIASectionCrimes(BladeRunnerEngine *vm, ActorClues *clues) : KIASectionBase(vm) {
	_uiContainer = new UIContainer(_vm);
	_isOpen = false;
	_clues = clues;

	_mouseX = 0;
	_mouseY = 0;

	_buttons = new UIImagePicker(_vm, 5);

	_cluesScrollBox = new UIScrollBox(_vm, scrollBoxCallback, this, 50, 1, false, Common::Rect(312, 172, 500, 376), Common::Rect(506, 160, 506, 394));
	_uiContainer->add(_cluesScrollBox);

	_acquiredClueCount = 0;
	for (int i = 0; i < kClueCount; ++i) {
		_acquiredClues[i].clueId  = -1;
		_acquiredClues[i].actorId = -1;
	}

	_crimeSelected = -1;
	_crimesFoundCount = 0;
	_crimesFound.resize(_vm->_gameInfo->getCrimeCount());

	_suspectSelected = -1;
	_suspectPhotoShapeId = -1;
	_suspectPhotoNotUsed = -1;
	_suspectPhotoShape = nullptr;
	_suspectsFoundCount = 0;
	_suspectsFound.resize(_vm->_gameInfo->getSuspectCount());
	_suspectsWithIdentity.resize(_vm->_gameInfo->getSuspectCount());
}

KIASectionCrimes::~KIASectionCrimes() {
	delete _suspectPhotoShape;

	_uiContainer->clear();

	delete _cluesScrollBox;
	delete _buttons;
	delete _uiContainer;
}

void KIASectionCrimes::reset() {
	_acquiredClueCount = 0;
    _crimesFoundCount = 0;
    _suspectsFoundCount = 0;
    _mouseX = 0;
    _mouseY = 0;
    _suspectSelected = -1;
    _crimeSelected = -1;
    _suspectPhotoShapeId = -1;
    _suspectPhotoNotUsed = -1;
}

void KIASectionCrimes::open() {
	_scheduledSwitch = false;

	_buttons->resetImages();
	_buttons->defineImage(0, Common::Rect(136, 326, 185, 342), nullptr, _vm->_kia->_shapes->get(32), _vm->_kia->_shapes->get(36), _vm->_textKIA->getText(32));
	_buttons->defineImage(1, Common::Rect(218, 326, 269, 342), nullptr, _vm->_kia->_shapes->get(33), _vm->_kia->_shapes->get(37), _vm->_textKIA->getText(33));
	_buttons->defineImage(2, Common::Rect(354, 128, 404, 144), nullptr, _vm->_kia->_shapes->get(30), _vm->_kia->_shapes->get(34), _vm->_textKIA->getText(34));
	_buttons->defineImage(3, Common::Rect(425, 128, 474, 144), nullptr, _vm->_kia->_shapes->get(31), _vm->_kia->_shapes->get(35), _vm->_textKIA->getText(35));
	_buttons->defineImage(4, Common::Rect(142, 150, 260, 297), nullptr, nullptr, nullptr, _vm->_textKIA->getText(36));
	_buttons->activate(nullptr, nullptr, nullptr, mouseUpCallback, this);

	_cluesScrollBox->show();

	populateAcquiredClues();
	populateCrimes();
	populateSuspects();
	populateVisibleClues();
	updateSuspectPhoto();

	_isOpen = true;
}

void KIASectionCrimes::close() {
	if (!_isOpen) {
		return;
	}
	_isOpen = false;
	_buttons->deactivate();
	_cluesScrollBox->hide();
	if (_suspectPhotoShapeId != -1) {
		delete _suspectPhotoShape;
		_suspectPhotoShape = nullptr;
		_suspectPhotoShapeId = -1;
	}
}

void KIASectionCrimes::draw(Graphics::Surface &surface) {
	const char *text = nullptr;
	if (_suspectPhotoShapeId != -1) {
		_suspectPhotoShape->draw(surface, 201 - _suspectPhotoShape->getWidth() / 2, 223 - _suspectPhotoShape->getHeight() / 2);
	}
	if (_suspectPhotoShapeId == 14 || _suspectPhotoShapeId == 13) {
		text = _vm->_textKIA->getText(49);
		_vm->_mainFont->drawString(&surface, text, 201 - _vm->_mainFont->getStringWidth(text) / 2, 218, surface.w, surface.format.RGBToColor(255, 255, 255));
	}

	surface.fillRect(Common::Rect(120, 134, 250, 145), 0);
	surface.hLine(120, 133, 250, surface.format.RGBToColor(48, 40, 40));
	surface.hLine(120, 146, 250, surface.format.RGBToColor(88, 80, 96));
	surface.vLine(119, 134, 145, surface.format.RGBToColor(48, 40, 40));
	surface.vLine(251, 134, 145, surface.format.RGBToColor(88, 80, 96));
	surface.hLine(251, 146, 251, surface.format.RGBToColor(72, 64, 72));

	if (_crimeSelected == -1) {
		text = _vm->_textKIA->getText(49);
	} else {
		text = _vm->_textCrimes->getText(_crimeSelected);
	}

	_vm->_mainFont->drawString(&surface, text, 185 - _vm->_mainFont->getStringWidth(text) / 2, 136, surface.w, surface.format.RGBToColor(136, 168, 255));

	surface.fillRect(Common::Rect(136, 304, 266, 315), 0);
	surface.hLine(136, 303, 266, surface.format.RGBToColor(48, 40, 40));
	surface.hLine(136, 316, 266, surface.format.RGBToColor(88, 80, 96));
	surface.vLine(135, 304, 315, surface.format.RGBToColor(48, 40, 40));
	surface.vLine(267, 304, 315, surface.format.RGBToColor(88, 80, 96));
	surface.hLine(267, 316, 267, surface.format.RGBToColor(72, 64, 72));

	char generatedText[64];
	if (_suspectSelected == -1) {
		text = _vm->_textKIA->getText(22);
	} else {
		const char *suspectName = _vm->_suspectsDatabase->get(_suspectSelected)->getName();
		if (_suspectsWithIdentity[_suspectSelected]) {
			text = suspectName;
		} else if (_vm->_suspectsDatabase->get(_suspectSelected)->getSex()) {
			sprintf(generatedText, "%s %s", _vm->_textKIA->getText(20), _vm->_kia->scrambleSuspectsName(suspectName));
			text = generatedText;
		} else {
			sprintf(generatedText, "%s %s", _vm->_textKIA->getText(21), _vm->_kia->scrambleSuspectsName(suspectName));
			text = generatedText;
		}
	}
	_vm->_mainFont->drawString(&surface, text, 201 - _vm->_mainFont->getStringWidth(text) / 2, 306, surface.w, surface.format.RGBToColor(136, 168, 255));

	_uiContainer->draw(surface);
	_buttons->draw(surface);
	_buttons->drawTooltip(surface, _mouseX, _mouseY);
}

void KIASectionCrimes::handleMouseMove(int mouseX, int mouseY) {
	_mouseX = mouseX;
	_mouseY = mouseY;
	_buttons->handleMouseAction(mouseX, mouseY, false, false, false);
	_uiContainer->handleMouseMove(mouseX, mouseY);
}

void KIASectionCrimes::handleMouseDown(bool mainButton) {
	if (mainButton) {
		_buttons->handleMouseAction(_mouseX, _mouseY, true, false, false);
	}
	_uiContainer->handleMouseDown(!mainButton);
}

void KIASectionCrimes::handleMouseUp(bool mainButton) {
	if (mainButton) {
		_buttons->handleMouseAction(_mouseX, _mouseY, false, true, false);
	}
	_uiContainer->handleMouseUp(!mainButton);
}

void KIASectionCrimes::handleMouseScroll(int direction) {
	_uiContainer->handleMouseScroll(direction);
}

void KIASectionCrimes::saveToLog() {
	int data[] = { _crimeSelected, _suspectSelected };
	_vm->_kia->_log->add(2, sizeof(data), &data);
}

void KIASectionCrimes::loadFromLog() {
	const int *data = (const int*)_vm->_kia->_log->getCurrentData();
	_crimeSelected = data[0];
	_suspectSelected = data[1];
	populateSuspects();
	populateVisibleClues();
}

void KIASectionCrimes::selectCrime(int crimeId) {
	_crimeSelected = crimeId;
	populateSuspects();
	populateVisibleClues();
	updateSuspectPhoto();
}

void KIASectionCrimes::scrollBoxCallback(void *callbackData, void *source, int lineData, int mouseButton) {
	KIASectionCrimes *self = (KIASectionCrimes *)callbackData;

	if (source == self->_cluesScrollBox && lineData >= 0) {
		if (mouseButton) {
			if (self->_vm->_gameFlags->query(kFlagKIAPrivacyAddon)) {
				self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxBEEP15), 70, 0, 0, 50, 0);

				if (self->_clues->isPrivate(lineData)) {
					self->_clues->setPrivate(lineData, false);
					self->_cluesScrollBox->resetFlags(lineData, 0x08);
				} else {
					self->_clues->setPrivate(lineData, true);
					self->_cluesScrollBox->setFlags(lineData, 0x08);
				}
			}
		} else {
			self->_clues->setViewed(lineData, true);
			self->_cluesScrollBox->resetHighlight(lineData);
			self->_vm->_kia->_script->playClueAssetScript(0, lineData);
		}
	}
}

void KIASectionCrimes::mouseUpCallback(int buttonId, void *callbackData) {
	((KIASectionCrimes *)callbackData)->onButtonPressed(buttonId);
}

void KIASectionCrimes::onButtonPressed(int buttonId) {
	switch (buttonId) {
	case 0:
		prevSuspect();
		break;
	case 1:
		nextSuspect();
		break;
	case 2:
		prevCrime();
		break;
	case 3:
		nextCrime();
		break;
	case 4:
		if (_suspectSelected != -1) {
			_scheduledSwitch = true;
		}
		break;
	}
}

void KIASectionCrimes::populateAcquiredClues() {
	_acquiredClueCount = 0;
	for (int i = 0; i < kClueCount; ++i) {
		int clueId = i;
		if (_clues->isAcquired(clueId)) {
			_acquiredClues[_acquiredClueCount].clueId = clueId;
			_acquiredClues[_acquiredClueCount].actorId = _clues->getFromActorId(clueId);
			++_acquiredClueCount;
		}
	}
	// sort clues by name, is it necessary?
}

void KIASectionCrimes::populateCrimes() {
	int firstCrime = -1;
	int crimeCount = _vm->_gameInfo->getCrimeCount();
	for (int i = 0; i < crimeCount; ++i) {
		_crimesFound[i] = false;
	}

	_crimesFoundCount = 0;

	if (!_acquiredClueCount) {
		return;
	}

	for (int i = 0; i < crimeCount; ++i) {
		for (int j = 0; j < _acquiredClueCount; ++j) {
			if (_vm->_crimesDatabase->getCrime(_acquiredClues[j].clueId) == i) {
				if (firstCrime == -1) {
					firstCrime = i;
				}
				_crimesFound[i] = true;
				++_crimesFoundCount;
			}
		}
	}

	if (_crimesFoundCount > 0 && _crimeSelected == -1) {
		_crimeSelected = firstCrime;
	}
}

void KIASectionCrimes::populateSuspects() {
	int firstSuspect = -1;
	int suspectCount = _vm->_gameInfo->getSuspectCount();

	for (int i = 0; i < suspectCount; ++i) {
		_suspectsFound[i] = false;
		_suspectsWithIdentity[i] = false;
	}

	_suspectsFoundCount = 0;

	if (!_acquiredClueCount || _crimeSelected == -1) {
		return;
	}

	for (int i = 0; i < suspectCount; ++i) {
		for (int j = 0; j < _acquiredClueCount; ++j) {
			if (_vm->_crimesDatabase->getCrime(_acquiredClues[j].clueId) == _crimeSelected
			 && _vm->_suspectsDatabase->get(i)->hasClue(_acquiredClues[j].clueId)
			) {
				if (firstSuspect == -1) {
					firstSuspect = i;
				}
				_suspectsFound[i] = true;
				++_suspectsFoundCount;
			}
		}

		if (_suspectsFound[i]) {
			for (int j = 0; j < _acquiredClueCount; ++j) {
				if (_vm->_suspectsDatabase->get(i)->hasIdentityClue(_acquiredClues[j].clueId)) {
					_suspectsWithIdentity[i] = true;
				}
			}
		}
	}

	if (_suspectsFoundCount) {
		if (_suspectSelected == -1 || !_suspectsFound[_suspectSelected]) {
			_suspectSelected = firstSuspect;
		}
	} else {
		_suspectSelected = -1;
	}
}

void KIASectionCrimes::populateVisibleClues() {
	_cluesScrollBox->clearLines();
	if (_crimeSelected != -1) {
		for (int i = 0; i < kClueCount; ++i) {
			int clueId = i;
			if (_vm->_crimesDatabase->getAssetType(clueId) != -1
			 && _vm->_crimesDatabase->getCrime(clueId) == _crimeSelected
			 && _clues->isAcquired(clueId)
			) {
				int flags = 0x30;
				if (_clues->isPrivate(clueId)) {
					flags = 0x08;
				} else if (_clues->isViewed(clueId)) {
					flags = 0x10;
				}
				_cluesScrollBox->addLine(_vm->_crimesDatabase->getClueText(clueId), clueId, flags);
			}
		}
		_cluesScrollBox->sortLines();
	}
}

void KIASectionCrimes::updateSuspectPhoto() {
	if (_suspectPhotoShapeId != -1) {
		delete _suspectPhotoShape;
		_suspectPhotoShape = nullptr;
	}

	if (_suspectSelected == -1) {
		_suspectPhotoShapeId = -1;
		return;
	}

	SuspectDatabaseEntry *suspect = _vm->_suspectsDatabase->get(_suspectSelected);

	_suspectPhotoShapeId = -1;
	_suspectPhotoNotUsed = -1;
	int photoCluesCount = suspect->getPhotoCount();
	if (photoCluesCount > 0) {
		for (int i = 0 ; i < photoCluesCount; i++) {
			//TODO: weird stuff going on here... original game is using internal clue index instead id
			if (_clues->isAcquired(suspect->getPhotoClueId(i))) {
				_suspectPhotoShapeId = suspect->getPhotoShapeId(i);
				_suspectPhotoNotUsed = suspect->getPhotoNotUsed(i);
				break;
			}
		}
	}

	if (_suspectPhotoShapeId == -1 && _suspectPhotoNotUsed == -1) {
		if (suspect->getSex()) {
			_suspectPhotoShapeId = 14;
		} else {
			_suspectPhotoShapeId = 13;
		}
	}

	if (_suspectPhotoShapeId != -1) {
		_suspectPhotoShape = new Shape(_vm);
		_suspectPhotoShape->open("photos.shp", _suspectPhotoShapeId);
	}
}

void KIASectionCrimes::nextCrime() {
	if (_crimesFoundCount >= 2) {
		while (true) {
			++_crimeSelected;
			if (_crimeSelected >= (int)_vm->_gameInfo->getCrimeCount()) {
				_crimeSelected = 0;
			}

			if (_crimesFound[_crimeSelected]) {
				selectCrime(_crimeSelected);
				break;
			}
		}
	}
}

void KIASectionCrimes::prevCrime() {
	if (_crimesFoundCount >= 2) {
		while (true) {
			--_crimeSelected;
			if (_crimeSelected < 0) {
				_crimeSelected = _vm->_gameInfo->getCrimeCount() - 1;
			}

			if (_crimesFound[_crimeSelected]) {
				selectCrime(_crimeSelected);
				break;
			}
		}
	}
}

void KIASectionCrimes::nextSuspect() {
	if (_suspectsFoundCount >= 2) {
		while (true) {
			++_suspectSelected;
			if (_suspectSelected >= (int)_vm->_gameInfo->getSuspectCount()) {
				_suspectSelected = 0;
			}

			if (_suspectsFound[_suspectSelected]) {
				updateSuspectPhoto();
				break;
			}
		}
	}
}

void KIASectionCrimes::prevSuspect() {
	if (_suspectsFoundCount >= 2) {
		while (true) {
			--_suspectSelected;
			if (_suspectSelected < 0) {
				_suspectSelected = _vm->_gameInfo->getSuspectCount() - 1;
			}

			if (_suspectsFound[_suspectSelected]) {
				updateSuspectPhoto();
				break;
			}
		}
	}
}

} // End of namespace BladeRunner
