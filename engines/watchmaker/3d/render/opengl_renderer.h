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

#ifndef WATCHMAKER_OPENGL_RENDERER_H
#define WATCHMAKER_OPENGL_RENDERER_H


#include "watchmaker/3d/math/Matrix4x4.h"
#include "watchmaker/utils.h"
#include "watchmaker/render.h"
#include "watchmaker/3d/render/opengl_3d.h"

namespace Watchmaker {

enum class TransformMatrix {
	PROJECTION,
	VIEW
};

enum class RenderState {
	LIGHT,
	CLIP,
	EXTENT,
	ALPHABLEND,
	ALPHAREF, // TODO
	ZENABLE, // TODO
	ZWRITE_ENABLE, // TODO
	TEXTUREFACTOR // TODO
};

enum class BlendFactor {
	ONE,
	ZERO,
	SRCALPHA,
	INVSRCALPHA,
	INVSRCCOLOR,
	SRCCOLOR,
	DESTCOLOR,
	INVDESTCOLOR
};

enum class TextureWrapMode {
	WRAP,
	CLAMP
};

enum class PrimitiveType {
	LINE,
	TRIANGLE
};

class OpenGLRenderer {
public:
	void pushModelView();
	void popModelView();
	void setTransformMatrix(TransformMatrix which, const Matrix4x4 &matrix);
	// TODO: This should be split.
	void setRenderState(RenderState state, int value);
	void setBlendFunc(BlendFactor src, BlendFactor dst);
	bool error() const {
		//warning("TODO: Implement error");
		return false;
	}
	Common::String getErrorString() {
		warning("TODO: Implement getErrorString");
		return "";
	}
	// TODO: This just maps to the D3D way to setting textures
	void setTexture(int stage, void *texture);
	void setTexture(int stage, const gTexture &texture);
	void setTextureWrapMode(int index, TextureWrapMode mode);

	void drawPrimitives(PrimitiveType primitiveType, Vertex *vertices, int numPrimitives);
	void drawIndexedPrimitivesVBO(PrimitiveType primitiveType, int VBO, int firstVertex, int numVertices, uint16 *faces, uint32 numFaces);
	void drawIndexedPrimitivesVBO(PrimitiveType primitiveType, Common::SharedPtr<VertexBuffer> VBO, int firstVertex, int numVertices, Common::Array<uint16> faces, uint32 numFaces);
	void drawIndexedPrimitivesVBO(PrimitiveType primitiveType, gBatchBlock &bb);
	bool supportsMultiTexturing() const { // TODO
		return false;
	}
};

extern OpenGLRenderer *g_renderer;

} // End of namespace Watchmaker

#endif // WATCHMAKER_OPENGL_RENDERER_H
