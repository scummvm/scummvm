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

class Parse {
public:
	void skipExpr(char stopToken);
	void printExpr(char stopToken);
	void printVarIndex(void);
	virtual int16 parseVarIndex(uint16 *arg_0 = 0, uint16 *arg_4 = 0) = 0;
	virtual int16 parseValExpr(byte stopToken = 99) = 0;
	virtual int16 parseExpr(byte stopToken, byte *resultPtr) = 0;

	Parse(GobEngine *vm);
	virtual ~Parse() {}

protected:
	enum PointerType {
		kExecPtr = 0,
		kInterVar = 1,
		kResStr = 2
	};

	GobEngine *_vm;

	int32 encodePtr(byte *ptr, int type);
	byte *decodePtr(int32 n);

	void printExpr_internal(char stopToken);
};

class Parse_v1 : public Parse {
public:
	Parse_v1(GobEngine *vm);
	virtual ~Parse_v1() {}

	virtual int16 parseVarIndex(uint16 *arg_0 = 0, uint16 *arg_4 = 0);
	virtual int16 parseValExpr(byte stopToken = 99);
	virtual int16 parseExpr(byte stopToken, byte *resultPtr);
};

class Parse_v2 : public Parse_v1 {
public:
	Parse_v2(GobEngine *vm);
	virtual ~Parse_v2() {}

	virtual int16 parseVarIndex(uint16 *arg_0 = 0, uint16 *arg_4 = 0);
	virtual int16 parseValExpr(byte stopToken = 99);
	virtual int16 parseExpr(byte stopToken, byte *resultPtr);
};

} // End of namespace Gob

#endif // GOB_PARSE_H
