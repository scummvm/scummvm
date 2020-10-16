/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/wintermute/wintypes.h"
#include "graphics/opengl/system_headers.h"

#if (defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)) && !defined(USE_GLES2)

#include "engines/wintermute/base/gfx/opengl/mesh3ds_opengl.h"

namespace Wintermute {

Mesh3DSOpenGL::Mesh3DSOpenGL() {
}

Mesh3DSOpenGL::~Mesh3DSOpenGL() {
}

void Mesh3DSOpenGL::fillVertexBuffer(uint32 color) {
	_color.x() = RGBCOLGetR(color) / 255.0f;
	_color.y() = RGBCOLGetG(color) / 255.0f;
	_color.z() = RGBCOLGetB(color) / 255.0f;
	_color.w() = RGBCOLGetA(color) / 255.0f;
}

void Mesh3DSOpenGL::render() {
	glColor4f(_color.x(), _color.y(), _color.z(), _color.w());
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(GeometryVertex), reinterpret_cast<byte *>(_vertexData));
	glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_SHORT, _indexData);
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_GAME) && !defined(USE_GLES2)
