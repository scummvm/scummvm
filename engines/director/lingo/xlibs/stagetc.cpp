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
 * מיץ פטל (Mitz Petel)
 *
 *************************************/
/* -- StageTocast XObject
StageTC
IIIII  mNew, rgnTop, rgnLeft, rgnBottom, rgnRight	--Creates a new instance of the XObject
I      mDispose              --Disposes of XObject instance
I      mGetHandle            --Copies a portion of the stage onto a cast member
*/

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/stagetc.h"


namespace Director {

const char *const StageTCXObj::xlibName = "StageTC";
const XlibFileDesc StageTCXObj::fileNames[] = {
	{ "STAGETC",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "New",		StageTCXObj::m_new,			4,	4,	400 },	// D4
	{ "Dispose",	StageTCXObj::m_dispose,		0,	0,	400 },	// D4
	{ "GetHandle",	StageTCXObj::m_gethandle,	0,	0,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void StageTCXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		StageTCXObject::initMethods(xlibMethods);
		StageTCXObject *xobj = new StageTCXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void StageTCXObj::close(ObjectType type) {
	if (type == kXObj) {
		StageTCXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


StageTCXObject::StageTCXObject(ObjectType ObjectType) :Object<StageTCXObject>("StageTC") {
	_objType = ObjectType;
}

void StageTCXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("StageTCXObj::m_new", nargs);
	g_lingo->push(g_lingo->_state->me);
}

void StageTCXObj::m_dispose(int nargs) {
	g_lingo->printSTUBWithArglist("StageTCXObj::m_dispose", nargs);
	g_lingo->dropStack(nargs);
}

void StageTCXObj::m_gethandle(int nargs) {
	g_lingo->printSTUBWithArglist("StageTCXObj::m_gethandle", nargs);
	g_lingo->dropStack(nargs);
}

} // End of namespace Director
