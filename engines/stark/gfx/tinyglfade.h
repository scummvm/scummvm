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

#ifndef STARK_GFX_TINYGL_FADE_H
#define STARK_GFX_TINYGL_FADE_H

#include "engines/stark/gfx/faderenderer.h"

#include "graphics/tinygl/tinygl.h"

namespace Stark {
namespace Gfx {

class TinyGLDriver;

/**
 * A programmable pipeline TinyGL fade screen renderer
 */
class TinyGLFadeRenderer : public FadeRenderer {
public:
	TinyGLFadeRenderer(TinyGLDriver *gfx);
	~TinyGLFadeRenderer();

	// FadeRenderer API
	void render(float fadeLevel);

private:
	TinyGLDriver *_gfx;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_TINYGL_FADE_H
