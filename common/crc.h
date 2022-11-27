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
 * Description: Fast implementation of the CRC standards.
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
class CRCNormal {
public:
	CRCNormal(T poly, T init_remainder, T final_xor);

	T crcFast(byte const message[], int nBytes) const;
	T processByte(byte byteVal, T remainder) const;
	T getInitRemainder() const { return _init_remainder; }
	T finalize(T remainder) const { return remainder ^ _final_xor; }

private:
	const T _init_remainder;
	const T _final_xor;

  	T _crcTable[256];
};

template <typename T>
class CRCReflected {
public:
	CRCReflected(T poly, T init_remainder, T final_xor);

	T crcFast(byte const message[], int nBytes) const;
	T processByte(byte byteVal, T remainder) const;
	T getInitRemainder() const { return _reflected_init_remainder; }
	T finalize(T remainder) const { return remainder ^ _final_xor; }

private:
  	T _crcTable[256];
	const T _reflected_init_remainder;
	const T _final_xor;
};


template <typename T>
CRCNormal<T>::CRCNormal(T poly, T init_remainder, T final_xor) : _init_remainder(init_remainder), _final_xor(final_xor) {
	const T topbit = 1ULL << (8 * sizeof(T) - 1);

	/*
	 * Compute the remainder of each possible dividend.
	 */
	for (int dividend = 0; dividend < 256; ++dividend) {
		/*
		 * Start with the dividend followed by zeros.
		 */
		T remainder = dividend << (8 * sizeof(T) - 8);

		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (byte bit = 8; bit > 0; --bit) {
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & topbit) {
				remainder = (remainder << 1) ^ poly;
			} else {
				remainder = (remainder << 1);
			}
		}

		/*
		 * Store the result into the table.
		 */
		_crcTable[dividend] = remainder;
	}	
}

template <typename T>
CRCReflected<T>::CRCReflected(T reflected_poly, T reflected_init_remainder, T final_xor) : _reflected_init_remainder(reflected_init_remainder), _final_xor(final_xor) {
	/*
	 * Compute the remainder of each possible dividend.
	 */
	for (int dividend = 0; dividend < 256; ++dividend) {
		/*
		 * Start with the dividend followed by zeros.
		 */
		T remainder = dividend;

		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (byte bit = 8; bit > 0; --bit) {
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & 1) {
				remainder = (remainder >> 1) ^ reflected_poly;
			} else {
				remainder = (remainder >> 1);
			}
		}

		/*
		 * Store the result into the table.
		 */
		_crcTable[dividend] = remainder;
	}	
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
T CRCNormal<T>::crcFast(byte const message[], int nBytes) const {
	T remainder = _init_remainder;

	/*
	 * Divide the message by the polynomial, a byte at a time.
	 */
	for (int b = 0; b < nBytes; ++b) {
		byte data = message[b] ^ (remainder >> (8 * sizeof(T) - 8));
		remainder = _crcTable[data] ^ (remainder << 8);
	}

	/*
	 * The final remainder is the CRC.
	 */
	return remainder ^ _final_xor;
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
T CRCReflected<T>::crcFast(byte const message[], int nBytes) const {
	T remainder = _reflected_init_remainder;

	/*
	 * Divide the message by the polynomial, a byte at a time.
	 */
	for (int b = 0; b < nBytes; ++b) {
		byte data = message[b] ^ remainder;
		remainder = _crcTable[data] ^ (remainder >> 8);
	}

	/*
	 * The final remainder is the CRC.
	 */
	return remainder ^ _final_xor;
}

template<typename T>
T CRCNormal<T>::processByte(byte byteVal, T remainder) const {
	byte data = byteVal ^ (remainder >> (8 * sizeof(T) - 8));

	return _crcTable[data] ^ (remainder << 8);
}

template<typename T>
T CRCReflected<T>::processByte(byte byteVal, T remainder) const {
	byte data = byteVal ^ remainder;

	return _crcTable[data] ^ (remainder >> 8);
}

class CRC_CCITT : public CRCNormal<uint16> {
public:
	CRC_CCITT() : CRCNormal<uint16>(0x1021, 0xFFFF, 0x0000) {}
};

class CRC_BINHEX : public CRCNormal<uint16> {
public:
	CRC_BINHEX() : CRCNormal<uint16>(0x1021, 0x0000, 0x0000) {}
};

class CRC16 : public CRCReflected<uint16> {
public:
	CRC16() : CRCReflected<uint16>(0xa001, 0x0000, 0x0000) {}
};

class CRC32 : public CRCReflected<uint32> {
public:
	CRC32() : CRCReflected<uint32>(0xEDB88320, 0xFFFFFFFF, 0xFFFFFFFF) {}
};

} // End of namespace Common

#endif
