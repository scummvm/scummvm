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
#include "bladerunner/text_resource.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/kia_shapes.h"
#include "bladerunner/ui/ui_container.h"
#include "bladerunner/ui/ui_scroll_box.h"

#include "common/error.h"
#include "common/system.h"

#include "engines/savestate.h"

namespace BladeRunner {

KIASectionLoad::KIASectionLoad(BladeRunnerEngine *vm) : KIASectionBase(vm) {
	_uiContainer = new UIContainer(_vm);
	_scrollBox   = new UIScrollBox(_vm, scrollBoxCallback, this, 1025, 0, true, Common::Rect(155, 158, 461, 346), Common::Rect(506, 160, 506, 350));
	_uiContainer->add(_scrollBox);
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

	SaveStateList saveList = SaveFileManager::list(_vm->getTargetName());

	_saveSlotMax = -1;

	if (!saveList.empty()) {
		_scrollBox->addLine(_vm->_textOptions->getText(36), -1, 4); // Load game:
		for (Common::Array<SaveStateDescriptor>::iterator save = saveList.begin(); save != saveList.end(); save++) {
			_scrollBox->addLine(save->getDescription(), save->getSaveSlot(), 0);
			_saveSlotMax = MAX(_saveSlotMax, save->getSaveSlot());
		}
		_scrollBox->addLine("", -1, 4);
	}


	_scrollBox->addLine(_vm->_textOptions->getText(37), -1, 4); // New game:
	_scrollBox->addLine(_vm->_textOptions->getText(20), _saveSlotMax + 1, 0); // Easy
	_scrollBox->addLine(_vm->_textOptions->getText(28), _saveSlotMax + 2, 0); // Medium
	_scrollBox->addLine(_vm->_textOptions->getText(29), _saveSlotMax + 3, 0); // Hard

	_hoveredSaveSlot = -1;
	_timeLast = _vm->getTotalPlayTime();
	_timeLeft = 800;
}

void KIASectionLoad::close() {
	_scrollBox->hide();

	_vm->_kia->playerReset();
}

void KIASectionLoad::draw(Graphics::Surface &surface){
	_vm->_kia->_shapes->get(69)->draw(surface, 501, 123);

	_uiContainer->draw(surface);

	int selectedSaveSlot = _scrollBox->getSelectedLineData();

	if (_hoveredSaveSlot != selectedSaveSlot) {
		if (selectedSaveSlot >= 0) {
			if (_timeLeft == 0) {
				SaveStateDescriptor desc = SaveFileManager::queryMetaInfos(_vm->getTargetName(), selectedSaveSlot);
				const Graphics::Surface *thumbnail = desc.getThumbnail();
				if (thumbnail != nullptr) {
					_vm->_kia->playImage(*thumbnail);
				}
			}
		} else {
			_vm->_kia->playerReset();
			_timeLeft = 800;
		}
		_hoveredSaveSlot = selectedSaveSlot;
	}

	uint32 now = _vm->getTotalPlayTime();
	if (selectedSaveSlot >= 0) {
		if (_timeLeft) {
			uint32 timeDiff = now - _timeLast;
			if (timeDiff >= _timeLeft) {
				SaveStateDescriptor desc = SaveFileManager::queryMetaInfos(_vm->getTargetName(), selectedSaveSlot);
				const Graphics::Surface *thumbnail = desc.getThumbnail();
				if (thumbnail != nullptr) {
					_vm->_kia->playImage(*thumbnail);
				}
			} else {
				_timeLeft -= timeDiff;
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

void KIASectionLoad::scrollBoxCallback(void *callbackData, void *source, int lineData, int mouseButton) {
	KIASectionLoad *self = (KIASectionLoad *)callbackData;

	if (mouseButton == 0 && source == self->_scrollBox && lineData >= 0) {
		if (lineData == self->_saveSlotMax + 1) {
			self->_vm->newGame(0);
		} else if (lineData == self->_saveSlotMax + 2) {
			self->_vm->newGame(1);
		} else if (lineData == self->_saveSlotMax + 3) {
			self->_vm->newGame(2);
		} else {
			self->_vm->loadGameState(lineData);
		}

		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(513), 90, 0, 0, 50, 0);
		self->_vm->_kia->resume();
		self->_scheduledSwitch = true;
	}
}

} // End of namespace BladeRunner
