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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/misc.h"
#include "ultima/nuvie/screen/scale.h"
#include "ultima/nuvie/screen/surface.h"
// Include all the Template Scaler Code
#include "ultima/nuvie/screen/scale.inl"

namespace Ultima {
namespace Nuvie {

//
// The Array of Scalers
//
const ScalerStruct ScalerRegistry::scaler_array[] = {

	// Point (this MUST be first)
	{
		"Point",
		0,
		Scalers<uint16, ManipRGBGeneric>::Scale_point,
		Scalers<uint16, ManipRGBGeneric>::Scale_point,
		Scalers<uint16, ManipRGBGeneric>::Scale_point,
		Scalers<uint32, ManipRGBGeneric>::Scale_point,
		Scalers<uint32, ManipRGBGeneric>::Scale_point
	},

	// Interlaced
	{
		"Interlaced",
		0,
		Scalers<uint16, ManipRGBGeneric>::Scale_interlaced,
		Scalers<uint16, ManipRGBGeneric>::Scale_interlaced,
		Scalers<uint16, ManipRGBGeneric>::Scale_interlaced,
		Scalers<uint32, ManipRGBGeneric>::Scale_interlaced,
		Scalers<uint32, ManipRGBGeneric>::Scale_interlaced
	},

	// 2xSaI
	{
		"2xSaI",
		SCALER_FLAG_2X_ONLY,
		Scalers<uint16, ManipRGBGeneric>::Scale_2xSaI,
		Scalers<uint16, ManipRGB555>::Scale_2xSaI,
		Scalers<uint16, ManipRGB565>::Scale_2xSaI,
		Scalers<uint32, ManipRGBGeneric>::Scale_2xSaI,
		Scalers<uint32, ManipRGB888>::Scale_2xSaI
	},

	// Super2xSaI
	{
		"Super2xSaI",
		SCALER_FLAG_2X_ONLY,
		Scalers<uint16, ManipRGBGeneric>::Scale_Super2xSaI,
		Scalers<uint16, ManipRGB555>::Scale_Super2xSaI,
		Scalers<uint16, ManipRGB565>::Scale_Super2xSaI,
		Scalers<uint32, ManipRGBGeneric>::Scale_Super2xSaI,
		Scalers<uint32, ManipRGB888>::Scale_Super2xSaI
	},


	// Scale2x - AdvanceMAME
	{
		"Scale2x",
		SCALER_FLAG_2X_ONLY,
		Scalers<uint16, ManipRGBGeneric>::Scale_Scale2x,
		Scalers<uint16, ManipRGB555>::Scale_Scale2x,
		Scalers<uint16, ManipRGB565>::Scale_Scale2x,
		Scalers<uint32, ManipRGBGeneric>::Scale_Scale2x,
		Scalers<uint32, ManipRGB888>::Scale_Scale2x
	},

	// SuperEagle
	{
		"SuperEagle",
		SCALER_FLAG_2X_ONLY,
		Scalers<uint16, ManipRGBGeneric>::Scale_SuperEagle,
		Scalers<uint16, ManipRGB555>::Scale_SuperEagle,
		Scalers<uint16, ManipRGB565>::Scale_SuperEagle,
		Scalers<uint32, ManipRGBGeneric>::Scale_SuperEagle,
		Scalers<uint32, ManipRGB888>::Scale_SuperEagle
	},

	// Bilinear
	{
		"Bilinear",
		SCALER_FLAG_2X_ONLY,
		Scalers<uint16, ManipRGBGeneric>::Scale_Bilinear,
		Scalers<uint16, ManipRGB555>::Scale_Bilinear,
		Scalers<uint16, ManipRGB565>::Scale_Bilinear,
		Scalers<uint32, ManipRGBGeneric>::Scale_Bilinear,
		Scalers<uint32, ManipRGB888>::Scale_Bilinear
	},

	// BilinearPlus
	{
		"BilinearPlus",
		SCALER_FLAG_2X_ONLY,
		Scalers<uint16, ManipRGBGeneric>::Scale_BilinearPlus,
		Scalers<uint16, ManipRGB555>::Scale_BilinearPlus,
		Scalers<uint16, ManipRGB565>::Scale_BilinearPlus,
		Scalers<uint32, ManipRGBGeneric>::Scale_BilinearPlus,
		Scalers<uint32, ManipRGB888>::Scale_BilinearPlus
	},

	// BilinearInterlaced
	{
		"BilinearInterlaced",
		SCALER_FLAG_2X_ONLY,
		Scalers<uint16, ManipRGBGeneric>::Scale_BilinearInterlaced,
		Scalers<uint16, ManipRGB555>::Scale_BilinearInterlaced,
		Scalers<uint16, ManipRGB565>::Scale_BilinearInterlaced,
		Scalers<uint32, ManipRGBGeneric>::Scale_BilinearInterlaced,
		Scalers<uint32, ManipRGB888>::Scale_BilinearInterlaced
	},

	// BilinearHalfInterlaced
	{
		"BilinearHalfInterlaced",
		SCALER_FLAG_2X_ONLY,
		Scalers<uint16, ManipRGBGeneric>::Scale_BilinearHalfInterlaced,
		Scalers<uint16, ManipRGB555>::Scale_BilinearHalfInterlaced,
		Scalers<uint16, ManipRGB565>::Scale_BilinearHalfInterlaced,
		Scalers<uint32, ManipRGBGeneric>::Scale_BilinearHalfInterlaced,
		Scalers<uint32, ManipRGB888>::Scale_BilinearHalfInterlaced
	},

	// BilinearQuarterInterlaced
	{
		"BilinearQuarterInterlaced",
		SCALER_FLAG_2X_ONLY,
		Scalers<uint16, ManipRGBGeneric>::Scale_BilinearQuarterInterlaced,
		Scalers<uint16, ManipRGB555>::Scale_BilinearQuarterInterlaced,
		Scalers<uint16, ManipRGB565>::Scale_BilinearQuarterInterlaced,
		Scalers<uint32, ManipRGBGeneric>::Scale_BilinearQuarterInterlaced,
		Scalers<uint32, ManipRGB888>::Scale_BilinearQuarterInterlaced
	},

	// Null Scaler (Terminator)
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0
	}
};

//
// Constructor
//
ScalerRegistry::ScalerRegistry() : num_scalers(0) {
	// Count number of scalers
	while (scaler_array[num_scalers].name) {
//		Std::cout << "Scaler " << num_scalers << ": " << scaler_array[num_scalers].name << Std::endl;
		num_scalers++;
	}
//	Std::cout << num_scalers << " Scalers." << Std::endl;
}

//
// Destructor
//
ScalerRegistry::~ScalerRegistry() {
}

//
// Get the Index of a scaler from it's Name
//
int ScalerRegistry::GetIndexForName(const Std::string &name) {
	// Make the name uppercase
	//FIX Std::string sclr = to_uppercase(name);

	for (int index = 0; index < num_scalers; index++) {

		// Make this name also uppercase
		//  Std::string sclr2 = to_uppercase(scaler_array[index].name);

//		if (sclr == sclr2) return index;
		if (string_i_compare(name, scaler_array[index].name)) return index;

	}

	return -1;
}

//
// Get Name of a Scaler from its Index
//
const char *ScalerRegistry::GetNameForIndex(int index) {
	if (index < 0 || index >= num_scalers) return 0;

	return scaler_array[index].name;
}


//
// Get a Scaler from it's Index
//
const ScalerStruct *ScalerRegistry::GetScaler(int index) {
	if (index < 0 || index >= num_scalers) return 0;

	return scaler_array + index;
}

//
// Get a Scaler from it's Index
//
const ScalerStruct *ScalerRegistry::GetPointScaler() {
	// Point scaler is always first
	return scaler_array;
}


#if 0

//
// BlurFilter
//
void Application::show_8to16_blur
(
    int x, int y, int w, int h  // Area to show.
) {
	Manip8to16 manip(palette,
	                 sdl_surf->format);
	BlurFilter<unsigned char, uint16, Manip8to16>
	(screen->pixels, x, y, w, h,
	 screen->w, screen->h, screen->pitch,
	 (uint16 *) sdl_surf->pixels,
	 sdl_surf->pitch /
	 sdl_surf->format->BytesPerPixel,
	 manip);
}

void Application::show_8to555_blur
(
    int x, int y, int w, int h  // Area to show.
) {
	Manip8to555 manip(palette);
	BlurFilter<unsigned char, uint16, Manip8to555>
	(screen->pixels, x, y, w, h,
	 screen->w, screen->h, screen->pitch,
	 (uint16 *) sdl_surf->pixels,
	 sdl_surf->pitch /
	 sdl_surf->format->BytesPerPixel,
	 manip);
}

void Application::show_8to565_blur
(
    int x, int y, int w, int h  // Area to show.
) {
	Manip8to565 manip(palette);
	BlurFilter<unsigned char, uint16, Manip8to565>
	(screen->pixels, x, y, w, h,
	 screen->w, screen->h, screen->pitch,
	 (uint16 *) sdl_surf->pixels,
	 sdl_surf->pitch /
	 sdl_surf->format->BytesPerPixel,
	 manip);
}

void Application::show_8to32_blur
(
    int x, int y, int w, int h  // Area to show.
) {
	Manip8to32 manip(palette,
	                 sdl_surf->format);
	BlurFilter<unsigned char, uint32, Manip8to32>
	(screen->pixels, x, y, w, h,
	 screen->w, screen->h, screen->pitch,
	 (uint32 *) sdl_surf->pixels,
	 sdl_surf->pitch /
	 sdl_surf->format->BytesPerPixel,
	 manip);
}

//
// Rotator
//
#if 0
// Flips vertically
#define RotatorFunc(DestType)                                               \
	Rotator(                                                                \
	        screen->pixels,             /* ->source pixels. */                  \
	        x, y,                       /* Start of rectangle within src. */    \
	        w, h,                       /* Dims. of rectangle. */               \
	        screen->w,                  /* Source width. */                     \
	        screen->h,                  /* Source height. */                    \
	        screen->pitch,              /* Pixels/line for source. */           \
	        \
	        (DestType*)( ((char*)sdl_surf->pixels) + (screen->h-1)*sdl_surf->pitch),/* ->dest pixels. */                    \
	        x, y,                       /* Start of rectangle within dest. */   \
	        w, h,                       /* Dims. of rectangle. */               \
	        screen->w,                  /* Dest height. */                      \
	        screen->h,                  /* Dest width. */                       \
	        1,                          /* Amount to increment for each y pixel */\
	        -(sdl_surf->pitch/sdl_surf->format->BytesPerPixel),/* Amount to increment for each x pixel */\
	        manip);                     /* Manipulator methods. */
// Flips horizontally
#define RotatorFunc(DestType)                                               \
	Rotator(                                                                \
	        screen->pixels,             /* ->source pixels. */                  \
	        x, y,                       /* Start of rectangle within src. */    \
	        w, h,                       /* Dims. of rectangle. */               \
	        screen->w,                  /* Source width. */                     \
	        screen->h,                  /* Source height. */                    \
	        screen->pitch,              /* Pixels/line for source. */           \
	        \
	        (DestType*)(sdl_surf->pixels) + screen->w-1, /* ->dest pixels. */   \
	        x, y,                       /* Start of rectangle within dest. */   \
	        w, h,                       /* Dims. of rectangle. */               \
	        screen->w,                  /* Dest height. */                      \
	        screen->h,                  /* Dest width. */                       \
	        -1,                         /* Amount to increment for each y pixel */\
	        (sdl_surf->pitch/sdl_surf->format->BytesPerPixel),/* Amount to increment for each x pixel */\
	        manip);                     /* Manipulator methods. */
#endif
// Rotates
#define RotatorFunc(DestType)                                               \
	Rotator(                                                                \
	        screen->pixels,             /* ->source pixels. */                  \
	        x, y,                       /* Start of rectangle within src. */    \
	        w, h,                       /* Dims. of rectangle. */               \
	        screen->w,                  /* Source width. */                     \
	        screen->h,                  /* Source height. */                    \
	        screen->pitch,              /* Pixels/line for source. */           \
	        \
	        (DestType*)(sdl_surf->pixels)+screen->h-1, /* ->dest pixels. */ \
	        (sdl_surf->pitch/sdl_surf->format->BytesPerPixel),/* Amount to increment for each x pixel */\
	        -1,                         /* Amount to increment for each y pixel */\
	        manip);                     /* Manipulator methods. */

void Application::show_8to16_rotated
(
    int x, int y, int w, int h  // Area to show.
) {
	Manip8to16 manip(palette, sdl_surf->format);

	RotatorFunc(uint16);
}

void Application::show_8to555_rotated
(
    int x, int y, int w, int h  // Area to show.
) {
	Manip8to555 manip(palette);
	RotatorFunc(uint16);
}

void Application::show_8to565_rotated
(
    int x, int y, int w, int h  // Area to show.
) {
	Manip8to565 manip(palette);
	RotatorFunc(uint16);
}

void Application::show_8to32_rotated
(
    int x, int y, int w, int h  // Area to show.
) {
	Manip8to32 manip(palette, sdl_surf->format);
	RotatorFunc(uint32);
}
#endif

} // End of namespace Nuvie
} // End of namespace Ultima
