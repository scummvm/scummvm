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

#include "backends/keymapper/remap-dialog.h"

#ifdef ENABLE_KEYMAPPER

#include "backends/keymapper/action.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/input-watcher.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"

#include "common/system.h"
#include "gui/gui-manager.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollcontainer.h"
#include "gui/ThemeEval.h"
#include "common/translation.h"

namespace Common {

enum {
	kRemapCmd  = 'REMP',
	kClearCmd  = 'CLER',
	kResetCmd  = 'RSET',
	kCloseCmd  = 'CLOS',
	kReflowCmd = 'REFL'
};

RemapDialog::RemapDialog()
	: Dialog("KeyMapper"), _remapTimeout(0), _remapAction(nullptr) {

	_keymapper = g_system->getEventManager()->getKeymapper();
	assert(_keymapper);

	EventDispatcher *eventDispatcher = g_system->getEventManager()->getEventDispatcher();
	_remapInputWatcher = new InputWatcher(eventDispatcher, _keymapper);

	_kmPopUpDesc = new GUI::StaticTextWidget(this, "KeyMapper.PopupDesc", _("Keymap:"));
	_kmPopUp = new GUI::PopUpWidget(this, "KeyMapper.Popup");

	_scrollContainer = new GUI::ScrollContainerWidget(this, "KeyMapper.KeymapArea", "", kReflowCmd);
	_scrollContainer->setTarget(this);

	new GUI::ButtonWidget(this, "KeyMapper.Close", _("Close"), 0, kCloseCmd);
}

RemapDialog::~RemapDialog() {
	delete _remapInputWatcher;
}

void RemapDialog::open() {
	_keymapTable = _keymapper->getKeymaps();

	debug(3, "RemapDialog::open keymaps: %d", _keymapTable.size());

	// Show the keymaps by order of priority (game keymaps first)
	for (int i = _keymapTable.size() - 1; i >= 0; i--) {
		_kmPopUp->appendEntry(_keymapTable[i]->getName(), i);
	}

	_changes = false;

	_kmPopUp->setSelected(0);

	loadKeymap();
	refreshKeymap();
	reflowActionWidgets();

	Dialog::open();
}

void RemapDialog::close() {
	_kmPopUp->clearEntries();

	if (_changes) {
		const Array<Keymap *> &keymaps = _keymapper->getKeymaps();
		for (uint i = 0; i < keymaps.size(); i++) {
			keymaps[i]->saveMappings();
		}

		ConfMan.flushToDisk();
	}

	Dialog::close();
}

void RemapDialog::reflowActionWidgets() {
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	int spacing = g_gui.xmlEval()->getVar("Globals.KeyMapper.Spacing");
	int keyButtonWidth = g_gui.xmlEval()->getVar("Globals.KeyMapper.ButtonWidth");
	int clearButtonWidth = g_gui.xmlEval()->getVar("Globals.Line.Height");
	int clearButtonHeight = g_gui.xmlEval()->getVar("Globals.Line.Height");
	int labelWidth = getWidth() - (spacing + keyButtonWidth + spacing + clearButtonWidth + spacing);

	uint textYOff = (buttonHeight - kLineHeight) / 2;
	uint clearButtonYOff = (buttonHeight - clearButtonHeight) / 2;

	for (uint i = 0; i < _actions.size(); i++) {
		ActionRow &row = _actions[i];
		uint y = spacing + (i) * (buttonHeight + spacing);

		uint x = spacing;
		row.keyButton->resize(x, y, keyButtonWidth, buttonHeight);

		x += keyButtonWidth + spacing;
		row.clearButton->resize(x, y + clearButtonYOff, clearButtonWidth, clearButtonHeight);

		x += clearButtonWidth + spacing;
		row.resetButton->resize(x, y + clearButtonYOff, clearButtonWidth, clearButtonHeight);

		x += clearButtonWidth + spacing;
		row.actionText->resize(x, y + textYOff, labelWidth, kLineHeight);
	}
}

void RemapDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	debug(3, "RemapDialog::handleCommand %u %u", cmd, data);

	if (cmd >= kRemapCmd && cmd < kRemapCmd + _actions.size()) {
		startRemapping(cmd - kRemapCmd);
	} else if (cmd >= kClearCmd && cmd < kClearCmd + _actions.size()) {
		clearMapping(cmd - kClearCmd);
	} else if (cmd >= kResetCmd && cmd < kResetCmd + _actions.size()) {
		resetMapping(cmd - kResetCmd);
	} else if (cmd == kCloseCmd) {
		close();
	} else if (cmd == kReflowCmd) {
		reflowActionWidgets();
	} else if (cmd == GUI::kPopUpItemSelectedCmd) {
		clearKeymap();
		loadKeymap();
		refreshKeymap();
		_scrollContainer->reflowLayout();
		g_gui.scheduleTopDialogRedraw();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

void RemapDialog::clearMapping(uint i) {
	debug(3, "clear the mapping %u", i);
	Action *action = _actions[i].action;
	Keymap *keymap = action->getParent();
	keymap->unregisterMapping(action);

	_changes = true;

	stopRemapping();
	refreshKeymap();
}

void RemapDialog::resetMapping(uint i) {
	debug(3, "Reset the mapping %u", i);
	Action *action = _actions[i].action;
	Keymap *keymap = action->getParent();
	keymap->resetMapping(action);

	_changes = true;

	stopRemapping();
	refreshKeymap();
}

void RemapDialog::startRemapping(uint i) {
	if (_remapInputWatcher->isWatching()) {
		// Handle a second click on the button as a stop to remapping
		stopRemapping();
		return;
	}

	_remapAction = _actions[i].action;
	_remapTimeout = g_system->getMillis() + kRemapTimeoutDelay;
	_remapInputWatcher->startWatching();

	_actions[i].keyButton->setLabel("...");
	_actions[i].keyButton->markAsDirty();
}

void RemapDialog::stopRemapping() {
	_remapAction = nullptr;

	refreshKeymap();

	_remapInputWatcher->stopWatching();
}

void RemapDialog::handleMouseDown(int x, int y, int button, int clickCount) {
	if (_remapInputWatcher->isWatching())
		stopRemapping();
	else
		Dialog::handleMouseDown(x, y, button, clickCount);
}

void RemapDialog::handleTickle() {
	const HardwareInput *hardwareInput = _remapInputWatcher->checkForCapturedInput();
	if (hardwareInput) {
		Keymap *keymap = _remapAction->getParent();
		keymap->registerMapping(_remapAction, hardwareInput);

		_changes = true;
		stopRemapping();
	}

	if (_remapInputWatcher->isWatching() && g_system->getMillis() > _remapTimeout)
		stopRemapping();
	Dialog::handleTickle();
}

void RemapDialog::clearKeymap() {
	for (uint i = 0; i < _actions.size(); i++) {
		if (_actions[i].keyButton)   _scrollContainer->removeWidget(_actions[i].keyButton);
		if (_actions[i].actionText)  _scrollContainer->removeWidget(_actions[i].actionText);
		if (_actions[i].clearButton) _scrollContainer->removeWidget(_actions[i].clearButton);
		if (_actions[i].resetButton) _scrollContainer->removeWidget(_actions[i].resetButton);

		delete _actions[i].keyButton;
		delete _actions[i].actionText;
		delete _actions[i].clearButton;
		delete _actions[i].resetButton;
	}

	_actions.clear();
}

void RemapDialog::loadKeymap() {
	assert(_actions.empty());
	assert(_kmPopUp->getSelected() != -1);

	Keymap *km = _keymapTable[_kmPopUp->getSelectedTag()];
	for (Keymap::ActionArray::const_iterator it = km->getActions().begin(); it != km->getActions().end(); ++it) {
		ActionRow row;
		row.action = *it;

		_actions.push_back(row);
	}
}

void RemapDialog::refreshKeymap() {
	int clearButtonWidth = g_gui.xmlEval()->getVar("Globals.Line.Height");
	int clearButtonHeight = g_gui.xmlEval()->getVar("Globals.Line.Height");

	for (uint i = 0; i < _actions.size(); i++) {
		ActionRow &row = _actions[i];

		if (!row.actionText) {
			row.actionText = new GUI::StaticTextWidget(_scrollContainer, 0, 0, 0, 0, "", Graphics::kTextAlignLeft);
			row.keyButton = new GUI::ButtonWidget(_scrollContainer, 0, 0, 0, 0, "", 0, kRemapCmd + i);
			row.clearButton = addClearButton(_scrollContainer, "", kClearCmd + i, 0, 0, clearButtonWidth, clearButtonHeight);
			row.resetButton = new GUI::ButtonWidget(_scrollContainer, 0, 0, 0, 0, "", 0, kResetCmd + i);
		}

		row.actionText->setLabel(row.action->description);

		Keymap *keymap = row.action->getParent();

		Array<const HardwareInput *> mappedInputs = keymap->getActionMapping(row.action);

		String keysLabel;
		for (uint j = 0; j < mappedInputs.size(); j++) {
			if (!keysLabel.empty()) {
				keysLabel += ", ";
			}

			keysLabel += mappedInputs[j]->description;
		}

		if (!keysLabel.empty()) {
			row.keyButton->setLabel(keysLabel);
			row.keyButton->setTooltip(keysLabel);
		} else {
			row.keyButton->setLabel("-");
		}

		// I18N: Button to reset key mapping to defaults
		row.resetButton->setLabel(_("R"));
		row.resetButton->setTooltip(_("Reset to defaults"));
	}
}

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
