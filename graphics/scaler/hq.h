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

#ifndef GRAPHICS_SCALER_HQ_H
#define GRAPHICS_SCALER_HQ_H

#include "graphics/scalerplugin.h"

class HQPlugin : public ScalerPluginObject {
public:
	HQPlugin();
	virtual void initialize(const Graphics::PixelFormat &format) override;
	virtual void deinitialize() override;
	virtual uint increaseFactor() override;
	virtual uint decreaseFactor() override;
	virtual bool canDrawCursor() const override { return false; }
	virtual uint extraPixels() const override { return 1; }
	virtual const char *getName() const override;
	virtual const char *getPrettyName() const override;
protected:
	virtual void scaleIntern(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) override;
};


#endif
