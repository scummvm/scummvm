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

#ifndef DIRECTOR_LINGO_BUILTINS_H
#define DIRECTOR_LINGO_BUILTINS_H

namespace Director {

namespace LB {

// builtin functions
void b_abs(int nargs);
void b_atan(int nargs);
void b_cos(int nargs);
void b_exp(int nargs);
void b_float(int nargs);
void b_integer(int nargs);
void b_log(int nargs);
void b_pi(int nargs);
void b_power(int nargs);
void b_random(int nargs);
void b_sin(int nargs);
void b_sqrt(int nargs);
void b_tan(int nargs);

void b_chars(int nargs);
void b_charToNum(int nargs);
void b_length(int nargs);
void b_numToChar(int nargs);
void b_offset(int nargs);
void b_string(int nargs);

void b_add(int nargs);
void b_addAt(int nargs);
void b_addProp(int nargs);
void b_append(int nargs);
void b_count(int nargs);
void b_deleteAt(int nargs);
void b_deleteOne(int nargs);
void b_deleteProp(int nargs);
void b_duplicateList(int nargs);
void b_findPos(int nargs);
void b_findPosNear(int nargs);
void b_getaProp(int nargs);
void b_getAt(int nargs);
void b_getLast(int nargs);
void b_getOne(int nargs);
void b_getPos(int nargs);
void b_getProp(int nargs);
void b_getPropAt(int nargs);
void b_list(int nargs);
void b_listP(int nargs);
void b_max(int nargs);
void b_min(int nargs);
void b_setaProp(int nargs);
void b_setAt(int nargs);
void b_setProp(int nargs);
void b_sort(int nargs);

void b_factory(int nargs);
void b_floatP(int nargs);
void b_ilk(int nargs);
void b_integerp(int nargs);
void b_objectp(int nargs);
void b_pictureP(int nargs);
void b_stringp(int nargs);
void b_symbolp(int nargs);
void b_voidP(int nargs);

void b_alert(int nargs);
void b_clearGlobals(int nargs);
void b_cursor(int nargs);
void b_framesToHMS(int nargs);
void b_HMStoFrames(int nargs);
void b_param(int nargs);
void b_printFrom(int nargs);
void b_put(int nargs);
void b_showGlobals(int nargs);
void b_showLocals(int nargs);
void b_value(int nargs);

void b_constrainH(int nargs);
void b_constrainV(int nargs);
void b_copyToClipBoard(int nargs);
void b_duplicate(int nargs);
void b_editableText(int nargs);
void b_erase(int nargs);
void b_findEmpty(int nargs);
void b_importFileInto(int nargs);
void b_installMenu(int nargs);
void b_label(int nargs);
void b_marker(int nargs);
void b_move(int nargs);
void b_moveableSprite(int nargs);
void b_pasteClipBoardInto(int nargs);
void b_puppetPalette(int nargs);
void b_puppetSound(int nargs);
void b_puppetSprite(int nargs);
void b_puppetTempo(int nargs);
void b_puppetTransition(int nargs);
void b_ramNeeded(int nargs);
void b_rollOver(int nargs);
void b_spriteBox(int nargs);
void b_unLoad(int nargs);
void b_unLoadCast(int nargs);
void b_unLoadMovie(int nargs);
void b_updateStage(int nargs);
void b_zoomBox(int nargs);
void b_immediateSprite(int nargs);

void b_clearFrame(int nargs);
void b_deleteFrame(int nargs);
void b_duplicateFrame(int nargs);
void b_insertFrame(int nargs);
void b_updateFrame(int nargs);

void b_abort(int nargs);
void b_cancelIdleLoad(int nargs);
void b_continue(int nargs);
void b_dontPassEvent(int nargs);
void b_delay(int nargs);
void b_do(int nargs);
void b_finishIdleLoad(int nargs);
void b_go(int nargs);
void b_halt(int nargs);
void b_idleLoadDone(int nargs);
void b_nothing(int nargs);
void b_pass(int nargs);
void b_pause(int nargs);
void b_play(int nargs);
void b_playAccel(int nargs);
void b_preLoad(int nargs);
void b_preLoadCast(int nargs);
void b_preLoadMovie(int nargs);
void b_quit(int nargs);
void b_restart(int nargs);
void b_shutDown(int nargs);
void b_startTimer(int nargs);
void b_return(int nargs);

void b_closeDA(int nargs);
void b_closeResFile(int nargs);
void b_closeXlib(int nargs);
void b_getNthFileNameInFolder(int nargs);
void b_open(int nargs);
void b_openDA(int nargs);
void b_openResFile(int nargs);
void b_openXlib(int nargs);
void b_setCallBack(int nargs);
void b_save(int nargs);
void b_saveMovie(int nargs);
void b_showResFile(int nargs);
void b_showXlib(int nargs);
void b_xFactoryList(int nargs);
void b_xtra(int nargs);

void b_point(int nargs);
void b_inflate(int nargs);
void b_inside(int nargs);
void b_intersect(int nargs);
void b_map(int nargs);
void b_offsetRect(int nargs);
void b_rect(int nargs);
void b_union(int nargs);

void b_beep(int nargs);
void b_mci(int nargs);
void b_mciwait(int nargs);
void b_sound(int nargs);
void b_soundBusy(int nargs);

void b_backspace(int nargs);
void b_empty(int nargs);
void b_enter(int nargs);
void b_false(int nargs);
void b_quote(int nargs);
void b_returnconst(int nargs);
void b_tab(int nargs);
void b_true(int nargs);
void b_version(int nargs);

void b_cast(int nargs);
void b_castLib(int nargs);
void b_member(int nargs);
void b_script(int nargs);
void b_sprite(int nargs);
void b_window(int nargs);
void b_windowPresent(int nargs);

void b_charPosToLoc(int nargs);
void b_linePosToLocV(int nargs);
void b_locToCharPos(int nargs);
void b_locVToLinePos(int nargs);
void b_scrollByLine(int nargs);
void b_scrollByPage(int nargs);
void b_lineHeight(int nargs);

void b_numberofchars(int nargs);
void b_numberofitems(int nargs);
void b_numberoflines(int nargs);
void b_numberofwords(int nargs);

void b_trackCount(int nargs);
void b_trackStartTime(int nargs);
void b_trackStopTime(int nargs);
void b_trackType(int nargs);

void b_scummvmassert(int nargs);
void b_scummvmassertequal(int nargs);
void b_scummvmNoFatalError(int nargs);

// XCMD/XFCN (HyperCard), normally exposed
void b_getVolumes(int nargs);

} // End of namespace LB

} // End of namespace Director

#endif
