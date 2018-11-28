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

#include "bladerunner/ui/kia_section_suspects.h"

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
#include "bladerunner/ui/kia_shapes.h"
#include "bladerunner/ui/ui_check_box.h"
#include "bladerunner/ui/ui_container.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/ui/ui_scroll_box.h"

#include "graphics/surface.h"

namespace BladeRunner {

KIASectionSuspects::KIASectionSuspects(BladeRunnerEngine *vm, ActorClues *clues) : KIASectionBase(vm) {
	_uiContainer = new UIContainer(_vm);
	_isOpen = false;
	_clues = clues;

	_mouseX = 0;
	_mouseY = 0;

	_whereaboutsFilter  = true;
	_MOFilter           = true;
	_replicantFilter    = true;
	_nonReplicantFilter = true;
	_othersFilter       = true;

	_buttons = new UIImagePicker(_vm, 4);

	_whereaboutsCheckBox  = new UICheckBox(_vm, checkBoxCallback, this, Common::Rect(142, 318, 275, 328), 1, _whereaboutsFilter);
	_MOCheckBox           = new UICheckBox(_vm, checkBoxCallback, this, Common::Rect(142, 328, 275, 338), 1, _MOFilter);
	_replicantCheckBox    = new UICheckBox(_vm, checkBoxCallback, this, Common::Rect(142, 338, 275, 348), 1, _replicantFilter);
	_nonReplicantCheckBox = new UICheckBox(_vm, checkBoxCallback, this, Common::Rect(142, 348, 275, 358), 1, _nonReplicantFilter);
	_othersCheckBox       = new UICheckBox(_vm, checkBoxCallback, this, Common::Rect(142, 358, 275, 368), 1, _othersFilter);
	_cluesScrollBox       = new UIScrollBox(_vm, scrollBoxCallback, this,_vm->_gameInfo->getClueCount(), 1, false, Common::Rect(312, 172, 500, 376), Common::Rect(506, 160, 506, 394));
	_crimesScrollBox      = new UIScrollBox(_vm, scrollBoxCallback, this, 50, 1, false, Common::Rect(154, 258, 291, 298), Common::Rect(120, 249, 120, 297));
	_uiContainer->add(_whereaboutsCheckBox);
	_uiContainer->add(_MOCheckBox);
	_uiContainer->add(_replicantCheckBox);
	_uiContainer->add(_nonReplicantCheckBox);
	_uiContainer->add(_othersCheckBox);
	_uiContainer->add(_cluesScrollBox);
	_uiContainer->add(_crimesScrollBox);

	_acquiredClueCount = 0;
	for (int i = 0; i < kClueCount; ++i) {
		_acquiredClues[i].clueId  = -1;
		_acquiredClues[i].actorId = -1;
	}

	_crimeSelected = -1;

	_suspectSelected = -1;
	_suspectPhotoShapeId = -1;
	_suspectPhotoNotUsed = -1;
	_suspectPhotoShape = nullptr;
	_suspectsFoundCount = 0;
	_suspectsFound.resize(_vm->_gameInfo->getSuspectCount());
	_suspectsWithIdentity.resize(_vm->_gameInfo->getSuspectCount());
}

KIASectionSuspects::~KIASectionSuspects() {
	delete _suspectPhotoShape;

	_uiContainer->clear();

	delete _crimesScrollBox;
	delete _cluesScrollBox;
	delete _othersCheckBox;
	delete _nonReplicantCheckBox;
	delete _replicantCheckBox;
	delete _MOCheckBox;
	delete _whereaboutsCheckBox;
	delete _buttons;
	delete _uiContainer;
}

void KIASectionSuspects::reset() {
	_acquiredClueCount = 0;
	_suspectsFoundCount = 0;
	_mouseX = 0;
	_mouseY = 0;
	_suspectSelected = -1;
	_crimeSelected = -1;
	_suspectPhotoShapeId = -1;
	_suspectPhotoNotUsed = -1;
	_whereaboutsFilter  = true;
	_MOFilter = true;
	_replicantFilter = true;
	_nonReplicantFilter = true;
	_othersFilter = true;
}

void KIASectionSuspects::open() {
	_scheduledSwitch = false;

	_buttons->resetImages();
	_buttons->defineImage(0, Common::Rect(142, 380, 191, 395), _vm->_kia->_shapes->get(79), _vm->_kia->_shapes->get(80), _vm->_kia->_shapes->get(81), _vm->_textKIA->getText(30));
	_buttons->defineImage(1, Common::Rect(193, 380, 242, 395), _vm->_kia->_shapes->get(76), _vm->_kia->_shapes->get(77), _vm->_kia->_shapes->get(77), _vm->_textKIA->getText(31));
	_buttons->defineImage(2, Common::Rect(354, 128, 404, 144), nullptr, _vm->_kia->_shapes->get(30), _vm->_kia->_shapes->get(34), _vm->_textKIA->getText(32));
	_buttons->defineImage(3, Common::Rect(424, 128, 474, 144), nullptr, _vm->_kia->_shapes->get(31), _vm->_kia->_shapes->get(35), _vm->_textKIA->getText(33));
	_buttons->activate(nullptr, nullptr, nullptr, mouseUpCallback, this);

	_cluesScrollBox->show();
	_crimesScrollBox->show();
	_whereaboutsCheckBox->enable();
	_MOCheckBox->enable();
	_replicantCheckBox->enable();
	_nonReplicantCheckBox->enable();
	_othersCheckBox->enable();
	_cluesScrollBox->show();
	_crimesScrollBox->show();

	populateAcquiredClues();
	populateSuspects();
	populateCrimes();
	populateVisibleClues();
	updateSuspectPhoto();

	_isOpen = true;
}

void KIASectionSuspects::close() {
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

void KIASectionSuspects::draw(Graphics::Surface &surface) {
	const char *text = nullptr;
	if (_suspectPhotoShapeId != -1) {
		_suspectPhotoShape->draw(surface, 142, 150);
	}
	if (_suspectPhotoShapeId == 14 || _suspectPhotoShapeId == 13) {
		text = _vm->_textKIA->getText(49);
		_vm->_mainFont->drawColor(text, surface, 190 - _vm->_mainFont->getTextWidth(text) / 2, 201, 0x7FFF);
	}

	_whereaboutsCheckBox->setChecked(_whereaboutsFilter);
	_MOCheckBox->setChecked(_MOFilter);
	_replicantCheckBox->setChecked(_replicantFilter);
	_nonReplicantCheckBox->setChecked(_nonReplicantFilter);
	_othersCheckBox->setChecked(_othersFilter);

	_uiContainer->draw(surface);

	_vm->_mainFont->drawColor(_vm->_textKIA->getText(0),  surface, 300, 162, 0x77DF);
	_vm->_mainFont->drawColor(_vm->_textKIA->getText(46), surface, 142, 248, 0x77DF);
	_vm->_mainFont->drawColor(_vm->_textKIA->getText(47), surface, 142, 308, 0x77DF);
	_vm->_mainFont->drawColor(_vm->_textKIA->getText(14), surface, 154, 319, 0x25B3);
	_vm->_mainFont->drawColor(_vm->_textKIA->getText(15), surface, 154, 329, 0x31F7);
	_vm->_mainFont->drawColor(_vm->_textKIA->getText(16), surface, 154, 339, 0x3A5B);
	_vm->_mainFont->drawColor(_vm->_textKIA->getText(17), surface, 154, 349, 0x31F7);
	_vm->_mainFont->drawColor(_vm->_textKIA->getText(48), surface, 154, 359, 0x25B3);


	surface.fillRect(Common::Rect(120, 134, 250, 145), 0);
	surface.hLine(120, 133, 250, 0x18A5);
	surface.hLine(120, 146, 250, 0x2D4C);
	surface.vLine(119, 134, 145, 0x18A5);
	surface.vLine(251, 134, 145, 0x2D4C);
	surface.hLine(251, 146, 251, 0x2509);

	char generatedText[64];
	if (_suspectSelected == -1) {
		text = _vm->_textKIA->getText(22);
	} else {
		const char *suspectName = _vm->_suspectsDatabase->get(_suspectSelected)->getName();
		if (_suspectsWithIdentity[_suspectSelected]) {
			text = suspectName;
		} else if (_vm->_suspectsDatabase->get(_suspectSelected)->getSex()) {
			sprintf(generatedText, "%s %s", _vm->_textKIA->getText(20), scrambleSuspectsName(suspectName));
			text = generatedText;
		} else {
			sprintf(generatedText, "%s %s", _vm->_textKIA->getText(21), scrambleSuspectsName(suspectName));
			text = generatedText;
		}
	}

	_vm->_mainFont->drawColor(text, surface, 185 - _vm->_mainFont->getTextWidth(text) / 2, 136, 0x46BF);

	_buttons->draw(surface);
	_buttons->drawTooltip(surface, _mouseX, _mouseY);
}

void KIASectionSuspects::handleMouseMove(int mouseX, int mouseY) {
	_mouseX = mouseX;
	_mouseY = mouseY;
	_buttons->handleMouseAction(mouseX, mouseY, false, false, false);
	_uiContainer->handleMouseMove(mouseX, mouseY);
}

void KIASectionSuspects::handleMouseDown(bool mainButton) {
	if (mainButton) {
		_buttons->handleMouseAction(_mouseX, _mouseY, true, false, false);
	}
	_uiContainer->handleMouseDown(!mainButton);
}

void KIASectionSuspects::handleMouseUp(bool mainButton) {
	if (mainButton) {
		_buttons->handleMouseAction(_mouseX, _mouseY, false, true, false);
	}
	_uiContainer->handleMouseUp(!mainButton);
}

void KIASectionSuspects::saveToLog() {
	int data[] = {
		_crimeSelected,
		_suspectSelected,
		_whereaboutsFilter,
		_MOFilter,
		_replicantFilter,
		_nonReplicantFilter,
		_othersFilter
	};
	_vm->_kia->_log->add(1, sizeof(data), &data);
}

void KIASectionSuspects::loadFromLog() {
	const int *data = (const int*)_vm->_kia->_log->getCurrentData();
	_crimeSelected = data[0];
	_suspectSelected = data[1];
	_whereaboutsFilter = data[2];
	_MOFilter = data[3];
	_replicantFilter = data[4];
	_nonReplicantFilter = data[5];
	_othersFilter = data[6];
	populateCrimes();
	populateVisibleClues();
}

void KIASectionSuspects::selectSuspect(int suspectId) {
	_suspectSelected = suspectId;
	populateCrimes();
	populateVisibleClues();
	updateSuspectPhoto();
}

const char *KIASectionSuspects::scrambleSuspectsName(const char *name) {
	static char buffer[32];

	char *bufferPtr = buffer;
	const char *namePtr = name;

	for (int i = 0 ; i < 6; ++i) {
		if (Common::isAlpha(*namePtr)) {
			char upper = toupper(*namePtr);
			if ( upper < 'J' ){
				*bufferPtr++ = upper - 16;
			} else {
				*bufferPtr++ = upper - 9;
			}
		} else {
			*bufferPtr++ = '0';
		}
		if (*namePtr) {
			++namePtr;
		}
		if (i == 1) {
			*bufferPtr++ = '-';
		}
	}
	*bufferPtr = 0;
	return buffer;
}

void KIASectionSuspects::scrollBoxCallback(void *callbackData, void *source, int lineData, int mouseButton) {
	KIASectionSuspects *self = (KIASectionSuspects *)callbackData;

	if (source == self->_cluesScrollBox && lineData >= 0) {
		if (mouseButton) {
			if (self->_vm->_gameFlags->query(kFlagKIAPrivacyAddon)) {
				self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(511), 70, 0, 0, 50, 0);

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
	} else if (source == self->_crimesScrollBox && lineData >= 0 && !mouseButton) {
		self->_crimeSelected = lineData - 5;
		self->_scheduledSwitch = true;
	}
}

void KIASectionSuspects::checkBoxCallback(void *callbackData, void *source) {
	KIASectionSuspects *self = (KIASectionSuspects *)callbackData;
	UICheckBox *checkBox = (UICheckBox *)source;

	if (checkBox == self->_whereaboutsCheckBox) {
		self->_whereaboutsFilter = checkBox->_isChecked;
	} else if (checkBox == self->_MOCheckBox) {
		self->_MOFilter = checkBox->_isChecked;
	} else if (checkBox == self->_replicantCheckBox) {
		self->_replicantFilter = checkBox->_isChecked;
	} else if (checkBox == self->_nonReplicantCheckBox) {
		self->_nonReplicantFilter = checkBox->_isChecked;
	} else if (checkBox == self->_othersCheckBox) {
		self->_othersFilter = checkBox->_isChecked;
	}
	self->populateVisibleClues();
}

void KIASectionSuspects::mouseUpCallback(int buttonId, void *callbackData) {
	((KIASectionSuspects *)callbackData)->onButtonPressed(buttonId);
}

void KIASectionSuspects::onButtonPressed(int buttonId) {
	switch (buttonId) {
	case 0:
		enableAllFilters();
		break;
	case 1:
		disableAllFilters();
		break;
	case 2:
		prevSuspect();
		break;
	case 3:
		nextSuspect();
		break;
	}
}

void KIASectionSuspects::populateAcquiredClues() {
	_acquiredClueCount = 0;
	for (int i = 0; i < kClueCount; ++i) {
		if (_clues->isAcquired(i)) {
			_acquiredClues[_acquiredClueCount].clueId = i;
			_acquiredClues[_acquiredClueCount].actorId = _clues->getFromActorId(i);
			++_acquiredClueCount;
		}
	}
	// sort clues by name, is it necessary
}

void KIASectionSuspects::populateSuspects() {
	int firstSuspect = -1;
	int suspectCount = _vm->_gameInfo->getSuspectCount();

	for (int i = 0; i < suspectCount; ++i) {
		_suspectsFound[i] = false;
		_suspectsWithIdentity[i] = false;
	}

	_suspectsFoundCount = 0;

	if (!_acquiredClueCount) {
		return;
	}

	for (int i = 0; i < suspectCount; ++i) {
		for (int j = 0; j < _acquiredClueCount; ++j) {
			if (_vm->_crimesDatabase->getCrime(_acquiredClues[j].clueId) != -1
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

	if (_suspectsFoundCount && _suspectSelected == -1) {
		_suspectSelected = firstSuspect;
	}
}

void KIASectionSuspects::populateCrimes() {
	_crimesScrollBox->clearLines();
	if (_suspectsFoundCount > 0 && _suspectSelected != -1) {
		for (int i = 0; i < (int)_vm->_gameInfo->getCrimeCount(); ++i) {
			for (int j = 0; j < _acquiredClueCount; ++j) {
				if (_vm->_crimesDatabase->getCrime(_acquiredClues[j].clueId) == i
				 && _vm->_suspectsDatabase->get(_suspectSelected)->hasClue(_acquiredClues[j].clueId)) {
					_crimesScrollBox->addLine(_vm->_textCrimes->getText(i), i + 5, 0);
					break;
				}
			}
		}
		_crimesScrollBox->sortLines();
	}
}

void KIASectionSuspects::populateVisibleClues() {
	_cluesScrollBox->clearLines();
	if (_suspectsFoundCount > 0 && _suspectSelected != -1) {
		for (int i = 0; i < _acquiredClueCount; ++i) {
			int clueId = _acquiredClues[i].clueId;

			if (_vm->_crimesDatabase->getAssetType(i) != -1) {
				SuspectDatabaseEntry *suspect = _vm->_suspectsDatabase->get(_suspectSelected);

				bool showClue = false;

				if (_whereaboutsFilter && suspect->hasWhereaboutsClue(clueId)) {
					showClue = true;
				} else if (_MOFilter && suspect->hasMOClue(clueId)) {
					showClue = true;
				} else if (_replicantFilter && suspect->hasReplicantClue(clueId)) {
					showClue = true;
				} else if (_nonReplicantFilter && suspect->hasNonReplicantClue(clueId)) {
					showClue = true;
				} else if (_othersFilter && suspect->hasOtherClue(clueId)) {
					showClue = true;
				}

				if (showClue) {
					int flags = 0x30;
					if (_clues->isPrivate(clueId)) {
						flags = 0x08;
					} else if (_clues->isViewed(clueId)) {
						flags = 0x10;
					}
					_cluesScrollBox->addLine(_vm->_crimesDatabase->getClueText(clueId), clueId, flags);
				}
			}
		}
		_cluesScrollBox->sortLines();
	}
}

void KIASectionSuspects::updateSuspectPhoto() {
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

void KIASectionSuspects::nextSuspect() {
	if (_suspectsFoundCount >= 2) {
		while (true) {
			++_suspectSelected;
			if (_suspectSelected >= (int)_vm->_gameInfo->getSuspectCount()){
				_suspectSelected = 0;
			}

			if (_suspectsFound[_suspectSelected]) {
				selectSuspect(_suspectSelected);
				break;
			}
		}
	}
}

void KIASectionSuspects::prevSuspect() {
	if (_suspectsFoundCount >= 2) {
		while (true) {
			--_suspectSelected;
			if (_suspectSelected < 0){
				_suspectSelected = _vm->_gameInfo->getSuspectCount() - 1;
			}

			if (_suspectsFound[_suspectSelected]) {
				selectSuspect(_suspectSelected);
				break;
			}
		}
	}
}

void KIASectionSuspects::enableAllFilters() {
	_whereaboutsFilter = true;
	_MOFilter = true;
	_replicantFilter = true;
	_nonReplicantFilter = true;
	_othersFilter = true;
	populateVisibleClues();
}

void KIASectionSuspects::disableAllFilters() {
	_whereaboutsFilter = false;
	_MOFilter = false;
	_replicantFilter = false;
	_nonReplicantFilter = false;
	_othersFilter = false;
	populateVisibleClues();
}

} // End of namespace BladeRunner
