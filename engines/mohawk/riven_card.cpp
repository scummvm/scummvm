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

#include "mohawk/riven_graphics.h"
#include "mohawk/sound.h"

#include "mohawk/resource.h"
#include "mohawk/riven.h"

namespace Mohawk {

RivenCard::RivenCard(MohawkEngine_Riven *vm, uint16 id) :
	_vm(vm),
	_id(id) {
	loadCardResource(id);
	loadCardPictureList(id);
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
	_vm->_activatedPLST = false;
	_vm->_activatedSLST = false;

	runScript(kCardLoadScript);
	defaultLoadScript();

	initializeZipMode();
	_vm->_gfx->updateScreen();

	runScript(kCardOpenScript);
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

void RivenCard::defaultLoadScript() {
	// Activate the first picture list if none have been activated
	if (!_vm->_activatedPLST)
		drawPicture(1);

	// Activate the first sound list if none have been activated
	if (!_vm->_activatedSLST)
		_vm->_sound->playSLST(1, _id);
}

void RivenCard::loadCardPictureList(uint16 id) {
	Common::SeekableReadStream* plst = _vm->getResource(ID_PLST, id);
	uint16 recordCount = plst->readUint16BE();
	_pictureList.resize(recordCount);

	for (uint16 i = 0; i < recordCount; i++) {
		Picture &picture = _pictureList[i];
		picture.index = plst->readUint16BE();
		picture.id = plst->readUint16BE();
		picture.rect.left = plst->readUint16BE();
		picture.rect.top = plst->readUint16BE();
		picture.rect.right = plst->readUint16BE();
		picture.rect.bottom = plst->readUint16BE();
	}

	delete plst;
}

void RivenCard::drawPicture(uint16 index, bool queue) {
	if (index > 0 && index <= _pictureList.size()) {
		RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(1, 39, 1, index);
		_vm->_scriptMan->runScript(script, queue);
	}
}

RivenCard::Picture RivenCard::getPicture(uint16 index) const {
	for (uint16 i = 0; i < _pictureList.size(); i++) {
		if (_pictureList[i].index == index) {
			return _pictureList[i];
		}
	}

	error("Could not find picture %d in card %d", index, _id);
}

} // End of namespace Mohawk
