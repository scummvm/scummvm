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

#ifndef QDENGINE_QDCORE_QD_RND_H
#define QDENGINE_QDCORE_QD_RND_H

#include "qdengine/qdengine.h"

namespace QDEngine {

/// Возвращает случайное значение в интервале [0, m-1].
inline uint32 qd_rnd(uint32 m) {
	if (!m)
		return 0;

	return g_engine->getRandomNumber(m - 1);
}
/// Возвращает случайное значение в интервале [-x, x].
inline float qd_frnd(float x) {
	return (float)(qd_rnd(0x7fff) * 2 - 0x7fff) * x / (float)0x7fff;
}
/// Возвращает случайное значение в интервале [0, x].
inline float qd_fabs_rnd(float x) {
	return (float)qd_rnd(0x7fff) * x / (float)0x7fff;
}

inline void qd_rnd_init(int seed = 83) {
	g_engine->setSeed(seed);
}


} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_RND_H
