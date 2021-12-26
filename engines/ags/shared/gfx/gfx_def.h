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

//=============================================================================
//
// Graphic definitions and type/unit conversions.
//
//=============================================================================

#ifndef AGS_SHARED_GFX_GFX_DEF_H
#define AGS_SHARED_GFX_GFX_DEF_H

namespace AGS3 {
namespace AGS {
namespace Shared {

enum BlendMode {
	// free blending (ARGB -> ARGB) modes
	kBlendMode_NoAlpha = 0, // ignore alpha channel
	kBlendMode_Alpha,              // alpha-blend src to dest, combining src & dest alphas
	// NOTE: add new modes here

	kNumBlendModes
};

namespace GfxDef {

inline int Trans100ToAlpha255(int transparency) {
	return ((100 - transparency) * 255) / 100;
}

inline int Alpha255ToTrans100(int alpha) {
	return 100 - ((alpha * 100) / 255);
}

// Special formulae to reduce precision loss and support flawless forth &
// reverse conversion for multiplies of 10%
inline int Trans100ToAlpha250(int transparency) {
	return ((100 - transparency) * 25) / 10;
}

inline int Alpha250ToTrans100(int alpha) {
	return 100 - ((alpha * 10) / 25);
}

// Convert correct 100-ranged transparency into legacy 255-ranged
// transparency; legacy inconsistent transparency value range:
// 0   = opaque,
// 255 = invisible,
// 1 -to- 254 = barely visible -to- mostly visible (as proper alpha)
inline int Trans100ToLegacyTrans255(int transparency) {
	if (transparency == 0) {
		return 0; // this means opaque
	} else if (transparency == 100) {
		return 255; // this means invisible
	}
	// the rest of the range works as alpha
	return Trans100ToAlpha250(transparency);
}

// Convert legacy 255-ranged "incorrect" transparency into proper
// 100-ranged transparency.
inline int LegacyTrans255ToTrans100(int legacy_transparency) {
	if (legacy_transparency == 0) {
		return 0; // this means opaque
	} else if (legacy_transparency == 255) {
		return 100; // this means invisible
	}
	// the rest of the range works as alpha
	return Alpha250ToTrans100(legacy_transparency);
}

// Convert legacy 100-ranged transparency into proper 255-ranged alpha
// 0      => alpha 255
// 100    => alpha 0
// 1 - 99 => alpha 1 - 244
inline int LegacyTrans100ToAlpha255(int legacy_transparency) {
	if (legacy_transparency == 0) {
		return 255; // this means opaque
	} else if (legacy_transparency == 100) {
		return 0; // this means invisible
	}
	// the rest of the range works as alpha (only 100-ranged)
	return legacy_transparency * 255 / 100;
}

} // namespace GfxDef

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
