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

#include "ultima8/misc/pent_include.h"
#include "ultima8/graphics/scalers/bilinear_scaler.h"
#include "ultima8/graphics/scalers/bilinear_scaler_internal.h"
#include "ultima8/graphics/manips.h"

namespace Ultima8 {
namespace Pentagram {

template<class uintX, class Manip, class uintS> class BilinearScalerInternal {
public:
	static bool ScaleBilinear(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
	                          uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
		// Must be multiples of 4!!!
		if ((sh & 3) || (sw & 3)) return false;

		// 2x Scaling
		if ((sw * 2 == dw) && (sh * 2 == dh))
			return BilinearScalerInternal_2x<uintX, Manip, uintS>(tex, sx, sy, sw, sh, pixel, dw, dh, pitch, clamp_src);
		// 2 X 2.4 Y
		else if ((sw * 2 == dw) && (dh * 5 == sh * 12))
			return BilinearScalerInternal_X2Y24<uintX, Manip, uintS>(tex, sx, sy, sw, sh, pixel, dw, dh, pitch, clamp_src);
		// 1 X 1.2 Y
		else if ((sw == dw) && (dh * 5 == sh * 6))
			return BilinearScalerInternal_X1Y12<uintX, Manip, uintS>(tex, sx, sy, sw, sh, pixel, dw, dh, pitch, clamp_src);
		// Arbitrary
		else
			return BilinearScalerInternal_Arb<uintX, Manip, uintS>(tex, sx, sy, sw, sh, pixel, dw, dh, pitch, clamp_src);
	}
};


BilinearScaler::BilinearScaler() : Scaler() {
#ifdef COMPILE_ALL_BILINEAR_SCALERS
	Scale16Nat = BilinearScalerInternal<uint16, Manip_Nat2Nat_16, uint16>::ScaleBilinear;
	Scale16Sta = BilinearScalerInternal<uint16, Manip_Sta2Nat_16, uint32>::ScaleBilinear;

	Scale32Nat = BilinearScalerInternal<uint32, Manip_Nat2Nat_32, uint32>::ScaleBilinear;
	Scale32Sta = BilinearScalerInternal<uint32, Manip_Sta2Nat_32, uint32>::ScaleBilinear;
	Scale32_A888 = BilinearScalerInternal<uint32, Manip_32_A888, uint32>::ScaleBilinear;
	Scale32_888A = BilinearScalerInternal<uint32, Manip_32_888A, uint32>::ScaleBilinear;
#else
	Scale16Nat = 0;
	Scale16Sta = 0;

	Scale32Nat = BilinearScalerInternal<uint32, Manip_Nat2Nat_32, uint32>::ScaleBilinear;
	Scale32Sta = BilinearScalerInternal<uint32, Manip_Sta2Nat_32, uint32>::ScaleBilinear;
	Scale32_A888 = Scale32Nat;
	Scale32_888A = Scale32Nat;
#endif
}

uint32 BilinearScaler::ScaleBits() const {
	return 0xFFFFFFFF;
}
bool BilinearScaler::ScaleArbitrary() const {
	return true;
}

const char *BilinearScaler::ScalerName() const {
	return "bilinear";
}
const char *BilinearScaler::ScalerDesc() const {
	return "Bilinear Filtering Scaler";
}
const char *BilinearScaler::ScalerCopyright() const {
	return "Copyright (C) 2005 The Pentagram Team";
}

const BilinearScaler bilinear_scaler;

#ifdef COMPILE_GAMMA_CORRECT_SCALERS
GC_BilinearScaler::GC_BilinearScaler() : Scaler() {
#ifdef COMPILE_ALL_BILINEAR_SCALERS
	Scale16Nat = BilinearScalerInternal<uint16, Manip_Nat2Nat_16_GC, uint16>::ScaleBilinear;
	Scale16Sta = BilinearScalerInternal<uint16, Manip_Sta2Nat_16_GC, uint32>::ScaleBilinear;

	Scale32Nat = BilinearScalerInternal<uint32, Manip_Nat2Nat_32_GC, uint32>::ScaleBilinear;
	Scale32Sta = BilinearScalerInternal<uint32, Manip_Sta2Nat_32_GC, uint32>::ScaleBilinear;
	Scale32_A888 = BilinearScalerInternal<uint32, Manip_32_A888_GC, uint32>::ScaleBilinear;
	Scale32_888A = BilinearScalerInternal<uint32, Manip_32_888A_GC, uint32>::ScaleBilinear;
#else
	Scale16Nat = 0;
	Scale16Sta = 0;

	Scale32Nat = BilinearScalerInternal<uint32, Manip_Nat2Nat_32_GC, uint32>::ScaleBilinear;
	Scale32Sta = BilinearScalerInternal<uint32, Manip_Sta2Nat_32_GC, uint32>::ScaleBilinear;
	Scale32_A888 = Scale32Nat;
	Scale32_888A = Scale32Nat;
#endif
}

uint32 GC_BilinearScaler::ScaleBits() const {
	return 0xFFFFFFFF;
}
bool GC_BilinearScaler::ScaleArbitrary() const {
	return true;
}

const char *GC_BilinearScaler::ScalerName() const {
	return "gc-bilinear";
}
const char *GC_BilinearScaler::ScalerDesc() const {
	return "Gamma 2.2 Correct Bilinear Filtering Scaler";
}
const char *GC_BilinearScaler::ScalerCopyright() const {
	return "Copyright (C) 2005 The Pentagram Team";
}

const GC_BilinearScaler GC_bilinear_scaler;
#endif

} // End of namespace Pentagram
} // End of namespace Ultima8
