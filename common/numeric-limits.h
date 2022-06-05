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

#ifndef COMMON_NUMERIC_LIMITS_H
#define COMMON_NUMERIC_LIMITS_H

#include "common/scummsys.h"
#include "common/type-traits.h"

namespace Common {

template <typename T, bool TIsIntegral, bool TIsUnsigned>
struct IntegralNumericLimitsImpl {
};

template<typename T>
struct IntegralNumericLimitsImpl<T, true, false> {
	static const T kMaxValue = ((T(1) << (sizeof(T) * 8 - 2)) - 1) * 2 + 1;
	static const T kMinValue = -kMaxValue - 1;
};

template<typename T>
struct IntegralNumericLimitsImpl<T, true, true> {
	static const T kMaxValue = ((T(1) << (sizeof(T) * 8 - 1)) - 1) * 2 + 1;
	static const T kMinValue = 0;
};

template<typename T>
struct IntegralNumericLimits : public IntegralNumericLimitsImpl<T, IsIntegral<T>::value, IsUnsigned<T>::value> {
};

template<>
struct IntegralNumericLimits<bool> {
	static const bool kMaxValue = true;
	static const bool kMinValue = false;
};

template<typename T>
struct NumericLimits {
	static inline T min() { return IntegralNumericLimits<T>::kMinValue; }
	static inline T max() { return IntegralNumericLimits<T>::kMaxValue; }
	static inline T lowest() { return IntegralNumericLimits<T>::kMinValue; }
};

template<>
struct NumericLimits<float> {
	static inline float max() { return 3.402823466e+38f; }			// Maximum normal value
	static inline float min() { return 1.175494351e-38f; }			// Minimum normal positive value
	static inline float denorm_min() { return 1.401298464e-45f; }	// Minimum positive value
	static inline float lowest() { return -max(); }
};

template<>
struct NumericLimits<double> {
	static inline double max() { return 1.7976931348623158e+308; }			// Maximum normal value
	static inline double min() { return 2.2250738585072014e-308; }			// Minimum normal positive value
	static inline double denorm_min() { return 4.9406564584124654e-324; }	// Minimum positive value
	static inline double lowest() { return -max(); }
};

} // namespace Common

#endif
