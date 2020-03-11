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

#include "ultima/ultima4/conversation.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/person.h"
#include "ultima/ultima4/script.h"

namespace Ultima {
namespace Ultima4 {

/* Static variable initialization */
const ResponsePart ResponsePart::NONE("<NONE>", "", true);
const ResponsePart ResponsePart::ASK("<ASK>", "", true);
const ResponsePart ResponsePart::END("<END>", "", true);
const ResponsePart ResponsePart::ATTACK("<ATTACK>", "", true);
const ResponsePart ResponsePart::BRAGGED("<BRAGGED>", "", true);
const ResponsePart ResponsePart::HUMBLE("<HUMBLE>", "", true);
const ResponsePart ResponsePart::ADVANCELEVELS("<ADVANCELEVELS>", "", true);
const ResponsePart ResponsePart::HEALCONFIRM("<HEALCONFIRM>", "", true);
const ResponsePart ResponsePart::STARTMUSIC_LB("<STARTMUSIC_LB>", "", true);
const ResponsePart ResponsePart::STARTMUSIC_HW("<STARTMUSIC_HW>", "", true);
const ResponsePart ResponsePart::STOPMUSIC("<STOPMUSIC>", "", true);
const ResponsePart ResponsePart::HAWKWIND("<HAWKWIND>", "", true);
const unsigned int Conversation::BUFFERLEN = 16;

Response::Response(const Common::String &response) : references(0) {
    add(response);
}

void Response::add(const ResponsePart &part) {
    parts.push_back(part);
}

const Std::vector<ResponsePart> &Response::getParts() const {
    return parts;
}

Response::operator Common::String() const {
    Common::String result;
    for (Std::vector<ResponsePart>::const_iterator i = parts.begin(); i != parts.end(); i++) {
        result += *i;
    }
    return result;
}

Response *Response::addref() {
    references++;
    return this;
}

void Response::release() {
    references--;
    if (references <= 0)
        delete this;
}

ResponsePart::ResponsePart(const Common::String &value, const Common::String &arg, bool command) {
    this->value = value;
    this->arg = arg;
    this->command = command;
}

ResponsePart::operator Common::String() const {
    return value;
}

bool ResponsePart::operator==(const ResponsePart &rhs) const {
    return value == rhs.value;
}

bool ResponsePart::isCommand() const {
    return command;
}

DynamicResponse::DynamicResponse(Response *(*generator)(const DynamicResponse *), const Common::String &param) : 
    Response(""), param(param) {
    this->generator = generator;
    currentResponse = NULL;
}

DynamicResponse::~DynamicResponse() {
    if (currentResponse)
        delete currentResponse;
}

const Std::vector<ResponsePart> &DynamicResponse::getParts() const {
    // blah, must cast away constness
    const_cast<DynamicResponse *>(this)->currentResponse = (*generator)(this);
    return currentResponse->getParts();
}

/*
 * Dialogue::Question class
 */
Dialogue::Question::Question(const Common::String &txt, Response *yes, Response *no) :
    text(txt), yesresp(yes->addref()), noresp(no->addref()) {}

Common::String Dialogue::Question::getText() {
    return text;
}

Response *Dialogue::Question::getResponse(bool yes) {
    if (yes)
        return yesresp;
    return noresp;
}

            
/*
 * Dialogue::Keyword class
 */ 
Dialogue::Keyword::Keyword(const Common::String &kw, Response *resp) :
    keyword(kw), response(resp->addref()) {
    trim(keyword);
    lowercase(keyword);
}

Dialogue::Keyword::Keyword(const Common::String &kw, const Common::String &resp) :
    keyword(kw), response((new Response(resp))->addref()) {
    trim(keyword);
    lowercase(keyword);
}

Dialogue::Keyword::~Keyword() {
    response->release();
}

bool Dialogue::Keyword::operator==(const Common::String &kw) const {
    // minimum 4-character "guessing"
    int testLen = (keyword.size() < 4) ? keyword.size() : 4;

    // exception: empty keyword only matches empty Common::String (alias for 'bye')
    if (testLen == 0 && kw.size() > 0)
        return false;

    if (scumm_strnicmp(kw.c_str(), keyword.c_str(), testLen) == 0)
        return true;
    return false;
}

/*
 * Dialogue class 
 */ 

Dialogue::Dialogue()
	: _intro(NULL)
	, _longIntro(NULL)
	, _defaultAnswer(NULL)
	, _question(NULL) {
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
    return NULL;
}

const ResponsePart &Dialogue::getAction() const { 
    int prob = xu4_random(0x100);

    /* Does the person turn away from/attack you? */
    if (prob >= _turnAwayProb)
        return ResponsePart::NONE;
    else {
        if (_attackProb - prob < 0x40)
            return ResponsePart::END;
        else
            return ResponsePart::ATTACK;
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

Conversation::Conversation() : logger(0), state(INTRO), script(new Script()) {
    logger = new Debug("debug/conversation.txt", "Conversation"); 
#ifdef IOS
    U4IOS::incrementConversationCount();
#endif

}

Conversation::~Conversation() {
#ifdef IOS
    U4IOS::decrementConversationCount();
#endif
    delete logger;
    delete script;
}

Conversation::InputType Conversation::getInputRequired(int *bufferlen) {    
    switch (state) {
    case BUY_QUANTITY:
    case SELL_QUANTITY:
        {
            *bufferlen = 2;
            return INPUT_STRING;
        }

    case TALK:
    case BUY_PRICE:
    case TOPIC:
        {
            *bufferlen = BUFFERLEN;
            return INPUT_STRING;
        }

    case GIVEBEGGAR:
        {
            *bufferlen = 2;
            return INPUT_STRING;
        }

    case ASK:
    case ASKYESNO:
        {
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

    ASSERT(0, "invalid state: %d", state);
    return INPUT_NONE;
}

} // End of namespace Ultima4
} // End of namespace Ultima
