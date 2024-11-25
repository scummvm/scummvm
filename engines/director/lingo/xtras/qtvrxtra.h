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

#ifndef DIRECTOR_LINGO_XTRAS_QTVRXTRA_H
#define DIRECTOR_LINGO_XTRAS_QTVRXTRA_H

#include "video/qt_decoder.h"

namespace Director {

class QtvrxtraXtraObject : public Object<QtvrxtraXtraObject> {
public:
	QtvrxtraXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;

	bool processEvent(Common::Event &event);

	Video::QuickTimeDecoder *_video;

	Common::Rect _rect;
	bool _visible;
	float _quality;

	Video::QuickTimeDecoder::NodeData _currentNode;

	Common::String _transitionMode;
	float _transitionSpeed;

	Common::String _updateMode;

	bool _capEventsMouseOver;
	bool _capEventsMouseDown;

	bool _passMouseDown;

	Common::String _mouseDownHandler;
};

namespace QtvrxtraXtra {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_forget(int nargs);
void m_QTVREnter(int nargs);
void m_QTVRExit(int nargs);
void m_QTVROpen(int nargs);
void m_QTVRClose(int nargs);
void m_QTVRUpdate(int nargs);
void m_QTVRGetQTVRType(int nargs);
void m_QTVRIdle(int nargs);
void m_QTVRMouseDown(int nargs);
void m_QTVRMouseOver(int nargs);
void m_QTVRGetPanAngle(int nargs);
void m_QTVRSetPanAngle(int nargs);
void m_QTVRGetTiltAngle(int nargs);
void m_QTVRSetTiltAngle(int nargs);
void m_QTVRGetFOV(int nargs);
void m_QTVRSetFOV(int nargs);
void m_QTVRGetClickLoc(int nargs);
void m_QTVRSetClickLoc(int nargs);
void m_QTVRGetClickPanAngles(int nargs);
void m_QTVRGetClickPanLoc(int nargs);
void m_QTVRGetHotSpotID(int nargs);
void m_QTVRSetHotSpotID(int nargs);
void m_QTVRGetHotSpotName(int nargs);
void m_QTVRGetHotSpotType(int nargs);
void m_QTVRGetHotSpotViewAngles(int nargs);
void m_QTVRGetObjectViewAngles(int nargs);
void m_QTVRGetObjectZoomRect(int nargs);
void m_QTVRGetNodeID(int nargs);
void m_QTVRSetNodeID(int nargs);
void m_QTVRGetNodeName(int nargs);
void m_QTVRGetQuality(int nargs);
void m_QTVRSetQuality(int nargs);
void m_QTVRGetTransitionMode(int nargs);
void m_QTVRSetTransitionMode(int nargs);
void m_QTVRGetTransitionSpeed(int nargs);
void m_QTVRSetTransitionSpeed(int nargs);
void m_QTVRGetUpdateMode(int nargs);
void m_QTVRSetUpdateMode(int nargs);
void m_QTVRGetVisible(int nargs);
void m_QTVRSetVisible(int nargs);
void m_QTVRGetWarpMode(int nargs);
void m_QTVRSetWarpMode(int nargs);
void m_QTVRCollapseToHotSpotRgn(int nargs);
void m_QTVRZoomOutEffect(int nargs);
void m_QTVRGetColumn(int nargs);
void m_QTVRSetColumn(int nargs);
void m_QTVRGetRow(int nargs);
void m_QTVRSetRow(int nargs);
void m_QTVRNudge(int nargs);
void m_QTVRGetMouseDownHandler(int nargs);
void m_QTVRSetMouseDownHandler(int nargs);
void m_QTVRGetMouseOverHandler(int nargs);
void m_QTVRSetMouseOverHandler(int nargs);
void m_QTVRGetMouseStillDownHandler(int nargs);
void m_QTVRSetMouseStillDownHandler(int nargs);
void m_QTVRGetNodeLeaveHandler(int nargs);
void m_QTVRSetNodeLeaveHandler(int nargs);
void m_QTVRGetPanZoomStartHandler(int nargs);
void m_QTVRSetPanZoomStartHandler(int nargs);
void m_QTVRGetRolloverHotSpotHandler(int nargs);
void m_QTVRSetRolloverHotSpotHandler(int nargs);
void m_QTVRExitMouseOver(int nargs);
void m_QTVRPassMouseDown(int nargs);
void m_IsQTVRMovie(int nargs);

} // End of namespace QtvrxtraXtra

} // End of namespace Director

#endif
