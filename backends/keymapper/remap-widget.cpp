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

#include "backends/keymapper/remap-widget.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/input-watcher.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"

#include "common/system.h"
#include "gui/gui-manager.h"
#include "gui/widgets/scrollcontainer.h"
#include "gui/ThemeEval.h"
#include "common/translation.h"

namespace Common {

enum {
	kRemapCmd        = 'REMP',
	kClearCmd        = 'CLER',
	kResetActionCmd  = 'RTAC',
	kResetKeymapCmd  = 'RTKM',
	kCloseCmd        = 'CLOS'
};

RemapWidget::RemapWidget(GuiObject *boss, const Common::String &name, const KeymapArray &keymaps) :
		OptionsContainerWidget(boss, name, "", true, ""),
		_keymapTable(keymaps),
		_remapKeymap(nullptr),
		_remapAction(nullptr),
		_remapTimeout(0) {

	Keymapper *keymapper = g_system->getEventManager()->getKeymapper();
	assert(keymapper);

	EventDispatcher *eventDispatcher = g_system->getEventManager()->getEventDispatcher();
	_remapInputWatcher = new InputWatcher(eventDispatcher, keymapper);
}

RemapWidget::~RemapWidget() {
	for (uint i = 0; i < _keymapTable.size(); i++) {
		delete _keymapTable[i];
	}
	delete _remapInputWatcher;
}

void RemapWidget::load() {
	debug(3, "RemapWidget::load keymaps: %d", _keymapTable.size());

	_changes = false;

	loadKeymap();
	refreshKeymap();
	reflowActionWidgets();
}

bool RemapWidget::save() {
	bool changes = _changes;

	if (_changes) {
		for (uint i = 0; i < _keymapTable.size(); i++) {
			_keymapTable[i]->saveMappings();
		}
		_changes = false;
	}

	return changes;
}

void RemapWidget::handleInputChanged() {
	Keymapper *keymapper = g_system->getEventManager()->getKeymapper();
	assert(keymapper);

	for (uint i = 0; i < _keymapTable.size(); i++) {
		keymapper->reloadKeymapMappings(_keymapTable[i]);
	}

	refreshKeymap();
}

void RemapWidget::reflowActionWidgets() {
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	int spacing = g_gui.xmlEval()->getVar("Globals.KeyMapper.Spacing");
	int keyButtonWidth = g_gui.xmlEval()->getVar("Globals.KeyMapper.ButtonWidth");
	int resetButtonWidth = g_gui.xmlEval()->getVar("Globals.KeyMapper.ResetWidth");
	int labelWidth = getWidth() - (spacing + keyButtonWidth + spacing);
	labelWidth = MAX(0, labelWidth);

	uint textYOff = (buttonHeight - kLineHeight) / 2;

	uint y = spacing;

	Keymap *previousKeymap = nullptr;

	for (uint i = 0; i < _actions.size(); i++) {
		uint x;

		ActionRow &row = _actions[i];

		if (previousKeymap != row.keymap) {
			previousKeymap = row.keymap;

			// Insert a keymap separator
			x = 2 * spacing + keyButtonWidth;

			KeymapTitleRow keymapTitle = _keymapSeparators[row.keymap];
			if (keymapTitle.descriptionText) {
				int descriptionWidth = getWidth() - x - spacing - resetButtonWidth - spacing;
				descriptionWidth = MAX(0, descriptionWidth);

				keymapTitle.descriptionText->resize(x, y + textYOff, descriptionWidth, kLineHeight);
				keymapTitle.resetButton->resize(x + descriptionWidth, y, resetButtonWidth, buttonHeight);
			}

			y += buttonHeight + spacing;
		}

		x = spacing;

		row.keyButton->resize(x, y, keyButtonWidth, buttonHeight);

		x += keyButtonWidth + spacing;
		row.actionText->resize(x, y + textYOff, labelWidth, kLineHeight);

		y += buttonHeight + spacing;
	}
}

void RemapWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	debug(3, "RemapWidget::handleCommand %u %u", cmd, data);

	if (cmd >= kRemapCmd && cmd < kRemapCmd + _actions.size()) {
		startRemapping(cmd - kRemapCmd);
	} else if (cmd >= kClearCmd && cmd < kClearCmd + _actions.size()) {
		clearMapping(cmd - kClearCmd);
	} else if (cmd >= kResetActionCmd && cmd < kResetActionCmd + _actions.size()) {
		resetMapping(cmd - kResetActionCmd);
	} else if (cmd >= kResetKeymapCmd && cmd < kResetKeymapCmd + _actions.size()) {
		resetKeymap(cmd - kResetKeymapCmd);
	} else if (cmd == kReflowCmd) {
		reflowActionWidgets();
	} else {
		OptionsContainerWidget::handleCommand(sender, cmd, data);
	}
}

void RemapWidget::clearMapping(uint actionIndex) {
	debug(3, "clear the mapping %u", actionIndex);
	Action *action = _actions[actionIndex].action;
	Keymap *keymap = _actions[actionIndex].keymap;
	keymap->unregisterMapping(action);

	_changes = true;

	stopRemapping();
	refreshKeymap();
}

void RemapWidget::resetMapping(uint actionIndex) {
	debug(3, "Reset the mapping %u", actionIndex);
	Action *action = _actions[actionIndex].action;
	Keymap *keymap = _actions[actionIndex].keymap;
	keymap->resetMapping(action);

	_changes = true;

	stopRemapping();
	refreshKeymap();
}

void RemapWidget::resetKeymap(uint actionIndex) {
	debug(3, "Reset the keymap %u", actionIndex);
	Keymap *keymap = _actions[actionIndex].keymap;

	for (uint i = 0; i < _actions.size(); i++) {
		ActionRow &row = _actions[i];
		if (row.keymap == keymap) {
			keymap->resetMapping(row.action);
		}
	}

	_changes = true;

	stopRemapping();
	refreshKeymap();
}

void RemapWidget::startRemapping(uint actionIndex) {
	if (_remapInputWatcher->isWatching()) {
		// Handle a second click on the button as a stop to remapping
		stopRemapping();
		return;
	}

	_remapKeymap = _actions[actionIndex].keymap;
	_remapAction = _actions[actionIndex].action;

	uint32 remapTimeoutDelay = kRemapMinTimeoutDelay;
	if (ConfMan.hasKey("remap_timeout_delay_ms") && ((uint32)ConfMan.getInt("remap_timeout_delay_ms") > kRemapMinTimeoutDelay)) {
		remapTimeoutDelay = (uint32)ConfMan.getInt("remap_timeout_delay_ms");
	}
	_remapTimeout = g_system->getMillis() + remapTimeoutDelay;
	_remapInputWatcher->startWatching();

	_actions[actionIndex].keyButton->setLabel("...");
	_actions[actionIndex].keyButton->setTooltip("");
	_actions[actionIndex].keyButton->markAsDirty();

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
}

void RemapWidget::stopRemapping() {
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	_remapKeymap = nullptr;
	_remapAction = nullptr;

	refreshKeymap();

	_remapInputWatcher->stopWatching();
}

void RemapWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (_remapInputWatcher->isWatching())
		stopRemapping();
	else
		OptionsContainerWidget::handleMouseDown(x, y, button, clickCount);
}

void RemapWidget::handleTickle() {
	const HardwareInput hardwareInput = _remapInputWatcher->checkForCapturedInput();
	if (hardwareInput.type != kHardwareInputTypeInvalid) {
		_remapKeymap->registerMapping(_remapAction, hardwareInput);

		_changes = true;
		stopRemapping();
	}

	if (_remapInputWatcher->isWatching() && g_system->getMillis() > _remapTimeout)
		stopRemapping();

	OptionsContainerWidget::handleTickle();
}

void RemapWidget::loadKeymap() {
	assert(_actions.empty());

	for (KeymapArray::const_iterator km = _keymapTable.begin(); km != _keymapTable.end(); km++) {
		for (Keymap::ActionArray::const_iterator it = (*km)->getActions().begin(); it != (*km)->getActions().end(); ++it) {
			ActionRow row;
			row.keymap = *km;
			row.action = *it;

			_actions.push_back(row);
		}
	}
}

void RemapWidget::refreshKeymap() {
	for (uint i = 0; i < _actions.size(); i++) {
		ActionRow &row = _actions[i];

		if (!row.actionText) {
			row.actionText = new GUI::StaticTextWidget(widgetsBoss(), 0, 0, 0, 0, U32String(), Graphics::kTextAlignStart, U32String(), GUI::ThemeEngine::kFontStyleNormal);
			row.actionText->setLabel(row.action->description);

			row.keyButton = new GUI::DropdownButtonWidget(widgetsBoss(), 0, 0, 0, 0, U32String(), U32String(), kRemapCmd + i);
			row.keyButton->appendEntry(_("Reset to defaults"), kResetActionCmd + i);
			row.keyButton->appendEntry(_("Clear mapping"), kClearCmd + i);
		}

		Array<HardwareInput> mappedInputs = row.keymap->getActionMapping(row.action);

		U32String keysLabel;
		for (uint j = 0; j < mappedInputs.size(); j++) {
			if (!keysLabel.empty()) {
				keysLabel += Common::U32String(", ");
			}

			keysLabel += mappedInputs[j].description;
		}

		if (!keysLabel.empty()) {
			row.keyButton->setLabel(keysLabel);
			row.keyButton->setTooltip(keysLabel);
		} else {
			row.keyButton->setLabel("-");
			row.keyButton->setTooltip("");
		}

		KeymapTitleRow &keymapTitle = _keymapSeparators[row.keymap];
		if (!keymapTitle.descriptionText) {
			keymapTitle.descriptionText = new GUI::StaticTextWidget(widgetsBoss(), 0, 0, 0, 0, row.keymap->getDescription(), Graphics::kTextAlignStart);
			keymapTitle.resetButton = new GUI::ButtonWidget(widgetsBoss(), 0, 0, 0, 0, U32String(), U32String(), kResetKeymapCmd + i);

			// I18N: Button to reset keymap mappings to defaults
			keymapTitle.resetButton->setLabel(_("Reset"));
			keymapTitle.resetButton->setTooltip(_("Reset to defaults"));
		}
	}
}

} // End of namespace Common
