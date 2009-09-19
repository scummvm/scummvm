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

#include "asylum/actionlist.h"

namespace Asylum {

ActionList::ActionList(Common::SeekableReadStream *stream) {
	load(stream);
}

ActionList::~ActionList() {
	entries.clear();
}

void ActionList::load(Common::SeekableReadStream *stream) {
	size       = stream->readUint32LE();
	numEntries = stream->readUint32LE();

	for (uint32 a = 0; a < numEntries; a++) {
		ActionDefinitions action;
		memset(&action, 0, sizeof(ActionDefinitions));

		for (uint32 c = 0; c < MAX_ACTION_COMMANDS; c++) {
			ActionCommand command;
			memset(&command, 0, sizeof(ActionCommand));

			command.numLines = stream->readUint32LE();
			command.opcode   = stream->readUint32LE(); // command type
			command.param1   = stream->readUint32LE(); // command parameters
			command.param2   = stream->readUint32LE();
			command.param3   = stream->readUint32LE();
			command.param4   = stream->readUint32LE();
			command.param5   = stream->readUint32LE();
			command.param6   = stream->readUint32LE();
			command.param7   = stream->readUint32LE();
			command.param8   = stream->readUint32LE();
			command.param9   = stream->readUint32LE();

			action.commands[c] = command;
		}

		action.field_1BAC = stream->readUint32LE();
		action.field_1BB0 = stream->readUint32LE();
		action.counter    = stream->readUint32LE();

		entries.push_back(action);
	}
}

} // end of namespace Asylum
