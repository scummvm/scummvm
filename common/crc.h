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

template <typename T, bool need_reflect>
class CRCCommon {
public:
	CRCCommon(T poly, T init_remainder, T final_xor);
	T finalize(T remainder) const;
	T crcSlow(byte const message[], int nBytes) const;
	T getInitRemainder() const { return reflectRemainder(_init_remainder); }

protected:
	const T _poly;
	const T _init_remainder;
	const T _final_xor;

	const int _width;
	const int _topbit;

	template<typename R>
	static R reflect(R data);

	byte reflectData(byte x) const { return need_reflect ? reflect<byte>(x) : x; }
	T reflectRemainder(T x) const { return need_reflect ? reflect<T>(x) : x; }
};

template <typename T>
class CRCNormal : public CRCCommon<T, false> {
public:
	CRCNormal(T poly, T init_remainder, T final_xor);

	T crcFast(byte const message[], int nBytes) const;
	T processByte(byte byteVal, T remainder) const;

private:
  	T _crcTable[256];
};

template <typename T>
class CRCReflected : public CRCCommon<T, true> {
public:
	CRCReflected(T poly, T init_remainder, T final_xor);

	T crcFast(byte const message[], int nBytes) const;
	T processByte(byte byteVal, T remainder) const;

private:
  	T _crcTable[256];
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
template<typename T, bool need_reflect> template<typename R>
R CRCCommon<T, need_reflect>::reflect(R data) {
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
template<typename T, bool need_reflect>
T CRCCommon<T, need_reflect>::crcSlow(byte const message[], int nBytes) const {
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


template<typename T, bool need_reflect>
CRCCommon<T, need_reflect>::CRCCommon(T poly, T init_remainder, T final_xor) :
	_poly(poly), _init_remainder(init_remainder), _final_xor(final_xor), _width(8 * sizeof(T)), _topbit(1 << (8 * sizeof(T) - 1)) {}

template <typename T>
CRCNormal<T>::CRCNormal(T poly, T init_remainder, T final_xor) : CRCCommon<T, false>(poly, init_remainder, final_xor) {
	/*
	 * Compute the remainder of each possible dividend.
	 */
	for (int dividend = 0; dividend < 256; ++dividend) {
		/*
		 * Start with the dividend followed by zeros.
		 */
		T remainder = dividend << (this->_width - 8);

		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (byte bit = 8; bit > 0; --bit) {
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & this->_topbit) {
				remainder = (remainder << 1) ^ this->_poly;
			} else {
				remainder = (remainder << 1);
			}
		}

		/*
		 * Store the result into the table.
		 */
		this->_crcTable[dividend] = remainder;
	}	
}

template <typename T>
CRCReflected<T>::CRCReflected(T poly, T init_remainder, T final_xor) : CRCCommon<T, true>(poly, init_remainder, final_xor) {
	T reflected_poly = this->reflectRemainder(poly);

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
	T remainder = this->_init_remainder;

	/*
	 * Divide the message by the polynomial, a byte at a time.
	 */
	for (int b = 0; b < nBytes; ++b) {
		byte data = message[b] ^ (remainder >> (this->_width - 8));
		remainder = this->_crcTable[data] ^ (remainder << 8);
	}

	/*
	 * The final remainder is the CRC.
	 */
	return remainder ^ this->_final_xor;
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
	T remainder = this->reflectRemainder(this->_init_remainder);

	/*
	 * Divide the message by the polynomial, a byte at a time.
	 */
	for (int b = 0; b < nBytes; ++b) {
		byte data = message[b] ^ remainder;
		remainder = this->_crcTable[data] ^ (remainder >> 8);
	}

	/*
	 * The final remainder is the CRC.
	 */
	return remainder ^ this->_final_xor;
}

template<typename T>
T CRCNormal<T>::processByte(byte byteVal, T remainder) const {
	byte data = byteVal ^ (remainder >> (this->_width - 8));

	return this->_crcTable[data] ^ (remainder << 8);
}

template<typename T>
T CRCReflected<T>::processByte(byte byteVal, T remainder) const {
	byte data = byteVal ^ remainder;

	return this->_crcTable[data] ^ (remainder >> 8);
}

template<typename T, bool need_reflect>
T CRCCommon<T, need_reflect>::finalize(T remainder) const {
	return remainder ^ _final_xor;
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
	CRC16() : CRCReflected<uint16>(0x8005, 0x0000, 0x0000) {}
};

class CRC32 : public CRCReflected<uint32> {
public:
	CRC32() : CRCReflected<uint32>(0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF) {}
};

} // End of namespace Common

#endif
