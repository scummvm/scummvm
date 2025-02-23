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

/**********************************************
 *
 * USED IN:
 * Star Trek TNG Interactive Technial Manual
 *
 **********************************************/

/*
 * -- QTVRW External Factory. 14Oct94 GRB
 * QTVRW
 * X    mDispose
 * S    mGetHPanAngle
 * S    mGetMovieRect
 * I    mGetNodeID
 * I    mGetQuality
 * S    mGetVPanAngle
 * S    mGetZoomAngle
 * S    mMouseOver
 * S    mName
 * IS   mNew, type
 * ISII mOpenMovie filename, x, y
 * II   mSetActive mode
 * XS   mSetHPanAngle angle
 * II   mSetNodeID node
 * II   mSetQuality qual
 * XOS  mSetRolloverCallback factory, method
 * IS   mSetTransitionMode mode
 * II   mSetTransitionSpeed speed
 * XS   mSetVPanAngle angle
 * XS   mSetZoomAngle angle
 * I    mUpdate
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/qtvr.h"
#include "director/window.h"


namespace Director {

const char *const QTVR::xlibName = "QTVRW";
const XlibFileDesc QTVR::fileNames[] = {
	{ "QTVR",		nullptr },
	{ "QTVR.QTC",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",					QTVR::m_new,					1, 1,	400 },	// D4
	{ "dispose",				QTVR::m_dispose,				1, 1,	400 },	// D4
	{ "getHPanAngle",			QTVR::m_getHPanAngle,			0, 0,	400 },	// D4
	{ "getMovieRect",			QTVR::m_getMovieRect,			0, 0,	400 },	// D4
	{ "getNodeID",				QTVR::m_getNodeID,				0, 0,	400 },	// D4
	{ "getQuality",				QTVR::m_getQuality,				0, 0,	400 },	// D4
	{ "getVPanAngle",			QTVR::m_getVPanAngle,			0, 0,	400 },	// D4
	{ "getZoomAngle",			QTVR::m_getZoomAngle,			0, 0,	400 },	// D4
	{ "mouseOver",				QTVR::m_mouseOver,				0, 0,	400 },	// D4
	{ "name",					QTVR::m_name,					0, 0,	400 },	// D4
	{ "openMovie",				QTVR::m_openMovie,				3, 3,	400 },	// D4
	{ "setActive",				QTVR::m_setActive,				1, 1,	400 },	// D4
	{ "setHPanAngle",			QTVR::m_setHPanAngle,			1, 1,	400 },	// D4
	{ "setNodeID",				QTVR::m_setNodeID,				1, 1,	400 },	// D4
	{ "setQuality",				QTVR::m_setQuality,				1, 1,	400 },	// D4
	{ "setRolloverCallback",	QTVR::m_setQuality,				2, 2,	400 },	// D4
	{ "setTransitionMode",		QTVR::m_setTransitionMode,		1, 1,	400 },	// D4
	{ "setTransitionSpeed",		QTVR::m_setTransitionSpeed,		1, 1,	400 },	// D4
	{ "setVPanAngle",			QTVR::m_setVPanAngle,			1, 1,	400 },	// D4
	{ "setZoomAngle",			QTVR::m_setZoomAngle,			1, 1,	400 },	// D4
	{ "update",					QTVR::m_update,					0, 0,	400 },	// D4
	{ nullptr,					nullptr,						0, 0,	0 }
};

QTVRXObject::QTVRXObject(ObjectType ObjectType) :Object<QTVRXObject>("QTVR") {
	_objType = ObjectType;
}

void QTVR::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		QTVRXObject::initMethods(xlibMethods);
		QTVRXObject *xobj = new QTVRXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void QTVR::close(ObjectType type) {
	if (type == kXObj) {
		QTVRXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


void QTVR::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("QTVR::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(QTVR::m_dispose)
XOBJSTUB(QTVR::m_getHPanAngle, "")
XOBJSTUB(QTVR::m_getMovieRect, "")
XOBJSTUB(QTVR::m_getNodeID, 0)
XOBJSTUB(QTVR::m_getQuality, 0)
XOBJSTUB(QTVR::m_getVPanAngle, "")
XOBJSTUB(QTVR::m_getZoomAngle, "")
XOBJSTUB(QTVR::m_mouseOver, "")
XOBJSTUB(QTVR::m_name, "")

void QTVR::m_openMovie(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVROpen", nargs);
	ARGNUMCHECK(3);

	int top = g_lingo->pop().asInt();
	int left = g_lingo->pop().asInt();
	Common::String pathStr = g_lingo->pop().asString();

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	Common::Path path = findMoviePath(pathStr);
	if (path.empty()) {
		Common::String error = Common::String::format("Error: Movie file ('%s') not found!", pathStr.c_str());
		g_lingo->push(error);
		return;
	}

	me->_video = new Video::QuickTimeDecoder();
	debugC(5, kDebugXObj, "QtvrxtraXtra::m_QTVROpen(): Loading QT file ('%s')", path.toString().c_str());
	if (!me->_video->loadFile(path)) {
		Common::String error = Common::String::format("Error: Failed to load movie file ('%s')!", path.toString().c_str());
		g_lingo->push(error);
		return;
	}

	me->_video->setTargetSize(320, 200);

	me->_rect = Common::Rect(left, top, left + me->_video->getWidth(), top + me->_video->getHeight());


	me->_widget = new QtvrWidget(me, g_director->getCurrentWindow(),
			me->_rect.left, me->_rect.top, me->_rect.width(), me->_rect.height(),
			g_director->getMacWindowManager());

	g_lingo->push(path.toString());
}

XOBJSTUB(QTVR::m_setActive, 0)
XOBJSTUBNR(QTVR::m_setHPanAngle)
XOBJSTUB(QTVR::m_setNodeID, 0)
XOBJSTUB(QTVR::m_setQuality, 0)
XOBJSTUBNR(QTVR::m_setRolloverCallback)
XOBJSTUB(QTVR::m_setTransitionMode, 0)
XOBJSTUB(QTVR::m_setTransitionSpeed, 0)
XOBJSTUBNR(QTVR::m_setVPanAngle)
XOBJSTUBNR(QTVR::m_setZoomAngle)

void QTVR::m_update(int nargs) {
	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	Graphics::Surface const *frame = me->_video->decodeNextFrame();

	if (!frame)
		return;

	Graphics::Surface *dither = frame->convertTo(g_director->_wm->_pixelformat, me->_video->getPalette(), 256, g_director->getPalette(), 256, Graphics::kDitherNaive);

	g_director->getCurrentWindow()->getSurface()->copyRectToSurface(
		dither->getPixels(), dither->pitch, me->_rect.left, me->_rect.top, dither->w, dither->h
	);
}


///////////////
// Widget
///////////////

QtvrWidget::QtvrWidget(QTVRXObject *xtra, Graphics::MacWidget *parent, int x, int y, int w, int h, Graphics::MacWindowManager *wm) :
	Graphics::MacWidget(parent, x, y, w, h, wm, true), _xtra(xtra) {

	_priority = 10000; // We stay on top of everything
}

bool QtvrWidget::processEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		_xtra->_video->handleMouseButton(true, event.mouse.x - _xtra->_rect.left, event.mouse.y - _xtra->_rect.top);
		return true;
	case Common::EVENT_LBUTTONUP:
		_xtra->_video->handleMouseButton(false, event.mouse.x - _xtra->_rect.left, event.mouse.y - _xtra->_rect.top);
		return true;
	case Common::EVENT_MOUSEMOVE:
		_xtra->_video->handleMouseMove(event.mouse.x - _xtra->_rect.left, event.mouse.y - _xtra->_rect.top);
		return true;
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
		_xtra->_video->handleKey(event.kbd, event.type == Common::EVENT_KEYDOWN);
		return true;
	default:
		return false;
	}
}

} // End of namespace Director
