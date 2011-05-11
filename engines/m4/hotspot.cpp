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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "m4/m4.h"
#include "m4/events.h"
#include "m4/hotspot.h"
#include "gui/debugger.h"

namespace M4 {

/*
	HotSpot
*/

HotSpot::HotSpot(int x1, int y1, int x2, int y2) :
	_vocab(NULL), _verb(NULL), _prep(NULL), _sprite(NULL) {

	_rect.left = x1;
	_rect.top = y1;
	_rect.right = x2 + 1;
	_rect.bottom = y2 + 1;
	_active = true;

	_syntax = 0;
	_cursor = 0;
	_facing = 5;
	_feetX = 32767;
	_feetY = 32767;
}

HotSpot::~HotSpot() {
	free(_vocab);
}

void HotSpot::setRect(int x1, int y1, int x2, int y2) {
	_rect.left = x1;
	_rect.top = y1;
	_rect.right = x2 + 1;
	_rect.bottom = y2 + 1;
}

void HotSpot::setFeet(int x, int y) {
	_feetX = x;
	_feetY = y;
}

void HotSpot::setVocab(const char *value) {
	free(_vocab);
	_vocab = strdup(value);
}

void HotSpot::setVerb(const char *value) {
	free(_verb);
	_verb = strdup(value);
}

void HotSpot::setPrep(const char *value) {
	free(_prep);
	_prep = strdup(value);
}

void HotSpot::setSprite(const char *value) {
	free(_sprite);
	_sprite = strdup(value);
}

Common::Rect HotSpot::getRect() const {
	Common::Rect tempRect;
	tempRect.left = _rect.left;
	tempRect.top = _rect.top;
	tempRect.right = _rect.right - 1;
	tempRect.bottom = _rect.bottom - 1;

	return tempRect;
}

/*
	HotSpotList
*/

HotSpotList::HotSpotList() {
}

HotSpotList::~HotSpotList() {
	clear();
}

int HotSpotList::add(HotSpot *hotspot, bool head) {
	if (head || _hotspots.size() == 0) {
		_hotspots.insert_at(0, hotspot);
		return 0;
	} else {
		int32 area = hotspot->area();
		int index = _hotspots.size();
		for (uint i = 0; i < _hotspots.size(); i++) {
			if (area < _hotspots[i]->area()) {
				index = i;
				break;
			}
		}
		_hotspots.insert_at(index, hotspot);
		return index;
	}
}

void HotSpotList::remove(HotSpot *hotspot) {
	unlinkItem(hotspot);
	delete hotspot; //TODO: check this?
}

void HotSpotList::unlinkItem(HotSpot *hotspot) {
	uint index = 0;
	while (index < _hotspots.size()) {
		if (_hotspots[index] == hotspot) {
			_hotspots.remove_at(index);
		} else {
			index++;
		}
	}
}

void HotSpotList::clear() {
	for (uint i = 0; i < _hotspots.size(); i++)
		delete _hotspots[i];
	_hotspots.clear();
}

HotSpot *HotSpotList::findByXY(int x, int y) {
	for (uint i = 0; i < _hotspots.size(); i++) {
		if (_hotspots[i]->getActive() && _hotspots[i]->pointInside(x, y)) {
			return _hotspots[i];
		}
	}
	return NULL;
}

void HotSpotList::setActive(const char *name, bool active) {
	for (uint i = 0; i < _hotspots.size(); i++) {
		if (!scumm_stricmp(_hotspots[i]->_vocab, name)) {
			_hotspots[i]->setActive(active);
		}
	}
}

void HotSpotList::setActiveXY(const char *name, int x, int y, bool active) {
	for (uint i = 0; i < _hotspots.size(); i++) {
		if (_hotspots[i]->pointInside(x, y) && !scumm_stricmp(_hotspots[i]->_vocab, name)) {
			_hotspots[i]->setActive(active);
		}
	}
}

void HotSpotList::dump() {
	_vm->_events->getConsole()->DebugPrintf("%d hotspots in the list\n", _hotspots.size());

	for (uint index = 0; index < _hotspots.size(); index++) {
		_vm->_events->getConsole()->DebugPrintf("(%d): %p x1 = %d; y1 = %d; x2 = %d; y2 = %d\n",
			index, (void *)_hotspots[index],
			_hotspots[index]->_rect.left, _hotspots[index]->_rect.top,
			_hotspots[index]->_rect.right - 1, _hotspots[index]->_rect.bottom - 1);
	}
}

uint32 HotSpotList::readHotSpotInteger(Common::SeekableReadStream* hotspotStream) {
	if (_vm->isM4())
		return hotspotStream->readSint32LE();
	else
		return hotspotStream->readSint16LE();
}

void HotSpotList::loadHotSpots(Common::SeekableReadStream* hotspotStream, int hotspotCount) {
	HotSpot *currentHotSpot;
	uint32 x1, y1, x2, y2;
	char buffer[256];
	uint32 strLength = 0;
	uint32 index = 0;
	int feetX, feetY;
	int cursorOffset = (_vm ->isM4()) ? 0 : 1;

	for (int i = 0; i < hotspotCount; i++) {
		x1 = readHotSpotInteger(hotspotStream);
		y1 = readHotSpotInteger(hotspotStream);
		x2 = readHotSpotInteger(hotspotStream);
		y2 = readHotSpotInteger(hotspotStream);
		index = add(new HotSpot(x1, y1, x2, y2), i == 0);
		currentHotSpot = get(index);
		currentHotSpot->setIndex(index);
		feetX = readHotSpotInteger(hotspotStream);
		feetY = readHotSpotInteger(hotspotStream);
		currentHotSpot->setFeet(feetX, feetY);
		currentHotSpot->setFacing((uint8)hotspotStream->readByte());
		currentHotSpot->setActive(hotspotStream->readByte() != 0);

		if (!_vm->isM4())
			hotspotStream->readByte();				// unused (always 255)

		index = hotspotStream->readByte();		// cursor
		if (index == 0)
			currentHotSpot->setCursor(0);
		else
			currentHotSpot->setCursor(index - cursorOffset);

		// Rex Nebular doesn't have this field
		if (!_vm->isM4() && _vm->getGameType() != GType_RexNebular) {
			// This looks to be some sort of bitmask. Perhaps it signifies
			// the valid verbs for this hotspot
			index = hotspotStream->readUint16LE();		// unknown
			//debugC(kDebugCore, "%i ", index);
		}

		if (_vm->isM4())
			hotspotStream->readByte();					// syntax (unused)

		currentHotSpot->setVocabID(readHotSpotInteger(hotspotStream));
		currentHotSpot->setVerbID(readHotSpotInteger(hotspotStream));

		// Load hotspot related strings (verb, vocab/noun etc)
		// These are loaded inside the hotspot data in M4 games,
		// and inside the global vocab data (vocab.dat) in MADS games
		if (_vm->isM4()) {
			strLength = hotspotStream->readUint32LE();	// vocabLength
			hotspotStream->read(buffer, strLength);		// vocab (the hotspot's name)
			// Capitalize the hotspot's name here
			str_upper(buffer);
			currentHotSpot->setVocab(buffer);
			// Verbs are used internally by the game scripts in Orion Burger
			strLength = hotspotStream->readUint32LE();	// verbLength
			hotspotStream->read(buffer, strLength);		// verb
			// Capitalize the hotspot's verb here
			str_upper(buffer);
			currentHotSpot->setVerb(buffer);

			/* Hotspot names for non-English versions are stored in prep.
			   Prep can be set two ways: For English versions, copy the
			   text from vocab. For non-English versions, use the prep text
			   from the room file.
			*/
			strLength = hotspotStream->readUint32LE();	// prepLength
			hotspotStream->read(buffer, strLength);		// prep
			str_upper(buffer);

			if (strlen(buffer) > 0 && strcmp(buffer, "--") != 0 && strcmp(buffer, "ON") != 0)
				currentHotSpot->setPrep(buffer);
			else
				currentHotSpot->setPrep(currentHotSpot->getVocab());

			// The following values are not used at all by Orion Burger
			strLength = hotspotStream->readUint32LE();	// spriteLength
			hotspotStream->read(buffer, strLength);		// sprite
			hotspotStream->readUint16LE();				// sprite hash
		} else {
			currentHotSpot->setVocab("");
			currentHotSpot->setVerb("");

			if (currentHotSpot->getVocabID() > 0)
				currentHotSpot->setVocab(_madsVm->globals()->getVocab(currentHotSpot->getVocabID()));

			if (currentHotSpot->getVerbID() > 0)
				currentHotSpot->setVerb(_madsVm->globals()->getVocab(currentHotSpot->getVerbID()));
		}
	}
}

} // End of namespace M4
