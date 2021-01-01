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
 * Copyright 2020 Google
 *
 */
#ifndef HADESCH_GFX_CONTEXT_H
#define HADESCH_GFX_CONTEXT_H

#include "common/ptr.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"

namespace Hadesch {
class GfxContext {
public:
	virtual void blitVideo(byte *sourcePixels, int sourcePitch, int sourceW, int sourceH,
			       byte *palette, Common::Point offset) = 0;
	// Use only in pod_image.cpp because of palette format
	virtual void blitPodImage(byte *sourcePixels, int sourcePitch, int sourceW, int sourceH,
				  byte *palette, size_t ncolours, Common::Point offset) = 0;
	virtual void clear() = 0;
	virtual void renderSubtitle(const Common::U32String &, Common::Point viewPoint) = 0;
	virtual void fade(int val) = 0;
	virtual void renderToScreen(Common::Point viewPoint) = 0;
	virtual ~GfxContext() {}
};

class GfxContext8Bit : public GfxContext, Common::NonCopyable {
public:
  	void blitVideo(byte *sourcePixels, int sourcePitch, int sourceW, int sourceH,
		       byte *palette, Common::Point offset) override;
	void blitPodImage(byte *sourcePixels, int sourcePitch, int sourceW, int sourceH,
			  byte *palette, size_t ncolours, Common::Point offset) override;
	void clear() override;
	void fade(int val) override;
	void renderToScreen(Common::Point viewPoint) override;
	void renderSubtitle(const Common::U32String &, Common::Point viewPoint) override;
	byte findColor(byte r, byte g, byte b);

	GfxContext8Bit(int canvasW, int canvasH);
	~GfxContext8Bit() {}

private:
	Graphics::ManagedSurface surf;
	byte _palette[256 * 4];
	bool _paletteUsed[256];
};

void blendVideo8To8(byte *targetPixels, int targetPitch, int targetW, int targetH,
		    byte *sourcePixels, int sourceW, int sourceH, Common::Point offset);

}

#endif
