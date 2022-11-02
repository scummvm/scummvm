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

#ifndef GRAPHICS_SCALER_HQ_H
#define GRAPHICS_SCALER_HQ_H

#include "graphics/scalerplugin.h"

#ifdef USE_NASM
struct hqx_parameters;
#endif

class HQScaler : public Scaler {
public:
	HQScaler(const Graphics::PixelFormat &format);
	~HQScaler();
	uint increaseFactor() override;
	uint decreaseFactor() override;
protected:
	virtual void scaleIntern(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) override;

	void initLUT(Graphics::PixelFormat format);
	inline void HQ2x16(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);
	inline void HQ3x16(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);

	uint32 *_RGBtoYUV;
#ifdef USE_NASM
	hqx_parameters *_hqx_params;
#endif

};


#endif
