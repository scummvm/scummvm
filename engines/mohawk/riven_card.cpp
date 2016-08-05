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

#include "mohawk/resource.h"
#include "mohawk/riven.h"

namespace Mohawk {

RivenCard::RivenCard(MohawkEngine_Riven *vm, uint16 id) :
	_vm(vm),
	_id(id) {
	loadCardResource(id);
	loadCardPictureList(id);
	loadCardSoundList(id);
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

	_vm->_gfx->beginScreenUpdate();
	runScript(kCardLoadScript);
	defaultLoadScript();

	initializeZipMode();
	_vm->_gfx->applyScreenUpdate(true);

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
		playSound(1);
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

void RivenCard::loadCardSoundList(uint16 id) {
	Common::SeekableReadStream *slstStream = _vm->getResource(ID_SLST, id);

	uint16 recordCount = slstStream->readUint16BE();
	_soundList.resize(recordCount);

	for (uint16 i = 0; i < recordCount; i++) {
		SLSTRecord &slstRecord = _soundList[i];
		slstRecord.index = slstStream->readUint16BE();

		uint16 soundCount = slstStream->readUint16BE();

		slstRecord.soundIds.resize(soundCount);
		for (uint16 j = 0; j < soundCount; j++)
			slstRecord.soundIds[j] = slstStream->readUint16BE();

		slstRecord.fadeFlags = slstStream->readUint16BE();
		slstRecord.loop = slstStream->readUint16BE();
		slstRecord.globalVolume = slstStream->readUint16BE();
		slstRecord.u0 = slstStream->readUint16BE();			// Unknown

		if (slstRecord.u0 > 1)
			warning("slstRecord.u0: %d non-boolean", slstRecord.u0);

		slstRecord.suspend = slstStream->readUint16BE();

		if (slstRecord.suspend != 0)
			warning("slstRecord.suspend: %d non-zero", slstRecord.suspend);

		slstRecord.volumes.resize(soundCount);
		slstRecord.balances.resize(soundCount);
		slstRecord.u2.resize(soundCount);

		for (uint16 j = 0; j < soundCount; j++)
			slstRecord.volumes[j] = slstStream->readUint16BE();

		for (uint16 j = 0; j < soundCount; j++)
			slstRecord.balances[j] = slstStream->readSint16BE();	// negative = left, 0 = center, positive = right

		for (uint16 j = 0; j < soundCount; j++) {
			slstRecord.u2[j] = slstStream->readUint16BE();		// Unknown

			if (slstRecord.u2[j] != 255 && slstRecord.u2[j] != 256)
				warning("slstRecord.u2[%d]: %d not 255 or 256", j, slstRecord.u2[j]);
		}
	}

	delete slstStream;
}

void RivenCard::playSound(uint16 index, bool queue) {
	if (index > 0 && index <= _soundList.size()) {
		RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(1, 40, 1, index);
		_vm->_scriptMan->runScript(script, queue);
	}
}

SLSTRecord RivenCard::getSound(uint16 index) const {
	for (uint16 i = 0; i < _soundList.size(); i++) {
		if (_soundList[i].index == index) {
			return _soundList[i];
		}
	}

	error("Could not find sound %d in card %d", index, _id);
}

RivenHotspot::RivenHotspot(MohawkEngine_Riven *vm, Common::ReadStream *stream) :
		_vm(vm) {
	loadFromStream(stream);
}

void RivenHotspot::loadFromStream(Common::ReadStream *stream) {
	_flags = kFlagEnabled;

	blstID = stream->readUint16BE();
	name_resource = stream->readSint16BE();

	int16 left = stream->readSint16BE();
	int16 top = stream->readSint16BE();
	int16 right = stream->readSint16BE();
	int16 bottom = stream->readSint16BE();

	// Riven has some invalid rects, disable them here
	// Known weird hotspots:
	// - tspit 371 (DVD: 377), hotspot 4
	if (left >= right || top >= bottom) {
		warning("Invalid hotspot: (%d, %d, %d, %d)", left, top, right, bottom);
		left = top = right = bottom = 0;
		enable(false);
	}

	rect = Common::Rect(left, top, right, bottom);

	_u0 = stream->readUint16BE();
	mouse_cursor = stream->readUint16BE();
	index = stream->readUint16BE();
	_u1 = stream->readSint16BE();
	_flags |= stream->readUint16BE();

	// Read in the scripts now
	_scripts = _vm->_scriptMan->readScripts(stream);
}

void RivenHotspot::runScript(uint16 scriptType) {
	for (uint16 i = 0; i < _scripts.size(); i++)
		if (_scripts[i].type == scriptType) {
			RivenScriptPtr script = _scripts[i].script;
			_vm->_scriptMan->runScript(script, false);
			break;
		}
}

bool RivenHotspot::isEnabled() const {
	return (_flags & kFlagEnabled) != 0;
}

void RivenHotspot::enable(bool e) {
	if (e) {
		_flags |= kFlagEnabled;
	} else {
		_flags &= ~kFlagEnabled;
	}
}

bool RivenHotspot::isZip() const {
	return (_flags & kFlagZip) != 0;
}

} // End of namespace Mohawk
