
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

#ifndef BAGEL_BOFLIB_CRC_H
#define BAGEL_BOFLIB_CRC_H

#include "bagel/boflib/stdinc.h"

namespace Bagel {

/**
 * Continues a CRC calculation with specified byte
 * @param chr       Byte to calc crc for
 * @param lOldCRC   Original crc to continue calculation
 * @return          New CRC value
 */
extern uint32 CalcCrcQuick(UBYTE chr, uint32 crcValue);

/**
 * Calculates the CRC (Cyclic Redundancy Check) for a buffer
 * @param pBuf          Pointer to buffer
 * @param lBufLen       Length of this buffer
 * @param lCrcValue     Previous CRC value (if running CRC)
 * @return              New CRC value
 */
extern uint32 CalculateCRC(const VOID *pBuffer, LONG lBufLen, uint32 lCrcValue = 0);

} // namespace Bagel

#endif
