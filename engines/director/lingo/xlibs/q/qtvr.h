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

#ifndef DIRECTOR_LINGO_XLIBS_QTVR_H
#define DIRECTOR_LINGO_XLIBS_QTVR_H

#include "video/qt_decoder.h"

namespace Director {

namespace QTVR {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_getHPanAngle(int nargs);
void m_getMovieRect(int nargs);
void m_getNodeID(int nargs);
void m_getQuality(int nargs);
void m_getVPanAngle(int nargs);
void m_getZoomAngle(int nargs);
void m_mouseOver(int nargs);
void m_name(int nargs);
void m_openMovie(int nargs);
void m_setActive(int nargs);
void m_setHPanAngle(int nargs);
void m_setNodeID(int nargs);
void m_setQuality(int nargs);
void m_setRolloverCallback(int nargs);
void m_setTransitionMode(int nargs);
void m_setTransitionSpeed(int nargs);
void m_setVPanAngle(int nargs);
void m_setZoomAngle(int nargs);
void m_update(int nargs);

} // End of namespace QTVR

class QTVRXObject;

class QtvrWidget : public Graphics::MacWidget {
public:
	QtvrWidget(QTVRXObject *xtra, Graphics::MacWidget *parent, int x, int y, int w, int h, Graphics::MacWindowManager *wm);

	virtual bool processEvent(Common::Event &event);

	QTVRXObject *_xtra;
};


class QTVRXObject : public Object<QTVRXObject> {
public:
	QTVRXObject(ObjectType objType);
	~QTVRXObject();

	void close();

	Common::Rect _rect;
	Video::QuickTimeDecoder *_video = nullptr;
	QtvrWidget *_widget = nullptr;
	Datum _rolloverCallbackObject;
	Common::String _rolloverCallbackMethod;
	bool _active = true;
};

} // End of namespace Director

#endif
