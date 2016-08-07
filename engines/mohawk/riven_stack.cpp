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

#include "mohawk/riven_stack.h"

#include "mohawk/riven.h"
#include "mohawk/resource.h"

namespace Mohawk {

RivenStack::RivenStack(MohawkEngine_Riven *vm, uint16 id) :
		_vm(vm),
		_id(id) {
	loadResourceNames();
}

RivenStack::~RivenStack() {

}

uint16 RivenStack::getId() const {
	return _id;
}

void RivenStack::loadResourceNames() {
	_varNames = RivenNameList(_vm, kVariableNames);
	_externalCommandNames = RivenNameList(_vm, kExternalCommandNames);
	_stackNames = RivenNameList(_vm, kStackNames);
	_cardNames = RivenNameList(_vm, kCardNames);
	_hotspotNames = RivenNameList(_vm, kHotspotNames);
}

Common::String RivenStack::getName(RivenNameResource nameResource, uint16 nameId) const {
	switch (nameResource) {
		case kVariableNames:
			return _varNames.getName(nameId);
		case kExternalCommandNames:
			return _externalCommandNames.getName(nameId);
		case kStackNames:
			return _stackNames.getName(nameId);
		case kCardNames:
			return _cardNames.getName(nameId);
		case kHotspotNames:
			return _hotspotNames.getName(nameId);
		default:
			error("Unknown name resource %d", nameResource);
	}
}

int16 RivenStack::getIdFromName(RivenNameResource nameResource, const Common::String &name) const {
	switch (nameResource) {
		case kVariableNames:
			return _varNames.getNameId(name);
		case kExternalCommandNames:
			return _externalCommandNames.getNameId(name);
		case kStackNames:
			return _stackNames.getNameId(name);
		case kCardNames:
			return _cardNames.getNameId(name);
		case kHotspotNames:
			return _hotspotNames.getNameId(name);
		default:
			error("Unknown name resource %d", nameResource);
	}
}

RivenNameList::RivenNameList() {

}

RivenNameList::RivenNameList(MohawkEngine_Riven *vm, uint16 id) {
	loadResource(vm, id);
}

RivenNameList::~RivenNameList() {

}

void RivenNameList::loadResource(MohawkEngine_Riven *vm, uint16 id) {
	Common::SeekableReadStream *nameStream = vm->getResource(ID_NAME, id);

	uint16 namesCount = nameStream->readUint16BE();

	Common::Array<uint16> stringOffsets;
	stringOffsets.resize(namesCount);
	for (uint16 i = 0; i < namesCount; i++) {
		stringOffsets[i] = nameStream->readUint16BE();
	}

	_index.resize(namesCount);
	for (uint16 i = 0; i < namesCount; i++) {
		_index[i] = nameStream->readUint16BE();
	}

	int32 curNamesPos = nameStream->pos();

	_names.resize(namesCount);
	for (uint32 i = 0; i < namesCount; i++) {
		nameStream->seek(curNamesPos + stringOffsets[i]);

		Common::String name;
		for (char c = nameStream->readByte(); c; c = nameStream->readByte())
			name += c;

		_names[i] = name;
	}

	delete nameStream;
}

Common::String RivenNameList::getName(uint16 nameID) const {
	return _names[nameID];
}

int16 RivenNameList::getNameId(const Common::String &name) const {
	int low = 0;
	int high = _index.size() - 1;
	int midpoint = 0;

	// Binary search using the sorted _index array
	while (low <= high)	{
		midpoint = low + (high - low) / 2;

		const Common::String &midpointName = _names[_index[midpoint]];

		int comparison = name.compareToIgnoreCase(midpointName);
		if (comparison == 0) {
			return _index[midpoint];
		} else if (comparison < 0) {
			high = midpoint - 1;
		} else {
			low = midpoint + 1;
		}
	}

	return -1;
}

} // End of namespace Mohawk
