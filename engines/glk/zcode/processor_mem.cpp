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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/zcode/processor.h"

namespace Glk {
namespace ZCode {

void Processor::flagsChanged(zbyte value) {
	if (value & SCRIPTING_FLAG) {
		if (!ostream_script)
			script_open();
	} else {
		if (ostream_script)
			script_close();
	}
}

int Processor::save_undo() {
	long diff_size;
	zword stack_size;
	undo_t *p;

	if (_undo_slots == 0)
		// undo feature unavailable
		return -1;

	// save undo possible
	while (last_undo != curr_undo) {
		p = last_undo;
		last_undo = last_undo->prev;
		delete p;
		undo_count--;
	}
	if (last_undo)
		last_undo->next = nullptr;
	else
		first_undo = nullptr;

	if (undo_count == _undo_slots)
		free_undo(1);

	diff_size = mem_diff(zmp, prev_zmp, h_dynamic_size, undo_diff);
	stack_size = _stack + STACK_SIZE - _sp;
	do {
		p = (undo_t *) malloc(sizeof(undo_t) + diff_size + stack_size * sizeof(*_sp));
		if (p == nullptr)
			free_undo(1);
	} while (!p && undo_count);
	if (p == nullptr)
		return -1;

	GET_PC(p->pc);
	p->frame_count = _frameCount;
	p->diff_size = diff_size;
	p->stack_size = stack_size;
	p->frame_offset = _fp - _stack;
	memcpy(p + 1, undo_diff, diff_size);
	memcpy((zbyte *)(p + 1) + diff_size, _sp, stack_size * sizeof(*_sp));

	if (!first_undo) {
		p->prev = nullptr;
		first_undo = p;
	} else {
		last_undo->next = p;
		p->prev = last_undo;
	}

	p->next = nullptr;
	curr_undo = last_undo = p;
	undo_count++;

	return 1;
}

int Processor::restore_undo(void) {
	if (_undo_slots == 0)
		// undo feature unavailable
		return -1;

	if (curr_undo == nullptr)
		// no saved game state
		return 0;

	// undo possible
	memcpy(zmp, prev_zmp, h_dynamic_size);
	SET_PC(curr_undo->pc);
	_sp = _stack + STACK_SIZE - curr_undo->stack_size;
	_fp = _stack + curr_undo->frame_offset;
	_frameCount = curr_undo->frame_count;
	mem_undiff((zbyte *)(curr_undo + 1), curr_undo->diff_size, prev_zmp);
	memcpy(_sp, (zbyte *)(curr_undo + 1) + curr_undo->diff_size,
		curr_undo->stack_size * sizeof(*_sp));

	curr_undo = curr_undo->prev;

	restart_header();

	return 2;
}

/**
 * TOR: glkify -- this is for V6 only
 */
static zword get_max_width(zword win) { return 80; }

void Processor::memory_open(zword table, zword xsize, bool buffering) {
	if (_redirect.size() < MAX_NESTING) {
		if (!buffering)
			xsize = 0xffff;
		if (buffering && (short)xsize <= 0)
			xsize = get_max_width((zword)(-(short)xsize));

		storew(table, 0);

		_redirect.push(Redirect(xsize, table));
		ostream_memory = true;
	} else {
		runtimeError(ERR_STR3_NESTING);
	}
}

void Processor::memory_new_line() {
	zword size;
	zword addr;

	Redirect &r = _redirect.top();
	r._total += r._width;
	r._width = 0;

	addr = r._table;

	LOW_WORD(addr, size);
	addr += 2;

	if (r._xSize != 0xffff) {
		r._table = addr + size;
		size = 0;
	} else {
		storeb((zword)(addr + (size++)), 13);
	}

	storew(r._table, size);
}

void Processor::memory_word(const zchar *s) {
	zword size;
	zword addr;
	zchar c;

	Redirect &r = _redirect.top();
	if (h_version == V6) {
		int width = os_string_width(s);

		if (r._xSize != 0xffff) {
			if (r._width + width > r._xSize) {

				if (*s == ' ' || *s == ZC_INDENT || *s == ZC_GAP)
					width = os_string_width(++s);

				memory_new_line();
			}
		}

		r._width += width;
	}

	addr = r._table;

	LOW_WORD(addr, size);
	addr += 2;

	while ((c = *s++) != 0)
		storeb((zword)(addr + (size++)), translate_to_zscii(c));

	storew(r._table, size);
}

void Processor::memory_close(void) {
	if (!_redirect.empty()) {
		Redirect &r = _redirect.top();

		if (r._xSize != 0xffff)
			memory_new_line();

		if (h_version == V6) {
			h_line_width = (r._xSize != 0xffff) ? r._total : r._width;

			SET_WORD(H_LINE_WIDTH, h_line_width);
		}

		_redirect.pop();
		if (_redirect.empty())
			ostream_memory = false;
	}
}

} // End of namespace ZCode
} // End of namespace Glk
