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

#ifndef GOB_PARSE_H
#define GOB_PARSE_H

namespace Gob {

enum {
	OP_NEG        =  1,
	OP_ADD        =  2,
	OP_SUB        =  3,
	OP_BITOR      =  4,
	OP_MUL        =  5,
	OP_DIV        =  6,
	OP_MOD        =  7,
	OP_BITAND     =  8,
	OP_BEGIN_EXPR =  9,
	OP_END_EXPR   = 10,
	OP_NOT        = 11,

	OP_END_MARKER = 12, // Marks end of an array or string


	OP_ARRAY_INT8              = 16,
	OP_LOAD_VAR_INT16          = 17,
	OP_LOAD_VAR_INT8           = 18,
	OP_LOAD_IMM_INT32          = 19,
	OP_LOAD_IMM_INT16          = 20,
	OP_LOAD_IMM_INT8           = 21,
	OP_LOAD_IMM_STR            = 22,
	OP_LOAD_VAR_INT32          = 23,
	OP_LOAD_VAR_INT32_AS_INT16 = 24,
	OP_LOAD_VAR_STR            = 25,
	OP_ARRAY_INT32             = 26,
	OP_ARRAY_INT16             = 27,
	OP_ARRAY_STR               = 28,

	OP_FUNC = 29,

	OP_OR      = 30, // Logical OR
	OP_AND     = 31, // Logical AND
	OP_LESS    = 32,
	OP_LEQ     = 33,
	OP_GREATER = 34,
	OP_GEQ     = 35,
	OP_EQ      = 36,
	OP_NEQ     = 37
};

enum {
	FUNC_SQRT1 =  0,
	FUNC_SQRT2 =  1,
	FUNC_SQRT3 =  6,

	FUNC_SQR   =  5,
	FUNC_ABS   =  7,
	FUNC_RAND  = 10
};

enum {
	// FIXME: The following two 'truth values' are stored inside the list
	// of "operators". So they somehow coincide with OP_LOAD_VAR_INT32
	// and OP_LOAD_VAR_INT32_AS_INT16. I haven't yet quite understood
	// how, resp. what that means. You have been warned.
	GOB_TRUE  = 24,
	GOB_FALSE = 23
};

class Parse {
public:
	void skipExpr(char stopToken);
	void printExpr(char stopToken);
	void printVarIndex(void);

	int16 parseVarIndex(uint16 *size = 0, uint16 *type = 0);
	int16 parseValExpr(byte stopToken = 99);
	int16 parseExpr(byte stopToken, byte *type);

	Parse(GobEngine *vm);
	virtual ~Parse() {}

private:
	enum PointerType {
		kExecPtr  = 0,
		kInterVar = 1,
		kResStr   = 2
	};

	GobEngine *_vm;

	int32 encodePtr(byte *ptr, int type);
	byte *decodePtr(int32 n);

	void printExpr_internal(char stopToken);

	int cmpHelper(byte *operPtr, int32 *valPtr);

	bool getVarBase(uint32 &varBase, bool mindStop = false,
			uint16 *size = 0, uint16 *type = 0);
};

} // End of namespace Gob

#endif // GOB_PARSE_H
