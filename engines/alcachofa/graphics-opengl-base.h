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

#ifndef ALCACHOFA_GRAPHICS_OPENGL_BASE_H
#define ALCACHOFA_GRAPHICS_OPENGL_BASE_H

// This file shall not use any OpenGL API surface or include headers related to it

#include "alcachofa/graphics.h"

namespace Alcachofa {

bool isCompatibleFormat(const Graphics::PixelFormat &format);

class OpenGLTextureBase : public ITexture {
public:
	OpenGLTextureBase(int32 w, int32 h, bool withMipmaps);
	~OpenGLTextureBase() override {}
	void update(const Graphics::Surface &surface) override;

protected:
	virtual void updateInner(const void *pixels) = 0; ///< expects pixels to be RGBA32

	bool _withMipmaps;
	bool _mirrorWrap = true;
	bool _didConvertOnce = false;
	Graphics::ManagedSurface _tmpSurface;
};

class OpenGLRendererBase : public virtual IRenderer {
public:
	OpenGLRendererBase(Common::Point resolution);

	bool hasOutput() const override;

protected:
	virtual void setViewportInner(int x, int y, int width, int height) = 0;
	virtual void setMatrices(bool flipped) = 0;

	void resetState();
	void setViewportToScreen();
	void setViewportToRect(int16 outputWidth, int16 outputHeight);
	void getQuadPositions(Math::Vector2d topLeft, Math::Vector2d size, Math::Angle rotation, Math::Vector2d positions[]) const;
	void getQuadTexCoords(Math::Vector2d texMin, Math::Vector2d texMax, Math::Vector2d texCoords[]) const;

	Common::Point _resolution, _outputSize;
	Graphics::Surface *_currentOutput = nullptr;
	BlendMode _currentBlendMode = (BlendMode)-1;
	float _currentLodBias = 0.0f;
	bool _isFirstDrawCommand = false;
};

}

#endif // ALCACHOFA_GRAPHICS_OPENGL_BASE_H
