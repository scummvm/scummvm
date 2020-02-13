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

namespace LC {
	void c_xpop();
	void c_printtop();

	void c_add();
	void c_sub();
	void c_mul();
	void c_div();
	void c_mod();
	void c_negate();

	void c_and();
	void c_or();
	void c_not();

	void c_ampersand();
	void c_after();
	void c_before();
	void c_concat();
	void c_contains();
	void c_starts();

	void c_intersects();
	void c_within();
	void c_of();
	void c_charOf();
	void c_charToOf();
	void c_itemOf();
	void c_itemToOf();
	void c_lineOf();
	void c_lineToOf();
	void c_wordOf();
	void c_wordToOf();

	void c_intpush();
	void c_voidpush();
	void c_floatpush();
	void c_stringpush();
	void c_symbolpush();
	void c_namepush();
	void c_varpush();
	void c_argcpush();
	void c_argcnoretpush();
	void c_arraypush();
	void c_proparraypush();
	void c_assign();
	bool verify(Symbol *s);
	void c_eval();
	void c_setImmediate();

	void c_swap();

	void c_theentitypush();
	void c_theentityassign();
	void c_themenuitementityassign();
	void c_objectfieldpush();
	void c_objectfieldassign();
	void c_objectrefpush();

	void c_repeatwhilecode();
	void c_repeatwithcode();
	void c_nextRepeat();
	void c_ifcode();
	void c_whencode();
	void c_tellcode();
	void c_tell();
	void c_telldone();
	void c_exitRepeat();
	void c_eq();
	void c_neq();
	void c_gt();
	void c_lt();
	void c_ge();
	void c_le();
	void c_jump();
	void c_jumpifz();
	void c_call();

	void call(Symbol *, int nargs);
	void call(Common::String name, int nargs);

	void c_procret();

	void c_mci();
	void c_mciwait();
	void c_goto();
	void c_gotoloop();
	void c_gotonext();
	void c_gotoprevious();
	void c_global();
	void c_instance();
	void c_factory();
	void c_property();

	void c_play();
	void c_playdone();

	void c_open();
	void c_hilite();

	// stubs for unknown instructions
	void cb_unk();
	void cb_unk1();
	void cb_unk2();

	// bytecode-related instructions
	void cb_call();
	void cb_field();
	void cb_globalassign();
	void cb_globalpush();
	void cb_list();
	void cb_localcall();
	void cb_methodcall();
	void cb_objectpush();
	void cb_varassign();
	void cb_varpush();
	void cb_v4assign();
	void cb_v4theentitypush();
	void cb_v4theentitynamepush();
	void cb_v4theentityassign();
	void cb_zeropush();

} // End of namespace LC

} // End of namespace Director

#endif
