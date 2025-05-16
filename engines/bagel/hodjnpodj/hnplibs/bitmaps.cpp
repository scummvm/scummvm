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

/*************************************************************************
 *
 * FetchScreenBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  int x, y                upper left hand corner of bitmap to fetch
 *  int dx, dy              size of rectangular area to retrive
 *
 * Return Value:
 *  CBitmap *               pointer to bitmap object or NULL
 *
 * Description:     return a rectangular area of the screen in a CBitmap object.
 *
 ************************************************************************/

CBitmap *FetchScreenBitmap(CDC *pDC, CPalette *pPalette, const int x, const int y, const int dx, const int dy) {
	BOOL        bSuccess = FALSE;
	CDC         *pWorkDC = NULL;
	CPalette    *pPalOld = NULL,
	             *pPalOldWork = NULL;
	CBitmap     *pWork = NULL,
	             *pWorkOld = NULL;

	if (pPalette != NULL) {                                 // map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pPalette, FALSE);
		(void)(*pDC).RealizePalette();
	}

	pWorkDC = new CDC();                                    // create the context and bitmap objects
	pWork = new CBitmap();

	if ((pWorkDC != NULL) &&                                // construct an offscreen bitmap that we
	        (pWork != NULL) &&                                  // ... can use as a work area, and then
	        (*pWorkDC).CreateCompatibleDC(pDC) &&               // ... use as a return value to the caller
	        (*pWork).CreateCompatibleBitmap(pDC, dx, dy)) {     // create a bitmap of the appropriate size
		if (pPalette != NULL) {                             // map the palette into the work area
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, FALSE);
			(void)(*pWorkDC).RealizePalette();
		}
		pWorkOld = (*pWorkDC).SelectObject(pWork);          // now map in the work area's bitmap
		if (pWorkOld != NULL)                               // capture the desired pixels
			bSuccess = (*pWorkDC).BitBlt(0, 0, dx, dy, pDC, x, y, SRCCOPY);
	}

	if (pPalOld != NULL)                                    // relinquish the resources we built
		(void)(*pDC).SelectPalette(pPalOld, FALSE);
	if (pWorkOld != NULL)
		(void)(*pWorkDC).SelectObject(pWorkOld);
	if (pPalOldWork != NULL)
		(void)(*pWorkDC).SelectPalette(pPalOldWork, FALSE);
	if (!bSuccess &&                                        // release the bitmap result we built
	        (pWork != NULL)) {                                  // ... if something failed
		(*pWork).DeleteObject();
		delete pWork;
		pWork = NULL;
	}
	if (pWorkDC != NULL) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}

	return (pWork);
}


/*************************************************************************
 *
 * FetchBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette **pPalette     address of pointer where to store palette from the DIB
 *  char *pszPathName       pointer to path string for disk based DIB file
 *
 * Return Value:
 *  CBitmap *               pointer to bitmap object or NULL
 *
 * Description:     Read in a DDB bitmap from disk and return it as a CBitmap.
 *
 ************************************************************************/

CBitmap *FetchBitmap(CDC *pDC, CPalette **pPalette, const char *pszPathName) {
	CDibDoc     *pDIB = NULL;                               // pointer to our loaded DIB file
	HDIB        hDIB = NULL;
	CBitmap     *pBitmap = NULL;
	CPalette    *pThisPalette = NULL;

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != NULL) &&                               // verify we have the objects we just created
	        (*pDIB).OpenDocument(pszPathName)) {            // .... bitmap file
		pThisPalette = (*pDIB).GetDocPalette();
		hDIB = (*pDIB).GetHDIB();
		pBitmap = ConvertDIB(pDC, hDIB, pThisPalette);
		if (pPalette != NULL)
			*pPalette = (*pDIB).DetachPalette();
	}

	if (pDIB != NULL)
		delete pDIB;

	return (pBitmap);
}


/*************************************************************************
 *
 * FetchBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette **pPalette     address of pointer where to store palette from the DIB
 *  char *pszPathName       pointer to path string for disk based DIB file
 *
 * Return Value:
 *  CBitmap *               pointer to bitmap object or NULL
 *
 * Description:     Read in a DDB bitmap from disk and return it as a CBitmap.
 *
 ************************************************************************/

CBitmap *FetchBitmap(CDC *pDC, CPalette *pPalette, const char *pszPathName) {
	CDibDoc     *pDIB = NULL;                               // pointer to our loaded DIB file
	HDIB        hDIB = NULL;
	CBitmap     *pBitmap = NULL;

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != NULL) &&                               // verify we have the objects we just created
	        (*pDIB).OpenDocument(pszPathName)) {            // .... bitmap file
		hDIB = (*pDIB).GetHDIB();
		pBitmap = ConvertDIB(pDC, hDIB, pPalette);
	}

	if (pDIB != NULL)
		delete pDIB;

	return (pBitmap);
}


/*************************************************************************
 *
 * FetchResourceBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette **pPalette     address of pointer where to store palette from the DIB
 *  char *pszName           pointer to name string for resource a based DIB
 *
 * Return Value:
 *  CBitmap *               pointer to bitmap object or NULL
 *
 * Description:     Read in a DDB bitmap resource and return it as a CBitmap.
 *
 ************************************************************************/

CBitmap *FetchResourceBitmap(CDC *pDC, CPalette **pPalette, const char *pszName) {
	CDibDoc     *pDIB = NULL;                               // pointer to our loaded DIB file
	HDIB        hDIB = NULL;
	CBitmap     *pBitmap = NULL;
	CPalette    *pThisPalette = NULL;

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != NULL) &&                               // verify we have the objects we just created
	        (*pDIB).OpenResourceDocument(pszName)) {        // .... bitmap file
		pThisPalette = (*pDIB).GetDocPalette();
		hDIB = (*pDIB).GetHDIB();
		pBitmap = ConvertDIB(pDC, hDIB, pThisPalette);
		if (pPalette != NULL)
			*pPalette = (*pDIB).DetachPalette();
	}

	if (pDIB != NULL)
		delete pDIB;

	return (pBitmap);
}


/*************************************************************************
 *
 * FetchResourceBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CPalette **pPalette     address of pointer where to store palette from the DIB
 *  int nResID              identifier for a resource based DIB
 *
 * Return Value:
 *  CBitmap *               pointer to bitmap object or NULL
 *
 * Description:     Read in a DDB bitmap resource and return it as a CBitmap.
 *
 ************************************************************************/

CBitmap *FetchResourceBitmap(CDC *pDC, CPalette **pPalette, const int nResID) {
	CDibDoc     *pDIB = NULL;                               // pointer to our loaded DIB file
	HDIB        hDIB = NULL;
	CBitmap     *pBitmap = NULL;
	char        chResName[16];
	CPalette    *pThisPalette = NULL;

	Common::sprintf_s(chResName, "#%d", nResID);

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != NULL) &&                               // verify we have the objects we just created
	        (*pDIB).OpenResourceDocument(chResName)) {      // .... bitmap file
		pThisPalette = (*pDIB).GetDocPalette();
		hDIB = (*pDIB).GetHDIB();
		pBitmap = ConvertDIB(pDC, hDIB, pThisPalette);
		if (pPalette != NULL)
			*pPalette = (*pDIB).DetachPalette();
	}

	if (pDIB != NULL)
		delete pDIB;

	return (pBitmap);
}


/*************************************************************************
 *
 * ExtractBitmap()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used for display
 *  CBitmap *pBase          pointer to source bitmap for extraction
 *  CPalette *pPalette      pointer to color palette to be used for the bitmap
 *  int x, y                upper left hand corner of bitmap to fetch
 *  int dx, dy              size of rectangular area to retrive
 *
 * Return Value:
 *  CBitmap *               pointer to bitmap object or NULL
 *
 * Description:     return a section of another CBitmap object.
 *
 ************************************************************************/

CBitmap *ExtractBitmap(CDC *pDC, CBitmap *pBitmap, CPalette *pPalette, const int x, const int y, const int dx, const int dy) {
	BOOL        bSuccess = FALSE;
	CDC         *pWorkDC = NULL,
	             *pBaseDC = NULL;
	CPalette    *pPalOld = NULL,
	             *pPalOldWork = NULL,
	              *pPalOldBase = NULL;
	CBitmap     *pWork = NULL,
	             *pWorkOld = NULL,
	              *pBaseOld = NULL;

	if (pPalette != NULL) {                                 // map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pPalette, FALSE);
		(void)(*pDC).RealizePalette();
	}

	pWorkDC = new CDC();                                    // create the context and bitmap objects
	pWork = new CBitmap();
	pBaseDC = new CDC();

	if ((pWorkDC != NULL) &&                                // construct an offscreen bitmap that we
	        (pWork != NULL) &&                                  // ... can use as a work area, and then
	        (pBaseDC != NULL) &&                                // ... use as a return value to the caller
	        (*pWorkDC).CreateCompatibleDC(pDC) &&               // setup a context for the source bitmap
	        (*pBaseDC).CreateCompatibleDC(pDC) &&
	        (*pWork).CreateCompatibleBitmap(pDC, dx, dy)) {     // create a bitmap of the appropriate size
		if (pPalette != NULL) {                             // map the palette into the contexts
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, FALSE);
			(void)(*pWorkDC).RealizePalette();
			pPalOldBase = (*pBaseDC).SelectPalette(pPalette, FALSE);
			(void)(*pBaseDC).RealizePalette();
		}
		pWorkOld = (*pWorkDC).SelectObject(pWork);          // now map in the work area's bitmap
		pBaseOld = (*pBaseDC).SelectObject(pBitmap);        // ... as well as the source bitmap
		if ((pWorkOld != NULL) &&
		        (pBaseOld != NULL))                             // grab the bitmap section if all is well
			bSuccess = (*pWorkDC).BitBlt(0, 0, dx, dy, pBaseDC, x, y, SRCCOPY);
	}

	if (pPalOld != NULL)                                    // relinquish the resources we built
		(void)(*pDC).SelectPalette(pPalOld, FALSE);
	if (pWorkOld != NULL)
		(void)(*pWorkDC).SelectObject(pWorkOld);
	if (pBaseOld != NULL)
		(void)(*pBaseDC).SelectObject(pBaseOld);
	if (pPalOldWork != NULL)
		(void)(*pWorkDC).SelectPalette(pPalOldWork, FALSE);
	if (pPalOldBase != NULL)
		(void)(*pBaseDC).SelectPalette(pPalOldBase, FALSE);
	if (!bSuccess &&                                        // release the bitmap result we built
	        (pWork != NULL)) {                                  // ... if something failed
		(*pWork).DeleteObject();
		delete pWork;
		pWork = NULL;
	}
	if (pWorkDC != NULL) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}
	if (pBaseDC != NULL) {
		(*pBaseDC).DeleteDC();
		delete pBaseDC;
	}

	return (pWork);
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
 *  BOOL            success / failure return
 *
 * Description:     paint a rectangular area of the screen from a CBitmap object.
 *
 ************************************************************************/

BOOL BltBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, CRect *pSrcRect, CRect *pDstRect, DWORD dwMode) {
	BOOL        bSuccess = FALSE;
	CDC         *pWorkDC = NULL;
	CPalette    *pPalOld = NULL,
	             *pPalOldWork = NULL;
	CBitmap     *pWorkOld = NULL;
	HDIB        hDib;

	if (pPalette != NULL) {                                 // map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pPalette, FALSE);
		(void)(*pDC).RealizePalette();
	}

	pWorkDC = new CDC();                                    // create the context and bitmap objects

	if ((pWorkDC != NULL) &&                                // verify we got what we asked for
	        (pBitmap != NULL) &&
	        (*pWorkDC).CreateCompatibleDC(pDC)) {               // create a context for our bitmap
		if (pPalette != NULL) {                             // map the palette into the context
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, FALSE);
			(void)(*pWorkDC).RealizePalette();
		}
		pWorkOld = (*pWorkDC).SelectObject(pBitmap);        // now map in our bitmap
		if (pWorkOld != NULL) {                             // paint back the saved pixels
			if ((((*pSrcRect).right - (*pSrcRect).left) != ((*pDstRect).right - (*pDstRect).left)) ||
			        (((*pSrcRect).bottom - (*pSrcRect).top) != ((*pDstRect).bottom - (*pDstRect).top))) {
				(*pDC).SetStretchBltMode(STRETCH_DELETESCANS);
				if (GetDeviceCaps((*pDC).m_hDC, RASTERCAPS) & RC_STRETCHBLT)
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
				else {
					hDib = (HDIB) BitmapToDIB((HBITMAP)(*pBitmap).m_hObject, (HPALETTE)(*pPalette).m_hObject);
					if (hDib != NULL) {
						bSuccess = PaintDIB((*pDC).m_hDC, pDstRect, (HDIB) hDib, pSrcRect, pPalette);
						GlobalFree(hDib);
					} else
						bSuccess = FALSE;
				}
			} else
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

	if (pPalOld != NULL)                                    // relinquish the resources we built
		(void)(*pDC).SelectPalette(pPalOld, FALSE);
	if (pWorkOld != NULL)
		(void)(*pWorkDC).SelectObject(pWorkOld);
	if (pPalOldWork != NULL)
		(void)(*pWorkDC).SelectPalette(pPalOldWork, FALSE);
	if (pWorkDC != NULL) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}

	return (bSuccess);
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
 *  BOOL                    success/failure condition
 *
 * Description:     "mask" paint a bitmap to the specified display context.  White
 *                  pixels are assumed to be transparent, allowing the background
 *                  artwork to show through.
 *
 ************************************************************************/

BOOL BltMaskedBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, CRect *pSrcRect, const int x, const int y) {
	CDC         *pImageDC = NULL,                       // context for the bitmap image
	             *pWorkDC = NULL,                        // context for the bitmap work area
	              *pMaskDC = NULL;                        // context for the image's mask
	CBitmap     *pImage = NULL,                         // bitmap that has our image
	             *pImageOld = NULL,                      // bitmap previously mapped to image context
	              *pWork = NULL,                          // work area bitmap
	               *pWorkOld = NULL,                       // bitmap previously mapped to work context
	                *pMask = NULL,                          // image mask bitmap
	                 *pMaskOld = NULL;                       // bitmap previously mapped to mask context
	CPalette    *pPalOldImage = NULL,                   // palette previously mapped to image context
	             *pPalOldWork = NULL;                    // palette previously mapped to work context
	BOOL        bSuccess = FALSE;
	CPalette    *pPalOld = NULL;
	CSize       cSize;
	int         dx, dy;
	CRect       dstRect;

	if (pPalette != NULL) {
		pPalOld = (*pDC).SelectPalette(pPalette, FALSE);
		(void)(*pDC).RealizePalette();
	}

	pImageDC = new CDC();                               // create the necessary objects
	pMaskDC = new CDC();
	pWorkDC = new CDC();
	pImage = new CBitmap();
	pMask = new CBitmap();
	pWork = new CBitmap();

	if ((pImageDC != NULL) &&                               // verify we got the requested objects
	        (pMaskDC != NULL) &&
	        (pWorkDC != NULL) &&
	        (pImage != NULL) &&
	        (pMask != NULL) &&
	        (pWork != NULL) &&
	        (*pImageDC).CreateCompatibleDC(pDC) &&              // now create all the compatible contexts
	        (*pMaskDC).CreateCompatibleDC(pDC) &&               // ... that we need to hold our bitmaps
	        (*pWorkDC).CreateCompatibleDC(pDC)) {

		dx = (*pSrcRect).right - (*pSrcRect).left;
		dy = (*pSrcRect).bottom - (*pSrcRect).top;

		if (pPalette != NULL) {                     // map in the color palette if specified
			pPalOldImage = (*pImageDC).SelectPalette(pPalette, FALSE);
			(void)(*pImageDC).RealizePalette();
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, FALSE);
			(void)(*pWorkDC).RealizePalette();
		}

		if ((*pImage).CreateCompatibleBitmap(pDC, dx, dy) &&
		        (*pWork).CreateCompatibleBitmap(pDC, dx, dy) &&
		        (*pMask).CreateBitmap(dx, dy, 1, 1, NULL)) {

			pImageOld = (*pImageDC).SelectObject(pImage);
			pMaskOld = (*pMaskDC).SelectObject(pMask);  // map the work and mask bitmaps into
			pWorkOld = (*pWorkDC).SelectObject(pWork);  // ... their contexts

			dstRect.SetRect(0, 0, dx, dy);
			BltBitmap(pImageDC, pPalette, pBitmap, pSrcRect, &dstRect, (DWORD) SRCCOPY);

			if ((pMaskOld != NULL) &&                   // verify so far so good
			        (pWorkOld != NULL)) {
				(void)(*pWorkDC).BitBlt(     // grab what the background looks like
				    0,                              // ... putting it in the work area
				    0,
				    dx,
				    dy,
				    pDC,
				    x,
				    y,
				    SRCCOPY);
				(void)(*pMaskDC).BitBlt(     // create the mask by inverting the image
				    0, 0,                           // ... i.e. the pixels occupied by the image
				    dx,
				    dy,
				    pImageDC,
				    0, 0,
				    NOTSRCCOPY);
				(void)(*pImageDC).BitBlt(     // remove the image's "transparent" white area
				    0, 0,                           // ... i.e. its background becomes black
				    dx,
				    dy,
				    pMaskDC,
				    0, 0,
				    SRCAND);
				(void)(*pMaskDC).BitBlt(     // invert the mask
				    0, 0,
				    dx,
				    dy,
				    pMaskDC,
				    0, 0,
				    DSTINVERT);
				(void)(*pWorkDC).BitBlt(     // remove the image's space from the work area
				    0, 0,                           // ... i.e. the pixels where the image will go
				    dx,
				    dy,
				    pMaskDC,
				    0, 0
				    , SRCAND);
				(void)(*pWorkDC).BitBlt(     // paint the image into the cleared (black) space
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

	if (pPalOld != NULL)
		(void)(*pDC).SelectPalette(pPalOld, FALSE);

	if (pImageOld != NULL)                              // select out bitmaps out of their contexts
		(void)(*pImageDC).SelectObject(pImageOld);
	if (pWorkOld != NULL)
		(void)(*pWorkDC).SelectObject(pWorkOld);
	if (pMaskOld != NULL)
		(void)(*pMaskDC).SelectObject(pMaskOld);

	if (pPalOldImage != NULL)                           // map out the palettes we used
		(void)(*pImageDC).SelectPalette(pPalOldImage, FALSE);
	if (pPalOldWork != NULL)
		(void)(*pWorkDC).SelectPalette(pPalOldWork, FALSE);

	if (pImage != NULL) {                                // delete the bitmaps we created
		(*pImage).DeleteObject();
		delete pImage;
	}
	if (pWork != NULL) {
		(*pWork).DeleteObject();
		delete pWork;
	}
	if (pMask != NULL) {
		(*pMask).DeleteObject();
		delete pMask;
	}

	if (pImageDC != NULL) {                             // delete the contexts we created
		(*pImageDC).DeleteDC();
		delete pImageDC;
	}
	if (pWorkDC != NULL) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}
	if (pMaskDC != NULL) {
		(*pMaskDC).DeleteDC();
		delete pMaskDC;
	}

	return (bSuccess);                                  // return success or failure
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
 *  BOOL            success / failure return
 *
 * Description:     paint a rectangular area of the screen from a CBitmap object.
 *
 ************************************************************************/

BOOL PaintBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, const int x, const int y, const int dx, const int dy) {
	BOOL        bSuccess = FALSE;
	CDC         *pWorkDC = NULL;
	CPalette    *pPalOld = NULL,
	             *pPalOldWork = NULL;
	CBitmap     *pWorkOld = NULL;
	BITMAP      cBitmapData;
	HDIB        hDib;
	CRect       SrcRect, DstRect;

	(*pBitmap).GetObject(sizeof(BITMAP), &cBitmapData);

	if (pPalette != NULL) {                                 // map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pPalette, FALSE);
		(void)(*pDC).RealizePalette();
	}

	pWorkDC = new CDC();                                    // create the context and bitmap objects

	if ((pWorkDC != NULL) &&                                // verify we got what we asked for
	        (pBitmap != NULL) &&
	        (*pWorkDC).CreateCompatibleDC(pDC)) {               // create a context for our bitmap
		if (pPalette != NULL) {                             // map the palette into the context
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, FALSE);
			(void)(*pWorkDC).RealizePalette();
		}
		pWorkOld = (*pWorkDC).SelectObject(pBitmap);        // now map in our bitmap
		if (pWorkOld != NULL) {                             // paint back the saved pixels
			if ((dx != 0) && (dy != 0) &&
			        ((dx != cBitmapData.bmWidth) || (dy != cBitmapData.bmHeight))) {
				(*pDC).SetStretchBltMode(STRETCH_DELETESCANS);
				if (GetDeviceCaps((*pDC).m_hDC, RASTERCAPS) & RC_STRETCHBLT)
					bSuccess = (*pDC).StretchBlt(x, y, dx, dy, pWorkDC, 0, 0, cBitmapData.bmWidth, cBitmapData.bmHeight, SRCCOPY);
				else {
					hDib = (HDIB) BitmapToDIB((HBITMAP)(*pBitmap).m_hObject, (HPALETTE)(*pPalette).m_hObject);
					if (hDib != NULL) {
						DstRect.SetRect(x, y, x + dx, y + dy);
						SrcRect.SetRect(0, 0, cBitmapData.bmWidth, cBitmapData.bmHeight);
						bSuccess = PaintDIB((*pDC).m_hDC, &DstRect, (HDIB) hDib, &SrcRect, pPalette);
						GlobalFree(hDib);
					} else
						bSuccess = FALSE;
				}
			} else
				bSuccess = (*pDC).BitBlt(x, y, cBitmapData.bmWidth, cBitmapData.bmHeight, pWorkDC, 0, 0, SRCCOPY);
		}
	}

	if (pPalOld != NULL)                                    // relinquish the resources we built
		(void)(*pDC).SelectPalette(pPalOld, FALSE);
	if (pWorkOld != NULL)
		(void)(*pWorkDC).SelectObject(pWorkOld);
	if (pPalOldWork != NULL)
		(void)(*pWorkDC).SelectPalette(pPalOldWork, FALSE);
	if (pWorkDC != NULL) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}

	return (bSuccess);
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
 *  BOOL            success / failure return
 *
 * Description:     paint a rectangular area of the screen from a disk based DIB.
 *
 ************************************************************************/

BOOL PaintBitmap(CDC *pDC, CPalette *pPalette, const char *pszPathName, const int x, const int y, const int dx, const int dy) {
	CDibDoc myDoc;
	HDIB    hDIB;
	LPSTR   lpDIB;
	CRect   rcDest;
	CRect   rcDIB;
	int     cxDIB;
	int     cyDIB;
	BOOL    bSuccess;

	bSuccess = myDoc.OpenDocument(pszPathName);
	if ((pDC != NULL) && bSuccess) {
		hDIB = myDoc.GetHDIB();
		if (hDIB == NULL)
			bSuccess = FALSE;
		else {
			lpDIB = (LPSTR) GlobalLock((HGLOBAL) hDIB);
			if (lpDIB == NULL)
				bSuccess = FALSE;
			else {
				cxDIB = (int) DIBWidth(lpDIB);
				cyDIB = (int) DIBHeight(lpDIB);
				GlobalUnlock((HGLOBAL) hDIB);
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
	}

	return (bSuccess);
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
 *  BOOL                    success/failure condition
 *
 * Description:     Read in a DDB bitmap from disk and "mask" paint it to the
 *                  specified display context.  White pixels are assumed to be
 *                  transparent, allowing the background artwork to show through.
 *
 ************************************************************************/

BOOL PaintMaskedDIB(CDC *pDC, CPalette *pPalette, const char *pszPathName, const int x, const int y, const int dx, const int dy) {
	BOOL    bSuccess = FALSE;
	CDibDoc *pDIB = NULL;                                   // pointer to our loaded DIB file

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != NULL) &&                               // verify we have the objects we just created
	        (*pDIB).OpenDocument(pszPathName))              // .... bitmap file
		bSuccess = PaintMaskedDIB(pDC, pPalette, pDIB, x, y, dx, dy) ;

	if (pDIB != NULL)
		delete pDIB;

	return (bSuccess);                                  // return success or failure
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
 *  BOOL                    success/failure condition
 *
 * Description:     Read in a DDB bitmap resource and "mask" paint it to the
 *                  specified display context.  White pixels are assumed to be
 *                  transparent, allowing the background artwork to show through.
 *
 ************************************************************************/

BOOL PaintMaskedResource(CDC *pDC, CPalette *pPalette, const int nResID, const int x, const int y, const int dx, const int dy) {
	BOOL    bSuccess = FALSE;
	CDibDoc *pDIB = NULL;                                   // pointer to our loaded DIB file
	char    chResName[16];

	Common::sprintf_s(chResName, "#%d", nResID);

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != NULL) &&                               // verify we have the objects we just created
	        (*pDIB).OpenResourceDocument(chResName))        // .... bitmap file
		bSuccess = PaintMaskedDIB(pDC, pPalette, pDIB, x, y, dx, dy) ;

	if (pDIB != NULL)
		delete pDIB;

	return (bSuccess);                                  // return success or failure
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
 *  BOOL                    success/failure condition
 *
 * Description:     Read in a DDB bitmap resource and "mask" paint it to the
 *                  specified display context.  White pixels are assumed to be
 *                  transparent, allowing the background artwork to show through.
 *
 ************************************************************************/

BOOL PaintMaskedResource(CDC *pDC, CPalette *pPalette, const char *pszName, const int x, const int y, const int dx, const int dy) {
	BOOL    bSuccess = FALSE;
	CDibDoc *pDIB = NULL;                                   // pointer to our loaded DIB file

	pDIB = new CDibDoc();                               // create an object for our DIB

	if ((pDIB != NULL) &&                               // verify we have the objects we just created
	        (*pDIB).OpenResourceDocument(pszName))          // .... bitmap file
		bSuccess = PaintMaskedDIB(pDC, pPalette, pDIB, x, y, dx, dy) ;

	if (pDIB != NULL)
		delete pDIB;

	return (bSuccess);                                  // return success or failure
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
 *  BOOL                    success/failure condition
 *
 * Description:     "mask" paint a DIB to the specified display context.  White
 *                  pixels are assumed to be transparent, allowing the background
 *                  artwork to show through.
 *
 ************************************************************************/

BOOL PaintMaskedDIB(CDC *pDC, CPalette *pPalette, CDibDoc *pDIB, const int x, const int y, const int dx, const int dy) {
	BOOL        bSuccess = FALSE;
	CBitmap     *pBitmap = NULL;
	CPalette    *pMyPalette = NULL;
	HDIB        hDIB = NULL;

	if (pPalette == NULL)
		pMyPalette = (*pDIB).GetDocPalette();
	else
		pMyPalette = pPalette;

	hDIB = (*pDIB).GetHDIB();
	pBitmap = ConvertDIB(pDC, hDIB, pMyPalette);

	if (pBitmap != NULL) {
		bSuccess = PaintMaskedBitmap(pDC, pMyPalette, pBitmap, x, y, dx, dy);
		(*pBitmap).DeleteObject();
		delete pBitmap;
	}

	return (bSuccess);
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
 *  BOOL                    success/failure condition
 *
 * Description:     "mask" paint a bitmap to the specified display context.  White
 *                  pixels are assumed to be transparent, allowing the background
 *                  artwork to show through.
 *
 ************************************************************************/

BOOL PaintMaskedBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, const int x, const int y, const int dx, const int dy) {
	BITMAP      myBitmap;                               // area for bitmap info
	CDC         *pImageDC = NULL,                       // context for the bitmap image
	             *pWorkDC = NULL,                        // context for the bitmap work area
	              *pMaskDC = NULL;                        // context for the image's mask
	CBitmap     *pImage = NULL,                         // bitmap that has our image
	             *pImageOld = NULL,                      // bitmap previously mapped to image context
	              *pWork = NULL,                          // work area bitmap
	               *pWorkOld = NULL,                       // bitmap previously mapped to work context
	                *pMask = NULL,                          // image mask bitmap
	                 *pMaskOld = NULL;                       // bitmap previously mapped to mask context
	CPalette    *pPalOldImage = NULL,                   // palette previously mapped to image context
	             *pPalOldWork = NULL;                    // palette previously mapped to work context
	BOOL        bSuccess = FALSE;
	CPalette    *pPalOld = NULL;
	CSize       cSize;

	if (pPalette != NULL) {
		pPalOld = (*pDC).SelectPalette(pPalette, FALSE);
		(void)(*pDC).RealizePalette();
	}

	pImageDC = new CDC();                               // create the necessary objects
	pMaskDC = new CDC();
	pWorkDC = new CDC();
	pImage = new CBitmap();
	pMask = new CBitmap();
	pWork = new CBitmap();

	if ((pImageDC != NULL) &&                               // verify we got the requested objects
	        (pMaskDC != NULL) &&
	        (pWorkDC != NULL) &&
	        (pImage != NULL) &&
	        (pMask != NULL) &&
	        (pWork != NULL) &&
	        (*pImageDC).CreateCompatibleDC(pDC) &&              // now create all the compatible contexts
	        (*pMaskDC).CreateCompatibleDC(pDC) &&               // ... that we need to hold our bitmaps
	        (*pWorkDC).CreateCompatibleDC(pDC)) {

		(void)(*pBitmap).GetObject(sizeof(BITMAP), &myBitmap);  // get the image's sizing info
		cSize.cx = myBitmap.bmWidth;
		cSize.cy = myBitmap.bmHeight;

		if ((dx != 0) && (dy != 0) &&
		        ((dx != cSize.cx) || (dy != cSize.cy))) {
			cSize.cx = dx;
			cSize.cy = dy;
		}

		if (pPalette != NULL) {                     // map in the color palette if specified
			pPalOldImage = (*pImageDC).SelectPalette(pPalette, FALSE);
			(void)(*pImageDC).RealizePalette();
			pPalOldWork = (*pWorkDC).SelectPalette(pPalette, FALSE);
			(void)(*pWorkDC).RealizePalette();
		}

		if ((*pImage).CreateCompatibleBitmap(pDC, cSize.cx, cSize.cy) &&
		        (*pWork).CreateCompatibleBitmap(pDC, cSize.cx, cSize.cy) &&
		        (*pMask).CreateBitmap(cSize.cx, cSize.cy, 1, 1, NULL)) {

			pImageOld = (*pImageDC).SelectObject(pImage);
			pMaskOld = (*pMaskDC).SelectObject(pMask);  // map the work and mask bitmaps into
			pWorkOld = (*pWorkDC).SelectObject(pWork);  // ... their contexts

			PaintBitmap(pImageDC, pPalette, pBitmap, 0, 0, dx, dy);

			if ((pMaskOld != NULL) &&                   // verify so far so good
			        (pWorkOld != NULL)) {
				(void)(*pWorkDC).BitBlt(     // grab what the background looks like
				    0,                              // ... putting it in the work area
				    0,
				    cSize.cx,
				    cSize.cy,
				    pDC,
				    x,
				    y,
				    SRCCOPY);
				(void)(*pMaskDC).BitBlt(     // create the mask by inverting the image
				    0, 0,                           // ... i.e. the pixels occupied by the image
				    cSize.cx,
				    cSize.cy,
				    pImageDC,
				    0, 0,
				    NOTSRCCOPY);
				(void)(*pImageDC).BitBlt(     // remove the image's "transparent" white area
				    0, 0,                           // ... i.e. its background becomes black
				    cSize.cx,
				    cSize.cy,
				    pMaskDC,
				    0, 0,
				    SRCAND);
				(void)(*pMaskDC).BitBlt(     // invert the mask
				    0, 0,
				    cSize.cx,
				    cSize.cy,
				    pMaskDC,
				    0, 0,
				    DSTINVERT);
				(void)(*pWorkDC).BitBlt(     // remove the image's space from the work area
				    0, 0,                           // ... i.e. the pixels where the image will go
				    cSize.cx,
				    cSize.cy,
				    pMaskDC,
				    0, 0
				    , SRCAND);
				(void)(*pWorkDC).BitBlt(     // paint the image into the cleared (black) space
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

	if (pPalOld != NULL)
		(void)(*pDC).SelectPalette(pPalOld, FALSE);

	if (pImageOld != NULL)                              // select out bitmaps out of their contexts
		(void)(*pImageDC).SelectObject(pImageOld);
	if (pWorkOld != NULL)
		(void)(*pWorkDC).SelectObject(pWorkOld);
	if (pMaskOld != NULL)
		(void)(*pMaskDC).SelectObject(pMaskOld);

	if (pPalOldImage != NULL)                           // map out the palettes we used
		(void)(*pImageDC).SelectPalette(pPalOldImage, FALSE);
	if (pPalOldWork != NULL)
		(void)(*pWorkDC).SelectPalette(pPalOldWork, FALSE);

	if (pImage != NULL) {                                // delete the bitmaps we created
		(*pImage).DeleteObject();
		delete pImage;
	}
	if (pWork != NULL) {
		(*pWork).DeleteObject();
		delete pWork;
	}
	if (pMask != NULL) {
		(*pMask).DeleteObject();
		delete pMask;
	}

	if (pImageDC != NULL) {                             // delete the contexts we created
		(*pImageDC).DeleteDC();
		delete pImageDC;
	}
	if (pWorkDC != NULL) {
		(*pWorkDC).DeleteDC();
		delete pWorkDC;
	}
	if (pMaskDC != NULL) {
		(*pMaskDC).DeleteDC();
		delete pMaskDC;
	}

	return (bSuccess);                                  // return success or failure
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
 *  BOOL                    success/failure condition
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


BOOL PaintBlockEffect(CDC *pDC, CDibDoc *pDIB, CPalette* pPalette, int nBlockSize) {
	HDIB    hDIB = NULL;
	CBitmap *pBitmap = NULL;
	BOOL    bSuccess = FALSE;

	if ((pDC == NULL) ||
	        (pDIB == NULL))
		return (FALSE);

	hDIB = (*pDIB).GetHDIB();
	pBitmap = ConvertDIB(pDC, hDIB, pPalette);

	bSuccess = PaintBlockEffect(pDC, pBitmap, pPalette, nBlockSize);

	if (pBitmap != NULL)
		delete pBitmap;

	return (bSuccess);
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
 *  BOOL                    success/failure condition
 *
 * Description:     randomly paint a bitmap to the specified display context
 *                  a section/block at a time, without repeating blocks.
 *
 ************************************************************************/

BOOL PaintBlockEffect(CDC *pDC, CBitmap *pBitmap, CPalette* pPalette, int nBlockSize) {
	CPalette *pOldPal = NULL;
	CPalette *pOldPal2 = NULL;
	CBitmap  *pBitmapOld = NULL;
	CDC      *pMemDC = NULL;
	BITMAP   myBitmap;
	BOOL     bSuccess = FALSE;
	unsigned long   seed;
	unsigned long   value, maxvalue;
	unsigned long   x, y, i;
	unsigned long   fill,
	         mask = (BIT16 | BIT15 | BIT2 | BIT0);
	unsigned long   tmp, cnt, shft;
	unsigned long   width, height;

	if ((pDC == NULL) ||
	        (pBitmap == NULL))
		return (FALSE);

	if (pPalette != NULL) {
		pOldPal = (*pDC).SelectPalette(pPalette, FALSE);
		(*pDC).RealizePalette();
	}

	pMemDC = new CDC();
	if ((pMemDC == NULL) ||
	        ((*pMemDC).CreateCompatibleDC(pDC) == FALSE))
		goto clean_up;

	pOldPal2 = (*pMemDC).SelectPalette(pPalette, FALSE);
	(*pMemDC).RealizePalette();
	pBitmapOld = (*pMemDC).SelectObject(pBitmap);

	(void)(*pBitmap).GetObject(sizeof(BITMAP), &myBitmap);
	width = myBitmap.bmWidth / nBlockSize;
	height = myBitmap.bmHeight;

	maxvalue = height / nBlockSize * width;
	seed = SEQ - 1;

	for (i = 0, value = seed; ; i++) {
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
		(*pDC).BitBlt((WORD) x,
		              (WORD) y,
		              nBlockSize,
		              nBlockSize,
		              pMemDC,
		              (WORD) x,
		              (WORD) y,
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

	bSuccess = TRUE;

clean_up:
	if (pBitmapOld != NULL)
		(*pMemDC).SelectObject(pBitmapOld);
	if (pOldPal2 != NULL)
		(void)(*pMemDC).SelectPalette(pOldPal2, FALSE);
	if (pMemDC != NULL)
		delete pMemDC;

	if (pOldPal != NULL)
		(void)(*pDC).SelectPalette(pOldPal, FALSE);

	return (bSuccess);
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
 *  BOOL                    success/failure condition
 *
 * Description:     randomly paint a solid background color to the specified
 *                  display context a section/block at a time, without repeating blocks.
 *
 ************************************************************************/

BOOL PaintBlockEffect(CDC *pDC, COLORREF rgbColor, CPalette* pPalette, int nBlockSize, int nX, int nY, int nWidth, int nHeight) {
	CBrush   myBrush;
	CRect    thisRect, fillRect;
	CPalette *pOldPal = NULL;
	BOOL     bSuccess = FALSE;
	unsigned long   seed;
	unsigned long   value, maxvalue;
	unsigned long   x, y, i;
	unsigned long   fill,
	         mask = (BIT16 | BIT15 | BIT2 | BIT0);
	unsigned long   tmp, cnt, shft;
	unsigned long   width, height;

	if (pDC == NULL)
		return (FALSE);

	if (pPalette != NULL) {
		pOldPal = (*pDC).SelectPalette(pPalette, FALSE);
		(*pDC).RealizePalette();
	}

	myBrush.CreateSolidBrush(rgbColor);

	width = nWidth / nBlockSize;
	height = nHeight;

	maxvalue = height / nBlockSize * width;
	seed = SEQ - 1;

	for (i = 0, value = seed; ; i++) {
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

	bSuccess = TRUE;

	if (pOldPal != NULL)
		(void)(*pDC).SelectPalette(pOldPal, FALSE);

	return (bSuccess);
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
	return (mySize);
}

} // namespace HodjNPodj
} // namespace Bagel
