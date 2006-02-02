/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#ifndef GOB_PARSE_H
#define GOB_PARSE_H

namespace Gob {

class Parse {
public:
	void skipExpr(char stopToken);
	void printExpr(char stopToken);
	void printVarIndex(void);
	virtual int16 parseVarIndex(void) = 0;
	virtual int16 parseValExpr(unsigned stopToken=99) = 0;
	virtual int16 parseExpr(char stopToken, byte *resultPtr) = 0;
	
	Parse(GobEngine *vm);
	virtual ~Parse() {};

protected:
	enum PointerType {
		kExecPtr = 0,
		kInterVar = 1,
		kResStr = 2
	};

	GobEngine *_vm;

	int32 encodePtr(char *ptr, int type);
	char *decodePtr(int32 n);
};

class Parse_v1 : public Parse {
public:
	Parse_v1(GobEngine *vm);
	virtual ~Parse_v1() {};

	virtual int16 parseVarIndex(void);
	virtual int16 parseValExpr(unsigned stopToken=99);
	virtual int16 parseExpr(char stopToken, byte *resultPtr);
};

class Parse_v2 : public Parse_v1 {
public:
	Parse_v2(GobEngine *vm);
	virtual ~Parse_v2() {};

	virtual int16 parseVarIndex(void);
	virtual int16 parseValExpr(unsigned stopToken=99);
	virtual int16 parseExpr(char stopToken, byte *resultPtr);
};

}				// End of namespace Gob

#endif
