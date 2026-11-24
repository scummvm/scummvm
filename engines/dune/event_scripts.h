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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DUNE_EVENT_SCRIPTS_H
#define DUNE_EVENT_SCRIPTS_H

#include "common/scummsys.h"

namespace Dune {

// A1F7 selects by the CURRENT stage, independently of the speaker or the
// record that called it. A delayed first specialist order can reach134F.
inline uint16 stageEventScript(byte stage) {
	return stage < 20 ? 0x12f8 : stage < 24 ? 0x134f : stage < 48 ? 0x1370 : 0x12db;
}

// Use the already persisted gathering-menu payload, not a new transient
// counter. DS:477A remains the authoritative next instruction across saves.
enum { kStageEventContinuation = 700 };

struct StageEventInstruction {
	byte opcode;
	byte argument;
	uint16 next;
	uint16 roster;
};

// The four A1F7 programs, including their counted drawing-only rosters.
// These are control instructions, not a list of unconditional PHRASE lines:
// opcode02 asks9761 to find an eligible record in that person's event part.
inline bool stageEventInstruction(const byte *code, uint16 cursor, StageEventInstruction &out) {
	const struct Program { uint16 base; uint size; } programs[] = {
		{ 0x12db, 0x12f8 - 0x12db }, { 0x12f8, 0x12fb - 0x12f8 },
		{ 0x134f, 0x1370 - 0x134f }, { 0x1370, 0x1392 - 0x1370 }
	};
	for (uint i = 0; i < sizeof(programs) / sizeof(programs[0]); i++) {
		const Program &p = programs[i];
		const byte *data = code + p.base;
		// Walk instruction boundaries: inline roster bytes are NOT opcodes.
		for (uint at = 0; at < p.size;) {
			const byte opcode = data[at];
			uint next = at + 1;
			byte argument = 0;
			uint16 roster = 0;
			if (opcode == 0) {
				if (at + 2 >= p.size)
					return false;
				argument = data[at + 1];
				roster = p.base + at + 2;
				next = at + 3 + data[at + 2];
			} else if (opcode == 2 || opcode == 6) {
				if (at + 1 >= p.size)
					return false;
				argument = data[at + 1];
				next = at + 2;
			} else if (opcode != 4 && opcode != 8 && opcode != 14 && opcode != 16 && opcode != 255) {
				return false;
			}
			if (next > p.size)
				return false;
			if (cursor == p.base + at) {
				out.opcode = opcode;
				out.argument = argument;
				out.next = opcode == 255 ? cursor : p.base + next;
				out.roster = roster;
				return true;
			}
			at = next;
		}
	}
	return false;
}

// Redraw/load must keep a scripted room tableau even if the saved return
// context is a map. It must not replace that tableau with the return map.
inline bool stageEventHasRoomFrame(const byte *code, uint16 cursor) {
	const uint16 starts[] = { 0x12db, 0x12f8, 0x134f, 0x1370 };
	for (uint i = 0; i < sizeof(starts) / sizeof(starts[0]); i++) {
		uint16 at = starts[i];
		bool frame = false;
		StageEventInstruction op;
		while (stageEventInstruction(code, at, op)) {
			if (at == cursor)
				return frame;
			if (op.opcode == 255)
				break;
			frame |= op.opcode == 0 || op.opcode == 4;
			at = op.next;
		}
	}
	return false;
}

} // namespace Dune

#endif
