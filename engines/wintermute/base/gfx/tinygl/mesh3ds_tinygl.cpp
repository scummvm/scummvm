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

#if defined(USE_TINYGL)

#include "engines/wintermute/base/gfx/tinygl/mesh3ds_tinygl.h"

namespace Wintermute {

Mesh3DSTinyGL::Mesh3DSTinyGL(BaseGame *inGame) : Mesh3DS(inGame) {
	_vertexCount = 0;
	_vertexData = nullptr;
}

Mesh3DSTinyGL::~Mesh3DSTinyGL() {
}

void Mesh3DSTinyGL::fillVertexBuffer() {
	_vertexCount = _numFaces * 3;
	_vertexData = (Mesh3DSVertex *)_vb.ptr();
}

void Mesh3DSTinyGL::render(bool color) {
	if (_vertexCount == 0)
		return;

	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglEnableClientState(TGL_COLOR_ARRAY);
	tglVertexPointer(3, TGL_FLOAT, sizeof(Mesh3DSVertex), &_vertexData[0]._x);
	tglColorPointer(4, TGL_FLOAT, sizeof(Mesh3DSVertex), &_vertexData[0]._r);
	tglDrawArrays(TGL_TRIANGLES, 0, _vertexCount);
	tglDisableClientState(TGL_COLOR_ARRAY);
	tglDisableClientState(TGL_VERTEX_ARRAY);
}

} // namespace Wintermute

#endif // defined(USE_TINYGL)
