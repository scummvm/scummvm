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

#ifndef NUVIE_SCREEN_SCALE_H
#define NUVIE_SCREEN_SCALE_H

#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Nuvie {

#define SCALER_FLAG_2X_ONLY         1
#define SCALER_FLAG_16BIT_ONLY      2
#define SCALER_FLAG_32BIT_ONLY      4

struct ScalerStruct {
	typedef void (*ScalerType16)(uint16 *, int , int , int , int , const int , const int , uint16 *, const int, int);
	typedef void (*ScalerType32)(uint32 *, int , int , int , int , const int , const int , uint32 *, const int, int);

	const char      *name;
	uint32          flags;              // Scaler flags

	ScalerType16    scale16;
	ScalerType16    scale555;
	ScalerType16    scale565;

	ScalerType32    scale32;
	ScalerType32    scale888;

	// Call this to scale a section of the screen
	inline void     Scale(
	    int type,                   // Format type of buffers 16, 555, 565, 32 or 888
	    void *source,               // ->source pixels.
	    int srcx, int srcy,         // Start of rectangle within src.
	    int srcw, int srch,         // Dims. of rectangle.
	    const int sline_pixels,     // Pixels /line for source.
	    const int sheight,          // Source height.
	    void *dest,                 // ->dest pixels.
	    const int dline_pixels,     // Pixels /line for dest.
	    int scale_factor            // Scale factor
	) const {
		if (type == 16) {
			scale16((uint16 *) source, srcx, srcy, srcw, srch, sline_pixels, sheight, (uint16 *) dest, dline_pixels, scale_factor);
		} else if (type == 555) {
			scale16((uint16 *) source, srcx, srcy, srcw, srch, sline_pixels, sheight, (uint16 *) dest, dline_pixels, scale_factor);
		} else if (type == 565) {
			scale16((uint16 *) source, srcx, srcy, srcw, srch, sline_pixels, sheight, (uint16 *) dest, dline_pixels, scale_factor);
		} else if (type == 32) {
			scale32((uint32 *) source, srcx, srcy, srcw, srch, sline_pixels, sheight, (uint32 *) dest, dline_pixels, scale_factor);
		} else if (type == 888) {
			scale888((uint32 *) source, srcx, srcy, srcw, srch, sline_pixels, sheight, (uint32 *) dest, dline_pixels, scale_factor);
		}
	}

};

//
// This entire class is just static
//
class ScalerRegistry {
	static const            ScalerStruct scaler_array[];
	int                     num_scalers;

public:

	// Constructor
	ScalerRegistry();

	// Destructor
	~ScalerRegistry();

	// Get the total Number of scalers
	int             GetNumScalers() {
		return num_scalers;
	}

	// Get the Scaler Index from it's name
	int             GetIndexForName(const Std::string &name);

	// Get Name of a Scaler from its Index
	const char      *GetNameForIndex(int index);

	// Get a Scaler from it's Index
	const ScalerStruct  *GetScaler(int index);

	// Get the Point Sampling Scaler
	const ScalerStruct  *GetPointScaler();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
