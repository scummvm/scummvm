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

#ifndef COMMON_BTEA_H
#define COMMON_BTEA_H

#include "common/system.h"

namespace Common {
namespace BTEACrypto {
/**
 * Corrected Block TEA (aka XXTEA) for ScummVM.
 *
 * In cryptography, Corrected Block TEA (often referred to as XXTEA)
 * is a block cipher designed to correct weaknesses in the original Block TEA.
 */

/**
 * Encrypt data with a specified key.
 * @param[in,out] data    the data to encrypt
 * @param[in] n	          the size of the data
 * @param[in] key         the key to use to encrypt the data
 */
void encrypt(uint32 *data, int n, const uint32 *key);

/**
 * Decrypt data encrypted before with btea with a specified key.
 * @param[in,out] data    the data to decrypt
 * @param[in] n	          the size of the data
 * @param[in] key         the key to use to decrypt the data
 */
void decrypt(uint32 *data, int n, const uint32 *key);

} // End of namespace BTEACrypto
} // End of namespace Common

#endif // COMMON_BTEA_H
