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

#ifndef MOHAWK_LIVINGBOOKS_ITEMSCRIPT_H
#define MOHAWK_LIVINGBOOKS_ITEMSCRIPT_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/str.h"

namespace Mohawk {

struct LBKey {
	byte code;
	byte modifiers;
	byte char_;
	byte repeats;
};

struct LBScriptEntry {
	LBScriptEntry();
	~LBScriptEntry();

	uint16 state;

	uint16 type;
	uint16 event;
	uint16 opcode;
	uint16 param;

	uint16 argc;
	uint16 *argvParam;
	uint16 *argvTarget;

	uint16 targetingType;
	Common::Array<Common::String> targets;

	// kLBNotifyChangeMode
	uint16 newUnknown;
	uint16 newMode;
	uint16 newPage;
	uint16 newSubpage;
	Common::String newCursor;

	// kLBEventNotified
	uint16 matchFrom;
	uint16 matchNotify;

	// kLBOpSendExpression
	uint32 offset;
	// kLBOpJumpUnlessExpression
	uint16 target;

	uint16 dataType;
	uint16 dataLen;
	byte *data;

	Common::Array<Common::String> conditions;
	Common::Array<LBScriptEntry *> subentries;
};

} // End of namespace Mohawk

#endif
