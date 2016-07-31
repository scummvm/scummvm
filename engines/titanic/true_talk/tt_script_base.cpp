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

#include "common/textconsole.h"
#include "titanic/true_talk/tt_script_base.h"
#include "titanic/titanic.h"

namespace Titanic {

TTscriptBase::TTscriptBase(int scriptId, const char *charClass, int state,
		const char *charName, int v3, int v4, int v5, int v6, int v7) :
		_charName(charName), _charClass(charClass), _status(0),
		_nodesP(nullptr), _id(0), _hist1P(nullptr),
		_field20(0), _field24(0), _field28(0), _field2C(0),
		_field30(0), _state(0), _hist2P(nullptr), _field3C(0),
		_respHeadP(nullptr), _respTailP(nullptr), _oldResponseP(nullptr) {
	if (!isValid()) {
		if (!v7 || !getStatus()) {
			_id = scriptId;
			_field20 = v3;
			_field24 = v4;
			_field28 = v5;
			_field2C = v6;
			_field30 = v7;
			_state = state;
		} else {
			_status = 5;
		}
	}

	if (_status)
		reset();
}

TTscriptBase::~TTscriptBase() {
	deleteResponses();
	delete _oldResponseP;

	delete _hist1P;
	delete _hist2P;

	if (_nodesP) {
		_nodesP->deleteSiblings();
		delete _nodesP;
	}
}

bool TTscriptBase::isValid() {
	bool result = !_charName.isValid() && !_charClass.isValid();
	_status = result ? 0 : 11;
	return result;
}

void TTscriptBase::reset() {
	_nodesP = nullptr;
	_id = 4;
	_hist1P = nullptr;
	_field20 = 0;
	_field24 = -1;
	_field28 = -1;
	_field2C = -1;
	_field30 = 0;
	_state = 0;
	_hist2P = nullptr;
	_field3C = 0;
	_respHeadP = nullptr;
	_respTailP = nullptr;
	_oldResponseP = nullptr;
}

int TTscriptBase::scriptPreprocess(TTsentence *sentence) {
	delete _hist1P;
	_hist1P = new TTscriptHist(sentence);

	return _hist1P ? SS_VALID : SS_7;
}

void TTscriptBase::addResponse(const TTstring &str) {
	appendResponse2(-1, nullptr, str);
}

void TTscriptBase::addResponse(int id) {
	appendResponse(-1, nullptr, id);
}

void TTscriptBase::applyResponse() {
	delete _oldResponseP;
	_oldResponseP = nullptr;

	if (_respHeadP) {
		g_vm->_scriptHandler->setResponse(this, _respHeadP);
		_oldResponseP = _respHeadP->copyChain();
		TTresponse *oldRespP = _respHeadP;
		_respHeadP = _respHeadP->getLink();
		_respTailP = nullptr;

		delete oldRespP;
	}
}

void TTscriptBase::deleteResponses() {
	while (_respTailP) {
		_respHeadP = _respTailP;
		_respTailP = _respHeadP->getLink();
		delete _respHeadP;
	}
}

void TTscriptBase::appendResponse(int val1, int *val2, int val3) {
	if (!val2 || val1 <= *val2) {
		if (_respHeadP) {
			_respHeadP = new TTresponse(_respHeadP);
		} else {
			_respHeadP = new TTresponse(val3, 3);
			if (_respTailP)
				_respTailP->addLink(_respHeadP);
			else
				_respTailP = _respHeadP;
		}
	}
}

void TTscriptBase::appendResponse(int val1, int *val2, const TTstring &str) {
	if (!val2 || val1 <= *val2) {
		if (_respHeadP) {
			_respHeadP = new TTresponse(str);
		} else {
			_respHeadP = new TTresponse(str);
			if (_respTailP)
				_respTailP->addLink(_respHeadP);
			else
				_respTailP = _respHeadP;
		}
	}
}

} // End of namespace Titanic
