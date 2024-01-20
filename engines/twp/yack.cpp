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

Common::String YackToken::toString() const {
	switch (id) {
	case YackTokenId::Assign:
		return "Assign";
	case YackTokenId::NewLine:
		return "NewLine";
	case YackTokenId::Colon:
		return "Colon";
	case YackTokenId::Code:
		return "Code";
	case YackTokenId::Comment:
		return "Comment";
	case YackTokenId::End:
		return "End";
	case YackTokenId::Goto:
		return "Goto";
	case YackTokenId::Identifier:
		return "Identifier";
	case YackTokenId::None:
		return "None";
	case YackTokenId::Int:
		return "Integer";
	case YackTokenId::Float:
		return "Float";
	case YackTokenId::Condition:
		return "Condition";
	case YackTokenId::String:
		return "String";
	case YackTokenId::Whitespace:
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
	: _reader(it._reader), _pos(it._pos), _YackToken(it._YackToken) {
}

YackTokenReader::Iterator &YackTokenReader::Iterator::operator++() {
	_reader->_stream->seek(_pos);
	_reader->readYackToken(_YackToken);
	_pos = _reader->_stream->pos();
	return *this;
}

YackTokenReader::Iterator YackTokenReader::Iterator::operator++(int) {
	Iterator tmp(*this);
	operator++();
	return tmp;
}

YackToken &YackTokenReader::Iterator::operator*() {
	return _YackToken;
}

const YackToken &YackTokenReader::Iterator::operator*() const {
	return _YackToken;
}

YackToken *YackTokenReader::Iterator::operator->() {
	return &_YackToken;
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

YackTokenId YackTokenReader::readCode() {
	byte c;
	byte previousChar = 0;
	while ((c = peek()) != '\n' && c != '\0') {
		ignore();
		if (previousChar == ' ' && c == '[' && peek() != ' ') {
			_stream->seek(-1, SEEK_CUR);
			return YackTokenId::Code;
		}
		previousChar = c;
	}
	return YackTokenId::Code;
}

YackTokenId YackTokenReader::readDollar() {
	char c;
	while ((c = peek()) != '[' && c != ' ' && c != '\n' && c != '\0') {
		ignore();
	}
	return YackTokenId::Dollar;
}

YackTokenId YackTokenReader::readCondition() {
	while (peek() != ']') {
		ignore();
	}
	ignore();
	return YackTokenId::Condition;
}

YackTokenId YackTokenReader::readNumber() {
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
	return isFloat ? YackTokenId::Float : YackTokenId::Int;
}

YackTokenId YackTokenReader::readComment() {
	ignore(INT_MAX, '\n');
	_stream->seek(_stream->pos() - 1);
	return YackTokenId::Comment;
}

YackTokenId YackTokenReader::readString() {
	ignore(INT_MAX, '\"');
	return YackTokenId::String;
}

YackTokenId YackTokenReader::readIdentifier(char c) {
	Common::String id;
	id += c;
	while (Common::isAlnum(peek()) || peek() == '_') {
		c = _stream->readByte();
		id += c;
	}
	if (id == "waitwhile") {
		readCode();
		return YackTokenId::WaitWhile;
	}
	return YackTokenId::Identifier;
}

YackTokenId YackTokenReader::readYackTokenId() {
	char c;
	_stream->read(&c, 1);
	if (_stream->eos()) {
		return YackTokenId::End;
	}

	switch (c) {
	case '\0':
		return YackTokenId::End;
	case '\n':
		_line++;
		return YackTokenId::NewLine;
	case '\t':
	case ' ':
		while (Common::isSpace(peek()) && peek() != '\n')
			ignore();
		return YackTokenId::Whitespace;
	case '!':
		return readCode();
	case ':':
		return YackTokenId::Colon;
	case '$':
		return readDollar();
	case '[':
		return readCondition();
	case '=':
		return YackTokenId::Assign;
	case '\"':
		return readString();
	case '#':
	case ';':
		return readComment();
	default:
		if (c == '-' && peek() == '>') {
			ignore();
			return YackTokenId::Goto;
		}
		if (c == '-' || Common::isDigit(c)) {
			return readNumber();
		} else if (Common::isAlpha(c)) {
			return readIdentifier(c);
		}
		debug("unknown character: %c", c);
		return YackTokenId::None;
	}
}

bool YackTokenReader::readYackToken(YackToken &YackToken) {
	int64 start = _stream->pos();
	int line = _line;
	auto id = readYackTokenId();
	while (id == YackTokenId::Whitespace || id == YackTokenId::Comment || id == YackTokenId::NewLine || id == YackTokenId::None) {
		start = _stream->pos();
		line = _line;
		id = readYackTokenId();
	}
	int64 end = _stream->pos();
	YackToken.id = id;
	YackToken.start = start;
	YackToken.end = end;
	YackToken.line = line;
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

Common::String YackTokenReader::readText(const YackToken &YackToken) {
	return readText(YackToken.start, YackToken.end - YackToken.start);
}

YackTokenReader::iterator YackTokenReader::begin() {
	return Iterator(*this, 0);
}

YackTokenReader::iterator YackTokenReader::end() {
	int64 pos = _stream->size();
	return Iterator(*this, pos);
}

bool YackParser::match(const std::initializer_list<YackTokenId> &ids) {
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
	// debug("label %s", pLabel->_name.c_str());
	do {
		if (match({YackTokenId::Colon}) || match({YackTokenId::End}))
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
	while (match({YackTokenId::Condition})) {
		pStatement->_conds.push_back(parseCondition());
	}
	return pStatement.release();
}
YCond *YackParser::parseCondition() {
	auto text = _reader.readText(*_it);
	auto conditionText = text.substr(1, text.size() - 2);
	auto line = _it->line;
	_it++;
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
	if (match({YackTokenId::Identifier, YackTokenId::Colon, YackTokenId::String}))
		return parseSayExpression();
	if (match({YackTokenId::WaitWhile}))
		return parseWaitWhileExpression();
	if (match({YackTokenId::Identifier}))
		return parseInstructionExpression();
	if (match({YackTokenId::Goto}))
		return parseGotoExpression();
	if (match({YackTokenId::Int}))
		return parseChoiceExpression();
	if (match({YackTokenId::Code}))
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
		if (_it->id == YackTokenId::Identifier) {
			auto actor = _reader.readText(*_it++);
			pExp->_actor = actor;
		}
		return pExp;
	} else if (identifier == "parrot") {
		// parrot [active]
		auto pExp = new YParrot();
		if (_it->id == YackTokenId::Identifier) {
			auto active = _reader.readText(*_it++);
			pExp->_active = active == "yes";
		}
		return pExp;
	} else if (identifier == "dialog") {
		// dialog [actor]
		auto pExp = new YDialog();
		if (_it->id == YackTokenId::Identifier) {
			auto actor = _reader.readText(*_it++);
			pExp->_actor = actor;
		}
		return pExp;
	} else if (identifier == "override") {
		// override [node]
		auto pExp = new YOverride();
		if (_it->id == YackTokenId::Identifier) {
			auto node = _reader.readText(*_it++);
			pExp->_node = node;
		}
		return pExp;
	} else if (identifier == "allowobjects") {
		// allowobjects [allow]
		auto pExp = new YAllowObjects();
		if (_it->id == YackTokenId::Identifier) {
			auto node = _reader.readText(*_it++);
			pExp->_active = node == "YES";
		}
		return pExp;
	} else if (identifier == "limit") {
		// limit [number]
		auto pExp = new YLimit();
		if (_it->id == YackTokenId::Int) {
			auto node = _reader.readText(*_it++);
			pExp->_max = strtol(node.c_str(), nullptr, 10);
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
	if (_it->id == YackTokenId::Dollar) {
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
	while (!match({YackTokenId::End})) {
		pCu->_labels.push_back(parseLabel());
	}
	return pCu.release();
}

} // namespace Twp
