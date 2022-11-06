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

#include "watchmaker/3d/render/opengl_renderer.h"
#include "watchmaker/t3d.h"

#if defined(USE_OPENGL_GAME)

#include "graphics/opengl/system_headers.h"
#include "math/glmath.h"

#include "math/glmath.h"

namespace Watchmaker {

OpenGLRenderer *g_renderer = nullptr;

void OpenGLRenderer::drawIndexedPrimitivesVBO(PrimitiveType primitiveType, Common::SharedPtr<VertexBuffer> VBO, int firstVertex, int numVertices, Common::Array<uint16> faces, uint32 numFaces) {
	assert(numFaces <= faces.size());

	assert(primitiveType == PrimitiveType::TRIANGLE);

	float fNearPlane = 1.0f;//5000.0f;
	float fFarPlane = 15000.0f;
	float width = 1024;
	float height = 768;

	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	float fAspect = 60.0f;//curCamera->FovRad;
	auto perspectiveMatrix = Math::makePerspectiveMatrix(fAspect, width / height, fNearPlane, fFarPlane);
	glLoadMatrixf(perspectiveMatrix.getData());

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glEnable(GL_TEXTURE_2D);

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < numFaces; i++) {
		int index = faces[i];
		assert(index <= VBO->_buffer.size());
		auto &vertex = VBO->_buffer[index];
		//warning("%d/%d %d: [%f, %f, %f], [%f, %f], [%f, %f]", i, numFaces, index, vertex.x, vertex.y, vertex.z, vertex.u1, vertex.v1, vertex.u2, vertex.v2);
		//glColor3f((float)i/numFaces, 1.0, 0.0);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(vertex.u1, vertex.v1);
		glVertex3f(vertex.x, vertex.y, -vertex.z);
	}
	glEnd();

	glFlush();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void OpenGLRenderer::drawPrimitives(PrimitiveType primitiveType, Vertex *vertices, int numPrimitives) {
	assert(primitiveType == PrimitiveType::TRIANGLE);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < numPrimitives; i++) {
		auto &vertex = vertices[i];
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(vertex.sx, vertex.sy, -vertex.sz);
	}
	glEnd();
	glFlush();
}
void OpenGLRenderer::drawIndexedPrimitivesVBO(PrimitiveType primitiveType, int VBO, int firstVertex, int numVertices, uint16 *faces, uint32 numFaces) {
	//warning("TODO: Implement drawIndexedPrimitivesVBO");
}


void OpenGLRenderer::drawIndexedPrimitivesVBO(PrimitiveType primitiveType, gBatchBlock &bb) {
	drawIndexedPrimitivesVBO(primitiveType,
	                         bb.VBO, 0, bb.NumVerts(),
	                         bb.FacesList,
	                         bb.NumFaces()/*, 0x0*/
	                        );
}

void OpenGLRenderer::setTransformMatrix(TransformMatrix which, const Matrix4x4 &matrix) {
	GLint oldMatrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(matrix.data);
	glScalef(1.0f, 1.0f, -1.0f);
	glMatrixMode(oldMatrixMode);
};

void OpenGLRenderer::pushModelView() {
	GLint oldMatrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMatrixMode(oldMatrixMode);
}

void OpenGLRenderer::popModelView() {
	GLint oldMatrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(oldMatrixMode);
}

void OpenGLRenderer::setTextureWrapMode(int index, TextureWrapMode mode) {
	GLint openGlWrapMode = 0;
	switch (mode) {
	case TextureWrapMode::WRAP:
		openGlWrapMode = GL_REPEAT;
		break;
	case TextureWrapMode::CLAMP:
		openGlWrapMode = GL_CLAMP;
		break;
	default:
		assert(0);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, openGlWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, openGlWrapMode);
}

void OpenGLRenderer::setTexture(int stage, void *texture) {
	if (texture == nullptr) {
		glBindTexture(GL_TEXTURE_2D, 0);
	} else {
		assert(0);
	}
}
void OpenGLRenderer::setTexture(int stage, const gTexture &texture) {
	texture._texture->bind();
}

void setGlFeature(GLint feature, bool state) {
	if (state) {
		glEnable(feature);
	} else {
		glDisable(feature);
	}
}

void OpenGLRenderer::setRenderState(RenderState state, int value) {
	switch (state) {
	case RenderState::ZENABLE: {
		glDepthFunc(GL_LEQUAL);
		setGlFeature(GL_DEPTH_TEST, value);
		break;
	}
	case RenderState::ALPHAREF: { // ALPHA-func is never changed.
		glAlphaFunc(GL_ALWAYS, value);
	}
	case RenderState::ALPHABLEND:
		setGlFeature(GL_BLEND, value);
		break; // TODO

	}
	//warning("TODO: Implement setRenderState");
}

GLenum translateBlendFactorToGL(BlendFactor factor) {
	switch (factor) {
	case BlendFactor::ONE:
		return GL_ONE;
	case BlendFactor::ZERO:
		return GL_ZERO;
	case BlendFactor::SRCALPHA:
		return GL_SRC_ALPHA;
	case BlendFactor::INVSRCALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case BlendFactor::INVSRCCOLOR:
		return GL_ONE_MINUS_SRC_COLOR;
	case BlendFactor::SRCCOLOR:
		return GL_SRC_COLOR;
	case BlendFactor::DESTCOLOR:
		return GL_DST_COLOR;
	case BlendFactor::INVDESTCOLOR:
		return GL_ONE_MINUS_DST_COLOR;
	default:
		assert(false);
	}
}

void OpenGLRenderer::setBlendFunc(BlendFactor src, BlendFactor dst) {
	glBlendFunc(translateBlendFactorToGL(src), translateBlendFactorToGL(dst));
}

} // End of namespace Watchmaker

#endif // USE_OPENGL_GAME
