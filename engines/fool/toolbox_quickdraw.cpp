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

#include "common/memstream.h"
#include "common/system.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/managed_surface.h"
#include "image/pict.h"

#include "fool/fool.h"
#include "fool/toolbox.h"
#include "fool/utils.h"

namespace Fool {

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

void Toolbox::CopyBits(const BitMap &srcBits, BitMap &dstBits, const Common::Rect &srcRect, const Common::Rect &dstRect, SourceMode mode, RgnHandle maskRgn) {
	if (!srcBits) {
		warning("Toolbox::CopyBits: empty srcBits handle");
		return;
	}

	if (!dstBits) {
		warning("Toolbox::CopyBits: empty dstBits handle");
		return;
	}
	if (maskRgn) {
		warning("Toolbox::CopyBits: maskRgn unimplemented");
	}
	// scale the source
	BitMap subsrc(new Graphics::ManagedSurface());
	subsrc->create(*srcBits, srcRect);
	if (srcRect.width() != dstRect.width() ||
			srcRect.height() != dstRect.height()) {
		subsrc->scale(dstRect.width(), dstRect.height());
	}
	BitMap mask(nullptr);
	Common::Rect result = blitMono(subsrc, dstBits, mask, Common::Point(dstRect.left, dstRect.top), mode);
	if (_port->portBits == _defaultBits) {
		_defaultWindow->addDirtyRect(result);
		_defaultWindow->setDirty(true);
	}
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
	// steps:
	// - create intermediate surface
	// - draw to intermediate surface using MacPlotData
	// - blit to target with blitMono
	// - add to target dirty rect list
	if (_port && _port->pnVis == 0) {
		BitMap intermediate(new Graphics::ManagedSurface(r.width(), r.height()));
		BitMap mask(nullptr);
		Graphics::MacPatterns macpat({pat.data});
		Graphics::MacPlotData pd(&(*intermediate), nullptr, &macpat, 1, 0, 0, _port->pnSize, _port->bkColor);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();
		pm.drawFilledRect(intermediate->getBounds(), _port->fgColor, &pd);
		Common::Point destPos(r.left, r.top);
		Common::Rect dstRect = blitMono(intermediate, _port->portBits, mask, destPos, _port->pnMode);
		if (_port->portBits == _defaultBits) {
			_defaultWindow->addDirtyRect(dstRect);
			_defaultWindow->setDirty(true);
		}

	}
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

PicHandle Toolbox::GetPicture(uint16 picID) {
	Handle handle = this->GetResource(MKTAG('P', 'I', 'C', 'T'), picID);
	if (handle) {
		Common::MemoryReadStream stream(handle->data(), handle->size());
		Image::PICTDecoder decoder;
		if (decoder.loadStream(stream)) {
			const Graphics::Surface *surface = decoder.getSurface();
			const Graphics::Palette &palette = decoder.getPalette();
			PicHandle result(createRemappedSurface(surface, palette.data(), palette.size()));
			_resPicts[result] = handle;
			return result;
		} else {
			warning("Toolbox::GetPicture: failed to load PICT id %d", picID);
		}
	}

	return nullptr;
}

void Toolbox::GetPort(GrafPtr &port) {
	port = _port;
}

void Toolbox::HideCursor() {
	_cursorLevel--;
	g_engine->_wm.replaceCursor(Graphics::MacGUIConstants::kMacCursorOff);
}

void Toolbox::InitCursor() {
	g_engine->_wm.replaceCursor(Graphics::MacGUIConstants::kMacCursorArrow);
	_cursorLevel = 0;
}

void Toolbox::InsetRect(Common::Rect &r, int16 dh, int16 dv) {
	warning("STUB: Toolbox::InsetRect");
}

void Toolbox::InvertOval(const Common::Rect &r) {
	warning("STUB: Toolbox::InvertOval");
}

void Toolbox::InvertRect(const Common::Rect &r) {
	if (_port && _port->pnVis == 0) {
		BitMap intermediate(new Graphics::ManagedSurface(r.width(), r.height()));
		BitMap mask(nullptr);
		Pattern pat({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
		Graphics::MacPatterns macpat({pat.data});
		Graphics::MacPlotData pd(&(*intermediate), nullptr, &macpat, 1, 0, 0, {1, 1}, g_engine->_wm._colorWhite);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();
		pm.drawFilledRect(intermediate->getBounds(), g_engine->_wm._colorBlack, &pd);
		Common::Point destPos(r.left, r.top);
		Common::Rect dstRect = blitMono(intermediate, _port->portBits, mask, destPos, kPatXor);
		if (_port->portBits == _defaultBits) {
			_defaultWindow->addDirtyRect(dstRect);
			_defaultWindow->setDirty(true);
		}


	}
	warning("STUB: Toolbox::InvertRect");
}

void Toolbox::KillPoly(PolyHandle poly) {
	warning("STUB: Toolbox::KillPoly");
}

void Toolbox::LineTo(int16 h, int16 v) {
	if (_port && _port->pnVis == 0) {
		Common::Point dirVec(h - _port->pnLoc.x, v - _port->pnLoc.y);
		Common::Rect interRect(ABS(dirVec.x) + _port->pnSize.x, ABS(dirVec.y) + _port->pnSize.y);
		Common::Point startPos(dirVec.x < 0 ? -dirVec.x : 0, dirVec.y < 0 ? -dirVec.y : 0);
		Common::Point endPos(dirVec.x < 0 ? 0 : dirVec.x, dirVec.y < 0 ? 0 : dirVec.y);

		BitMap intermediate(new Graphics::ManagedSurface(interRect.width(), interRect.height()));
		BitMap mask(new Graphics::ManagedSurface(interRect.width(), interRect.height()));
		Graphics::MacPatterns pat({_port->pnPat.data});

		Graphics::MacPlotData pd(&(*intermediate), &(*mask), &pat, 1, 0, 0, _port->pnSize, _port->bkColor);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();
		pm.drawLine(startPos.x, startPos.y, endPos.x, endPos.y, _port->fgColor, &pd);
		Common::Point destPos(dirVec.x < 0 ? h + dirVec.x : h - dirVec.x, dirVec.y < 0 ? v + dirVec.y : v - dirVec.y);

		Common::Rect dstRect = blitMono(intermediate, _port->portBits, mask, destPos, _port->pnMode);
		if (_port->portBits == _defaultBits) {
			_defaultWindow->addDirtyRect(dstRect);
			_defaultWindow->setDirty(true);
		}

	}
	if (_port) {
		_port->pnLoc = Common::Point(h, v);
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

PolyHandle Toolbox::OpenPoly() {
	warning("STUB: Toolbox::OpenPoly");
	return 0;
}

void Toolbox::OpenPort(GrafPtr port) {
	// source: QuickDraw Routines I-163
	//port->portBits = screenBits;
	//port->portRect = screenBits.bounds;
	port->portBits = _defaultBits;
	port->visRgn = RgnHandle(new Region({ 1, Common::Rect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) }));
	port->fgColor = g_engine->_wm._colorBlack;
	port->bkColor = g_engine->_wm._colorWhite;
	//port->clipRgn
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

void Toolbox::PenMode(PatternMode mode) {
	if (_port) {
		_port->pnMode = mode;
	}
}

void Toolbox::PenNormal() {
	if (_port) {
		_port->pnSize = Common::Point(1, 1);
		_port->pnMode = kPatCopy;
		_port->pnPat = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
	}
}

void Toolbox::PenPat(const Pattern &pat) {
	if (_port) {
		_port->pnPat = pat;
	}
}

void Toolbox::PenSize(uint16 width, uint16 height) {
	if (_port) {
		_port->pnSize = Common::Point(width, height);
	}
}

void Toolbox::PortSize(uint16 width, uint16 height) {
	if (_port) {
		_port->portRect.setWidth(width);
		_port->portRect.setHeight(height);
	}
}

void Toolbox::SetCPixel(int16 h, int16 v, const RGBColor &cPix) {
	warning("STUB: Toolbox::SetCPixel");
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
	if (_cursorLevel == 0)
		g_engine->_wm.replaceCursor(Graphics::MacGUIConstants::kMacCursorArrow);
}


uint16 Toolbox::StringWidth(const Common::String &s) {
	warning("STUB: Toolbox::StringWidth");
	return 0;
}


} // namespace Fool
