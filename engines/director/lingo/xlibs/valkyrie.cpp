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
 * Walküre no Densetsu Gaiden: Rosa no Bōken (ワルキューレの伝説 ローザの冒険)
 *
 *************************************/
/* -- Valkyrie External Factory. 16Feb93 PTM
 * Valkyrie
 * I      mNew                --Creates a new instance of the XObject
 * X      mDispose            --Disposes of XObject instance
 * S      mName               --Returns the XObject name (Valkyrie)
 * I      mStatus             --Returns an integer status code
 * SI     mError, code        --Returns an error string
 * S      mLastError          --Returns last error string
 * IS     mSave, str          --Saving SaveData to namco.ini.
 * S      mLoad               --Loading SaveData from namco.ini.
*/

#include "common/formats/ini-file.h"
#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/valkyrie.h"


namespace Director {

const char *const ValkyrieXObj::xlibName = "Valkyrie";
const XlibFileDesc ValkyrieXObj::fileNames[] = {
	{ "VALKYRIE",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "New",					ValkyrieXObj::m_new,			0,	0,	400 },	// D4
	{ "Dispose",				ValkyrieXObj::m_dispose,		0,	0,	400 },	// D4
	{ "Name",					ValkyrieXObj::m_name,			0,	0,	400 },	// D4
	{ "Status",					ValkyrieXObj::m_status,			0,	0,	400 },	// D4
	{ "Error",					ValkyrieXObj::m_error,			1,	1,	400 },	// D4
	{ "LastError",				ValkyrieXObj::m_lastError,		0,	0,	400 },	// D4
	{ "Save",					ValkyrieXObj::m_save,			0,	0,	400 },	// D4
	{ "Load",					ValkyrieXObj::m_load,			0,	0,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void ValkyrieXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		ValkyrieXObject::initMethods(xlibMethods);
		ValkyrieXObject *xobj = new ValkyrieXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void ValkyrieXObj::close(ObjectType type) {
	if (type == kXObj) {
		ValkyrieXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


ValkyrieXObject::ValkyrieXObject(ObjectType ObjectType) :Object<ValkyrieXObject>("Valkyrie") {
	_objType = ObjectType;
}

void ValkyrieXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(ValkyrieXObj::m_dispose)

void ValkyrieXObj::m_name(int nargs) {
	g_lingo->push(Datum("Valkyrie"));
}

XOBJSTUB(ValkyrieXObj::m_status, 0)

void ValkyrieXObj::m_error(int nargs) {
	// TODO: Save error code for m_lastError?
	int errorCode = g_lingo->pop().asInt();
	warning("ValkyrieXObj::m_error: Got error %d", errorCode);
}

XOBJSTUB(ValkyrieXObj::m_lastError, "")

void ValkyrieXObj::m_save(int nargs) {
	// should write to namco.ini > Valkyrie > Data
	// TODO: Should report errors if we fail to save
	Common::String saveName = savePrefix() + "namco.ini.txt";
	Common::String saveString = g_lingo->pop().asString();
	Common::INIFile *saveFile = new Common::INIFile();
	saveFile->loadFromSaveFile(saveName);
	saveFile->setKey("Data", "Valkyrie", saveString);
	saveFile->saveToSaveFile(saveName);
	delete saveFile;
	g_lingo->push(Datum(1));
}

void ValkyrieXObj::m_load(int nargs) {
	// should load save from namco.ini > Valkyrie > Data
	// TODO: Report errors if we fail to load?
	Common::String saveString;
	Common::INIFile *saveFile = new Common::INIFile();
	saveFile->loadFromSaveFile(savePrefix() + "namco.ini.txt");
	if (!(saveFile->hasKey("Data", "Valkyrie"))) {
		saveString = "0NAX";
	} else {
		saveFile->getKey("Data", "Valkyrie", saveString);
	}
	delete saveFile;
	g_lingo->push(Datum(saveString));
}

} // End of namespace Director
