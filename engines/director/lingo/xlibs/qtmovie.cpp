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
 * L-Zone (Mac II)
 *
 *************************************/
/*
  QTMovie OpenMovie, windowType, fileName, location, options...
*/

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/qtmovie.h"


namespace Director {

const char *QTMovie::xlibName = "QTMovie";
const char *QTMovie::fileNames[] = {
	"QTMovie",
	nullptr
};

static BuiltinProto builtins[] = {
	{ "QTMovie", QTMovie::m_qtmovie, 3, 6, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void QTMovie::open(int type) {
	g_lingo->initBuiltIns(builtins);
}

void QTMovie::close(int type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void QTMovie::m_qtmovie(int nargs) {
	g_lingo->printSTUBWithArglist("QTMovie::m_qtmovie", nargs);
	g_lingo->dropStack(nargs);
}

} // End of namespace Director
