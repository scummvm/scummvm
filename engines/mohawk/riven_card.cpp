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

#include "mohawk/cursors.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_stack.h"
#include "mohawk/riven_video.h"

#include "mohawk/resource.h"
#include "mohawk/riven.h"

namespace Mohawk {

RivenCard::RivenCard(MohawkEngine_Riven *vm, uint16 id) :
	_vm(vm),
	_id(id),
	_hoveredHotspot(nullptr),
	_pressedHotspot(nullptr) {
	loadCardResource(id);
	loadHotspots(id);
	loadCardPictureList(id);
	loadCardSoundList(id);
	loadCardMovieList(id);
	loadCardHotspotEnableList(id);
	loadCardWaterEffectList(id);
}

RivenCard::~RivenCard() {
	for (uint i = 0; i < _hotspots.size(); i++) {
		delete _hotspots[i];
	}

	_vm->_gfx->clearWaterEffects();
	_vm->_gfx->clearFliesEffect();
	_vm->_video->closeVideos();
}

void RivenCard::loadCardResource(uint16 id) {
	Common::SeekableReadStream *inStream = _vm->getResource(ID_CARD, id);

	_name = inStream->readSint16BE();
	_zipModePlace = inStream->readUint16BE();
	_scripts = _vm->_scriptMan->readScripts(inStream);

	delete inStream;
}

void RivenCard::enter(bool unkMovies) {
	setCurrentCardVariable();

	_vm->_activatedPLST = false;
	_vm->_activatedSLST = false;

	_vm->_gfx->beginScreenUpdate();
	runScript(kCardLoadScript);
	defaultLoadScript();

	initializeZipMode();
	_vm->_gfx->applyScreenUpdate(true);

	runScript(kCardEnterScript);
}

void RivenCard::initializeZipMode() {
	if (_zipModePlace) {
		_vm->addZipVisitedCard(_id, _name);
	}

	// Check if a zip mode hotspot is enabled by checking the name/id against the ZIPS records.
	for (uint32 i = 0; i < _hotspots.size(); i++) {
		if (_hotspots[i]->isZip()) {
			if (_vm->_vars["azip"] != 0) {
				// Check if a zip mode hotspot is enabled by checking the name/id against the ZIPS records.
				Common::String hotspotName = _hotspots[i]->getName();
				bool visited = _vm->isZipVisitedCard(hotspotName);

				_hotspots[i]->enable(visited);
			} else // Disable the hotspot if zip mode is disabled
				_hotspots[i]->enable(false);
		}
	}
}

RivenScriptPtr RivenCard::getScript(uint16 scriptType) const {
	for (uint16 i = 0; i < _scripts.size(); i++)
		if (_scripts[i].type == scriptType) {
			return _scripts[i].script;
		}

	return RivenScriptPtr();
}

void RivenCard::runScript(uint16 scriptType) {
	RivenScriptPtr script = getScript(scriptType);
	_vm->_scriptMan->runScript(script, false);
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
		RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(1, kRivenCommandActivatePLST, 1, index);
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
		RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(1, kRivenCommandActivateSLST, 1, index);
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

void RivenCard::overrideSound(uint16 index, uint16 withIndex) {
	_soundList[index].soundIds = _soundList[withIndex].soundIds;
}

void RivenCard::loadHotspots(uint16 id) {
	Common::SeekableReadStream *inStream = _vm->getResource(ID_HSPT, id);

	uint16 hotspotCount = inStream->readUint16BE();
	_hotspots.resize(hotspotCount);

	for (uint16 i = 0; i < hotspotCount; i++) {
		_hotspots[i] = new RivenHotspot(_vm, inStream);
	}

	delete inStream;
}

void RivenCard::drawHotspotRects() {
	for (uint16 i = 0; i < _hotspots.size(); i++)
		_vm->_gfx->drawRect(_hotspots[i]->getRect(), _hotspots[i]->isEnabled());
}

RivenHotspot *RivenCard::getHotspotContainingPoint(const Common::Point &point) const {
	RivenHotspot *hotspot = nullptr;
	for (uint16 i = 0; i < _hotspots.size(); i++)
		if (_hotspots[i]->isEnabled() && _hotspots[i]->containsPoint(point)) {
			hotspot = _hotspots[i];
		}

	return hotspot;
}

Common::Array<RivenHotspot *> RivenCard::getHotspots() const {
	return _hotspots;
}

RivenHotspot *RivenCard::getHotspotByName(const Common::String &name) const {
	int16 nameId = _vm->getStack()->getIdFromName(kHotspotNames, name);

	for (uint i = 0; i < _hotspots.size(); i++) {
		if (_hotspots[i]->getNameId() == nameId) {
			return _hotspots[i];
		}
	}

	error("Card %d does not have an hotspot named %s", _id, name.c_str());
}

RivenHotspot *RivenCard::getHotspotByBlstId(const uint16 blstId) const {
	for (uint i = 0; i < _hotspots.size(); i++) {
		if (_hotspots[i]->getBlstId() == blstId) {
			return _hotspots[i];
		}
	}

	return nullptr;
}

void RivenCard::loadCardHotspotEnableList(uint16 id) {
	Common::SeekableReadStream *blst = _vm->getResource(ID_BLST, id);

	uint16 recordCount = blst->readUint16BE();
	_hotspotEnableList.resize(recordCount);

	for (uint16 i = 0; i < recordCount; i++) {
		HotspotEnableRecord &record = _hotspotEnableList[i];
		record.index = blst->readUint16BE();
		record.enabled = blst->readUint16BE();
		record.hotspotId = blst->readUint16BE();
	}

	delete blst;
}

void RivenCard::activateHotspotEnableRecord(uint16 index) {
	for (uint16 i = 0; i < _hotspotEnableList.size(); i++) {
		const HotspotEnableRecord &record = _hotspotEnableList[i];
		if (record.index == index) {
			RivenHotspot *hotspot = getHotspotByBlstId(record.hotspotId);
			hotspot->enable(record.enabled == 1);
			break;
		}
	}
}

void RivenCard::loadCardWaterEffectList(uint16 id) {
	Common::SeekableReadStream *flst = _vm->getResource(ID_FLST, id);

	uint16 recordCount = flst->readUint16BE();
	_waterEffectList.resize(recordCount);

	for (uint16 i = 0; i < recordCount; i++) {
		WaterEffectRecord &record = _waterEffectList[i];
		record.index = flst->readUint16BE();
		record.sfxeId = flst->readUint16BE();
		record.u0 = flst->readUint16BE();

		if (record.u0 != 0) {
			warning("FLST u0 non-zero");
		}
	}

	delete flst;
}

void RivenCard::activateWaterEffect(uint16 index) {
	for (uint16 i = 0; i < _waterEffectList.size(); i++) {
		const WaterEffectRecord &record = _waterEffectList[i];
		if (record.index == index) {
			_vm->_gfx->scheduleWaterEffect(record.sfxeId);
			break;
		}
	}
}

RivenHotspot *RivenCard::getCurHotspot() const {
	return _hoveredHotspot;
}

RivenScriptPtr RivenCard::onMouseDown(const Common::Point &mouse) {
	RivenScriptPtr script = onMouseMove(mouse);

	_pressedHotspot = _hoveredHotspot;

	if (_pressedHotspot) {
		script += _pressedHotspot->getScript(kMouseDownScript);
	}

	return script;
}

RivenScriptPtr RivenCard::onMouseUp(const Common::Point &mouse) {
	RivenScriptPtr script = onMouseMove(mouse);

	if (_pressedHotspot && _pressedHotspot == _hoveredHotspot) {
		script += _pressedHotspot->getScript(kMouseUpScript);
	}

	_pressedHotspot = nullptr;

	return script;
}

RivenScriptPtr RivenCard::onMouseMove(const Common::Point &mouse) {
	RivenHotspot *hotspot = getHotspotContainingPoint(mouse);

	RivenScriptPtr script = RivenScriptPtr(new RivenScript());

	// Detect hotspot exit
	if (_hoveredHotspot && (!hotspot || hotspot != _hoveredHotspot)) {
		script += _hoveredHotspot->getScript(kMouseLeaveScript);
	}

	// Detect hotspot entry
	if (hotspot && hotspot != _hoveredHotspot) {
		_hoveredHotspot = hotspot;
		script += _hoveredHotspot->getScript(kMouseEnterScript);
	}

	if (!hotspot) {
		_hoveredHotspot = nullptr;
	}

	return script;
}

RivenScriptPtr RivenCard::onMouseDragUpdate() {
	RivenScriptPtr script;
	if (_pressedHotspot) {
		script = _pressedHotspot->getScript(kMouseDragScript);
	}

	return script;
}

RivenScriptPtr RivenCard::onFrame() {
	return getScript(kCardFrameScript);
}

RivenScriptPtr RivenCard::onMouseUpdate() {
	RivenScriptPtr script;
	if (_hoveredHotspot) {
		script = _hoveredHotspot->getScript(kMouseInsideScript);
	}

	if (!script || script->empty()) {
		updateMouseCursor();
	}

	return script;
}

void RivenCard::updateMouseCursor() {
	uint16 cursor;
	if (_hoveredHotspot) {
		cursor = _hoveredHotspot->getMouseCursor();
	} else {
		cursor = kRivenMainCursor;
	}

	_vm->_cursor->setCursor(cursor);
}

void RivenCard::leave() {
	RivenScriptPtr script(new RivenScript());

	if (_pressedHotspot) {
		script += _pressedHotspot->getScript(kMouseUpScript);
	}

	if (_hoveredHotspot) {
		script += _hoveredHotspot->getScript(kMouseLeaveScript);
	}

	script += getScript(kCardLeaveScript);

	_vm->_scriptMan->runScript(script, false);
}

void RivenCard::setCurrentCardVariable() {
	_vm->_vars["currentcardid"] = _id;
}

void RivenCard::dump() const {
	debug("== Card ==");
	debug("id: %d", _id);
	if (_name >= 0) {
		debug("name: %s", _vm->getStack()->getName(kCardNames, _name).c_str());
	} else {
		debug("name: [no name]");
	}
	debug("zipModePlace: %d", _zipModePlace);
	debug("globalId: %x", _vm->getStack()->getCardGlobalId(_id));
	debugN("\n");

	for (uint i = 0; i < _scripts.size(); i++) {
		debug("== Script %d ==", i);
		debug("type: %s", RivenScript::getTypeName(_scripts[i].type));
		_scripts[i].script->dumpScript(0);
		debugN("\n");
	}

	for (uint i = 0; i < _hotspots.size(); i++) {
		debug("== Hotspot %d ==", i);
		_hotspots[i]->dump();
	}

	for (uint i = 0; i < _pictureList.size(); i++) {
		const Common::Rect &rect = _pictureList[i].rect;
		debug("== Picture %d ==", _pictureList[i].index);
		debug("pictureId: %d", _pictureList[i].id);
		debug("rect: (%d, %d, %d, %d)", rect.left, rect.top, rect.right, rect.bottom);
		debugN("\n");
	}

	for (uint i = 0; i < _waterEffectList.size(); i++) {
		debug("== Effect %d ==", _waterEffectList[i].index);
		debug("sfxeId: %d", _waterEffectList[i].sfxeId);
		debug("u0: %d", _waterEffectList[i].u0);
		debugN("\n");
	}

	for (uint i = 0; i < _hotspotEnableList.size(); i++) {
		debug("== Hotspot enable %d ==", _hotspotEnableList[i].index);
		debug("hotspotId: %d", _hotspotEnableList[i].hotspotId);
		debug("enabled: %d", _hotspotEnableList[i].enabled);
		debugN("\n");
	}

	for (uint i = 0; i < _soundList.size(); i++) {
		debug("== Ambient sound list %d ==", _soundList[i].index);
		debug("globalVolume: %d", _soundList[i].globalVolume);
		debug("fadeFlags: %d", _soundList[i].fadeFlags);
		debug("loop: %d", _soundList[i].loop);
		debug("suspend: %d", _soundList[i].suspend);
		debug("u0: %d", _soundList[i].u0);
		for (uint j = 0; j < _soundList[i].soundIds.size(); j++) {
			debug("sound[%d].id: %d", j, _soundList[i].soundIds[j]);
			debug("sound[%d].volume: %d", j, _soundList[i].volumes[j]);
			debug("sound[%d].balance: %d", j, _soundList[i].balances[j]);
			debug("sound[%d].u2: %d", j, _soundList[i].u2[j]);
		}
		debugN("\n");
	}

	for (uint i = 0; i < _movieList.size(); i++) {
		debug("== Movie %d ==", _movieList[i].index);
		debug("movieID: %d", _movieList[i].movieID);
		debug("playbackSlot: %d", _movieList[i].playbackSlot);
		debug("left: %d", _movieList[i].left);
		debug("top: %d", _movieList[i].top);
		debug("u0[0]: %d", _movieList[i].u0[0]);
		debug("u0[1]: %d", _movieList[i].u0[1]);
		debug("u0[2]: %d", _movieList[i].u0[2]);
		debug("loop: %d", _movieList[i].loop);
		debug("volume: %d", _movieList[i].volume);
		debug("u1: %d", _movieList[i].u1);
		debugN("\n");
	}
}

void RivenCard::loadCardMovieList(uint16 id) {
	Common::SeekableReadStream *mlstStream = _vm->getResource(ID_MLST, id);

	uint16 recordCount = mlstStream->readUint16BE();
	_movieList.resize(recordCount);

	for (uint16 i = 0; i < recordCount; i++) {
		MLSTRecord &mlstRecord = _movieList[i];
		mlstRecord.index = mlstStream->readUint16BE();
		mlstRecord.movieID = mlstStream->readUint16BE();
		mlstRecord.playbackSlot = mlstStream->readUint16BE();
		mlstRecord.left = mlstStream->readUint16BE();
		mlstRecord.top = mlstStream->readUint16BE();

		for (byte j = 0; j < 2; j++)
			if (mlstStream->readUint16BE() != 0)
				warning("u0[%d] in MLST non-zero", j);

		if (mlstStream->readUint16BE() != 0xFFFF)
			warning("u0[2] in MLST not 0xFFFF");

		mlstRecord.loop = mlstStream->readUint16BE();
		mlstRecord.volume = mlstStream->readUint16BE();
		mlstRecord.u1 = mlstStream->readUint16BE();

		if (mlstRecord.u1 != 1)
			warning("mlstRecord.u1 not 1");
	}

	delete mlstStream;
}

MLSTRecord RivenCard::getMovie(uint16 index) const {
	for (uint16 i = 0; i < _movieList.size(); i++) {
		if (_movieList[i].index == index) {
			return _movieList[i];
		}
	}

	error("Could not find movie %d in card %d", index, _id);
}

void RivenCard::playMovie(uint16 index, bool queue) {
	if (index > 0 && index <= _movieList.size()) {
		RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(1, kRivenCommandActivateMLSTAndPlay, 1, index);
		_vm->_scriptMan->runScript(script, queue);
	}
}

RivenHotspot::RivenHotspot(MohawkEngine_Riven *vm, Common::ReadStream *stream) :
		_vm(vm) {
	loadFromStream(stream);
}

void RivenHotspot::loadFromStream(Common::ReadStream *stream) {
	_flags = kFlagEnabled;

	_blstID = stream->readUint16BE();
	_nameResource = stream->readSint16BE();

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

	_rect = Common::Rect(left, top, right, bottom);

	_u0 = stream->readUint16BE();
	_mouseCursor = stream->readUint16BE();
	_index = stream->readUint16BE();
	_u1 = stream->readSint16BE();
	_flags |= stream->readUint16BE();

	// Read in the scripts now
	_scripts = _vm->_scriptMan->readScripts(stream);
}

RivenScriptPtr RivenHotspot::getScript(uint16 scriptType) const {
	for (uint16 i = 0; i < _scripts.size(); i++)
		if (_scripts[i].type == scriptType) {
			return _scripts[i].script;
		}

	return RivenScriptPtr();
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

Common::Rect RivenHotspot::getRect() const {
	return _rect;
}

bool RivenHotspot::containsPoint(const Common::Point &point) const {
	return _rect.contains(point);
}

uint16 RivenHotspot::getMouseCursor() const {
	return _mouseCursor;
}

Common::String RivenHotspot::getName() const {
	if (_nameResource < 0)
		return Common::String();

	return _vm->getStack()->getName(kHotspotNames, _nameResource);
}

uint16 RivenHotspot::getIndex() const {
	return _index;
}

uint16 RivenHotspot::getBlstId() const {
	return _blstID;
}

void RivenHotspot::setRect(const Common::Rect &rect) {
	_rect = rect;
}

int16 RivenHotspot::getNameId() const {
	return _nameResource;
}

void RivenHotspot::dump() const {
	debug("index: %d", _index);
	debug("blstId: %d", _blstID);
	debug("name: %s", getName().c_str());
	debug("rect: (%d, %d, %d, %d)", _rect.left, _rect.top, _rect.right, _rect.bottom);
	debug("flags: %d", _flags);
	debug("mouseCursor: %d", _mouseCursor);
	debug("u0: %d", _u0);
	debug("u1: %d", _u1);
	debugN("\n");

	for (uint i = 0; i < _scripts.size(); i++) {
		debug("=== Hotspot script %d ===", i);
		debug("type: %s", RivenScript::getTypeName(_scripts[i].type));
		_scripts[i].script->dumpScript(0);
		debugN("\n");
	}
}

} // End of namespace Mohawk
