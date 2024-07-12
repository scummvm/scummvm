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

#pragma once
#include "common/textconsole.h"


namespace QDEngine {

//////////////////////////////////////////////////////////////////////////////
// FAST INVERSE SQUARE ROOT
// By Chris Lomont
// http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
//
// Максимальная погрешность 0.0017758, приблизительно в 6 раз быстрее 1/sqrtf()
//////////////////////////////////////////////////////////////////////////////
inline float invSqrtFast(float x) {
	x += 1e-7f; // Добавка, устраняющая деление на 0
	float xhalf = 0.5f * x;
	int i = *(int *)&x; // get bits for floating value
	i = 0x5f375a86 - (i >> 1); // gives initial guess y0
	x = *(float *)&i; // convert bits back to float
	x = x * (1.5f - xhalf * x * x); // Newton step, repeating increases accuracy
	return x;
}

// В 3 раза быстрее за счет проверки аргументов, точная.

inline unsigned int F2DW(float f) {
	return *((unsigned int *)&f);
}

inline float DW2F(unsigned int f) {
	return *((float *)&f);
}
} // namespace QDEngine

