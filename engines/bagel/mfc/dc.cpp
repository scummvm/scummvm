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

CDC::~CDC() {
	if (m_hDC && _permanent)
		DeleteDC();
	else if (m_hDC) {
		auto *pMap = AfxGetApp()->afxMapHDC(true);
		assert(!pMap->LookupTemporary(m_hDC));
	}
}

CDC *CDC::FromHandle(HDC hDC) {
	CHandleMap<CDC> *pMap = AfxGetApp()->afxMapHDC(TRUE);
	assert(pMap != nullptr);

	CDC *pObject = pMap->FromHandle(hDC);
	assert(pObject == nullptr || pObject->m_hDC == hDC);
	return pObject;
}

void CDC::AfxHookObject() {
	CHandleMap<CDC> *pMap = AfxGetApp()->afxMapHDC(TRUE);
	assert(pMap != nullptr);

	if (!pMap->LookupPermanent(m_hDC)) {
		pMap->SetPermanent(m_hDC, this);
		_permanent = true;
	}
}

void CDC::AfxUnhookObject() {
	if (m_hDC && _permanent) {
		CHandleMap<CDC> *pMap = AfxGetApp()->afxMapHDC(TRUE);
		assert(pMap != nullptr);

		pMap->RemoveHandle(m_hDC);
		_permanent = false;
	}
}

BOOL CDC::CreateDC(LPCSTR lpszDriverName, LPCSTR lpszDeviceName,
                   LPCSTR lpszOutput, const void *lpInitData) {
	error("TODO: CDC::CreateDC");
}

BOOL CDC::CreateCompatibleDC(CDC *pDC) {
	assert(!m_hDC);
	CDC::Impl *dc = new CDC::Impl();

	if (pDC) {
		dc->setFormat(pDC->impl()->getFormat());
	} else {
		CDC::Impl *src = (CDC::Impl *)MFC::GetDC(nullptr);
		dc->setFormat(src->getFormat());
		MFC::ReleaseDC(nullptr, src);
	}

	m_hDC = dc;

	// This is where it becomes permanent
	AfxHookObject();
	return true;
}

BOOL CDC::DeleteDC() {
	AfxUnhookObject();

	CDC::Impl *dc = static_cast<CDC::Impl *>(m_hDC);
	delete dc;
	m_hDC = nullptr;
	return true;
}

BOOL CDC::Attach(HDC hDC) {
	assert(m_hDC == nullptr);

	if (hDC == nullptr)
		return FALSE;

	m_hDC = hDC;
	AfxHookObject();

	return TRUE;
}

HDC CDC::Detach() {
	HDC hDC = m_hDC;
	if (hDC != nullptr)
		AfxUnhookObject();

	m_hDC = nullptr;
	return hDC;
}

int CDC::SetStretchBltMode(int nStretchMode) {
	int oldMode = _stretchMode;
	_stretchMode = nStretchMode;
	return oldMode;
}

int CDC::GetDeviceCaps(int nIndex) const {
	return MFC::GetDeviceCaps(m_hDC, nIndex);
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

void CDC::setClipRect(const Common::Rect &r) {
	CDC::Impl *dc = static_cast<CDC::Impl *>(m_hDC);
	assert(dc);
	dc->getSurface()->setClipRect(r);
}

void CDC::resetClipRect() {
	CDC::Impl *dc = static_cast<CDC::Impl *>(m_hDC);
	assert(dc);
	dc->getSurface()->resetClip();
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
	impl()->ellipse(lpRect);
}

void CDC::Ellipse(int x1, int y1, int x2, int y2) {
	impl()->ellipse(x1, y1, x2, y2);
}

void CDC::FrameRect(LPCRECT lpRect, CBrush *pBrush) {
	impl()->drawRect(*lpRect, pBrush);
}

void CDC::Draw3dRect(const CRect &rect, COLORREF clrTopLeft, COLORREF clrBottomRight) {
	impl()->draw3dRect(rect, clrTopLeft, clrBottomRight);
}

void CDC::DrawFocusRect(const CRect &rect) {
	impl()->drawFocusRect(rect);
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
	impl()->floodFill(x, y, crColor);
	return true;
}

BOOL CDC::FloodFill(int x, int y, COLORREF crColor,
        UINT nFillType) {
	impl()->floodFill(x, y, crColor, nFillType);
	return true;
}

void CDC::Rectangle(LPCRECT lpRect) {
	impl()->rectangle(lpRect);
}

void CDC::Rectangle(int x1, int y1, int x2, int y2) {
	impl()->rectangle(x1, y1, x2, y2);
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
	impl()->frameRgn(pRgn, pBrush, nWidth, nHeight);
	return true;
}

void CDC::MoveTo(int x, int y) {
	impl()->moveTo(x, y);
}

void CDC::LineTo(int x, int y) {
	impl()->lineTo(x, y);
}

COLORREF CDC::GetPixel(int x, int y) const {
	return impl()->getPixel(x, y);
}

COLORREF CDC::GetPixel(const POINT &point) const {
	return impl()->getPixel(point.x, point.y);
}

CGdiObject *CDC::SelectStockObject(int nIndex) {
	HGDIOBJ hObject = MFC::GetStockObject(nIndex);
	assert(hObject != nullptr);

	HGDIOBJ hOldObj = SelectObject(hObject);
	return CGdiObject::FromHandle(hOldObj);
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

HGDIOBJ CDC::SelectObject(HGDIOBJ hGdiObj) {
	return impl()->Attach(hGdiObj);
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

COLORREF CDC::GetBkColor() const {
	return impl()->getBkColor();
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
	_defaultBitmap.CreateBitmap(1, 1, 1, 8, nullptr);
	_bitmap = _defaultBitmap._bitmap;

	// Defaults
	CWinApp *app = AfxGetApp();
	_font = app->getDefaultFont();
	_pen = app->getDefaultPen();
	_brush = app->getDefaultBrush();
	_palette = app->getSystemPalette();
}

CDC::Impl::Impl(HDC srcDc) {
	const CDC::Impl *src = (CDC::Impl *)srcDc;

	// By default the _bitmap will point to
	// this dummy 1x1 bitmap
	_defaultBitmap.CreateBitmap(1, 1, 1, 8, nullptr);
	_bitmap = _defaultBitmap._bitmap;

	if (src) {
		_font = src->_font;
		_pen = src->_pen;
		_brush = src->_brush;
		_palette = src->_palette;
	} else {
		// Defaults
		CWinApp *app = AfxGetApp();
		_font = app->getDefaultFont();
		_pen = app->getDefaultPen();
		_brush = app->getDefaultBrush();
		_palette = app->getSystemPalette();
	}
}

HGDIOBJ CDC::Impl::Attach(HGDIOBJ gdiObj) {
	CGdiObjectImpl *obj = (CGdiObjectImpl *)gdiObj;

	CBitmap::Impl *bitmap = dynamic_cast<CBitmap::Impl *>(obj);
	if (bitmap) {
		HBITMAP result = _bitmap;
		_bitmap = bitmap;
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

	CBrush::Impl *brush = dynamic_cast<CBrush::Impl *>(obj);
	if (brush) {
		HBRUSH result = _brush;
		_brush = brush;
		return result;
	}

	if (gdiObj)
		error("Unsupported gdi object");
	return nullptr;
}

Gfx::ClippedSurface *CDC::Impl::getSurface() const {
	assert(_bitmap);
	return static_cast<CBitmap::Impl *>(_bitmap);
}

const Graphics::PixelFormat &CDC::Impl::getFormat() const {
	return getSurface()->format;
}

void CDC::Impl::setFormat(const Graphics::PixelFormat &format) {
	_defaultBitmap._bitmap->create(1, 1, format);
	_bitmap = _defaultBitmap._bitmap;
}

void CDC::Impl::setScreenRect() {
	Graphics::Screen *scr = AfxGetApp()->getScreen();
	_defaultBitmap._bitmap->create(*scr, Common::Rect(0, 0, scr->w, scr->h));
	_bitmap = _defaultBitmap._bitmap;
}

void CDC::Impl::setScreenRect(const Common::Rect &r) {
	Graphics::Screen *scr = AfxGetApp()->getScreen();
	assert(r.left >= 0 && r.top >= 0 &&
		r.right <= scr->w && r.bottom <= scr->h);

	_defaultBitmap._bitmap->create(*scr, r);
	_bitmap = _defaultBitmap._bitmap;
}

HPALETTE CDC::Impl::selectPalette(HPALETTE pal) {
	HPALETTE oldPal = _palette;

	if (pal) {
		_palette = pal;
		CBitmap::Impl *bitmap = (CBitmap::Impl *)_bitmap;

		auto *newPal = static_cast<CPalette::Impl *>(pal);
		if (bitmap)
			bitmap->setPalette(newPal->data(), 0, newPal->size());
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
	assert(pal);
	AfxGetApp()->setPalette(*pal);
	return 256;
}

COLORREF CDC::Impl::GetNearestColor(COLORREF crColor) const {
	if (crColor <= 255 || (crColor >> 24) == 1)
		return crColor & 0xff;

	const auto *pal = static_cast<const CPalette::Impl *>(_palette);

	if (pal)
		return pal->findBestColor(
			GetRValue(crColor),
			GetGValue(crColor),
			GetBValue(crColor));

	return AfxGetApp()->getColor(crColor);
}

void CDC::Impl::fillSolidRect(LPCRECT lpRect, COLORREF clr) {
	fillRect(*lpRect, clr);
}

void CDC::Impl::fillSolidRect(int x, int y, int cx, int cy, COLORREF clr) {
	fillRect(Common::Rect(x, y, x + cx, y + cy), clr);
}

void CDC::Impl::fillRect(const Common::Rect &r, COLORREF crColor) {
	static_cast<CBitmap::Impl *>(_bitmap)->fillRect(r,
		GetNearestColor(crColor));
}

void CDC::Impl::drawRect(const Common::Rect &r, CBrush *brush) {
	CBitmap::Impl *bitmap = (CBitmap::Impl *)_bitmap;
	byte color = brush->_brush->getColor();
	bitmap->frameRect(r, color);
}

void CDC::Impl::drawRect(const Common::Rect &r, COLORREF crColor) {
	CPen::Impl *pen = (CPen::Impl *)_pen;
	CBitmap::Impl *bitmap = (CBitmap::Impl *)_bitmap;
	uint brushColor = GetNearestColor(crColor);
	uint penColor = getPenColor();

	if (pen->_penStyle == PS_INSIDEFRAME)
		bitmap->fillRect(r, brushColor);
	
	bitmap->frameRect(r, penColor);
}

void CDC::Impl::frameRgn(const CRgn *pRgn, CBrush *brush, int nWidth, int nHeight) {
	// We don't currently support larger brush sizes
	assert(nWidth == 1 && nHeight == 1);
	uint brushColor = getBrushColor();

	// Set up a pen using the specified brush color
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, brushColor);
	HPEN oldPen = Attach(pen.m_hObject);

	// Iterate over drawing lines to each point
	bool firstTime = true;
	for (const POINT &pt : pRgn->_points) {
		if (firstTime) {
			firstTime = false;
			moveTo(pt.x, pt.y);
		} else {
			lineTo(pt.x, pt.y);
		}
	}

	// Final line segment back to original point
	lineTo(pRgn->_points[0].x, pRgn->_points[0].y);

	// Restore old pen
	Attach(oldPen);
}

void CDC::Impl::rectangle(LPCRECT lpRect) {
	drawRect(*lpRect, getBrushColor());
}

void CDC::Impl::rectangle(int x1, int y1, int x2, int y2) {
	drawRect(Common::Rect(x1, y1, x2, y2), getBrushColor());
}

void CDC::Impl::floodFill(int x, int y, COLORREF crColor) {
	CBitmap::Impl *bitmap = static_cast<CBitmap::Impl *>(_bitmap);
	assert(bitmap->format.bytesPerPixel == 1);

	if (x < 0 || y < 0 || x >= bitmap->w || y >= bitmap->h)
		return;

	uint color = GetNearestColor(crColor);
	byte *pixel = (byte *)bitmap->getBasePtr(x, y);
	const byte oldColor = *pixel;
	int cx, cy;
	int minX = 9999, maxX = -1, minY = 9999, maxY = -1;

	Common::Queue<Common::Pair<int, int>> queue;
	queue.push({ x, y });

	while (!queue.empty()) {
		cx = queue.front().first;
		cy = queue.front().second;
		queue.pop();

		// Check bounds and color match
		if (cx < 0 || cx >= bitmap->w || cy < 0 || cy >= bitmap->h)
			continue;
		pixel = (byte *)bitmap->getBasePtr(cx, cy);
		if (*pixel != oldColor)
			continue;

		// Set new color
		*pixel = color;

		// Keep track of the modified area
		minX = MIN(minX, cx);
		maxX = MAX(maxX, cx);
		minY = MIN(minY, cy);
		maxY = MAX(maxY, cy);

		// Push neighboring pixels
		queue.push({ cx + 1, cy });
		queue.push({ cx - 1, cy });
		queue.push({ cx, cy + 1 });
		queue.push({ cx, cy - 1 });
	}

	bitmap->addDirtyRect(Common::Rect(minX, minY, maxX + 1, maxY + 1));
}

void CDC::Impl::floodFill(int x, int y, COLORREF crColor,
		UINT nFillType) {
	error("TODO: CDC::floodFill");
}

void CDC::Impl::draw3dRect(const CRect &rect, COLORREF clrTopLeft, COLORREF clrBottomRight) {
	fillSolidRect(rect.left, rect.top, rect.Width() - 1, 1, clrTopLeft);
	fillSolidRect(rect.left, rect.top + 1, 1, rect.Height() - 2, clrTopLeft);
	fillSolidRect(rect.left, rect.bottom - 1, rect.Width(), 1, clrBottomRight);
	fillSolidRect(rect.right - 1, rect.top, 1, rect.Height() - 1, clrBottomRight);
}

void CDC::Impl::drawFocusRect(const CRect &rect) {
	drawRect(rect, GetNearestColor(RGB(128, 128, 128)));
}

void CDC::Impl::ellipse(const Common::Rect &r, COLORREF crColor) {
	CPen::Impl *pen = (CPen::Impl *)_pen;
	CBitmap::Impl *bitmap = (CBitmap::Impl *)_bitmap;
	uint brushColor = GetNearestColor(crColor);
	uint penColor = getPenColor();

	if (pen->_penStyle == PS_INSIDEFRAME)
		bitmap->drawEllipse(r.left, r.top, r.right, r.bottom,
			brushColor, true);

	bitmap->drawEllipse(r.left, r.top, r.right, r.bottom,
		penColor, false);
}

void CDC::Impl::ellipse(LPCRECT lpRect) {
	ellipse(*lpRect, getBrushColor());
}

void CDC::Impl::ellipse(int x1, int y1, int x2, int y2) {
	ellipse(Common::Rect(x1, y1, x2, y2), getBrushColor());
}

void CDC::Impl::bitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, DWORD dwRop) {
	const Common::Rect srcRect(xSrc, ySrc, xSrc + nWidth, ySrc + nHeight);

	Graphics::ManagedSurface dummySrc;
	Graphics::ManagedSurface *src = &dummySrc;
	if (pSrcDC) {
		src = pSrcDC->impl()->getSurface();
		assert(srcRect.left >= 0 && srcRect.top >= 0 &&
			srcRect.right <= src->w && srcRect.bottom <= src->h);
	}

	Graphics::ManagedSurface *dest = getSurface();
	const Common::Point destPos(x, y);
	uint bgColor = getBkPixel();

	assert(x >= 0 && y >= 0 && (x + srcRect.width()) <= dest->w &&
		(y + srcRect.height()) <= dest->h);

	Gfx::blit(src, dest, srcRect, destPos, bgColor, dwRop);
}

void CDC::Impl::stretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
	int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop) {
	Graphics::ManagedSurface *src = pSrcDC->impl()->getSurface();
	Graphics::ManagedSurface *dest = getSurface();
	const Common::Rect srcRect(xSrc, ySrc, xSrc + nSrcWidth, ySrc + nSrcHeight);
	const Common::Rect destRect(x, y, x + nWidth, y + nHeight);
	uint bgColor = getBkPixel();

	Gfx::stretchBlit(src, dest, srcRect, destRect, bgColor, dwRop);
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

COLORREF CDC::Impl::getPixel(int x, int y) const {
	Graphics::ManagedSurface *src = getSurface();
	assert(src->format.bytesPerPixel == 1);
	const byte *pixel = (const byte *)src->getBasePtr(x, y);

	assert(_palette);
	const auto *pal = static_cast<const CPalette::Impl *>(_palette);
	byte r, g, b;
	pal->get(*pixel, r, g, b);

	return RGB(r, g, b);
}

uint CDC::Impl::getPenColor() const {
	CPen::Impl *pen = (CPen::Impl *)_pen;
	assert(pen->_penStyle == PS_SOLID ||
		pen->_penStyle == PS_INSIDEFRAME);

	return GetNearestColor(pen->_color);
}

uint CDC::Impl::getBrushColor() const {
	CBrush::Impl *brush = static_cast<CBrush::Impl *>(_brush);
	assert(brush->_type == HS_HORIZONTAL ||
		brush->_type == HS_VERTICAL);
	return brush->getColor();
}

COLORREF CDC::Impl::setBkColor(COLORREF crColor) {
	COLORREF oldColor = _bkColor;
	_bkColor = crColor;
	return oldColor;
}

COLORREF CDC::Impl::getBkColor() const {
	return _bkColor;
}

COLORREF CDC::Impl::getBkPixel() const {
	return _bkColor == RGB(255, 255, 255) ? 255 :
		GetNearestColor(_bkColor);
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

BOOL CDC::Impl::textOut(int x, int y, LPCSTR lpszString, int nCount,
		int nTabPositions, const LPINT lpnTabStopPositions,
		int nTabOrigin, CSize *size) {
	Graphics::ManagedSurface *dest = getSurface();
	RECT r;

	if ((_textAlign & 6) == TA_RIGHT) {
		r.left = 0;
		r.right = x;
	} else if ((_textAlign & 6) == TA_CENTER) {
		r.left = 0;
		r.right = dest->w;
	} else {
		// Left align
		r.left = x;
		r.right = dest->w;
	}

	if ((_textAlign & 24) == TA_BOTTOM) {
		r.top = 0;
		r.bottom = y;
	} else {
		r.top = y;
		r.bottom = dest->h;
	}

	CString str(lpszString, nCount);
	drawText(str, &r, DT_SINGLELINE | DT_NOPREFIX |
		_textAlign, nTabPositions, lpnTabStopPositions,
		nTabOrigin, size);
	return true;
}

BOOL CDC::Impl::textOut(int x, int y, const CString &str,
		int nTabPositions, const LPINT lpnTabStopPositions,
		int nTabOrigin, CSize *size) {
	Graphics::ManagedSurface *dest = getSurface();
	RECT r;
	r.left = x;
	r.top = y;
	r.right = dest->w;
	r.bottom = dest->h;

	drawText(str, &r, DT_SINGLELINE | DT_NOPREFIX,
		nTabPositions, lpnTabStopPositions,
		nTabOrigin, size);
	return true;
}

BOOL CDC::Impl::extTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
		LPCSTR lpszString, UINT nCount, LPINT lpDxWidths) {
	error("TODO: extTextOut");
}

BOOL CDC::Impl::extTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
		const CString &str, LPINT lpDxWidths) {
	error("TODO: extTextOut");
}

CSize CDC::Impl::tabbedTextOut(int x, int y, LPCSTR lpszString, int nCount,
		int nTabPositions, const LPINT lpnTabStopPositions, int nTabOrigin) {
	CString str(lpszString, nCount);

	CSize size;
	textOut(x, y, str.c_str(), str.size(),
		nTabPositions, lpnTabStopPositions, nTabOrigin, &size);

	return size;
}

CSize CDC::Impl::tabbedTextOut(int x, int y, const CString &str,
		int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) {
	CSize size;
	textOut(x, y, str, nTabPositions, lpnTabStopPositions, nTabOrigin, &size);
	return size;
}

int CDC::Impl::drawText(LPCSTR lpszString, int nCount,
		LPRECT lpRect, UINT nFormat, int nTabPositions,
		const LPINT lpnTabStopPositions, int nTabOrigin,
		CSize *size) {
	return drawText(CString(lpszString, nCount),
		lpRect, nFormat, nTabPositions,
		lpnTabStopPositions, nTabOrigin, size);
}

int CDC::Impl::drawText(const CString &str, LPRECT lpRect, UINT nFormat,
		int nTabPositions, const LPINT lpnTabStopPositions,
		int nTabOrigin, CSize *size) {
	Graphics::Font *font = *(CFont::Impl *)_font;
	Graphics::ManagedSurface *dest = getSurface();
	uint textCol = GetNearestColor(_textColor);
	CSize dummySize;
	if (!size)
		size = &dummySize;

	Common::Array<int> tabStops;
	for (int i = 0; i < nTabPositions; ++i)
		tabStops.push_back(lpnTabStopPositions[i]);

	*size = renderText(str, dest, font, textCol, lpRect,
		nFormat, tabStops, nTabOrigin,
		GetNearestColor(_bkColor), _bkMode,
		GetNearestColor(_textColor), _textAlign);

	return size->cy;
}

CSize CDC::Impl::getTextExtent(LPCSTR lpszString, int nCount) const {
	CString str(lpszString, nCount);
	Graphics::Font *font = *(CFont::Impl *)_font;

	CSize s;
	s.cx = font->getStringWidth(str);
	s.cy = font->getFontHeight();
	return s;
}

CSize CDC::Impl::getTextExtent(const CString &str) const {
	Graphics::Font *font = *(CFont::Impl *)_font;

	CSize s;
	s.cx = font->getStringWidth(str);
	s.cy = font->getFontHeight();
	return s;
}

CSize CDC::Impl::getOutputTextExtent(LPCSTR lpszString, int nCount) const {
	// TODO: Proper implementation that handles tabs, etc.
	return getTextExtent(lpszString, nCount);
}

CSize CDC::Impl::getOutputTextExtent(const CString &str) const {
	// TODO: Proper implementation that handles tabs, etc.
	return getTextExtent(str);
}

CSize CDC::Impl::getTabbedTextExtent(LPCSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions) const {
	// TODO: Proper implementation that handles tabs, etc.
	return getTextExtent(lpszString, nCount);
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
	return _textAlign;
}

UINT CDC::Impl::setTextAlign(UINT nFlags) {
	UINT oldAlign = _textAlign;
	_textAlign = nFlags;
	return oldAlign;
}

BOOL CDC::Impl::getTextMetrics(LPTEXTMETRIC lpMetrics) const {
	TEXTMETRIC &tm = *lpMetrics;
	Gfx::Font *font = *(CFont::Impl *)_font;

	memset(&tm, 0, sizeof(TEXTMETRIC)); // Initialize to zero

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
	tm.tmAscent = font->getFontAscent();
	tm.tmDescent = font->getFontDescent();

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
