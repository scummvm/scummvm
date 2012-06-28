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
 */

#ifndef GRAPHICS_SCALER_EDGE_H
#define GRAPHICS_SCALER_EDGE_H

#include "graphics/scalerplugin.h"

class EdgePlugin : public ScalerPluginObject {
public:
	EdgePlugin();
	virtual void initialize(Graphics::PixelFormat format);
	virtual void deinitialize();
	virtual void scale(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y);
	virtual uint increaseFactor();
	virtual uint decreaseFactor();
	virtual uint getFactor() const { return _factor; }
	virtual bool canDrawCursor() const { return false; }
	virtual uint extraPixels() const { return 1; }
	virtual const char *getName() const;
	virtual const char *getPrettyName() const;
private:
	int16* chooseGreyscale(uint16 *pixels);
	int32 calcPixelDiffNosqrt(uint16 pixel1, uint16 pixel2);
	int findPrincipleAxis(uint16 *pixels, int16 *diffs, int16 *bplane,
		int8 *sim,
		int32 *return_angle);
	int checkArrows(int best_dir, uint16 *pixels, int8 *sim, int half_flag);
	int refineDirection(char edge_type, uint16 *pixels, int16 *bptr,
		int8 *sim, double angle);
	int fixKnights(int sub_type, uint16 *pixels, int8 *sim);
	void initTables(const uint8 *srcPtr, uint32 srcPitch,
		int width, int height);

	template<typename ColorMask>
	void anti_alias_grid_2x(uint8 *dptr, int dstPitch,
		uint16 *pixels, int sub_type, int16 *bptr,
		int8 *sim,
		int interpolate_2x);
	template<typename ColorMask>
	void anti_alias_grid_clean_3x(uint8 *dptr, int dstPitch,
		uint16 *pixels, int sub_type, int16 *bptr);
	template<typename ColorMask>
	void antiAliasPass2x(const uint8 *src, uint8 *dst,
		int w, int h, int w_new, int h_new,
		int srcPitch, int dstPitch,
		int overlay_flag,
		int interpolate_2x);
	template<typename ColorMask>
	void antiAliasPass3x(const uint8 *src, uint8 *dst,
		int w, int h, int w_new, int h_new,
		int srcPitch, int dstPitch,
		int overlay_flag);

	int16 _rgbTable[65536][3];       ///< table lookup for RGB
	int16 _greyscaleTable[3][65536]; ///< greyscale tables
	int16 *_chosenGreyscale;               ///< pointer to chosen greyscale table
	int16 *_bptr;                          ///< too awkward to pass variables
	int8 _simSum;                          ///< sum of similarity matrix
	int16 _greyscaleDiffs[3][8];
	int16 _bplanes[3][9];
};


#endif
