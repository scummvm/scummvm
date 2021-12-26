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

#include "engines/stark/gfx/tinyglfade.h"
#include "engines/stark/gfx/tinygl.h"

namespace Stark {
namespace Gfx {

static const TGLfloat fadeVertices[] = {
	// X   Y
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	-1.0f, -1.0f,
	 1.0f, -1.0f,
};

TinyGLFadeRenderer::TinyGLFadeRenderer(TinyGLDriver *gfx) :
	FadeRenderer(),
	_gfx(gfx) {
}

TinyGLFadeRenderer::~TinyGLFadeRenderer() {
}

void TinyGLFadeRenderer::render(float fadeLevel) {
	_gfx->start2DMode();

	tglMatrixMode(TGL_PROJECTION);
	tglPushMatrix();
	tglLoadIdentity();

	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
	tglLoadIdentity();

	tglDisable(TGL_TEXTURE_2D);

	tglColor4f(0.0f, 0.0f, 0.0f, 1.0f - fadeLevel);

	tglEnableClientState(TGL_VERTEX_ARRAY);

	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), &fadeVertices[0]);

	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglDisableClientState(TGL_VERTEX_ARRAY);

	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();

	tglMatrixMode(TGL_PROJECTION);
	tglPopMatrix();

	_gfx->end2DMode();
}

} // End of namespace Gfx
} // End of namespace Stark
