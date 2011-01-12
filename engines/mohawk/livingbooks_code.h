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

#ifndef MOHAWK_LIVINGBOOKS_CODE_H
#define MOHAWK_LIVINGBOOKS_CODE_H

#include "common/substream.h"

namespace Mohawk {

class MohawkEngine_LivingBooks;
class LBItem;

enum LBValueType {
	kLBValueString,
	kLBValueInteger
};

struct LBValue {
	LBValue() { type = kLBValueInteger; integer = 0; }

	LBValueType type;
	Common::String string;
	int integer;

	bool operator==(const LBValue &x) const;
	bool operator!=(const LBValue &x) const;
};

enum {
	kLBCodeLiteralInteger = 0x1
};

enum {
	kLBCodeTokenString = 0x1,
	kLBCodeTokenLiteral = 0x5,
	kLBCodeTokenChar = 0x6,
	kLBCodeTokenEndOfStatement = 0x7,
	kLBCodeTokenEndOfFile = 0x8,
	kLBCodeTokenOpenBracket = 0xf,
	kLBCodeTokenCloseBracket = 0x10,
	kLBCodeTokenLong = 0x11,

	kLBCodeTokenEquals = 0x22, // TODO: maybe..
	kLBCodeTokenQuote = 0x27, // "'"
	kLBCodeTokenComma = 0x2c // ","
};

class LBCode {
public:
	LBCode(MohawkEngine_LivingBooks *vm);
	~LBCode();

	void runCode(LBItem *src, uint32 offset);

protected:
	MohawkEngine_LivingBooks *_vm;

	uint32 size;
	byte *data;

	Common::HashMap<uint16, Common::String> strings;

	Common::Array<LBValue> readParams(LBItem *src, uint32 &offset);
	void runCodeCommand(LBItem *src, uint32 &offset);
	void runCodeItemCommand(LBItem *src, uint32 &offset);
	void runCodeNotifyCommand(LBItem *src, uint32 &offset);
};

} // End of namespace Mohawk

#endif
