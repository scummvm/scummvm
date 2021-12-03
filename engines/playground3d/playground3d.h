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

#ifndef PLAYGROUND3D_H
#define PLAYGROUND3D_H

#include "common/array.h"

#include "graphics/framelimiter.h"

#include "engines/engine.h"

#include "engines/playground3d/gfx.h"

namespace Playground3d {

class Playground3dEngine : public Engine {
public:
	Playground3dEngine(OSystem *syst);
	~Playground3dEngine() override;

	Common::Error run() override;

	bool hasFeature(EngineFeature f) const override;

	void processInput();

	void drawFrame(int testId);

private:
	OSystem *_system;
	Renderer *_gfx;
	Graphics::FrameLimiter *_frameLimiter;
	Math::Vector4d _clearColor;
	float _fade;
	bool _fadeIn;
	Graphics::Surface *_rgbaTexture;
	Graphics::Surface *_rgbTexture;

	float _rotateAngleX, _rotateAngleY, _rotateAngleZ;

	Graphics::Surface *generateRgbaTexture(int width, int height, Graphics::PixelFormat format);
	void drawAndRotateCube();
	void drawPolyOffsetTest();
	void dimRegionInOut();
	void drawInViewport();
	void drawRgbaTexture();
};

} // End of namespace Playground3d

#endif // PLAYGROUND3D_H
