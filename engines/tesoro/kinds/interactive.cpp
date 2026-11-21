/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tesoro/kinds/interactive.h"
#include "tesoro/world.h"

#include "tesoro/core/animbank.h"
#include "tesoro/core/namekey.h"

namespace Tesoro {

const SpeechTable::Line *InteractiveObject::defaultLine(const char *name) {
	return _host->phrase(name);
}

static const SpeechTable::Line *phraseAt(World &host, const ObjectRecord &record, uint offset) {
	char name[13];
	for (uint i = 0; i < 12; ++i)
		name[i] = (char)record.dataByte(offset + i);
	name[12] = '\0';
	return record.dataByte(offset) ? host.phrase(name) : nullptr;
}

void InteractiveObject::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host, uint16 stateSize, uint dataStart) {
	_host = &host;
	initAnimated(record, roomSortOffset, levelPlayers, stateSize);

	if (_anim.bankCount() > 0) {
		AnimPlayer &player = _anim.current();
		const bool image = player.bank() && player.bank()->step(player.current()).hasImage();
		_flags = (_flags & ~kVisible) | (image ? kVisible : 0) | kCollidable | kInRoomList | kHitter;
		const byte first = record.dataByte(dataStart);
		player.setRange(first, first, false, false, now);
	}

	_approachX = _x + (int16)(record.dataByte(dataStart + 2) | (record.dataByte(dataStart + 3) << 8));
	_approachY = _y + (int16)(record.dataByte(dataStart + 4) | (record.dataByte(dataStart + 5) << 8));
	_facing = record.dataByte(dataStart + 6) + 1;
	_hoverName = phraseAt(host, record, dataStart + 7);

	// The record lists the verb phrases in this order; each goes to its own verb.
	static const int order[kVerbs] = { 0, 1, 3, 4, 5, 2, 6 };
	// The original reads six of them; the seventh slot stays empty.
	for (int i = 0; i < 6; ++i)
		_verbLine[order[i]] = phraseAt(host, record, dataStart + 0x13 + 12 * i);
	_flags |= kHitter;
}

int InteractiveObject::runVerb() {
	if (_verb >= 0 && _verb < kVerbs)
		return verb(_verb);
	_host->returnPointerItem();
	return useItem(_verb - 7);
}

int InteractiveObject::update() {
	if (_pending && !_host->jimSpeaking()) {
		_host->setPointerActive(true);
		_pending = false;
		_host->removeFromMainList(this);
		if (!_answered)
			return runVerb();
		_answered = false;
	}
	updateBounds(_host->now());
	return 1;
}

int InteractiveObject::arrived() {
	_host->jimFace(_facing);
	return runVerb();
}

/**
 * Same as onClick but stores the verb
 */
int InteractiveObject::setVerbAndClick(int verb) {
	_verb = verb;
	onClick();
	return 1;
}

/**
 * Clicking on the object stops jims current line, makes him face the object,
 * and has him walk to it.
 */
void InteractiveObject::onClick() {
	if (_host->jimSpeaking())
		_host->jimStopTalking();
	if (_host->isJim(this)) {
		_facing = _host->jimFacing();
		arrived();
		return;
	}
	_host->jimForgetLine();
	_host->jimWalkTo(_approachX, _approachY, this);
	_host->jimUpdate();
}

int InteractiveObject::showHoverText() {
	if (_hoverName) {
		_host->setHoverText(_hoverName->str());
	} else {
		char name[13];
		nameFromKey(_nameKey, name);
		_host->setHoverText(name);
	}
	return 1;
}

int InteractiveObject::clearHoverText() {
	_host->setHoverText(nullptr);
	return 1;
}

int InteractiveObject::answer(const SpeechTable::Line *line) {
	_host->jimSayAnswer(*line);
	_host->setPointerActive(false);
	_pending = true;
	_host->insertInMainList(this, 5);
	return 3;
}

/**
 * Actually executes the verb associated with this object.
 */
int InteractiveObject::verb(int verb) {
	// Verbs 5 and 6 have no default: they do nothing and do not mark the click answered.
	if (verb < 0 || verb > 4)
		return 0;
	_answered = true;
	switch (verb) {
	case 0:
		return _verbLine[0] ? answer(_verbLine[0]) : 0;
	case 1:
		return answer(_verbLine[1] ? _verbLine[1] : defaultLine("OBJ125T03"));
	case 2:
		return answer(_verbLine[2] ? _verbLine[2] : defaultLine("OBJ096T07"));
	case 3:
		return answer(_verbLine[3] ? _verbLine[3] : defaultLine("OBJ106T05"));
	case 4:
		return answer(_verbLine[4] ? _verbLine[4] : defaultLine("OBJ106T05"));
	default:
		return 0;
	}
}

int InteractiveObject::useItem(int item) {
	_answered = true;
	const SpeechTable::Line *line = nullptr;
	if (!(_host->pointerHasItem() && _host->combinationReply(_nameKey, line)))
		line = defaultLine(item == 0 ? "ICON01T058" : "MODELO2");
	return answer(line);
}

} // End of namespace Tesoro
