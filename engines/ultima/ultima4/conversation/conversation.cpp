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

#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/game/person.h"
#include "ultima/ultima4/game/script.h"

namespace Ultima {
namespace Ultima4 {

const uint Conversation::BUFFERLEN = 16;
ResponseParts *g_responseParts;

ResponseParts::ResponseParts() :
		NONE("<NONE>", "", true),
		ASK("<ASK>", "", true),
		END("<END>", "", true),
		ATTACK("<ATTACK>", "", true),
		BRAGGED("<BRAGGED>", "", true),
		HUMBLE("<HUMBLE>", "", true),
		ADVANCELEVELS("<ADVANCELEVELS>", "", true),
		HEALCONFIRM("<HEALCONFIRM>", "", true),
		STARTMUSIC_LB("<STARTMUSIC_LB>", "", true),
		STARTMUSIC_HW("<STARTMUSIC_HW>", "", true),
		STOPMUSIC("<STOPMUSIC>", "", true),
		HAWKWIND("<HAWKWIND>", "", true) {
	g_responseParts = this;
}

ResponseParts::~ResponseParts() {
	g_responseParts = nullptr;
}

/*-------------------------------------------------------------------*/

Response::Response(const Common::String &response) : _references(0) {
	add(response);
}

void Response::add(const ResponsePart &part) {
	_parts.push_back(part);
}

const Std::vector<ResponsePart> &Response::getParts() const {
	return _parts;
}

Response::operator Common::String() const {
	Common::String result;
	for (Std::vector<ResponsePart>::const_iterator i = _parts.begin(); i != _parts.end(); i++) {
		result += *i;
	}
	return result;
}

Response *Response::addref() {
	_references++;
	return this;
}

void Response::release() {
	_references--;
	if (_references <= 0)
		delete this;
}

ResponsePart::ResponsePart(const Common::String &value, const Common::String &arg, bool command) {
	_value = value;
	_arg = arg;
	_command = command;
}

ResponsePart::operator Common::String() const {
	return _value;
}

bool ResponsePart::operator==(const ResponsePart &rhs) const {
	return _value == rhs._value;
}

bool ResponsePart::isCommand() const {
	return _command;
}

DynamicResponse::DynamicResponse(Response * (*generator)(const DynamicResponse *), const Common::String &param) :
	Response(""), _param(param) {
	_generator = generator;
	_currentResponse = nullptr;
}

DynamicResponse::~DynamicResponse() {
	if (_currentResponse)
		delete _currentResponse;
}

const Std::vector<ResponsePart> &DynamicResponse::getParts() const {
	// blah, must cast away constness
	const_cast<DynamicResponse *>(this)->_currentResponse = (*_generator)(this);
	return _currentResponse->getParts();
}

/*
 * Dialogue::Question class
 */
Dialogue::Question::Question(const Common::String &txt, Response *yes, Response *no) :
	_text(txt), _yesResp(yes->addref()), _noResp(no->addref()) {}

Common::String Dialogue::Question::getText() {
	return _text;
}

Response *Dialogue::Question::getResponse(bool yes) {
	if (yes)
		return _yesResp;
	return _noResp;
}


/*
 * Dialogue::Keyword class
 */
Dialogue::Keyword::Keyword(const Common::String &kw, Response *resp) :
	_keyword(kw), _response(resp->addref()) {
	trim(_keyword);
	lowercase(_keyword);
}

Dialogue::Keyword::Keyword(const Common::String &kw, const Common::String &resp) :
	_keyword(kw), _response((new Response(resp))->addref()) {
	trim(_keyword);
	lowercase(_keyword);
}

Dialogue::Keyword::~Keyword() {
	_response->release();
}

bool Dialogue::Keyword::operator==(const Common::String &kw) const {
	// minimum 4-character "guessing"
	int testLen = (_keyword.size() < 4) ? _keyword.size() : 4;

	// exception: empty keyword only matches empty Common::String (alias for 'bye')
	if (testLen == 0 && kw.size() > 0)
		return false;

	if (scumm_strnicmp(kw.c_str(), _keyword.c_str(), testLen) == 0)
		return true;
	return false;
}

/*
 * Dialogue class
 */

Dialogue::Dialogue()
	: _intro(nullptr)
	, _longIntro(nullptr)
	, _defaultAnswer(nullptr)
	, _question(nullptr) {
}

Dialogue::~Dialogue() {
	for (KeywordMap::iterator i = _keywords.begin(); i != _keywords.end(); i++) {
		delete i->_value;
	}
}

void Dialogue::addKeyword(const Common::String &kw, Response *response) {
	if (_keywords.find(kw) != _keywords.end())
		delete _keywords[kw];

	_keywords[kw] = new Keyword(kw, response);
}

Dialogue::Keyword *Dialogue::operator[](const Common::String &kw) {
	KeywordMap::iterator i = _keywords.find(kw);

	// If they entered the keyword verbatim, return it!
	if (i != _keywords.end())
		return i->_value;
	// Otherwise, go find one that fits the description.
	else {
		for (i = _keywords.begin(); i != _keywords.end(); i++) {
			if ((*i->_value) == kw)
				return i->_value;
		}
	}
	return nullptr;
}

const ResponsePart &Dialogue::getAction() const {
	int prob = xu4_random(0x100);

	/* Does the person turn away from/attack you? */
	if (prob >= _turnAwayProb)
		return g_responseParts->NONE;
	else {
		if (_attackProb - prob < 0x40)
			return g_responseParts->END;
		else
			return g_responseParts->ATTACK;
	}
}

Common::String Dialogue::dump(const Common::String &arg) {
	Common::String result;
	if (arg == "") {
		result = "keywords:\n";
		for (KeywordMap::iterator i = _keywords.begin(); i != _keywords.end(); i++) {
			result += i->_key + "\n";
		}
	} else {
		if (_keywords.find(arg) != _keywords.end())
			result = static_cast<Common::String>(*_keywords[arg]->getResponse());
	}

	return result;
}

/*
 * Conversation class
 */

Conversation::Conversation() : _state(INTRO), _script(new Script()),
		_question(nullptr), _quant(0), _player(0), _price(0) {
#ifdef IOS_ULTIMA4
	U4IOS::incrementConversationCount();
#endif
}

Conversation::~Conversation() {
#ifdef IOS_ULTIMA4
	U4IOS::decrementConversationCount();
#endif
	delete _script;
}

Conversation::InputType Conversation::getInputRequired(int *bufferlen) {
	switch (_state) {
	case BUY_QUANTITY:
	case SELL_QUANTITY: {
		*bufferlen = 2;
		return INPUT_STRING;
	}

	case TALK:
	case BUY_PRICE:
	case TOPIC: {
		*bufferlen = BUFFERLEN;
		return INPUT_STRING;
	}

	case GIVEBEGGAR: {
		*bufferlen = 2;
		return INPUT_STRING;
	}

	case ASK:
	case ASKYESNO: {
		*bufferlen = 3;
		return INPUT_STRING;
	}

	case VENDORQUESTION:
	case BUY_ITEM:
	case SELL_ITEM:
	case CONFIRMATION:
	case CONTINUEQUESTION:
	case PLAYER:
		return INPUT_CHARACTER;

	case ATTACK:
	case DONE:
	case INTRO:
	case FULLHEAL:
	case ADVANCELEVELS:
		return INPUT_NONE;
	}

	error("invalid state: %d", _state);
	return INPUT_NONE;
}

} // End of namespace Ultima4
} // End of namespace Ultima
