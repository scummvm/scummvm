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

#include "bladerunner/ui/kia_section_save.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/font.h"
#include "bladerunner/game_info.h"
#include "bladerunner/savefile.h"
#include "bladerunner/shape.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/ui_container.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/ui/ui_input_box.h"
#include "bladerunner/ui/ui_scroll_box.h"

#include "common/error.h"
#include "common/keyboard.h"
#include "common/system.h"

namespace BladeRunner {

KIASectionSave::KIASectionSave(BladeRunnerEngine *vm) : KIASectionBase(vm) {
	_uiContainer = new UIContainer(_vm);

	_scrollBox = new UIScrollBox(_vm, scrollBoxCallback, this, 1024, 0, true, Common::Rect(155, 158, 461, 346), Common::Rect(506, 160, 506, 350));
	_uiContainer->add(_scrollBox);

	_inputBox = new UIInputBox(_vm, inputBoxCallback, this, Common::Rect(155, 367, 461, 376), SaveFileManager::kNameLength, ""); // original game has limit 41 characters
	_uiContainer->add(_inputBox);
	_inputBox->hide();

	_buttons = new UIImagePicker(_vm, 3);

	_timeLast = 0u;
	_timeLeft = 0u;

	_state = kStateNormal;

	_mouseX = 0;
	_mouseY = 0;

	_hoveredLineId = -1;
	_displayingLineId = -1;
	_selectedLineId = -1;
	_newSaveLineId = -1;
}

KIASectionSave::~KIASectionSave() {
	delete _buttons;

	_uiContainer->clear();
	delete _inputBox;
	delete _scrollBox;
	delete _uiContainer;
}

void KIASectionSave::open() {
	_scheduledSwitch = false;
	_state = kStateNormal;

	_buttons->resetImages();
	_buttons->defineImage(
		0,
		Common::Rect(460, 366, 497, 402),
		_vm->_kia->_shapes->get(82),
		_vm->_kia->_shapes->get(83),
		_vm->_kia->_shapes->get(84),
		_vm->_textOptions->getText(22) // Save
	);

	_scrollBox->show();

	_saveList = SaveFileManager::list(_vm->getTargetName());

	bool ableToSaveGame = true;

	_newSaveLineId = _saveList.size();

	if (!_saveList.empty() || ableToSaveGame) {

		_buttons->activate(nullptr, nullptr, nullptr, onButtonPressed, this);
		_inputBox->show();

		_scrollBox->clearLines();

		if (ableToSaveGame) {
			_scrollBox->addLine(_vm->_textOptions->getText(23), _newSaveLineId, 0);
		}

		for (uint i = 0; i < _saveList.size(); ++i) {
			_scrollBox->addLine(_saveList[i].getDescription(), i, 0);
		}

		if (ableToSaveGame) {
			// New save
			_selectedLineId = _newSaveLineId;
			_inputBox->setText("");
		} else {
			// Overwrite first save
			_selectedLineId = 0;
			_inputBox->setText(_saveList[_selectedLineId].getDescription());
		}

		_scrollBox->setFlags(_selectedLineId, 8);
	}

	_hoveredLineId = -1;
	_timeLast = _vm->_time->currentSystem();
	_timeLeft = 800u;
}

void KIASectionSave::close() {
	_inputBox->hide();
	_scrollBox->hide();
	_buttons->deactivate();

	_vm->_kia->playerReset();

	_saveList.clear();
}

void KIASectionSave::draw(Graphics::Surface &surface) {
	_vm->_kia->_shapes->get(69)->draw(surface, 501, 123);
	_buttons->draw(surface);

	if (_state == kStateNormal) {
		const char *textChooseSlot = _vm->_textOptions->getText(24); // Choose a slot ...
		int textChooseSlotWidth = _vm->_mainFont->getStringWidth(textChooseSlot);
		_vm->_mainFont->drawString(&surface, textChooseSlot, 308 - textChooseSlotWidth / 2, 143, surface.w, surface.format.RGBToColor(240, 232, 192));

		// Original game shows warnings/error here, but we don't have any

		const char *textTypeName = _vm->_textOptions->getText(25); // Type a name ...
		int textTypeNameWidth = _vm->_mainFont->getStringWidth(textTypeName);
		_vm->_mainFont->drawString(&surface, textTypeName, 308 - textTypeNameWidth / 2, 352, surface.w, surface.format.RGBToColor(240, 232, 192));

		_uiContainer->draw(surface);
	} else if (_state == kStateOverwrite) {
		surface.fillRect(Common::Rect(155, 230, 462, 239), surface.format.RGBToColor(80, 56, 32));

		const Common::String &saveName = _saveList[_selectedLineId].getDescription();
		int saveNameWidth = _vm->_mainFont->getStringWidth(saveName);
		_vm->_mainFont->drawString(&surface, saveName, 308 - saveNameWidth / 2, 230, surface.w, surface.format.RGBToColor(232, 208, 136));

		const char *textOverwrite = _vm->_textOptions->getText(35); // Overwrite previously saved game?
		int textOverwriteWidth = _vm->_mainFont->getStringWidth(textOverwrite);
		_vm->_mainFont->drawString(&surface, textOverwrite, 308 - textOverwriteWidth / 2, 240, surface.w, surface.format.RGBToColor(240, 232, 192));
	} else if (_state == kStateDelete) {
		surface.fillRect(Common::Rect(155, 230, 462, 239), surface.format.RGBToColor(80, 56, 32));

		const Common::String &saveName = _saveList[_selectedLineId].getDescription();
		int saveNameWidth = _vm->_mainFont->getStringWidth(saveName); // Delete this game?
		_vm->_mainFont->drawString(&surface, saveName, 308 - saveNameWidth / 2, 230, surface.w, surface.format.RGBToColor(232, 208, 136));

		const char *textDelete = _vm->_textOptions->getText(40);
		int textDeleteWidth = _vm->_mainFont->getStringWidth(textDelete);
		_vm->_mainFont->drawString(&surface, textDelete, 308 - textDeleteWidth / 2, 240, surface.w, surface.format.RGBToColor(240, 232, 192));
	}

	int selectedLineId = _scrollBox->getSelectedLineData();

	if (selectedLineId != _hoveredLineId) {
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
	_buttons->drawTooltip(surface, _mouseX, _mouseY);
}

void KIASectionSave::handleKeyUp(const Common::KeyState &kbd) {
	if (_state == kStateNormal) {
		_uiContainer->handleKeyUp(kbd);
	}
}

void KIASectionSave::handleKeyDown(const Common::KeyState &kbd) {
	if (_state == kStateNormal) {
		if (kbd.keycode == Common::KEYCODE_DELETE && _selectedLineId != _newSaveLineId) {
			changeState(kStateDelete);
		}
		_uiContainer->handleKeyDown(kbd);
	} else if (_state == kStateOverwrite) {
		if (kbd.keycode == Common::KEYCODE_RETURN) {
			save();
			changeState(kStateNormal);
		}
	} else if (_state == kStateDelete) {
		if (kbd.keycode == Common::KEYCODE_RETURN) {
			deleteSave();
			changeState(kStateNormal);
		}
	}
}

void KIASectionSave::handleMouseMove(int mouseX, int mouseY) {
	_mouseX = mouseX;
	_mouseY = mouseY;

	_buttons->handleMouseAction(_mouseX, _mouseY, false, false, false);

	if (_state == kStateNormal) {
		_uiContainer->handleMouseMove(_mouseX, _mouseY);
	}
}

void KIASectionSave::handleMouseDown(bool mainButton) {
	if (mainButton) {
		if (_state == kStateNormal) {
			_uiContainer->handleMouseDown(false);
		}

		_buttons->handleMouseAction(_mouseX, _mouseY, true, false, false);
	}
}

void KIASectionSave::handleMouseUp(bool mainButton) {
	if (mainButton) {
		_buttons->handleMouseAction(_mouseX, _mouseY, false, true, false);

		if (_state == kStateNormal) {
			_uiContainer->handleMouseUp(false);
		}
	}
}

void KIASectionSave::handleMouseScroll(int direction) {
	if (_state == kStateNormal) {
		_uiContainer->handleMouseScroll(direction);
	}
}


void KIASectionSave::scrollBoxCallback(void *callbackData, void *source, int lineData, int mouseButton) {
	KIASectionSave *self = (KIASectionSave *)callbackData;

	if (mouseButton == 0 && source == self->_scrollBox && lineData >= 0 && lineData <= (int)self->_saveList.size()) {
		self->_scrollBox->resetFlags(self->_selectedLineId, 8);
		self->_selectedLineId = lineData;
		self->_scrollBox->setFlags(self->_selectedLineId, 8);

		if (self->_selectedLineId == self->_newSaveLineId) {
			self->_inputBox->setText("");
		} else {
			self->_inputBox->setText(self->_saveList[self->_selectedLineId].getDescription());
		}

		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxSPNBEEP3), 40, 0, 0, 50, 0);
		self->_vm->_kia->resume();
	}
}

void  KIASectionSave::inputBoxCallback(void *callbackData, void *source) {
	KIASectionSave *self = (KIASectionSave *)callbackData;
	if (source == self->_inputBox) {
		if (self->_selectedLineId == self->_newSaveLineId) {
			self->save();
		} else {
			self->changeState(kStateOverwrite);
		}
	}
}

void KIASectionSave::onButtonPressed(int buttonId, void *callbackData) {
	KIASectionSave *self = (KIASectionSave *)callbackData;

	if (buttonId == 0) {
		if (self->_selectedLineId == self->_newSaveLineId)
		{
			self->save();
		}
		else
		{
			self->changeState(kStateOverwrite);
		}
	} else if (buttonId == 1) {
		self->changeState(kStateNormal);
		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxSPNBEEP6), 90, -50, -50, 50, 0);
	} else if (buttonId == 2) {
		if (self->_state == kStateOverwrite)
		{
			self->save();
		}
		else if (self->_state == kStateDelete)
		{
			self->deleteSave();
		}
	}
}

void KIASectionSave::changeState(State state) {
	_state = state;
	if (state == kStateNormal) {
		_buttons->resetImages();
		_buttons->defineImage(
			0,
			Common::Rect(460, 366, 497, 402),
			_vm->_kia->_shapes->get(82),
			_vm->_kia->_shapes->get(83),
			_vm->_kia->_shapes->get(84),
			_vm->_textOptions->getText(22) // Save
		);
	} else {
		_buttons->resetImages();
		_buttons->defineImage(
			1,
			Common::Rect(318, 260, 357, 299),
			_vm->_kia->_shapes->get(126),
			_vm->_kia->_shapes->get(127),
			_vm->_kia->_shapes->get(128),
			_vm->_textOptions->getText(38) // No
		);
		_buttons->defineImage(
			2,
			Common::Rect(258, 260, 297, 299),
			_vm->_kia->_shapes->get(129),
			_vm->_kia->_shapes->get(130),
			_vm->_kia->_shapes->get(131),
			_vm->_textOptions->getText(39) // Yes
		);
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxSPNBEEP7), 90, 0, 0, 50, 0);
	}
}

void KIASectionSave::save() {
	int slot = -1;

	if (_selectedLineId < (int)_saveList.size()) {
		slot = _saveList[_selectedLineId].getSaveSlot();
	} else {
		// Find first available save slot
		int maxSlot = -1;
		for (int i = 0; i < (int)_saveList.size(); ++i) {
			maxSlot = MAX(maxSlot, _saveList[i].getSaveSlot());
			if (_saveList[i].getSaveSlot() != i) {
				slot = i;
				break;
			}
		}

		if (slot == -1) {
			slot = maxSlot + 1;
		}
	}

	Common::OutSaveFile *saveFile = BladeRunner::SaveFileManager::openForSaving(_vm->getTargetName(), slot);
	if (saveFile == nullptr || saveFile->err()) {
		delete saveFile;
		warning("KIASectionSave::save(): Can not open savegame file for writing");
		return;
	}

	BladeRunner::SaveFileHeader header;
	header._name = _inputBox->getText();
	header._playTime = _vm->getTotalPlayTime();

	BladeRunner::SaveFileManager::writeHeader(*saveFile, header);

	_vm->saveGame(*saveFile, _vm->_kia->_thumbnail);

	saveFile->finalize();

	delete saveFile;

	_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxELECBP1), 90, 0, 0, 50, 0);

	_scheduledSwitch = true;
}
void KIASectionSave::deleteSave() {
	BladeRunner::SaveFileManager::remove(_vm->getTargetName(), _saveList[_selectedLineId].getSaveSlot());
	close();
	open();
}

} // End of namespace BladeRunner
