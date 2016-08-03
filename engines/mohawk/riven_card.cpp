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

#include "mohawk/riven_card.h"

#include "mohawk/resource.h"
#include "mohawk/riven.h"

namespace Mohawk {

RivenCard::RivenCard(MohawkEngine_Riven *vm, uint16 id) :
	_vm(vm),
	_id(id) {
	loadCardResource(id);
}

RivenCard::~RivenCard() {

}

void RivenCard::loadCardResource(uint16 id) {
	Common::SeekableReadStream *inStream = _vm->getResource(ID_CARD, id);

	_name = inStream->readSint16BE();
	_zipModePlace = inStream->readUint16BE();
	_scripts = _vm->_scriptMan->readScripts(inStream);

	delete inStream;
}

void RivenCard::open() {
	initializeZipMode();
}

void RivenCard::initializeZipMode() {
	if (_zipModePlace) {
		_vm->addZipVisitedCard(_id, _name);
	}
}

void RivenCard::runScript(uint16 scriptType) {
	for (uint16 i = 0; i < _scripts.size(); i++)
		if (_scripts[i].type == scriptType) {
			RivenScriptPtr script = _scripts[i].script;
			_vm->_scriptMan->runScript(script, false);
			break;
		}
}

uint16 RivenCard::getId() const {
	return _id;
}

} // End of namespace Mohawk
