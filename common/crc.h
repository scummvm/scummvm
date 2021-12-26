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
 * Description: Slow and fast implementations of the CRC standards.
 *
 * Notes:
 *
 *
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

#ifndef COMMON_CRC_H
#define COMMON_CRC_H

#include "common/system.h" // For types.

namespace Common {

template <typename T>
class CRC {
public:
	CRC(T poly, T init_remainder, T final_xor, bool reflect);
	T init(void);
	T processByte(byte byteVal, T remainder);
	T finalize(T remainder);

	T crcSlow(byte const message[], int nBytes);
	T crcFast(byte const message[], int nBytes);

private:
	T _poly;
	T _init_remainder;
	T _final_xor;
	bool _reflect;

	int _width;
	int _topbit;

	T _crcTable[256];

	bool _inited;

	uint32 reflect(uint32 data, byte nBits);

	byte reflectData(byte x) { return _reflect ? (byte)reflect(x, 8) : x; }
	T reflectRemainder(T x) { return _reflect ? (T)reflect(x, _width) : x; }
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
template<typename T>
uint32 CRC<T>::reflect(uint32 data, byte nBits) {
	uint32 reflection = 0x00000000;

	/*
	 * Reflect the data about the center bit.
	 */
	for (byte bit = 0; bit < nBits; ++bit) {
		/*
		 * If the LSB bit is set, set the reflection of it.
		 */
		if (data & 0x01) {
			reflection |= (1 << ((nBits - 1) - bit));
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
T CRC<T>::crcSlow(byte const message[], int nBytes) {
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
CRC<T>::CRC(T poly, T init_remainder, T final_xor, bool reflect) :
		_poly(poly), _init_remainder(init_remainder), _final_xor(final_xor), _reflect(reflect) {
	_width = 8 * sizeof(T);
	_topbit = 1 << (_width - 1);

	for (int i = 0; i < 256; ++i)
		_crcTable[i] = 0;

	_inited = false;
}

/*********************************************************************
 *
 * Function:    crcInit()
 *
 * Description: Populate the partial CRC lookup table.
 *
 * Notes:       This function must be rerun any time the CRC standard
 *              is changed.  If desired, it can be run "offline" and
 *              the table results stored in an embedded system's ROM.
 *
 * Returns:     Initial remainder.
 *
 *********************************************************************/
template<typename T>
T CRC<T>::init() {
	/*
	 * Compute the remainder of each possible dividend.
	 */
	for (int dividend = 0; dividend < 256; ++dividend) {
		/*
		 * Start with the dividend followed by zeros.
		 */
		T remainder = dividend << (_width - 8);

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

		/*
		 * Store the result into the table.
		 */
		_crcTable[dividend] = remainder;
	}

	_inited = true;

	return _init_remainder;
}


/*********************************************************************
 *
 * Function:    crcFast()
 *
 * Description: Compute the CRC of a given message.
 *
 * Notes:       crcInit() must be called first.
 *
 * Returns:     The CRC of the message.
 *
 *********************************************************************/
template<typename T>
T CRC<T>::crcFast(byte const message[], int nBytes) {
	T remainder = _init_remainder;

	if (!_inited)
		error("CRC::crcFast(): init method must be called first");

	/*
	 * Divide the message by the polynomial, a byte at a time.
	 */
	for (int b = 0; b < nBytes; ++b) {
		byte data = reflectData(message[b] ^ (remainder >> (_width - 8)));
		remainder = _crcTable[data] ^ (remainder << 8);
	}

	/*
	 * The final remainder is the CRC.
	 */
	return reflectRemainder(remainder) ^ _final_xor;

}

template<typename T>
T CRC<T>::processByte(byte byteVal, T remainder) {
	byte data = reflectData(byteVal) ^ (remainder >> (_width - 8));

	remainder = _crcTable[data] ^ (remainder << 8);
	return remainder;
}

template<typename T>
T CRC<T>::finalize(T remainder) {
	return reflectRemainder(remainder) ^ _final_xor;
}

class CRC_CCITT : public CRC<uint16> {
public:
	CRC_CCITT() : CRC<uint16>(0x1021, 0xFFFF, 0x0000, false) {}
};

class CRC_BINHEX : public CRC<uint16> {
public:
	CRC_BINHEX() : CRC<uint16>(0x1021, 0x0000, 0x0000, false) {}
};

class CRC16 : public CRC<uint16> {
public:
	CRC16() : CRC<uint16>(0x8005, 0x0000, 0x0000, true) {}
};

class CRC32 : public CRC<uint32> {
public:
	CRC32() : CRC<uint32>(0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true) {}
};

} // End of namespace Common

#endif
