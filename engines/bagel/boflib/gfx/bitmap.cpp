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
#include "bagel/boflib/app.h"
#include "bagel/boflib/file.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

#define TEST_DEBUG 0

bool CBofBitmap::m_bUseBackdrop = false;

extern bool g_bRealizePalette;

CBofBitmap::CBofBitmap() {
	m_szFileName[0] = '\0';
}

CBofBitmap::CBofBitmap(int dx, int dy, CBofPalette *pPalette, bool bOwnPalette, byte *pPrivateBuff) {
	Assert((dx > 0) && (dy > 0));

	// Use application's palette if not supplied
	//
	if (pPalette == nullptr) {
		bOwnPalette = false;
		pPalette = CBofApp::GetApp()->GetPalette();
	}

	// Inits
	//
	m_nDX = dx;
	m_nDY = dy;
	m_nScanDX = ((dx + 3) & ~3);
	m_bTopDown = true;
	m_bOwnPalette = bOwnPalette;
	m_bReadOnly = false;
	m_szFileName[0] = '\0';
	m_pPalette = nullptr;
	m_bInitialized = true;

	// Allow privatization of the bitmap (used only on mac from displaytextex).;
	m_bPrivateBmp = (pPrivateBuff != nullptr);
	if (m_bPrivateBmp == true) {
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
#elif BOF_MAC || BOF_WINMAC
#if COPYBITS
	FlipVerticalFast();         // mac bitmaps are read in top down
#endif
#endif

	m_pPalette = pPalette;
	Load();
}

CBofBitmap::CBofBitmap(const char *pszFileName, CBofPalette *pPalette, bool bOwnPalette) {
	Assert(pszFileName != nullptr);
	m_bOwnPalette = bOwnPalette;
	m_szFileName[0] = '\0';
	m_bInitialized = true;

	if (pPalette == nullptr) {
		if ((pPalette = new CBofPalette(pszFileName)) != nullptr) {
			m_bOwnPalette = true;
		}
	}

	// Init the info needed to load a bitmap from disk
	m_pPalette = pPalette;

	if (FileGetFullPath(m_szFileName, pszFileName) != nullptr) {
		// Load this bitmap into the cache
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
		m_bOwnPalette = false;
	}

	m_pPalette = nullptr;
	m_bInitialized = false;
}


ErrorCode CBofBitmap::BuildBitmap(CBofPalette *pPalette) {
	Assert(IsValidObject(this));
	Assert(pPalette != nullptr);

	if (m_errCode == ERR_NONE) {

		_bitmap.create(m_nDX, m_nDY, Graphics::PixelFormat::createFormatCLUT8());
		m_pBits = (byte *)_bitmap.getBasePtr(0, 0);
		m_nScanDX = _bitmap.pitch;

		// Set this bitmap's palette
		SetPalette(pPalette, m_bOwnPalette);

#if BOF_MAC || BOF_WINMAC || BOF_WIN16
		// set this bitmap's palette
		SetPalette(pPalette, m_bOwnPalette);

		m_nDX = (int)m_cBitmapInfo.m_cInfoHeader.biWidth;
		m_nDY = (int)ABS(m_cBitmapInfo.m_cInfoHeader.biHeight);
		m_nScanDX = (m_nDX + 3) & ~3;
		m_bTopDown = (m_cBitmapInfo.m_cInfoHeader.biHeight < 0);
		if (m_bPrivateBmp == false) {
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

bool CBofBitmap::Alloc() {
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

ErrorCode CBofBitmap::LoadBitmap(const char *pszFileName, CBofPalette *pPalette) {
	Assert(IsValidObject(this));
	Assert(pszFileName != nullptr);
	Assert(pPalette != nullptr);

	// Release any previous bitmap info
	ReleaseBitmap();

	if (m_errCode == ERR_NONE) {
		CBofFile *pFile = new CBofFile(pszFileName, CBOFFILE_READONLY);

		// Open bitmap
		if (pFile != nullptr) {
			// filename must fit into our buffer
			Assert(strlen(pszFileName) < MAX_FNAME);

			// Keep track of this filename
			Common::strcpy_s(m_szFileName, pszFileName);

			// Decode the bitmap
			Image::BitmapDecoder decoder;
			Common::SeekableReadStream *rs = *pFile;
			if (!rs || !decoder.loadStream(*rs))
				error("Could not load bitmap");

			// Load up the decoded bitmap
			_bitmap.copyFrom(*decoder.getSurface());

			// Load the bitmap palette
			_bitmap.setPalette(decoder.getPalette(), 0, PALETTE_COUNT);

			m_nDX = _bitmap.w;
			m_nDY = _bitmap.h;
			m_nScanDX = _bitmap.pitch;
			m_pBits = (byte*)_bitmap.getBasePtr(0, 0);

			// Close bitmap file
			delete pFile;

		} else {
			ReportError(ERR_MEMORY, "Could not allocate a CBofFile for %s", pszFileName);
		}
	}

	return m_errCode;
}

ErrorCode CBofBitmap::Paint(CBofWindow *pWnd, int x, int y, CBofRect *pSrcRect, int nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

	CBofRect cRect(x, y, x + m_nDX - 1, y + m_nDY - 1);

	if (pSrcRect != nullptr) {
		cRect.SetRect(x, y, x + pSrcRect->Width() - 1, y + pSrcRect->Height() - 1);
	}

	return Paint(pWnd, &cRect, pSrcRect, nMaskColor);
}

ErrorCode CBofBitmap::Paint(CBofWindow *pWnd, CBofRect *pDstRect, CBofRect *pSrcRect, int nMaskColor) {
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

ErrorCode CBofBitmap::PaintMaskBackdrop(CBofWindow *pWnd, CBofRect *pDstRect, CBofRect *pSrcRect, int nMaskColor) {
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

		// Use the backdrop to get the background instead of trying to
		// capture the screen
		if ((pBackdrop = pWnd->GetBackdrop()) != nullptr) {
			CBofRect cTempRect = cTempBitmap.GetRect();
			pBackdrop->Paint(&cTempBitmap, &cTempRect, &cDestRect);
		}

		Paint(&cTempBitmap, 0, 0, &cSourceRect, nMaskColor);
		cTempBitmap.Paint(pWnd, &cDestRect);

		UnLock();
	}

	return m_errCode;
}

ErrorCode CBofBitmap::Paint(CBofBitmap *pBmp, int x, int y, CBofRect *pSrcRect, int nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	CBofRect cRect(x, y, x + m_nDX - 1, y + m_nDY - 1);

	if (pSrcRect != nullptr) {
		cRect.SetRect(x, y, x + pSrcRect->Width() - 1, y + pSrcRect->Height() - 1);
	}

	return Paint(pBmp, &cRect, pSrcRect, nMaskColor);
}

ErrorCode CBofBitmap::Paint(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect, int nMaskColor) {
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

		// Don't need a lock on these guys anymore
		pBmp->UnLock();
		UnLock();
	}

	return m_errCode;
}

ErrorCode CBofBitmap::Paint1To1(CBofBitmap *pBmp) {
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

ErrorCode CBofBitmap::PaintStretch4(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);
	Assert(pDstRect != nullptr);
	Assert(pSrcRect != nullptr);

	// These bitmaps MUST be locked down before hand
	Assert(IsLocked());
	Assert(pBmp->IsLocked());

	if (m_errCode == ERR_NONE) {
		int dy1 = m_nDY;
		int dx1 = m_nScanDX;

		int dy2 = pBmp->m_nDY;
		int dx2 = pBmp->m_nScanDX;

		byte *pDestBits = pBmp->m_pBits;
		byte *pSrcBits = m_pBits;

		int32 dy = pSrcRect->Height();

		int32 nDstHeight = pDstRect->Height();

		int32 x1 = pSrcRect->left;
		int32 y1 = pSrcRect->top;

		int32 x2 = pDstRect->left;
		int32 y2 = pDstRect->top;

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

		Fixed SourceStepY = FixedDivide(IntToFixed(dy), IntToFixed(nDstHeight));
		Fixed PosY = 0;
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

ErrorCode CBofBitmap::PaintStretchOpt(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect, int nOptSize) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);
	Assert(pDstRect != nullptr);
	Assert(pSrcRect != nullptr);

	// These bitmaps MUST be locked down before hand
	Assert(IsLocked());
	Assert(pBmp->IsLocked());

	if (m_errCode == ERR_NONE) {
		int dy1 = m_nDY;
		int dx1 = m_nScanDX;

		int dy2 = pBmp->m_nDY;
		int dx2 = pBmp->m_nScanDX;

		byte *pDestBits = pBmp->m_pBits;
		byte *pSrcBits = m_pBits;

		int32 dy = pSrcRect->Height();

		int32 nDstHeight = pDstRect->Height();

		int32 x1 = pSrcRect->left;
		int32 y1 = pSrcRect->top;

		int32 x2 = pDstRect->left;
		int32 y2 = pDstRect->top;

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

		Fixed SourceStepY = FixedDivide(IntToFixed(dy), IntToFixed(nDstHeight));
		Fixed PosY = 0;
		byte *pSrcEnd = pSrcBits + (dy - 1) * dx1;
		byte *pDestEnd = pDestBits + (nDstHeight - 1) * dx2;

		int nMod = (int)nDstHeight & 1;
		nDstHeight >>= 1;

		while (nDstHeight-- > 0) {

			if (PosY >= 0x00010000) {

				int32 lInc = FixedToInt(PosY) * dx1;
				pSrcBits += lInc;
				pSrcEnd -= lInc;

				PosY &= 0x0000FFFF;
			}

			for (int i = 0; i < nOptSize; i += 4) {
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

ErrorCode CBofBitmap::CaptureScreen(CBofWindow *pWnd, CBofRect *pSrcRect, CBofRect *pDstRect) {
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
		CBofBitmap *pBackdrop = pWnd->GetBackdrop();

		// If we're capturing the screen, we have to convert the format first.
		if (!m_bUseBackdrop || pBackdrop == nullptr) {
			Graphics::Surface tmp;
			tmp.copyFrom(*pWnd->getSurface());
			_bitmap.blitFrom(tmp.convertTo(_bitmap.format, nullptr, 0, m_pPalette->GetData(), PALETTE_COUNT),
				cSrcRect,
				cDestRect);
			tmp.free();

		} else {
			// Optimization to use the window's backdrop bitmap instead of doing
			// an actual screen capture.
			pBackdrop->Paint(this, &cDestRect, &cSrcRect);
		}
	}

	return m_errCode;
}

void CBofBitmap::SetPalette(CBofPalette *pBofPalette, bool bOwnPalette) {
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
		}
	}
}

byte *CBofBitmap::GetPixelAddress(int x, int y) {
	Assert(IsValidObject(this));

	// You can not call this function unless you manually lock this bitmap
	Assert(IsLocked());

	// The pixel in question must be in the bitmap area
	Assert(GetRect().PtInRect(CBofPoint(x, y)));

	return (byte *)_bitmap.getBasePtr(x, y);
}

byte CBofBitmap::ReadPixel(int x, int y) {
	Assert(IsValidObject(this));

	Lock();
	byte chPixel = *GetPixelAddress(x, y);
	UnLock();

	return chPixel;
}

void CBofBitmap::WritePixel(int x, int y, byte iColor) {
	Assert(IsValidObject(this));

	Lock();
	byte *pPixel = GetPixelAddress(x, y);
	*pPixel = iColor;
	UnLock();
}

void CBofBitmap::Circle(int xCenter, int yCenter, uint16 nRadius, byte iColor) {
	Assert(IsValidObject(this));

	if (m_errCode == ERR_NONE) {

		int x = 0;
		int y = nRadius;
		int i = 3 - 2 * y;

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

void CBofBitmap::FillCircle(int x, int y, uint16 nRadius, byte iColor) {
	Assert(IsValidObject(this));

	Circle(x, y, nRadius, iColor);

	// Still need to fill it
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
		if (pRect == nullptr) {
			_bitmap.clear(iColor);
		} else {
			Common::Rect rect(pRect->left, pRect->top, pRect->right, pRect->bottom);
			_bitmap.fillRect(rect, iColor);
		}
	}
}

void CBofBitmap::Line(int nSrcX, int nSrcY, int nDstX, int nDstY, byte iColor) {
	Assert(IsValidObject(this));

	// The source and destination points must be in the bitmap area
	Assert(GetRect().PtInRect(CBofPoint(nSrcX, nSrcY)));
	Assert(GetRect().PtInRect(CBofPoint(nDstX, nDstY)));

	if (m_errCode == ERR_NONE) {
		// Horizontal lines are a special case that can be done optimally
		if (nSrcY == nDstY) {
			Lock();
			BofMemSet(GetPixelAddress(MIN(nSrcX, nDstX), nSrcY), iColor, ABS(nDstX - nSrcX));
			UnLock();

		} else {
			// Otherwise use standard Bresenham Line algorithm
			int dx = nDstX - nSrcX;
			int dy = nDstY - nSrcY;

			int ix = dx < 0 ? (dx = -dx, -1) : !!dx;
			int iy = dy < 0 ? (dy = -dy, -1) : !!dy;

			int distance = MAX(dx, dy);

			int xerr = 0;
			int yerr = 0;
			for (int i = -2; i < distance; i++) {
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

CBofBitmap *CBofBitmap::ExtractBitmap(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	CBofBitmap *pNewBmp = nullptr;

	if (m_errCode == ERR_NONE) {
		CBofPalette *pPalette = GetPalette();
		if (pPalette != nullptr) {
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

ErrorCode CBofBitmap::ScrollRight(int nPixels, CBofRect * /*pRect*/) {
	Assert(IsValidObject(this));

	if (m_errCode == ERR_NONE) {
		if (nPixels != 0) {
			Assert(m_pBits != nullptr);
			byte *pTemp = (byte *)BofAlloc(abs(nPixels));

			if (pTemp != nullptr) {
				int nBytes = m_nDX - nPixels;
				if (nPixels < 0) {
					nBytes = m_nDX + nPixels;
				}

				byte *p = m_pBits;

				Lock();

				if (nPixels > 0) {
					for (int i = 0; i < m_nDY; i++) {
						BofMemCopy(pTemp, p + nBytes, nPixels);
						BofMemMove(p + nPixels, p, nBytes);
						BofMemCopy(p, pTemp, nPixels);
						p += m_nScanDX;
					}
				} else {
					nPixels = -nPixels;

					for (int i = 0; i < m_nDY; i++) {
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

ErrorCode CBofBitmap::ScrollUp(int nPixels, CBofRect *pRect) {
	Assert(IsValidObject(this));

	if (m_errCode == ERR_NONE) {
		Lock();

		CBofRect cRect(0, 0, m_nDX  - 1, m_nDY  - 1);

		// Flip entire bitmap ?
		if (pRect == nullptr) {
			pRect = &cRect;
		}

		int32 x = pRect->left;
		int32 y = pRect->top;
		int32 dx = pRect->Width();
		int32 dy = pRect->Height();

		// Height must be valid or we're hosed
		Assert(dy > 0);

		// We don't have to scroll more than the height of the bitmap, because
		// scrolling (bitmap height + 1) pixels is the same as scrolling 1 pixel,
		// and scrolling 1 pixel is obviously more efficient then scrolling
		// (bitmap height + 1) pixels.
		//
		// Also, we can handle down scrolling as scrolling up:
		// For example, if the bitmap-height is 10 pixels, and we want to
		// scroll down 6 pixels, that has the same effect as scrolling up
		// 4 pixels (10 - 6 = 4).  So, when we get negative nPixels, we will just
		// scroll (height + nPixels) in the opposite direction.
		if (nPixels >= 0) {
			nPixels = nPixels % dy;
		} else {
			nPixels = -(-nPixels % dy);
			if (nPixels < 0)
				nPixels = dy + nPixels;
			Assert(nPixels >= 0 && nPixels < dy);
		}

		// Only scroll if we need to
		if (nPixels != 0) {
			byte *pRowBuf = (byte *)BofAlloc(dx);

			// Allocate a buffer to hold one horizontal line
			if (pRowBuf != nullptr) {

				byte *pEnd;
				byte *pStart = pEnd = m_pBits;

				int32 dx1 = m_nScanDX;
				int32 dy1 = m_nDY;

				// Is bitmap top-down or bottom up?
				if (m_bTopDown) {
					pStart += y * dx1 + x;
					pEnd += (y + dy - 1) * dx1 + x;
				} else {
					pStart += (dy1 - y - 1) * dx1 + x;
					pEnd += (dy1 - (y + dy - 1) - 1) * dx1 + x;
					dx1 = -dx1;
				}
				byte *pCurRow = pStart;

				// Copy 1st row into temp row buffer
				BofMemCopy(pRowBuf, pCurRow, dx);

				int32 lJump = dx1 * nPixels;

				byte *pLastRow = pCurRow;
				pCurRow += lJump;
				byte *p1stRow = pStart;

				// Working row by row
				for (int32 i = 1; i < dy; i++) {
					// Copy this row to row above it
					BofMemCopy(pLastRow, pCurRow, dx);

					pLastRow = pCurRow;

					pCurRow += lJump;
					if (pCurRow < pEnd && !m_bTopDown) {
						pCurRow = pStart - (pEnd - pCurRow) - dx1;

						if (pCurRow == p1stRow) {

							i++;

							// Copy 1st row into this row
							BofMemCopy(pLastRow, pRowBuf, dx);

							pCurRow += dx1;
							p1stRow = pLastRow = pCurRow;

							// Copy this next row into temp row buffer
							BofMemCopy(pRowBuf, p1stRow, dx);

							pCurRow += lJump;
						}
					}
				}

				// Copy 1st row into last row
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

ErrorCode CBofBitmap::FadeIn(CBofWindow *pWnd, int xStart, int yStart, int nMaskColor, int nBlockSize, int /*nSpeed*/) {
	Assert(IsValidObject(this));

	Assert(pWnd != nullptr);
	Assert(xStart >= 0);
	Assert(yStart >= 0);

	if (m_errCode == ERR_NONE) {

		CBofRect cSrcRect, cDstRect;
		uint32 mask = (BIT16 | BIT15 | BIT2 | BIT0);

		uint32 width = m_nDX / nBlockSize;
		uint32 height = m_nDY;

		uint32 maxvalue = height / nBlockSize * width;
		uint32 seed = SEQ - 1;
		uint32 value = seed;

		for (;;) {
			uint32 tmp = (value & mask);
			uint32 shft = BIT0;
			uint32 cnt = 0;
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

			uint32 y = (value / width) * nBlockSize;
			uint32 x = (value % width) * nBlockSize;

			cSrcRect.SetRect((int)x, (int)y, (int)x + nBlockSize - 1, (int)y + nBlockSize - 1);
			x += xStart;
			y += yStart;
			cDstRect.SetRect((int)x, (int)y, (int)x + nBlockSize - 1, (int)y + nBlockSize - 1);
			Paint(pWnd, &cDstRect, &cSrcRect, nMaskColor);
		}

		cSrcRect.SetRect(0, 0, nBlockSize - 1, nBlockSize - 1);
		Paint(pWnd, &cSrcRect, &cSrcRect, nMaskColor);
	}

	return m_errCode;
}

ErrorCode CBofBitmap::Curtain(CBofWindow *pWnd, int nSpeed, int nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

	if (m_errCode == ERR_NONE) {
		CBofRect cRect;

		int nWidth = pWnd->Width();
		int nHeight = pWnd->Height();

		for (int i = 0; i < nHeight; i += nSpeed) {
			cRect.SetRect(0, i, nWidth - 1, i + nSpeed - 1);
			Paint(pWnd, &cRect, &cRect, nMaskColor);
			Sleep(1);
		}
	}

	return m_errCode;
}

ErrorCode CBofBitmap::FadeLines(CBofWindow *pWnd, CBofRect *pDstRect, CBofRect *pSrcRect, int nSpeed, int nMaskColor) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);
	Assert(nSpeed != 0);

	if (m_errCode == ERR_NONE) {

		CBofRect cDstRect, cSrcRect, cWindowRect, cBmpRect;

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

		int x1 = pDstRect->left;
		int y1 = pDstRect->top;
		int nWidth1 = pDstRect->Width();
		int nHeight1 = pDstRect->Height();

		int x2 = pSrcRect->left;
		int y2 = pSrcRect->top;
		int nWidth2 = pSrcRect->Width();

		for (int j = 0; j < 4; j++) {
			for (int i = 0; i < nHeight1 ; i += 4) {

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

const char *CBofBitmap::GetFileName() {
	Assert(IsValidObject(this));

	const char *p = nullptr;
	if (m_szFileName[0] != '\0')
		p = (const char *)&m_szFileName[0];

	return p;
}

//////////////////////////////////////////////////////////////////////////////
//
// Misc global graphics routines
//
//////////////////////////////////////////////////////////////////////////////

CBofBitmap *LoadBitmap(const char *pszFileName, CBofPalette *pPalette, bool bUseShared) {
	CBofPalette *pUsePal = pPalette;

	// If no palette was passed in and a shared palette was requested, then
	// use our default one established by "SHAREDPAL=" in the script
	if (bUseShared && pPalette == nullptr) {
		pUsePal = CBofPalette::GetSharedPalette();
	}

	CBofBitmap *pBmp = new CBofBitmap(pszFileName, pUsePal);

	return pBmp;
}

ErrorCode PaintBitmap(CBofWindow *pWindow, const char *pszFileName, CBofRect *pDstRect, CBofRect *pSrcRect, CBofPalette *pPalette, int nMaskColor) {
	Assert(pWindow != nullptr);
	Assert(pszFileName != nullptr);

	// Assume no error
	ErrorCode errCode = ERR_NONE;
	CBofBitmap *pBmp = new CBofBitmap(pszFileName, pPalette);

	if (pBmp != nullptr) {
		CBofRect cRect = pBmp->GetRect();

		if (pSrcRect == nullptr)
			pSrcRect = &cRect;

		if (pDstRect == nullptr)
			pDstRect = &cRect;

		// For this one draw, make sure that the right palette is
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

		// Restore the previous palette
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

ErrorCode PaintBitmap(CBofBitmap *pBitmap, const char *pszFileName, CBofRect *pDstRect, CBofRect *pSrcRect, CBofPalette *pPalette, int nMaskColor) {
	Assert(pBitmap != nullptr);
	Assert(pszFileName != nullptr);

	// Assume no error
	ErrorCode errCode = ERR_NONE;
	CBofBitmap *pBmp = new CBofBitmap(pszFileName, pPalette);
	if (pBmp != nullptr) {

		CBofRect cRect = pBmp->GetRect();

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

} // namespace Bagel
