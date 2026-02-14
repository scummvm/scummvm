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
	Common::Rect clipSrcRect = srcRect;
	Common::Rect clipDstRect = dstRect;
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
		subsrc->scale(clipDstRect.width(), clipDstRect.height());
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
	if (_port->portBits == _defaultBits) {
		_defaultWindow->addDirtyRect(result);
		_defaultWindow->setDirty(true);
	}
}

void Toolbox::DrawMenuBar() {
	warning("STUB: Toolbox::DrawMenuBar");
}

void Toolbox::DrawPicture(PicHandle &myPicture, const Common::Rect &dstRect) {
	const Graphics::Surface *surface = myPicture->getSurface();
	const Graphics::Palette &palette = myPicture->getPalette();
	//const BitMap mask(nullptr);
	const BitMap mask(new Graphics::ManagedSurface());
	mask->copyFrom(*myPicture->getMask());
	const BitMap intermediate(createRemappedSurface(surface, palette.data(), palette.size()));
	_copyBits(intermediate, mask, _port->portBits, intermediate->getBounds(), dstRect, kSrcCopy, nullptr);
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

void Toolbox::EraseRect(const Common::Rect &r) {
	if (_port) {
		_drawRect(r, _port->bkPat, kPatCopy, false, _port->fgColor, _port->bkColor);
	}
}

void Toolbox::EraseRoundRect(const Common::Rect &r, int16 ovalWidth, int16 ovalHeight) {
	warning("STUB: Toolbox::EraseRoundRect");
}

void Toolbox::EndUpdate(WindowRecord &theWindow) {
	warning("STUB: Toolbox::EndUpdate");
}

void Toolbox::FillOval(const Common::Rect &r, const Pattern &pat) {
	warning("STUB: Toolbox::FillOval");
}

void Toolbox::_drawRect(const Common::Rect &r, const Pattern &pat, PatternMode mode, bool frame, uint32 fgColor, uint32 bkColor) {
	if (_port && _port->pnVis == 0) {
		BitMap intermediate(new Graphics::ManagedSurface(r.width(), r.height()));
		// special case because rect fills the entire surface
		BitMap mask(nullptr);
		if (frame) {
			mask = BitMap(new Graphics::ManagedSurface(r.width(), r.height()));
		}

		Common::Point destPos(r.left, r.top);
		Graphics::MacPatterns macpat({pat.data});

		Graphics::MacPlotData pd(intermediate.get(), mask.get(), &macpat, 1, destPos.x, destPos.y, _port->pnSize, bkColor);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();
		// For thicker outlines, the shape should be adjusted inward
		Common::Rect destRect = intermediate->getBounds();
		destRect.right -= _port->pnSize.x - 1;
		destRect.bottom -= _port->pnSize.y - 1;

		if (frame) {
			pm.drawRect(destRect, fgColor, &pd);
		} else {
			pm.drawFilledRect(destRect, fgColor, &pd);
		}
		Common::Rect dstRect = blitMono(intermediate, _port->portBits, mask, destPos, mode);

		// Dirty rects check
		if (_port->portBits == _defaultBits) {
			_defaultWindow->addDirtyRect(dstRect);
			_defaultWindow->setDirty(true);
		}
	}
}

void Toolbox::FillRect(const Common::Rect &r, const Pattern &pat) {
	if (_port) {
		_drawRect(r, pat, kPatCopy, false, _port->fgColor, _port->bkColor);
	}
}

void Toolbox::FrameArc(const Common::Rect &r, int16 startAngle, int16 arcAngle) {
	warning("STUB: Toolbox::FrameArc");
}

void Toolbox::FrameOval(const Common::Rect &r) {
	if (_port && _port->pnVis == 0) {

		BitMap intermediate(new Graphics::ManagedSurface(r.width(), r.height()));
		BitMap mask(new Graphics::ManagedSurface(r.width(), r.height()));
		Common::Point destPos(r.left, r.top);
		Graphics::MacPatterns macpat({_port->pnPat.data});

		Graphics::MacPlotData pd(intermediate.get(), mask.get(), &macpat, 1, destPos.x, destPos.y, _port->pnSize, _port->bkColor);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();
		pm.drawEllipse(0, 0, r.width(), r.height(), _port->fgColor, false, &pd);

		Common::Rect dstRect = blitMono(intermediate, _port->portBits, mask, destPos, _port->pnMode);
		if (_port->portBits == _defaultBits) {
			_defaultWindow->addDirtyRect(dstRect);
			_defaultWindow->setDirty(true);
		}
	}
}

void Toolbox::FrameRect(const Common::Rect &r) {
	if (_port) {
		_drawRect(r, _port->pnPat, _port->pnMode, true, _port->fgColor, _port->bkColor);
	}
}

void Toolbox::FrameRoundRect(const Common::Rect &r, int16 ovalWidth, int16 ovalHeight) {
	warning("STUB: Toolbox::FrameRoundRect");
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

PicHandle Toolbox::GetPicture(uint16 picID) {
	Handle handle = this->GetResource(MKTAG('P', 'I', 'C', 'T'), picID);
	if (handle) {
		Common::MemoryReadStream stream(handle->data(), handle->size());
		PicHandle decoder(new Image::PICTDecoder());
		if (decoder->loadStream(stream)) {
			_resPicts[decoder] = handle;
			return decoder;
		} else {
			warning("Toolbox::GetPicture: failed to load PICT id %d", picID);
		}
	}

	return nullptr;
}

void Toolbox::GetPort(GrafPtr &port) {
	port = _port;
}

void Toolbox::GlobalToLocal(Common::Point &pt) {
	warning("STUB: Toolbox::GlobalToLocal");
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
	r.left += dh;
	r.right -= dh;
	r.top += dv;
	r.bottom -= dv;
	if ((r.width() < 1) || (r.height() < 1)) {
		r = { 0, 0, 0, 0 };
	}
}

void Toolbox::InvertOval(const Common::Rect &r) {
	warning("STUB: Toolbox::InvertOval");
}

void Toolbox::InvertRect(const Common::Rect &r) {
	if (_port) {
		// set pattern to full black
		Pattern pat({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
		_drawRect(r, pat, kPatXor, false, g_engine->_wm._colorBlack, g_engine->_wm._colorWhite);
	}
}

void Toolbox::InvertRoundRect(const Common::Rect &r, int16 ovalWidth, int16 ovalHeight) {
	warning("STUB: Toolbox::InvertRoundRect");
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
		Common::Point destPos(dirVec.x < 0 ? h + dirVec.x : h - dirVec.x, dirVec.y < 0 ? v + dirVec.y : v - dirVec.y);
		Graphics::MacPatterns macpat({_port->pnPat.data});

		Graphics::MacPlotData pd(intermediate.get(), mask.get(), &macpat, 1, destPos.x, destPos.y, _port->pnSize, _port->bkColor);
		Graphics::Primitives &pm = g_engine->_wm.getDrawPrimitives();
		pm.drawLine(startPos.x, startPos.y, endPos.x, endPos.y, _port->fgColor, &pd);

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

void Toolbox::LocalToGlobal(Common::Point &pt) {
	warning("STUB: Toolbox::LocalToGlobal");
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
	_port = port;
	//port->clipRgn
}

void Toolbox::PaintOval(const Common::Rect &r) {
	warning("STUB: Toolbox::PaintOval");
}

void Toolbox::PaintPoly(PolyHandle poly) {
	warning("STUB: Toolbox::PaintPoly");
}

void Toolbox::PaintRect(const Common::Rect &r) {
	if (_port) {
		_drawRect(r, _port->pnPat, _port->pnMode, false, _port->fgColor, _port->bkColor);
	}
}

void Toolbox::PaintRoundRect(const Common::Rect &r, int16 ovalWidth, int16 ovalHeight) {
	warning("STUB: Toolbox::PaintRoundRect");
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
		// Solid black for the pattern
		_port->pnPat = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
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

bool Toolbox::PtInRect(const Common::Point &pt, const Common::Rect &r) {
	return r.contains(pt);
}

void Toolbox::SetCPixel(int16 h, int16 v, const RGBColor &cPix) {
	warning("STUB: Toolbox::SetCPixel");
}

void Toolbox::SetCursor(const Cursor &crsr) {
	warning("STUB: Toolbox::SetCursor");
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



} // namespace Fool
