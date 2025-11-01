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
#include "director/lingo/xtras/m/mui.h"

/**************************************************
 *
 * USED IN:
 * I Spy Spooky Mansion
 *
 **************************************************/

/*
-- xtra Mui
-- methods are Initialize, Run, WindowOperation, ItemUpdate --
----------------------------------------------------------------------
--  Data types (prop lists required formats)
--      #widgetTypes	-- call GetWidgetList on object for complete list
--          #checkBox , #radioButton, #editText, #editTextLarge, #labelNormal, etc
--      #attributeTypes	-- varies depending on the type of widget, contains 0 or more of the following
--          #textSize	  : [#large, #tiny, #normal(default)]
--          #textStyle   : [#bold, #italic, #underline, #plain (default), #inverse (v2)
--          #textAlign   : [#left, #right, #center (defaults to system language standard)
--          #popupStyle  : [#tiny, #cramped, #normal (default)
--          #valueList   : ['one', #two, 3, 4.0] (list of mmvalues, all coerced to strings)
--          #valueRange  : [#min:0.0, #max:1000.0, #increment:1.0, #jump:10.0, #acceleration:0.5]
--          #sliderStyle : [#ticks, #value]
--          #layoutStyle : [#lockPosition, #lockSize, #minimize, #centerH, #right, #left, #centerV, #top, #bottom ]
--      according to the following breakdown of supported widgets and appropriate attributes
--      #widgetTypes                         | vs  | UT | Attributes to use, UT	= Uses Title
--          #none,                           | 1.0 | No | <layoutStyle>
--          #dividerV,                       | 1.0 | No | <layoutStyle>
--          #dividerH,                       | 1.0 | No | <layoutStyle>
--          #bitmap,                         | 1.0 | No | <layoutStyle>
--          #checkBox,                       | 1.0 | Yes| <textSize><layoutStyle>
--          #radioButton,                    | 1.0 | Yes| <textSize><layoutStyle>
--          #popupList,                      | 1.0 | No | <popupStyle><valueList><layoutStyle>
--          #editText,                       | 1.0 | No | <textSize><justification><textStyle><layoutStyle>
--          #windowBegin,                    | 1.0 | No |
--          #windowEnd,                      | 1.0 | No |
--          #groupHBegin,                    | 1.0 | No |
--          #groupHEnd,                      | 1.0 | No |
--          #groupVBegin,                    | 1.0 | No |
--          #groupVEnd,                      | 1.0 | No |
--          #label,                          | 1.0 | No | <textSize><justification><textStyle><layoutStyle>
--          #IntegerSliderH,                 | 1.0 | No | <sliderStyle><valueRange><layoutStyle>
--          #floatSliderH,                   | 1.0 | No | <sliderStyle><valueRange><layoutStyle>
--          #defaultPushButton,              | 1.0 | Yes| <textSize><layoutStyle>
--          #cancelPushButton,               | 1.0 | Yes| <textSize><layoutStyle>
--          #pushButton,                     | 1.0 | Yes| <textSize><layoutStyle>
--          #toggleButton,                   | 1.0 | Yes| <textSize><layoutStyle>
--
--     alertList    =
--        [ #buttons    : <#Ok/#OkCancel/#AbortRetryIgnore/#YesNoCancel/#YesNo/#RetryCancel>,
--          #default    : 1, -- button number to default, 0 for no default
--          #title      : 'Unsaved File Alert',
--          #message    : 'Save before quitting?',
--          #icon       : <#stop/#note/#caution/#question/#error>,
--          #movable    : <TRUE/FALSE>]
--     itemPropList    =
--        [ #value      : <1 or 1.1 or 'string'>,
--          #type       : <#widgetType>,
--          #attributes : #attributeTypes,
--          #title      : <'title'>,          -- title to display for item, '' for no title
--          #tip        : <'tooltip'>,        -- tip for item, '' for no tip
--          #locH       : <10>,               -- in pixels from upper left, 0 to accept default
--          #locV       : <10>                -- in pixels from upper left, 0 to accept default
--          #width      : <16>,               -- in pixels, 0 if don't care
--          #height     : <16>,               -- in pixels, 0 if don't care
--          #enabled    : <TRUE or FALSE>     -- whether or not the item is enabled ]
--      #windowItemList = [ itemPropList, itemPropList, ...]
--      #windowPropList =
--        [ #type       : <#alert/#normal/#palette>,
--          #name       : 'windowName',       -- name of window, '' for no name
--          #callback   : 'nothing',          -- callback interface
--          #mode       : <#data/#dialogUnit/#pixel>,  -- autolayout or your specifications
--          #XPosition  : <16>,               -- left of window, from left of dialog, -1 = Center
--          #YPosition  : <16>;               -- top  of window, from top  of dialog, -1 = Center
--          #width      : <200>;              -- pixel width of window, 0 if don't care/automatic
--          #height     : <200>,              -- pixel height of window, 0 if don't care/automatic
--          #modal      : <TRUE or FALSE>,    -- whether or not dialog is modal
--          #toolTips   : <TRUE or FALSE>,    -- whether or not to INITIALLY use tooltips
--          #closeBox   : <TRUE or FALSE>,    -- whether or not dialog has close box
--          #canZoom    : <TRUE or FALSE>;    -- whether or not window zooms
--      initPropList = [ #windowPropList : [], #windowItemList : [] ]
--      When an event occurs in dialog, the callback is called with 3 params, 1st a callback event symbol,
--      callbackEvents = #itemChanged/#itemClicked/#windowOpening/#windowClosed/#windowZoomed/#windowResized
--                       #itemEnteringFocus/#itemLosingFocus
--      second, event specific information. (e.g.item events will return item position in #windowItemList), and
--      third, the new item proplist for the item that was effected.
----------------------------------------------------------------------
New                object me -- call this first to access object methods
Initialize         object me, object initPropList -- call this to setup window items
Run                object me -- shows window
Stop               object me, integer stopItem -- stops window, stopitem will be passed to callback if specified
WindowOperation    object me, symbol operation -- #hide, #show, #center, #zoom, #tipsOn, #tipsOff
ItemUpdate         object me, integer itemNumber, object itemInputPropList -- update an item
GetWindowPropList  object me -- returns a property list in the standard format for initing window
GetItemPropList    object me -- returns a default itemPropList
GetWidgetList      object me -- returns a linear list of current supported widget symbols
Alert              object me, object alertList -- display an alert
GetUrl             object me, string url, boolean movable -- puts up url entry dialog
FileOpen           object me, string file -- puts up a system standard dialog for opening a file
FileSave           object me, string file, string prompt -- puts up system dialog for saving a file.
*MoaErrorToString  integer MoaError -- xtra returns a big neg. int, call and get a string explaining.

 */

namespace Director {

const char *MuiXtra::xlibName = "Mui";
const XlibFileDesc MuiXtra::fileNames[] = {
	{ "mui",   nullptr },
	{ "Mui Dialog",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "New",				MuiXtra::m_New,		 0, 0,	500 },
	{ "Initialize",				MuiXtra::m_Initialize,		 1, 0,	500 },
	{ "Run",				MuiXtra::m_Run,		 0, 0,	500 },
	{ "Stop",				MuiXtra::m_Stop,		 1, 0,	500 },
	{ "WindowOperation",				MuiXtra::m_WindowOperation,		 1, 0,	500 },
	{ "ItemUpdate",				MuiXtra::m_ItemUpdate,		 2, 0,	500 },
	{ "GetWindowPropList",				MuiXtra::m_GetWindowPropList,		 0, 0,	500 },
	{ "GetItemPropList",				MuiXtra::m_GetItemPropList,		 0, 0,	500 },
	{ "GetWidgetList",				MuiXtra::m_GetWidgetList,		 0, 0,	500 },
	{ "Alert",				MuiXtra::m_Alert,		 1, 0,	500 },
	{ "GetUrl",				MuiXtra::m_GetUrl,		 2, 0,	500 },
	{ "FileOpen",				MuiXtra::m_FileOpen,		 1, 0,	500 },
	{ "FileSave",				MuiXtra::m_FileSave,		 2, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "MoaErrorToString", MuiXtra::m_MoaErrorToString, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MuiXtraObject::MuiXtraObject(ObjectType ObjectType) :Object<MuiXtraObject>("Mui") {
	_objType = ObjectType;
}

bool MuiXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum MuiXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(MuiXtra::xlibName);
	warning("MuiXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void MuiXtra::open(ObjectType type, const Common::Path &path) {
    MuiXtraObject::initMethods(xlibMethods);
    MuiXtraObject *xobj = new MuiXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MuiXtra::close(ObjectType type) {
    MuiXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MuiXtra::m_New(int nargs) {
	g_lingo->printSTUBWithArglist("MuiXtra::m_New", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(MuiXtra::m_Initialize, 0)
XOBJSTUB(MuiXtra::m_Run, 0)
XOBJSTUB(MuiXtra::m_Stop, 0)
XOBJSTUB(MuiXtra::m_WindowOperation, 0)
XOBJSTUB(MuiXtra::m_ItemUpdate, 0)
XOBJSTUB(MuiXtra::m_GetWindowPropList, 0)
XOBJSTUB(MuiXtra::m_GetItemPropList, 0)
XOBJSTUB(MuiXtra::m_GetWidgetList, 0)
XOBJSTUB(MuiXtra::m_Alert, 0)
XOBJSTUB(MuiXtra::m_GetUrl, 0)
XOBJSTUB(MuiXtra::m_FileOpen, 0)
XOBJSTUB(MuiXtra::m_FileSave, 0)
XOBJSTUB(MuiXtra::m_MoaErrorToString, 0)

}
