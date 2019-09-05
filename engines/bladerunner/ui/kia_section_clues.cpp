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

#include "bladerunner/ui/kia_section_clues.h"

#include "bladerunner/actor_clues.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/crimes_database.h"
#include "bladerunner/game_flags.h"
#include "bladerunner/game_info.h"
#include "bladerunner/font.h"
#include "bladerunner/script/kia_script.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/kia_log.h"
#include "bladerunner/ui/kia_shapes.h"
#include "bladerunner/ui/ui_container.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/ui/ui_scroll_box.h"

#include "common/rect.h"

namespace BladeRunner {

KIASectionClues::KIASectionClues(BladeRunnerEngine *vm, ActorClues *clues) : KIASectionBase(vm) {
	_uiContainer = new UIContainer(_vm);

	_isOpen = false;

	_debugIntangible = false;
	_debugNop = 0;

	_clues = clues;

	_mouseX = 0;
	_mouseY = 0;

	_buttons = new UIImagePicker(_vm, 2);

	_cluesScrollBox = new UIScrollBox(_vm, scrollBoxCallback, this, kClueCount, 1, false, Common::Rect(312, 172, 500, 376), Common::Rect(506, 160, 506, 394));
	_uiContainer->add(_cluesScrollBox);

	_filterScrollBox = new UIScrollBox(_vm, scrollBoxCallback, this, 128, 1, false, Common::Rect(142, 162, 291, 376), Common::Rect(120, 160, 120, 370));
	_uiContainer->add(_filterScrollBox);

	_assetTypeFilterCount = 4 + 1; // we have 4 asset types
	_crimeFilterCount     = _vm->_gameInfo->getCrimeCount() + 1;
	_filterCount          = _assetTypeFilterCount + _crimeFilterCount;
	_filters.resize(_filterCount);
	for (int i = 0; i < _filterCount; ++i) {
		_filters[i] = true;
	}
}

KIASectionClues::~KIASectionClues() {
	_uiContainer->clear();
	delete _filterScrollBox;
	delete _cluesScrollBox;
	delete _buttons;
	delete _uiContainer;
}

void KIASectionClues::reset() {
	_debugIntangible = false;
	_debugNop = 0;

	_mouseX = 0;
	_mouseY = 0;

	for (int i = 0; i < _filterCount; ++i) {
		_filters[i] = true;
	}
}

void KIASectionClues::open() {
	_isOpen = true;

	_buttons->resetImages();
	_buttons->defineImage(0, Common::Rect(142, 380, 191, 395), _vm->_kia->_shapes->get(79), _vm->_kia->_shapes->get(80), _vm->_kia->_shapes->get(81), _vm->_textKIA->getText(30));
	_buttons->defineImage(1, Common::Rect(193, 380, 242, 395), _vm->_kia->_shapes->get(76), _vm->_kia->_shapes->get(77), _vm->_kia->_shapes->get(78), _vm->_textKIA->getText(31));
	_buttons->activate(nullptr, nullptr, nullptr, mouseUpCallback, this);

	_cluesScrollBox->show();
	_filterScrollBox->show();

	populateFilters();
	populateClues();
}

void KIASectionClues::close() {
	if (_isOpen) {
		_isOpen = false;

		_buttons->deactivate();
		_cluesScrollBox->hide();
		_filterScrollBox->hide();
	}
}

void KIASectionClues::draw(Graphics::Surface &surface) {
	_uiContainer->draw(surface);

	_vm->_mainFont->drawString(&surface, _vm->_textKIA->getText(0), 300, 162, surface.w, surface.format.RGBToColor(232, 240, 255));
	_vm->_mainFont->drawString(&surface, _vm->_textKIA->getText(2), 440, 426, surface.w, surface.format.RGBToColor(80, 96, 136));
	_vm->_mainFont->drawString(&surface, _vm->_textKIA->getText(1), 440, 442, surface.w, surface.format.RGBToColor(80, 96, 136));
	_vm->_mainFont->drawString(&surface, _vm->_textKIA->getText(4), 440, 458, surface.w, surface.format.RGBToColor(80, 96, 136));

	int clueId = _cluesScrollBox->getSelectedLineData();
	if (clueId != -1) {
		Common::String text;

		int actorId = _clues->getFromActorId(clueId);
		if (actorId != -1) {
			text = _vm->_textActorNames->getText(actorId);
		} else {
			text.clear();
		}
		_vm->_mainFont->drawString(&surface, text, 490, 426, surface.w, surface.format.RGBToColor(136, 168, 255));

		int crimeId = _vm->_crimesDatabase->getCrime(clueId);
		if (crimeId != -1) {
			text = _vm->_textCrimes->getText(crimeId);
		} else {
			text.clear();
		}
		_vm->_mainFont->drawString(&surface, text, 490, 442, surface.w, surface.format.RGBToColor(136, 168, 255));

		int assetType = _vm->_crimesDatabase->getAssetType(clueId);
		if (assetType != -1) {
			text = _vm->_textClueTypes->getText(assetType);
		} else {
			text.clear();
		}
		_vm->_mainFont->drawString(&surface, text, 490, 458, surface.w, surface.format.RGBToColor(136, 168, 255));
	}

	_buttons->draw(surface);
	_buttons->drawTooltip(surface, _mouseX, _mouseY);

	if (_debugNop) {
		_vm->_mainFont->drawString(&surface, Common::String::format("Debug display: %s", _vm->_textActorNames->getText(_debugNop)), 120, 132, surface.w, surface.format.RGBToColor(255, 255, 0));
	}
	if (_debugIntangible) {
		_vm->_mainFont->drawString(&surface, "Debug Mode: Showing intangible clues.", 220, 105, surface.w, surface.format.RGBToColor(255, 255, 0));
	}
}

void KIASectionClues::handleMouseMove(int mouseX, int mouseY) {
	_mouseX = mouseX;
	_mouseY = mouseY;
	_buttons->handleMouseAction(mouseX, mouseY, false, false, false);
	_uiContainer->handleMouseMove(mouseX, mouseY);
}

void KIASectionClues::handleMouseDown(bool mainButton) {
	_uiContainer->handleMouseDown(!mainButton);
	if (mainButton) {
		_buttons->handleMouseAction(_mouseX, _mouseY, true, false, false);
	}
}

void KIASectionClues::handleMouseUp(bool mainButton) {
	_uiContainer->handleMouseUp(!mainButton);
	if (mainButton) {
		_buttons->handleMouseAction(_mouseX, _mouseY, false, true, false);
	}
}

void KIASectionClues::handleMouseScroll(int direction) {
	_uiContainer->handleMouseScroll(direction);
}

void KIASectionClues::saveToLog() {
	_vm->_kia->_log->add(0, sizeof(bool) * _filterCount, _filters.data());
}

void KIASectionClues::loadFromLog() {
	memcpy(_filters.data(), _vm->_kia->_log->getCurrentData(), sizeof(bool) * _filterCount);
	populateFilters();
	populateClues();
}

void KIASectionClues::scrollBoxCallback(void *callbackData, void *source, int lineData, int mouseButton) {
	KIASectionClues *self = (KIASectionClues *)callbackData;

	if (source == self->_filterScrollBox && lineData >= 0) {
		self->_filters[lineData] = !self->_filters[lineData];
		self->_filterScrollBox->toggleCheckBox(lineData);
		self->populateClues();
	} else if (source == self->_cluesScrollBox && lineData >= 0) {
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

void KIASectionClues::mouseUpCallback(int buttonId, void *callbackData) {
	KIASectionClues *self = (KIASectionClues *)callbackData;

	if (buttonId <= 1) {
		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxBEEP10A), 100, 0, 0, 50, 0);
	}

	self->onButtonPressed(buttonId);
}

void KIASectionClues::onButtonPressed(int buttonId) {
	if (buttonId == 1) {
		disableAllFilters();
	}
	if (buttonId == 0) {
		enableAllFilters();
	}
}

void KIASectionClues::enableAllFilters() {
	for (int i = 0; i < _filterCount; ++i) {
		if (_filterScrollBox->hasLine(i)) {
			_filters[i] = true;
		}
	}
	_filterScrollBox->checkAll();
	populateClues();
}

void KIASectionClues::disableAllFilters() {
	for (int i = 0; i < _filterCount; ++i) {
		if (_filterScrollBox->hasLine(i)) {
			_filters[i] = false;
		}
	}
	_filterScrollBox->uncheckAll();
	populateClues();
}

void KIASectionClues::populateFilters() {
	_filterScrollBox->clearLines();

	Common::Array<bool> availableFilters(_filterCount);
	for (int i = 0; i < _filterCount; ++i) {
		availableFilters[i] = false;
	}

	Common::String assetTypeNames[] = {
		_vm->_textKIA->getText(6),
		_vm->_textKIA->getText(7),
		_vm->_textKIA->getText(8),
		_vm->_textKIA->getText(9)
	};

	for (int i = 0; i < kClueCount; ++i) {
		int clueId = i;
		if (_clues->isAcquired(clueId)) {
			int assetType = _vm->_crimesDatabase->getAssetType(clueId);
			int crimeId = _vm->_crimesDatabase->getCrime(clueId);
			if (_debugIntangible || assetType != -1) {
				availableFilters[getLineIdForAssetType(assetType)] = true;
				availableFilters[getLineIdForCrimeId(crimeId)] = true;
			}
		}
	}

	int assetTypeFiltersAvailable = 0;
	for (int i = 0; i < _assetTypeFilterCount; ++i) {
		if (availableFilters[i]) {
			++assetTypeFiltersAvailable;
		}
	}

	int crimeFiltersAvailable = 0;
	for (int i = _assetTypeFilterCount; i < _filterCount; ++i) {
		if (availableFilters[i]) {
			++crimeFiltersAvailable;
		}
	}

	if (assetTypeFiltersAvailable > 1) {
		_filterScrollBox->addLine(_vm->_textKIA->getText(11), -1, 0x04);

		for (int i = 0; i < _assetTypeFilterCount; ++i) {
			if (availableFilters[i]) {
				int flags = 0x01;
				if (_filters[i]) {
					flags |= 0x02;
				}

				Common::String text;
				int typeTextId = getClueFilterTypeTextId(i);
				if (typeTextId == -1) {
					text = _vm->_textKIA->getText(10);
				} else {
					text =  assetTypeNames[typeTextId];
				}

				_filterScrollBox->addLine(text, i, flags);
			}
		}
	}

	if (crimeFiltersAvailable > 1) {
		if (assetTypeFiltersAvailable > 1) {
			_filterScrollBox->addLine(" ", -1, 0);
		}

		_filterScrollBox->addLine(_vm->_textKIA->getText(12), -1, 0x04);

		Common::Array<Line> crimeLines;
		crimeLines.reserve(crimeFiltersAvailable);

		for (int i = _assetTypeFilterCount; i < _filterCount; ++i) {
			if (availableFilters[i]) {
				Line line;

				line.lineData = i;

				line.flags = 0x01;
				if (_filters[i]) {
					line.flags |= 0x02;
				}

				int crimeId = getClueFilterCrimeId(i);
				if (crimeId == -1) {
					line.crimeName = _vm->_textKIA->getText(5);
				} else {
					line.crimeName = _vm->_textCrimes->getText(crimeId);
				}

				crimeLines.push_back(line);
			}
		}

		for (int i = 0; i < crimeFiltersAvailable - 1; ++i) {
			for (int j = i + 1; j < crimeFiltersAvailable; ++j) {
				if (crimeLines[i].lineData != _assetTypeFilterCount) {
					if (crimeLines[i].crimeName.compareToIgnoreCase(crimeLines[j].crimeName) <= 0) {
						continue;
					}
				}
				SWAP(crimeLines[i], crimeLines[j]);
			}
		}

		for (uint i = 0; i < crimeLines.size(); ++i) {
			_filterScrollBox->addLine(crimeLines[i].crimeName, crimeLines[i].lineData, crimeLines[i].flags);
		}
	}
}

void KIASectionClues::populateClues() {
	_cluesScrollBox->clearLines();
	for (int i = 0; i < kClueCount; ++i) {
		int clueId = i;
		if (_clues->isAcquired(clueId)) {
			int assetType = _vm->_crimesDatabase->getAssetType(clueId);
			int crimeId = _vm->_crimesDatabase->getCrime(clueId);
			if (assetType != -1 || _debugIntangible) {
				if (_filters[getLineIdForAssetType(assetType)] && _filters[getLineIdForCrimeId(crimeId)]) {
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
	}
	_cluesScrollBox->sortLines();
}

int KIASectionClues::getClueFilterTypeTextId(int filterId) {
	if (filterId) {
		return filterId - 1;
	}
	return -1;
}

int KIASectionClues::getClueFilterCrimeId(int filterId) {
	if (filterId != _assetTypeFilterCount) {
		return filterId - (_assetTypeFilterCount + 1);
	}
	return -1;
}

int KIASectionClues::getLineIdForAssetType(int assetType) {
	if (assetType == kClueTypeIntangible) {
		return 0;
	}
	return assetType + 1;
}

int KIASectionClues::getLineIdForCrimeId(int crimeId) {
	if (crimeId == -1) {
		return _assetTypeFilterCount;
	}
	return _assetTypeFilterCount + crimeId + 1;
}

} // End of namespace BladeRunner
