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

#ifdef ENABLE_KEYMAPPER

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
	kRemapCmd  = 'REMP',
	kClearCmd  = 'CLER',
	kResetCmd  = 'RSET',
	kCloseCmd  = 'CLOS',
	kReflowCmd = 'REFL'
};

RemapWidget::RemapWidget(GuiObject *boss, const Common::String &name, const KeymapArray &keymaps) :
		Widget(boss, name),
		_keymapTable(keymaps),
		_remapKeymap(nullptr),
		_remapAction(nullptr),
		_remapTimeout(0) {

	Keymapper *keymapper = g_system->getEventManager()->getKeymapper();
	assert(keymapper);

	EventDispatcher *eventDispatcher = g_system->getEventManager()->getEventDispatcher();
	_remapInputWatcher = new InputWatcher(eventDispatcher, keymapper);

	_scrollContainer = new GUI::ScrollContainerWidget(this, 0, 0, 0, 0, kReflowCmd);
	_scrollContainer->setTarget(this);
	_scrollContainer->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);
}

RemapWidget::~RemapWidget() {
	for (uint i = 0; i < _keymapTable.size(); i++) {
		delete _keymapTable[i];
	}
	delete _remapInputWatcher;
}

void RemapWidget::build() {
	debug(3, "RemapWidget::build keymaps: %d", _keymapTable.size());

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

void RemapWidget::reflowActionWidgets() {
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	int spacing = g_gui.xmlEval()->getVar("Globals.KeyMapper.Spacing");
	int keyButtonWidth = g_gui.xmlEval()->getVar("Globals.KeyMapper.ButtonWidth");
	int clearButtonWidth = g_gui.xmlEval()->getVar("Globals.Line.Height");
	int clearButtonHeight = g_gui.xmlEval()->getVar("Globals.Line.Height");
	int labelWidth = getWidth() - (spacing + keyButtonWidth + spacing + clearButtonWidth + spacing);

	uint textYOff = (buttonHeight - kLineHeight) / 2;
	uint clearButtonYOff = (buttonHeight - clearButtonHeight) / 2;

	uint y = spacing;

	Keymap *previousKeymap = nullptr;

	for (uint i = 0; i < _actions.size(); i++) {
		uint x;

		ActionRow &row = _actions[i];

		if (previousKeymap != row.keymap) {
			previousKeymap = row.keymap;

			// Insert a keymap separator
			x = 4 * spacing + keyButtonWidth + 2 * clearButtonWidth;

			GUI::StaticTextWidget *serarator = _keymapSeparators[row.keymap];
			serarator->resize(x, y, getWidth() - x - spacing, kLineHeight);

			y += kLineHeight + spacing;
		}

		x = spacing;

		row.keyButton->resize(x, y, keyButtonWidth, buttonHeight);

		x += keyButtonWidth + spacing;
		row.clearButton->resize(x, y + clearButtonYOff, clearButtonWidth, clearButtonHeight);

		x += clearButtonWidth + spacing;
		row.resetButton->resize(x, y + clearButtonYOff, clearButtonWidth, clearButtonHeight);

		x += clearButtonWidth + spacing;
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
	} else if (cmd >= kResetCmd && cmd < kResetCmd + _actions.size()) {
		resetMapping(cmd - kResetCmd);
	} else if (cmd == kReflowCmd) {
		reflowActionWidgets();
	} else {
		Widget::handleCommand(sender, cmd, data);
	}
}

void RemapWidget::clearMapping(uint i) {
	debug(3, "clear the mapping %u", i);
	Action *action = _actions[i].action;
	Keymap *keymap = _actions[i].keymap;
	keymap->unregisterMapping(action);

	_changes = true;

	stopRemapping();
	refreshKeymap();
}

void RemapWidget::resetMapping(uint i) {
	debug(3, "Reset the mapping %u", i);
	Action *action = _actions[i].action;
	Keymap *keymap = _actions[i].keymap;
	keymap->resetMapping(action);

	_changes = true;

	stopRemapping();
	refreshKeymap();
}

void RemapWidget::startRemapping(uint i) {
	if (_remapInputWatcher->isWatching()) {
		// Handle a second click on the button as a stop to remapping
		stopRemapping();
		return;
	}

	_remapKeymap = _actions[i].keymap;
	_remapAction = _actions[i].action;
	_remapTimeout = g_system->getMillis() + kRemapTimeoutDelay;
	_remapInputWatcher->startWatching();

	_actions[i].keyButton->setLabel("...");
	_actions[i].keyButton->markAsDirty();
}

void RemapWidget::stopRemapping() {
	_remapKeymap = nullptr;
	_remapAction = nullptr;

	refreshKeymap();

	_remapInputWatcher->stopWatching();
}

void RemapWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (_remapInputWatcher->isWatching())
		stopRemapping();
	else
		Widget::handleMouseDown(x, y, button, clickCount);
}

void RemapWidget::handleTickle() {
	const HardwareInput *hardwareInput = _remapInputWatcher->checkForCapturedInput();
	if (hardwareInput) {
		_remapKeymap->registerMapping(_remapAction, hardwareInput);

		_changes = true;
		stopRemapping();
	}

	if (_remapInputWatcher->isWatching() && g_system->getMillis() > _remapTimeout)
		stopRemapping();

	Widget::handleTickle();
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
	int clearButtonWidth = g_gui.xmlEval()->getVar("Globals.Line.Height");
	int clearButtonHeight = g_gui.xmlEval()->getVar("Globals.Line.Height");

	for (uint i = 0; i < _actions.size(); i++) {
		ActionRow &row = _actions[i];

		if (!row.actionText) {
			row.actionText = new GUI::StaticTextWidget(_scrollContainer, 0, 0, 0, 0, "", Graphics::kTextAlignLeft, nullptr, GUI::ThemeEngine::kFontStyleNormal);
			row.keyButton = new GUI::ButtonWidget(_scrollContainer, 0, 0, 0, 0, "", 0, kRemapCmd + i);
			row.clearButton = addClearButton(_scrollContainer, "", kClearCmd + i, 0, 0, clearButtonWidth, clearButtonHeight);
			row.resetButton = new GUI::ButtonWidget(_scrollContainer, 0, 0, 0, 0, "", 0, kResetCmd + i);
		}

		row.actionText->setLabel(row.action->description);

		Array<const HardwareInput *> mappedInputs = row.keymap->getActionMapping(row.action);

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

		if (!_keymapSeparators.contains(row.keymap)) {
			_keymapSeparators[row.keymap] = new GUI::StaticTextWidget(_scrollContainer, 0, 0, 0, 0, row.keymap->getName(), Graphics::kTextAlignLeft);
		}
	}
}

void RemapWidget::reflowLayout() {
	Widget::reflowLayout();

	_scrollContainer->resize(_x, _y, _w, _h);

	Widget *w = _firstWidget;
	while (w) {
		w->reflowLayout();
		w = w->next();
	}
}

GUI::Widget *RemapWidget::findWidget(int x, int y) {
	// Iterate over all child widgets and find the one which was clicked
	return Widget::findWidgetInChain(_firstWidget, x, y);
}

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
