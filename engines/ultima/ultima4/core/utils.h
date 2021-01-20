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

#ifndef ULTIMA4_CORE_UTILS_H
#define ULTIMA4_CORE_UTILS_H

#include "ultima/ultima4/ultima4.h"
#include "ultima/shared/std/containers.h"
#include "common/savefile.h"
#include "common/hash-str.h"

namespace Ultima {
namespace Ultima4 {

extern void assertMsg(bool exp, const char *desc, ...);

/* The AdjustValue functions used to be #define'd macros, but these are
 * evil for several reasons, *especially* when they contain multiple
 * statements, and have if statements in them. The macros did both.
 * See http://www.parashift.com/c++-faq-lite/inline-functions.html#faq-9.5
 * for more information.
 */
inline void AdjustValueMax(int &v, int val, int max) {
	v += val;
	if (v > max) v = max;
}
inline void AdjustValueMin(int &v, int val, int min) {
	v += val;
	if (v < min) v = min;
}
inline void AdjustValue(int &v, int val, int max, int min) {
	v += val;
	if (v > max) v = max;
	if (v < min) v = min;
}

inline void AdjustValueMax(short &v, int val, int max) {
	v += val;
	if (v > max) v = max;
}
inline void AdjustValueMin(short &v, int val, int min) {
	v += val;
	if (v < min) v = min;
}
inline void AdjustValue(short &v, int val, int max, int min) {
	v += val;
	if (v > max) v = max;
	if (v < min) v = min;
}

inline void AdjustValueMax(unsigned short &v, int val, int max) {
	v += val;
	if (v > max) v = max;
}
inline void AdjustValueMin(unsigned short &v, int val, int min) {
	v += val;
	if (v < min) v = min;
}
inline void AdjustValue(unsigned short &v, int val, int max, int min) {
	v += val;
	if (v > max) v = max;
	if (v < min) v = min;
}

/**
 * Seed the random number generator.
 */
void xu4_srandom();

/**
 * Generate a random number between 0 and (upperRange - 1)
 */
int xu4_random(int upperval);

/**
 * Trims whitespace from a Common::String
 * @param val The Common::String you are trimming
 * @param chars_to_trim A list of characters that will be trimmed
 */
Common::String &trim(Common::String &val, const Common::String &chars_to_trim = "\t\013\014 \n\r");

/**
 * Converts the Common::String to lowercase
 */
Common::String &lowercase(Common::String &val);

/**
 * Converts the Common::String to uppercase
 */
Common::String &uppercase(Common::String &val);

/**
 * Converts an integer value to a Common::String
 */
Common::String xu4_to_string(int val);

/**
 * Splits a Common::String into substrings, divided by the charactars in
 * separators.  Multiple adjacent seperators are treated as one.
 */
Std::vector<Common::String> split(const Common::String &s, const Common::String &separators);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
