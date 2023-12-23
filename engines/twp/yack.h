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

#ifndef TWP_YACK_H
#define TWP_YACK_H

#include "common/array.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/debug.h"

namespace Twp {

template<typename T, class DL = Common::DefaultDeleter<T> >
using unique_ptr = Common::ScopedPtr<T, DL>;

enum class YackTokenId {
	None,
	NewLine,
	Identifier,
	WaitWhile,
	Int,
	Float,
	Whitespace,
	Colon,
	Condition,
	String,
	Assign,
	Comment,
	Goto,
	Code,
	Dollar,
	End
};

// enumeration that lists all errors that can occur
enum YackError {
	ERR_NONE,              // no error
	ERR_INVALIDYackToken,      // invalid YackToken
	ERR_STRINGEXPECTED,    // string expected
	ERR_COLONEXPECTED,     // `:` expected
	ERR_COMMAEXPECTED,     // `,` expected
	ERR_BRACKETRIEXPECTED, // `]` expected
	ERR_CURLYRIEXPECTED,   // `}` expected
	ERR_QUOTEEXPECTED,     // `"` or `'` expected
	ERR_EOC_EXPECTED,      // `*/` expected
	ERR_EOFEXPECTED,       // EOF expected
	ERR_EXPREXPECTED       // expr expected
};

class YackVisitor;
class YackNode {
public:
	virtual ~YackNode() {}

	virtual void accept(YackVisitor &v) = 0;
};

// Represents a condition
class YCond : public YackNode {
public:
	virtual ~YCond() {}

public:
	int _line;
};

class YCodeCond : public YCond {
public:
	YCodeCond(int line) { _line = line; }
	virtual ~YCodeCond() {}
	virtual void accept(YackVisitor &v) override;

public:
	Common::String _code;
};

class YOnce : public YCond {
public:
	YOnce(int line) { _line = line; }
	virtual ~YOnce() {}
	virtual void accept(YackVisitor &v) override;
};

class YShowOnce : public YCond {
public:
	YShowOnce(int line) { _line = line; }
	virtual ~YShowOnce() {}
	virtual void accept(YackVisitor &v) override;
};

class YOnceEver : public YCond {
public:
	YOnceEver(int line) { _line = line; }
	virtual ~YOnceEver() {}
	virtual void accept(YackVisitor &v) override;
};

class YTempOnce : public YCond {
public:
	YTempOnce(int line) { _line = line; }
	virtual ~YTempOnce() {}
	virtual void accept(YackVisitor &v) override;
};

// Expression
class YExp : public YackNode {
public:
	YExp() {}
	virtual ~YExp() {}
};

class YGoto : public YExp {
public:
	YGoto(int line) { _line = line; }
	virtual ~YGoto() {}
	virtual void accept(YackVisitor &v) override;

public:
	Common::String _name;
	int _line = 0;
};

class YCodeExp : public YExp {
public:
	virtual ~YCodeExp() {}
	virtual void accept(YackVisitor &v) override;

public:
	Common::String _code;
};

class YChoice : public YExp {
public:
	virtual ~YChoice() {}
	virtual void accept(YackVisitor &v) override;

public:
	int _number = 0;
	Common::String _text;
	unique_ptr<YGoto> _goto;
};

class YSay : public YExp {
public:
	virtual ~YSay() {}
	virtual void accept(YackVisitor &v) override;

public:
	Common::String _actor;
	Common::String _text;
};

class YPause : public YExp {
public:
	virtual ~YPause() {}
	virtual void accept(YackVisitor &v) override;

public:
	int _time = 0;
};

class YParrot : public YExp {
public:
	virtual ~YParrot() {}
	virtual void accept(YackVisitor &v) override;

public:
	bool _active = true;
};

class YDialog : public YExp {
public:
	virtual ~YDialog() {}
	virtual void accept(YackVisitor &v) override;

public:
	Common::String _actor;
};

class YOverride : public YExp {
public:
	virtual ~YOverride() {}
	virtual void accept(YackVisitor &v) override;

public:
	Common::String _node;
};

class YShutup : public YExp {
public:
	virtual ~YShutup() {}
	virtual void accept(YackVisitor &v) override;
};

class YAllowObjects : public YExp {
public:
	virtual ~YAllowObjects() {}
	virtual void accept(YackVisitor &v) override;

public:
	bool _active = true;
};

class YLimit : public YExp {
public:
	virtual ~YLimit() {}
	virtual void accept(YackVisitor &v) override;

public:
	int _max = 8;
};

class YWaitWhile : public YExp {
public:
	virtual ~YWaitWhile() {}
	virtual void accept(YackVisitor &v) override;

public:
	Common::String _cond;
};

class YWaitFor : public YExp {
public:
	virtual ~YWaitFor() {}
	virtual void accept(YackVisitor &v) override;

public:
	Common::String _actor;
};

//

class YStatement : public YackNode {
public:
	virtual ~YStatement() override {}

	virtual void accept(YackVisitor &v) override;

public:
	unique_ptr<YExp> _exp;
	Common::Array<YCond *> _conds;
};

class YLabel : public YackNode {
public:
	YLabel(int line);
	virtual ~YLabel() override;
	virtual void accept(YackVisitor &v) override;

public:
	Common::String _name;
	Common::Array<YStatement *> _stmts;
	int _line = 0;
};

class YCompilationUnit : public YackNode {
public:
	virtual ~YCompilationUnit() override;
	virtual void accept(YackVisitor &v) override;

public:
	Common::Array<YLabel*> _labels;
};

class YackVisitor {
public:
	virtual ~YackVisitor() {}

	virtual void visit(const YCompilationUnit &node) { defaultVisit(node); }
	virtual void visit(const YStatement &node) { defaultVisit(node); }
	virtual void visit(const YWaitFor &node) { defaultVisit(node); }
	virtual void visit(const YWaitWhile &node) { defaultVisit(node); }
	virtual void visit(const YLimit &node) { defaultVisit(node); }
	virtual void visit(const YAllowObjects &node) { defaultVisit(node); }
	virtual void visit(const YShutup &node) { defaultVisit(node); }
	virtual void visit(const YOverride &node) { defaultVisit(node); }
	virtual void visit(const YDialog &node) { defaultVisit(node); }
	virtual void visit(const YParrot &node) { defaultVisit(node); }
	virtual void visit(const YPause &node) { defaultVisit(node); }
	virtual void visit(const YSay &node) { defaultVisit(node); }
	virtual void visit(const YChoice &node) { defaultVisit(node); }
	virtual void visit(const YCodeExp &node) { defaultVisit(node); }
	virtual void visit(const YGoto &node) { defaultVisit(node); }
	virtual void visit(const YTempOnce &node) { defaultVisit(node); }
	virtual void visit(const YOnceEver &node) { defaultVisit(node); }
	virtual void visit(const YShowOnce &node) { defaultVisit(node); }
	virtual void visit(const YOnce &node) { defaultVisit(node); }
	virtual void visit(const YCodeCond &node) { defaultVisit(node); }
	virtual void visit(const YLabel &node) { defaultVisit(node); }

	virtual void defaultVisit(const YackNode &node) {}
};

struct YackToken {
	YackTokenId id;
	int64 start;
	int64 end;
	int line;

	Common::String toString() const;
};

class YackTokenReader {
public:
	class Iterator {
	public:
		using value_type = YackToken;
		using difference_type = ptrdiff_t;
		using pointer = YackToken *;
		using reference = YackToken &;

	private:
		YackTokenReader *_reader = nullptr;
		int64 _pos = 0;
		YackToken _YackToken;

	public:
		Iterator() {}
		Iterator(YackTokenReader &reader, int64 pos);
		Iterator(const Iterator &it);
		Iterator &operator++();
		Iterator operator++(int);

		Iterator &operator=(const Iterator &rhs) {
			_pos = rhs._pos;
			_YackToken = rhs._YackToken;
			_reader = rhs._reader;
			return *this;
		}
		bool operator==(const Iterator &rhs) const { return _pos == rhs._pos; }
		bool operator!=(const Iterator &rhs) const { return _pos != rhs._pos; }
		YackToken &operator*();
		const YackToken &operator*() const;
		YackToken *operator->();
	};

	using iterator = Iterator;

public:
	void open(Common::SeekableReadStream *stream);

	iterator begin();
	iterator end();
	Common::String readText(const YackToken &YackToken);

private:
	bool readYackToken(YackToken &YackToken);
	Common::String readText(int64 pos, int64 size);
	YackTokenId readYackTokenId();
	YackTokenId readCode();
	YackTokenId readCondition();
	YackTokenId readDollar();
	YackTokenId readNumber();
	YackTokenId readComment();
	YackTokenId readString();
	YackTokenId readIdentifier(char c);
	byte peek();
	void ignore(int64 n = 1, int delim = EOF);

private:
	Common::SeekableReadStream *_stream = nullptr;
	int _line = 0;

	friend class YackParser;
};

class YackParser {
public:
	YackParser() {}
	YCompilationUnit* parse(Common::SeekableReadStream *stream);

private:
	bool match(const std::initializer_list<YackTokenId> &ids);
	YLabel* parseLabel();
	YStatement *parseStatement();
	YCond *parseCondition();
	YExp *parseExpression();
	YSay *parseSayExpression();
	YExp *parseWaitWhileExpression();
	YExp *parseInstructionExpression();
	YGoto *parseGotoExpression();
	YCodeExp *parseCodeExpression();
	YChoice *parseChoiceExpression();

private:
	YackTokenReader _reader;
	YackTokenReader::iterator _it;
};

class YackDump : public YackVisitor {
public:
	virtual ~YackDump() {}

	virtual void visit(const YCompilationUnit &node) {
		debug("CompilationUnit:");
		for (const auto &cond : node._labels) {
			cond->accept(*this);
		}
	}
	virtual void visit(const YLabel &node) {
		debug("Label: %s [%d]", node._name.c_str(), node._line);
		for (const auto &stmt : node._stmts) {
			stmt->accept(*this);
		}
	}
	virtual void visit(const YStatement &node) {
		debug("Statement:");
		for (const auto &cond : node._conds) {
			cond->accept(*this);
		}
		node._exp->accept(*this);
	}
	virtual void visit(const YWaitFor &node) { debug("WaifFor %s", node._actor.c_str()); }
	virtual void visit(const YWaitWhile &node) { debug("WaitWhile %s", node._cond.c_str()); }
	virtual void visit(const YLimit &node) { debug("Limit: %d", node._max); }
	virtual void visit(const YAllowObjects &node) { debug("AllowObjects"); }
	virtual void visit(const YShutup &node) { debug("Shutup"); }
	virtual void visit(const YOverride &node) { defaultVisit(node); }
	virtual void visit(const YDialog &node) { debug("Dialog: %s", node._actor.c_str()); }
	virtual void visit(const YParrot &node) { debug("Parrot: %s", node._active ? "YES" : "NO"); }
	virtual void visit(const YPause &node) { debug("Pause: %d", node._time); }
	virtual void visit(const YSay &node) { debug("Say: actor: %s, text = %s", node._actor.c_str(), node._text.c_str()); }
	virtual void visit(const YChoice &node) { debug("Choice %d: %s -> %s", node._number, node._text.c_str(), node._goto->_name.c_str()); }
	virtual void visit(const YCodeExp &node) { debug("Code: %s", node._code.c_str()); }
	virtual void visit(const YGoto &node) { debug("Goto: %s", node._name.c_str()); }
	virtual void visit(const YTempOnce &node) { debug("TempOnce"); }
	virtual void visit(const YOnceEver &node) { debug("OnceEver"); }
	virtual void visit(const YShowOnce &node) { debug("ShowOnce"); }
	virtual void visit(const YOnce &node) { debug("Once"); }
	virtual void visit(const YCodeCond &node) { debug("Cond: %s", node._code.c_str()); }

	virtual void defaultVisit(const YackNode &node) {}
};

} // namespace Twp

#endif
