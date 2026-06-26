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
#include "director/lingo/xtras/x/xmlparser.h"

/**************************************************
 *
 * USED IN:
 * TKKG 6-9
 *
 **************************************************/

/*
-- xtra XmlParser
new object me
-- XmlParser Xtra --
parseString object me, string data -- parses the buffer
parseURL object me, string url, * -- parses the specified url
doneParsing object me -- returns true if it's done parsing a URL
ignoreWhitespace object me, int ignore -- toggles whether to ignore whitespace
getError object me -- returns the error string (if any) generated when parsing
makeList object me -- make a lingo list based on the XML document
 */

namespace Director {

const char *XMLParserXtra::xlibName = "XMLParser";
const XlibFileDesc XMLParserXtra::fileNames[] = {
	{ "xmlparser",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				XMLParserXtra::m_new,		 0, 0,	500 },
	{ "parseString",				XMLParserXtra::m_parseString,		 1, 1,	500 },
	{ "parseURL",				XMLParserXtra::m_parseURL,		 -1, 0,	500 },
	{ "doneParsing",				XMLParserXtra::m_doneParsing,		 0, 0,	500 },
	{ "ignoreWhitespace",				XMLParserXtra::m_ignoreWhitespace,		 1, 1,	500 },
	{ "getError",				XMLParserXtra::m_getError,		 0, 0,	500 },
	{ "makeList",				XMLParserXtra::m_makeList,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

XMLParserXtraObject::XMLParserXtraObject(ObjectType ObjectType) :Object<XMLParserXtraObject>("XMLParser") {
	_objType = ObjectType;
}

bool XMLParserXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum XMLParserXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(XMLParserXtra::xlibName);
	warning("XMLParserXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void XMLParserXtra::open(ObjectType type, const Common::Path &path) {
    XMLParserXtraObject::initMethods(xlibMethods);
    XMLParserXtraObject *xobj = new XMLParserXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void XMLParserXtra::close(ObjectType type) {
    XMLParserXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void XMLParserXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("XMLParserXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(XMLParserXtra::m_parseString, 0)
XOBJSTUB(XMLParserXtra::m_parseURL, 0)
XOBJSTUB(XMLParserXtra::m_doneParsing, 0)
XOBJSTUB(XMLParserXtra::m_ignoreWhitespace, 0)
XOBJSTUB(XMLParserXtra::m_getError, 0)
XOBJSTUB(XMLParserXtra::m_makeList, 0)

}
