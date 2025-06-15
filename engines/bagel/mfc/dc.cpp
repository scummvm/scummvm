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
#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/win_hand.h"
#include "bagel/mfc/gfx/blitter.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNCREATE(CDC, CObject)

CDC *CDC::FromHandle(HDC hDC) {
	CHandleMap<CDC> *pMap = AfxGetApp()->afxMapHDC(TRUE);
	assert(pMap != nullptr);

	CDC *pObject = pMap->FromHandle(hDC);
	assert(pObject == nullptr || pObject->m_hDC == hDC);
	return pObject;
}

BOOL CDC::CreateDC(LPCSTR lpszDriverName, LPCSTR lpszDeviceName,
                   LPCSTR lpszOutput, const void *lpInitData) {
	error("TODO: CDC::CreateDC");
}

BOOL CDC::CreateCompatibleDC(CDC *pDC) {
	CDC::Impl *dc = new CDC::Impl();
	dc->_format = pDC->impl()->getFormat();
	m_hDC = dc;

	return true;
}

BOOL CDC::DeleteDC() {
	CDC::Impl *dc = static_cast<CDC::Impl *>(m_hDC);
	delete dc;
	m_hDC = nullptr;
	return true;
}

BOOL CDC::Attach(HDC hDC) {
	m_hDC = hDC;
	return true;
}

void CDC::Detach() {
	error("TODO: CDC::Detach");
}

int CDC::SetStretchBltMode(int nStretchMode) {
	error("TODO: CDC::SetStretchBltMode");
}

int CDC::GetDeviceCaps(int nIndex) const {
	const Graphics::PixelFormat format =
	    g_system->getScreenFormat();

	switch (nIndex) {
	case HORZRES:
		return g_system->getWidth();
	case VERTRES:
		return g_system->getHeight();
	case BITSPIXEL:
		return format.bytesPerPixel * 8;
	case RASTERCAPS:
		return (format.bytesPerPixel == 1 ? RC_PALETTE : 0) |
		       RC_BITBLT |
		       RC_BITMAP64 |
		       RC_DI_BITMAP |
		       RC_DIBTODEV |
		       RC_PALETTE |
		       RC_STRETCHBLT;

	default:
		break;
	}

	error("TODO: CDC::GetDeviceCaps");
	return 0;
}

int CDC::GetMapMode() const {
	error("TODO: CDC::GetMapMode");
}

CPoint CDC::GetViewportOrg() const {
	error("TODO: CDC::GetViewportOrg");
}

int CDC::SetMapMode(int nMapMode) {
	error("TODO: CDC::SetMapMode");
}

CPoint CDC::SetViewportOrg(int x, int y) {
	error("TODO: CDC::SetViewportOrg");
}

CPoint CDC::SetViewportOrg(POINT point) {
	error("TODO: CDC::SetViewportOrg");
}

CPoint CDC::OffsetViewportOrg(int nWidth, int nHeight) {
	error("TODO: CDC::OffsetViewportOrg");
}

int CDC::GetClipBox(LPRECT lpRect) const {
	error("TODO: CDC::GetClipBox");
}

BOOL CDC::PtVisible(int x, int y) const {
	error("TODO: CDC::PtVisible");
}

BOOL CDC::PtVisible(POINT point) const {
	error("TODO: CDC::PtVisible");
}

BOOL CDC::RectVisible(LPCRECT lpRect) const {
	error("TODO: CDC::RectVisible");
}

int CDC::SelectClipRgn(CRgn *pRgn) {
	error("TODO: CDC::SelectClipRgn");
}

int CDC::ExcludeClipRect(int x1, int y1, int x2, int y2) {
	error("TODO: CDC::ExcludeClipRect");
}

int CDC::ExcludeClipRect(LPCRECT lpRect) {
	error("TODO: CDC::ExcludeClipRect");
}

int CDC::ExcludeUpdateRgn(CWnd *pWnd) {
	error("TODO: CDC::ExcludeUpdateRgn");
}

int CDC::IntersectClipRect(int x1, int y1, int x2, int y2) {
	error("TODO: CDC::IntersectClipRect");
}

int CDC::IntersectClipRect(LPCRECT lpRect) {
	error("TODO: CDC::IntersectClipRect");
}

int CDC::OffsetClipRgn(int x, int y) {
	error("TODO: CDC::OffsetClipRgn");
}

int CDC::OffsetClipRgn(SIZE size) {
	error("TODO: CDC::OffsetClipRgn");
}

int CDC::SelectClipRgn(CRgn *pRgn, int nMode) {
	error("TODO: CDC::SelectClipRgn");
}

int CDC::SetROP2(int nDrawMode) {
	error("TODO: CDC::SetROP2");
}

BOOL CDC::DPtoLP(LPPOINT lpPoints, int nCount) {
	error("TODO: CDC::DPtoLP");
}

BOOL CDC::DPtoLP(RECT *lpRect) {
	error("TODO: CDC::DPtoLP");
}

BOOL CDC::LPtoDP(RECT *lpRect) {
	error("TODO: CDC::LPtoDP");
}

BOOL CDC::BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
        int xSrc, int ySrc, DWORD dwRop) {
	impl()->bitBlt(x, y, nWidth, nHeight, pSrcDC, xSrc, ySrc, dwRop);
	return true;
}

BOOL CDC::StretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
        int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop) {
	impl()->stretchBlt(x, y, nWidth, nHeight, pSrcDC,
		xSrc, ySrc, nSrcWidth, nSrcHeight, dwRop);
	return true;
}

void CDC::Ellipse(LPCRECT lpRect) {
	error("TODO: CDC::Ellipse");
}

void CDC::Ellipse(int x1, int y1, int x2, int y2) {
	error("TODO: CDC::Ellipse");
}

void CDC::FrameRect(LPCRECT lpRect, CBrush *pBrush) {
	error("TODO: CDC::FrameRect");
}

void CDC::FillRect(LPCRECT lpRect, CBrush *pBrush) {
	CBrush::Impl *brush = static_cast<CBrush::Impl *>(pBrush->m_hObject);
	assert(brush->_type == HS_HORIZONTAL ||
	       brush->_type == HS_VERTICAL);

	FillSolidRect(lpRect, brush->getColor());
}

void CDC::FillSolidRect(LPCRECT lpRect, COLORREF color) {
	auto *surf = impl();
	surf->fillRect(*lpRect, color);
}

BOOL CDC::FloodFill(int x, int y, COLORREF crColor) {
	error("TODO: CDC::FloodFill");
}

BOOL CDC::FloodFill(int x, int y, COLORREF crColor,
                    UINT nFillType) {
	error("TODO: CDC::FloodFill");
}

void CDC::Rectangle(LPCRECT lpRect) {
	error("TODO: CDC::Rectangle");
}

void CDC::Rectangle(int x1, int y1, int x2, int y2) {
	error("TODO: CDC::Rectangle");
}

BOOL CDC::Pie(int x1, int y1, int x2, int y2,
              int x3, int y3, int x4, int y4) {
	error("TODO: CDC::Pie");
}

BOOL CDC::DrawEdge(LPRECT lpRect, UINT nEdge, UINT nFlags) {
	CRect rect = *lpRect;

	// Determine edge colors
	COLORREF clrTL = GetSysColor(COLOR_3DHIGHLIGHT); // Top-left
	COLORREF clrBR = GetSysColor(COLOR_3DSHADOW);    // Bottom-right

	// Adjust for EDGE_SUNKEN
	if (nEdge == EDGE_SUNKEN || nEdge == EDGE_ETCHED)
		SWAP(clrTL, clrBR);

	// Create pens
	CPen penTL(PS_SOLID, 1, clrTL);
	CPen penBR(PS_SOLID, 1, clrBR);
	CPen *pOldPen = SelectObject(&penTL);

	// Draw top and left
	if (nFlags & BF_TOP) {
		MoveTo(rect.left, rect.top);
		LineTo(rect.right - 1, rect.top);
	}
	if (nFlags & BF_LEFT) {
		MoveTo(rect.left, rect.top);
		LineTo(rect.left, rect.bottom - 1);
	}

	// Draw bottom and right
	SelectObject(&penBR);

	if (nFlags & BF_BOTTOM) {
		MoveTo(rect.left, rect.bottom - 1);
		LineTo(rect.right, rect.bottom - 1);
	}
	if (nFlags & BF_RIGHT) {
		MoveTo(rect.right - 1, rect.top);
		LineTo(rect.right - 1, rect.bottom);
	}

	// Optionally fill the middle area
	if (nFlags & BF_MIDDLE) {
		COLORREF fill = GetSysColor(COLOR_BTNFACE);
		CBrush brush(fill);
		CRect inner = rect;
		inner.DeflateRect(1, 1);
		FillRect(&inner, &brush);
	}

	// Optionally modify the rect (like real DrawEdge)
	if (!(nFlags & BF_ADJUST))
		lpRect->left += (nFlags & BF_LEFT) ? 1 : 0,
		lpRect->top += (nFlags & BF_TOP) ? 1 : 0,
		lpRect->right -= (nFlags & BF_RIGHT) ? 1 : 0,
		lpRect->bottom -= (nFlags & BF_BOTTOM) ? 1 : 0;

	// Restore old pen
	SelectObject(pOldPen);

	return TRUE;
}

BOOL CDC::Pie(LPCRECT lpRect, const POINT &ptStart, const POINT &ptEnd) {
	error("TODO: CDC::Pie");
}

BOOL CDC::FrameRgn(CRgn *pRgn, CBrush *pBrush,
                   int nWidth, int nHeight) {
	error("TODO: CDC::FrameRgn");
}

void CDC::MoveTo(int x, int y) {
	impl()->moveTo(x, y);
}

void CDC::LineTo(int x, int y) {
	impl()->lineTo(x, y);
}

COLORREF CDC::GetPixel(int x, int y) const {
	error("TODO: CDC::GetPixel");
}

COLORREF CDC::GetPixel(const POINT &point) const {
	error("TODO: CDC::GetPixel");
}

CGdiObject *CDC::SelectStockObject(int nIndex) {
	error("TODO: CDC::SelectStockObject");
}

CPen *CDC::SelectObject(CPen *pPen) {
	HPEN hOld = impl()->Attach(pPen->m_hObject);
	return (CPen *)CPen::FromHandle(hOld);
}

CBrush *CDC::SelectObject(CBrush *pBrush) {
	HBRUSH hOld = impl()->Attach(pBrush->m_hObject);
	return (CBrush *)CBrush::FromHandle(hOld);
}

CFont *CDC::SelectObject(CFont *pFont) {
	HFONT hOld = impl()->Attach(pFont->m_hObject);
	return (CFont *)CFont::FromHandle(hOld);
}

CBitmap *CDC::SelectObject(CBitmap *pBitmap) {
	HBITMAP hOld = impl()->Attach(pBitmap->m_hObject);
	return (CBitmap *)CBitmap::FromHandle(hOld);
}

int CDC::SelectObject(CRgn *pRgn) {
	error("TODO: CDC::SelectObject");
}

CGdiObject *CDC::SelectObject(CGdiObject *pObject) {
	HGDIOBJ hOld = impl()->Attach(pObject->m_hObject);
	return CGdiObject::FromHandle(hOld);
}

CPalette *CDC::SelectPalette(CPalette *pPalette, BOOL bForceBackground) {
	HPALETTE hOld = impl()->selectPalette(!pPalette ? nullptr :
		pPalette->m_hObject);
	return (CPalette *)CGdiObject::FromHandle(hOld);
}

COLORREF CDC::GetNearestColor(COLORREF crColor) const {
	return impl()->GetNearestColor(crColor);
}

UINT CDC::RealizePalette() {
	return impl()->realizePalette();
}

void CDC::UpdateColors() {
	error("TODO: CDC::UpdateColors");
}

COLORREF CDC::SetBkColor(COLORREF crColor) {
	return impl()->setBkColor(crColor);
}

int CDC::SetBkMode(int nBkMode) {
	return impl()->setBkMode(nBkMode);
}

COLORREF CDC::SetTextColor(COLORREF crColor) {
	return impl()->setTextColor(crColor);
}

BOOL CDC::TextOut(int x, int y, LPCSTR lpszString, int nCount) {
	return impl()->textOut(x, y, lpszString, nCount);
}

BOOL CDC::TextOut(int x, int y, const CString &str) {
	return impl()->textOut(x, y, str);
}

BOOL CDC::ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
        LPCSTR lpszString, UINT nCount, LPINT lpDxWidths) {
	return impl()->extTextOut(x, y, nOptions, lpRect, lpszString, nCount, lpDxWidths);
}

BOOL CDC::ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
        const CString &str, LPINT lpDxWidths) {
	return impl()->extTextOut(x, y, nOptions, lpRect, str, lpDxWidths);
}

CSize CDC::TabbedTextOut(int x, int y, LPCSTR lpszString, int nCount,
        int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) {
	return impl()->tabbedTextOut(x, y, lpszString, nCount,
		nTabPositions, lpnTabStopPositions, nTabOrigin);
}

CSize CDC::TabbedTextOut(int x, int y, const CString &str,
        int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) {
	return impl()->tabbedTextOut(x, y, str, nTabPositions,
		lpnTabStopPositions, nTabOrigin);
}

int CDC::DrawText(LPCSTR lpszString, int nCount,
        LPRECT lpRect, UINT nFormat) {
	return impl()->drawText(lpszString, nCount, lpRect, nFormat);
}

int CDC::DrawText(const CString &str, LPRECT lpRect, UINT nFormat) {
	return impl()->drawText(str, lpRect, nFormat);
}

CSize CDC::GetTextExtent(LPCSTR lpszString, int nCount) const {
	return impl()->getTextExtent(lpszString, nCount);
}

CSize CDC::GetTextExtent(const CString &str) const {
	return impl()->getTextExtent(str);
}

CSize CDC::GetOutputTextExtent(LPCSTR lpszString, int nCount) const {
	return impl()->getOutputTextExtent(lpszString, nCount);
}

CSize CDC::GetOutputTextExtent(const CString &str) const {
	return impl()->getOutputTextExtent(str);
}

CSize CDC::GetTabbedTextExtent(LPCSTR lpszString, int nCount,
        int nTabPositions, LPINT lpnTabStopPositions) const {
	return impl()->getTabbedTextExtent(lpszString, nCount,
		nTabPositions, lpnTabStopPositions);
}

CSize CDC::GetTabbedTextExtent(const CString &str,
        int nTabPositions, LPINT lpnTabStopPositions) const {
	return impl()->getTabbedTextExtent(str, nTabPositions,
		lpnTabStopPositions);
}

CSize CDC::GetOutputTabbedTextExtent(LPCSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions) const {
	return impl()->getOutputTabbedTextExtent(lpszString, nCount,
		nTabPositions, lpnTabStopPositions);
}

CSize CDC::GetOutputTabbedTextExtent(const CString &str,
        int nTabPositions, LPINT lpnTabStopPositions) const {
	return impl()->getOutputTabbedTextExtent(str,
		nTabPositions, lpnTabStopPositions);
}

BOOL CDC::GrayString(CBrush *pBrush,
        BOOL(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
        int nCount, int x, int y, int nWidth, int nHeight) {
	return impl()->grayString(pBrush, lpfnOutput, lpData,
		nCount, x, y, nWidth, nHeight);
}

UINT CDC::GetTextAlign() const {
	return impl()->getTextAlign();
}

UINT CDC::SetTextAlign(UINT nFlags) {
	return impl()->setTextAlign(nFlags);
}

BOOL CDC::GetTextMetrics(LPTEXTMETRIC lpMetrics) const {
	return impl()->getTextMetrics(lpMetrics);
}

/*--------------------------------------------*/

CDC::Impl::Impl() {
	// By default the _bitmap will point to
	// this dummy 1x1 bitmap
	_format = Graphics::PixelFormat::createFormatCLUT8();
	_bitmap1x1.create(1, 1, _format);

	// Set up the system font as default
	_font = AfxGetApp()->getDefaultFont();

	// Default pen
	_pen = AfxGetApp()->_defaultPen.m_hObject;
}

CDC::Impl::Impl(HDC srcDc) {
	const CDC::Impl *src = (CDC::Impl *)srcDc;

	// By default the _bitmap will point to
	// this dummy 1x1 bitmap
	_format = Graphics::PixelFormat::createFormatCLUT8();
	_bitmap1x1.create(1, 1, _format);

	_font = src->_font;
	_format = src->_format;
	_pen = src->_pen;
}

HGDIOBJ CDC::Impl::Attach(HGDIOBJ gdiObj) {
	CGdiObjectImpl *obj = (CGdiObjectImpl *)gdiObj;

	CBitmap::Impl *bitmap = dynamic_cast<CBitmap::Impl *>(obj);
	if (bitmap) {
		HBITMAP result = _bitmap;
		_bitmap = bitmap;

		_format = bitmap->format;
		return result;
	}

	CFont::Impl *font = dynamic_cast<CFont::Impl *>(obj);
	if (font) {
		HFONT result = _font;
		_font = font;
		return result;
	}

	CPen::Impl *pen = dynamic_cast<CPen::Impl *>(obj);
	if (pen) {
		HPEN result = _pen;
		_pen = pen;
		return result;
	}
#if 0
	HBRUSH brush = dynamic_cast<HBRUSH>(gdiObj);
#endif
	error("Unsupported gdi object");
	return nullptr;
}

Graphics::ManagedSurface *CDC::Impl::getSurface() const {
	assert(_bitmap);
	return static_cast<CBitmap::Impl *>(_bitmap);
}

const Graphics::PixelFormat &CDC::Impl::getFormat() const {
	return _bitmap ? getSurface()->format : _format;
}

HPALETTE CDC::Impl::selectPalette(HPALETTE pal) {
	HPALETTE oldPal = _palette;
	_palette = pal;

	CBitmap::Impl *bitmap = (CBitmap::Impl *)_bitmap;

	if (pal) {
		auto *newPal = static_cast<CPalette::Impl *>(pal);
		if (bitmap)
			bitmap->setPalette(newPal->data(), 0, newPal->size());
	} else {
		if (bitmap)
			bitmap->clearPalette();
	}

	return oldPal;
}

CPalette *CDC::Impl::selectPalette(CPalette *pal) {
	CPalette *oldPal = _cPalette;
	_cPalette = pal;
	selectPalette((HPALETTE)_cPalette->m_hObject);
	return oldPal;
}


UINT CDC::Impl::realizePalette() {
	const auto *pal = static_cast<const CPalette::Impl *>(_palette);
	AfxGetApp()->setPalette(*pal);
	return 256;
}

COLORREF CDC::Impl::GetNearestColor(COLORREF crColor) const {
	if (crColor <= 255)
		return crColor;

	const auto *pal = static_cast<const CPalette::Impl *>(_palette);

	if (pal)
		return pal->findBestColor(
			GetRValue(crColor),
			GetGValue(crColor),
			GetBValue(crColor));

	return AfxGetApp()->getColor(crColor);
}

void CDC::Impl::fillRect(const Common::Rect &r, COLORREF crColor) {
	assert(_bitmap);
	static_cast<CBitmap::Impl *>(_bitmap)->fillRect(r,
		GetNearestColor(crColor));
}

void CDC::Impl::bitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
	int xSrc, int ySrc, DWORD dwRop) {
	Graphics::ManagedSurface *src = pSrcDC->impl()->getSurface();
	Graphics::ManagedSurface *dest = getSurface();
	const Common::Rect srcRect(xSrc, ySrc, xSrc + nWidth, ySrc + nHeight);
	const Common::Point destPos(x, y);

	if (dwRop == SRCCOPY) {
		dest->blitFrom(*src, srcRect, destPos);
	} else {
		Gfx::blit(src, dest, srcRect, destPos, dwRop);
	}
}

void CDC::Impl::stretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
	int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop) {
	Graphics::ManagedSurface *src = pSrcDC->impl()->getSurface();
	Graphics::ManagedSurface *dest = getSurface();
	const Common::Rect srcRect(xSrc, ySrc, xSrc + nSrcWidth, ySrc + nSrcHeight);
	const Common::Rect destRect(x, y, x + nWidth, y + nHeight);

	Gfx::stretchBlit(src, dest, srcRect, destRect, dwRop);
}

void CDC::Impl::moveTo(int x, int y) {
	_linePos.x = x;
	_linePos.y = y;
}

void CDC::Impl::lineTo(int x, int y) {
	Graphics::ManagedSurface *dest = getSurface();
	uint color = getPenColor();

	dest->drawLine(_linePos.x, _linePos.y, x, y, color);
	_linePos.x = x;
	_linePos.y = y;
}

uint CDC::Impl::getPenColor() const {
	CPen::Impl *pen = (CPen::Impl *)_pen;
	assert(pen->_penStyle == PS_SOLID);

	return GetNearestColor(pen->_color);
}

COLORREF CDC::Impl::setBkColor(COLORREF crColor) {
	COLORREF oldColor = _bkColor;
	_bkColor = crColor;
	return oldColor;
}

int CDC::Impl::setBkMode(int nBkMode) {
	int oldMode = _bkMode;
	_bkMode = nBkMode;
	return oldMode;
}

COLORREF CDC::Impl::setTextColor(COLORREF crColor) {
	COLORREF oldColor = _textColor;
	_textColor = crColor;
	return oldColor;
}

BOOL CDC::Impl::textOut(int x, int y, LPCSTR lpszString, int nCount) {
	error("TODO");
}

BOOL CDC::Impl::textOut(int x, int y, const CString &str) {
	error("TODO");
}

BOOL CDC::Impl::extTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
	LPCSTR lpszString, UINT nCount, LPINT lpDxWidths) {
	error("TODO");
}

BOOL CDC::Impl::extTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
	const CString &str, LPINT lpDxWidths) {
	error("TODO");
}

CSize CDC::Impl::tabbedTextOut(int x, int y, LPCSTR lpszString, int nCount,
	int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) {
	error("TODO");
}

CSize CDC::Impl::tabbedTextOut(int x, int y, const CString &str,
	int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) {
	error("TODO");
}

int CDC::Impl::drawText(LPCSTR lpszString, int nCount,
		LPRECT lpRect, UINT nFormat) {
	return drawText(CString(lpszString, nCount),
		lpRect, nFormat);
}

int CDC::Impl::drawText(const CString &str, LPRECT lpRect, UINT nFormat) {
	Graphics::Font *font = *(CFont::Impl *)_font;
	Graphics::ManagedSurface *dest = getSurface();
	const int maxWidth = lpRect->right - lpRect->left;
	Common::Rect textRect = *lpRect;
	Common::StringArray lines;
	uint textCol = GetNearestColor(_textColor);

	if (nFormat & DT_SINGLELINE) {
		lines.push_back(str);
	} else {
		// We don't currently support prefix & characters
		// on potentially multi-line text. If this is ever
		// needed, we'll have to basically re-write the
		// wordWrapText function to handle them.
		assert((nFormat & DT_NOPREFIX) || !str.contains('&'));

		// Perform word wrapping of the text as necessary
		Common::String temp = str;
		font->wordWrapText(temp, maxWidth, lines);
	}

	// Handle vertical alignment
	const int linesHeight = lines.size() * font->getFontHeight();

	if (nFormat & DT_BOTTOM) {
		textRect.moveTo(textRect.left,
			MAX<int16>(lpRect->top, textRect.bottom - linesHeight));
	}
	if (nFormat & DT_VCENTER) {
		textRect.moveTo(textRect.left, MAX<int16>(lpRect->top,
			lpRect->top + ((lpRect->bottom - lpRect->top) -
				linesHeight) / 2));
	}

	// Iterate through the lines
	for (const Common::String &line : lines) {
		// Constrain within passed rect
		if (textRect.top >= lpRect->bottom)
			break;

		Common::String tempLine = line;
		int idx;
		while ((idx = tempLine.findFirstOf('&')) != Common::String::npos)
			tempLine.deleteChar(idx);

		const int lineWidth = font->getStringWidth(tempLine);

		// Form sub-rect for the single line
		Common::Rect lineRect(textRect.left, textRect.top,
			textRect.right, textRect.top + font->getFontHeight());

		// Handle horizontal alignment
		if (nFormat & DT_RIGHT) {
			textRect.moveTo(MAX<int16>(textRect.left,
				textRect.right - lineWidth),
				textRect.top);
		}
		if (nFormat & DT_CENTER) {
			textRect.moveTo(MAX<int16>(textRect.left,
				textRect.left + (textRect.width() - lineWidth) / 2),
				textRect.top);
		}

		// If the background is opaque, clear it
		if (_bkMode == OPAQUE)
			fillRect(textRect, _bkColor);

		// Write the actual text. This is slightly
		// complicated to detect '&' characters when
		// DT_NOPREFIX isn't set
		Common::String fragment;
		tempLine = line;
		while (!tempLine.empty()) {
			if (!(nFormat & DT_NOPREFIX) && tempLine.firstChar() == '&') {
				tempLine.deleteChar(0);

				// Draw an underline
				const int x1 = textRect.left;
				const int x2 = x1 + font->getCharWidth(tempLine.firstChar());
				const int y = textRect.top + font->getFontHeight() - 3;

				dest->hLine(x1, y, x2, textCol);
			}

			idx = (nFormat & DT_NOPREFIX) ? Common::String::npos :
				tempLine.findFirstOf('&');
			if (idx == Common::String::npos) {
				fragment = tempLine;
				tempLine.clear();
			} else {
				fragment = Common::String(tempLine.c_str(), tempLine.c_str() + idx);
				tempLine = Common::String(tempLine.c_str() + fragment.size());
			}

			font->drawString(dest, fragment, textRect.left, textRect.top,
				textRect.width(), textCol);
			textRect.left += font->getStringWidth(fragment);
		}

		// Move to next line
		textRect.top += font->getFontHeight();
	}

	return lines.size() * font->getFontHeight();
}

CSize CDC::Impl::getTextExtent(LPCSTR lpszString, int nCount) const {
	error("TODO");
}

CSize CDC::Impl::getTextExtent(const CString &str) const {
	error("TODO");
}

CSize CDC::Impl::getOutputTextExtent(LPCSTR lpszString, int nCount) const {
	error("TODO");
}

CSize CDC::Impl::getOutputTextExtent(const CString &str) const {
	error("TODO");
}

CSize CDC::Impl::getTabbedTextExtent(LPCSTR lpszString, int nCount,
	int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO");
}

CSize CDC::Impl::getTabbedTextExtent(const CString &str,
	int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO");
}

CSize CDC::Impl::getOutputTabbedTextExtent(LPCSTR lpszString, int nCount,
	int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO");
}

CSize CDC::Impl::getOutputTabbedTextExtent(const CString &str,
	int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO");
}

BOOL CDC::Impl::grayString(CBrush *pBrush,
	BOOL(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
	int nCount, int x, int y, int nWidth, int nHeight) {
	error("TODO");
}

UINT CDC::Impl::getTextAlign() const {
	error("TODO");
}

UINT CDC::Impl::setTextAlign(UINT nFlags) {
	error("TODO");
}

BOOL CDC::Impl::getTextMetrics(LPTEXTMETRIC lpMetrics) const {
	TEXTMETRIC &tm = *lpMetrics;
	Gfx::Font *font = *(CFont::Impl *)_font;

	memset(&tm, 0, sizeof(TEXTMETRIC)); // Initialize to zero

	tm.tmDescent = 0;
	tm.tmInternalLeading = 0;
	tm.tmExternalLeading = 0;

	tm.tmAveCharWidth = font->getCharWidth();
	if (!tm.tmAveCharWidth) {
		tm.tmAveCharWidth = font->getStringWidth(
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") / 52 + 1;
	}

	tm.tmHeight = font->getCharHeight();
	if (!tm.tmHeight)
		tm.tmHeight = font->getFontHeight();
	tm.tmAscent = tm.tmHeight;

	tm.tmMaxCharWidth = font->getMaxCharWidth();
	tm.tmWeight = FW_NORMAL;
	tm.tmOverhang = 0;

	tm.tmDigitizedAspectX = 1;
	tm.tmDigitizedAspectY = 1;

	const char first = (char)32;
	const char last = (char)127;
	tm.tmFirstChar = first;
	tm.tmLastChar = last;
	tm.tmDefaultChar = '?'; // Pick a fallback character
	tm.tmBreakChar = ' ';   // Typically space is used for breaking

	// Assume fixed-pitch if all characters have same width
	bool fixedPitch = true;
	int firstWidth = font->getCharWidth(first);
	for (int c = first + 1; c <= last; ++c) {
		if (font->getCharWidth(c) != firstWidth) {
			fixedPitch = false;
			break;
		}
	}

	tm.tmPitchAndFamily = (fixedPitch ? TMPF_FIXED_PITCH : 0);
	tm.tmCharSet = ANSI_CHARSET;

	return true;
}

} // namespace MFC
} // namespace Bagel
