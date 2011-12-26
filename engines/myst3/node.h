/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef MYST3_ROOM_H
#define MYST3_ROOM_H

#ifdef SDL_BACKEND
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "engines/myst3/archive.h"

#include "graphics/surface.h"
#include "graphics/jpeg.h"
#include "graphics/conversion.h"

namespace Myst3 {

class Face {
	public:
		Graphics::Surface *_bitmap;
		GLuint _textureId;

		void createTexture();
		void setTextureFromJPEG(Graphics::JPEG *jpeg);
		void uploadTexture();
		void unload();
};


class Node {
	protected:
		Face _faces[6];

	public:
		virtual void draw() = 0;
		virtual void load(Archive &archive, uint16 id) = 0;
		virtual void unload();
		void dumpFaceMask(Archive &archive, uint16 index, int face);
		virtual ~Node() {};

		static const int _cubeTextureSize = 1024;
};

} // end of namespace Myst3

#endif
