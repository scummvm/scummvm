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

#ifndef TETRAEDGE_TE_TE_RENDERER_H
#define TETRAEDGE_TE_TE_RENDERER_H

#include "common/ptr.h"
#include "tetraedge/te/te_mesh.h"
#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_camera.h"
#include "tetraedge/te/te_matricies_stack.h"

namespace Tetraedge {

class TeRenderer {
public:
	TeRenderer();

	enum MatrixMode {
		MM_GL_PROJECTION = 0,
		MM_GL_MODELVIEW = 1,
		MM_GL_TEXTURE = 2
	};

	enum ShadowMode {
		ShadowMode0 = 0,
		ShadowMode1 = 1,
		ShadowMode2 = 2
	};

	class TransparentMeshProperties {
	public:
		TransparentMeshProperties() : _camera(nullptr), _vertexCount(0), _shouldDraw(false), _scissorEnabled(false), _hasColor(false) {}
		TeCamera *_camera;
		int _vertexCount;
		TeMatrix4x4 _matrix;

		TeMaterial _material;

		uint32 _glTexEnvMode;
		uint _sourceTransparentMesh;
		bool _hasColor;
		float _zOrder;
		bool _scissorEnabled;
		int _scissorX;
		int _scissorY;
		int _scissorWidth;
		int _scissorHeight;
		bool _shouldDraw;
	};

	enum Buffer {
		DepthBuffer = 1,
		ColorBuffer = 2,
		StencilBuffer = 4,
		ColorAndDepth = DepthBuffer | ColorBuffer,
		AllBuffers = DepthBuffer | ColorBuffer | StencilBuffer
	};

	void addTransparentMesh(const TeMesh &mesh, unsigned long i1, unsigned long i2, unsigned long i3);
	void checkError(const Common::String &str) {};
	void clearBuffer(Buffer buf);
	void create();
	TeMatrix4x4 currentMatrix();
	void disableTexture();
	void disableWireFrame();
	void disableZBuffer();
	void drawLine(const TeVector3f32 &from, const TeVector3f32 &to);
	void enableTexture();
	void enableWireFrame();
	void enableZBuffer();
	//void extractFrameBufferToImg(const TeVector2s32 &from, const TeVector2s32 &to, TeImage &output);
	void init();
	void loadIdentityMatrix();
	void loadMatrix(const TeMatrix4x4 &matrix);
	void loadCurrentMatrixToGL();
	void loadProjectionMatrix(const TeMatrix4x4 &matrix);
	void multiplyMatrix(const TeMatrix4x4 &matrix);
	void optimiseTransparentMeshProperties();
	void popMatrix();
	void pushMatrix();
	Common::String renderer();
	void renderTransparentMeshes();
	void reset();
	void rotate(const TeQuaternion &rot);
	void rotate(float angle, float rx, float ry, float rz);
	void scale(float xs, float ys, float zs);
	bool scissorEnabled() const { return _scissorEnabled; }
	int scissorHeight() const { return _scissorHeight; }
	int scissorWidth() const { return _scissorWidth; }
	int scissorX() const { return _scissorX; }
	int scissorY() const { return _scissorY; }
	void sendModelMatrix(const TeMatrix4x4 &matrix) {}
	void setClearColor(const TeColor &col);
	void setCurrentCamera(TeCamera *camera) {
		_currentCamera = camera;
	}
	void setCurrentColor(const TeColor &col);
	void setMatrixMode(enum MatrixMode mode);
	void setScissor(int x, int y, int w, int h);
	void setScissorEnabled(bool val) { _scissorEnabled = val; }
	void setViewport(int x, int y, int w, int h);
	void shadowMode(enum ShadowMode mode);
	enum ShadowMode shadowMode() const { return _shadowMode; }
	void translate(float x, float y, float z);
	Common::String vendor();

	void dumpTransparentMeshProps() const;
	void dumpTransparentMeshData() const;
	const TeColor &currentColor() const { return _currentColor; }

private:
	TeCamera *_currentCamera;
	TeColor _currentColor;
	TeColor _clearColor;
	bool _textureEnabled;

	ShadowMode _shadowMode;
	MatrixMode _matrixMode;

	bool _scissorEnabled;
	int _scissorHeight;
	int _scissorWidth;
	int _scissorX;
	int _scissorY;

	long _numTransparentMeshes;
	Common::Array<TeVector3f32> _transparentMeshVertexes;
	Common::Array<TeVector3f32> _transparentMeshNormals;
	Common::Array<TeVector2f32> _transparentMeshCoords;
	Common::Array<TeColor> _transparentMeshColors;
	Common::Array<unsigned short> _transparentMeshVertexNums;

	int _pendingTransparentMeshProperties;
	Common::Array<TransparentMeshProperties> _transparentMeshProps;

	TeMatriciesStack _matriciesStacks[3];  // one per matrix mode.

	void loadMatrixToGL(const TeMatrix4x4 &matrix);
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_RENDERER_H
