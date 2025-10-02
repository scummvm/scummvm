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
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-code.h"
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
	{ "setRolloverCallback",	QTVR::m_setRolloverCallback,	2, 2,	400 },	// D4
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

QTVRXObject::~QTVRXObject() {
	close();
}

void QTVRXObject::close() {
	if (_video) {
		_video->close();
		delete _video;
		_video = nullptr;
	}
	if (_widget) {
		delete _widget;
		_widget = nullptr;
	}
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

void QTVR::m_dispose(int nargs) {
	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;
	me->close();
}

void QTVR::m_getHPanAngle(int nargs) {
	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_video->getPanAngle());
}

void QTVR::m_getMovieRect(int nargs) {
	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(Common::String::format("%d,%d,%d,%d", me->_rect.left, me->_rect.top, me->_rect.right, me->_rect.bottom));
}

void QTVR::m_getNodeID(int nargs) {
	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	g_lingo->push((int)me->_video->getCurrentNodeID());
}

void QTVR::m_getQuality(int nargs) {
	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_video->getQuality());
}

void QTVR::m_getVPanAngle(int nargs) {
	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_video->getTiltAngle());
}

void QTVR::m_getZoomAngle(int nargs) {
	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_video->getFOV());
}

void QTVR::m_mouseOver(int nargs) {
	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;
	Common::Point pos = g_director->getCurrentWindow()->getMousePos();

	if (!me->_active || !me->_rect.contains(pos)) {
		g_lingo->pushVoid();
		return;
	}

	// Execute handler on first call to MouseOver
	int hotspot;

	int node;
	bool nodeChanged = false;

	while (true) {
		Graphics::Surface const *frame = me->_video->decodeNextFrame();

		if (!frame) {
			g_lingo->pushVoid();
			return;
		}

		Graphics::Surface *dither = frame->convertTo(g_director->_wm->_pixelformat, me->_video->getPalette(), 256, g_director->getPalette(), 256, Graphics::kDitherNaive);

		g_director->getCurrentWindow()->getSurface()->copyRectToSurface(
			dither->getPixels(), dither->pitch, me->_rect.left, me->_rect.top, dither->w, dither->h
		);

		dither->free();
		delete dither;

		g_director->getCurrentWindow()->setDirty(true);

		Common::Event event;

		while (g_system->getEventManager()->pollEvent(event)) {
			if (Common::isMouseEvent(event)) {
				pos = g_director->getCurrentWindow()->getMousePos();

				if (!me->_rect.contains(pos))
					break;
			}

			node = me->_video->getCurrentNodeID();

			if (event.type == Common::EVENT_LBUTTONUP) {
				me->_widget->processEvent(event);

				if (me->_video->getCurrentNodeID() != node)
					nodeChanged = true;

				hotspot = me->_video->getClickedHotspotID();

				if (!hotspot) {
					if (nodeChanged)
						g_lingo->push(Common::String::format("jump,%d", node));
					else
						g_lingo->push(Common::String("pan ,0"));

					return;
				}

				const Common::QuickTimeParser::PanoHotSpot *hotspotData = me->_video->getHotSpotByID(hotspot);

				if (nodeChanged || (hotspotData && hotspotData->type == MKTAG('l','i','n','k')))
					g_lingo->push(Common::String::format("jump,%d", node));
				else
					g_lingo->push(Common::String::format("%s,%d", hotspotData ? tag2str((uint32)hotspotData->type) : "undf", hotspot));

				return;
			}

			hotspot = me->_video->getRolloverHotspotID();

			me->_widget->processEvent(event);

			if (hotspot != me->_video->getRolloverHotspotID() && !me->_rolloverCallbackMethod.empty()) {
				g_lingo->push(me->_rolloverCallbackObject);
				g_lingo->push(me->_video->getRolloverHotspotID());

				int cframe = g_lingo->_state->callstack.size();
				LC::call(me->_rolloverCallbackObject.u.obj->getMethod(me->_rolloverCallbackMethod), 2, false);
				g_lingo->execute(cframe);
			}
		}

		LB::b_updateStage(0);

		if (!me->_rect.contains(pos))
			break;

		if (event.type == Common::EVENT_QUIT) {
			g_director->processEventQUIT();
			break;
		}

		g_director->delayMillis(10);
	}

	g_lingo->_theResult = 0;
}

void QTVR::m_name(int nargs) {
	// TODO Clarify that it is indeed hotspot name
	g_lingo->printArgs("QTVR::m_name", nargs);

	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	const Common::QuickTimeParser::PanoHotSpot *hotspot = me->_video->getClickedHotspot();

	if (!hotspot) {
		g_lingo->push(Common::String(""));
		return;
	}

	g_lingo->push(me->_video->getHotSpotName(hotspot->id));
}

void QTVR::m_openMovie(int nargs) {
	g_lingo->printArgs("QTVR::m_QTVROpen", nargs);
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


	me->_widget = new QtvrWidget(me, g_director->getCurrentWindow()->getMacWindow(),
			me->_rect.left, me->_rect.top, me->_rect.width(), me->_rect.height(),
			g_director->getMacWindowManager());

	g_lingo->push(path.toString());
}

void QTVR::m_setActive(int nargs) {
	ARGNUMCHECK(1);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	me->_active = (g_lingo->pop().asInt() != 0);
}

void QTVR::m_setHPanAngle(int nargs) {
	ARGNUMCHECK(1);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	me->_video->setPanAngle(atof(g_lingo->pop().asString().c_str()));
}

void QTVR::m_setNodeID(int nargs) {
	ARGNUMCHECK(1);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	me->_video->goToNode(g_lingo->pop().asInt());
}

void QTVR::m_setQuality(int nargs) {
	ARGNUMCHECK(1);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	me->_video->setQuality(g_lingo->pop().asInt());
}

void QTVR::m_setRolloverCallback(int nargs) {
	ARGNUMCHECK(2);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	me->_rolloverCallbackMethod = g_lingo->pop().asString();
	me->_rolloverCallbackObject = g_lingo->pop();

	if (me->_rolloverCallbackObject.type != OBJECT || !(me->_rolloverCallbackObject.u.obj->getObjType() & kFactoryObj))
		error("QTVR::m_setRolloverCallback(): first argument is not a Factory but %s", me->_rolloverCallbackObject.type2str());
}

void QTVR::m_setTransitionMode(int nargs) {
	ARGNUMCHECK(1);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	me->_video->setTransitionMode(g_lingo->pop().asString());
}

void QTVR::m_setTransitionSpeed(int nargs) {
	ARGNUMCHECK(1);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	me->_video->setTransitionSpeed(g_lingo->pop().asFloat());
}

void QTVR::m_setVPanAngle(int nargs) {
	ARGNUMCHECK(1);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	me->_video->setTiltAngle(atof(g_lingo->pop().asString().c_str()));
}

void QTVR::m_setZoomAngle(int nargs) {
	ARGNUMCHECK(1);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	me->_video->setFOV(atof(g_lingo->pop().asString().c_str()));
}

void QTVR::m_update(int nargs) {
	ARGNUMCHECK(0);

	QTVRXObject *me = (QTVRXObject *)g_lingo->_state->me.u.obj;

	me->_active = true;

	Graphics::Surface const *frame = me->_video->decodeNextFrame();

	if (!frame)
		return;

	Graphics::Surface *dither = frame->convertTo(g_director->_wm->_pixelformat, me->_video->getPalette(), 256, g_director->getPalette(), 256, Graphics::kDitherNaive);

	g_director->getCurrentWindow()->getSurface()->copyRectToSurface(
		dither->getPixels(), dither->pitch, me->_rect.left, me->_rect.top, dither->w, dither->h
	);

	dither->free();
	delete dither;
}


///////////////
// Widget
///////////////

QtvrWidget::QtvrWidget(QTVRXObject *xtra, Graphics::MacWidget *parent, int x, int y, int w, int h, Graphics::MacWindowManager *wm) :
	Graphics::MacWidget(parent, x, y, w, h, wm, true), _xtra(xtra) {

	_priority = 10000; // We stay on top of everything
}

bool QtvrWidget::processEvent(Common::Event &event) {
	if (!_xtra->_active)
		return false;

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
