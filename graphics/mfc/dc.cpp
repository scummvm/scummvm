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
#include "graphics/mfc/afxwin.h"
#include "graphics/mfc/win_hand.h"
#include "graphics/mfc/gfx/blitter.h"

namespace Graphics {
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
	CHandleMap<CDC> *pMap = AfxGetApp()->afxMapHDC(true);
	assert(pMap != nullptr);

	CDC *pObject = pMap->FromHandle(hDC);
	assert(pObject == nullptr || pObject->m_hDC == hDC);
	return pObject;
}

void CDC::AfxHookObject() {
	CHandleMap<CDC> *pMap = AfxGetApp()->afxMapHDC(true);
	assert(pMap != nullptr);

	if (!pMap->LookupPermanent(m_hDC)) {
		pMap->SetPermanent(m_hDC, this);
		_permanent = true;
	}
}

void CDC::AfxUnhookObject() {
	if (m_hDC && _permanent) {
		CHandleMap<CDC> *pMap = AfxGetApp()->afxMapHDC(true);
		assert(pMap != nullptr);

		pMap->RemoveHandle(m_hDC);
		_permanent = false;
	}
}

bool CDC::CreateDC(const char *lpszDriverName, const char *lpszDeviceName,
                   const char *lpszOutput, const void *lpInitData) {
	error("TODO: CDC::CreateDC");
}

bool CDC::CreateCompatibleDC(CDC *pDC) {
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

bool CDC::DeleteDC() {
	AfxUnhookObject();

	CDC::Impl *dc = static_cast<CDC::Impl *>(m_hDC);
	delete dc;
	m_hDC = nullptr;
	return true;
}

bool CDC::Attach(HDC hDC) {
	assert(m_hDC == nullptr);

	if (hDC == nullptr)
		return false;

	m_hDC = hDC;
	AfxHookObject();

	return true;
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

int CDC::SetMapMode(int nMapMode) {
	assert(nMapMode == MM_TEXT);
	return MM_TEXT;
}

CPoint CDC::SetViewportOrg(int x, int y) {
	auto *surface = impl()->getSurface();
	CPoint oldPos = surface->getViewportOrg();
	surface->setViewportOrg(CPoint(x, y));
	return oldPos;
}

CPoint CDC::SetViewportOrg(POINT point) {
	auto *surface = impl()->getSurface();
	CPoint oldPos = surface->getViewportOrg();
	surface->setViewportOrg(point);
	return oldPos;
}

CPoint CDC::GetViewportOrg() const {
	return impl()->getSurface()->getViewportOrg();
}

CPoint CDC::OffsetViewportOrg(int nWidth, int nHeight) {
	auto *surface = impl()->getSurface();
	surface->offsetViewportOrg(nWidth, nHeight);
	return surface->getViewportOrg();
}

int CDC::GetClipBox(LPRECT lpRect) const {
	Common::Rect rect = impl()->getSurface()->getClipRect();
	*lpRect = RectToRECT(rect);
	return rect.isEmpty() ? NULLREGION : SIMPLEREGION;
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

bool CDC::PtVisible(int x, int y) {
	Gfx::Surface *surface = impl()->getSurface();
	Common::Rect clipRect = surface->getClipRect();

	// Clip rect is in device co-ordinates, but the
	// point is in logical units, so need to convert
	POINT pts[2];
	pts[0].x = clipRect.left;
	pts[0].y = clipRect.top;
	pts[1].x = clipRect.right;
	pts[1].y = clipRect.bottom;
	DPtoLP(pts, 2);

	return Common::Rect(pts[0].x, pts[0].y, pts[1].x, pts[1].y).contains(x, y);
}

bool CDC::PtVisible(POINT point) {
	return PtVisible(point.x, point.y);
}

bool CDC::RectVisible(LPCRECT lpRect) {
	Gfx::Surface *surface = impl()->getSurface();
	Common::Rect clipRect = surface->getClipRect();

	// Clip rect is in device co-ordinates, but the
	// point is in logical units, so need to convert
	POINT pts[2];
	pts[0].x = clipRect.left;
	pts[0].y = clipRect.top;
	pts[1].x = clipRect.right;
	pts[1].y = clipRect.bottom;
	DPtoLP(pts, 2);

	return Common::Rect(pts[0].x, pts[0].y, pts[1].x, pts[1].y).intersects(*lpRect);
}

int CDC::SelectClipRgn(CRgn *pRgn) {
	// Custom clipping regions not supported in ScummVM yet
	assert(!pRgn);
	impl()->getSurface()->resetClip();
	return SIMPLEREGION;
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
	CDC::Impl *dc = static_cast<CDC::Impl *>(m_hDC);
	assert(dc);
	return dc->getSurface()->intersectClipRect(
		Common::Rect(x1, y1, x2, y2));
}

int CDC::IntersectClipRect(LPCRECT lpRect) {
	return impl()->getSurface()->intersectClipRect(*lpRect);
}

int CDC::OffsetClipRgn(int x, int y) {
	return impl()->getSurface()->offsetClipRect(x, y);
}

int CDC::OffsetClipRgn(SIZE size) {
	return impl()->getSurface()->offsetClipRect(size.cx, size.cy);
}

int CDC::SelectClipRgn(CRgn *pRgn, int nMode) {
	error("TODO: CDC::SelectClipRgn");
}

int CDC::SetROP2(int nDrawMode) {
	return impl()->setROP2(nDrawMode);
}

bool CDC::DPtoLP(LPPOINT lpPoints, int nCount) {
	// Currently we only support MM_TEXT mode,
	// which has a 1 to 1 mapping, which simplifies matters
	const CPoint WINDOW_ORG(0, 0);
	const CPoint VIEWPORT_ORG = impl()->getSurface()->getViewportOrg();

	for (int i = 0; i < nCount; ++i, ++lpPoints) {
		lpPoints->x += WINDOW_ORG.x - VIEWPORT_ORG.x;
		lpPoints->y += WINDOW_ORG.y - VIEWPORT_ORG.y;
	}

	return true;
}

bool CDC::DPtoLP(RECT *lpRect) {
	// Currently we only support MM_TEXT mode,
	// which has a 1 to 1 mapping, which simplifies matters
	const CPoint WINDOW_ORG(0, 0);
	const CPoint VIEWPORT_ORG = impl()->getSurface()->getViewportOrg();

	lpRect->left += WINDOW_ORG.x - VIEWPORT_ORG.x;
	lpRect->right += WINDOW_ORG.x - VIEWPORT_ORG.x;
	lpRect->top += WINDOW_ORG.y - VIEWPORT_ORG.y;
	lpRect->bottom += WINDOW_ORG.y - VIEWPORT_ORG.y;

	return true;
}

bool CDC::LPtoDP(LPPOINT lpPoints, int nCount) {
	// Currently we only support MM_TEXT mode,
	// which has a 1 to 1 mapping, which simplifies matters
	const CPoint WINDOW_ORG(0, 0);
	const CPoint VIEWPORT_ORG = impl()->getSurface()->getViewportOrg();

	for (; nCount > 0; --nCount, ++lpPoints) {
		lpPoints->x += VIEWPORT_ORG.x - WINDOW_ORG.x;
		lpPoints->y += VIEWPORT_ORG.y - WINDOW_ORG.y;
	}

	return true;
}

bool CDC::LPtoDP(RECT *lpRect) {
	// Currently we only support MM_TEXT mode,
	// which has a 1 to 1 mapping, which simplifies matters
	const CPoint WINDOW_ORG(0, 0);
	const CPoint VIEWPORT_ORG = impl()->getSurface()->getViewportOrg();

	lpRect->left += VIEWPORT_ORG.x - WINDOW_ORG.x;
	lpRect->right += VIEWPORT_ORG.x - WINDOW_ORG.x;
	lpRect->top += VIEWPORT_ORG.y - WINDOW_ORG.y;
	lpRect->bottom += VIEWPORT_ORG.y - WINDOW_ORG.y;

	return true;
}

bool CDC::BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
        int xSrc, int ySrc, uint32 dwRop) {
	impl()->bitBlt(x, y, nWidth, nHeight, pSrcDC, xSrc, ySrc, dwRop);
	return true;
}

bool CDC::StretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
        int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, uint32 dwRop) {
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
	impl()->frameRect(*lpRect, pBrush);
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

bool CDC::FloodFill(int x, int y, COLORREF crColor) {
	impl()->floodFill(x, y, crColor);
	return true;
}

bool CDC::FloodFill(int x, int y, COLORREF crColor,
        unsigned int nFillType) {
	impl()->floodFill(x, y, crColor, nFillType);
	return true;
}

void CDC::Rectangle(LPCRECT lpRect) {
	impl()->rectangle(lpRect);
}

void CDC::Rectangle(int x1, int y1, int x2, int y2) {
	impl()->rectangle(x1, y1, x2, y2);
}

bool CDC::Pie(int x1, int y1, int x2, int y2,
              int x3, int y3, int x4, int y4) {
	error("TODO: CDC::Pie");
}

bool CDC::DrawEdge(LPRECT lpRect, unsigned int nEdge, unsigned int nFlags) {
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

	return true;
}

bool CDC::Pie(LPCRECT lpRect, const POINT &ptStart, const POINT &ptEnd) {
	error("TODO: CDC::Pie");
}

bool CDC::FrameRgn(CRgn *pRgn, CBrush *pBrush,
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

CPalette *CDC::SelectPalette(CPalette *pPalette, bool bForceBackground) {
	HPALETTE hOld = impl()->selectPalette(
		!pPalette ? nullptr : pPalette->m_hObject,
		bForceBackground);
	return (CPalette *)CGdiObject::FromHandle(hOld);
}

COLORREF CDC::GetNearestColor(COLORREF crColor) const {
	return impl()->GetNearestColor(crColor);
}

unsigned int CDC::RealizePalette() {
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

bool CDC::TextOut(int x, int y, const char *lpszString, int nCount) {
	return impl()->textOut(x, y, lpszString, nCount);
}

bool CDC::TextOut(int x, int y, const CString &str) {
	return impl()->textOut(x, y, str);
}

bool CDC::ExtTextOut(int x, int y, unsigned int nOptions, LPCRECT lpRect,
        const char *lpszString, unsigned int nCount, int *lpDxWidths) {
	return impl()->extTextOut(x, y, nOptions, lpRect, lpszString, nCount, lpDxWidths);
}

bool CDC::ExtTextOut(int x, int y, unsigned int nOptions, LPCRECT lpRect,
        const CString &str, int *lpDxWidths) {
	return impl()->extTextOut(x, y, nOptions, lpRect, str, lpDxWidths);
}

CSize CDC::TabbedTextOut(int x, int y, const char *lpszString, int nCount,
        int nTabPositions, int *lpnTabStopPositions, int nTabOrigin) {
	return impl()->tabbedTextOut(x, y, lpszString, nCount,
		nTabPositions, lpnTabStopPositions, nTabOrigin);
}

CSize CDC::TabbedTextOut(int x, int y, const CString &str,
        int nTabPositions, int *lpnTabStopPositions, int nTabOrigin) {
	return impl()->tabbedTextOut(x, y, str, nTabPositions,
		lpnTabStopPositions, nTabOrigin);
}

int CDC::DrawText(const char *lpszString, int nCount,
        LPRECT lpRect, unsigned int nFormat) {
	return impl()->drawText(lpszString, nCount, lpRect, nFormat);
}

int CDC::DrawText(const CString &str, LPRECT lpRect, unsigned int nFormat) {
	return impl()->drawText(str, lpRect, nFormat);
}

CSize CDC::GetTextExtent(const char *lpszString, int nCount) const {
	return impl()->getTextExtent(lpszString, nCount);
}

CSize CDC::GetTextExtent(const CString &str) const {
	return impl()->getTextExtent(str);
}

CSize CDC::GetOutputTextExtent(const char *lpszString, int nCount) const {
	return impl()->getOutputTextExtent(lpszString, nCount);
}

CSize CDC::GetOutputTextExtent(const CString &str) const {
	return impl()->getOutputTextExtent(str);
}

CSize CDC::GetTabbedTextExtent(const char *lpszString, int nCount,
        int nTabPositions, int *lpnTabStopPositions) const {
	return impl()->getTabbedTextExtent(lpszString, nCount,
		nTabPositions, lpnTabStopPositions);
}

CSize CDC::GetTabbedTextExtent(const CString &str,
        int nTabPositions, int *lpnTabStopPositions) const {
	return impl()->getTabbedTextExtent(str, nTabPositions,
		lpnTabStopPositions);
}

CSize CDC::GetOutputTabbedTextExtent(const char *lpszString, int nCount,
		int nTabPositions, int *lpnTabStopPositions) const {
	return impl()->getOutputTabbedTextExtent(lpszString, nCount,
		nTabPositions, lpnTabStopPositions);
}

CSize CDC::GetOutputTabbedTextExtent(const CString &str,
        int nTabPositions, int *lpnTabStopPositions) const {
	return impl()->getOutputTabbedTextExtent(str,
		nTabPositions, lpnTabStopPositions);
}

bool CDC::GrayString(CBrush *pBrush,
        bool(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
        int nCount, int x, int y, int nWidth, int nHeight) {
	return impl()->grayString(pBrush, lpfnOutput, lpData,
		nCount, x, y, nWidth, nHeight);
}

unsigned int CDC::GetTextAlign() const {
	return impl()->getTextAlign();
}

unsigned int CDC::SetTextAlign(unsigned int nFlags) {
	return impl()->setTextAlign(nFlags);
}

bool CDC::GetTextMetrics(LPTEXTMETRIC lpMetrics) const {
	return impl()->getTextMetrics(lpMetrics);
}

/*--------------------------------------------*/

CDC::Impl::Impl(CWnd *wndOwner) : m_pWnd(wndOwner), _drawMode(R2_COPYPEN) {
	// By default the _bitmap will point to
	// this dummy 1x1 bitmap
	_defaultBitmap.CreateBitmap(1, 1, 1, 8, nullptr);
	_bitmap = _defaultBitmap.bitmap();

	// Defaults
	CWinApp *app = AfxGetApp();
	_font = app->getDefaultFont();
	_pen = app->getDefaultPen();
	_brush = app->getDefaultBrush();
	_palette = app->getSystemDefaultPalette();
}

CDC::Impl::Impl(HDC srcDc) {
	const CDC::Impl *src = (CDC::Impl *)srcDc;

	// By default the _bitmap will point to
	// this dummy 1x1 bitmap
	_defaultBitmap.CreateBitmap(1, 1, 1, 8, nullptr);
	_bitmap = _defaultBitmap.bitmap();

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
		_palette = app->getSystemDefaultPalette();
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

Gfx::Surface *CDC::Impl::getSurface() const {
	assert(_bitmap);
	return static_cast<CBitmap::Impl *>(_bitmap);
}

const Graphics::PixelFormat &CDC::Impl::getFormat() const {
	return getSurface()->format;
}

void CDC::Impl::setFormat(const Graphics::PixelFormat &format) {
	_defaultBitmap.bitmap()->create(1, 1, format);
	_bitmap = _defaultBitmap.bitmap();
}

void CDC::Impl::setScreenRect() {
	Graphics::Screen *scr = AfxGetApp()->getScreen();
	_defaultBitmap.bitmap()->create(*scr, Common::Rect(0, 0, scr->w, scr->h));
	_bitmap = _defaultBitmap.bitmap();
}

void CDC::Impl::setScreenRect(const Common::Rect &r) {
	Graphics::Screen *scr = AfxGetApp()->getScreen();
	assert(r.left >= 0 && r.top >= 0 &&
		r.right <= scr->w && r.bottom <= scr->h);

	_defaultBitmap.bitmap()->create(*scr, r);
	_bitmap = _defaultBitmap.bitmap();
}

HPALETTE CDC::Impl::selectPalette(HPALETTE pal, bool bForceBackground) {
	CWinApp *app = AfxGetApp();
	HPALETTE oldPal = _palette;
	//CWnd *pTopLevel = m_pWnd->GetTopLevelFrame();

	_paletteRealized = false;
	m_bForceBackground = bForceBackground;
	if (!m_bForceBackground) {
		m_bForceBackground = app->GetActiveWindow() != m_pWnd;
		//CDC *dc = wnd->GetDC();
		//m_bForceBackground = dc->m_hDC == this;
		//wnd->ReleaseDC(dc);
	}

	if (pal) {
		_palette = pal;
		_hasLogicalPalette = app->getSystemDefaultPalette() != pal;
		CBitmap::Impl *bitmap = (CBitmap::Impl *)_bitmap;

		auto *newPal = static_cast<CPalette::Impl *>(pal);
		if (bitmap)
			bitmap->setPalette(newPal->data(), 0, newPal->size());
	}

	return oldPal;
}

CPalette *CDC::Impl::selectPalette(CPalette *pal, bool bForceBackground) {
	CPalette *oldPal = _cPalette;
	_cPalette = pal;
	selectPalette((HPALETTE)_cPalette->m_hObject, bForceBackground);
	return oldPal;
}


unsigned int CDC::Impl::realizePalette() {
	const auto *pal = static_cast<const CPalette::Impl *>(_palette);
	if (m_pWnd == nullptr || !pal)
		return 0;

	if (!m_bForceBackground) {
		// This window is active - update the system palette
		AfxGetApp()->setPalette(*pal);
		_paletteRealized = true;
		return 1;  // number of entries changed - simplified
	} else {
		_paletteRealized = true;
	}

	return 0;
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
	CPen::Impl *pen = (CPen::Impl *)_pen;
	byte brushColor = brush->brush()->getColor();
	uint penColor = getPenColor();

	if (pen->_penStyle == PS_INSIDEFRAME &&
			brush->brush()->_type != BS_HOLLOW) {
		Common::Rect rInner(r.left + 1, r.top + 1,
			r.right - 1, r.bottom - 1);
		bitmap->fillRect(rInner, brushColor);
	}

	Gfx::frameRect(bitmap, r, penColor, _drawMode);
}

void CDC::Impl::frameRect(const Common::Rect &r, CBrush *brush) {
	CBitmap::Impl *bitmap = (CBitmap::Impl *)_bitmap;
	byte brushColor = brush->brush()->getColor();

	bitmap->frameRect(r, brushColor);
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
	CBrush *brush = CBrush::FromHandle(_brush);
	drawRect(*lpRect, brush);
}

void CDC::Impl::rectangle(int x1, int y1, int x2, int y2) {
	CBrush *brush = CBrush::FromHandle(_brush);
	drawRect(Common::Rect(x1, y1, x2, y2), brush);
}

void CDC::Impl::floodFill(int x, int y, COLORREF crColor) {
	CBitmap::Impl *bitmap = static_cast<CBitmap::Impl *>(_bitmap);
	assert(bitmap->format.bytesPerPixel == 1);

	if (x < 0 || y < 0 || x >= bitmap->w || y >= bitmap->h)
		return;

	uint color = GetNearestColor(crColor);
	byte *startPixel = (byte *)bitmap->getBasePtr(x, y);
	const byte oldColor = *startPixel;

	if (color == oldColor)
		return;

	struct Point {
		int x, y;
	};

	// Set up queue with initial point
	Common::Queue<Point> queue;
	queue.push({ x, y });
	byte *pixelP;

	int minX = x, maxX = x, minY = y, maxY = y;

	while (!queue.empty()) {
		Point startPt = queue.front();
		queue.pop();

		int curY = startPt.y;
		int xStart = startPt.x;
		int xEnd = startPt.x;

		// Find the left edge, changing pixels as it goes 
		pixelP = (byte *)bitmap->getBasePtr(xStart, curY);
		*pixelP = color;
		for (; xStart > 0 && *(pixelP - 1) == oldColor; --xStart, --pixelP)
			*(pixelP - 1) = color;

		// Find the right edge, changing pixels as it goes
		pixelP = (byte *)bitmap->getBasePtr(xEnd, curY);
		for (; xEnd < (bitmap->w - 1) && *(pixelP + 1) == oldColor; ++xEnd, ++pixelP)
			*(pixelP + 1) = color;

		// Track modified area
		minX = MIN(minX, xStart);
		maxX = MAX(maxX, xEnd);
		minY = MIN(minY, curY);
		maxY = MAX(maxY, curY);

		// Scan for line segments above or below
		for (int deltaY = -1; deltaY <= 1; deltaY += 2) {
			int lineY = curY + deltaY;
			if (lineY < 0 || lineY >= bitmap->h)
				continue;

			// Loop looking for line segments
			pixelP = (byte *)bitmap->getBasePtr(xStart, lineY);
			int curX = xStart;

			while (curX <= xEnd) {
				// Find the start of any line segment
				if (*pixelP == oldColor) {
					// Add new line starting point to queue
					queue.push({ curX, lineY });

					// Move beyond it
					do {
						++curX;
						++pixelP;
					} while (curX <= xEnd && *pixelP == oldColor);
				} else {
					// Move to next pixel, looking for new line segment
					++curX;
					++pixelP;
				}
			}
		}
	}

	bitmap->addDirtyRect(Common::Rect(minX, minY, maxX + 1, maxY + 1));
}

void CDC::Impl::floodFill(int x, int y, COLORREF crColor,
		unsigned int nFillType) {
	error("TODO: CDC::floodFill");
}

void CDC::Impl::draw3dRect(const CRect &rect, COLORREF clrTopLeft, COLORREF clrBottomRight) {
	fillSolidRect(rect.left, rect.top, rect.Width() - 1, 1, clrTopLeft);
	fillSolidRect(rect.left, rect.top + 1, 1, rect.Height() - 2, clrTopLeft);
	fillSolidRect(rect.left, rect.bottom - 1, rect.Width(), 1, clrBottomRight);
	fillSolidRect(rect.right - 1, rect.top, 1, rect.Height() - 1, clrBottomRight);
}

void CDC::Impl::drawFocusRect(const CRect &rect) {
	CBrush brush(RGB(128, 128, 128));
	drawRect(rect, &brush);
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
		int xSrc, int ySrc, uint32 dwRop) {
	const Common::Rect srcRect(xSrc, ySrc, xSrc + nWidth, ySrc + nHeight);

	Gfx::Surface dummySrc;
	Gfx::Surface *src = &dummySrc;
	uint32 *paletteMap = nullptr;

	if (pSrcDC) {
		auto *srcImpl = pSrcDC->impl();
		src = srcImpl->getSurface();

		// Get a palette map if necessary
		paletteMap = getPaletteMap(srcImpl);
	}

	Gfx::Surface *dest = getSurface();
	const Common::Point destPos(x, y);
	uint bgColor = getBkPixel();

	Gfx::blit(src, dest, srcRect, destPos, bgColor, dwRop, paletteMap);

	delete[] paletteMap;
}

void CDC::Impl::stretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
	int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, uint32 dwRop) {
	auto *srcImpl = pSrcDC->impl();
	Gfx::Surface *src = srcImpl->getSurface();
	Gfx::Surface *dest = getSurface();
	const Common::Rect srcRect(xSrc, ySrc, xSrc + nSrcWidth, ySrc + nSrcHeight);
	const Common::Rect destRect(x, y, x + nWidth, y + nHeight);
	uint bgColor = getBkPixel();
	uint32 *paletteMap = getPaletteMap(srcImpl);

	Gfx::stretchBlit(src, dest, srcRect, destRect, bgColor, dwRop, nullptr);

	delete[] paletteMap;
}

uint32 *CDC::Impl::getPaletteMap(const CDC::Impl *srcImpl) {
	Graphics::Palette *srcPal, *destPal;

	// If we have a logical palette, but are in the background (i.e. not the active one),
	// then source pixels map from the local logical palette to the system one
	if (_paletteRealized && m_bForceBackground) {
		srcPal = dynamic_cast<Graphics::Palette *>(_palette);
		destPal = dynamic_cast<Graphics::Palette *>(AfxGetApp()->getCurrentPalette());
	}
	// If we haven't realized our palette locally, or the source bitmap hasn't had any
	// palette at all set, then return null indicating no palette mapping will occur
	else if (!_paletteRealized || !srcImpl->_hasLogicalPalette)
		return nullptr;
	else {
		srcPal = dynamic_cast<Graphics::Palette *>(srcImpl->_palette);
		destPal = dynamic_cast<Graphics::Palette *>(_palette);
	}

	if (!srcPal || srcPal->empty() || !destPal || destPal->empty())
		return nullptr;
	assert(srcPal->size() == destPal->size());

	// Create the map
	Graphics::PaletteLookup palLookup(destPal->data(), destPal->size());
	return palLookup.createMap(srcPal->data(), srcPal->size());
}

void CDC::Impl::moveTo(int x, int y) {
	_linePos.x = x;
	_linePos.y = y;
}

void CDC::Impl::lineTo(int x, int y) {
	Gfx::Surface *dest = getSurface();
	uint color = getPenColor();

	dest->drawLine(_linePos.x, _linePos.y, x, y, color);
	_linePos.x = x;
	_linePos.y = y;
}

COLORREF CDC::Impl::getPixel(int x, int y) const {
	Gfx::Surface *src = getSurface();
	assert(src->format.bytesPerPixel == 1);
	const byte pixel = src->getPixel(x, y);

	assert(_palette);
	const auto *pal = static_cast<const CPalette::Impl *>(_palette);
	byte r, g, b;
	pal->get(pixel, r, g, b);

	return RGB(r, g, b);
}

int CDC::Impl::setROP2(int nDrawMode) {
	int oldMode = _drawMode;
	_drawMode = nDrawMode;
	return oldMode;
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

bool CDC::Impl::textOut(int x, int y, const char *lpszString, int nCount,
		int nTabPositions, const int *lpnTabStopPositions,
		int nTabOrigin, CSize *size) {
	Gfx::Surface *dest = getSurface();
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

bool CDC::Impl::textOut(int x, int y, const CString &str,
		int nTabPositions, const int *lpnTabStopPositions,
		int nTabOrigin, CSize *size) {
	Gfx::Surface *dest = getSurface();
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

bool CDC::Impl::extTextOut(int x, int y, unsigned int nOptions, LPCRECT lpRect,
		const char *lpszString, unsigned int nCount, int *lpDxWidths) {
	error("TODO: extTextOut");
}

bool CDC::Impl::extTextOut(int x, int y, unsigned int nOptions, LPCRECT lpRect,
		const CString &str, int *lpDxWidths) {
	error("TODO: extTextOut");
}

CSize CDC::Impl::tabbedTextOut(int x, int y, const char *lpszString, int nCount,
		int nTabPositions, const int *lpnTabStopPositions, int nTabOrigin) {
	CString str(lpszString, nCount);

	CSize size;
	textOut(x, y, str.c_str(), str.size(),
		nTabPositions, lpnTabStopPositions, nTabOrigin, &size);

	return size;
}

CSize CDC::Impl::tabbedTextOut(int x, int y, const CString &str,
		int nTabPositions, const int *lpnTabStopPositions, int nTabOrigin) {
	CSize size;
	textOut(x, y, str, nTabPositions, lpnTabStopPositions, nTabOrigin, &size);
	return size;
}

int CDC::Impl::drawText(const char *lpszString, int nCount,
		LPRECT lpRect, unsigned int nFormat, int nTabPositions,
		const int *lpnTabStopPositions, int nTabOrigin,
		CSize *size) {
	return drawText(CString(lpszString, nCount),
		lpRect, nFormat, nTabPositions,
		lpnTabStopPositions, nTabOrigin, size);
}

int CDC::Impl::drawText(const CString &str, LPRECT lpRect, unsigned int nFormat,
		int nTabPositions, const int *lpnTabStopPositions,
		int nTabOrigin, CSize *size) {
	Graphics::Font *font = *(CFont::Impl *)_font;
	Gfx::Surface *dest = getSurface();
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

CSize CDC::Impl::getTextExtent(const char *lpszString, int nCount) const {
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

CSize CDC::Impl::getOutputTextExtent(const char *lpszString, int nCount) const {
	// TODO: Proper implementation that handles tabs, etc.
	return getTextExtent(lpszString, nCount);
}

CSize CDC::Impl::getOutputTextExtent(const CString &str) const {
	// TODO: Proper implementation that handles tabs, etc.
	return getTextExtent(str);
}

CSize CDC::Impl::getTabbedTextExtent(const char *lpszString, int nCount,
		int nTabPositions, int *lpnTabStopPositions) const {
	// TODO: Proper implementation that handles tabs, etc.
	return getTextExtent(lpszString, nCount);
}

CSize CDC::Impl::getTabbedTextExtent(const CString &str,
	int nTabPositions, int *lpnTabStopPositions) const {
	error("TODO");
}

CSize CDC::Impl::getOutputTabbedTextExtent(const char *lpszString, int nCount,
	int nTabPositions, int *lpnTabStopPositions) const {
	error("TODO");
}

CSize CDC::Impl::getOutputTabbedTextExtent(const CString &str,
	int nTabPositions, int *lpnTabStopPositions) const {
	error("TODO");
}

bool CDC::Impl::grayString(CBrush *pBrush,
	bool(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
	int nCount, int x, int y, int nWidth, int nHeight) {
	error("TODO");
}

unsigned int CDC::Impl::getTextAlign() const {
	return _textAlign;
}

unsigned int CDC::Impl::setTextAlign(unsigned int nFlags) {
	unsigned int oldAlign = _textAlign;
	_textAlign = nFlags;
	return oldAlign;
}

bool CDC::Impl::getTextMetrics(LPTEXTMETRIC lpMetrics) const {
	TEXTMETRIC &tm = *lpMetrics;
	Gfx::Font *font = *(CFont::Impl *)_font;

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
} // namespace Graphics
