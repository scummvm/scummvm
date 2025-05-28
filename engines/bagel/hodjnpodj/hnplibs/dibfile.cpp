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

namespace Bagel {
namespace HodjNPodj {

/*
 * Dib Header Marker - used in writing DIBs to files
 */
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')


/*************************************************************************
 *
 * SaveDIB()
 *
 * Saves the specified DIB into the specified CFile.  The CFile
 * is opened and closed by the caller.
 *
 * Parameters:
 *
 * HDIB hDib - Handle to the dib to save
 *
 * CFile& file - open CFile used to save DIB
 *
 * Return value: TRUE if successful, else FALSE or CFileException
 *
 *************************************************************************/

BOOL WINAPI SaveDIB(HDIB hDib, CFile &file) {
	#ifdef TODO
	BITMAPFILEHEADER bmfHdr; // Header for Bitmap file
	LPBITMAPINFOHEADER lpBI;   // Pointer to DIB info structure
	DWORD dwDIBSize;

	if (hDib == nullptr)
		return FALSE;

	/*
	 * Get a pointer to the DIB memory, the first of which contains
	 * a BITMAPINFO structure
	 */
	lpBI = (LPBITMAPINFOHEADER) GlobalLock((HGLOBAL)hDib);
	if (lpBI == nullptr)
		return FALSE;

	if (!IS_WIN30_DIB(lpBI)) {
		GlobalUnlock((HGLOBAL)hDib);
		return FALSE;       // It's an other-style DIB (save not supported)
	}

	/*
	 * Fill in the fields of the file header
	 */

	/* Fill in file type (first 2 bytes must be "BM" for a bitmap) */
	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"

	// Calculating the size of the DIB is a bit tricky (if we want to
	// do it right).  The easiest way to do this is to call GlobalSize()
	// on our global handle, but since the size of our global memory may have
	// been padded a few bytes, we may end up writing out a few too
	// many bytes to the file (which may cause problems with some apps).
	//
	// So, instead let's calculate the size manually (if we can)
	//
	// First, find size of header plus size of color table.  Since the
	// first DWORD in both BITMAPINFOHEADER and BITMAPCOREHEADER conains
	// the size of the structure, let's use this.

	dwDIBSize = *(LPDWORD)lpBI + ::PaletteSize((LPSTR)lpBI);  // Partial Calculation

	// Now calculate the size of the image

	if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4)) {
		// It's an RLE bitmap, we can't calculate size, so trust the
		// biSizeImage field

		dwDIBSize += lpBI->biSizeImage;
	} else {
		DWORD dwBmBitsSize;  // Size of Bitmap Bits only

		// It's not RLE, so size is Width (DWORD aligned) * Height

		dwBmBitsSize = WIDTHBYTES((lpBI->biWidth) * ((DWORD)lpBI->biBitCount)) * lpBI->biHeight;

		dwDIBSize += dwBmBitsSize;

		// Now, since we have calculated the correct size, why don't we
		// fill in the biSizeImage field (this will fix any .BMP files which
		// have this field incorrect).

		lpBI->biSizeImage = dwBmBitsSize;
	}


	// Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)

	bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;

	/*
	 * Now, calculate the offset the actual bitmap bits will be in
	 * the file -- It's the Bitmap file header plus the DIB header,
	 * plus the size of the color table.
	 */
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize
	                   + PaletteSize((LPSTR)lpBI);

	TRY {
		// Write the file header
		file.Write((LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER));
		//
		// Write the DIB header and the bits
		//
		file.WriteHuge(lpBI, dwDIBSize);
	}
	CATCH(CFileException, e) {
		GlobalUnlock((HGLOBAL)hDib);
		THROW_LAST();
	}
	END_CATCH

	GlobalUnlock((HGLOBAL)hDib);
	return TRUE;
	#else
	error("TODO: SaveDIB");
	#endif
}


/*************************************************************************

  Function:  ReadDIBFile (CFile&)

   Purpose:  Reads in the specified DIB file into a global chunk of
             memory.

   Returns:  A handle to a dib (hDIB) if successful.
             nullptr if an error occurs.

  Comments:  BITMAPFILEHEADER is stripped off of the DIB.  Everything
             from the end of the BITMAPFILEHEADER structure on is
             returned in the global memory handle.

*************************************************************************/


HDIB WINAPI ReadDIBFile(CFile &file) {
	#ifdef TODO
	BITMAPFILEHEADER bmfHeader;
	DWORD dwBitsSize;
	HDIB hDIB = nullptr;
	LPSTR pDIB;
	BOOL bRetry = FALSE;

	/*
	 * get length of DIB in bytes for use when reading
	 */

try_again:

	dwBitsSize = file.GetLength();

	/*
	 * Go read the DIB file header and check if it's valid.
	 */
	if ((file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) !=
	        sizeof(bmfHeader)) || (bmfHeader.bfType != DIB_HEADER_MARKER)) {
		goto done;
	}
	/*
	 * Allocate memory for DIB
	 */
	hDIB = (HDIB) ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize);
	if (hDIB == nullptr) {
		goto done;
	}
	pDIB = (LPSTR) GlobalLock((HGLOBAL)hDIB);

	/*
	 * Go read the bits.
	 */
	if (file.ReadHuge(pDIB, dwBitsSize - sizeof(BITMAPFILEHEADER)) !=
	        dwBitsSize - sizeof(BITMAPFILEHEADER)) {
		GlobalUnlock((HGLOBAL)hDIB);
		::GlobalFree((HGLOBAL)hDIB);
		hDIB = nullptr;
		goto done;
	}
	GlobalUnlock((HGLOBAL)hDIB);

done:

	if ((hDIB == nullptr) && !bRetry) {
		bRetry = TRUE;
		(void)GlobalCompact(1000000L);
		goto try_again;
	}


	return hDIB;
	#else
	error("TODO: ReadDIBFile");
	#endif
}


/*************************************************************************

  Function:  ReadDIBResource (CFile&)

   Purpose:  Reads in the specified DIB file into a global chunk of
             memory.

   Returns:  A handle to a dib (hDIB) if successful.
             nullptr if an error occurs.

  Comments:  BITMAPFILEHEADER is stripped off of the DIB.  Everything
             from the end of the BITMAPFILEHEADER structure on is
             returned in the global memory handle.

*************************************************************************/


HDIB WINAPI ReadDIBResource(const char *pszName) {
	#ifdef TODO
	HRSRC       hRsc = nullptr;
	HINSTANCE   hInst = nullptr;
	HGLOBAL     hGbl = nullptr;
	char *pData = nullptr;
	size_t      dwBytes;
	HDIB        hDIB = nullptr;
	LPSTR       pDIB = nullptr;
	BOOL        bRetry = FALSE;

	hInst = AfxGetInstanceHandle();

try_again:

	hRsc = FindResource(hInst, pszName, RT_BITMAP);
	if (hRsc != nullptr) {
		dwBytes = (size_t)SizeofResource(hInst, hRsc);
		hGbl = LoadResource(hInst, hRsc);
		if ((dwBytes != 0) &&
		        (hGbl != nullptr)) {
			pData = (char *)LockResource(hGbl);
			hDIB = (HDIB) ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBytes);
			if (hDIB != nullptr) {
				pDIB = (LPSTR) GlobalLock((HGLOBAL)hDIB);
				memcpy(pDIB, pData, dwBytes);
				GlobalUnlock((HGLOBAL)hDIB);
				UnlockResource(hGbl);
				FreeResource(hGbl);
				return (hDIB);
			}
			UnlockResource(hGbl);
		}
	}

	if (hGbl != nullptr)
		FreeResource(hGbl);

	if (!bRetry) {
		bRetry = TRUE;
		(void)GlobalCompact(1000000L);
		goto try_again;
	}

	return (nullptr);
	#else
	error("TODO: ReadDIBResource");
	#endif
}

} // namespace HodjNPodj
} // namespace Bagel
