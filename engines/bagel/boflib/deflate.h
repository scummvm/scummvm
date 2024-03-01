
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

#ifndef BAGEL_BOFLIB_DEFLATE_H
#define BAGEL_BOFLIB_DEFLATE_H

#include "bagel/boflib/file.h"

namespace Bagel {

/**
 * Huffman Encoding after Compression with 32k Sliding dictionary.
 * Will deflate Input file and store it in zipFile data area
 * @return		Error return code
 *
**/
extern ERROR_CODE Deflate(CBofFile *pDestFile, UBYTE *pSrcBuf, INT nSrcBufSize, USHORT factor, ULONG *pCrc, ULONG *pCompressedSize);

/**
 * Inflate a Deflated file from the zipFile data area
 * @return		Error return code
**/
extern ERROR_CODE Inflate(UBYTE *pDstBuf, INT nDstBufSize, UBYTE *pSrcBuf, INT nSrcBufSize, ULONG lOriginalCrc);

} // namespace Bagel

#endif
