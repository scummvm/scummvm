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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_ACTIONLIST_H_
#define ASYLUM_ACTIONLIST_H_

#include "common/array.h"
#include "common/stream.h"

namespace Asylum {

#define MAX_ACTION_COMMANDS 161

typedef struct ActionCommand {
	uint32 numLines;	//	Only set on the first line of each script
	uint32 opcode;
	uint32 param1;
	uint32 param2;
	uint32 param3;
	uint32 param4;
	uint32 param5;
	uint32 param6;
	uint32 param7;
	uint32 param8;
	uint32 param9;

} ActionCommand;

typedef struct ActionDefinitions {
	ActionCommand commands[MAX_ACTION_COMMANDS];
	uint32        field_1BAC;
	uint32        field_1BB0;
	uint32        counter;

} ActionDefinitions;

class ActionList {
public:
	ActionList(Common::SeekableReadStream *stream);
	virtual ~ActionList();

	uint32 size;
	uint32 numEntries;

	Common::Array<ActionDefinitions> entries;

private:

	void load(Common::SeekableReadStream *stream);

};

} // end of namespace Asylum

#endif
