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
#include "director/lingo/xtras/a/atlas-map.h"

/**************************************************
 *
 * USED IN:
 * Atlas der Schweiz \ Atlas of Switzerland 
 * Atlas der Schweiz \ Atlas of Switzerland 2
 *
 **************************************************/

/*
-- xtra Map -- Map Xtra is the most important extension for the creation of all kind of interactive two-dimensional maps. It is mainly based on the versatile multimedia map file format.
-- http://www.atlasofswitzerland.ch/
-- http://www.swisstopo.ch/en/products/digital/multimedia/ads2/
new object me
XInitMapEnv object me, string filename -- add a map environment by description file name
XAdSSpriteInit object me, integer number, object sprite, object mapRect -- set an AdS sprite
XAdSSpriteVisibility object me, integer number, integer visibility -- set visibility
XQueryNameList object me, string identifier, integer index -- query or create a name list
XGetNameList object me, string identifier, integer startIndex, integer endIndex -- get a name list excerpt
XGetNameListPosition object me, string identifier, string text -- search a text in a name list
XQuery object me, object point -- Query map
XMouseDown object me, object point -- Mouse down event, return if map has been scrolled
XUpdate * -- me: Update map and legend
XUpdateLock object me, integer inLock -- lock/unlock the map update
XRedrawLock object me, integer inLock -- lock/unlock the map redraw
XIdle object me -- Map idler
XSetMapArea object me, integer West, integer South, integer East, integer North -- set the map area
XSetMapScale object me, string scaleStr -- set the map scale
XSetNavigationMode object me, integer mode -- set the navigation mode
XBaseMapVis object me, string group, integer visibilityState
XThematicLayerBlend object me, integer inMapID, integer inBlend -- set the main/link thematic layer blend
XSetActiveLayer object me, integer inMapID -- set the active map
XSpaceAndTimeMenuCall object me, string commandString, integer updateLayer -- load map
XMarkMapItem object me, integer ID -- mark a map by id
XGetColorByID object me, integer ID -- get an item's color
XValuePlusCommand object me, integer command -- submit a value+ command
XAddOverlapRect object me, integer inID, object inRect -- add an overlapping rectangle to clipping
XRemoveOverlapRect object me, integer inID -- remove an overlapping rectangle from clipping
XPinVisibility object me, integer inVis -- pins are visible (1) or not (0)
XPin object me, integer inIndex, point inCoord -- set/select/remove a pin
XKeyDown object me, integer keyChar, integer keyCode, integer modifier -- handle key event
XAnalyzerCommand object me, integer inCommand -- execute an analyzer command
XAnalyzerChangeColor object me, integer inRed, integer inGreen, integer inBlue -- change selected analyzer colors
XThemeMenuLoad object me, string filename -- load the theme menu 2.0
XThemeMenuUnload object me -- delete the menu
XThemeMenuCall object me, string commandString -- get menu items as string/load map
XThemeMenuCall_2 object me, integer main, integer depth -- get menu items as string/load map
XThemeMenuCurrent object me, integer main, integer level -- get current menu number (one-based)
XThemeMenuLastResult object me -- get the last theme menu call result
XThemeMenuGetIndex object me, string filename -- get a menu index by file name
XTextLoad object me, integer id, string fileName -- load XML file
XText object me, integer id, string id -- get text by id
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

const char *MapXtra::xlibName = "Map";
const XlibFileDesc MapXtra::fileNames[] = {
	{ "map",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				MapXtra::m_new,		 0, 0,	600 },
	{ "XInitMapEnv",				MapXtra::m_XInitMapEnv,		 1, 1,	600 },
	{ "XAdSSpriteInit",				MapXtra::m_XAdSSpriteInit,		 3, 3,	600 },
	{ "XAdSSpriteVisibility",				MapXtra::m_XAdSSpriteVisibility,		 2, 2,	600 },
	{ "XQueryNameList",				MapXtra::m_XQueryNameList,		 2, 2,	600 },
	{ "XGetNameList",				MapXtra::m_XGetNameList,		 3, 3,	600 },
	{ "XGetNameListPosition",				MapXtra::m_XGetNameListPosition,		 2, 2,	600 },
	{ "XQuery",				MapXtra::m_XQuery,		 1, 1,	600 },
	{ "XMouseDown",				MapXtra::m_XMouseDown,		 1, 1,	600 },
	{ "XUpdate",				MapXtra::m_XUpdate,		 -1, 0,	600 },
	{ "XUpdateLock",				MapXtra::m_XUpdateLock,		 1, 1,	600 },
	{ "XRedrawLock",				MapXtra::m_XRedrawLock,		 1, 1,	600 },
	{ "XIdle",				MapXtra::m_XIdle,		 0, 0,	600 },
	{ "XSetMapArea",				MapXtra::m_XSetMapArea,		 4, 4,	600 },
	{ "XSetMapScale",				MapXtra::m_XSetMapScale,		 1, 1,	600 },
	{ "XSetNavigationMode",				MapXtra::m_XSetNavigationMode,		 1, 1,	600 },
	{ "XBaseMapVis",				MapXtra::m_XBaseMapVis,		 2, 2,	600 },
	{ "XThematicLayerBlend",				MapXtra::m_XThematicLayerBlend,		 2, 2,	600 },
	{ "XSetActiveLayer",				MapXtra::m_XSetActiveLayer,		 1, 1,	600 },
	{ "XSpaceAndTimeMenuCall",				MapXtra::m_XSpaceAndTimeMenuCall,		 2, 2,	600 },
	{ "XMarkMapItem",				MapXtra::m_XMarkMapItem,		 1, 1,	600 },
	{ "XGetColorByID",				MapXtra::m_XGetColorByID,		 1, 1,	600 },
	{ "XValuePlusCommand",				MapXtra::m_XValuePlusCommand,		 1, 1,	600 },
	{ "XAddOverlapRect",				MapXtra::m_XAddOverlapRect,		 2, 2,	600 },
	{ "XRemoveOverlapRect",				MapXtra::m_XRemoveOverlapRect,		 1, 1,	600 },
	{ "XPinVisibility",				MapXtra::m_XPinVisibility,		 1, 1,	600 },
	{ "XPin",				MapXtra::m_XPin,		 2, 2,	600 },
	{ "XKeyDown",				MapXtra::m_XKeyDown,		 3, 3,	600 },
	{ "XAnalyzerCommand",				MapXtra::m_XAnalyzerCommand,		 1, 1,	600 },
	{ "XAnalyzerChangeColor",				MapXtra::m_XAnalyzerChangeColor,		 3, 3,	600 },
	{ "XThemeMenuLoad",				MapXtra::m_XThemeMenuLoad,		 1, 1,	600 },
	{ "XThemeMenuUnload",				MapXtra::m_XThemeMenuUnload,		 0, 0,	600 },
	{ "XThemeMenuCall",				MapXtra::m_XThemeMenuCall,		 1, 1,	600 },
	{ "XThemeMenuCall_2",				MapXtra::m_XThemeMenuCall_2,		 2, 2,	600 },
	{ "XThemeMenuCurrent",				MapXtra::m_XThemeMenuCurrent,		 2, 2,	600 },
	{ "XThemeMenuLastResult",				MapXtra::m_XThemeMenuLastResult,		 0, 0,	600 },
	{ "XThemeMenuGetIndex",				MapXtra::m_XThemeMenuGetIndex,		 1, 1,	600 },
	{ "XTextLoad",				MapXtra::m_XTextLoad,		 2, 2,	600 },
	{ "XText",				MapXtra::m_XText,		 2, 2,	600 },
	{ "XHistory",				MapXtra::m_XHistory,		 1, 1,	600 },
	{ "XImportXML",				MapXtra::m_XImportXML,		 0, 0,	600 },
	{ "XExportXML",				MapXtra::m_XExportXML,		 0, 0,	600 },
	{ "XPrint",				MapXtra::m_XPrint,		 0, 0,	600 },
	{ "XExportImage",				MapXtra::m_XExportImage,		 2, 2,	600 },
	{ "XSetPreferences",				MapXtra::m_XSetPreferences,		 2, 2,	600 },
	{ "XPreferences",				MapXtra::m_XPreferences,		 1, 1,	600 },
	{ """,				MapXtra::m_",		 -1, -1,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MapXtraObject::MapXtraObject(ObjectType ObjectType) :Object<MapXtraObject>("Map") {
	_objType = ObjectType;
}

bool MapXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum MapXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(MapXtra::xlibName);
	warning("MapXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void MapXtra::open(ObjectType type, const Common::Path &path) {
    MapXtraObject::initMethods(xlibMethods);
    MapXtraObject *xobj = new MapXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MapXtra::close(ObjectType type) {
    MapXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MapXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MapXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(MapXtra::m_XInitMapEnv, 0)
XOBJSTUB(MapXtra::m_XAdSSpriteInit, 0)
XOBJSTUB(MapXtra::m_XAdSSpriteVisibility, 0)
XOBJSTUB(MapXtra::m_XQueryNameList, 0)
XOBJSTUB(MapXtra::m_XGetNameList, 0)
XOBJSTUB(MapXtra::m_XGetNameListPosition, 0)
XOBJSTUB(MapXtra::m_XQuery, 0)
XOBJSTUB(MapXtra::m_XMouseDown, 0)
XOBJSTUB(MapXtra::m_XUpdate, 0)
XOBJSTUB(MapXtra::m_XUpdateLock, 0)
XOBJSTUB(MapXtra::m_XRedrawLock, 0)
XOBJSTUB(MapXtra::m_XIdle, 0)
XOBJSTUB(MapXtra::m_XSetMapArea, 0)
XOBJSTUB(MapXtra::m_XSetMapScale, 0)
XOBJSTUB(MapXtra::m_XSetNavigationMode, 0)
XOBJSTUB(MapXtra::m_XBaseMapVis, 0)
XOBJSTUB(MapXtra::m_XThematicLayerBlend, 0)
XOBJSTUB(MapXtra::m_XSetActiveLayer, 0)
XOBJSTUB(MapXtra::m_XSpaceAndTimeMenuCall, 0)
XOBJSTUB(MapXtra::m_XMarkMapItem, 0)
XOBJSTUB(MapXtra::m_XGetColorByID, 0)
XOBJSTUB(MapXtra::m_XValuePlusCommand, 0)
XOBJSTUB(MapXtra::m_XAddOverlapRect, 0)
XOBJSTUB(MapXtra::m_XRemoveOverlapRect, 0)
XOBJSTUB(MapXtra::m_XPinVisibility, 0)
XOBJSTUB(MapXtra::m_XPin, 0)
XOBJSTUB(MapXtra::m_XKeyDown, 0)
XOBJSTUB(MapXtra::m_XAnalyzerCommand, 0)
XOBJSTUB(MapXtra::m_XAnalyzerChangeColor, 0)
XOBJSTUB(MapXtra::m_XThemeMenuLoad, 0)
XOBJSTUB(MapXtra::m_XThemeMenuUnload, 0)
XOBJSTUB(MapXtra::m_XThemeMenuCall, 0)
XOBJSTUB(MapXtra::m_XThemeMenuCall_2, 0)
XOBJSTUB(MapXtra::m_XThemeMenuCurrent, 0)
XOBJSTUB(MapXtra::m_XThemeMenuLastResult, 0)
XOBJSTUB(MapXtra::m_XThemeMenuGetIndex, 0)
XOBJSTUB(MapXtra::m_XTextLoad, 0)
XOBJSTUB(MapXtra::m_XText, 0)
XOBJSTUB(MapXtra::m_XHistory, 0)
XOBJSTUB(MapXtra::m_XImportXML, 0)
XOBJSTUB(MapXtra::m_XExportXML, 0)
XOBJSTUB(MapXtra::m_XPrint, 0)
XOBJSTUB(MapXtra::m_XExportImage, 0)
XOBJSTUB(MapXtra::m_XSetPreferences, 0)
XOBJSTUB(MapXtra::m_XPreferences, 0)
XOBJSTUB(MapXtra::m_", 0)

}
