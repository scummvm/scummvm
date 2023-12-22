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

#include "twp/yack.h"
#include "common/util.h"

namespace Twp {

Common::String Token::toString() const {
	switch (id) {
	case TokenId::Assign:
		return "Assign";
	case TokenId::NewLine:
		return "NewLine";
	case TokenId::Colon:
		return "Colon";
	case TokenId::Code:
		return "Code";
	case TokenId::Comment:
		return "Comment";
	case TokenId::End:
		return "End";
	case TokenId::Goto:
		return "Goto";
	case TokenId::Identifier:
		return "Identifier";
	case TokenId::None:
		return "None";
	case TokenId::Int:
		return "Integer";
	case TokenId::Float:
		return "Float";
	case TokenId::Condition:
		return "Condition";
	case TokenId::String:
		return "String";
	case TokenId::Whitespace:
		return "Whitespace";
	default:
		return "?";
	}
}

YackTokenReader::Iterator::Iterator(YackTokenReader &reader, int64 pos)
	: _reader(&reader),
	  _pos(pos) {
	operator++();
}

YackTokenReader::Iterator::Iterator(const Iterator &it)
	: _reader(it._reader), _pos(it._pos), _token(it._token) {
}

YackTokenReader::Iterator &YackTokenReader::Iterator::operator++() {
	_reader->_stream->seek(_pos);
	_reader->readToken(_token);
	_pos = _reader->_stream->pos();
	return *this;
}

YackTokenReader::Iterator YackTokenReader::Iterator::operator++(int) {
	Iterator tmp(*this);
	operator++();
	return tmp;
}

Token &YackTokenReader::Iterator::operator*() {
	return _token;
}

const Token &YackTokenReader::Iterator::operator*() const {
	return _token;
}

Token *YackTokenReader::Iterator::operator->() {
	return &_token;
}

void YackTokenReader::open(Common::SeekableReadStream *stream) {
	_stream = stream;
	_line = 1;
}

byte YackTokenReader::peek() {
	byte b = _stream->readByte();
	_stream->seek(-1, SEEK_CUR);
	return b;
}

void YackTokenReader::ignore(int64 n, int delim) {
	int64 i = 0;
	byte b;
	while ((i < n) && (b = _stream->readByte() != delim)) {
		i++;
	}
}

TokenId YackTokenReader::readCode() {
	byte c;
	byte previousChar = 0;
	while ((c = peek()) != '\n' && c != '\0') {
		ignore();
		if (previousChar == ' ' && c == '[' && peek() != ' ') {
			_stream->seek(-1, SEEK_CUR);
			return TokenId::Code;
		}
		previousChar = c;
	}
	return TokenId::Code;
}

TokenId YackTokenReader::readDollar() {
	char c;
	while ((c = peek()) != '[' && c != ' ' && c != '\n' && c != '\0') {
		ignore();
	}
	return TokenId::Dollar;
}

TokenId YackTokenReader::readCondition() {
	while (peek() != ']') {
		ignore();
	}
	ignore();
	return TokenId::Condition;
}

TokenId YackTokenReader::readNumber() {
	bool isFloat = false;
	while (Common::isDigit(peek())) {
		ignore();
	}
	if (peek() == '.') {
		ignore();
		isFloat = true;
	}
	while (Common::isDigit(peek())) {
		ignore();
	}
	return isFloat ? TokenId::Float : TokenId::Int;
}

TokenId YackTokenReader::readComment() {
	ignore(INT_MAX, '\n');
	_stream->seek(_stream->pos() - 1);
	return TokenId::Comment;
}

TokenId YackTokenReader::readString() {
	ignore(INT_MAX, '\"');
	return TokenId::String;
}

TokenId YackTokenReader::readIdentifier(char c) {
	Common::String id;
	id += c;
	while (Common::isAlnum(peek()) || peek() == '_') {
		c = _stream->readByte();
		id += c;
	}
	if (id == "waitwhile") {
		readCode();
		return TokenId::WaitWhile;
	}
	return TokenId::Identifier;
}

TokenId YackTokenReader::readTokenId() {
	char c;
	_stream->read(&c, 1);
	if (_stream->eos()) {
		return TokenId::End;
	}

	switch (c) {
	case '\0':
		return TokenId::End;
	case '\n':
		_line++;
		return TokenId::NewLine;
	case '\t':
	case ' ':
		while (Common::isSpace(peek()) && peek() != '\n')
			ignore();
		return TokenId::Whitespace;
	case '!':
		return readCode();
	case ':':
		return TokenId::Colon;
	case '$':
		return readDollar();
	case '[':
		return readCondition();
	case '=':
		return TokenId::Assign;
	case '\"':
		return readString();
	case '#':
	case ';':
		return readComment();
	default:
		if (c == '-' && peek() == '>') {
			ignore();
			return TokenId::Goto;
		}
		if (c == '-' || Common::isDigit(c)) {
			return readNumber();
		} else if (Common::isAlpha(c)) {
			return readIdentifier(c);
		}
		error("unknown character: %c", c);
		return TokenId::None;
	}
}

bool YackTokenReader::readToken(Token &token) {
	int64 start = _stream->pos();
	int line = _line;
	auto id = readTokenId();
	while (id == TokenId::Whitespace || id == TokenId::Comment || id == TokenId::NewLine || id == TokenId::None) {
		start = _stream->pos();
		line = _line;
		id = readTokenId();
	}
	int64 end = _stream->pos();
	token.id = id;
	token.start = start;
	token.end = end;
	token.line = line;
	return true;
}

Common::String YackTokenReader::readText(int64 pos, int64 size) {
	Common::String out;
	_stream->seek(pos);
	char c;
	for (int i = 0; i < size; i++) {
		c = _stream->readByte();
		out += c;
	}
	return out;
}

Common::String YackTokenReader::readText(const Token &token) {
	return readText(token.start, token.end - token.start);
}

YackTokenReader::iterator YackTokenReader::begin() {
	return Iterator(*this, 0);
}

YackTokenReader::iterator YackTokenReader::end() {
	int64 pos = _stream->size();
	return Iterator(*this, pos);
}

bool YackParser::match(const std::initializer_list<TokenId> &ids) {
	auto it = _it;
	for (auto id : ids) {
		if (it->id != id)
			return false;
		it++;
	}
	return true;
}

void YCodeCond::accept(YackVisitor &v) { v.visit(*this); }
void YOnce::accept(YackVisitor &v) { v.visit(*this); }
void YShowOnce::accept(YackVisitor &v) { v.visit(*this); }
void YOnceEver::accept(YackVisitor &v) { v.visit(*this); }
void YTempOnce::accept(YackVisitor &v) { v.visit(*this); }
void YGoto::accept(YackVisitor &v) { v.visit(*this); }
void YChoice::accept(YackVisitor &v) { v.visit(*this); }
void YSay::accept(YackVisitor &v) { v.visit(*this); }
void YPause::accept(YackVisitor &v) { v.visit(*this); }
void YStatement::accept(YackVisitor &v) { v.visit(*this); }
void YWaitWhile::accept(YackVisitor &v) { v.visit(*this); }
void YAllowObjects::accept(YackVisitor &v) { v.visit(*this); }
void YLimit::accept(YackVisitor &v) { v.visit(*this); }
void YDialog::accept(YackVisitor &v) { v.visit(*this); }
void YLabel::accept(YackVisitor &v) { v.visit(*this); }
void YParrot::accept(YackVisitor &v) { v.visit(*this); }
void YShutup::accept(YackVisitor &v) { v.visit(*this); }
void YCodeExp::accept(YackVisitor &v) { v.visit(*this); }
void YWaitFor::accept(YackVisitor &v) { v.visit(*this); }
void YOverride::accept(YackVisitor &v) { v.visit(*this); }
void YCompilationUnit::accept(YackVisitor &v) { v.visit(*this); }

YLabel::YLabel(int line) { _line = line; }

YLabel::~YLabel() {
	for (size_t i = 0; i < _stmts.size(); i++) {
		delete _stmts[i];
	}
}

YCompilationUnit::~YCompilationUnit() {
	for (size_t i = 0; i < _labels.size(); i++) {
		delete _labels[i];
	}
}

YLabel *YackParser::parseLabel() {
	unique_ptr<YLabel> pLabel;
	// :
	_it++;
	// label
	pLabel.reset(new YLabel(_it->line));
	pLabel->_name = _reader.readText(*_it++);
	do {
		if (match({TokenId::Colon}) || match({TokenId::End}))
			break;
		YStatement *pStatement = parseStatement();
		pLabel->_stmts.push_back(pStatement);
	} while (true);

	return pLabel.release();
}
YStatement *YackParser::parseStatement() {
	unique_ptr<YStatement> pStatement;
	pStatement.reset(new YStatement());

	// expression
	pStatement->_exp.reset(parseExpression());
	// conditions
	while (match({TokenId::Condition})) {
		pStatement->_conds.push_back(parseCondition());
	}
	return pStatement.release();
}
YCond *YackParser::parseCondition() {
	auto text = _reader.readText(*_it);
	auto conditionText = text.substr(1, text.size() - 2);
	auto line = _it->line;
	if (conditionText == "once") {
		return new YOnce(line);
	} else if (conditionText == "showonce") {
		return new YShowOnce(line);
	} else if (conditionText == "onceever") {
		return new YOnceEver(line);
	} else if (conditionText == "temponce") {
		return new YTempOnce(line);
	}
	auto pCondition = new YCodeCond(line);
	pCondition->_code = conditionText;
	return pCondition;
}
YExp *YackParser::parseExpression() {
	if (match({TokenId::Identifier, TokenId::Colon, TokenId::String}))
		return parseSayExpression();
	if (match({TokenId::WaitWhile}))
		return parseWaitWhileExpression();
	if (match({TokenId::Identifier}))
		return parseInstructionExpression();
	if (match({TokenId::Goto}))
		return parseGotoExpression();
	if (match({TokenId::Int}))
		return parseChoiceExpression();
	if (match({TokenId::Code}))
		return parseCodeExpression();
	return nullptr;
}
YSay *YackParser::parseSayExpression() {
	auto actor = _reader.readText(*_it++);
	_it++;
	auto text = _reader.readText(*_it);
	_it++;
	auto pExp = new YSay();
	pExp->_actor = actor;
	pExp->_text = text.substr(1, text.size() - 2);
	return pExp;
}
YExp *YackParser::parseWaitWhileExpression() {
	auto waitwhile = _reader.readText(*_it++);
	auto code = waitwhile.substr(10);
	auto pExp = new YWaitWhile();
	pExp->_cond = code;
	return pExp;
}
YExp *YackParser::parseInstructionExpression() {
	auto identifier = _reader.readText(*_it++);
	if (identifier == "shutup") {
		return new YShutup();
	} else if (identifier == "pause") {
		// pause number
		auto time = atof(_reader.readText(*_it++).c_str());
		auto pExp = new YPause();
		pExp->_time = time;
		return pExp;
	} else if (identifier == "waitfor") {
		// waitfor [actor]
		auto pExp = new YWaitFor();
		if (_it->id == TokenId::Identifier) {
			auto actor = _reader.readText(*_it++);
			pExp->_actor = actor;
		}
		return pExp;
	} else if (identifier == "parrot") {
		// parrot [active]
		auto pExp = new YParrot();
		if (_it->id == TokenId::Identifier) {
			auto active = _reader.readText(*_it++);
			pExp->_active = active == "yes";
		}
		return pExp;
	} else if (identifier == "dialog") {
		// dialog [actor]
		auto pExp = new YDialog();
		if (_it->id == TokenId::Identifier) {
			auto actor = _reader.readText(*_it++);
			pExp->_actor = actor;
		}
		return pExp;
	} else if (identifier == "override") {
		// override [node]
		auto pExp = new YOverride();
		if (_it->id == TokenId::Identifier) {
			auto node = _reader.readText(*_it++);
			pExp->_node = node;
		}
		return pExp;
	} else if (identifier == "allowobjects") {
		// allowobjects [allow]
		auto pExp = new YAllowObjects();
		if (_it->id == TokenId::Identifier) {
			auto node = _reader.readText(*_it++);
			pExp->_active = node == "YES";
		}
		return pExp;
	} else if (identifier == "limit") {
		// limit [number]
		auto pExp = new YLimit();
		if (_it->id == TokenId::Int) {
			auto node = _reader.readText(*_it++);
			pExp->_max = std::strtol(node.c_str(), nullptr, 10);
		}
		return pExp;
	}
	error("Unknown instruction: %s", identifier.c_str());
}
YGoto *YackParser::parseGotoExpression() {
	_it++;
	int line = _it->line;
	auto name = _reader.readText(*_it++);
	auto pExp = new YGoto(line);
	pExp->_name = name;
	return pExp;
}
YCodeExp *YackParser::parseCodeExpression() {
	auto code = _reader.readText(*_it++);
	auto pExp = new YCodeExp();
	pExp->_code = code.substr(1);
	return pExp;
}
YChoice *YackParser::parseChoiceExpression() {
	auto number = atol(_reader.readText(*_it).c_str());
	_it++;
	Common::String text;
	if (_it->id == TokenId::Dollar) {
		text = _reader.readText(*_it);
	} else {
		text = _reader.readText(*_it);
		text = text.substr(1, text.size() - 2);
	}

	_it++;
	auto pExp = new YChoice();
	pExp->_number = number;
	pExp->_text = text;
	pExp->_goto.reset(parseGotoExpression());
	return pExp;
}

YCompilationUnit *YackParser::parse(Common::SeekableReadStream *stream) {
	_reader.open(stream);
	_it = _reader.begin();
	auto pCu = unique_ptr<YCompilationUnit>();
	pCu.reset(new YCompilationUnit());
	while (!match({TokenId::End})) {
		pCu->_labels.push_back(parseLabel());
	}
	return pCu.release();
}

} // namespace Twp
