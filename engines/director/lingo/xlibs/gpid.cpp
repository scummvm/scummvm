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
 * Gahan Wilson's Ultimate Haunted House
 *
 *************************************/

/*
 * Product Identification Number
 * Gahan Wilson's Ultimate Haunted House
 * Copyright 1994 Byron Preiss Multimedia Company, Inc.
 * 'Copyright 1994, Byron Preiss Multimedia
 * -- copyright 1994 by Byron Priess Multimedia, authored by MayoSmith
 * gpid
 * I      mNew                        --Read Docs to avoid Hard Drive failure
 * X      mDispose                    --Disposes of XObject instance
 * S      mName                       --Returns the XObject name (Widget)
 * I      mStatus                     --Returns an integer status code
 * SI     mError, code                --Returns an error string
 * S      mLastError                  --Returns last error string
 * I      mGetPid                     --Retrieves PID
 *
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/gpid.h"


namespace Director {

const char *GpidXObj::xlibName = "gpid";
const char *GpidXObj::fileNames[] = {
	"GPID",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",   GpidXObj::m_new,					0,	0,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void GpidXObj::open(int type) {
	if (type == kXObj) {
		ProductIdXObject::initMethods(xlibMethods);
		ProductIdXObject *xobj = new ProductIdXObject(kXObj);
		g_lingo->_globalvars[xlibName] = xobj;
	}
}

void GpidXObj::close(int type) {
	if (type == kXObj) {
		ProductIdXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


ProductIdXObject::ProductIdXObject(ObjectType ObjectType) :Object<ProductIdXObject>("GpidXObj") {
	_objType = ObjectType;
}

void GpidXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("gpid::new", nargs);
	g_lingo->push(g_lingo->_currentMe);
}

} // End of namespace Director
