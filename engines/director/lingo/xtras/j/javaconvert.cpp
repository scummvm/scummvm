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
#include "director/lingo/xtras/j/javaconvert.h"

/**************************************************
 *
 * USED IN:
 * TKKG 6-9
 *
 **************************************************/

/*
-- xtra JavaConvert -- This Xtra provides java output of Director movies 
new object me
ConfigureTranslation object me, string MovieToTranslateName, integer bScoreGoesInMediaFile, integer bAllMediaInMediaFile, integer bConvertScripts, integer iJpegQual, string sImageCompression, integer iSingleGifPalette, integer bListsContainIntsOnly, integer bLoopTheMovie, integer optimizationSettings  -- Sets options for translation.
GetVersion  object me  -- returns a version number for this Xtra
CheckMovie  object me -- checks movie for errors.  Returns list of errors
TranslateMovie  object me,  string SourceFileName, string MediaFileName  -- Translates movie to Source and Media files.  Returns list of errors
TranslateScript  object me, any scriptmember, string outputFileName  -- Translate a given script, Returns string translation 
ExportMedia  object me, string MovieToTranslateName, any member, string outputFileName, integer OutputAsJpeg, integer QualityPercentage, integer castlibNum -- Translate a given media castmember (only image hooked up), Returns 0 
CastMemForLineNumber object me, integer lineNumber -- Get castmember index whose translation output into the source file would include the given line number
GetTranslatedMovieName object me -- Gets the name of the movie class.  This may be different than the movie name if it conflicts with some other exported symbol
 */

namespace Director {

const char *JavaConvertXtra::xlibName = "JavaConvert";
const XlibFileDesc JavaConvertXtra::fileNames[] = {
	{ "JavaCnvt",      nullptr },	// on-disk Xtra filename (JavaCnvt.x32)
	{ "javaconvert",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				JavaConvertXtra::m_new,		 0, 0,	500 },
	{ "ConfigureTranslation",				JavaConvertXtra::m_ConfigureTranslation,		 10, 10,	500 },
	{ "GetVersion",				JavaConvertXtra::m_GetVersion,		 0, 0,	500 },
	{ "CheckMovie",				JavaConvertXtra::m_CheckMovie,		 0, 0,	500 },
	{ "TranslateMovie",				JavaConvertXtra::m_TranslateMovie,		 2, 2,	500 },
	{ "TranslateScript",				JavaConvertXtra::m_TranslateScript,		 2, 2,	500 },
	{ "ExportMedia",				JavaConvertXtra::m_ExportMedia,		 6, 6,	500 },
	{ "CastMemForLineNumber",				JavaConvertXtra::m_CastMemForLineNumber,		 1, 1,	500 },
	{ "GetTranslatedMovieName",				JavaConvertXtra::m_GetTranslatedMovieName,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

JavaConvertXtraObject::JavaConvertXtraObject(ObjectType ObjectType) :Object<JavaConvertXtraObject>("JavaConvert") {
	_objType = ObjectType;
}

bool JavaConvertXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum JavaConvertXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(JavaConvertXtra::xlibName);
	warning("JavaConvertXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void JavaConvertXtra::open(ObjectType type, const Common::Path &path) {
    JavaConvertXtraObject::initMethods(xlibMethods);
    JavaConvertXtraObject *xobj = new JavaConvertXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void JavaConvertXtra::close(ObjectType type) {
    JavaConvertXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void JavaConvertXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("JavaConvertXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(JavaConvertXtra::m_ConfigureTranslation, 0)
XOBJSTUB(JavaConvertXtra::m_GetVersion, 0)
XOBJSTUB(JavaConvertXtra::m_CheckMovie, 0)
XOBJSTUB(JavaConvertXtra::m_TranslateMovie, 0)
XOBJSTUB(JavaConvertXtra::m_TranslateScript, 0)
XOBJSTUB(JavaConvertXtra::m_ExportMedia, 0)
XOBJSTUB(JavaConvertXtra::m_CastMemForLineNumber, 0)
XOBJSTUB(JavaConvertXtra::m_GetTranslatedMovieName, 0)

}
