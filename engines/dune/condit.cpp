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

#include "common/textconsole.h"

#include "dune/condit.h"
#include "dune/detection.h"

namespace Dune {

enum {
	kEndOfProgram = 0xff,
	kBracket = 0x80,
	kNumberByte = 0x80,
	kStateByte = 0x01
};

bool Condit::load(Archive &archive) {
	Resource file = archive.openMember("CONDIT.HSQ");
	if (!file) {
		warning("Dune: CONDIT.HSQ is missing");
		return false;
	}

	const uint32 size = file->size();
	Common::Array<byte> data;
	data.resize(size);
	file->read(data.begin(), size);

	const uint count = READ_LE_UINT16(data.begin()) / 2;
	if (count == 0 || 2 * count > size)
		return false;

	_programs.resize(count);

	for (uint i = 0; i != count; i++) {
		const uint32 start = READ_LE_UINT16(data.begin() + 2 * i);
		const uint32 end = i + 1 < count ? READ_LE_UINT16(data.begin() + 2 * (i + 1)) : size;

		for (uint32 at = start; at < end && at < size; at++)
			_programs[i].push_back(data[at]);
	}

	debugC(1, kDebugScript, "CONDIT.HSQ: %u expressions", count);
	return true;
}

namespace {

// Reads one term, moving past what it takes.
uint16 readTerm(const Common::Array<byte> &program, uint &at, const byte *state, uint stateSize) {
	if (at >= program.size())
		return 0;

	const byte kind = program[at++];

	if (kind >= kNumberByte) {
		if (kind == kNumberByte)
			return at < program.size() ? program[at++] : 0;

		if (at + 1 >= program.size())
			return 0;

		const uint16 number = program[at] | (program[at + 1] << 8);
		at += 2;
		return number;
	}

	if (at >= program.size())
		return 0;

	const byte where = program[at++];
	if (where >= stateSize)
		return 0;

	if (kind == kStateByte)
		return state[where];

	return (uint)(where + 1) < stateSize ? (state[where] | (state[where + 1] << 8)) : 0;
}

uint16 apply(byte op, uint16 left, uint16 right) {
	const uint16 kTrue = 0xffff;

	switch (op) {
	case 0x00: return left == right ? kTrue : 0;
	case 0x02: return left < right ? kTrue : 0;
	case 0x04: return left > right ? kTrue : 0;
	case 0x06: return left != right ? kTrue : 0;
	case 0x08: return (int16)left <= (int16)right ? kTrue : 0;
	case 0x0a: return (int16)left >= (int16)right ? kTrue : 0;
	case 0x0c: return left + right;
	case 0x0e: return left - right;
	case 0x10: return left & right;
	case 0x12: return left | right;
	default:   return 0;
	}
}

} // End of anonymous namespace

bool Condit::evaluate(uint index, const byte *state, uint stateSize) const {
	if (index >= _programs.size())
		return true;

	const Common::Array<byte> &program = _programs[index];
	if (program.empty())
		return true;

	// A396 saves each segment with its joining operator. These are not
	// nested brackets: A3D4 folds them in their original, forward order.
	Common::Array<uint16> saved;
	Common::Array<byte> savedOps;

	uint at = 0;
	uint16 value = readTerm(program, at, state, stateSize);

	while (at < program.size()) {
		const byte op = program[at++];

		if (op == kEndOfProgram)
			break;

		if (op >= kBracket) {
			saved.push_back(value);
			savedOps.push_back(op);
			value = readTerm(program, at, state, stateSize);
			continue;
		}

		value = apply(op, value, readTerm(program, at, state, stateSize));
	}

	if (!saved.empty()) {
		saved.push_back(value);
		value = saved[0];
		for (uint i = 0; i < savedOps.size(); i++)
			// A334 masks the same operator table for segment joins. In
			// particular, 90 is bitwise AND, not a logical truth conversion.
			value = apply(savedOps[i] & 0x1f, value, saved[i + 1]);
	}

	return value != 0;
}

namespace {

Common::String describeTerm(const Common::Array<byte> &program, uint &at) {
	if (at >= program.size())
		return Common::String("?");

	const byte kind = program[at++];

	if (kind >= kNumberByte) {
		if (kind == kNumberByte)
			return at < program.size() ? Common::String::format("%d", program[at++]) : Common::String("?");

		if (at + 1 >= program.size())
			return Common::String("?");

		const uint16 number = program[at] | (program[at + 1] << 8);
		at += 2;
		return Common::String::format("%d", number);
	}

	if (at >= program.size())
		return Common::String("?");

	return Common::String::format("%s[%02x]", kind == kStateByte ? "byte" : "word", program[at++]);
}

} // End of anonymous namespace

/**
 * Whether the expression reads a given byte of the game's own state.  A byte
 * of 0x01 in a program is followed by the address it reads, so the program is
 * walked the same way evaluate walks it and every such pair is compared.
 */
bool Condit::reads(uint index, byte where) const {
	if (index >= _programs.size())
		return false;

	const Common::Array<byte> &program = _programs[index];
	if (program.empty())
		return false;

	// The program is walked the same way evaluate walks it, so that a number
	// which happens to carry the same byte is not taken for an address.
	struct Term {
		static bool reads(const Common::Array<byte> &program, uint &at, byte where) {
			if (at >= program.size())
				return false;

			const byte kind = program[at++];

			if (kind >= kNumberByte) {
				at += kind == kNumberByte ? 1 : 2;
				return false;
			}

			return at < program.size() && program[at++] == where;
		}
	};

	uint at = 0;
	bool found = Term::reads(program, at, where);

	while (at < program.size()) {
		const byte op = program[at++];

		if (op == kEndOfProgram)
			break;

		found = Term::reads(program, at, where) || found;
	}

	return found;
}

Common::String Condit::describe(uint index) const {
	if (index >= _programs.size())
		return Common::String();

	const Common::Array<byte> &program = _programs[index];
	Common::String text;
	uint at = 0;

	// A term and an operator take turns, which is the only thing telling a
	// number of 0x80 from a bracket of 0x80.
	text += describeTerm(program, at);

	while (at < program.size()) {
		const byte op = program[at++];

		if (op == kEndOfProgram)
			break;

		if (op >= kBracket) {
			text += op == 0x90 ? " && " : Common::String::format(" [%02x] ", op);
			text += describeTerm(program, at);
			continue;
		}

		static const char *const kNames[] = {
			"==", "?", "<", "?", ">", "?", "!=", "?", "<=", "?",
			">=", "?", "+", "?", "-", "?", "&", "?", "|"
		};

		text += Common::String::format(" %s ", op < ARRAYSIZE(kNames) ? kNames[op] : "?");
		text += describeTerm(program, at);
	}

	return text;
}

} // End of namespace Dune
