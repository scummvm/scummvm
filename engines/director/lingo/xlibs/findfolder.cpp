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

/*************************************
 *
 * USED IN:
 * Yearn2Learn: The Flintstones Coloring Book
 *
 *************************************/

/*
 *                                              FindFolder 1.1
 *
 *                                         by Frédéric RINALDI
 *
 *
 * DESCRIPTION
 * ------------
 *   FindFolder returns the full pathname of the active System folder or any of its reserved folders (System 7.0).
 *
 *
 * SYNTAX
 * -------
 *                         FindFolder([<folder descriptor>[,<create it>]])
 *
 * PARAMETERS
 * ------------
 *    <folder descriptor> is a string describing the folder. It must be one of these values (only first char is checked) :
 *           A[pple menu]
 *           C[ontrol Panel]
 *           D[esktop]
 *           E[xtensions]
 *           F[onts] (System 7.1 only)
 *           P[references]
 *           M[onitor printing]
 *           N[etwork trash]
 *           T[rash]
 *           I[tems startup]
 *           S[ystem]
 *           Y : TemporarY items
 *
 *    Default value is "S".
 *
 *    <create it> is a boolean telling if the XFCN must create the folder if not found (only with System 7.0). Default is FALSE.
 *
 *    Both parameters are optional. Using "!", "?" or "=" as first parameter will return an online help (resp. copyright,  syntax and output).
 *
 * USING
 * -----
 *   The XFCN returns the full pathname of the requested folder, ending with colon.
 *    Under System 6.0, only current System folder can be returned.
 *
 * ERRORS
 * -------
 * If an error occurs, the XFCN can return :
 *      "Error : Empty parameter"
 *      "Error : Second param must be boolean"
 *      "Error : Folder not found"
 *      "Error : Found file instead of folder"
 *
 * HISTORY
 * -------
 * 1.1 :                                                                                          04/21/92
 * • Added "font" param for System 7.1
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/findfolder.h"


namespace Director {

const char *const FindFolder::xlibName = "FindFolder";
const XlibFileDesc FindFolder::fileNames[] = {
	{ "FindFolder",	nullptr },
	{ nullptr,		nullptr },
};

static const BuiltinProto builtins[] = {
	{ "FindFolder", FindFolder::m_findfolder, 0, 2, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void FindFolder::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);
}

void FindFolder::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void FindFolder::m_findfolder(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

} // End of namespace Director
