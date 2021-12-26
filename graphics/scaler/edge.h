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
 */

#ifndef GRAPHICS_SCALER_EDGE_H
#define GRAPHICS_SCALER_EDGE_H

#include "graphics/scalerplugin.h"

class EdgeScaler : public SourceScaler {
public:

	EdgeScaler(const Graphics::PixelFormat &format);
	uint increaseFactor() override;
	uint decreaseFactor() override;

protected:

	virtual void internScale(const uint8 *srcPtr, uint32 srcPitch,
						   uint8 *dstPtr, uint32 dstPitch,
						   const uint8 *oldSrcPtr, uint32 oldSrcPitch,
						   int width, int height, const uint8 *buffer, uint32 bufferPitch) override;

private:

	/**
	 * Choose greyscale bitplane to use, return diff array.  Exit early and
	 * return NULL for a block of solid color (all diffs zero).
	 *
	 * No matter how you do it, mapping 3 bitplanes into a single greyscale
	 * bitplane will always result in colors which are very different mapping to
	 * the same greyscale value.  Inevitably, these pixels will appear next to
	 * each other at some point in some image, and edge detection on a single
	 * bitplane will behave quite strangely due to them having the same or nearly
	 * the same greyscale values.  Calculating distances between pixels using all
	 * three RGB bitplanes is *way* too time consuming, so single bitplane
	 * edge detection is used for speed's sake.  In order to try to avoid the
	 * color mapping problems of using a single bitplane, 3 different greyscale
	 * mappings are tested for each 3x3 grid, and the one with the most "signal"
	 * (sum of squares difference from center pixel) is chosen.  This usually
	 * results in useable contrast within the 3x3 grid.
	 *
	 * This results in a whopping 25% increase in overall runtime of the filter
	 * over simply using luma or some other single greyscale bitplane, but it
	 * does greatly reduce the amount of errors due to greyscale mapping
	 * problems.  I think this is the best compromise between accuracy and
	 * speed, and is still a lot faster than edge detecting over all three RGB
	 * bitplanes.  The increase in image quality is well worth the speed hit.
	 */
	template<typename ColorMask>
	int16 *chooseGreyscale(typename ColorMask::PixelType *pixels);

	/**
	 * Calculate the distance between pixels in RGB space.  Greyscale isn't
	 * accurate enough for choosing nearest-neighbors :(  Luma-like weighting
	 * of the individual bitplane distances prior to squaring gives the most
	 * useful results.
	 */
	template<typename ColorMask>
	int32 calcPixelDiffNosqrt(typename ColorMask::PixelType pixel1, typename ColorMask::PixelType pixel2);

	/**
	 * Create vectors of all delta grey values from center pixel, with magnitudes
	 * ranging from [1.0, 0.0] (zero difference, maximum difference).  Find
	 * the two principle axes of the grid by calculating the eigenvalues and
	 * eigenvectors of the inertia tensor.  Use the eigenvectors to calculate the
	 * edge direction.  In other words, find the angle of the line that optimally
	 * passes through the 3x3 pattern of pixels.
	 *
	 * Return horizontal (-), vertical (|), diagonal (/,\), multi (*), or none '0'
	 *
	 * Don't replace any of the double math with integer-based approximations,
	 * since everything I have tried has lead to slight mis-detection errors.
	 */
	int findPrincipleAxis(int16 *diffs, int16 *bplane,
		int8 *sim,
		int32 *return_angle);

	/**
	 * Check for mis-detected arrow patterns.  Return 1 (good), 0 (bad).
	 */
	template<typename Pixel>
	int checkArrows(int best_dir, Pixel *pixels, int8 *sim, int half_flag);

	/**
	 * Take original direction, refine it by testing different pixel difference
	 * patterns based on the initial gross edge direction.
	 *
	 * The angle value is not currently used, but may be useful for future
	 * refinement algorithms.
	 */
	template<typename Pixel>
	int refineDirection(char edge_type, Pixel *pixels, int16 *bptr,
		int8 *sim, double angle);

	/**
	 * "Chess Knight" patterns can be mis-detected, fix easy cases.
	 */
	template<typename Pixel>
	int fixKnights(int sub_type, Pixel *pixels, int8 *sim);

	/**
	 * Initialize various lookup tables
	 */
	void initTables(const uint8 *srcPtr, uint32 srcPitch,
		int width, int height);

	/**
	 * Fill pixel grid with or without interpolation, using the detected edge
	 */
	template<typename ColorMask>
	void antiAliasGrid2x(uint8 *dptr, int dstPitch,
		typename ColorMask::PixelType *pixels, int sub_type, int16 *bptr,
		int8 *sim,
		int interpolate_2x);

	/**
	 * Fill pixel grid without interpolation, using the detected edge
	 */
	template<typename ColorMask>
	void antiAliasGridClean3x(uint8 *dptr, int dstPitch,
		typename ColorMask::PixelType *pixels, int sub_type, int16 *bptr);

	/**
	 * Perform edge detection, draw the new 2x pixels
	 */
	template<typename ColorMask>
	void antiAliasPass2x(const uint8 *src, uint8 *dst,
		int w, int h,
		int srcPitch, int dstPitch,
		int interpolate_2x,
		bool haveOldSrc,
		const uint8 *oldSrc, int oldSrcPitch,
		const uint8 *buffer, int bufferPitch);

	/**
	 * Perform edge detection, draw the new 3x pixels
	 */
	template<typename ColorMask>
	void antiAliasPass3x(const uint8 *src, uint8 *dst,
		int w, int h,
		int srcPitch, int dstPitch,
		bool haveOldSrc,
		const uint8* oldSrc, int oldPitch,
		const uint8 *buffer, int bufferPitch);

	int16 _rgbTable[65536][3];       ///< table lookup for RGB
	int16 _greyscaleTable[3][65536]; ///< greyscale tables
	int16 *_chosenGreyscale;               ///< pointer to chosen greyscale table
	int16 *_bptr;                          ///< too awkward to pass variables
	int8 _simSum;                          ///< sum of similarity matrix
	int16 _greyscaleDiffs[3][8];
	int16 _bplanes[3][9];
};


#endif
