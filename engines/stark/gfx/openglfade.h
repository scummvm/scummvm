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

#ifndef STARK_GFX_OPENGL_FADE_H
#define STARK_GFX_OPENGL_FADE_H

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL_GAME)

#include "engines/stark/gfx/faderenderer.h"

namespace Stark {
namespace Gfx {

class OpenGLDriver;

/**
 * A programmable pipeline OpenGL fade screen renderer
 */
class OpenGLFadeRenderer : public FadeRenderer {
public:
	OpenGLFadeRenderer(OpenGLDriver *gfx);
	~OpenGLFadeRenderer();

	// FadeRenderer API
	void render(float fadeLevel);

private:
	OpenGLDriver *_gfx;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_OPENGL_GAME)

#endif // STARK_GFX_OPENGL_FADE_H
