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

#include "titanic/true_talk/tt_sentence.h"
#include "titanic/true_talk/script_handler.h"

namespace Titanic {

TTsentenceSubBase::TTsentenceSubBase() : _field0(0), _field4(0), _field8(0),
	_fieldC(0), _field10(0), _field14(0), _field18(0), _field1C(0),
	_nextP(nullptr), _field24(0) {
}

void TTsentenceSubBase::deleteSiblings() {
	// Iterate through the linked chain of nodes, deleting each in turn
	for (TTsentenceSubBase *curP = _nextP, *nextP = nullptr; nextP; curP = nextP) {
		nextP = curP->_nextP;
		delete curP;
	}

	_nextP = nullptr;
}

/*------------------------------------------------------------------------*/

TTsentence::TTsentence(int inputCtr, const TTstring &line, CScriptHandler *owner,
		TTroomScript *roomScript, TTnpcScript *npcScript) :
		_owner(owner), _field2C(1), _inputCtr(inputCtr), _field34(0),
		_field38(0), _initialLine(line), _nodesP(nullptr), _roomScript(roomScript),
		_npcScript(npcScript), _field58(0), _field5C(0) {
	_status = _initialLine.isValid() && _normalizedLine.isValid() ? SS_11: SS_VALID;
}

TTsentence::TTsentence(const TTsentence *src) : _initialLine(src->_initialLine),
		_normalizedLine(src->_normalizedLine) {
	copyFrom(*src);
}

TTsentence::~TTsentence() {
	_sub.deleteSiblings();

	if (_nodesP) {
		_nodesP->deleteSiblings();
		delete _nodesP;
	}
}

void TTsentence::copyFrom(const TTsentence &src) {
	if (!src.getStatus())
		_status = SS_5;
	else if (!src._initialLine.isValid() || !src._normalizedLine.isValid())
		_status = SS_11;
	else
		_status = SS_VALID;

	_inputCtr = src._inputCtr;
	_owner = src._owner;
	_roomScript = src._roomScript;
	_npcScript = src._npcScript;
	_field58 = src._field58;
	_field5C = src._field5C;
	_field34 = src._field34;
	_field38 = src._field38;
	_field2C = src._field2C;
	_nodesP = nullptr;

	if (src._nodesP) {
		// Source has processed nodes, so duplicate them
		for (TTsentenceNode *node = src._nodesP; node;
				node = static_cast<TTsentenceNode *>(node->_nextP)) {
			TTsentenceNode *newNode = new TTsentenceNode(node->_wordP);
			if (_nodesP)
				_nodesP->addToTail(newNode);
			else
				_nodesP = newNode;
		}
	}
}

int TTsentence::storeVocabHit(TTword *word) {
	if (!word)
		return 0;

	TTsentenceNode *node = new TTsentenceNode(word);
	if (_nodesP) {
		_nodesP->addToTail(node);
	} else {
		_nodesP = node;
	}

	return 0;
}

} // End of namespace Titanic
