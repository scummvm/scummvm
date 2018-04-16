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

#include "titanic/true_talk/tt_response.h"

namespace Titanic {

TTresponse::TTresponse(const TTstring &src) : _field0(0), _text(src),
	_dialogueId(0), _nextP(nullptr), _linkP(nullptr) {
}

TTresponse::TTresponse(int dialogueId, int val2) : _field0(val2), _text(" "),
		_dialogueId(dialogueId), _nextP(nullptr), _linkP(nullptr) {
}

TTresponse::TTresponse(const TTresponse *src) : _field0(src->_field0),
		_text(src->_text), _dialogueId(src->_dialogueId), _nextP(src->_nextP),
		_linkP(src->_linkP) {
}

TTresponse::~TTresponse() {
	// Iterate through destroying any successive linked response items
	TTresponse *nextP;
	for (TTresponse *currP = _nextP; currP; currP = nextP) {
		// Get the following response and detach it from the current one,
		// so that when the current is destroyed, it will only destroy itself
		nextP = currP->_nextP;
		currP->_nextP = nullptr;
		delete currP;
	}
}

TTresponse *TTresponse::appendResponse(int id) {
	TTresponse *resp = new TTresponse(id, 3);
	_nextP = resp;
	return resp;
}

TTresponse *TTresponse::copyChain() const {
	TTresponse *returnResponseP = new TTresponse(this);

	for (TTresponse *srcP = _nextP, *destP = returnResponseP;
			srcP; srcP = srcP->_nextP, destP = destP->_nextP) {
		destP->_nextP = new TTresponse(*srcP);
	}

	return returnResponseP;
}

void TTresponse::addLink(TTresponse *item) {
	TTresponse *currP = this;
	while (currP->_linkP)
		currP = currP->_linkP;

	currP->_linkP = item;
}

} // End of namespace Titanic
