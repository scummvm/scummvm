#include "backends/keymapper/remap-dialog.h"
#include "gui/eval.h"
#include "gui/newgui.h"
#include "gui/PopUpWidget.h"

namespace Common {

enum {
	kRemapCmd = 'REMP'
};

RemapDialog::RemapDialog()
	: Dialog("remap"), _activeRemap(0) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_keymapper = g_system->getEventManager()->getKeymapper();
	assert(_keymapper);
	_activeKeymaps = &_keymapper->_activeMaps;

	int keymapCount = 0;
	_globalKeymaps = &_keymapper->_keymapMan->getGlobalDomain();
	if (_globalKeymaps->count() == 0)
		_globalKeymaps = 0;
	else
		keymapCount += _globalKeymaps->count();
	if (ConfMan.getActiveDomain() != 0) {
		_gameKeymaps = &_keymapper->_keymapMan->getGameDomain();
		if (_gameKeymaps->count() == 0)
			_gameKeymaps = 0;
		else
			keymapCount += _gameKeymaps->count();
	} else
		_gameKeymaps = 0;

	_keymapTable = (Keymap**)malloc(sizeof(Keymap*) * keymapCount);

	int labelWidth = g_gui.evaluator()->getVar("remap_popup_labelW");
	_kmPopUp = new GUI::PopUpWidget(this, "remap_popup", "Keymap:", labelWidth);

	if (_activeKeymaps->size() > 0) {
		_kmPopUp->appendEntry(_activeKeymaps->top().keymap->getName() + " (Active)");
	}
	KeymapManager::Domain::iterator it;
	uint32 idx = 0;

	if (_globalKeymaps) {
		_kmPopUp->appendEntry("");
		for (it = _globalKeymaps->begin(); it != _globalKeymaps->end(); it++) {
			_kmPopUp->appendEntry(it->_value->getName() + " (Global)", idx);
			_keymapTable[idx++] = it->_value;
		}
	}
	if (_gameKeymaps) {
		_kmPopUp->appendEntry("");
		for (it = _gameKeymaps->begin(); it != _gameKeymaps->end(); it++) {
			_kmPopUp->appendEntry(it->_value->getName() + " (Game)", idx);
			_keymapTable[idx++] = it->_value;
		}
	}

	_w = screenW - 2 * 20;
	_h = screenH - 2 * 20;
	// Center the dialog
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;
	
	_colCount = 2;
	_spacing = 10;
	_colWidth = (_w - (_colCount - 1) * _spacing) / _colCount;
	_widgetsY = g_gui.evaluator()->getVar("remap_widgetsY");
	if (g_gui.getWidgetSize() == GUI::kBigWidgetSize)
		_buttonHeight = GUI::kBigButtonHeight;
	else
		_buttonHeight = GUI::kButtonHeight;
	
}

RemapDialog::~RemapDialog() {
	free(_keymapTable);
}

void RemapDialog::open() {
	Dialog::open();
	
	_kmPopUp->setSelected(0);
	refreshKeymap();
}

void RemapDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd >= kRemapCmd && cmd < kRemapCmd + _keymapMappings.size()) {
		startRemapping(&_keymapMappings[cmd - kRemapCmd]);
	} else if (cmd == GUI::kPopUpItemSelectedCmd) {
		refreshKeymap();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

void RemapDialog::startRemapping(Mapping *remap) {
	_activeRemap = remap;
	_activeRemap->keyButton->setLabel("...");
	_keymapper->setEnabled(false);
}

void RemapDialog::stopRemapping() {
	refreshKeymap();
	_activeRemap = 0;
	_keymapper->setEnabled(true);
}

void RemapDialog::handleKeyDown(Common::KeyState state) {
	if (_activeRemap) {
		const HardwareKey *hwkey = _keymapper->getHardwareKey(state);
		if (hwkey) {
			_activeRemap->action->mapKey(hwkey);
			stopRemapping();
		}
	} else {
		GUI::Dialog::handleKeyDown(state);
	}

}

void RemapDialog::refreshKeymap() {
	if (_activeKeymaps->size() > 0 && _kmPopUp->getSelected() == 0) {
		// TODO: show active keymaps (with inherited mappings)
	} else {
		Keymap *km = _keymapTable[_kmPopUp->getSelectedTag()];
		List<Action*>& actions = km->getActions();
		setNumOfWidgets(actions.size());
		uint idx = 0;
		List<Action*>::iterator it;
		for (it = actions.begin(); it != actions.end(); it++, idx++) {
			Mapping& ma = _keymapMappings[idx];
			ma.action = *it;
			ma.actionText->setLabel(ma.action->description + ":");
			ma.keyButton->setLabel(ma.action->getMappedKey()->description);
		}
	}

}

void RemapDialog::setNumOfWidgets(uint newNum) {
	uint num = _keymapMappings.size();
	if (num < newNum) {
		uint textYOff = (_buttonHeight - kLineHeight) / 2;
		while (num < newNum) {
			uint x = (num % _colCount) * (_colWidth + _spacing);
			uint y = _widgetsY + (num / _colCount) * (_buttonHeight + _spacing);
			Mapping ma;
			ma.action = 0;
			ma.actionText = new GUI::StaticTextWidget(this, x, y + textYOff, 
				_colWidth / 2, kLineHeight, "", Graphics::kTextAlignRight);
			ma.keyButton = new GUI::ButtonWidget(this, x + _colWidth / 2,
				y, _colWidth / 2, _buttonHeight, "", kRemapCmd + num);
			_keymapMappings.push_back(ma);
			num++;
		}
	} else {
		while (num > newNum) {
			Mapping ma = _keymapMappings.remove_at(num - 1);
			removeWidget(ma.actionText);
			delete ma.actionText;
			removeWidget(ma.keyButton);
			delete ma.keyButton;
			num--;
		}
	}

}

} // end of namespace Common
