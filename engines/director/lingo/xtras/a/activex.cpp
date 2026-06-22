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

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/a/activex.h"

/**************************************************
 *
 * USED IN:
 * Standard Director Xtra
 *
 **************************************************/

/*
-- xtra ActiveX
new object me
-- ActiveX Xtra --
* ActiveXInstalled -- Returns -1 if ActiveX support is installed, otherwise 0.
* ActiveXSecurityDialog -- Displays a dialog box for the user to set ActiveX security settings.
* ActiveXSecuritySetting -- Returns ActiveX security setting: 'High', 'Medium', or 'None'.
* ActiveXDownloadSetting -- Returns ActiveX download setting: 'Enabled' or 'Disabled'.
* ActiveXControlQuery string CLASSID -- Returns -1 if the ActiveX control is installed, otherwise 0.
* ActiveXControlDownload string CLASSID, string URL, integer ver1, integer ver2, integer ver3, integer ver4 -- Downloads a particular version of the specified ActiveX control from a URL. Returns -1 if the download succeeded, otherwise 0.
* ActiveXControlRegister string FILENAME -- Locates the specified control file on the application path and calls the DllRegisterServer entry point.  Typically, this will install an ActiveX control that is self registering.  Returns -1 if the install succeeded, otherwise 0.
* ActiveXControlUnregister string FILENAME -- Locates the specified control file on the application path and calls the DllUnregisterServer entry point.  Typically, this will uninstall an ActiveX control that is self registering.  Returns -1 if the install succeeded, otherwise 0."
 */

namespace Director {

const char *ActivexXtra::xlibName = "Activex";
const XlibFileDesc ActivexXtra::fileNames[] = {
	{ "activex",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				ActivexXtra::m_new,		 0, 0,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "ActiveXInstalled", ActivexXtra::m_ActiveXInstalled, 0, 0, 600, HBLTIN },
	{ "ActiveXSecurityDialog", ActivexXtra::m_ActiveXSecurityDialog, 0, 0, 600, HBLTIN },
	{ "ActiveXSecuritySetting", ActivexXtra::m_ActiveXSecuritySetting, 0, 0, 600, HBLTIN },
	{ "ActiveXDownloadSetting", ActivexXtra::m_ActiveXDownloadSetting, 0, 0, 600, HBLTIN },
	{ "ActiveXControlQuery", ActivexXtra::m_ActiveXControlQuery, 1, 1, 600, HBLTIN },
	{ "ActiveXControlDownload", ActivexXtra::m_ActiveXControlDownload, 6, 6, 600, HBLTIN },
	{ "ActiveXControlRegister", ActivexXtra::m_ActiveXControlRegister, 1, 1, 600, HBLTIN },
	{ "ActiveXControlUnregister", ActivexXtra::m_ActiveXControlUnregister, 1, 1, 600, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ActivexXtraObject::ActivexXtraObject(ObjectType ObjectType) :Object<ActivexXtraObject>("Activex") {
	_objType = ObjectType;
}

bool ActivexXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum ActivexXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(ActivexXtra::xlibName);
	warning("ActivexXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void ActivexXtra::open(ObjectType type, const Common::Path &path) {
    ActivexXtraObject::initMethods(xlibMethods);
    ActivexXtraObject *xobj = new ActivexXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ActivexXtra::close(ObjectType type) {
    ActivexXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void ActivexXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ActivexXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ActivexXtra::m_ActiveXInstalled, 0)
XOBJSTUB(ActivexXtra::m_ActiveXSecurityDialog, 0)
XOBJSTUB(ActivexXtra::m_ActiveXSecuritySetting, 0)
XOBJSTUB(ActivexXtra::m_ActiveXDownloadSetting, 0)
XOBJSTUB(ActivexXtra::m_ActiveXControlQuery, 0)
XOBJSTUB(ActivexXtra::m_ActiveXControlDownload, 0)
XOBJSTUB(ActivexXtra::m_ActiveXControlRegister, 0)
XOBJSTUB(ActivexXtra::m_ActiveXControlUnregister, 0)

}
