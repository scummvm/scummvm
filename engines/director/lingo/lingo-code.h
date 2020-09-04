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

Datum mapBinaryOp(Datum (*func)(Datum &, Datum &), Datum &d1, Datum &d2);
Datum addData(Datum &d1, Datum &d2);
void c_add();
Datum subData(Datum &d1, Datum &d2);
void c_sub();
Datum mulData(Datum &d1, Datum &d2);
void c_mul();
Datum divData(Datum &d1, Datum &d2);
void c_div();
Datum modData(Datum &d1, Datum &d2);
void c_mod();
Datum negateData(Datum &d1);
void c_negate();

void c_and();
void c_or();
void c_not();

void c_ampersand();
void c_putafter();
void c_putbefore();
void c_concat();
void c_contains();
void c_starts();

void c_intersects();
void c_within();
Datum chunkRef(ChunkType type, int startChunk, int endChunk, const Datum &src);
void c_of();
void c_charOf();
void c_charToOf();
void c_itemOf();
void c_itemToOf();
void c_lineOf();
void c_lineToOf();
void c_wordOf();
void c_wordToOf();

void c_constpush();
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
void c_stackpeek();
void c_stackdrop();
void c_assign();
bool verify(const Symbol &s);
void c_eval();
void c_setImmediate();

void c_swap();

void c_theentitypush();
void c_themenuentitypush();
void c_theentityassign();
void c_themenuitementityassign();
void c_objectproppush();
void c_objectpropassign();

void c_whencode();
void c_tell();
void c_telldone();
Datum compareArrays(Datum (*compareFunc)(Datum, Datum), Datum d1, Datum d2, bool location = false, bool value = false);
Datum eqData(Datum d1, Datum d2);
void c_eq();
Datum neqData(Datum d1, Datum d2);
void c_neq();
Datum gtData(Datum d1, Datum d2);
void c_gt();
Datum ltData(Datum d1, Datum d2);
void c_lt();
Datum geData(Datum d1, Datum d2);
void c_ge();
Datum leData(Datum d1, Datum d2);
void c_le();
void c_jump();
void c_jumpifz();
void c_callcmd();
void c_callfunc();

void call(const Symbol &targetSym, int nargs, bool allowRetVal);
void call(const Common::String &name, int nargs, bool allowRetVal);

void c_procret();

void c_mci();
void c_mciwait();
void c_goto();
void c_gotoloop();
void c_gotonext();
void c_gotoprevious();

void c_play();

void c_open();
void c_hilite();

// custom instructions for testing
void c_asserterror();
void c_asserterrordone();

// stubs for unknown instructions
void cb_unk();
void cb_unk1();
void cb_unk2();

// bytecode-related instructions
void cb_call();
void cb_delete();
void cb_field();
void cb_globalassign();
void cb_globalpush();
void cb_list();
void cb_localcall();
void cb_objectcall();
void cb_objectfieldassign();
void cb_objectfieldpush();
void cb_varrefpush();
void cb_theassign();
void cb_theassign2();
void cb_thepush();
void cb_thepush2();
void cb_proplist();
void cb_varassign();
void cb_varpush();
void cb_v4assign();
void cb_v4assign2();
void cb_v4theentitypush();
void cb_v4theentitynamepush();
void cb_v4theentityassign();
void cb_zeropush();

} // End of namespace LC

} // End of namespace Director

#endif
