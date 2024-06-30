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

#ifndef QDENGINE_CORE_QDCORE_QD_RND_H
#define QDENGINE_CORE_QDCORE_QD_RND_H


namespace QDEngine {

extern RandomGenerator qd_random_generator;

/// Возвращает случайное значение в интервале [0, m-1].
inline unsigned qd_rnd(unsigned m) {
	return qd_random_generator(m);
}
/// Возвращает случайное значение в интервале [-x, x].
inline float qd_frnd(float x) {
	return qd_random_generator.frnd(x);
}
/// Возвращает случайное значение в интервале [0, x].
inline float qd_fabs_rnd(float x) {
	return qd_random_generator.fabsRnd(x);
}

bool qd_rnd_init(int seed = 83);


} // namespace QDEngine

#endif /* QDENGINE_CORE_QDCORE_QD_RND_H */
