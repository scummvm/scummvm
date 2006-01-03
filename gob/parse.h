/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
	int16 parseExpr(char stopToken, byte *resultPtr);
	void skipExpr(char stopToken);
	int16 parseValExpr(void);
	int16 parseVarIndex(void);
	void printExpr(char stopToken);
	void printVarIndex(void);
	
	Parse(GobEngine *vm);

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

}				// End of namespace Gob

#endif
