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

#ifndef TWP_BTEA_H
#define TWP_BTEA_H

#include "common/system.h"

namespace Twp {

class BTEACrypto {
public:
  static void encrypt(uint32_t *v, int n, const uint32_t *k);
  static void decrypt(uint32_t *v, int n, const uint32_t *k);

private:
  static void btea(uint32_t *v, int n, const uint32_t *k);
};

} // End of namespace Twp

#endif // TWP_H
