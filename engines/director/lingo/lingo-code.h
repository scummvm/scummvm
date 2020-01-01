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

#ifndef DIRECTOR_LINGO_LINGO_CODE_H
#define DIRECTOR_LINGO_LINGO_CODE_H

namespace Director {

class LC {
public:
	static void c_xpop();
	static void c_printtop();

	static void c_add();
	static void c_sub();
	static void c_mul();
	static void c_div();
	static void c_mod();
	static void c_negate();

	static void c_and();
	static void c_or();
	static void c_not();

	static void c_ampersand();
	static void c_after();
	static void c_before();
	static void c_concat();
	static void c_contains();
	static void c_starts();

	static void c_intersects();
	static void c_within();
	static void c_field();
	static void c_of();
	static void c_charOf();
	static void c_charToOf();
	static void c_itemOf();
	static void c_itemToOf();
	static void c_lineOf();
	static void c_lineToOf();
	static void c_wordOf();
	static void c_wordToOf();

	static void c_intpush();
	static void c_voidpush();
	static void c_floatpush();
	static void c_stringpush();
	static void c_symbolpush();
	static void c_namepush();
	static void c_varpush();
	static void c_argcpush();
	static void c_argcnoretpush();
	static void c_arraypush();
	static void c_assign();
	static bool verify(Symbol *s);
	static void c_eval();
	static void c_setImmediate();

	static void c_swap();

	static void c_theentitypush();
	static void c_theentityassign();
	static void c_objectfieldpush();
	static void c_objectfieldassign();

	static void c_repeatwhilecode();
	static void c_repeatwithcode();
	static void c_ifcode();
	static void c_whencode();
	static void c_tellcode();
	static void c_tell();
	static void c_telldone();
	static void c_exitRepeat();
	static void c_eq();
	static void c_neq();
	static void c_gt();
	static void c_lt();
	static void c_ge();
	static void c_le();
	static void c_jump();
	static void c_jumpifz();
	static void c_call();

	static void call(Common::String name, int nargs);

	static void c_procret();

	static void c_mci();
	static void c_mciwait();
	static void c_goto();
	static void c_gotoloop();
	static void c_gotonext();
	static void c_gotoprevious();
	static void c_global();
	static void c_instance();
	static void c_factory();
	static void c_property();

	static void c_play();
	static void c_playdone();

	static void c_open();
	static void c_hilite();

	// stubs for unknown instructions
	static void c_unk();
	static void c_unk1();
	static void c_unk2();

	// bytecode-related instructions
	static void cb_localcall();
	static void cb_call();
	static void cb_v4theentitypush();
	static void cb_v4theentitynamepush();
	static void cb_v4theentityassign();

};

extern LC *g_lc;

} // End of namespace Director

#endif
