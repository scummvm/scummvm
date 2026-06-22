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
#include "director/lingo/xtras/a/atlas-topo3d.h"

/**************************************************
 *
 * USED IN:
 * Atlas der Schweiz \ Atlas of Switzerland 
 * Atlas der Schweiz \ Atlas of Switzerland 2
 *
 **************************************************/

/*
-- xtra Topo3D -- Topo Xtra is a visualisation extension for the display of three-dimensional topographic models.
-- http://www.atlasofswitzerland.ch/
-- http://www.swisstopo.ch/en/products/digital/multimedia/ads2/
new object me
XAdSSpriteInit object me, integer number, object sprite, object mapRect -- set an AdS sprite
XMouseWithin object me, integer id, object point -- handle mouse within events
XMouseLeave object me, integer id -- handle mouse leave events
XMouseDown object me, object point -- Mouse pushed on topo3d
XKeyDown object me, integer keyChar, integer keyCode, integer modifier -- handle key event
XUpdate object me, integer spriteChannelindex, object rect -- Update event by AdSSprite
XAddOverlapRect object me, integer inID, object inRect -- add an overlapping rectangle to clipping
XRemoveOverlapRect object me, integer inID -- remove an overlapping rectangle from clipping
XIdle object me -- topo3D idler
XSetValueF object me, integer type, float value -- set camera parameter
XSetValueS object me, integer type, string value -- set camera parameter
XResetTopo3D object me -- deletes topo3d instance
XDraw object me -- draw topo3D
XSetInteractMode object me, integer interactMode -- how to handle user interaction
XReadTopoDescXML object me, string filename, string fileNameDisplayText -- load topo3d settings from xml file
XSetTopoOffset object me, integer offsetH, integer offsetV -- vertical and horicontal offset settings
XSetColoringState object me, integer coloringID, integer on -- TRUE enable
XColoringLoadModules object me, string fileName -- load coloringmodules from file
XSetColoringVisPart object me, float visPart, integer coloringIdentifier -- set the coloringmodul visibility part
XColoringVisPart object me, integer coloringIdentifier -- returns the coloringmodul visibility part
XColoringRemoveModules object me -- tells topo3d to delete all coloringmodules
XSetColoringValue object me,integer valueType,float value1, float value2, inAlpha, inRed, inGreen, inBlue -- set a coloring attr.
XSetColoringReLoadModul object me, string fileName, integer inModulID -- reload a single modul
XColoringDialog object me, integer coloringID, integer onOff, object inDialogRect -- open coloring settings display
XColoringDialogCommand object me, integer inCommand -- execute an analyzer command
XColoringAnalyzerChangeColor object me,integer inAlpha, integer inRed, integer inGreen, integer inBlue -- change selected analyzer colors
XColoringValiDate object me,integer inDay,integer inMonth,integer inYear,integer inHour,integer inCorrDir -- validates date
XUpdatePreviewImage object me -- updates previewImage
XPreview object me, integer onOff -- create/destroy preview
XRefMap object me, integer whichOne , integer onOff, string fileName -- create/destroy reference map
XProfileDraw object me, integer where, integer superElevationType -- draw a profile
XProfile object me, integer onOff -- start/stop profile
XTextLoad object me, integer id, string fileName -- load XML file
XText object me, integer id, string id -- get text by id
XListLoad object me, integer listID, string fileName -- load XML file
XListCount object me, integer listID -- get number of entries
XListPosition object me, integer listID -- get first returned entry position
XListCommand object me, integer listID, integer position -- do command
XListEntriesByPosition object me, integer listID, integer startPos, integer numOfItems -- get sublist by start position
XListEntriesBySearchString object me, integer listID, string searchString, integer numOfItems -- get sublist by search string
XListThemesPositionByID object me, integer inID -- get get position by id (T2)
XHistory object me, integer direction -- history step back/forward
XImportXML object me -- display the load xml map dialog
XExportXML object me  -- display the save xml map dialog
XPrint object me -- display the print dialog
XExportImage object me, integer format, integer size -- export the image with the given parameters
XSetPreferences object me, integer type, integer value -- set prefs value
XPreferences object me, integer type -- get prefs value
"
 */

namespace Director {

const char *Topo3dXtra::xlibName = "Topo3d";
const XlibFileDesc Topo3dXtra::fileNames[] = {
	{ "topo3d",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				Topo3dXtra::m_new,		 0, 0,	600 },
	{ "XAdSSpriteInit",				Topo3dXtra::m_XAdSSpriteInit,		 3, 3,	600 },
	{ "XMouseWithin",				Topo3dXtra::m_XMouseWithin,		 2, 2,	600 },
	{ "XMouseLeave",				Topo3dXtra::m_XMouseLeave,		 1, 1,	600 },
	{ "XMouseDown",				Topo3dXtra::m_XMouseDown,		 1, 1,	600 },
	{ "XKeyDown",				Topo3dXtra::m_XKeyDown,		 3, 3,	600 },
	{ "XUpdate",				Topo3dXtra::m_XUpdate,		 2, 2,	600 },
	{ "XAddOverlapRect",				Topo3dXtra::m_XAddOverlapRect,		 2, 2,	600 },
	{ "XRemoveOverlapRect",				Topo3dXtra::m_XRemoveOverlapRect,		 1, 1,	600 },
	{ "XIdle",				Topo3dXtra::m_XIdle,		 0, 0,	600 },
	{ "XSetValueF",				Topo3dXtra::m_XSetValueF,		 2, 2,	600 },
	{ "XSetValueS",				Topo3dXtra::m_XSetValueS,		 2, 2,	600 },
	{ "XResetTopo3D",				Topo3dXtra::m_XResetTopo3D,		 0, 0,	600 },
	{ "XDraw",				Topo3dXtra::m_XDraw,		 0, 0,	600 },
	{ "XSetInteractMode",				Topo3dXtra::m_XSetInteractMode,		 1, 1,	600 },
	{ "XReadTopoDescXML",				Topo3dXtra::m_XReadTopoDescXML,		 2, 2,	600 },
	{ "XSetTopoOffset",				Topo3dXtra::m_XSetTopoOffset,		 2, 2,	600 },
	{ "XSetColoringState",				Topo3dXtra::m_XSetColoringState,		 2, 2,	600 },
	{ "XColoringLoadModules",				Topo3dXtra::m_XColoringLoadModules,		 1, 1,	600 },
	{ "XSetColoringVisPart",				Topo3dXtra::m_XSetColoringVisPart,		 2, 2,	600 },
	{ "XColoringVisPart",				Topo3dXtra::m_XColoringVisPart,		 1, 1,	600 },
	{ "XColoringRemoveModules",				Topo3dXtra::m_XColoringRemoveModules,		 0, 0,	600 },
	{ "XSetColoringValue",				Topo3dXtra::m_XSetColoringValue,		 7, 7,	600 },
	{ "XSetColoringReLoadModul",				Topo3dXtra::m_XSetColoringReLoadModul,		 2, 2,	600 },
	{ "XColoringDialog",				Topo3dXtra::m_XColoringDialog,		 3, 3,	600 },
	{ "XColoringDialogCommand",				Topo3dXtra::m_XColoringDialogCommand,		 1, 1,	600 },
	{ "XColoringAnalyzerChangeColor",				Topo3dXtra::m_XColoringAnalyzerChangeColor,		 4, 4,	600 },
	{ "XColoringValiDate",				Topo3dXtra::m_XColoringValiDate,		 5, 5,	600 },
	{ "XUpdatePreviewImage",				Topo3dXtra::m_XUpdatePreviewImage,		 0, 0,	600 },
	{ "XPreview",				Topo3dXtra::m_XPreview,		 1, 1,	600 },
	{ "XRefMap",				Topo3dXtra::m_XRefMap,		 3, 3,	600 },
	{ "XProfileDraw",				Topo3dXtra::m_XProfileDraw,		 2, 2,	600 },
	{ "XProfile",				Topo3dXtra::m_XProfile,		 1, 1,	600 },
	{ "XTextLoad",				Topo3dXtra::m_XTextLoad,		 2, 2,	600 },
	{ "XText",				Topo3dXtra::m_XText,		 2, 2,	600 },
	{ "XListLoad",				Topo3dXtra::m_XListLoad,		 2, 2,	600 },
	{ "XListCount",				Topo3dXtra::m_XListCount,		 1, 1,	600 },
	{ "XListPosition",				Topo3dXtra::m_XListPosition,		 1, 1,	600 },
	{ "XListCommand",				Topo3dXtra::m_XListCommand,		 2, 2,	600 },
	{ "XListEntriesByPosition",				Topo3dXtra::m_XListEntriesByPosition,		 3, 3,	600 },
	{ "XListEntriesBySearchString",				Topo3dXtra::m_XListEntriesBySearchString,		 3, 3,	600 },
	{ "XListThemesPositionByID",				Topo3dXtra::m_XListThemesPositionByID,		 1, 1,	600 },
	{ "XHistory",				Topo3dXtra::m_XHistory,		 1, 1,	600 },
	{ "XImportXML",				Topo3dXtra::m_XImportXML,		 0, 0,	600 },
	{ "XExportXML",				Topo3dXtra::m_XExportXML,		 0, 0,	600 },
	{ "XPrint",				Topo3dXtra::m_XPrint,		 0, 0,	600 },
	{ "XExportImage",				Topo3dXtra::m_XExportImage,		 2, 2,	600 },
	{ "XSetPreferences",				Topo3dXtra::m_XSetPreferences,		 2, 2,	600 },
	{ "XPreferences",				Topo3dXtra::m_XPreferences,		 1, 1,	600 },
	{ """,				Topo3dXtra::m_",		 -1, -1,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

Topo3dXtraObject::Topo3dXtraObject(ObjectType ObjectType) :Object<Topo3dXtraObject>("Topo3d") {
	_objType = ObjectType;
}

bool Topo3dXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum Topo3dXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(Topo3dXtra::xlibName);
	warning("Topo3dXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void Topo3dXtra::open(ObjectType type, const Common::Path &path) {
    Topo3dXtraObject::initMethods(xlibMethods);
    Topo3dXtraObject *xobj = new Topo3dXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void Topo3dXtra::close(ObjectType type) {
    Topo3dXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void Topo3dXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("Topo3dXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(Topo3dXtra::m_XAdSSpriteInit, 0)
XOBJSTUB(Topo3dXtra::m_XMouseWithin, 0)
XOBJSTUB(Topo3dXtra::m_XMouseLeave, 0)
XOBJSTUB(Topo3dXtra::m_XMouseDown, 0)
XOBJSTUB(Topo3dXtra::m_XKeyDown, 0)
XOBJSTUB(Topo3dXtra::m_XUpdate, 0)
XOBJSTUB(Topo3dXtra::m_XAddOverlapRect, 0)
XOBJSTUB(Topo3dXtra::m_XRemoveOverlapRect, 0)
XOBJSTUB(Topo3dXtra::m_XIdle, 0)
XOBJSTUB(Topo3dXtra::m_XSetValueF, 0)
XOBJSTUB(Topo3dXtra::m_XSetValueS, 0)
XOBJSTUB(Topo3dXtra::m_XResetTopo3D, 0)
XOBJSTUB(Topo3dXtra::m_XDraw, 0)
XOBJSTUB(Topo3dXtra::m_XSetInteractMode, 0)
XOBJSTUB(Topo3dXtra::m_XReadTopoDescXML, 0)
XOBJSTUB(Topo3dXtra::m_XSetTopoOffset, 0)
XOBJSTUB(Topo3dXtra::m_XSetColoringState, 0)
XOBJSTUB(Topo3dXtra::m_XColoringLoadModules, 0)
XOBJSTUB(Topo3dXtra::m_XSetColoringVisPart, 0)
XOBJSTUB(Topo3dXtra::m_XColoringVisPart, 0)
XOBJSTUB(Topo3dXtra::m_XColoringRemoveModules, 0)
XOBJSTUB(Topo3dXtra::m_XSetColoringValue, 0)
XOBJSTUB(Topo3dXtra::m_XSetColoringReLoadModul, 0)
XOBJSTUB(Topo3dXtra::m_XColoringDialog, 0)
XOBJSTUB(Topo3dXtra::m_XColoringDialogCommand, 0)
XOBJSTUB(Topo3dXtra::m_XColoringAnalyzerChangeColor, 0)
XOBJSTUB(Topo3dXtra::m_XColoringValiDate, 0)
XOBJSTUB(Topo3dXtra::m_XUpdatePreviewImage, 0)
XOBJSTUB(Topo3dXtra::m_XPreview, 0)
XOBJSTUB(Topo3dXtra::m_XRefMap, 0)
XOBJSTUB(Topo3dXtra::m_XProfileDraw, 0)
XOBJSTUB(Topo3dXtra::m_XProfile, 0)
XOBJSTUB(Topo3dXtra::m_XTextLoad, 0)
XOBJSTUB(Topo3dXtra::m_XText, 0)
XOBJSTUB(Topo3dXtra::m_XListLoad, 0)
XOBJSTUB(Topo3dXtra::m_XListCount, 0)
XOBJSTUB(Topo3dXtra::m_XListPosition, 0)
XOBJSTUB(Topo3dXtra::m_XListCommand, 0)
XOBJSTUB(Topo3dXtra::m_XListEntriesByPosition, 0)
XOBJSTUB(Topo3dXtra::m_XListEntriesBySearchString, 0)
XOBJSTUB(Topo3dXtra::m_XListThemesPositionByID, 0)
XOBJSTUB(Topo3dXtra::m_XHistory, 0)
XOBJSTUB(Topo3dXtra::m_XImportXML, 0)
XOBJSTUB(Topo3dXtra::m_XExportXML, 0)
XOBJSTUB(Topo3dXtra::m_XPrint, 0)
XOBJSTUB(Topo3dXtra::m_XExportImage, 0)
XOBJSTUB(Topo3dXtra::m_XSetPreferences, 0)
XOBJSTUB(Topo3dXtra::m_XPreferences, 0)
XOBJSTUB(Topo3dXtra::m_", 0)

}
