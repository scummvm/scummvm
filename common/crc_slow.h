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

/**********************************************************************
 *
 * Filename:    crc.c
 *
 * Description: Slow implementation of the CRC standards.
 *
 * Notes:
 *
 *
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

#ifndef COMMON_CRC_SLOW_H
#define COMMON_CRC_SLOW_H

#include "common/system.h" // For types.

namespace Common {

template <typename T>
class CRCSlow {
public:
	CRCSlow(T poly, T init_remainder, T final_xor, bool need_reflect);
	T crcSlow(byte const message[], int nBytes) const;

protected:
	const T _poly;
	const T _init_remainder;
	const T _final_xor;

	const int _width;
	const int _topbit;

	const bool _reflect;

	template<typename R>
	static R reflect(R data);

	byte reflectData(byte x) const { return _reflect ? reflect<byte>(x) : x; }
	T reflectRemainder(T x) const { return _reflect ? reflect<T>(x) : x; }
};

/*********************************************************************
 *
 * Function:    reflect()
 *
 * Description: Reorder the bits of a binary sequence, by reflecting
 *              them about the middle position.
 *
 * Notes:       No checking is done that nBits <= 32.
 *
 * Returns:     The reflection of the original data.
 *
 *********************************************************************/
template<typename T> template<typename R>
R CRCSlow<T>::reflect(R data) {
	R reflection = 0;

	/*
	 * Reflect the data about the center bit.
	 */
	for (byte bit = 0; bit < sizeof(R) * 8; ++bit) {
		/*
		 * If the LSB bit is set, set the reflection of it.
		 */
		if (data & 0x01) {
			reflection |= (1 << ((sizeof(R) * 8 - 1) - bit));
		}

		data = (data >> 1);
	}

	return reflection;
}


/*********************************************************************
 *
 * Function:    crcSlow()
 *
 * Description: Compute the CRC of a given message.
 *
 * Notes:
 *
 * Returns:     The CRC of the message.
 *
 *********************************************************************/
template<typename T>
T CRCSlow<T>::crcSlow(byte const message[], int nBytes) const {
	T remainder = _init_remainder;

	/*
	 * Perform modulo-2 division, a byte at a time.
	 */
	for (int b = 0; b < nBytes; ++b) {
		/*
		 * Bring the next byte into the remainder.
		 */
		remainder ^= reflectData(message[b]) << (_width - 8);

		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (byte bit = 8; bit > 0; --bit) {
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & _topbit) {
				remainder = (remainder << 1) ^ _poly;
			} else {
				remainder = (remainder << 1);
			}
		}
	}

	/*
	 * The final remainder is the CRC result.
	 */
	return reflectRemainder(remainder) ^ _final_xor;
}


template<typename T>
CRCSlow<T>::CRCSlow(T poly, T init_remainder, T final_xor, bool need_reflect) :
	_poly(poly), _init_remainder(init_remainder), _final_xor(final_xor), _width(8 * sizeof(T)), _topbit(1 << (8 * sizeof(T) - 1)), _reflect(need_reflect) {}

class CRC_CCITT_Slow : public CRCSlow<uint16> {
public:
	CRC_CCITT_Slow() : CRCSlow<uint16>(0x1021, 0xFFFF, 0x0000, false) {}
};

class CRC_BINHEX_Slow : public CRCSlow<uint16> {
public:
	CRC_BINHEX_Slow() : CRCSlow<uint16>(0x1021, 0x0000, 0x0000, false) {}
};

class CRC16_Slow : public CRCSlow<uint16> {
public:
	CRC16_Slow() : CRCSlow<uint16>(0x8005, 0x0000, 0x0000, true) {}
};

class CRC32_Slow : public CRCSlow<uint32> {
public:
	CRC32_Slow() : CRCSlow<uint32>(0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true) {}
};

} // End of namespace Common

#endif
