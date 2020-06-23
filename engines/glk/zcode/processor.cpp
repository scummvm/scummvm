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
#include "glk/zcode/zcode.h"
#include "glk/conf.h"

namespace Glk {
namespace ZCode {

// TODO: Stubs to replace with actual code
zword save_undo() { return 0; }
zword restore_undo() { return 0; }


Opcode Processor::var_opcodes[64] = {
	&Processor::__illegal__,
	&Processor::z_je,
	&Processor::z_jl,
	&Processor::z_jg,
	&Processor::z_dec_chk,
	&Processor::z_inc_chk,
	&Processor::z_jin,
	&Processor::z_test,
	&Processor::z_or,
	&Processor::z_and,
	&Processor::z_test_attr,
	&Processor::z_set_attr,
	&Processor::z_clear_attr,
	&Processor::z_store,
	&Processor::z_insert_obj,
	&Processor::z_loadw,
	&Processor::z_loadb,
	&Processor::z_get_prop,
	&Processor::z_get_prop_addr,
	&Processor::z_get_next_prop,
	&Processor::z_add,
	&Processor::z_sub,
	&Processor::z_mul,
	&Processor::z_div,
	&Processor::z_mod,
	&Processor::z_call_s,
	&Processor::z_call_n,
	&Processor::z_set_colour,
	&Processor::z_throw,
	&Processor::__illegal__,
	&Processor::__illegal__,
	&Processor::__illegal__,
	&Processor::z_call_s,
	&Processor::z_storew,
	&Processor::z_storeb,
	&Processor::z_put_prop,
	&Processor::z_read,
	&Processor::z_print_char,
	&Processor::z_print_num,
	&Processor::z_random,
	&Processor::z_push,
	&Processor::z_pull,
	&Processor::z_split_window,
	&Processor::z_set_window,
	&Processor::z_call_s,
	&Processor::z_erase_window,
	&Processor::z_erase_line,
	&Processor::z_set_cursor,
	&Processor::z_get_cursor,
	&Processor::z_set_text_style,
	&Processor::z_buffer_mode,
	&Processor::z_output_stream,
	&Processor::z_input_stream,
	&Processor::z_sound_effect,
	&Processor::z_read_char,
	&Processor::z_scan_table,
	&Processor::z_not,
	&Processor::z_call_n,
	&Processor::z_call_n,
	&Processor::z_tokenise,
	&Processor::z_encode_text,
	&Processor::z_copy_table,
	&Processor::z_print_table,
	&Processor::z_check_arg_count
};

Opcode Processor::ext_opcodes[64] = {
	&Processor::z_save,
	&Processor::z_restore,
	&Processor::z_log_shift,
	&Processor::z_art_shift,
	&Processor::z_set_font,
	&Processor::z_draw_picture,
	&Processor::z_picture_data,
	&Processor::z_erase_picture,
	&Processor::z_set_margins,
	&Processor::z_save_undo,
	&Processor::z_restore_undo,
	&Processor::z_print_unicode,
	&Processor::z_check_unicode,
	&Processor::z_set_true_colour,	// spec 1.1
	&Processor::__illegal__,
	&Processor::__illegal__,
	&Processor::z_move_window,
	&Processor::z_window_size,
	&Processor::z_window_style,
	&Processor::z_get_wind_prop,
	&Processor::z_scroll_window,
	&Processor::z_pop_stack,
	&Processor::z_read_mouse,
	&Processor::z_mouse_window,
	&Processor::z_push_stack,
	&Processor::z_put_wind_prop,
	&Processor::z_print_form,
	&Processor::z_make_menu,
	&Processor::z_picture_table,
	&Processor::z_buffer_screen		// spec 1.1
};

Processor::Processor(OSystem *syst, const GlkGameDescription &gameDesc) :
		GlkInterface(syst, gameDesc),
		_finished(0), _sp(nullptr), _fp(nullptr), _frameCount(0),
		zargc(0), _decoded(nullptr), _encoded(nullptr), _resolution(0),
		_randomInterval(0), _randomCtr(0), first_restart(true), script_valid(false),
		_bufPos(0), _locked(false), _prevC('\0'), script_width(0),
		sfp(nullptr), rfp(nullptr), pfp(nullptr), ostream_screen(true), ostream_script(false),
		ostream_memory(false), ostream_record(false), istream_replay(false), message(false) {
	static const Opcode OP0_OPCODES[16] = {
		&Processor::z_rtrue,
		&Processor::z_rfalse,
		&Processor::z_print,
		&Processor::z_print_ret,
		&Processor::z_nop,
		&Processor::z_save,
		&Processor::z_restore,
		&Processor::z_restart,
		&Processor::z_ret_popped,
		&Processor::z_catch,
		&Processor::z_quit,
		&Processor::z_new_line,
		&Processor::z_show_status,
		&Processor::z_verify,
		&Processor::__extended__,
		&Processor::z_piracy
	};
	static const Opcode OP1_OPCODES[16] = {
		&Processor::z_jz,
		&Processor::z_get_sibling,
		&Processor::z_get_child,
		&Processor::z_get_parent,
		&Processor::z_get_prop_len,
		&Processor::z_inc,
		&Processor::z_dec,
		&Processor::z_print_addr,
		&Processor::z_call_s,
		&Processor::z_remove_obj,
		&Processor::z_print_obj,
		&Processor::z_ret,
		&Processor::z_jump,
		&Processor::z_print_paddr,
		&Processor::z_load,
		&Processor::z_call_n
	};

	op0_opcodes.resize(16);
	op1_opcodes.resize(16);
	Common::copy(&OP0_OPCODES[0], &OP0_OPCODES[16], &op0_opcodes[0]);
	Common::copy(&OP1_OPCODES[0], &OP1_OPCODES[16], &op1_opcodes[0]);
	Common::fill(&_stack[0], &_stack[STACK_SIZE], 0);
	Common::fill(&zargs[0], &zargs[8], 0);
	Common::fill(&_buffer[0], &_buffer[TEXT_BUFFER_SIZE], '\0');
	Common::fill(&_errorCount[0], &_errorCount[ERR_NUM_ERRORS], 0);
}

void Processor::initialize() {
	Mem::initialize();
	GlkInterface::initialize();

	if (h_version <= V4) {
		op0_opcodes[9] = &Processor::z_pop;
		op1_opcodes[15] = &Processor::z_not;
	} else {
		op0_opcodes[9] = &Processor::z_catch;
		op1_opcodes[15] = &Processor::z_call_n;
	}
}

void Processor::load_operand(zbyte type) {
	zword value;

	if (type & 2) {
		// variable
		zbyte variable;

		CODE_BYTE(variable);

		if (variable == 0)
			value = *_sp++;
		else if (variable < 16)
			value = *(_fp - variable);
		else {
			zword addr = h_globals + 2 * (variable - 16);
			LOW_WORD(addr, value);
		}
	} else if (type & 1) {
		// small constant
		zbyte bvalue;

		CODE_BYTE(bvalue);
		value = bvalue;

	} else {
		// large constant
		CODE_WORD(value);
	}

	zargs[zargc++] = value;
}

void Processor::load_all_operands(zbyte specifier) {
	for (int i = 6; i >= 0; i -= 2) {
		zbyte type = (specifier >> i) & 0x03;

		if (type == 3)
			break;

		load_operand(type);
	}
}

void Processor::interpret() {
	do {
		zbyte opcode;
		CODE_BYTE(opcode);
		zargc = 0;

		if (opcode < 0x80) {
			// 2OP opcodes
			load_operand((zbyte)(opcode & 0x40) ? 2 : 1);
			load_operand((zbyte)(opcode & 0x20) ? 2 : 1);

			(*this.*var_opcodes[opcode & 0x1f])();

		} else if (opcode < 0xb0) {
			// 1OP opcodes
			load_operand((zbyte)(opcode >> 4));

			(*this.*op1_opcodes[opcode & 0x0f])();

		} else if (opcode < 0xc0) {
			// 0OP opcodes
			(*this.*op0_opcodes[opcode - 0xb0])();


		} else {
			// VAR opcodes
			zbyte specifier1;
			zbyte specifier2;

			if (opcode == 0xec || opcode == 0xfa) {	// opcodes 0xec
				CODE_BYTE(specifier1);			// and 0xfa are
				CODE_BYTE(specifier2);          // call opcodes
				load_all_operands(specifier1);	// with up to 8
				load_all_operands(specifier2);	// arguments
			} else {
				CODE_BYTE(specifier1);
				load_all_operands(specifier1);
			}

			(*this.*var_opcodes[opcode - 0xc0])();
		}

#if defined(DJGPP) && defined(SOUND_SUPPORT)
		if (end_of_sound_flag)
			end_of_sound();
#endif
	} while (!shouldQuit() && !_finished);

	_finished--;
}

void Processor::call(zword routine, int argc, zword *args, int ct) {
	uint32 pc;
	zword value;
	zbyte count;
	int i;

	if (_sp - _stack < 4)
		runtimeError(ERR_STK_OVF);

	GET_PC(pc);

	*--_sp = (zword)(pc >> 9);
	*--_sp = (zword)(pc & 0x1ff);
	*--_sp = (zword)(_fp - _stack - 1);
	*--_sp = (zword)(argc | (ct << (_quetzal ? 12 : 8)));

	_fp = _sp;
	_frameCount++;

	// Calculate byte address of routine
	if (h_version <= V3)
		pc = (long)routine << 1;
	else if (h_version <= V5)
		pc = (long)routine << 2;
	else if (h_version <= V7)
		pc = ((long)routine << 2) + ((long)h_functions_offset << 3);
	else if (h_version <= V8)
		pc = (long)routine << 3;
	else {
		// h_version == V9
		long indirect = (long)routine << 2;
		HIGH_LONG(indirect, pc);
	}

	if ((uint)pc >= story_size)
		runtimeError(ERR_ILL_CALL_ADDR);

	SET_PC(pc);

	// Initialise local variables
	CODE_BYTE(count);

	if (count > 15)
		runtimeError(ERR_CALL_NON_RTN);
	if (_sp - _stack < count)
		runtimeError(ERR_STK_OVF);

	if (_quetzal)
		_fp[0] |= (zword)count << 8;	// Save local var count for Quetzal.

	value = 0;

	for (i = 0; i < count; i++) {
		if (h_version <= V4)		// V1 to V4 games provide default
			CODE_WORD(value);		// values for all local variables

			*--_sp = (zword)((argc-- > 0) ? args[i] : value);
	}

	// Start main loop for direct calls
	if (ct == 2)
		interpret();
}

void Processor::ret(zword value) {
	offset_t pc;
	int ct;

	if (_sp > _fp)
		runtimeError(ERR_STK_UNDF);

	_sp = _fp;

	ct = *_sp++ >> (_quetzal ? 12 : 8);
	_frameCount--;
	_fp = _stack + 1 + *_sp++;
	pc = *_sp++;
	pc = ((offset_t)*_sp++ << 9) | pc;

	SET_PC(pc);

	// Handle resulting value
	if (ct == 0)
		store(value);
	if (ct == 2)
		*--_sp = value;

	// Stop main loop for direct calls
	if (ct == 2)
		_finished++;
}

void Processor::branch(bool flag) {
	offset_t pc;
	zword offset;
	zbyte specifier;
	zbyte off1;
	zbyte off2;

	CODE_BYTE(specifier);
	off1 = specifier & 0x3f;

	if (!flag)
		specifier ^= 0x80;

	if (!(specifier & 0x40)) {
		// it's a long branch
		if (off1 & 0x20)		// propagate sign bit
			off1 |= 0xc0;

		CODE_BYTE(off2);
		offset = (off1 << 8) | off2;
	} else {
		// It's a short branch
		offset = off1;
	}

	if (specifier & 0x80) {
		if (offset > 1) {
			// normal branch
			GET_PC(pc);
			pc += (short)offset - 2;
			SET_PC(pc);
		} else {
			// special case, return 0 or 1
			ret(offset);
		}
	}
}

void Processor::store(zword value) {
	zbyte variable;

	CODE_BYTE(variable);

	if (variable == 0)
		*--_sp = value;
	else if (variable < 16)
		*(_fp - variable) = value;
	else {
		zword addr = h_globals + 2 * (variable - 16);
		SET_WORD(addr, value);
	}
}

int Processor::direct_call(zword addr) {
	zword saved_zargs[8];
	int saved_zargc;
	int i;

	// Calls to address 0 return false
	if (addr == 0)
		return 0;

	// Save operands and operand count
	for (i = 0; i < 8; i++)
		saved_zargs[i] = zargs[i];

	saved_zargc = zargc;

	// Call routine directly
	call(addr, 0, 0, 2);

	// Restore operands and operand count
	for (i = 0; i < 8; i++)
		zargs[i] = saved_zargs[i];

	zargc = saved_zargc;

	// Resulting value lies on top of the stack
	return (short)*_sp++;
}

void Processor::seed_random(int value) {
	if (value == 0) {
		// Now using random values
		_randomInterval = 0;
	} else if (value < 1000) {
		// special seed value
		_randomCtr = 0;
		_randomInterval = value;
	} else {
		// standard seed value
		_random.setSeed(value);
		_randomInterval = 0;
	}
}

void Processor::__extended__() {
	zbyte opcode;
	zbyte specifier;

	CODE_BYTE(opcode);
	CODE_BYTE(specifier);

	load_all_operands(specifier);

	if (opcode < 0x1e)					// extended opcodes from 0x1e on
		(*this.*ext_opcodes[opcode])();	// are reserved for future spec'
}

void Processor::__illegal__() {
	runtimeError(ERR_ILL_OPCODE);
}

void Processor::z_catch() {
	store(_quetzal ? _frameCount : (zword)(_fp - _stack));
}

void Processor::z_throw() {
	if (_quetzal) {
		if (zargs[1] > _frameCount)
			runtimeError(ERR_BAD_FRAME);

		// Unwind the stack a frame at a time.
		for (; _frameCount > zargs[1]; --_frameCount)
			_fp = _stack + 1 + _fp[1];
	} else {
		if (zargs[1] > STACK_SIZE)
			runtimeError(ERR_BAD_FRAME);

		_fp = _stack + zargs[1];
	}

	ret(zargs[0]);
}

void Processor::z_call_n() {
	if (zargs[0] != 0)
		call(zargs[0], zargc - 1, zargs + 1, 1);
}

void Processor::z_call_s() {
	if (zargs[0] != 0)
		call(zargs[0], zargc - 1, zargs + 1, 0);
	else
		store(0);
}

void Processor::z_check_arg_count() {
	if (_fp == _stack + STACK_SIZE)
		branch(zargs[0] == 0);
	else
		branch(zargs[0] <= (*_fp & 0xff));
}

void Processor::z_jump() {
	offset_t pc;
	GET_PC(pc);

	pc += (short)zargs[0] - 2;

	if (pc >= story_size)
		runtimeError(ERR_ILL_JUMP_ADDR);

	SET_PC(pc);
}

void Processor::z_nop() {
	// Do nothing
}

void Processor::z_quit() {
	_finished = 9999;
}

void Processor::z_ret() {
	ret(zargs[0]);
}

void Processor::z_ret_popped() {
	ret(*_sp++);
}

void Processor::z_rfalse() {
	ret(0);
}

void Processor::z_rtrue() {
	ret(1);
}

void Processor::z_random() {
	if ((short) zargs[0] <= 0) {
		// set random seed
		seed_random(- (short) zargs[0]);
		store(0);

	} else {
		// generate random number
		zword result;
		if (_randomInterval != 0) {
			// ...in special mode
			result = _randomCtr++;
			if (_randomCtr == _randomInterval)
				_randomCtr = 0;
		} else {
			// ...in standard mode
			result = _random.getRandomNumber(0xffff);
		}

		store((zword)(result % zargs[0] + 1));
	}
}

void Processor::z_sound_effect() {
	zword number = zargs[0];
	zword effect = zargs[1];
	zword volume = zargs[2];

	if (zargc < 1)
		number = 0;
	if (zargc < 2)
		effect = EFFECT_PLAY;
	if (zargc < 3)
		volume = 8;

	if (number >= 3 || number == 0) {
		_soundLocked = true;

		if (_storyId == LURKING_HORROR && (number == 9 || number == 16)) {
			if (effect == EFFECT_PLAY) {
				next_sample = number;
				next_volume = volume;

				_soundLocked = false;

				if (!_soundPlaying)
					start_next_sample();
			} else {
				_soundLocked = false;
			}
			return;
		}

		_soundPlaying = false;

		switch (effect) {
		case EFFECT_PREPARE:
			os_prepare_sample(number);
			break;
		case EFFECT_PLAY:
			start_sample(number, lo(volume), hi(volume), (zargc == 4) ? zargs[3] : 0);
			break;
		case EFFECT_STOP:
			os_stop_sample (number);
			break;
		case EFFECT_FINISH_WITH:
			os_finish_with_sample (number);
			break;
		default:
			break;
		}

		_soundLocked = false;
	} else {
		os_beep(number);
	}
}

void Processor::z_piracy() {
	branch(!_piracy);
}

void Processor::z_save_undo(void) {
	store((zword)save_undo());
}

void Processor::z_restore_undo(void) {
	store((zword)restore_undo());
}

} // End of namespace ZCode
} // End of namespace Glk
