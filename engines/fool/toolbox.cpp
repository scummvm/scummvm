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

#include "fool/fool.h"
#include "fool/toolbox.h"

namespace Fool {

Toolbox::Toolbox() {
	this->_frameLimiter = new Graphics::FrameLimiter(g_system, 60);
}

Toolbox::~Toolbox() {
	delete this->_frameLimiter;
}

void Toolbox::_pumpEvents() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		EventRecord newRecord;
		newRecord.when = this->TickCount();
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			newRecord.what = kMouseDown;
			newRecord.where = Common::Point(event.mouse.x, event.mouse.y);
			this->_events.push(newRecord);
			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			newRecord.what = kMouseUp;
			newRecord.where = Common::Point(event.mouse.x, event.mouse.y);
			this->_events.push(newRecord);
			break;
		default:
			break;
		}
	}
}

void Toolbox::_updateScreen() {
	g_engine->_wm.draw();
	this->_frameLimiter->delayBeforeSwap();
	g_system->updateScreen();
	this->_frameLimiter->startFrame();
}

void Toolbox::BeginUpdate(WindowRecord &theWindow) {
	warning("STUB: Toolbox::BeginUpdate");
}

void Toolbox::ClearMenuBar() {
	warning("STUB: Toolbox::ClearMenuBar");
}

void Toolbox::ClosePoly() {
	warning("STUB: Toolbox::ClosePoly");
}

void Toolbox::ClipRect(Common::Rect &r) {
	warning("STUB: Toolbox::ClipRect");
}

void Toolbox::CopyBits(const Graphics::Surface &srcBits, Graphics::Surface &dstBits, const Common::Rect &srcRect, const Common::Rect &dstRect, uint16 mode, RgnHandle maskRgn) {
	warning("STUB: Toolbox::CopyBits");
}

uint32 Toolbox::Delay(uint32 numTicks) {
	uint32 target = g_system->getMillis() + (numTicks * 1000 / 60);
	while (g_system->getMillis() < target) {
		this->_pumpEvents();
		this->_updateScreen();
	}
	return (uint32)(g_system->getMillis() * 60 / 1000);
}

void Toolbox::DrawString(const Common::String &s) {
	warning("STUB: Toolbox::DrawString");
}

void Toolbox::EndUpdate(WindowRecord &theWindow) {
	warning("STUB: Toolbox::EndUpdate");
}

void Toolbox::FillOval(const Common::Rect &r, const Pattern &pat) {
	warning("STUB: Toolbox::FillOval");
}

void Toolbox::FillRect(const Common::Rect &r, const Pattern &pat) {
	warning("STUB: Toolbox::FillRect");
}

void Toolbox::FrameArc(const Common::Rect &r, int16 startAngle, int16 arcAngle) {
	warning("STUB: Toolbox::FrameArc");
}

void Toolbox::FrameOval(const Common::Rect &r) {
	warning("STUB: Toolbox::FrameOval");
}

void Toolbox::FrameRect(const Common::Rect &r) {
	warning("STUB: Toolbox::FrameRect");
}

void Toolbox::GetCPixel(int16 h, int16 v, RGBColor &cPix) {
	warning("STUB: Toolbox::GetCPixel");
}

bool Toolbox::GetNextEvent(uint16 eventMask, EventRecord &theEvent) {
	warning("STUB: Toolbox::GetNextEvent");
	this->_pumpEvents();
	if (!this->_events.empty()) {
		theEvent = this->_events.pop();
	} else {
		theEvent.what = kNullEvent;
		// pretend mouse button is up
		theEvent.modifiers = 0x80;
	}
	return true;
}

PicHandle Toolbox::GetPicture(uint16 picID) {
	warning("STUB: Toolbox::GetPicture");
	return 0;
}

void Toolbox::HideCursor() {
	warning("STUB: Toolbox::HideCursor");
}

void Toolbox::InitCursor() {
	warning("STUB: Toolbox::InitCursor");
}

void Toolbox::InsetRect(Common::Rect &r, int16 dh, int16 dv) {
	warning("STUB: Toolbox::InsetRect");
}

void Toolbox::InvertOval(const Common::Rect &r) {
	warning("STUB: Toolbox::InvertOval");
}

void Toolbox::InvertRect(const Common::Rect &r) {
	warning("STUB: Toolbox::InvertRect");
}

void Toolbox::KillPoly(PolyHandle poly) {
	warning("STUB: Toolbox::KillPoly");
}

void Toolbox::LineTo(int16 h, int16 v) {
	warning("STUB: Toolbox::LineTo");
}

void Toolbox::MovePortTo(int16 leftGlobal, int16 topGlobal) {
	warning("STUB: Toolbox::MovePortTo");
}

void Toolbox::MoveTo(int16 h, int16 v) {
	warning("STUB: Toolbox::MoveTo");
}

PolyHandle Toolbox::OpenPoly() {
	warning("STUB: Toolbox::OpenPoly");
	return 0;
}

void Toolbox::OpenPort(GrafPtr port) {
	warning("STUB: Toolbox::OpenPort");
	// source: QuickDraw Routines I-163
	port->device = 0;
	//port->portBits = screenBits;
	//port->portRect = screenBits.bounds;
	//port->visRgn
	//port->clipRgn
	//port->bkPat = white
}

void Toolbox::PaintOval(const Common::Rect &r) {
	warning("STUB: Toolbox::PaintOval");
}

void Toolbox::PaintPoly(PolyHandle poly) {
	warning("STUB: Toolbox::PaintPoly");
}

void Toolbox::PaintRect(const Common::Rect &r) {
	warning("STUB: Toolbox::PaintRect");
}

void Toolbox::PenMode(uint16 mode) {
	warning("STUB: Toolbox::PenMode");
}

void Toolbox::PenNormal() {
	warning("STUB: Toolbox::PenNormal");
}

void Toolbox::PenPat(const Pattern &pat) {
	warning("STUB: Toolbox::PenPat");

}

void Toolbox::PenSize(uint16 width, uint16 height) {
	warning("STUB: Toolbox::PenSize");
}

void Toolbox::PortSize(uint16 width, uint16 height) {
	warning("STUB: Toolbox::PortSize");
}

void Toolbox::SetCPixel(int16 h, int16 v, const RGBColor &cPix) {
	warning("STUB: Toolbox::SetCPixel");

}

void Toolbox::SetPort(GrafPtr port) {
	warning("STUB: Toolbox::SetPort");

}

void Toolbox::SetPortBits(const Graphics::Surface *bm) {
	warning("STUB: Toolbox::SetPortBits");

}

void Toolbox::SetRect(Common::Rect &r, int16 left, int16 top, int16 right, int16 bottom) {
		r.left = left;
		r.top = top;
		r.right = right;
		r.bottom = bottom;
	}

uint16 Toolbox::StringWidth(const Common::String &s) {
	warning("STUB: Toolbox::StringWidth");
	return 0;
}

uint32 Toolbox::TickCount() {
	return (uint32)(g_system->getMillis() * 60 / 1000);
}


} // namespace Fool
