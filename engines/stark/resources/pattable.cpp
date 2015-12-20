/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/resources/pattable.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/string.h"

#include "engines/stark/services/stateprovider.h"

#include "engines/stark/formats/xrc.h"

namespace Stark {
namespace Resources {

PATTable::~PATTable() {
}

PATTable::PATTable(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_defaultAction(-1),
		_tooltipOverrideIndex(-1) {
	_type = TYPE;
}

void PATTable::readData(Formats::XRCReadStream *stream) {
	uint32 entryCount = stream->readUint32LE();
	for (uint i = 0; i < entryCount; i++) {
		Entry entry;

		entry._actionType = stream->readSint32LE();
		entry._scriptIndex = stream->readSint32LE();
		entry._script = nullptr;

		_ownEntries.push_back(entry);
	}

	_defaultAction = stream->readSint32LE();
}

void PATTable::printData() {
	for (uint i = 0; i < _ownEntries.size(); i++) {
		debug("entry[%d].actionType: %d", i, _ownEntries[i]._actionType);
		debug("entry[%d].scriptIndex: %d", i, _ownEntries[i]._scriptIndex);
	}
	debug("defaultAction: %d", _defaultAction);
}

void PATTable::onAllLoaded() {
	Object::onAllLoaded();

	_itemEntries.clear();
	addOwnEntriesToItemEntries();
}

void PATTable::onEnterLocation() {
	Object::onEnterLocation();

	_itemEntries.clear();

	// Add our own entries to the list of available actions
	addOwnEntriesToItemEntries();

	// If the PAT's owning item has a template, find it
	ItemTemplate *itemTemplate = findItemTemplate();

	// Add the item template actions to the list of available actions
	if (itemTemplate) {
		PATTable *templatePAT = itemTemplate->findChild<PATTable>();

		Common::Array<Entry> templateEntries = templatePAT->listItemEntries();
		for (uint i = 0; i < templateEntries.size(); i++) {
			if (!_itemEntries.contains(templateEntries[i]._actionType)) {
				_itemEntries[templateEntries[i]._actionType] = templateEntries[i];
			}
		}
	}
}

void PATTable::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsSint32LE(_tooltipOverrideIndex);

	if (serializer->isLoading() && _tooltipOverrideIndex >= 0) {
		String *string = findChildWithIndex<String>(_tooltipOverrideIndex);
		setTooltip(string);
	}
}

ItemTemplate *PATTable::findItemTemplate() {
	Item *parent = findParent<Item>();

	ItemTemplate *itemTemplate = nullptr;
	if (parent->getSubType() == Item::kItemModel) {
		ModelItem *item = Object::cast<ModelItem>(parent);
		itemTemplate = item->getItemTemplate();

	} else if (parent->getSubType() == Item::kItemLevelTemplate) {
		LevelItemTemplate *item = Object::cast<LevelItemTemplate>(parent);
		itemTemplate = item->getItemTemplate();
	}

	return itemTemplate;
}

void PATTable::addOwnEntriesToItemEntries() {
	for (uint i = 0; i < _ownEntries.size(); i++) {
		if (_ownEntries[i]._scriptIndex != -1) {
			Entry entry = _ownEntries[i];
			entry._script = findChildWithIndex<Script>(_ownEntries[i]._scriptIndex);
			_itemEntries[entry._actionType] = entry;
		}
	}
}

Common::Array<PATTable::Entry> PATTable::listItemEntries() const {
	Common::Array<PATTable::Entry> entries;

	for (EntryMap::const_iterator it = _itemEntries.begin(); it != _itemEntries.end(); it++) {
		entries.push_back(it->_value);
	}

	return entries;
}

ActionArray PATTable::listPossibleActions() const {
	ActionArray actions;

	for (EntryMap::const_iterator it = _itemEntries.begin(); it != _itemEntries.end(); it++) {
		// Check the script can be launched
		if (it->_value._script->shouldExecute(Script::kCallModePlayerAction)) {
			actions.push_back(it->_key);
		}
	}

	return actions;
}

bool PATTable::canPerformAction(uint32 action) const {
	if (_itemEntries.contains(action)) {
		return _itemEntries[action]._script->shouldExecute(Script::kCallModePlayerAction);
	}

	return false;
}

int32 PATTable::getDefaultAction() const {
	if (_defaultAction != -1 && canPerformAction(_defaultAction)) {
		return _defaultAction;
	} else {
		return -1;
	}
}

bool PATTable::runScriptForAction(uint32 action) {
	if (_itemEntries.contains(action)) {
		_itemEntries[action]._script->execute(Script::kCallModePlayerAction);
		return true;
	}

	return false;
}

void PATTable::setTooltip(String *string) {
	_name = string->getName();
	_tooltipOverrideIndex = string->getIndex();
}

} // End of namespace Resources
} // End of namespace Stark
