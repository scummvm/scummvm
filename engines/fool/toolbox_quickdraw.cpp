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

#include "common/debug.h"
#include "common/memstream.h"
#include "common/system.h"

#include "fool/detection.h"
#include "graphics/cursor.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/managed_surface.h"
#include "image/pict.h"

#include "fool/fool.h"
#include "fool/toolbox.h"
#include "fool/utils.h"

namespace Fool {

void Toolbox::BackPat(const Pattern &pat) {
	if (_port) {
		_port->bkPat = pat;
	}
}

void Toolbox::BeginUpdate(WindowRecord &theWindow) {
	warning("STUB: Toolbox::BeginUpdate");
}

void Toolbox::ClosePoly() {
	ShowPen();
	if (_port) {
		if (_port->polySave) {
			if (_port->polySave->polyPoints.size() >= 2) {
				Common::Point p1 = _port->polySave->polyPoints[0];
				Common::Point p2 = _port->polySave->polyPoints[1];
				_port->polySave->polyBBox = Common::Rect(
					p1.x < p2.x ? p1.x : p2.x,
					p1.y < p2.y ? p1.y : p2.y,
					p1.x < p2.x ? p2.x : p1.x,
					p1.y < p2.y ? p2.y : p1.y
				);
				for (size_t i = 2; i < _port->polySave->polyPoints.size(); i++) {
					_port->polySave->polyBBox.extend(_port->polySave->polyPoints[i]);
				}
			}
			_port->polySave->polySize = _port->polySave->polyPoints.size()*4 + 10;
			_port->polySave = nullptr;
		}
	}
}

void Toolbox::ClipRect(Common::Rect &r) {
	warning("STUB: Toolbox::ClipRect");
}

void Toolbox::CopyBits(const BitMap &srcBits, BitMap &dstBits, const Common::Rect &srcRect, const Common::Rect &dstRect, SourceMode mode, RgnHandle maskRgn) {
	_copyBits(srcBits, BitMap(nullptr), dstBits, srcRect, dstRect, mode, maskRgn);
}

void Toolbox::_copyBits(const BitMap &srcBits, const BitMap &mask, BitMap &dstBits, const Common::Rect &srcRect, const Common::Rect &dstRect, SourceMode mode, RgnHandle maskRgn) {
	if (!_port) {
		warning("Toolbox::_copyBits: empty port");
		return;
	}

	if (!srcBits) {
		warning("Toolbox::_copyBits: empty srcBits handle");
		return;
	}

	if (!dstBits) {
		warning("Toolbox::_copyBits: empty dstBits handle");
		return;
	}
	if (maskRgn) {
		warning("Toolbox::_copyBits: maskRgn unimplemented");
	}
	if (!srcRect.isValidRect()) {
		warning("Toolbox::_copyBits: invalid src rectangle");
		return;
	}
	if (!dstRect.isValidRect()) {
		warning("Toolbox::_copyBits: invalid dst rectangle");
		return;
	}

	Common::Rect clipSrcRect = srcRect;
	Common::Rect clipDstRect = dstRect;

	// destination rectangle should be moved relative to port origin
	clipDstRect.translate(_port->portRect.left, _port->portRect.top);

	// the source rectangle can include areas out of bounds, crop it
	clipSrcRect.clip(srcBits->getBounds());

	// scale the source
	BitMap subsrc(new Graphics::ManagedSurface());
	subsrc->create(*srcBits, clipSrcRect);
	if (srcRect.width() != dstRect.width() ||
			srcRect.height() != dstRect.height()) {
		// source and destination are different sizes, scale it

		// determine scale difference
		float sx = srcRect.width() == 0 ? 1.0f : (float)dstRect.width() / (float)srcRect.width();
		float sy = srcRect.height() == 0 ? 1.0f : (float)dstRect.height() / (float)srcRect.height();
		if (clipSrcRect != srcRect) {
			// apply clipping offsets
			clipDstRect.left += (clipSrcRect.left - srcRect.left)*sx;
			clipDstRect.top += (clipSrcRect.top - srcRect.top)*sy;
			clipDstRect.right += (clipSrcRect.right - srcRect.right)*sx;
			clipDstRect.bottom += (clipSrcRect.bottom - srcRect.bottom)*sy;
		}
		subsrc = BitMap(subsrc->scale(clipDstRect.width(), clipDstRect.height()));
	} else {
		// source and destination are the same size, no scaling
		if (clipSrcRect != srcRect) {
			// apply clipping offsets
			clipDstRect.left += clipSrcRect.left - srcRect.left;
			clipDstRect.top += clipSrcRect.top - srcRect.top;
			clipDstRect.right += clipSrcRect.right - srcRect.right;
			clipDstRect.bottom += clipSrcRect.bottom - srcRect.bottom;
		}
	}
	Common::Rect result = blitMono(subsrc, dstBits, mask, Common::Point(clipDstRect.left, clipDstRect.top), mode);
	_addDirtyRect(dstBits, result);
}

// maybe the better way of refactoring this would be to fork the Primitives
// object from MacWindowManager and make one that uses GrafPort as a state store?
// as it stands, all the operations fall back to calling DrawPoint over and over again,
// which can touch the same pixel multiple times.
// this is a problem for all the xor blend modes.
// we could have it so that the draw commands check a mask before drawing again, and it
// gets reset after every op. this would be pretty inefficient; e.g. in fools errand most
// of the time is spent drawing to the screen or to a screen-sized buffer.

// steps:
// - create intermediate surface
// - draw to intermediate surface using MacPlotData
// - blit to target with blitMono
// - add to target dirty rect list

void Toolbox::EraseOval(const Common::Rect &r) {
	if (_port) {
		_drawOval(r, _port->bkPat, kPatCopy, false, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpRect(kOpEraseOval, r);
		}
	}
}

void Toolbox::ErasePoly(const PolyHandle &poly) {
	if (_port) {
		_drawPoly(poly, _port->bkPat, kPatCopy, false, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpPoly(kOpErasePoly, poly);
		}
	}
}

void Toolbox::EraseRect(const Common::Rect &r) {
	if (_port) {
		_drawRect(r, _port->bkPat, kPatCopy, false, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpRect(kOpEraseRect, r);
		}
	}
}

void Toolbox::EraseRoundRect(const Common::Rect &r, uint16 ovalWidth, uint16 ovalHeight) {
	if (_port) {
		_drawRoundRect(r, _port->bkPat, kPatCopy, false, _port->fgColor, _port->bkColor, ovalWidth, ovalHeight);
		if (_port->picSave) {
			_port->picSave->pushOpPoint(kOpOvSize, Common::Point(ovalWidth, ovalHeight));
			_port->picSave->pushOpRect(kOpEraseRRect, r);
		}
	}
}

void Toolbox::EndUpdate(WindowRecord &theWindow) {
	warning("STUB: Toolbox::EndUpdate");
}

void Toolbox::_addDirtyRect(const BitMap &dest, const Common::Rect &rect) {
	if (dest == _defaultBits) {
		_defaultWindow->addDirtyRect(rect);
		_defaultWindow->setDirty(true);
	}
	if (dest == _defaultMenuBits) {
	//	_defaultMenu->addDirtyRect(dstRect);
		_defaultMenu->setDirty(true);
	}
}

void Toolbox::_drawOval(const Common::Rect &r, const Pattern &pat, PatternMode mode, bool frame, uint32 fgColor, uint32 bkColor) {
	if ((!r.isValidRect()) || r.isEmpty()) {
		warning("Toolbox::_drawOval: invalid rect %d %d %d %d", r.left, r.top, r.right, r.bottom);
		return;
	}
	if (_port && _port->pnVis == 0) {

		BitMap intermediate(new Graphics::ManagedSurface(r.width(), r.height()));
		BitMap mask(new Graphics::ManagedSurface(r.width(), r.height()));
		Common::Point destPos(r.left, r.top);
		// move to port coordinate space
		destPos.x += _port->portRect.left;
		destPos.y += _port->portRect.top;
		Graphics::MacPatterns macpat({pat.data});

		// Only use a different pen size in frame mode, otherwise small filled shapes appear wrong
		Common::Point penSize = frame ? _port->pnSize : Common::Point(1, 1);
		Graphics::MacPlotData pd(intermediate.get(), mask.get(), &macpat, 1, destPos.x, destPos.y, penSize, bkColor);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();
		// For thicker outlines, the shape should be adjusted inward
		Common::Rect destRect = intermediate->getBounds();
		destRect.right -= penSize.x;
		destRect.bottom -= penSize.y;

		pm.drawEllipse(0, 0, destRect.width(), destRect.height(), fgColor, !frame, &pd);

		//byte fakePal[768];
		//Common::fill(fakePal, fakePal+3, 0xff);
		//Common::fill(fakePal+3, fakePal+768, 0x00);
		//mask->rawSurface().debugPrint(5, 0, 0, 0, 0, -1, 512, fakePal);

		Common::Rect dstRect = blitMono(intermediate, _port->portBits, mask, destPos, mode);
		_addDirtyRect(_port->portBits, dstRect);
	}
}

void Toolbox::_drawPoly(const PolyHandle &poly, const Pattern &pat, PatternMode mode, bool frame, uint32 fgColor, uint32 bkColor) {
	if (!poly) {
		warning("_drawPoly: Polygon data not found, skipping");
		return;
	}
	if (poly->polyPoints.size() < 2) {
		warning("_drawPoly: need at least 2 points");
		return;
	}
	if (_port && _port->pnVis == 0) {
		Common::Rect bbox = poly->polyBBox;
		Common::Rect destRect(bbox.width(), bbox.height());
		// For thicker outlines, the shape will overflow the dimensions a little bit
		destRect.right += _port->pnSize.x - 1;
		destRect.bottom += _port->pnSize.y - 1;

		BitMap intermediate(new Graphics::ManagedSurface(destRect.width(), destRect.height()));
		BitMap mask(new Graphics::ManagedSurface(destRect.width(), destRect.height()));

		Common::Point destPos(bbox.left, bbox.top);
		Graphics::MacPatterns macpat({pat.data});

		// move to port coordinate space
		destPos.x += _port->portRect.left;
		destPos.y += _port->portRect.top;

		Graphics::MacPlotData pd(intermediate.get(), mask.get(), &macpat, 1, destPos.x, destPos.y, _port->pnSize, bkColor);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();

		Common::Point origin(bbox.left, bbox.top);

		if (frame) {
			for (int i = 0; i < (int)poly->polyPoints.size() - 1; i++) {
				Common::Point p1 = poly->polyPoints[i] - origin;
				Common::Point p2 = poly->polyPoints[i+1] - origin;
				pm.drawLine(p1.x, p1.y, p2.x, p2.y, fgColor, &pd);
			}
		} else {
			Common::Array<int> polyX;
			Common::Array<int> polyY;
			for (auto &it : poly->polyPoints) {
				polyX.push_back(it.x - origin.x);
				polyY.push_back(it.y - origin.y);
				debugC(5, kDebugGraphics, "Toolbox::_drawPoly: (%d, %d)", it.x - bbox.left, it.y - bbox.top);
			}
			pm.drawPolygonScan(polyX.data(), polyY.data(), poly->polyPoints.size(), destRect, fgColor, &pd);
		}


		Common::Rect dstRect = blitMono(intermediate, _port->portBits, mask, destPos, mode);

		if (debugChannelSet(5, kDebugGraphics)) {
			//byte fakePal[768];
			//Common::fill(fakePal, fakePal+3, 0xff);
			//Common::fill(fakePal+3, fakePal+768, 0x00);
			//mask->rawSurface().debugPrint(5, 0, 0, 0, 0, -1, 512, fakePal);

			debugC(5, kDebugGraphics, "Toolbox::_drawPoly: dstRect (%d, %d) %dx%d, pattern %s, mode %d, frame %d, fgColor %08x, bkColor %08x", dstRect.left, dstRect.top, dstRect.width(), dstRect.height(), pat.format().c_str(), mode, frame, fgColor, bkColor);
		}

		_addDirtyRect(_port->portBits, dstRect);
	}
}


void Toolbox::_drawRect(const Common::Rect &r, const Pattern &pat, PatternMode mode, bool frame, uint32 fgColor, uint32 bkColor) {
	if ((!r.isValidRect()) || r.isEmpty()) {
		warning("Toolbox::_drawRect: invalid rect %d %d %d %d", r.left, r.top, r.right, r.bottom);
		return;
	}
	if (_port && _port->pnVis == 0) {
		BitMap intermediate(new Graphics::ManagedSurface(r.width(), r.height()));
		// special case because rect fills the entire surface
		BitMap mask(nullptr);
		if (frame) {
			mask = BitMap(new Graphics::ManagedSurface(r.width(), r.height()));
		}

		Common::Point destPos(r.left, r.top);
		// move to port coordinate space
		destPos.x += _port->portRect.left;
		destPos.y += _port->portRect.top;
		Graphics::MacPatterns macpat({pat.data});

		// Only use a different pen size in frame mode, otherwise small filled shapes appear wrong
		Common::Point penSize = frame ? _port->pnSize : Common::Point(1, 1);
		Graphics::MacPlotData pd(intermediate.get(), mask.get(), &macpat, 1, destPos.x, destPos.y, penSize, bkColor);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();
		// For thicker outlines, the shape should be adjusted inward
		Common::Rect destRect = intermediate->getBounds();
		destRect.right -= penSize.x - 1;
		destRect.bottom -= penSize.y - 1;

		if (frame) {
			pm.drawRect(destRect, fgColor, &pd);
		} else {
			pm.drawFilledRect(destRect, fgColor, &pd);
		}
		Common::Rect dstRect = blitMono(intermediate, _port->portBits, mask, destPos, mode);

		if (debugChannelSet(5, kDebugGraphics)) {
			debugC(5, kDebugGraphics, "Toolbox::_drawRect: dstRect (%d, %d) %dx%d, pattern %s, mode %d, frame %d, fgColor %08x, bkColor %08x", dstRect.left, dstRect.top, dstRect.width(), dstRect.height(), pat.format().c_str(), mode, frame, fgColor, bkColor);
		}

		_addDirtyRect(_port->portBits, dstRect);
	}
}

void Toolbox::_drawRoundRect(const Common::Rect &r, const Pattern &pat, PatternMode mode, bool frame, uint32 fgColor, uint32 bkColor, uint16 ovalWidth, uint16 ovalHeight) {
	if ((!r.isValidRect()) || r.isEmpty()) {
		warning("Toolbox::_drawRoundRect: invalid rect %d %d %d %d", r.left, r.top, r.right, r.bottom);
		return;
	}
	if (_port && _port->pnVis == 0) {
		BitMap intermediate(new Graphics::ManagedSurface(r.width(), r.height()));
		BitMap mask(new Graphics::ManagedSurface(r.width(), r.height()));

		Common::Point destPos(r.left, r.top);
		// move to port coordinate space
		destPos.x += _port->portRect.left;
		destPos.y += _port->portRect.top;
		Graphics::MacPatterns macpat({pat.data});

		// Only use a different pen size in frame mode, otherwise small filled shapes appear wrong
		Common::Point penSize = frame ? _port->pnSize : Common::Point(1, 1);
		Graphics::MacPlotData pd(intermediate.get(), mask.get(), &macpat, 1, destPos.x, destPos.y, penSize, bkColor);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();
		// For thicker outlines, the shape should be adjusted inward
		Common::Rect destRect = intermediate->getBounds();
		destRect.right -= penSize.x - 1;
		destRect.bottom -= penSize.y - 1;

		if (ovalWidth != ovalHeight) {
			warning("Toolbox::_drawRoundRect: different corner diameters not supported");
		}
		pm.drawRoundRect(destRect, ovalHeight/2, fgColor, !frame, &pd);

		Common::Rect dstRect = blitMono(intermediate, _port->portBits, mask, destPos, mode);

		if (debugChannelSet(5, kDebugGraphics)) {
			debugC(5, kDebugGraphics, "Toolbox::_drawRoundRect: dstRect (%d, %d) %dx%d, pattern %s, mode %d, frame %d, fgColor %08x, bkColor %08x, ovalWidth %d, ovalHeight %d", dstRect.left, dstRect.top, dstRect.width(), dstRect.height(), pat.format().c_str(), mode, frame, fgColor, bkColor, ovalWidth, ovalHeight);
		}

		_addDirtyRect(_port->portBits, dstRect);
	}
}

void Toolbox::FillOval(const Common::Rect &r, const Pattern &pat) {
	if (_port) {
		_port->fillPat = pat;
		_drawOval(r, pat, kPatCopy, false, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpPat(kOpFillPat, pat);
			_port->picSave->pushOpRect(kOpFillOval, r);
		}
	}
}

void Toolbox::FillPoly(const PolyHandle &poly, const Pattern &pat) {
	if (_port) {
		_port->fillPat = pat;
		_drawPoly(poly, pat, kPatCopy, false, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpPat(kOpFillPat, pat);
			_port->picSave->pushOpPoly(kOpFillPoly, poly);
		}
	}
}

void Toolbox::FillRect(const Common::Rect &r, const Pattern &pat) {
	if (_port) {
		_port->fillPat = pat;
		_drawRect(r, pat, kPatCopy, false, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpPat(kOpFillPat, pat);
			_port->picSave->pushOpRect(kOpFillRect, r);
		}
	}
}

void Toolbox::FillRoundRect(const Common::Rect &r, uint16 ovalWidth, uint16 ovalHeight, const Pattern &pat) {
	if (_port) {
		_port->fillPat = pat;
		_drawRoundRect(r, pat, kPatCopy, false, _port->fgColor, _port->bkColor, ovalWidth, ovalHeight);
		if (_port->picSave) {
			_port->picSave->pushOpPat(kOpFillPat, pat);
			_port->picSave->pushOpPoint(kOpOvSize, Common::Point(ovalWidth, ovalHeight));
			_port->picSave->pushOpRect(kOpFillRRect, r);
		}
	}
}

void Toolbox::FrameArc(const Common::Rect &r, int16 startAngle, int16 arcAngle) {
	warning("STUB: Toolbox::FrameArc");
}

void Toolbox::FrameOval(const Common::Rect &r) {
	if (_port) {
		_drawOval(r, _port->pnPat, _port->pnMode, true, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpRect(kOpFrameOval, r);
		}
	}
}

void Toolbox::FramePoly(const PolyHandle &poly) {
	if (_port) {
		_drawPoly(poly, _port->pnPat, _port->pnMode, true, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpPoly(kOpFramePoly, poly);
		}
	}
}

void Toolbox::FrameRect(const Common::Rect &r) {
	if (_port) {
		_drawRect(r, _port->pnPat, _port->pnMode, true, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpRect(kOpFrameRect, r);
		}
	}
}

void Toolbox::FrameRoundRect(const Common::Rect &r, uint16 ovalWidth, uint16 ovalHeight) {
	if (_port) {
		_drawRoundRect(r, _port->pnPat, _port->pnMode, true, _port->fgColor, _port->bkColor, ovalWidth, ovalHeight);
		if (_port->picSave) {
			_port->picSave->pushOpPoint(kOpOvSize, Common::Point(ovalWidth, ovalHeight));
			_port->picSave->pushOpRect(kOpFrameRRect, r);
		}
	}
}

void Toolbox::GetCPixel(int16 h, int16 v, RGBColor &cPix) {
	warning("STUB: Toolbox::GetCPixel");
}

Handle Toolbox::GetIcon(uint16 iconID) {
	Handle handle = this->GetResource(MKTAG('I', 'C', 'O', 'N'), iconID);
	if (handle) {
		return handle;
	} else {
		warning("Toolbox::GetIcon: failed to load ICON id %d", iconID);
	}

	return nullptr;
}

void Toolbox::GetPort(GrafPtr &port) {
	port = _port;
}

void Toolbox::GlobalToLocal(Common::Point &pt) {
	if (_port) {
		pt.x -= _port->portRect.left;
		pt.y -= _port->portRect.top;
	}
}

void Toolbox::HideCursor() {
	_cursorLevel--;
	g_engine->_wm.replaceCursor(Graphics::MacGUIConstants::kMacCursorOff);
}

void Toolbox::HidePen() {
	if (_port) {
		_port->pnVis--;
	}
}

void Toolbox::InitCursor() {
	_cursorLevel = 0;
	g_engine->_wm.replaceCursor(Graphics::MacGUIConstants::kMacCursorArrow);
	if (_cursor) {
		_cursor = nullptr;
	}
}

void Toolbox::InsetRect(Common::Rect &r, int16 dh, int16 dv) {
	r.left += dh;
	r.right -= dh;
	r.top += dv;
	r.bottom -= dv;
}

void Toolbox::InvertOval(const Common::Rect &r) {
	if (_port) {
		// set pattern to full black
		Pattern pat({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
		_drawOval(r, pat, kPatXor, false, g_engine->_wm._colorBlack, g_engine->_wm._colorWhite);
		if (_port->picSave) {
			_port->picSave->pushOpRect(kOpInvertOval, r);
		}
	}
}

void Toolbox::InvertPoly(const PolyHandle &poly) {
	if (_port) {
		// set pattern to full black
		Pattern pat({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
		_drawPoly(poly, pat, kPatXor, false, g_engine->_wm._colorBlack, g_engine->_wm._colorWhite);
		if (_port->picSave) {
			_port->picSave->pushOpPoly(kOpInvertPoly, poly);
		}
	}
}

void Toolbox::InvertRect(const Common::Rect &r) {
	if (_port) {
		// set pattern to full black
		Pattern pat({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
		_drawRect(r, pat, kPatXor, false, g_engine->_wm._colorBlack, g_engine->_wm._colorWhite);
		if (_port->picSave) {
			_port->picSave->pushOpRect(kOpInvertRect, r);
		}
	}
}

void Toolbox::InvertRoundRect(const Common::Rect &r, uint16 ovalWidth, uint16 ovalHeight) {
	if (_port) {
		// set pattern to full black
		Pattern pat({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
		_drawRoundRect(r, pat, kPatXor, false, g_engine->_wm._colorBlack, g_engine->_wm._colorWhite, ovalWidth, ovalHeight);
		if (_port->picSave) {
			_port->picSave->pushOpPoint(kOpOvSize, Common::Point(ovalWidth, ovalHeight));
			_port->picSave->pushOpRect(kOpInvertRRect, r);
		}
	}
}


void Toolbox::KillPoly(PolyHandle &poly) {
	// zero out the destination pointer
	poly = nullptr;
}

void Toolbox::Line(int16 dh, int16 dv) {
	if (_port) {
		LineTo(_port->pnLoc.x + dh, _port->pnLoc.y + dv);
	}
}

void Toolbox::LineTo(int16 h, int16 v) {
	if (_port && _port->pnVis == 0) {
		Common::Point dirVec(h - _port->pnLoc.x, v - _port->pnLoc.y);
		Common::Rect interRect(ABS(dirVec.x) + _port->pnSize.x, ABS(dirVec.y) + _port->pnSize.y);
		Common::Point startPos(dirVec.x < 0 ? -dirVec.x : 0, dirVec.y < 0 ? -dirVec.y : 0);
		Common::Point endPos(dirVec.x < 0 ? 0 : dirVec.x, dirVec.y < 0 ? 0 : dirVec.y);

		BitMap intermediate(new Graphics::ManagedSurface(interRect.width(), interRect.height()));
		BitMap mask(new Graphics::ManagedSurface(interRect.width(), interRect.height()));
		Common::Point destPos(
			dirVec.x < 0 ? (_port->pnLoc.x + dirVec.x) : _port->pnLoc.x,
			dirVec.y < 0 ? (_port->pnLoc.y + dirVec.y) : _port->pnLoc.y
		);
		// move to port coordinate space
		destPos.x += _port->portRect.left;
		destPos.y += _port->portRect.top;

		Graphics::MacPatterns macpat({_port->pnPat.data});

		Graphics::MacPlotData pd(intermediate.get(), mask.get(), &macpat, 1, destPos.x, destPos.y, _port->pnSize, _port->bkColor);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();
		pm.drawLine(startPos.x, startPos.y, endPos.x, endPos.y, _port->fgColor, &pd);

		Common::Rect dstRect = blitMono(intermediate, _port->portBits, mask, destPos, _port->pnMode);
		_addDirtyRect(_port->portBits, dstRect);

		if (debugChannelSet(5, kDebugGraphics)) {
			debugC(5, kDebugGraphics, "Toolbox::LineTo: startPos(%d, %d), endPos(%d, %d), dstRect (%d, %d) %dx%d, pattern %s, mode %d, fgColor %08x, bkColor %08x", _port->pnLoc.x, _port->pnLoc.y, h, v, dstRect.left, dstRect.top, dstRect.width(), dstRect.height(), _port->pnPat.format().c_str(), _port->pnMode, _port->fgColor, _port->bkColor);
		}

	}
	if (_port) {
		// update polygon if we're in openPoly mode
		if (_port->polySave) {
			if (_port->polySave->polyPoints.size() == 0) {
				_port->polySave->polyPoints.push_back(_port->pnLoc);
			}
			_port->polySave->polyPoints.push_back(Common::Point(h, v));
		}

		_port->pnLoc = Common::Point(h, v);
	}
}

void Toolbox::LocalToGlobal(Common::Point &pt) {
	if (_port) {
		pt.x += _port->portRect.left;
		pt.y += _port->portRect.top;
	}
}

void Toolbox::Move(int16 dh, int16 dv) {
	if (_port) {
		MoveTo(_port->portRect.width() + dh, _port->portRect.height() + dv);
	}
}

void Toolbox::MovePortTo(int16 leftGlobal, int16 topGlobal) {
	if (_port) {
		_port->portRect.left = leftGlobal;
		_port->portRect.top = topGlobal;
	}
}

void Toolbox::MoveTo(int16 h, int16 v) {
	if (_port) {
		_port->pnLoc = Common::Point(h, v);
	}
}

void Toolbox::OffsetRect(Common::Rect &r, int16 dh, int16 dv) {
	r.left += dh;
	r.right += dh;
	r.top += dv;
	r.bottom += dv;
}


PolyHandle Toolbox::OpenPoly() {
	HidePen();
	PolyHandle handle(new Polygon());
	if (_port) {
		_port->polySave = handle;
	}
	return handle;
}

void Toolbox::OpenPort(GrafPtr port) {
	// source: QuickDraw Routines I-163
	port->portBits = _defaultBits;
	port->portRect = Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT);
	port->visRgn = RgnHandle(new Region({ 1, Common::Rect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) }));
	port->fgColor = g_engine->_wm._colorBlack;
	port->bkColor = g_engine->_wm._colorWhite;
	_port = port;
	//port->clipRgn
}

void Toolbox::PaintOval(const Common::Rect &r) {
	if (_port) {
		_drawOval(r, _port->pnPat, _port->pnMode, false, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpRect(kOpPaintOval, r);
		}
	}
}

void Toolbox::PaintPoly(const PolyHandle &poly) {
	if (_port) {
		_drawPoly(poly, _port->pnPat, _port->pnMode, false, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpPoly(kOpPaintPoly, poly);
		}
	}
}

void Toolbox::PaintRect(const Common::Rect &r) {
	if (_port) {
		_drawRect(r, _port->pnPat, _port->pnMode, false, _port->fgColor, _port->bkColor);
		if (_port->picSave) {
			_port->picSave->pushOpRect(kOpPaintRect, r);
		}
	}
}

void Toolbox::PaintRoundRect(const Common::Rect &r, uint16 ovalWidth, uint16 ovalHeight) {
	if (_port) {
		_drawRoundRect(r, _port->pnPat, _port->pnMode, false, _port->fgColor, _port->bkColor, ovalWidth, ovalHeight);
		if (_port->picSave) {
			_port->picSave->pushOpPoint(kOpOvSize, Common::Point(ovalWidth, ovalHeight));
			_port->picSave->pushOpRect(kOpPaintRRect, r);
		}
	}
}

void Toolbox::PenMode(PatternMode mode) {
	if (_port) {
		_port->pnMode = mode;
		if (_port->picSave) {
			_port->picSave->pushOpU16(kOpPnMode, mode);
		}
	}
}

void Toolbox::PenNormal() {
	if (_port) {
		_port->pnSize = Common::Point(1, 1);
		_port->pnMode = kPatCopy;
		// Solid black for the pattern
		_port->pnPat = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
	}
}

void Toolbox::PenPat(const Pattern &pat) {
	if (_port) {
		_port->pnPat = pat;
		if (_port->picSave) {
			_port->picSave->pushOpPat(kOpPnPat, pat);
		}
	}
}

void Toolbox::PenSize(uint16 width, uint16 height) {
	if (_port) {
		_port->pnSize = Common::Point(width, height);
		if (_port->picSave) {
			_port->picSave->pushOpPoint(kOpPnPat, _port->pnSize);
		}
	}
}

void Toolbox::PlotIcon(const Common::Rect &theRect, const Handle &theIcon) {
	if (!theIcon) {
		warning("Toolbox::PlotIcon: empty handle");
		return;
	}
	if (_port) {
		PixMap pm;
		pm.bounds = Common::Rect(0, 0, 32, 32);
		pm.rowBytes = 4;
		pm._isBitMap = true;
		Common::MemoryReadStream stream(theIcon->data(), theIcon->size());
		BitMap result = readBitsRectMono(stream, pm, false);
		const BitMap intermediate(createRemappedSurface(result->surfacePtr(), nullptr, 0));
		CopyBits(intermediate, _port->portBits, result->getBounds(), theRect, kSrcCopy, nullptr);
	}
}

void Toolbox::PortSize(uint16 width, uint16 height) {
	if (_port) {
		_port->portRect.setWidth(width);
		_port->portRect.setHeight(height);
	}
}

bool Toolbox::PtInRect(const Common::Point &pt, const Common::Rect &r) {
	return r.contains(pt);
}

void Toolbox::SetCPixel(int16 h, int16 v, const RGBColor &cPix) {
	warning("STUB: Toolbox::SetCPixel");
}

void Toolbox::SetCursor(const Common::SharedPtr<Cursor> &crsr) {
	_cursor = crsr;
	if (_cursorLevel == 0)
		g_engine->_wm.replaceCursor(Graphics::MacGUIConstants::kMacCursorCustom, crsr.get());
}

void Toolbox::SetOrigin(int16 h, int16 v) {
	if (_port) {
		_port->portRect.left = h;
		_port->portRect.top = v;
		if (_port->picSave) {
			_port->picSave->pushOpPoint(kOpOrigin, Common::Point(h, v));
		}
	}
}

void Toolbox::SetPort(GrafPtr port) {
	_port = port;
}

void Toolbox::SetPortBits(BitMap &bm) {
	if (_port) {
		_port->portBits = bm;
	}
}

void Toolbox::SetRect(Common::Rect &r, int16 left, int16 top, int16 right, int16 bottom) {
	r.left = left;
	r.top = top;
	r.right = right;
	r.bottom = bottom;
}

void Toolbox::ShowCursor() {
	if (_cursorLevel < 0)
		_cursorLevel++;
	if (_cursorLevel == 0) {
		if (_cursor) {
			g_engine->_wm.replaceCursor(Graphics::MacGUIConstants::kMacCursorCustom, _cursor.get());
		} else {
			g_engine->_wm.replaceCursor(Graphics::MacGUIConstants::kMacCursorArrow);
		}
	}
}

void Toolbox::ShowPen() {
	if (_port) {
		_port->pnVis++;
	}
}


} // namespace Fool
