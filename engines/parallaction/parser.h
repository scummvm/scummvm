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
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_PARSER_H
#define PARALLACTION_PARSER_H

#include "common/stream.h"
#include "parallaction/objects.h"

namespace Parallaction {

char   *parseNextToken(char *s, char *tok, uint16 count, const char *brk, bool ignoreQuotes = false);

#define MAX_TOKEN_LEN	50
extern char _tokens[][MAX_TOKEN_LEN];

class Script {

	Common::ReadStream *_input;
	bool	_disposeSource;
	uint	_line;				// for debug messages

	void clearTokens();
	uint16 fillTokens(char* line);

public:
	Script(Common::ReadStream *, bool _disposeSource = false);
	~Script();

	char *readLine(char *buf, size_t bufSize);
	uint16 readLineToken(bool errorOnEOF = false);

	void skip(const char* endToken);

	uint	getLine() { return _line; }
};



class Opcode {

public:
	virtual void operator()() const = 0;
	virtual ~Opcode() { }
};

template <class T>
class OpcodeImpl : public Opcode {

	typedef void (T::*Fn)();

	T*	_instance;
	Fn	_fn;

public:
	OpcodeImpl(T* instance, const Fn &fn) : _instance(instance), _fn(fn) { }

	void operator()() const {
		(_instance->*_fn)();
	}

};

typedef Common::Array<const Opcode*>	OpcodeSet;


class Parser {

public:
	Parser() { reset(); }
	~Parser() {}

	uint	_lookup;

	Common::Stack<OpcodeSet*>	_opcodes;
	Common::Stack<Table*>		_statements;

	OpcodeSet	*_currentOpcodes;
	Table		*_currentStatements;

	void	bind(Script *script);
	void	unbind();
	void	pushTables(OpcodeSet *opcodes, Table* statements);
	void	popTables();
	void	parseStatement();

protected:
	void	reset();

	Script	*_script;
};





} // namespace Parallaction

#endif


