/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_STRING_BIDI_H
#define COMMON_STRING_BIDI_H

#include "common/str.h"
#include "common/ustr.h"
#include "common/str-enc.h"

namespace Common {

/*
 * Wrapper for GNU FriBidi implementation of the Unicode Bidirectional Algorithm
 * For LTR (Left To Right) languages, returns the original input
 * For RTL (Right To Left) languages, returns visual representation of a logical single-line input
 */
U32String convertBiDiU32String(const U32String &input);
String convertBiDiString(const String &input, const Common::CodePage page);

} // End of namespace Common

#endif
