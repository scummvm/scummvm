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
	error("TODO: CDC::MoveTo");
}

void CDC::LineTo(int x, int y) {
	error("TODO: CDC::LineTo");
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
	error("TODO: CDC::SetBkColor");
}

int CDC::SetBkMode(int nBkMode) {
	error("TODO: CDC::SetBkMode");
}

COLORREF CDC::SetTextColor(COLORREF crColor) {
	error("TODO: CDC::SetTextColor");
}

BOOL CDC::TextOut(int x, int y, LPCSTR lpszString, int nCount) {
	error("TODO: CDC::TextOut");
}

BOOL CDC::TextOut(int x, int y, const CString &str) {
	error("TODO: CDC::TextOut");
}

BOOL CDC::ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
                     LPCSTR lpszString, UINT nCount, LPINT lpDxWidths) {
	error("TODO: CDC::ExtTextOut");
}

BOOL CDC::ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
                     const CString &str, LPINT lpDxWidths) {
	error("TODO: CDC::ExtTextOut");
}

CSize CDC::TabbedTextOut(int x, int y, LPCSTR lpszString, int nCount,
                         int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) {
	error("TODO: CDC::TabbedTextOut");
}

CSize CDC::TabbedTextOut(int x, int y, const CString &str,
                         int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) {
	error("TODO: CDC::TabbedTextOut");
}

int CDC::DrawText(LPCSTR lpszString, int nCount,
                  LPRECT lpRect, UINT nFormat) {
	error("TODO: CDC::DrawText");
}

int CDC::DrawText(const CString &str, LPRECT lpRect, UINT nFormat) {
	error("TODO: CDC::DrawText");
}

CSize CDC::GetTextExtent(LPCSTR lpszString, int nCount) const {
	error("TODO: CDC::GetTextExtent");
}

CSize CDC::GetTextExtent(const CString &str) const {
	error("TODO: CDC::GetTextExtent");
}

CSize CDC::GetOutputTextExtent(LPCSTR lpszString, int nCount) const {
	error("TODO: CDC::GetOutputTextExtent");
}

CSize CDC::GetOutputTextExtent(const CString &str) const {
	error("TODO: CDC::GetOutputTextExtent");
}

CSize CDC::GetTabbedTextExtent(LPCSTR lpszString, int nCount,
                               int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO: CDC::GetTabbedTextExtent");
}

CSize CDC::GetTabbedTextExtent(const CString &str,
                               int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO: CDC::GetTabbedTextExtent");
}

CSize CDC::GetOutputTabbedTextExtent(LPCSTR lpszString, int nCount,
                                     int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO: CDC::GetOutputTabbedTextExtent");
}

CSize CDC::GetOutputTabbedTextExtent(const CString &str,
                                     int nTabPositions, LPINT lpnTabStopPositions) const {
	error("TODO: CDC::GetOutputTabbedTextExtent");
}

BOOL CDC::GrayString(CBrush *pBrush,
                     BOOL(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
                     int nCount, int x, int y, int nWidth, int nHeight) {
	error("TODO: CDC::GrayString");
}

UINT CDC::GetTextAlign() const {
	error("TODO: CDC::GetTextAlign");
}

UINT CDC::SetTextAlign(UINT nFlags) {
	error("TODO: CDC::SetTextAlign");
}

BOOL CDC::GetTextMetrics(LPTEXTMETRIC lpMetrics) const {
	TEXTMETRIC &tm = *lpMetrics;
	Graphics::Font *font = *(CFont::Impl *)impl()->_font;

	memset(&tm, 0, sizeof(TEXTMETRIC)); // Initialize to zero

	tm.tmHeight = font->getFontHeight();
	tm.tmAscent = tm.tmHeight;
	tm.tmDescent = 0;
	tm.tmInternalLeading = 0;
	tm.tmExternalLeading = 0;

	tm.tmAveCharWidth = 0;
	int totalWidth = 0;
	char first = (char)32;
	char last = (char)127;

	for (int c = first; c <= last; ++c)
		totalWidth += font->getCharWidth(c);

	tm.tmAveCharWidth = totalWidth / (last - first + 1);

	tm.tmMaxCharWidth = font->getMaxCharWidth();
	tm.tmWeight = FW_NORMAL;
	tm.tmOverhang = 0;

	tm.tmDigitizedAspectX = 1;
	tm.tmDigitizedAspectY = 1;

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

} // namespace MFC
} // namespace Bagel
