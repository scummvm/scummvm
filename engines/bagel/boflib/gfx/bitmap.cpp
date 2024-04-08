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

#include "common/file.h"
#include "common/system.h"
#include "image/bmp.h"
#include "image/png.h"

#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/file.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

#define TEST_DEBUG 0

BOOL CBofBitmap::m_bUseBackdrop = FALSE;

extern BOOL g_bRealizePalette;

CBofBitmap::CBofBitmap() {
	m_szFileName[0] = '\0';
}

CBofBitmap::CBofBitmap(INT dx, INT dy, CBofPalette *pPalette, BOOL bOwnPalette, byte *pPrivateBuff) {
	Assert((dx > 0) && (dy > 0));

	// use application's palette if not supplied
	//
	if (pPalette == nullptr) {
		bOwnPalette = FALSE;
		pPalette = CBofApp::GetApp()->GetPalette();
	}

	// Inits
	//
	m_nDX = dx;
	m_nDY = dy;
	m_nScanDX = ((dx + 3) & ~3);
	m_bTopDown = TRUE;
	m_bOwnPalette = bOwnPalette;
	m_bReadOnly = FALSE;
	m_szFileName[0] = '\0';
	m_pPalette = nullptr;
	m_bInitialized = TRUE;

	// allow privatization of the bitmap (used only on mac from displaytextex).;
	m_bPrivateBmp = (pPrivateBuff != nullptr);
	if (m_bPrivateBmp == TRUE) {
		m_pBits = pPrivateBuff;

		_bitmap.w = dx;
		_bitmap.h = dy;
		_bitmap.pitch = m_nScanDX;
		_bitmap.format = Graphics::PixelFormat::createFormatCLUT8();
		_bitmap.setPixels(pPrivateBuff);

	} else {
		m_pBits = nullptr;
	}

	// Fill the info structure
	m_cBitmapInfo.m_cInfoHeader.biSize = sizeof(BOFBITMAPINFOHEADER);
	m_cBitmapInfo.m_cInfoHeader.biWidth = dx;
	m_cBitmapInfo.m_cInfoHeader.biHeight = dy;
	m_cBitmapInfo.m_cInfoHeader.biPlanes = 1;
	m_cBitmapInfo.m_cInfoHeader.biBitCount = 8;
	m_cBitmapInfo.m_cInfoHeader.biCompression = 0;
	m_cBitmapInfo.m_cInfoHeader.biSizeImage = 0;
	m_cBitmapInfo.m_cInfoHeader.biXPelsPerMeter = 0;
	m_cBitmapInfo.m_cInfoHeader.biYPelsPerMeter = 0;
	m_cBitmapInfo.m_cInfoHeader.biClrUsed = 0;
	m_cBitmapInfo.m_cInfoHeader.biClrImportant = 0;

#if BOF_WINDOWS && !BOF_WINMAC

	m_hBitmap = nullptr;
	m_hDC = nullptr;
	m_hOldBmp = nullptr;
	m_hPalOld = nullptr;

#elif BOF_MAC || BOF_WINMAC
#if COPYBITS
	FlipVerticalFast();         // mac bitmaps are read in top down
#endif
#endif

	m_pPalette = pPalette;
	Load();
}

CBofBitmap::CBofBitmap(const CHAR *pszFileName, CBofPalette *pPalette, BOOL bOwnPalette) {
	Assert(pszFileName != nullptr);
	m_bOwnPalette = bOwnPalette;
	m_szFileName[0] = '\0';
	m_bInitialized = TRUE;

	if (pPalette == nullptr) {
		if ((pPalette = new CBofPalette(pszFileName)) != nullptr) {
			m_bOwnPalette = TRUE;
		}
	}

	// Init the info needed to load a bitmap from disk
	m_pPalette = pPalette;

	if (FileGetFullPath(m_szFileName, pszFileName) != nullptr) {
		// load this bitmap into the cache
		Load();
	} else {
		ReportError(ERR_FFIND, "Could not build full path to %s", pszFileName);
	}
}

CBofBitmap::~CBofBitmap() {
	Assert(IsValidObject(this));

	Release();

	if (m_bOwnPalette && (m_pPalette != nullptr)) {
		delete m_pPalette;
		m_bOwnPalette = FALSE;
	}

	m_pPalette = nullptr;
	m_bInitialized = FALSE;
}

ERROR_CODE CBofBitmap::BuildBitmap(CBofPalette *pPalette) {
	Assert(IsValidObject(this));
	Assert(pPalette != nullptr);

	if (m_errCode == ERR_NONE) {

		_bitmap.create(m_nDX, m_nDY, Graphics::PixelFormat::createFormatCLUT8());
		m_pBits = (byte *)_bitmap.getBasePtr(0, 0);
		m_nScanDX = _bitmap.pitch;

		// set this bitmap's palette
		SetPalette(pPalette, m_bOwnPalette);

#if BOF_WINNT && !BOF_WINMAC

		HPALETTE hPal;
		HDC hDC;

		if ((hDC = ::GetDC(nullptr)) != nullptr) {

			if ((hPal = ::SelectPalette(hDC, (HPALETTE)pPalette->GetPalette(), FALSE)) == nullptr) {
				LogError("::SelectPalette() failed");
			}

			::RealizePalette(hDC);

			// set this bitmap's palette
			SetPalette(pPalette, m_bOwnPalette);

			// make sure this DIB is top down
			// m_cBitmapInfo.m_cInfoHeader.biHeight = -ABS(m_cBitmapInfo.m_cInfoHeader.biHeight);

			// Can't already have a bitmap handle
			// unless we already have a private bmp.
			Assert(m_hBitmap == nullptr);
			Assert(m_pBits == nullptr);

			if ((m_hBitmap = CreateDIBSection(hDC, (BITMAPINFO *)&m_cBitmapInfo.m_cInfoHeader, DIB_RGB_COLORS, (void **)&m_pBits, nullptr, nullptr)) != nullptr) {

				m_nDX = m_cBitmapInfo.m_cInfoHeader.biWidth;
				m_nDY = ABS(m_cBitmapInfo.m_cInfoHeader.biHeight);
				m_nScanDX = (m_nDX + 3) & ~3;
				m_bTopDown = (m_cBitmapInfo.m_cInfoHeader.biHeight < 0);

				// Can't already have a device context for this bitmap
				Assert(m_hDC == nullptr);

				if ((m_hDC = ::CreateCompatibleDC(nullptr)) != nullptr) {

					if ((m_hOldBmp = (HBITMAP)::SelectObject(m_hDC, m_hBitmap)) == nullptr) {
						LogError("::SelectObject() failed");
					}

					Assert(m_pPalette != nullptr);

					// Commented out because this was the cause of the MASSIVE
					// GDI resource leak.
					//
					/*if ((m_hPalOld = ::SelectPalette(m_hDC, m_pPalette->GetPalette(), FALSE)) == nullptr) {
					    LogError("::SelectPalette() failed");
					}
					if (::RealizePalette(m_hDC) == GDI_ERROR) {
					    LogError("::RealizePalette() failed");
					}*/

				} else {
					ReportError(ERR_MEMORY, "CreateCompatibleDC() failed");
				}

			} else {
				ReportError(ERR_MEMORY, "CreateDIBSection failed");
			}

			if (::SelectPalette(hDC, hPal, FALSE) == nullptr) {
				LogError("::SelectPalette() failed");
			}

			::ReleaseDC(nullptr, hDC);
		}

#elif BOF_MAC || BOF_WINMAC || BOF_WIN16

		// set this bitmap's palette
		SetPalette(pPalette, m_bOwnPalette);

		m_nDX = (INT)m_cBitmapInfo.m_cInfoHeader.biWidth;
		m_nDY = (INT)ABS(m_cBitmapInfo.m_cInfoHeader.biHeight);
		m_nScanDX = (m_nDX + 3) & ~3;
		m_bTopDown = (m_cBitmapInfo.m_cInfoHeader.biHeight < 0);
		if (m_bPrivateBmp == FALSE) {
			if ((m_pBits = (byte *)BofAlloc((int32)m_nScanDX * m_nDY)) != nullptr) {
			} else {
				ReportError(ERR_MEMORY, "Unable to allocate %ld bytes for m_pBits", (int32)m_nScanDX * m_nDY);
			}
		} else {
			Assert(m_pBits != nullptr);
		}

#endif // BOF_MAC
	}
	return m_errCode;
}

BOOL CBofBitmap::Alloc() {
	if (m_szFileName[0] != '\0') {
		LoadBitmap(m_szFileName, m_pPalette);
	} else {
		BuildBitmap(m_pPalette);
	}

	return !ErrorOccurred();
}

void CBofBitmap::Free() {
	ReleaseBitmap();
}

void CBofBitmap::ReleaseBitmap() {
	Assert(IsValidObject(this));

	_bitmap.clear();
}

ERROR_CODE CBofBitmap::LoadBitmap(const CHAR *pszFileName, CBofPalette *pPalette) {
	Assert(IsValidObject(this));
	Assert(pszFileName != nullptr);
	Assert(pPalette != nullptr);

	// release any previous bitmap info
	ReleaseBitmap();

	if (m_errCode == ERR_NONE) {
		CBofFile *pFile;

		// Open bitmap
		if ((pFile = new CBofFile(pszFileName, CBOFFILE_READONLY)) != nullptr) {
			// filename must fit into our buffer
			Assert(strlen(pszFileName) < MAX_FNAME);

			// keep track of this filename
			Common::strcpy_s(m_szFileName, pszFileName);

			// Decode the bitmap
			Image::BitmapDecoder decoder;
			Common::SeekableReadStream *rs = *pFile;
			if (!rs || !decoder.loadStream(*rs))
				error("Could not load bitmap");

			// Load up the decoded bitmap
			_bitmap.copyFrom(*decoder.getSurface());

			// Load the bitmap palette
			// TODO: I think the bitmap's palette is used to remap the bitmap
			// to the globally active palette
			_bitmap.setPalette(decoder.getPalette(), 0, PALETTE_COUNT);

			m_nDX = _bitmap.w;
			m_nDY = _bitmap.h;
			m_nScanDX = _bitmap.pitch;
			m_pBits = (byte*)_bitmap.getBasePtr(0, 0);

			// Close bitmap-file
			delete pFile;

		} else {
			ReportError(ERR_MEMORY, "Could not allocate a CBofFile for %s", pszFileName);
		}
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::Paint(CBofWindow *pWnd, INT x, INT y, CBofRect *pSrcRect, INT nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

	CBofRect cRect(x, y, x + m_nDX - 1, y + m_nDY - 1);

	if (pSrcRect != nullptr) {
		cRect.SetRect(x, y, x + pSrcRect->Width() - 1, y + pSrcRect->Height() - 1);
	}

	return Paint(pWnd, &cRect, pSrcRect, nMaskColor);
}

ERROR_CODE CBofBitmap::Paint(CBofWindow *pWnd, CBofRect *pDstRect, CBofRect *pSrcRect, INT nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

#if PALETTESHIFTFIX
	CBofWindow::CheckPaletteShiftList();
#endif

	Graphics::ManagedSurface *dstSurf = pWnd->getSurface();
	CBofRect cDestRect(0, 0, m_nDX - 1, m_nDY - 1), cSourceRect(0, 0, m_nDX - 1, m_nDY - 1);

	if (m_errCode == ERR_NONE) {
		if (pDstRect != nullptr) {
			Assert((pDstRect->Width() > 0) && (pDstRect->Height() > 0));
			cDestRect = *pDstRect;
		}

		if (pSrcRect != nullptr) {
			Assert((pSrcRect->Width() > 0) && (pSrcRect->Height() > 0));
			cSourceRect = *pSrcRect;
		}

		if (_bitmap.format.bytesPerPixel == 1) {
			// Bitmap is paletted, so ensure it's palette is updated
			const HPALETTE &pal = m_pPalette->GetPalette();
			_bitmap.setPalette(pal._data, 0, pal._numColors);
		}

		// Handle the blitting
		if (nMaskColor == NOT_TRANSPARENT) {
			dstSurf->blitFrom(_bitmap, cSourceRect, cDestRect);
		} else {
			dstSurf->transBlitFrom(_bitmap, cSourceRect, cDestRect, nMaskColor);
		}
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::PaintMaskBackdrop(CBofWindow *pWnd, CBofRect *pDstRect, CBofRect *pSrcRect, INT nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

	// This function requires the use of a backdrop
	Assert(pWnd->GetBackdrop() != nullptr);

	CBofRect cDestRect(0, 0, m_nDX - 1, m_nDY - 1), cSourceRect(0, 0, m_nDX - 1, m_nDY - 1);

	if (m_errCode == ERR_NONE) {

		if (pDstRect != nullptr) {
			Assert((pDstRect->Width() > 0) && (pDstRect->Height() > 0));
			cDestRect = *pDstRect;
		}

		if (pSrcRect != nullptr) {
			Assert((pSrcRect->Width() > 0) && (pSrcRect->Height() > 0));

			cSourceRect = *pSrcRect;
		}

		CBofBitmap cTempBitmap(cSourceRect.Width(), cSourceRect.Height(), m_pPalette);
		CBofBitmap *pBackdrop;

		//cTempBitmap.CaptureScreen(pWnd, &cDestRect);
		//
		// Use the backdrop to get the background instead of trying to
		// capture the screen
		//
		if ((pBackdrop = pWnd->GetBackdrop()) != nullptr) {

			CBofRect cTempRect;
			cTempRect = cTempBitmap.GetRect();

			pBackdrop->Paint(&cTempBitmap, &cTempRect, &cDestRect);
		}

		Paint(&cTempBitmap, 0, 0, &cSourceRect, nMaskColor);

		cTempBitmap.Paint(pWnd, &cDestRect);

		UnLock();
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::Paint(CBofBitmap *pBmp, INT x, INT y, CBofRect *pSrcRect, INT nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	CBofRect cRect(x, y, x + m_nDX - 1, y + m_nDY - 1);

	if (pSrcRect != nullptr) {
		cRect.SetRect(x, y, x + pSrcRect->Width() - 1, y + pSrcRect->Height() - 1);
	}

	return Paint(pBmp, &cRect, pSrcRect, nMaskColor);
}

ERROR_CODE CBofBitmap::Paint(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect, INT nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	CBofRect cDestRect(0, 0, m_nDX - 1, m_nDY - 1), cSourceRect(0, 0, m_nDX - 1, m_nDY - 1);
	CBofRect cClipRect;

	if (!ErrorOccurred() && !pBmp->ErrorOccurred()) {
		if (pDstRect != nullptr) {
			Assert((pDstRect->Width() > 0) && (pDstRect->Height() > 0));
			cDestRect = *pDstRect;
		}

		if (pSrcRect != nullptr) {
			Assert((pSrcRect->Width() > 0) && (pSrcRect->Height() > 0));
			cSourceRect = *pSrcRect;
		}

		// If painting entirely outside destination bitmap, then no need to paint.
		CBofRect cRect = pBmp->GetRect();
		if (cClipRect.IntersectRect(&cRect, &cDestRect) == 0) {
			return m_errCode;
		}

		// Clip so we don't paint outside of the bitmap's memory buffer
		cSourceRect.left += cClipRect.left - cDestRect.left;
		cSourceRect.right += cClipRect.right - cDestRect.right;
		cSourceRect.top += cClipRect.top - cDestRect.top;
		cSourceRect.bottom += cClipRect.bottom - cDestRect.bottom;
		cDestRect = cClipRect;

		// If painting from entirely outside this bitmap, then don't paint at all.
		cRect = GetRect();
		if (cClipRect.IntersectRect(&cRect, &cSourceRect) == 0) {
			return m_errCode;
		}

		// Clip so we don't paint outside of the bitmap's memory buffer
		cDestRect.left += cClipRect.left - cSourceRect.left;
		cDestRect.right += cClipRect.right - cSourceRect.right;
		cDestRect.top += cClipRect.top - cSourceRect.top;
		cDestRect.bottom += cClipRect.bottom - cSourceRect.bottom;
		cSourceRect = cClipRect;

		// Lock these bitmaps down so we can start painting
		Lock();
		pBmp->Lock();

		// Do the actual painting.
		// Since we are copying from bitmap to bitmap, using the
		// blit bitmap routine is a kosher thing to do, there is no screen drawing
		if (nMaskColor == NOT_TRANSPARENT) {
			pBmp->_bitmap.blitFrom(_bitmap, cSourceRect, cDestRect);
		} else {
			pBmp->_bitmap.transBlitFrom(_bitmap, cSourceRect, cDestRect, nMaskColor);
		}

		// don't need a lock on these guys anymore
		pBmp->UnLock();
		UnLock();
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::Paint1To1(CBofBitmap *pBmp) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	if (!ErrorOccurred() && !pBmp->ErrorOccurred()) {

		Lock();
		pBmp->Lock();

		// Direct 1 to 1 copy
		BofMemCopy(pBmp->m_pBits, m_pBits, m_nScanDX * m_nDY);

		pBmp->UnLock();
		UnLock();
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::PaintStretch4(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);
	Assert(pDstRect != nullptr);
	Assert(pSrcRect != nullptr);

	// These bitmaps MUST be locked down before hand
	Assert(IsLocked());
	Assert(pBmp->IsLocked());

	if (m_errCode == ERR_NONE) {
		int32  dy, x1, y1, x2, y2, nDstHeight;
		byte *pSrcBits, *pDestBits;
		INT dy1, dx1, dy2, dx2;

		dy1 = m_nDY;
		dx1 = m_nScanDX;

		dy2 = pBmp->m_nDY;
		dx2 = pBmp->m_nScanDX;

		pDestBits = pBmp->m_pBits;
		pSrcBits = m_pBits;

		dy = pSrcRect->Height();

		nDstHeight = pDstRect->Height();

		x1 = pSrcRect->left;
		y1 = pSrcRect->top;

		x2 = pDstRect->left;
		y2 = pDstRect->top;

		Fixed SourceStepY, PosY;

		if (m_bTopDown) {
			pSrcBits += y1 * dx1 + x1;
		} else {
			pSrcBits += (dy1 - y1 - 1) * dx1 + x1;
			dx1 = -dx1;
		}

		if (pBmp->m_bTopDown) {
			pDestBits += y2 * dx2 + x2;
		} else {
			pDestBits += (dy2 - y2 - 1) * dx2 + x2;
			dx2 = -dx2;
		}

		SourceStepY = FixedDivide(IntToFixed(dy), IntToFixed(nDstHeight));

		PosY = 0;
		byte *pSrcEnd = pSrcBits + (dy - 1) * dx1;
		byte *pDestEnd = pDestBits + (nDstHeight - 1) * dx2;

		nDstHeight >>= 1;

		while (nDstHeight > 0) {
			nDstHeight -= 4;

			if (PosY >= 0x00010000) {

				pSrcBits += dx1;
				pSrcEnd -= dx1;

				PosY &= 0x0000FFFF;
			}
			*(uint32 *)pDestBits = *(uint32 *)pSrcBits;
			*(uint32 *)pDestEnd = *(uint32 *)pSrcEnd;

			PosY += SourceStepY;

			pDestBits += dx2;
			pDestEnd -= dx2;

			if (PosY >= 0x00010000) {

				pSrcBits += dx1;
				pSrcEnd -= dx1;

				PosY &= 0x0000FFFF;
			}
			*(uint32 *)pDestBits = *(uint32 *)pSrcBits;
			*(uint32 *)pDestEnd = *(uint32 *)pSrcEnd;

			PosY += SourceStepY;

			pDestBits += dx2;
			pDestEnd -= dx2;

			if (PosY >= 0x00010000) {

				pSrcBits += dx1;
				pSrcEnd -= dx1;

				PosY &= 0x0000FFFF;
			}
			*(uint32 *)pDestBits = *(uint32 *)pSrcBits;
			*(uint32 *)pDestEnd = *(uint32 *)pSrcEnd;

			PosY += SourceStepY;

			pDestBits += dx2;
			pDestEnd -= dx2;

			if (PosY >= 0x00010000) {

				pSrcBits += dx1;
				pSrcEnd -= dx1;

				PosY &= 0x0000FFFF;
			}
			*(uint32 *)pDestBits = *(uint32 *)pSrcBits;
			*(uint32 *)pDestEnd = *(uint32 *)pSrcEnd;

			PosY += SourceStepY;

			pDestBits += dx2;
			pDestEnd -= dx2;
		}
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::PaintStretchOpt(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect, INT nOptSize) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);
	Assert(pDstRect != nullptr);
	Assert(pSrcRect != nullptr);

	// These bitmaps MUST be locked down before hand
	Assert(IsLocked());
	Assert(pBmp->IsLocked());

	if (m_errCode == ERR_NONE) {
		int32  dy, x1, y1, x2, y2, nDstHeight;
		byte *pSrcBits, *pDestBits;
		INT dy1, dx1, dy2, dx2;

		dy1 = m_nDY;
		dx1 = m_nScanDX;

		dy2 = pBmp->m_nDY;
		dx2 = pBmp->m_nScanDX;

		pDestBits = pBmp->m_pBits;
		pSrcBits = m_pBits;

		dy = pSrcRect->Height();

		nDstHeight = pDstRect->Height();

		x1 = pSrcRect->left;
		y1 = pSrcRect->top;

		x2 = pDstRect->left;
		y2 = pDstRect->top;

		if (m_bTopDown) {
			pSrcBits += y1 * dx1 + x1;
		} else {
			pSrcBits += (dy1 - y1 - 1) * dx1 + x1;
			dx1 = -dx1;
		}

		if (pBmp->m_bTopDown) {
			pDestBits += y2 * dx2 + x2;
		} else {
			pDestBits += (dy2 - y2 - 1) * dx2 + x2;
			dx2 = -dx2;
		}

		Fixed SourceStepY, PosY;

		SourceStepY = FixedDivide(IntToFixed(dy), IntToFixed(nDstHeight));

		int32 lInc;
		byte *pSrcEnd, *pDestEnd;
		INT nMod, i;
		PosY = 0;
		pSrcEnd = pSrcBits + (dy - 1) * dx1;
		pDestEnd = pDestBits + (nDstHeight - 1) * dx2;

		nMod = (INT)nDstHeight & 1;
		nDstHeight >>= 1;

		while (nDstHeight-- > 0) {

			if (PosY >= 0x00010000) {

				lInc = FixedToInt(PosY) * dx1;
				pSrcBits += lInc;
				pSrcEnd -= lInc;

				PosY &= 0x0000FFFF;
			}

			for (i = 0; i < nOptSize; i += 4) {
				*(uint32 *)(pDestBits + i) = *(uint32 *)(pSrcBits + i);
				*(uint32 *)(pDestEnd + i) = *(uint32 *)(pSrcEnd + i);
			}

			PosY += SourceStepY;

			pDestBits += dx2;
			pDestEnd -= dx2;
		}

		if (nMod != 0) {

			*(uint32 *)pDestEnd = *(uint32 *)pSrcEnd;
		}
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::CaptureScreen(CBofWindow *pWnd, CBofRect *pSrcRect, CBofRect *pDstRect) {
	Assert(IsValidObject(this));

	Assert(pWnd != nullptr);
	Assert(pSrcRect != nullptr);

	CBofRect cDestRect(0, 0, m_nDX - 1, m_nDY - 1);
	CBofRect cSrcRect(0, 0, m_nDX - 1, m_nDY - 1);

	if (m_errCode == ERR_NONE) {
		if (pDstRect != nullptr) {
			cDestRect = *pDstRect;
		}

		cSrcRect = *pSrcRect;

		CBofBitmap *pBackdrop;

		pBackdrop = pWnd->GetBackdrop();

		// If we're capturing the screen, we have to convert the format first.
		if (!m_bUseBackdrop || pBackdrop == nullptr) {
			Graphics::Surface tmp;
			tmp.copyFrom(*pWnd->getSurface());
			_bitmap.blitFrom(tmp.convertTo(_bitmap.format, 0, 0, m_pPalette->GetData(), PALETTE_COUNT),
				cSrcRect,
				cDestRect);
			tmp.free();

		// Optimization to use the window's backdrop bitmap instead of doing
		// an actual screen capture.
		//
		} else {
			Assert(pBackdrop != nullptr);
			pBackdrop->Paint(this, &cDestRect, &cSrcRect);
		}
	}

	return m_errCode;
}

void CBofBitmap::SetPalette(CBofPalette *pBofPalette, BOOL bOwnPalette) {
	Assert(IsValidObject(this));
	Assert(pBofPalette != nullptr);

	if (m_errCode == ERR_NONE) {
		if (pBofPalette != nullptr) {
			if (m_bOwnPalette && (m_pPalette != nullptr) && (m_pPalette != pBofPalette)) {
				delete m_pPalette;
			}

			m_bOwnPalette = bOwnPalette;
			m_pPalette = pBofPalette;

			_bitmap.setPalette(m_pPalette->GetData(), 0, PALETTE_COUNT);

#if BOF_WINDOWS

			HPALETTE hPalette;

			if ((hPalette = pBofPalette->GetPalette()) != nullptr) {

				// Another GDI Resource leak
				//
#if 0 //!BOF_WINMAC
				if (m_hDC != nullptr) {
					if (::SelectPalette(m_hDC, hPalette, FALSE) == nullptr) {
						LogError("::SelectPalette() failed");
					}
					if (::RealizePalette(m_hDC) == GDI_ERROR) {
						LogError("::RealizePalette() failed");
					}
				}
#endif

				PALETTEENTRY  pe[256];
				INT           i;
				INT           nDibColors;
				RGBQUAD      *pRgb;

				pRgb = &m_cBitmapInfo.m_cRgbValues[0];

				nDibColors = 1 << m_cBitmapInfo.m_cInfoHeader.biBitCount;

				GetPaletteEntries(hPalette, 0, nDibColors, &pe[0]);

				//
				// Now copy the RGBs in the logical palette to the dib color table
				//
				for (i = 0; i < nDibColors; i++) {

					pRgb->rgbRed      = pe[i].peRed;
					pRgb->rgbGreen    = pe[i].peGreen;
					pRgb->rgbBlue     = pe[i].peBlue;
					pRgb->rgbReserved = (byte)0;

					pRgb++;
				}
			}
#endif
		}
	}
}

void CBofBitmap::ReMapPalette(CBofPalette *pBofPalette) {
	Assert(IsValidObject(this));
	Assert(pBofPalette != nullptr);

	if (m_errCode == ERR_NONE) {

#if BOF_WINDOWS

		HPALETTE hPalette;

		if ((hPalette = pBofPalette->GetPalette()) != nullptr) {

			Lock();

			Assert(m_pBits != nullptr);

			byte                xlat[256];
			RGBQUAD            *pRgb;
			BITMAPINFOHEADER   *pBmpInfo;
			byte              *pBits;
			int32                lBufSize;
			int32                n, i;
			INT                 nDibColors;
			INT                 nPalColors = 0;

			pBmpInfo = &m_cBitmapInfo.m_cInfoHeader;

			pRgb = &m_cBitmapInfo.m_cRgbValues[0];

			GetObject(hPalette, sizeof(int), (LPSTR)&nPalColors);
			nDibColors = 1 << pBmpInfo->biBitCount;

			if ((lBufSize = (int32)pBmpInfo->biSizeImage) == 0)
				lBufSize = (int32)m_nScanDX * m_nDY;

			/*
			*   build a xlat table. from the current DIB colors to the given
			*   palette.
			*/
			for (n = 0; n < nDibColors; n++) {
				xlat[n] = (byte)GetNearestPaletteIndex(hPalette, RGB(pRgb->rgbRed, pRgb->rgbGreen, pRgb->rgbBlue));
				pRgb++;
			}

			pBmpInfo->biClrUsed = 0;

			/*
			* translate the DIB bits
			*/

			// We do not support RLE compression
			Assert(pBmpInfo->biCompression == 0);

			pBits = m_pBits;
			for (i = 0; i < lBufSize; i++, pBits++)
				*pBits = xlat[*pBits];

			SetPalette(pBofPalette, ((pBofPalette == m_pPalette) ? m_bOwnPalette : FALSE));

			UnLock();
		}
#elif BOF_MAC

#endif
	}
}

void CBofBitmap::FloodFill(INT /*x*/, INT /*y*/, byte /*iFillColor*/) {
	Assert(IsValidObject(this));

	// This function needs to be finished
	LogWarning("CBofBitmap::FloodFill has not been written yet");
}

byte *CBofBitmap::GetPixelAddress(INT x, INT y) {
	Assert(IsValidObject(this));

	// you can not call this function unless you manually lock this bitmap
	Assert(IsLocked());

	// The pixel in question must be in the bitmap area
	Assert(GetRect().PtInRect(CBofPoint(x, y)));

	return (byte *)_bitmap.getBasePtr(x, y);

#if 0
	int32 lOffset;

	if (m_bTopDown) {
		lOffset = (int32)y * m_nScanDX + x;
	} else {
		lOffset = (int32)(m_nDY - y - 1) * m_nScanDX + x;
	}
	Assert(lOffset >= 0);

	return m_pBits + lOffset;
#endif
}

byte CBofBitmap::ReadPixel(CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	return ReadPixel(pPoint->x, pPoint->y);
}

byte CBofBitmap::ReadPixel(INT x, INT y) {
	Assert(IsValidObject(this));

	byte chPixel;

	Lock();

	chPixel = *GetPixelAddress(x, y);

	UnLock();

	return chPixel;
}

void CBofBitmap::WritePixel(CBofPoint *pPoint, byte iColor) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	WritePixel(pPoint->x, pPoint->y, iColor);
}

void CBofBitmap::WritePixel(INT x, INT y, byte iColor) {
	Assert(IsValidObject(this));

	Lock();

	byte *pPixel;

	pPixel = GetPixelAddress(x, y);

	*pPixel = iColor;

	UnLock();
}

void CBofBitmap::Circle(INT xCenter, INT yCenter, uint16 nRadius, byte iColor) {
	Assert(IsValidObject(this));

	if (m_errCode == ERR_NONE) {

		INT i, x, y;

		x = 0;
		i = 3 - 2 * (y = nRadius);

		while (x < y) {

			WritePixel(xCenter + x, yCenter + y, iColor);
			WritePixel(xCenter - x, yCenter + y, iColor);
			WritePixel(xCenter + y, yCenter + x, iColor);
			WritePixel(xCenter - y, yCenter + x, iColor);
			WritePixel(xCenter - x, yCenter - y, iColor);
			WritePixel(xCenter + x, yCenter - y, iColor);
			WritePixel(xCenter - y, yCenter - x, iColor);
			WritePixel(xCenter + y, yCenter - x, iColor);

			if (i <= 0) {
				i += 4 * x + 6;

			} else {

				i +=  4 * (x - y) + 10;
				y--;
			}
			x++;
		}

		if (x == y) {
			WritePixel(xCenter + x, yCenter + y, iColor);
			WritePixel(xCenter - x, yCenter + y, iColor);
			WritePixel(xCenter + y, yCenter + x, iColor);
			WritePixel(xCenter - y, yCenter + x, iColor);
			WritePixel(xCenter - x, yCenter - y, iColor);
			WritePixel(xCenter + x, yCenter - y, iColor);
			WritePixel(xCenter - y, yCenter - x, iColor);
			WritePixel(xCenter + y, yCenter - x, iColor);
		}
	}
}

void CBofBitmap::Circle(CBofPoint *pCenter, uint16 nRadius, byte iColor) {
	Assert(IsValidObject(this));

	Assert(pCenter != nullptr);

	Circle(pCenter->x, pCenter->y, nRadius, iColor);
}

void CBofBitmap::FillCircle(INT x, INT y, uint16 nRadius, byte iColor) {
	Assert(IsValidObject(this));

	Circle(x, y, nRadius, iColor);

	// still need to fill it
}

void CBofBitmap::FillCircle(CBofPoint *pCenter, uint16 nRadius, byte iColor) {
	Assert(IsValidObject(this));
	Assert(pCenter != nullptr);

	FillCircle(pCenter->x, pCenter->y, nRadius, iColor);
}

void CBofBitmap::DrawRect(CBofRect *pRect, byte iColor) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	if (m_errCode == ERR_NONE) {

		Line(pRect->left, pRect->top, pRect->right, pRect->top, iColor);
		Line(pRect->right, pRect->top, pRect->right, pRect->bottom, iColor);
		Line(pRect->right, pRect->bottom, pRect->left, pRect->bottom, iColor);
		Line(pRect->left, pRect->bottom, pRect->left, pRect->top, iColor);
	}
}

void CBofBitmap::FillRect(CBofRect *pRect, byte iColor) {
	Assert(IsValidObject(this));

	if (m_errCode == ERR_NONE) {

		Lock();

		if (pRect == nullptr) {
			_bitmap.clear(iColor);
		} else {
			Common::Rect rect(pRect->left, pRect->top, pRect->right, pRect->bottom);
			_bitmap.fillRect(rect, iColor);
		}
#if 0
		// if entire bitmap
		//
		if (pRect == nullptr) {
			Assert(m_pBits != nullptr);
			BofMemSet(m_pBits, iColor, (int32)m_nScanDX * m_nDY);

		} else {
			CBofRect cRect;
			byte *pSrcBits;
			INT y, x1, y1, dx, dy, dx1;

			// Clip to my rectangle
			//
			if (cRect.IntersectRect(*pRect, GetRect())) {

				dx = cRect.Width();
				dy = cRect.Height();

				x1 = cRect.left;
				y1 = cRect.top;

				dx1 = m_nScanDX;

				pSrcBits = GetPixelAddress(x1, y1);

				if (!m_bTopDown) {
					dx1 = -dx1;
				}

				for (y = 0; y < dy; y++) {

					BofMemSet(pSrcBits, iColor, dx);

					pSrcBits += dx1;
				}
			}
		}
		UnLock();

		// Make our bitmaps top down!!!!!!!!!!!!!!!!!!
#if BOF_MAC && COPYBITS
		Assert(m_bTopDown == true);
#endif

#endif
	}
}

void CBofBitmap::Line(INT nSrcX, INT nSrcY, INT nDstX, INT nDstY, byte iColor) {
	Assert(IsValidObject(this));

	// The source and destination points must be in the bitmap area
	Assert(GetRect().PtInRect(CBofPoint(nSrcX, nSrcY)));
	Assert(GetRect().PtInRect(CBofPoint(nDstX, nDstY)));

	if (m_errCode == ERR_NONE) {

		// Horizontal lines are a special case that can be done optimally
		//
		if (nSrcY == nDstY) {

			Lock();
			BofMemSet(GetPixelAddress(min(nSrcX, nDstX), nSrcY), iColor, ABS(nDstX - nSrcX));
			UnLock();

			// Otherwise use standard Breshnaham Line alogrithm
			//
		} else {

			INT i, distance, xerr, yerr, dx, dy, ix, iy;

			ix = (dx = nDstX - nSrcX) < 0 ? (dx = -dx, -1) : !!dx;
			iy = (dy = nDstY - nSrcY) < 0 ? (dy = -dy, -1) : !!dy;

			distance = max(dx, dy);

			xerr = yerr = 0;
			for (i = -2; i < distance; i++) {

				WritePixel(nSrcX, nSrcY, iColor);

				xerr += dx;
				yerr += dy;

				if (xerr > distance) {
					xerr -= distance;
					nSrcX += ix;
				}

				if (yerr > distance) {
					yerr -= distance;
					nSrcY += iy;
				}
			}
		}
	}
}

void CBofBitmap::Line(CBofPoint *pSrc, CBofPoint *pDest, byte iColor) {
	Assert(IsValidObject(this));
	Assert(pSrc != nullptr);
	Assert(pDest != nullptr);

	Line(pSrc->x, pSrc->y, pDest->x, pDest->y, iColor);
}

ERROR_CODE CBofBitmap::FlipVertical(CBofRect *pRect) {
	Assert(IsValidObject(this));

	if (m_errCode == ERR_NONE) {

		Lock();

		CBofRect cRect(0, 0, m_nDX - 1, m_nDY - 1);
		int32 x, y, dx, dy, dx1, dy1, i, nRows;
		byte *pStart, *pEnd;
		byte *pLine;

		// flip entire bitmap ?
		//
		if (pRect == nullptr) {
			pRect = &cRect;
		}

		x = pRect->left;
		y = pRect->top;
		dx = pRect->Width();
		dy = pRect->Height();

		if ((pLine = (byte *)BofAlloc(dx)) != nullptr) {

			pEnd = pStart = m_pBits;

			dx1 = m_nScanDX;
			dy1 = m_nDY;


			// is bitmap top-down or bottom up?
			//
			if (m_bTopDown) {
				pStart += y * dx1 + x;
				pEnd += (y + dy - 1) * dx1 + x;

			} else {
				pStart += (dy1 - y - 1) * dx1 + x;
				pEnd += (dy1 - (y + dy - 1) - 1) * dx1 + x;
				dx1 = -dx1;
			}

			// working row by row
			//
			nRows = dy / 2;
			for (i = 0; i < nRows; i++) {

				// copy this row into temp row buffer
				BofMemCopy(pLine, pStart, dx);

				// copy mirrored row to this row
				BofMemCopy(pStart, pEnd, dx);

				// copy temp row buffer to mirrored row
				BofMemCopy(pEnd, pLine, dx);

				pStart += dx1;
				pEnd -= dx1;
			}

			BofFree(pLine);

		} else {
			ReportError(ERR_MEMORY, "Error: FlipVertical - Could not allocate %ld bytes for row", dx);
		}

		UnLock();
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::FlipHorizontal(CBofRect *pRect) {
	Assert(IsValidObject(this));

	if (m_errCode == ERR_NONE) {

		Lock();

		CBofRect cRect(0, 0, m_nDX - 1, m_nDY - 1);

		int32 x, y, dx, dy, dx1, dy1, i, j, nCols;
		byte *pStart, *pCurr, *pMirr;
		byte cPixel;

		// flip entire bitmap ?
		//
		if (pRect == nullptr) {
			pRect = &cRect;
		}

		x = pRect->left;
		y = pRect->top;
		dx = pRect->Width();
		dy = pRect->Height();

		pStart = m_pBits;

		dx1 = m_nScanDX;
		dy1 = m_nDY;

		// is bitmap top-down or bottom up?
		//
		if (m_bTopDown) {
			pStart += y * dx1 + x;

		} else {
			pStart += (dy1 - y - 1) * dx1 + x;
			dx1 = -dx1;
		}
		nCols = dx / 2;

		for (j = 0; j < dy; j++) {

			// point to the 1st and last pixel in this row
			pCurr = pStart;
			pMirr = pStart + dx - 1;

			// for each pixel in half this row,
			//
			for (i = 0; i < nCols; i++) {

				// swap it for it's mirrored pixel

				// copy this pixel to our temp pixel buffer
				cPixel = *pCurr;

				// copy the mirrored pixel to this pixel
				*pCurr = *pMirr;

				// copy the temp pixel buffer to the mirrored pixel
				*pMirr = cPixel;

				// point to next pair
				pCurr++;
				pMirr--;
			}

			pStart += dx1;
		}

		UnLock();
	}

	return m_errCode;
}

CBofBitmap *CBofBitmap::ExtractBitmap(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	CBofBitmap *pNewBmp;
	CBofPalette *pPalette;

	pNewBmp = nullptr;

	if (m_errCode == ERR_NONE) {

		if ((pPalette = GetPalette()) != nullptr) {

			if (m_bOwnPalette) {
				pPalette = pPalette->CopyPalette();
			}

			if ((pNewBmp = new CBofBitmap(pRect->Width(), pRect->Height(), pPalette, m_bOwnPalette)) != nullptr) {
				Paint(pNewBmp, 0, 0, pRect);
			} else {
				LogFatal("Unable to allocate a new CBofBitmap");
			}

		} else {
			LogFatal("This bitmap does not have a valid palette");
		}
	}

	return pNewBmp;
}

ERROR_CODE CBofBitmap::ScrollRight(INT nPixels, CBofRect * /*pRect*/) {
	Assert(IsValidObject(this));

	if (m_errCode == ERR_NONE) {

		if (nPixels != 0) {

			Assert(m_pBits != nullptr);

			byte *p, *pTemp;

			if ((pTemp = (byte *)BofAlloc(abs(nPixels))) != nullptr) {
				INT nBytes, i;

				nBytes = m_nDX - nPixels;
				if (nPixels < 0) {
					nBytes = m_nDX + nPixels;
				}

				p = m_pBits;

				Lock();

				if (nPixels > 0) {

					for (i = 0; i < m_nDY; i++) {
						BofMemCopy(pTemp, p + nBytes, nPixels);
						BofMemMove(p + nPixels, p, nBytes);
						BofMemCopy(p, pTemp, nPixels);
						p += m_nScanDX;
					}

				} else {
					nPixels = -nPixels;

					for (i = 0; i < m_nDY; i++) {
						BofMemCopy(pTemp, p, nPixels);
						BofMemMove(p, p + nPixels, nBytes);
						BofMemCopy(p + nBytes, pTemp, nPixels);
						p += m_nScanDX;
					}
				}

				UnLock();

				BofFree(pTemp);
			}
		}
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::ScrollUp(INT nPixels, CBofRect *pRect) {
	Assert(IsValidObject(this));

	if (m_errCode == ERR_NONE) {

		Lock();

		CBofRect cRect(0, 0, m_nDX  - 1, m_nDY  - 1);
		int32 lJump;
		int32 x, y, dx, dy, dx1, dy1, i;
		byte *pStart, *pEnd, *p1stRow;
		byte *pCurRow, *pLastRow, *pRowBuf;

		// flip entire bitmap ?
		//
		if (pRect == nullptr) {
			pRect = &cRect;
		}

		x = pRect->left;
		y = pRect->top;
		dx = pRect->Width();
		dy = pRect->Height();

		// height must be valid or we're hosed
		Assert(dy > 0);

		// We don't have to scroll more than the height of the bitmap, because
		// scrolling (bitmap height + 1) pixels is the same as scrolling 1 pixel,
		// and scrolling 1 pixel is obviously more efficient then scrolling
		// (bitmap height + 1) pixels.
		//
		// Also, we can handle down srolling as scrolling up:
		// For example, if the bitmap-height is 10 pixels, and we want to
		// scroll down 6 pixels, that has the same effect as scrolling up
		// 4 pixels (10 - 6 = 4).  So, when we get negative nPixels, we will just
		// scroll (height + nPixels) in the opposite direction.
		//
		if (nPixels >= 0) {
			nPixels = nPixels % dy;
		} else {
			nPixels = -(-nPixels % dy);
			if (nPixels < 0)
				nPixels = dy + nPixels;
			Assert(nPixels >= 0 && nPixels < dy);
		}

		// only scroll if we need to
		//
		if (nPixels != 0) {

			// allocate a buffer to hold one horizontal line
			//
			if ((pRowBuf = (byte *)BofAlloc(dx)) != nullptr) {

				pStart = pEnd = m_pBits;

				dx1 = m_nScanDX;
				dy1 = m_nDY;

				// is bitmap top-down or bottom up?
				//
				if (m_bTopDown) {
					pStart += y * dx1 + x;
					pEnd += (y + dy - 1) * dx1 + x;
				} else {
					pStart += (dy1 - y - 1) * dx1 + x;
					pEnd += (dy1 - (y + dy - 1) - 1) * dx1 + x;
					dx1 = -dx1;
				}
				pCurRow = pStart;

				// copy 1st row into temp row buffer
				BofMemCopy(pRowBuf, pCurRow, dx);

				lJump = dx1 * nPixels;

				pLastRow = pCurRow;
				pCurRow += lJump;
				p1stRow = pStart;

				// working row by row
				//
				for (i = 1; i < dy; i++) {

					// copy this row to row above it
					BofMemCopy(pLastRow, pCurRow, dx);

					pLastRow = pCurRow;

					pCurRow += lJump;
					if (pCurRow < pEnd && !m_bTopDown) {
						pCurRow = pStart - (pEnd - pCurRow) - dx1;

						if (pCurRow == p1stRow) {

							i++;

							// copy 1st row into this row
							BofMemCopy(pLastRow, pRowBuf, dx);

							pCurRow += dx1;
							p1stRow = pLastRow = pCurRow;

							// copy this next row into temp row buffer
							BofMemCopy(pRowBuf, p1stRow, dx);

							pCurRow += lJump;
						}
					}
				}

				// copy 1st row into last row
				BofMemCopy(pLastRow, pRowBuf, dx);

				BofFree(pRowBuf);

			} else {
				ReportError(ERR_MEMORY, "Error: ScrollUp - Could not allocate %ld bytes for row", dx);
			}
		}
		UnLock();
	}

	return m_errCode;
}

#define BIT0    0x00000001
#define BIT2    0x00000004
#define BIT15   0x00008000
#define BIT16   0x00010000
#define BIT17   0x00020000
#define SEQ     (BIT17)
#define BLOCK   4

ERROR_CODE CBofBitmap::FadeIn(CBofWindow *pWnd, INT xStart, INT yStart, INT nMaskColor, INT nBlockSize, INT /*nSpeed*/) {
	Assert(IsValidObject(this));

	Assert(pWnd != nullptr);
	Assert(xStart >= 0);
	Assert(yStart >= 0);

	if (m_errCode == ERR_NONE) {

		CBofRect cSrcRect, cDstRect;
		uint32    seed;
		uint32    value, maxvalue;
		uint32    x, y;
		uint32    mask = (BIT16 | BIT15 | BIT2 | BIT0);
		uint32    tmp, cnt, shft;
		uint32    width, height;

		width = m_nDX / nBlockSize;
		height = m_nDY;

		maxvalue = height / nBlockSize * width;
		value = seed = SEQ - 1;

		for (;;) {
			tmp = (value & mask);
			shft = BIT0;
			cnt = 0;
			while (shft < SEQ) {
				if (tmp & shft) {
					cnt++;
				}
				shft <<= 1;
			}
			value <<= 1;

			if (cnt & BIT0) {
				value |= BIT0;
			}

			value &= (SEQ - 1);
			if (value == seed)
				break;
			if (value > maxvalue)
				continue;

			y = (value / width) * nBlockSize;
			x = (value % width) * nBlockSize;

			cSrcRect.SetRect((INT)x, (INT)y, (INT)x + nBlockSize - 1, (INT)y + nBlockSize - 1);
			x += xStart;
			y += yStart;
			cDstRect.SetRect((INT)x, (INT)y, (INT)x + nBlockSize - 1, (INT)y + nBlockSize - 1);
			Paint(pWnd, &cDstRect, &cSrcRect, nMaskColor);
		}

		cSrcRect.SetRect(0, 0, nBlockSize - 1, nBlockSize - 1);
		Paint(pWnd, &cSrcRect, &cSrcRect, nMaskColor);
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::Curtain(CBofWindow *pWnd, INT nSpeed, INT nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

	if (m_errCode == ERR_NONE) {
		CBofRect cRect;
		INT i, nHeight, nWidth;

		nWidth = pWnd->Width();
		nHeight = pWnd->Height();

		for (i = 0; i < nHeight; i += nSpeed) {
			cRect.SetRect(0, i, nWidth - 1, i + nSpeed - 1);
			Paint(pWnd, &cRect, &cRect, nMaskColor);
			Sleep(1);
		}
	}

	return m_errCode;
}

ERROR_CODE CBofBitmap::FadeLines(CBofWindow *pWnd, CBofRect *pDstRect, CBofRect *pSrcRect, INT nSpeed, INT nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);
	Assert(nSpeed != 0);

	if (m_errCode == ERR_NONE) {

		CBofRect cDstRect, cSrcRect, cWindowRect, cBmpRect;
		INT i, j, nWidth1, nHeight1, x1, y1;
		INT nWidth2, x2, y2;

		// Entire window?
		//
		if (pDstRect == nullptr) {
			cWindowRect = pWnd->GetRect();
			pDstRect = &cWindowRect;
		}
		if (pSrcRect == nullptr) {
			cBmpRect = GetRect();
			pSrcRect = &cBmpRect;
		}

		x1 = pDstRect->left;
		y1 = pDstRect->top;
		nWidth1 = pDstRect->Width();
		nHeight1 = pDstRect->Height();

		x2 = pSrcRect->left;
		y2 = pSrcRect->top;
		nWidth2 = pSrcRect->Width();

		for (j = 0; j < 4; j++) {

			for (i = 0; i < nHeight1 ; i += 4) {

				cDstRect.SetRect(x1, y1 + i + j, x1 + nWidth1 - 1, y1 + i + j);
				cSrcRect.SetRect(x2, y2 + i + j, x2 + nWidth2 - 1, y2 + i + j);
				Paint(pWnd, &cDstRect, &cSrcRect, nMaskColor);

				if (i % nSpeed == 0)
					Sleep(1);
			}
		}
	}

	return m_errCode;
}

const CHAR *CBofBitmap::GetFileName() {
	Assert(IsValidObject(this));

	const CHAR *p = nullptr;
	if (m_szFileName[0] != '\0')
		p = (const CHAR *)&m_szFileName[0];

	return p;
}

#if BOF_DEBUG

double CBofBitmap::FPSTest(CBofWindow *pWnd, CBofPalette *pPalette) {
	Assert(pWnd != nullptr);
	Assert(pPalette != nullptr);

	CBofBitmap cBmp(640, 480, pPalette);
	double fFPS;
	INT i;

	TimerStart();
	for (i = 0; i < 1000; i++) {
		cBmp.Paint(pWnd, 0, 0);
	}

	fFPS = (double)1000000 / TimerStop();
	LogInfo(BuildString("CBofBitmap::FPSTest: %f FPS", fFPS));

	return fFPS;
}

double CBofBitmap::OffScreenFPSTest(CBofPalette *pPalette) {
	Assert(pPalette != nullptr);

	CBofBitmap cBmp1(640, 480, pPalette);
	CBofBitmap cBmp2(640, 480, pPalette);
	double fFPS;
	INT i;

	TimerStart();
	for (i = 0; i < 1000; i++) {
		cBmp1.Paint(&cBmp2, 0, 0);
	}

	fFPS = (double)1000000 / TimerStop();
	LogInfo(BuildString("CBofBitmap::OffScreenFPSTest: %f FPS", fFPS));

	return fFPS;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// Misc global graphics routines
//
//////////////////////////////////////////////////////////////////////////////

CBofBitmap *LoadBitmap(const CHAR *pszFileName, CBofPalette *pPalette, BOOL bUseShared) {
	CBofPalette *pUsePal = pPalette;
	CBofBitmap *pBmp;

	// If no palette was passed in and a shared palette was requested, then
	// use our default one established by "SHAREDPAL=" in the script
	if (bUseShared && pPalette == nullptr) {
		pUsePal = CBofPalette::GetSharedPalette();
	}

	pBmp = new CBofBitmap(pszFileName, pUsePal);

	return pBmp;
}

ERROR_CODE PaintBitmap(CBofWindow *pWindow, const CHAR *pszFileName, CBofRect *pDstRect, CBofRect *pSrcRect, CBofPalette *pPalette, INT nMaskColor) {
	Assert(pWindow != nullptr);
	Assert(pszFileName != nullptr);

	CBofBitmap *pBmp;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pBmp = new CBofBitmap(pszFileName, pPalette)) != nullptr) {
		CBofRect cRect;

		cRect = pBmp->GetRect();

		if (pSrcRect == nullptr)
			pSrcRect = &cRect;

		if (pDstRect == nullptr)
			pDstRect = &cRect;

		// for this one draw, make sure that the right palette is
		// in place.
#if BOF_MAC
		CBofPalette *pPal = (pPalette == nullptr ? pBmp->GetPalette() : pPalette);
		CBofPalette *pSavePalette = nullptr;
		CBofApp *pApp = nullptr;
		if (pPal) {
			pApp = CBofApp::GetApp();
			if (pApp) {
				pSavePalette = pApp->GetPalette();
				pApp->SetPalette(pPal);
			}
		}
#endif

		errCode = pBmp->Paint(pWindow, pDstRect, pSrcRect, nMaskColor);

		delete pBmp;

		// restore the previous palette
#if BOF_MAC
		if (pSavePalette) {
			pApp->SetPalette(pSavePalette);
		}
#endif

	} else {
		errCode = ERR_MEMORY;
	}

	return errCode;
}

ERROR_CODE PaintBitmap(CBofBitmap *pBitmap, const CHAR *pszFileName, CBofRect *pDstRect, CBofRect *pSrcRect, CBofPalette *pPalette, INT nMaskColor) {
	Assert(pBitmap != nullptr);
	Assert(pszFileName != nullptr);

	CBofBitmap *pBmp;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pBmp = new CBofBitmap(pszFileName, pPalette)) != nullptr) {

		CBofRect cRect;

		cRect = pBmp->GetRect();

		if (pSrcRect == nullptr)
			pSrcRect = &cRect;

		if (pDstRect == nullptr)
			pDstRect = &cRect;

		errCode = pBmp->Paint(pBitmap, pDstRect, pSrcRect, nMaskColor);

		delete pBmp;

	} else {
		errCode = ERR_MEMORY;
	}

	return errCode;
}

CBofPalette *LoadPalette(const CHAR *pszFileName) {
	Assert(pszFileName != nullptr);
	Assert(FileExists(pszFileName));

	CBofPalette *pPal;

	pPal = new CBofPalette(pszFileName);

	return pPal;
}

CBofSize GetBitmapSize(const CHAR *pszFileName) {
	Assert(pszFileName != nullptr);
	BITMAP_EX stBitmap;
	CBofSize cSize;
	CBofFile *pFile;

	// make sure this file exists
	Assert(FileExists(pszFileName));

	// open bitmap
	//
	if ((pFile = new CBofFile(pszFileName, CBOFFILE_READONLY)) != nullptr) {

		// read header
		//
		if (pFile->Read(&stBitmap, sizeof(BOFBITMAPFILEHEADER) + sizeof(BOFBITMAPINFOHEADER)) == ERR_NONE) {

#if BOF_MAC || BOF_WINMAC
			//
			// Swap bytes for Macintosh Big-Endian of fields that
			// we are actually using.
			//
			stBitmap.m_cInfoHeader.biWidth = SWAPint32(stBitmap.m_cInfoHeader.biWidth);
			stBitmap.m_cInfoHeader.biHeight = SWAPint32(stBitmap.m_cInfoHeader.biHeight);
#endif
			cSize.cx = (INT)stBitmap.m_cInfoHeader.biWidth;
			cSize.cy = (INT)stBitmap.m_cInfoHeader.biHeight;

		} else {
			LogError(BuildString("Error reading BOFBITMAPFILEHEADER from %s", pszFileName));
		}

		// close bitmap
		delete pFile;

	} else {
		LogError(BuildString("Could not allocate a CBofFile for %s", pszFileName));
	}

	return cSize;
}


//	Routine that takes a palette and a grafport and synchronizes
//	their color definitions.  Makes the current grafport the
//  exact same as the palette passed. not currently
//  used.
#if BOF_MAC || BOF_WINMAC
#if SYNCPALETTES
void SynchronizeColorTables(PaletteHandle pPalette, CGrafPtr cGrafPtr) {
	PixMapHandle        pmh = cGrafPtr->portPixMap;
	CTabHandle          clutH = (*pmh)->pmTable;

	Assert((*pmh)->pixelSize == 8);
	Assert((*clutH)->ctSize == 255);
	Assert((*pPalette)->pmEntries == 256);

	if (pPalette == nullptr)
		return;

	ColorSpecPtr cTable = (*clutH)->ctTable;
	HLock((Handle) clutH);

	for (int i = 0; i < 256; i++) {
		cTable[i].rgb.red = (*pPalette)->pmInfo[i].ciRGB.red;
		cTable[i].rgb.green = (*pPalette)->pmInfo[i].ciRGB.green;
		cTable[i].rgb.blue = (*pPalette)->pmInfo[i].ciRGB.blue;
	}

	HUnlock((Handle) clutH);
}
#endif
#endif

void CBofBitmap::FlipVerticalFast() {
	m_bTopDown = !m_bTopDown;
	m_cBitmapInfo.m_cInfoHeader.biHeight = -m_cBitmapInfo.m_cInfoHeader.biHeight;
}

void CBofBitmap::FlipBits()  {
	Assert(m_cBitmapInfo.m_cInfoHeader.biBitCount == 8);
	Assert(m_cBitmapInfo.m_cInfoHeader.biPlanes == 1);

	int32 dx = m_cBitmapInfo.m_cInfoHeader.biWidth;
	int32 dy = ABS(m_cBitmapInfo.m_cInfoHeader.biHeight);

	dx = m_nScanDX;
	dy = m_nDY;

	Assert(dx > 0);
	Assert(dy > 0);

	Assert(m_nScanDX);
	Assert(m_nDY > 0);
	Assert(m_pBits != 0);

	Lock();

	byte *pOrigBits = m_pBits;
	byte *pDestBits = (byte *) BofAlloc((int32) m_nScanDX * (int32) m_nDY);

	Assert(pDestBits != nullptr);

	for (INT i = 0; i < dy; i++) {

#if BOF_MAC && !__POWERPC__
		BofMacMemCopyNotTransparent(&pDestBits[i * dx], &pOrigBits[dx * (dy - (i + 1))], dx);
#else
		BofMemCopy(&pDestBits[i * dx], &pOrigBits[dx * (dy - (i + 1))], dx);
#endif
	}

	m_pBits = pDestBits;

	UnLock();

	BofFree(pOrigBits);
}

#if COPYBITS && BOF_MAC
void BofCopyBits(PixMapHandle srcPixMap, PixMapHandle trgPixMap, Rect *srcRect, Rect *trgRect, INT nMaskColor) {

	RGBColor        myWhite = {0xFFFF, 0xFFFF, 0xFFFF};
	RGBColor        myBlack = {0x0000, 0x0000, 0x0000};
	CGrafPtr        curCPort;
	RGBColor        curRGBBkColor;
	RGBColor        curRGBFgColor;
	long            curRGBBkColorIndex;
	long            curRGBFgColorIndex;
	short           nTransferMode = srcCopy;
	CTabHandle      oldCTab;

	oldCTab = (*trgPixMap)->pmTable;
	(*trgPixMap)->pmTable       = (*srcPixMap)->pmTable;

	// We really shouldn't have to stretch in this routine.

	Assert((srcRect->right - srcRect->left) == (trgRect->right - trgRect->left));
	Assert((srcRect->bottom - srcRect->top) == (trgRect->bottom - trgRect->top));

	// don't need to set the foreground and background in spacebar.
	if (nMaskColor == NOT_TRANSPARENT) {
		//GetPort (&(GrafPtr) curCPort);

		//curRGBBkColor = curCPort->rgbBkColor;
		//curRGBFgColor = curCPort->rgbFgColor;
		//curRGBBkColorIndex = curCPort->bkColor;
		//curRGBFgColorIndex = curCPort->fgColor;

		//RGBForeColor (&myBlack);
		//RGBBackColor (&myWhite);
	} else {
		if (nMaskColor == COLOR_WHITE) {
			nTransferMode = transparent;
		} else {
			// Assert (nMaskColor == COLOR_WHITE || nMaskColor == NOT_TRANSPARENT);
		}
	}

	HLock((Handle) srcPixMap);
	HLock((Handle) trgPixMap);

	CopyBits(*(BitMap **) srcPixMap,
	         *(BitMap **) trgPixMap,
	         srcRect,
	         trgRect,
	         nTransferMode,
	         nullptr);

	HUnlock((Handle) trgPixMap);
	HUnlock((Handle) srcPixMap);

	//  Restore the original ctable.
	(*trgPixMap)->pmTable = oldCTab;

	if (nMaskColor == NOT_TRANSPARENT) {
		//RGBForeColor (&curRGBFgColor);
		//RGBBackColor (&curRGBBkColor);
		//curCPort->bkColor     = curRGBBkColorIndex;
		//curCPort->fgColor     = curRGBFgColorIndex;
	}

	return;
}
#endif

ERROR_CODE CBofBitmap::PaintPalette(CBofWindow *pWin, INT x, INT y) {
	Assert(IsValidObject(this));
	Assert(pWin != nullptr);

	CBofBitmap cBmp(256, 256, m_pPalette);
	CBofRect cRect;
	INT i, j;

	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			cRect.SetRect(j * 16, i * 16, j * 16 + 16 - 1, i * 16 + 16 - 1);
			cBmp.FillRect(&cRect, (byte)(i * 16 + j));
		}
	}

	cBmp.Paint(pWin, x, y);

	return m_errCode;
}

Graphics::ManagedSurface CBofBitmap::getSurface() {
	Graphics::ManagedSurface s;
	s.w = m_nDX;
	s.h = m_nDY;
	s.pitch = m_nScanDX;
	s.format = Graphics::PixelFormat::createFormatCLUT8();
	s.setPalette(m_pPalette->GetData(), 0, PALETTE_COUNT);
	s.setPixels(m_pBits);

	return s;
}

void CBofBitmap::dumpToPng(Common::String fname, bool grayscale) {
	Common::DumpFile bitmapFile;
	Graphics::ManagedSurface surface = getSurface();

	bitmapFile.open(Common::Path(fname));
	byte pal[256*3];

	if (!grayscale) {
		surface.grabPalette(pal, 0, PALETTE_COUNT);
	} else {
		for (int i = 0; i < PALETTE_COUNT; i++)
			pal[i * 3] = pal[i * 3 + 1] = pal[i * 3 + 2] = i;
	}

	Image::writePNG(bitmapFile, surface.rawSurface(), pal);
	bitmapFile.close();
}

} // namespace Bagel
