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

#if defined(USE_HQ2X_SCALER) || defined(USE_HQ3X_SCALER) || defined(USE_HQ4X_SCALER)

#include "hqScaler.h"
#include "Manips.h"
#include "ultima8/graphics/texture.h"

#include "hq2xScaler.h"
#include "hq3xScaler.h"
#include "hq4xScaler.h"

namespace Pentagram {

#ifdef USE_HQ2X_SCALER
#define CASE_2X(Scale) \
	case 2: \
	return ((const Scaler*)&hq2x_scaler)->Scale(tex,sx,sy,sw,sh,pixel,dw,dh,pitch,clamp_src);
#else
#define CASE_2X(Scale)

#endif

#ifdef USE_HQ3X_SCALER
#define CASE_3X(Scale) \
	case 3: \
	return ((const Scaler*)&hq3x_scaler)->Scale(tex,sx,sy,sw,sh,pixel,dw,dh,pitch,clamp_src);
#else
#define CASE_3X(Scale)
#endif

#ifdef USE_HQ4X_SCALER
#define CASE_4X(Scale) \
	case 4: \
	return ((const Scaler*)&hq4x_scaler)->Scale(tex,sx,sy,sw,sh,pixel,dw,dh,pitch,clamp_src);
#else
#define CASE_4X(Scale)
#endif

bool hqScaler::_Scale16Nat(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
                           uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	switch (dw / sw) {
		CASE_2X(Scale16Nat)
		CASE_3X(Scale16Nat)
		CASE_4X(Scale16Nat)

	default:
		return false;
	}
}
bool hqScaler::_Scale16Sta(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
                           uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	switch (dw / sw) {
		CASE_2X(Scale16Sta)
		CASE_3X(Scale16Sta)
		CASE_4X(Scale16Sta)

	default:
		return false;
	}
}

bool hqScaler::_Scale32Nat(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
                           uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	switch (dw / sw) {
		CASE_2X(Scale32Nat)
		CASE_3X(Scale32Nat)
		CASE_4X(Scale32Nat)

	default:
		return false;
	}
}
bool hqScaler::_Scale32Sta(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
                           uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	switch (dw / sw) {
		CASE_2X(Scale32Sta)
		CASE_3X(Scale32Sta)
		CASE_4X(Scale32Sta)

	default:
		return false;
	}
}
bool hqScaler::_Scale32_A888(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
                             uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	switch (dw / sw) {
		CASE_2X(Scale32_A888)
		CASE_3X(Scale32_A888)
		CASE_4X(Scale32_A888)

	default:
		return false;
	}
}
bool hqScaler::_Scale32_888A(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
                             uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	switch (dw / sw) {
		CASE_2X(Scale32_888A)
		CASE_3X(Scale32_888A)
		CASE_4X(Scale32_888A)

	default:
		return false;
	}
}

hqScaler::hqScaler() : Scaler() {
	Scale16Nat = _Scale16Nat;
	Scale16Sta = _Scale16Sta;

	Scale32Nat = _Scale32Nat;
	Scale32Sta = _Scale32Sta;
	Scale32_A888 = _Scale32_A888;
	Scale32_888A = _Scale32_888A;
}

const uint32 hqScaler::ScaleBits() const {
	return 0
#ifdef USE_HQ2X_SCALER
	       | 1 << 2
#endif
#ifdef USE_HQ3X_SCALER
	       | 1 << 3
#endif
#ifdef USE_HQ4X_SCALER
	       | 1 << 4
#endif
	       ;
}
const bool hqScaler::ScaleArbitrary() const {
	return false;
}

const char *hqScaler::ScalerName() const {
	return "hq";
}
const char *hqScaler::ScalerDesc() const {
	return
#ifdef USE_HQ2X_SCALER
	    "hq2x "
#endif
#ifdef USE_HQ3X_SCALER
	    "hq3x "
#endif
#ifdef USE_HQ4X_SCALER
	    "hq4x "
#endif
	    "Magnification Filter";
}
const char *hqScaler::ScalerCopyright() const {
	return "Copyright (C) 2003 MaxSt";
}

const hqScaler hq_scaler;

};  // namespace Pentagram

#endif
