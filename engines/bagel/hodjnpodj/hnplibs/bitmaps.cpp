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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/dibapi.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"

namespace Bagel {
namespace HodjNPodj {

CBitmap *FetchScreenBitmap(CDC *pDC, CPalette *pPalette, const int x, const int y, const int dx, const int dy) {
	bool        bSuccess = false;
	CDC         *pWorkDC = nullptr;
	CPalette    *pPalOld = nullptr,
	             *pPalOldWork = nullptr;
	CBitmap     *pWork = nullptr,
	             *pWorkOld = nullptr;

	if (pPalette != nullptr) {                                 // map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pPalette, false);
		(*pDC).RealizePalette();
	}

	pWorkDC = new CDC();                                    // create the context and bitmap objects
	pWork = new CBitmap();

	if ((pWorkDC != nullptr) &&                                // construct an offscreen bitmap that we
	        (pWork != nullptr) &&                                  // ... can use as a work area, and then
	        (*pWorkDC).CreateCompatibleDC(pDC) &&               // ... use as a return value to the caller
	        (*pWork).CreateCompatibleBitmap(pDC, dx, dy)) {     // create a bitmap of the appropriate size
		if (pPalette != nullptr) {                             // map the palette into the work area
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, false);
			(*pWorkDC).RealizePalette();
		}
		pWorkOld = (*pWorkDC).SelectObject(pWork);          // now map in the work area's bitmap
		if (pWorkOld != nullptr)                               // capture the desired pixels
			bSuccess = (*pWorkDC).BitBlt(0, 0, dx, dy, pDC, x, y, SRCCOPY);
	}

	if (pPalOld != nullptr)                                    // relinquish the resources we built
		(*pDC).SelectPalette(pPalOld, false);
	if (pWorkOld != nullptr)
		(*pWorkDC).SelectObject(pWorkOld);
	if (pPalOldWork != nullptr)
		(*pWorkDC).SelectPalette(pPalOldWork, false);
	if (!bSuccess &&                                        // release the bitmap result we built
	        (pWork != nullptr)) {                                  // ... if something failed
		(*pWork).DeleteObject();
		delete pWork;
		pWork = nullptr;
	}
	if (pWorkDC != nullptr) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}

	return pWork;
}

CBitmap *FetchBitmap(CDC *pDC, CPalette **pPalette, const char *pszPathName) {
	CDibDoc *pDIB;  // Pointer to our loaded DIB file
	HDIB hDIB = nullptr;
	CBitmap *pBitmap = nullptr;
	CPalette *pThisPalette = nullptr;

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != nullptr) &&                               // verify we have the objects we just created
	        (*pDIB).OpenDocument(pszPathName)) {            // .... bitmap file
		pThisPalette = (*pDIB).GetDocPalette();
		hDIB = (*pDIB).GetHDIB();
		pBitmap = ConvertDIB(pDC, hDIB, pThisPalette);
		if (pPalette != nullptr)
			*pPalette = (*pDIB).DetachPalette();
	}

	delete pDIB;

	return pBitmap;
}

CBitmap *FetchBitmap(CDC *pDC, CPalette *pPalette, const char *pszPathName) {
	CDibDoc     *pDIB = nullptr;                               // pointer to our loaded DIB file
	HDIB        hDIB = nullptr;
	CBitmap     *pBitmap = nullptr;

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != nullptr) &&                               // verify we have the objects we just created
	        (*pDIB).OpenDocument(pszPathName)) {            // .... bitmap file
		hDIB = (*pDIB).GetHDIB();
		pBitmap = ConvertDIB(pDC, hDIB, pPalette);
	}

	if (pDIB != nullptr)
		delete pDIB;

	return pBitmap;
}

CBitmap *FetchResourceBitmap(CDC *pDC, CPalette **pPalette, const char *pszName) {
	CDibDoc     *pDIB = nullptr;                               // pointer to our loaded DIB file
	HDIB        hDIB = nullptr;
	CBitmap     *pBitmap = nullptr;
	CPalette    *pThisPalette = nullptr;

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != nullptr) &&                               // verify we have the objects we just created
	        (*pDIB).OpenResourceDocument(pszName)) {        // .... bitmap file
		pThisPalette = (*pDIB).GetDocPalette();
		hDIB = (*pDIB).GetHDIB();
		pBitmap = ConvertDIB(pDC, hDIB, pThisPalette);
		if (pPalette != nullptr)
			*pPalette = (*pDIB).DetachPalette();
	}

	if (pDIB != nullptr)
		delete pDIB;

	return pBitmap;
}

CBitmap *FetchResourceBitmap(CDC *pDC, CPalette **pPalette, const int nResID) {
	CDibDoc     *pDIB = nullptr;                               // pointer to our loaded DIB file
	HDIB        hDIB = nullptr;
	CBitmap     *pBitmap = nullptr;
	char        chResName[16];
	CPalette    *pThisPalette = nullptr;

	Common::sprintf_s(chResName, "#%d", nResID);

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != nullptr) &&                               // verify we have the objects we just created
	        (*pDIB).OpenResourceDocument(chResName)) {      // .... bitmap file
		pThisPalette = (*pDIB).GetDocPalette();
		hDIB = (*pDIB).GetHDIB();
		pBitmap = ConvertDIB(pDC, hDIB, pThisPalette);
		if (pPalette != nullptr)
			*pPalette = (*pDIB).DetachPalette();
	}

	if (pDIB != nullptr)
		delete pDIB;

	return pBitmap;
}

CBitmap *ExtractBitmap(CDC *pDC, CBitmap *pBitmap, CPalette *pPalette, const int x, const int y, const int dx, const int dy) {
	bool        bSuccess = false;
	CDC         *pWorkDC = nullptr,
	             *pBaseDC = nullptr;
	CPalette    *pPalOld = nullptr,
	             *pPalOldWork = nullptr,
	              *pPalOldBase = nullptr;
	CBitmap     *pWork = nullptr,
	             *pWorkOld = nullptr,
	              *pBaseOld = nullptr;

	if (pPalette != nullptr) {                                 // map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pPalette, false);
		(*pDC).RealizePalette();
	}

	pWorkDC = new CDC();                                    // create the context and bitmap objects
	pWork = new CBitmap();
	pBaseDC = new CDC();

	if ((pWorkDC != nullptr) &&                                // construct an offscreen bitmap that we
	        (pWork != nullptr) &&                                  // ... can use as a work area, and then
	        (pBaseDC != nullptr) &&                                // ... use as a return value to the caller
	        (*pWorkDC).CreateCompatibleDC(pDC) &&               // setup a context for the source bitmap
	        (*pBaseDC).CreateCompatibleDC(pDC) &&
	        (*pWork).CreateCompatibleBitmap(pDC, dx, dy)) {     // create a bitmap of the appropriate size
		if (pPalette != nullptr) {                             // map the palette into the contexts
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, false);
			(*pWorkDC).RealizePalette();
			pPalOldBase = (*pBaseDC).SelectPalette(pPalette, false);
			(*pBaseDC).RealizePalette();
		}
		pWorkOld = (*pWorkDC).SelectObject(pWork);          // now map in the work area's bitmap
		pBaseOld = (*pBaseDC).SelectObject(pBitmap);        // ... as well as the source bitmap
		if ((pWorkOld != nullptr) &&
		        (pBaseOld != nullptr))                             // grab the bitmap section if all is well
			bSuccess = (*pWorkDC).BitBlt(0, 0, dx, dy, pBaseDC, x, y, SRCCOPY);
	}

	if (pPalOld != nullptr)                                    // relinquish the resources we built
		(*pDC).SelectPalette(pPalOld, false);
	if (pWorkOld != nullptr)
		(*pWorkDC).SelectObject(pWorkOld);
	if (pBaseOld != nullptr)
		(*pBaseDC).SelectObject(pBaseOld);
	if (pPalOldWork != nullptr)
		(*pWorkDC).SelectPalette(pPalOldWork, false);
	if (pPalOldBase != nullptr)
		(*pBaseDC).SelectPalette(pPalOldBase, false);
	if (!bSuccess &&                                        // release the bitmap result we built
	        (pWork != nullptr)) {                                  // ... if something failed
		(*pWork).DeleteObject();
		delete pWork;
		pWork = nullptr;
	}
	if (pWorkDC != nullptr) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}
	if (pBaseDC != nullptr) {
		(*pBaseDC).DeleteDC();
		delete pBaseDC;
	}

	return pWork;
}


/*************************************************************************
 *
 * BltBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  CBitmap *pBitmap        pointer to bitmap to be painted
 *  CRect *pSrcRect         source rectange in bitmap
 *  CRect *pDstRect         destination rectange in context
 *  int nMode               blt mode for transfer
 *
 * Return Value:
 *  bool            success / failure return
 *
 * Description:     paint a rectangular area of the screen from a CBitmap object.
 *
 ************************************************************************/

bool BltBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, CRect *pSrcRect, CRect *pDstRect, uint32 dwMode) {
	bool        bSuccess = false;
	CDC         *pWorkDC = nullptr;
	CPalette    *pPalOld = nullptr,
	             *pPalOldWork = nullptr;
	CBitmap     *pWorkOld = nullptr;

	if (pPalette != nullptr) {                                 // map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pPalette, false);
		(*pDC).RealizePalette();
	}

	pWorkDC = new CDC();                                    // create the context and bitmap objects

	if ((pWorkDC != nullptr) &&                                // verify we got what we asked for
	        (pBitmap != nullptr) &&
	        (*pWorkDC).CreateCompatibleDC(pDC)) {               // create a context for our bitmap
		if (pPalette != nullptr) {                             // map the palette into the context
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, false);
			(*pWorkDC).RealizePalette();
		}
		pWorkOld = (*pWorkDC).SelectObject(pBitmap);        // now map in our bitmap
		if (pWorkOld != nullptr) {                             // paint back the saved pixels
			if ((((*pSrcRect).right - (*pSrcRect).left) != ((*pDstRect).right - (*pDstRect).left)) ||
			        (((*pSrcRect).bottom - (*pSrcRect).top) != ((*pDstRect).bottom - (*pDstRect).top))) {
				(*pDC).SetStretchBltMode(STRETCH_DELETESCANS);
				assert(GetDeviceCaps((*pDC).m_hDC, RASTERCAPS) & RC_STRETCHBLT);
				bSuccess = (*pDC).StretchBlt((*pDstRect).left,
					(*pDstRect).top,
					(*pDstRect).right - (*pDstRect).left,
					(*pDstRect).bottom - (*pDstRect).top,
					pWorkDC,
					(*pSrcRect).left,
					(*pSrcRect).top,
					(*pSrcRect).right - (*pSrcRect).left,
					(*pSrcRect).bottom - (*pSrcRect).top,
					dwMode);
			} else {
				bSuccess = (*pDC).BitBlt((*pDstRect).left,
					(*pDstRect).top,
					(*pSrcRect).right - (*pSrcRect).left,
					(*pSrcRect).bottom - (*pSrcRect).top,
					pWorkDC,
					(*pSrcRect).left,
					(*pSrcRect).top,
					dwMode);
			}
		}
	}

	if (pPalOld != nullptr)                                    // relinquish the resources we built
		(*pDC).SelectPalette(pPalOld, false);
	if (pWorkOld != nullptr)
		(*pWorkDC).SelectObject(pWorkOld);
	if (pPalOldWork != nullptr)
		(*pWorkDC).SelectPalette(pPalOldWork, false);
	if (pWorkDC != nullptr) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}

	return bSuccess;
}


/*************************************************************************
 *
 * BltMaskedBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  CBitmap *pBitmap        pointer to bitmap to be displayed
 *  int x, y                upper lefthand corner of the destination rectangle
 *  CRect *pSrcRect         source rectange in bitmap
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     "mask" paint a bitmap to the specified display context.  White
 *                  pixels are assumed to be transparent, allowing the background
 *                  artwork to show through.
 *
 ************************************************************************/

bool BltMaskedBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, CRect *pSrcRect, const int x, const int y) {
	CDC         *pImageDC = nullptr,                       // context for the bitmap image
	             *pWorkDC = nullptr,                        // context for the bitmap work area
	              *pMaskDC = nullptr;                        // context for the image's mask
	CBitmap     *pImage = nullptr,                         // bitmap that has our image
	             *pImageOld = nullptr,                      // bitmap previously mapped to image context
	              *pWork = nullptr,                          // work area bitmap
	               *pWorkOld = nullptr,                       // bitmap previously mapped to work context
	                *pMask = nullptr,                          // image mask bitmap
	                 *pMaskOld = nullptr;                       // bitmap previously mapped to mask context
	CPalette    *pPalOldImage = nullptr,                   // palette previously mapped to image context
	             *pPalOldWork = nullptr;                    // palette previously mapped to work context
	bool        bSuccess = false;
	CPalette    *pPalOld = nullptr;
	CSize       cSize;
	int         dx, dy;
	CRect       dstRect;

	if (pPalette != nullptr) {
		pPalOld = (*pDC).SelectPalette(pPalette, false);
		(*pDC).RealizePalette();
	}

	pImageDC = new CDC();                               // create the necessary objects
	pMaskDC = new CDC();
	pWorkDC = new CDC();
	pImage = new CBitmap();
	pMask = new CBitmap();
	pWork = new CBitmap();

	if ((pImageDC != nullptr) &&                               // verify we got the requested objects
	        (pMaskDC != nullptr) &&
	        (pWorkDC != nullptr) &&
	        (pImage != nullptr) &&
	        (pMask != nullptr) &&
	        (pWork != nullptr) &&
	        (*pImageDC).CreateCompatibleDC(pDC) &&              // now create all the compatible contexts
	        (*pMaskDC).CreateCompatibleDC(pDC) &&               // ... that we need to hold our bitmaps
	        (*pWorkDC).CreateCompatibleDC(pDC)) {

		dx = (*pSrcRect).right - (*pSrcRect).left;
		dy = (*pSrcRect).bottom - (*pSrcRect).top;

		if (pPalette != nullptr) {                     // map in the color palette if specified
			pPalOldImage = (*pImageDC).SelectPalette(pPalette, false);
			(*pImageDC).RealizePalette();
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, false);
			(*pWorkDC).RealizePalette();
		}

		if ((*pImage).CreateCompatibleBitmap(pDC, dx, dy) &&
		        (*pWork).CreateCompatibleBitmap(pDC, dx, dy) &&
		        (*pMask).CreateBitmap(dx, dy, 1, 1, nullptr)) {

			pImageOld = (*pImageDC).SelectObject(pImage);
			pMaskOld = (*pMaskDC).SelectObject(pMask);  // map the work and mask bitmaps into
			pWorkOld = (*pWorkDC).SelectObject(pWork);  // ... their contexts

			dstRect.SetRect(0, 0, dx, dy);
			BltBitmap(pImageDC, pPalette, pBitmap, pSrcRect, &dstRect, (uint32) SRCCOPY);

			if ((pMaskOld != nullptr) &&                   // verify so far so good
			        (pWorkOld != nullptr)) {
				(*pWorkDC).BitBlt(     // grab what the background looks like
				    0,                              // ... putting it in the work area
				    0,
				    dx,
				    dy,
				    pDC,
				    x,
				    y,
				    SRCCOPY);
				(*pMaskDC).BitBlt(     // create the mask by inverting the image
				    0, 0,                           // ... i.e. the pixels occupied by the image
				    dx,
				    dy,
				    pImageDC,
				    0, 0,
				    NOTSRCCOPY);
				(*pImageDC).BitBlt(     // remove the image's "transparent" white area
				    0, 0,                           // ... i.e. its background becomes black
				    dx,
				    dy,
				    pMaskDC,
				    0, 0,
				    SRCAND);
				(*pMaskDC).BitBlt(     // invert the mask
				    0, 0,
				    dx,
				    dy,
				    pMaskDC,
				    0, 0,
				    DSTINVERT);
				(*pWorkDC).BitBlt(     // remove the image's space from the work area
				    0, 0,                           // ... i.e. the pixels where the image will go
				    dx,
				    dy,
				    pMaskDC,
				    0, 0
				    , SRCAND);
				(*pWorkDC).BitBlt(     // paint the image into the cleared (black) space
				    0, 0,                           // ... we made in the work area
				    dx,
				    dy,
				    pImageDC,
				    0, 0,
				    SRCPAINT);
				bSuccess = (*pDC).BitBlt(    // now splat the result to the destination context
				               x,
				               y,
				               dx,
				               dy,
				               pWorkDC,
				               0,
				               0,
				               SRCCOPY);
			}
		}
	}

	if (pPalOld != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	if (pImageOld != nullptr)                              // select out bitmaps out of their contexts
		(*pImageDC).SelectObject(pImageOld);
	if (pWorkOld != nullptr)
		(*pWorkDC).SelectObject(pWorkOld);
	if (pMaskOld != nullptr)
		(*pMaskDC).SelectObject(pMaskOld);

	if (pPalOldImage != nullptr)                           // map out the palettes we used
		(*pImageDC).SelectPalette(pPalOldImage, false);
	if (pPalOldWork != nullptr)
		(*pWorkDC).SelectPalette(pPalOldWork, false);

	if (pImage != nullptr) {                                // delete the bitmaps we created
		(*pImage).DeleteObject();
		delete pImage;
	}
	if (pWork != nullptr) {
		(*pWork).DeleteObject();
		delete pWork;
	}
	if (pMask != nullptr) {
		(*pMask).DeleteObject();
		delete pMask;
	}

	if (pImageDC != nullptr) {                             // delete the contexts we created
		(*pImageDC).DeleteDC();
		delete pImageDC;
	}
	if (pWorkDC != nullptr) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}
	if (pMaskDC != nullptr) {
		(*pMaskDC).DeleteDC();
		delete pMaskDC;
	}

	return bSuccess;                                  // return success or failure
}


/*************************************************************************
 *
 * PaintBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  CBitmap *pBitmap        pointer to bitmap to be painted
 *  int x, y                upper left hand corner where bitmap is to be painted
 *  int dx, dy              optional sizing info to force a stretch blt
 *
 * Return Value:
 *  bool            success / failure return
 *
 * Description:     paint a rectangular area of the screen from a CBitmap object.
 *
 ************************************************************************/

bool PaintBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, const int x, const int y, const int dx, const int dy) {
	bool        bSuccess = false;
	CDC         *pWorkDC = nullptr;
	CPalette    *pPalOld = nullptr,
	             *pPalOldWork = nullptr;
	CBitmap     *pWorkOld = nullptr;
	BITMAP      cBitmapData;
	CRect       SrcRect, DstRect;

	(*pBitmap).GetObject(sizeof(BITMAP), &cBitmapData);

	if (pPalette != nullptr) {                                 // map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pPalette, false);
		(*pDC).RealizePalette();
	}

	pWorkDC = new CDC();                                    // create the context and bitmap objects

	if ((pWorkDC != nullptr) &&                                // verify we got what we asked for
	        (pBitmap != nullptr) &&
	        (*pWorkDC).CreateCompatibleDC(pDC)) {               // create a context for our bitmap
		if (pPalette != nullptr) {                             // map the palette into the context
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, false);
			(*pWorkDC).RealizePalette();
		}
		pWorkOld = (*pWorkDC).SelectObject(pBitmap);        // now map in our bitmap
		if (pWorkOld != nullptr) {                             // paint back the saved pixels
			if ((dx != 0) && (dy != 0) &&
			        ((dx != cBitmapData.bmWidth) || (dy != cBitmapData.bmHeight))) {
				(*pDC).SetStretchBltMode(STRETCH_DELETESCANS);
				assert(GetDeviceCaps((*pDC).m_hDC, RASTERCAPS) & RC_STRETCHBLT);
				bSuccess = (*pDC).StretchBlt(x, y, dx, dy, pWorkDC, 0, 0, cBitmapData.bmWidth, cBitmapData.bmHeight, SRCCOPY);
			} else {
				bSuccess = (*pDC).BitBlt(x, y, cBitmapData.bmWidth, cBitmapData.bmHeight, pWorkDC, 0, 0, SRCCOPY);
			}
		}
	}

	if (pPalOld != nullptr)                                    // relinquish the resources we built
		(*pDC).SelectPalette(pPalOld, false);
	if (pWorkOld != nullptr)
		(*pWorkDC).SelectObject(pWorkOld);
	if (pPalOldWork != nullptr)
		(*pWorkDC).SelectPalette(pPalOldWork, false);
	if (pWorkDC != nullptr) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}

	return bSuccess;
}


/*************************************************************************
 *
 * PaintBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  char *pszPathName       pointer to path string for disk based DIB file
 *  int x, y                upper left hand corner where bitmap is to be painted
 *  int dx, dy              optional sizing info to force a stretch blt
 *
 * Return Value:
 *  bool            success / failure return
 *
 * Description:     paint a rectangular area of the screen from a disk based DIB.
 *
 ************************************************************************/

bool PaintBitmap(CDC *pDC, CPalette *pPalette, const char *pszPathName, const int x, const int y, const int dx, const int dy) {
	CDibDoc myDoc;
	HDIB    hDIB;
	CRect   rcDest;
	CRect   rcDIB;
	int     cxDIB;
	int     cyDIB;
	bool    bSuccess;

	bSuccess = myDoc.OpenDocument(pszPathName);
	if ((pDC != nullptr) && bSuccess) {
		hDIB = myDoc.GetHDIB();
		if (hDIB == nullptr)
			bSuccess = false;
		else {
			cxDIB = (int) DIBWidth(hDIB);
			cyDIB = (int) DIBHeight(hDIB);

			rcDIB.top = rcDIB.left = 0;
			rcDIB.right = cxDIB;
			rcDIB.bottom = cyDIB;
			if ((dx == 0) || (dy == 0))
				rcDest.SetRect(x, y, x + cxDIB, y + cyDIB);
			else
				rcDest.SetRect(x, y, x + dx, y + dy);
			bSuccess = PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pPalette);
		}
	}

	return bSuccess;
}


/*************************************************************************
 *
 * PaintMaskedDIB()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  char *pszPathName       pointer to path string for disk based DIB file
 *  int x, y                upper lefthand corner of the destination rectangle
 *  int dx, dy              optional sizing info to force a stretch blt
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Read in a DDB bitmap from disk and "mask" paint it to the
 *                  specified display context.  White pixels are assumed to be
 *                  transparent, allowing the background artwork to show through.
 *
 ************************************************************************/

bool PaintMaskedDIB(CDC *pDC, CPalette *pPalette, const char *pszPathName, const int x, const int y, const int dx, const int dy) {
	bool    bSuccess = false;
	CDibDoc *pDIB = nullptr;                                   // pointer to our loaded DIB file

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != nullptr) &&                               // verify we have the objects we just created
	        (*pDIB).OpenDocument(pszPathName))              // .... bitmap file
		bSuccess = PaintMaskedDIB(pDC, pPalette, pDIB, x, y, dx, dy) ;

	if (pDIB != nullptr)
		delete pDIB;

	return bSuccess;                                  // return success or failure
}


/*************************************************************************
 *
 * PaintMaskedResource()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  int resId               resource number for the bitmap in the .RC file
 *  int x, y                upper lefthand corner of the destination rectangle
 *  int dx, dy              optional sizing info to force a stretch blt
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Read in a DDB bitmap resource and "mask" paint it to the
 *                  specified display context.  White pixels are assumed to be
 *                  transparent, allowing the background artwork to show through.
 *
 ************************************************************************/

bool PaintMaskedResource(CDC *pDC, CPalette *pPalette, const int nResID, const int x, const int y, const int dx, const int dy) {
	bool    bSuccess = false;
	CDibDoc *pDIB = nullptr;                                   // pointer to our loaded DIB file
	char    chResName[16];

	Common::sprintf_s(chResName, "#%d", nResID);

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != nullptr) &&                               // verify we have the objects we just created
	        (*pDIB).OpenResourceDocument(chResName))        // .... bitmap file
		bSuccess = PaintMaskedDIB(pDC, pPalette, pDIB, x, y, dx, dy) ;

	if (pDIB != nullptr)
		delete pDIB;

	return bSuccess;                                  // return success or failure
}



/*************************************************************************
 *
 * PaintMaskedResource()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  char *pszName           resource name for the bitmap in the .RC file
 *  int x, y                upper lefthand corner of the destination rectangle
 *  int dx, dy              optional sizing info to force a stretch blt
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Read in a DDB bitmap resource and "mask" paint it to the
 *                  specified display context.  White pixels are assumed to be
 *                  transparent, allowing the background artwork to show through.
 *
 ************************************************************************/

bool PaintMaskedResource(CDC *pDC, CPalette *pPalette, const char *pszName, const int x, const int y, const int dx, const int dy) {
	bool    bSuccess = false;
	CDibDoc *pDIB = nullptr;                                   // pointer to our loaded DIB file

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != nullptr) &&                               // verify we have the objects we just created
	        (*pDIB).OpenResourceDocument(pszName))          // .... bitmap file
		bSuccess = PaintMaskedDIB(pDC, pPalette, pDIB, x, y, dx, dy) ;

	if (pDIB != nullptr)
		delete pDIB;

	return bSuccess;                                  // return success or failure
}


/*************************************************************************
 *
 * PaintMaskedDIB()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  CDibDoc *pDIB           pointer to DIB to be displayed
 *  int x, y                upper lefthand corner of the destination rectangle
 *  int dx, dy              optional sizing info to force a stretch blt
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     "mask" paint a DIB to the specified display context.  White
 *                  pixels are assumed to be transparent, allowing the background
 *                  artwork to show through.
 *
 ************************************************************************/

bool PaintMaskedDIB(CDC *pDC, CPalette *pPalette, CDibDoc *pDIB, const int x, const int y, const int dx, const int dy) {
	bool        bSuccess = false;
	CBitmap     *pBitmap = nullptr;
	CPalette    *pMyPalette = nullptr;
	HDIB        hDIB = nullptr;

	if (pPalette == nullptr)
		pMyPalette = (*pDIB).GetDocPalette();
	else
		pMyPalette = pPalette;

	hDIB = (*pDIB).GetHDIB();
	pBitmap = ConvertDIB(pDC, hDIB, pMyPalette);

	if (pBitmap != nullptr) {
		bSuccess = PaintMaskedBitmap(pDC, pMyPalette, pBitmap, x, y, dx, dy);
		(*pBitmap).DeleteObject();
		delete pBitmap;
	}

	return bSuccess;
}


/*************************************************************************
 *
 * PaintMaskedBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  CBitmap *pBitmap        pointer to bitmap to be displayed
 *  int x, y                upper lefthand corner of the destination rectangle
 *  int dx, dy              optional sizing info to force a stretch blt
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     "mask" paint a bitmap to the specified display context.  White
 *                  pixels are assumed to be transparent, allowing the background
 *                  artwork to show through.
 *
 ************************************************************************/

bool PaintMaskedBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, const int x, const int y, const int dx, const int dy) {
	BITMAP      myBitmap;                               // area for bitmap info
	CDC         *pImageDC = nullptr,                       // context for the bitmap image
	             *pWorkDC = nullptr,                        // context for the bitmap work area
	              *pMaskDC = nullptr;                        // context for the image's mask
	CBitmap     *pImage = nullptr,                         // bitmap that has our image
	             *pImageOld = nullptr,                      // bitmap previously mapped to image context
	              *pWork = nullptr,                          // work area bitmap
	               *pWorkOld = nullptr,                       // bitmap previously mapped to work context
	                *pMask = nullptr,                          // image mask bitmap
	                 *pMaskOld = nullptr;                       // bitmap previously mapped to mask context
	CPalette    *pPalOldImage = nullptr,                   // palette previously mapped to image context
	             *pPalOldWork = nullptr;                    // palette previously mapped to work context
	bool        bSuccess = false;
	CPalette    *pPalOld = nullptr;
	CSize       cSize;

	if (pPalette != nullptr) {
		pPalOld = (*pDC).SelectPalette(pPalette, false);
		(*pDC).RealizePalette();
	}

	pImageDC = new CDC();                               // create the necessary objects
	pMaskDC = new CDC();
	pWorkDC = new CDC();
	pImage = new CBitmap();
	pMask = new CBitmap();
	pWork = new CBitmap();

	if ((pImageDC != nullptr) &&                               // verify we got the requested objects
	        (pMaskDC != nullptr) &&
	        (pWorkDC != nullptr) &&
	        (pImage != nullptr) &&
	        (pMask != nullptr) &&
	        (pWork != nullptr) &&
	        (*pImageDC).CreateCompatibleDC(pDC) &&              // now create all the compatible contexts
	        (*pMaskDC).CreateCompatibleDC(pDC) &&               // ... that we need to hold our bitmaps
	        (*pWorkDC).CreateCompatibleDC(pDC)) {

		(*pBitmap).GetObject(sizeof(BITMAP), &myBitmap);  // get the image's sizing info
		cSize.cx = myBitmap.bmWidth;
		cSize.cy = myBitmap.bmHeight;

		if ((dx != 0) && (dy != 0) &&
		        ((dx != cSize.cx) || (dy != cSize.cy))) {
			cSize.cx = dx;
			cSize.cy = dy;
		}

		if (pPalette != nullptr) {                     // map in the color palette if specified
			pPalOldImage = (*pImageDC).SelectPalette(pPalette, false);
			(*pImageDC).RealizePalette();
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, false);
			(*pWorkDC).RealizePalette();
		}

		if ((*pImage).CreateCompatibleBitmap(pDC, cSize.cx, cSize.cy) &&
		        (*pWork).CreateCompatibleBitmap(pDC, cSize.cx, cSize.cy) &&
		        (*pMask).CreateBitmap(cSize.cx, cSize.cy, 1, 1, nullptr)) {

			pImageOld = (*pImageDC).SelectObject(pImage);
			pMaskOld = (*pMaskDC).SelectObject(pMask);  // map the work and mask bitmaps into
			pWorkOld = (*pWorkDC).SelectObject(pWork);  // ... their contexts

			PaintBitmap(pImageDC, pPalette, pBitmap, 0, 0, dx, dy);

			if ((pMaskOld != nullptr) &&                   // verify so far so good
			        (pWorkOld != nullptr)) {
				(*pWorkDC).BitBlt(     // grab what the background looks like
				    0,                              // ... putting it in the work area
				    0,
				    cSize.cx,
				    cSize.cy,
				    pDC,
				    x,
				    y,
				    SRCCOPY);
				(*pMaskDC).BitBlt(     // create the mask by inverting the image
				    0, 0,                           // ... i.e. the pixels occupied by the image
				    cSize.cx,
				    cSize.cy,
				    pImageDC,
				    0, 0,
				    NOTSRCCOPY);
				(*pImageDC).BitBlt(     // remove the image's "transparent" white area
				    0, 0,                           // ... i.e. its background becomes black
				    cSize.cx,
				    cSize.cy,
				    pMaskDC,
				    0, 0,
				    SRCAND);
				(*pMaskDC).BitBlt(     // invert the mask
				    0, 0,
				    cSize.cx,
				    cSize.cy,
				    pMaskDC,
				    0, 0,
				    DSTINVERT);
				(*pWorkDC).BitBlt(     // remove the image's space from the work area
				    0, 0,                           // ... i.e. the pixels where the image will go
				    cSize.cx,
				    cSize.cy,
				    pMaskDC,
				    0, 0
				    , SRCAND);
				(*pWorkDC).BitBlt(     // paint the image into the cleared (black) space
				    0, 0,                           // ... we made in the work area
				    cSize.cx,
				    cSize.cy,
				    pImageDC,
				    0, 0,
				    SRCPAINT);
				bSuccess = (*pDC).BitBlt(    // now splat the result to the destination context
				               x,
				               y,
				               cSize.cx,
				               cSize.cy,
				               pWorkDC,
				               0,
				               0,
				               SRCCOPY);
			}
		}
	}

	if (pPalOld != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	if (pImageOld != nullptr)                              // select out bitmaps out of their contexts
		(*pImageDC).SelectObject(pImageOld);
	if (pWorkOld != nullptr)
		(*pWorkDC).SelectObject(pWorkOld);
	if (pMaskOld != nullptr)
		(*pMaskDC).SelectObject(pMaskOld);

	if (pPalOldImage != nullptr)                           // map out the palettes we used
		(*pImageDC).SelectPalette(pPalOldImage, false);
	if (pPalOldWork != nullptr)
		(*pWorkDC).SelectPalette(pPalOldWork, false);

	if (pImage != nullptr) {                                // delete the bitmaps we created
		(*pImage).DeleteObject();
		delete pImage;
	}
	if (pWork != nullptr) {
		(*pWork).DeleteObject();
		delete pWork;
	}
	if (pMask != nullptr) {
		(*pMask).DeleteObject();
		delete pMask;
	}

	if (pImageDC != nullptr) {                             // delete the contexts we created
		(*pImageDC).DeleteDC();
		delete pImageDC;
	}
	if (pWorkDC != nullptr) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}
	if (pMaskDC != nullptr) {
		(*pMaskDC).DeleteDC();
		delete pMaskDC;
	}

	return bSuccess;                                  // return success or failure
}


/*************************************************************************
 *
 * PaintBlockEffect()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CBitmap *pBitmap        pointer to the DIB to be displayed
 *  CPalette *pPalette      pointer to color palette to be used for the DIB
 *  int nBlockSize          size of block to be painted at each step
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     randomly paint a DIB to the specified display context
 *                  a section/block at a time, without repeating any blocks;
 *                  the DIB is converted to a bitmap and then a companion
 *                  routine is called.
 *
 ************************************************************************/

#define BIT0    0x00000001
#define BIT2    0x00000004
#define BIT15   0x00008000
#define BIT16   0x00010000
#define BIT17   0x00020000
#define SEQ     (BIT17)
#define BLOCK   4


bool PaintBlockEffect(CDC *pDC, CDibDoc *pDIB, CPalette* pPalette, int nBlockSize) {
	HDIB    hDIB = nullptr;
	CBitmap *pBitmap = nullptr;
	bool    bSuccess = false;

	if ((pDC == nullptr) ||
	        (pDIB == nullptr))
		return false;

	hDIB = (*pDIB).GetHDIB();
	pBitmap = ConvertDIB(pDC, hDIB, pPalette);

	bSuccess = PaintBlockEffect(pDC, pBitmap, pPalette, nBlockSize);

	if (pBitmap != nullptr)
		delete pBitmap;

	return bSuccess;
}


/*************************************************************************
 *
 * PaintBlockEffect()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CBitmap *pBitmap        pointer to bitmap to be displayed
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  int nBlockSize          size of block to be painted at each step
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     randomly paint a bitmap to the specified display context
 *                  a section/block at a time, without repeating blocks.
 *
 ************************************************************************/

bool PaintBlockEffect(CDC *pDC, CBitmap *pBitmap, CPalette* pPalette, int nBlockSize) {
	CPalette *pOldPal = nullptr;
	CPalette *pOldPal2 = nullptr;
	CBitmap  *pBitmapOld = nullptr;
	CDC      *pMemDC = nullptr;
	BITMAP   myBitmap;
	bool     bSuccess = false;
	unsigned long   seed;
	unsigned long   value, maxvalue;
	unsigned long   x, y;
	unsigned long   fill,
	         mask = (BIT16 | BIT15 | BIT2 | BIT0);
	unsigned long   tmp, cnt, shft;
	unsigned long   width, height;

	if ((pDC == nullptr) ||
	        (pBitmap == nullptr))
		return false;

	if (pPalette != nullptr) {
		pOldPal = (*pDC).SelectPalette(pPalette, false);
		(*pDC).RealizePalette();
	}

	pMemDC = new CDC();
	if ((pMemDC == nullptr) ||
	        ((*pMemDC).CreateCompatibleDC(pDC) == false))
		goto clean_up;

	pOldPal2 = (*pMemDC).SelectPalette(pPalette, false);
	(*pMemDC).RealizePalette();
	pBitmapOld = (*pMemDC).SelectObject(pBitmap);

	(*pBitmap).GetObject(sizeof(BITMAP), &myBitmap);
	width = myBitmap.bmWidth / nBlockSize;
	height = myBitmap.bmHeight;

	maxvalue = height / nBlockSize * width;
	seed = SEQ - 1;

	for (value = seed; ;) {
		fill = 0L;
		tmp = (value & mask);
		shft = BIT0;
		cnt = 0;
		while (shft < SEQ) {
			if (tmp & shft)
				cnt++;
			shft <<= 1;
		}
		if (cnt & BIT0)
			fill = BIT0;
		value <<= 1;
		value |= fill;
		value &= (SEQ - 1);
		if (value == seed)
			break;
		if (value > maxvalue)
			continue;
		y = (value / width) * nBlockSize;
		x = (value % width) * nBlockSize;
		(*pDC).BitBlt((uint16) x,
		              (uint16) y,
		              nBlockSize,
		              nBlockSize,
		              pMemDC,
		              (uint16) x,
		              (uint16) y,
		              SRCCOPY);
	}

	(*pDC).BitBlt(0,
	              0,
	              nBlockSize,
	              nBlockSize,
	              pMemDC,
	              0,
	              0,
	              SRCCOPY);

	bSuccess = true;

clean_up:
	if (pBitmapOld != nullptr)
		(*pMemDC).SelectObject(pBitmapOld);
	if (pOldPal2 != nullptr)
		(*pMemDC).SelectPalette(pOldPal2, false);
	if (pMemDC != nullptr)
		delete pMemDC;

	if (pOldPal != nullptr)
		(*pDC).SelectPalette(pOldPal, false);

	return bSuccess;
}


/*************************************************************************
 *
 * PaintBlockEffect()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  COLORREF                color to be displayed
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  int nBlockSize          size of block to be painted at each step
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     randomly paint a solid background color to the specified
 *                  display context a section/block at a time, without repeating blocks.
 *
 ************************************************************************/

bool PaintBlockEffect(CDC *pDC, COLORREF rgbColor, CPalette* pPalette, int nBlockSize, int nX, int nY, int nWidth, int nHeight) {
	CBrush   myBrush;
	CRect    thisRect, fillRect;
	CPalette *pOldPal = nullptr;
	bool     bSuccess = false;
	unsigned long   seed;
	unsigned long   value, maxvalue;
	unsigned long   x, y;
	unsigned long   fill,
	         mask = (BIT16 | BIT15 | BIT2 | BIT0);
	unsigned long   tmp, cnt, shft;
	unsigned long   width, height;

	if (pDC == nullptr)
		return false;

	if (pPalette != nullptr) {
		pOldPal = (*pDC).SelectPalette(pPalette, false);
		(*pDC).RealizePalette();
	}

	myBrush.CreateSolidBrush(rgbColor);

	width = nWidth / nBlockSize;
	height = nHeight;

	maxvalue = height / nBlockSize * width;
	seed = SEQ - 1;

	for (value = seed; ;) {
		fill = 0L;
		tmp = (value & mask);
		shft = BIT0;
		cnt = 0;
		while (shft < SEQ) {
			if (tmp & shft)
				cnt++;
			shft <<= 1;
		}
		if (cnt & BIT0)
			fill = BIT0;
		value <<= 1;
		value |= fill;
		value &= (SEQ - 1);
		if (value == seed)
			break;
		if (value > maxvalue)
			continue;
		y = (value / width) * nBlockSize;
		x = (value % width) * nBlockSize;
		fillRect.SetRect(nX + x, nY + y, nX + x + nBlockSize, nY + y + nBlockSize);
		(*pDC).FillRect(&fillRect, &myBrush);
	}

	fillRect.SetRect(nX, nY, nX + nBlockSize, nY + nBlockSize);
	(*pDC).FillRect(&fillRect, &myBrush);

	bSuccess = true;

	if (pOldPal != nullptr)
		(*pDC).SelectPalette(pOldPal, false);

	return bSuccess;
}


/*************************************************************************
 *
 * GetBitmapSize()
 *
 * Parameters:
 *  CBitmap *pBitmap        pointer to bitmap
 *
 * Return Value:
 *  CSize           size of the bitmap
 *
 * Description:     get sizing information for a bitmap objext.
 *
 ************************************************************************/

CSize GetBitmapSize(CBitmap *pBitmap) {
	CSize   mySize;
	BITMAP  cBitmapData;

	(*pBitmap).GetObject(sizeof(BITMAP), &cBitmapData);
	mySize.cx = cBitmapData.bmWidth;
	mySize.cy = cBitmapData.bmHeight;
	return mySize;
}

} // namespace HodjNPodj
} // namespace Bagel
