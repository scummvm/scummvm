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
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/input-watcher.h"

#include "common/system.h"
#include "gui/gui-manager.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollbar.h"
#include "gui/ThemeEval.h"
#include "common/translation.h"

namespace Common {

enum {
	kRemapCmd = 'REMP',
	kClearCmd = 'CLER',
	kCloseCmd = 'CLOS'
};

RemapDialog::RemapDialog()
	: Dialog("KeyMapper"), _topAction(0), _remapTimeout(0), _topKeymapIsGui(false), _remapAction(nullptr) {

	_keymapper = g_system->getEventManager()->getKeymapper();
	assert(_keymapper);

	EventDispatcher *eventDispatcher = g_system->getEventManager()->getEventDispatcher();
	_remapInputWatcher = new InputWatcher(eventDispatcher, _keymapper);

	_kmPopUpDesc = new GUI::StaticTextWidget(this, "KeyMapper.PopupDesc", _("Keymap:"));
	_kmPopUp = new GUI::PopUpWidget(this, "KeyMapper.Popup");

	_scrollBar = new GUI::ScrollBarWidget(this, 0, 0, 0, 0);

	GUI::ContainerWidget *keymapArea = new GUI::ContainerWidget(this, "KeyMapper.KeymapArea");
	keymapArea->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);

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

	Dialog::open();

	_kmPopUp->setSelected(0);
	loadKeymap();
}

void RemapDialog::close() {
	_kmPopUp->clearEntries();

	if (_changes)
		ConfMan.flushToDisk();

	Dialog::close();
}

void RemapDialog::reflowLayout() {
	Dialog::reflowLayout();

	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);
	int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);

	int16 areaX, areaY;
	uint16 areaW, areaH;
	g_gui.xmlEval()->getWidgetData((const String&)String("KeyMapper.KeymapArea"), areaX, areaY, areaW, areaH);

	int spacing = g_gui.xmlEval()->getVar("Globals.KeyMapper.Spacing");
	int keyButtonWidth = g_gui.xmlEval()->getVar("Globals.KeyMapper.ButtonWidth");
	int clearButtonWidth = g_gui.xmlEval()->getVar("Globals.Line.Height");
	int clearButtonHeight = g_gui.xmlEval()->getVar("Globals.Line.Height");

	int colWidth = areaW - scrollbarWidth;
	int labelWidth =  colWidth - (keyButtonWidth + spacing + clearButtonWidth + spacing);

	_rowCount = (areaH + spacing) / (buttonHeight + spacing);
	debug(7, "rowCount = %d" , _rowCount);
	if (colWidth <= 0  || _rowCount <= 0)
		error("Remap dialog too small to display any keymaps");

	_scrollBar->resize(areaX + areaW - scrollbarWidth, areaY, scrollbarWidth, areaH);
	_scrollBar->_entriesPerPage = _rowCount;
	_scrollBar->_numEntries = 1;
	_scrollBar->recalc();

	uint textYOff = (buttonHeight - kLineHeight) / 2;
	uint clearButtonYOff = (buttonHeight - clearButtonHeight) / 2;
	uint oldSize = _keymapWidgets.size();
	uint newSize = _rowCount;

	_keymapWidgets.reserve(newSize);

	for (uint i = 0; i < newSize; i++) {
		ActionWidgets widg;

		if (i >= _keymapWidgets.size()) {
			widg.actionText =
				new GUI::StaticTextWidget(this, 0, 0, 0, 0, "", Graphics::kTextAlignLeft);
			widg.keyButton =
				new GUI::ButtonWidget(this, 0, 0, 0, 0, "", 0, kRemapCmd + i);
			widg.clearButton = addClearButton(this, "", kClearCmd + i, 0, 0, clearButtonWidth, clearButtonHeight);
			_keymapWidgets.push_back(widg);
		} else {
			widg = _keymapWidgets[i];
		}

		uint x = areaX;
		uint y = areaY + (i) * (buttonHeight + spacing);

		widg.keyButton->resize(x, y, keyButtonWidth, buttonHeight);
		widg.clearButton->resize(x + keyButtonWidth + spacing, y + clearButtonYOff, clearButtonWidth, clearButtonHeight);
		widg.actionText->resize(x + keyButtonWidth + spacing + clearButtonWidth + spacing, y + textYOff, labelWidth, kLineHeight);

	}
	while (oldSize > newSize) {
		ActionWidgets widg = _keymapWidgets.remove_at(--oldSize);

		removeWidget(widg.actionText);
		delete widg.actionText;

		removeWidget(widg.keyButton);
		delete widg.keyButton;

		removeWidget(widg.clearButton);
		delete widg.clearButton;
	}
}

void RemapDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	debug(3, "RemapDialog::handleCommand %u %u", cmd, data);

	if (cmd >= kRemapCmd && cmd < kRemapCmd + _keymapWidgets.size()) {
		startRemapping(cmd - kRemapCmd);
	} else if (cmd >= kClearCmd && cmd < kClearCmd + _keymapWidgets.size()) {
		clearMapping(cmd - kClearCmd);
	} else if (cmd == GUI::kPopUpItemSelectedCmd) {
		loadKeymap();
	} else if (cmd == GUI::kSetPositionCmd) {
		refreshKeymap();
	} else if (cmd == kCloseCmd) {
		close();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

void RemapDialog::clearMapping(uint i) {
	if (_topAction + i >= _currentActions.size())
		return;

	debug(3, "clear the mapping %u", i);
	Action *activeRemapAction = _currentActions[_topAction + i];
	_keymapper->clearMapping(activeRemapAction);
	_changes = true;

	stopRemapping();
	refreshKeymap();
}

void RemapDialog::startRemapping(uint i) {
	if (_topAction + i >= _currentActions.size())
		return;

	if (_remapInputWatcher->isWatching()) {
		// Handle a second click on the button as a stop to remapping
		stopRemapping();
		return;
	}

	_remapAction = _currentActions[_topAction + i];
	_remapTimeout = g_system->getMillis() + kRemapTimeoutDelay;
	_remapInputWatcher->startWatching();

	_keymapWidgets[i].keyButton->setLabel("...");
	_keymapWidgets[i].keyButton->markAsDirty();
}

void RemapDialog::stopRemapping() {
	_topAction = -1;
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
		_keymapper->registerMapping(_remapAction, hardwareInput);

		_changes = true;
		stopRemapping();
	}

	if (_remapInputWatcher->isWatching() && g_system->getMillis() > _remapTimeout)
		stopRemapping();
	Dialog::handleTickle();
}

void RemapDialog::loadKeymap() {
	_currentActions.clear();

	if (_kmPopUp->getSelected() != -1) {
		// This is the regular view of a keymap that isn't the topmost one.
		// It shows all of that keymap's actions

		Keymap *km = _keymapTable[_kmPopUp->getSelectedTag()];

		List<Action *>::iterator it;

		for (it = km->getActions().begin(); it != km->getActions().end(); ++it) {
			_currentActions.push_back(*it);
		}
	}

	// refresh scroll bar
	_scrollBar->_currentPos = 0;
	_scrollBar->_numEntries = _currentActions.size();
	_scrollBar->recalc();

	// force refresh
	_topAction = -1;
	refreshKeymap();
}

void RemapDialog::refreshKeymap() {
	int newTopAction = _scrollBar->_currentPos;

	if (newTopAction == _topAction)
		return;

	_topAction = newTopAction;

	//_container->markAsDirty();
	_scrollBar->markAsDirty();

	uint actionI = _topAction;

	for (uint widgetI = 0; widgetI < _keymapWidgets.size(); widgetI++) {
		ActionWidgets &widg = _keymapWidgets[widgetI];

		if (actionI < _currentActions.size()) {
			debug(8, "RemapDialog::refreshKeymap actionI=%u", actionI);
			Action *action = _currentActions[actionI];

			widg.actionText->setLabel(action->description);

			Keymap *keymap = action->getParent();

			const HardwareInput *mappedInput = keymap->getActionMapping(action);
			if (mappedInput)
				widg.keyButton->setLabel(mappedInput->description);
			else
				widg.keyButton->setLabel("-");

			widg.actionText->setVisible(true);
			widg.keyButton->setVisible(true);
			widg.clearButton->setVisible(true);

			actionI++;
		} else {
			widg.actionText->setVisible(false);
			widg.keyButton->setVisible(false);
			widg.clearButton->setVisible(false);
		}
	}

	// need to redraw entire Dialog so that invisible widgets disappear
	g_gui.scheduleTopDialogRedraw();
}


} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
