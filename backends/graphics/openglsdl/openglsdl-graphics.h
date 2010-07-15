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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef BACKENDS_GRAPHICS_OPENGLSDL_H
#define BACKENDS_GRAPHICS_OPENGLSDL_H

#include <SDL.h>
#if defined(ARRAYSIZE) && !defined(_WINDOWS_)
#undef ARRAYSIZE
#endif
#include <SDL_opengl.h>

#include "backends/graphics/opengl/opengl-graphics.h"

/**
 * SDL OpenGL graphics manager
 */
class OpenGLSdlGraphicsManager : public OpenGLGraphicsManager {
public:
	OpenGLSdlGraphicsManager();
	virtual ~OpenGLSdlGraphicsManager();

#ifdef USE_RGB_COLOR
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
#endif

	virtual void warpMouse(int x, int y);

protected:
	virtual void internUpdateScreen();

	virtual bool loadGFXMode();
	virtual void unloadGFXMode();
	virtual bool hotswapGFXMode();

	// Hardware screen
	SDL_Surface *_hwscreen;
};

#endif
