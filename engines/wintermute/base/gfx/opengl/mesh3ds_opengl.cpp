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

#include "engines/wintermute/wintypes.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL_GAME)

#include "engines/wintermute/base/gfx/opengl/mesh3ds_opengl.h"

namespace Wintermute {

Mesh3DSOpenGL::Mesh3DSOpenGL() {
}

Mesh3DSOpenGL::~Mesh3DSOpenGL() {
}

void Mesh3DSOpenGL::fillVertexBuffer(uint32 color) {
	_color._x = RGBCOLGetR(color) / 255.0f;
	_color._y = RGBCOLGetG(color) / 255.0f;
	_color._z = RGBCOLGetB(color) / 255.0f;
	_color._w = RGBCOLGetA(color) / 255.0f;
}

void Mesh3DSOpenGL::render() {
	glColor4f(_color._x, _color._y, _color._z, _color._w);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(GeometryVertex), reinterpret_cast<byte *>(_vertexData));
	glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_SHORT, _indexData);
	glDisableClientState(GL_VERTEX_ARRAY);
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_GAME)
