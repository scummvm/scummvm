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

#include "bladerunner/ui/kia_section_load.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/game_info.h"
#include "bladerunner/savefile.h"
#include "bladerunner/shape.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/ui_container.h"
#include "bladerunner/ui/ui_scroll_box.h"

#include "common/error.h"
#include "common/system.h"

namespace BladeRunner {

KIASectionLoad::KIASectionLoad(BladeRunnerEngine *vm) : KIASectionBase(vm) {
	_uiContainer = new UIContainer(_vm);
	_scrollBox   = new UIScrollBox(_vm, scrollBoxCallback, this, 1025, 0, true, Common::Rect(155, 158, 461, 346), Common::Rect(506, 160, 506, 350));
	_uiContainer->add(_scrollBox);

	_timeLast = 0u;
	_timeLeft = 0u;

	_hoveredLineId = -1;
	_displayingLineId = -1;
	_newGameEasyLineId = -1;
	_newGameMediumLineId = -1;
	_newGameHardLineId = -1;
}

KIASectionLoad::~KIASectionLoad() {
	_uiContainer->clear();
	delete _scrollBox;
	delete _uiContainer;
}

void KIASectionLoad::open() {
	_scheduledSwitch = false;
	_scrollBox->show();
	_scrollBox->clearLines();

	_saveList = SaveFileManager::list(_vm->getTargetName());

	if (!_saveList.empty()) {
		_scrollBox->addLine(_vm->_textOptions->getText(36), -1, 4); // Load game:
		for (uint i = 0; i < _saveList.size(); ++i) {
			_scrollBox->addLine(_saveList[i].getDescription(), i, 0);
		}
		_scrollBox->addLine("", -1, 4);
	}

	_newGameEasyLineId = _saveList.size();
	_newGameMediumLineId = _saveList.size() + 1;
	_newGameHardLineId = _saveList.size() + 2;

	_scrollBox->addLine(_vm->_textOptions->getText(37), -1, 4); // New game:
	_scrollBox->addLine(_vm->_textOptions->getText(20), _newGameEasyLineId, 0); // Easy
	_scrollBox->addLine(_vm->_textOptions->getText(28), _newGameMediumLineId, 0); // Medium
	_scrollBox->addLine(_vm->_textOptions->getText(29), _newGameHardLineId, 0); // Hard

	_hoveredLineId = -1;
	_timeLast = _vm->_time->currentSystem();
	_timeLeft = 800u;
}

void KIASectionLoad::close() {
	_scrollBox->hide();
	_vm->_kia->playerReset();

	_saveList.clear();
}

void KIASectionLoad::draw(Graphics::Surface &surface) {
	_vm->_kia->_shapes->get(69)->draw(surface, 501, 123);

	_uiContainer->draw(surface);

	int selectedLineId = _scrollBox->getSelectedLineData();

	if (_hoveredLineId != selectedLineId) {
		if (selectedLineId >= 0 && selectedLineId < (int)_saveList.size() && _displayingLineId != selectedLineId) {
			if (_timeLeft == 0u) {
				SaveStateDescriptor desc = SaveFileManager::queryMetaInfos(_vm->getTargetName(), selectedLineId);
				const Graphics::Surface *thumbnail = desc.getThumbnail();
				if (thumbnail != nullptr) {
					_vm->_kia->playImage(*thumbnail);
					_displayingLineId = selectedLineId;
				}
			}
		} else {
			_vm->_kia->playerReset();
			_timeLeft = 800u;
			_displayingLineId = -1;
		}
		_hoveredLineId = selectedLineId;
	}

	uint32 now = _vm->_time->currentSystem();
	if (selectedLineId >= 0 && selectedLineId < (int)_saveList.size() && _displayingLineId != selectedLineId) {
		if (_timeLeft) {
			uint32 timeDiff = now - _timeLast; // unsigned difference is intentional
			if (timeDiff >= _timeLeft) {
				SaveStateDescriptor desc = SaveFileManager::queryMetaInfos(_vm->getTargetName(), _saveList[selectedLineId].getSaveSlot());
				const Graphics::Surface *thumbnail = desc.getThumbnail();
				if (thumbnail != nullptr) {
					_vm->_kia->playImage(*thumbnail);
					_displayingLineId = selectedLineId;
				}
			} else {
				_timeLeft = (_timeLeft < timeDiff) ? 0u : (_timeLeft - timeDiff);
			}
		}
	}

	_timeLast = now;
}

void KIASectionLoad::handleMouseMove(int mouseX, int mouseY) {
	_uiContainer->handleMouseMove(mouseX, mouseY);
}

void KIASectionLoad::handleMouseDown(bool mainButton) {
	_uiContainer->handleMouseDown(!mainButton);
}

void KIASectionLoad::handleMouseUp(bool mainButton) {
	_uiContainer->handleMouseUp(!mainButton);
}

void KIASectionLoad::handleMouseScroll(int direction) {
	_uiContainer->handleMouseScroll(direction);
}

void KIASectionLoad::scrollBoxCallback(void *callbackData, void *source, int lineData, int mouseButton) {
	KIASectionLoad *self = (KIASectionLoad *)callbackData;

	if (mouseButton == 0 && source == self->_scrollBox && lineData >= 0) {
		if (lineData == self->_newGameEasyLineId) {
			self->_vm->newGame(kGameDifficultyEasy);
		} else if (lineData == self->_newGameMediumLineId) {
			self->_vm->newGame(kGameDifficultyMedium);
		} else if (lineData == self->_newGameHardLineId) {
			self->_vm->newGame(kGameDifficultyHard);
		} else {
			self->_vm->loadGameState(self->_saveList[lineData].getSaveSlot());
		}

		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxELECBP1), 90, 0, 0, 50, 0);
		self->_vm->_kia->resume();
		self->_scheduledSwitch = true;
	}
}

} // End of namespace BladeRunner
