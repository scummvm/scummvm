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
	virtual ~TeRenderer() {};

	enum MatrixMode {
		MM_GL_PROJECTION = 0,
		MM_GL_MODELVIEW = 1,
		MM_GL_TEXTURE = 2
	};

	enum ShadowMode {
		ShadowModeNone = 0,
		ShadowModeCreating = 1,
		ShadowModeDrawing = 2
	};

	class TransparentMeshProperties {
	public:
		TransparentMeshProperties() : _camera(nullptr), _vertexCount(0), _shouldDraw(false),
			_scissorEnabled(false), _hasColor(false), _glTexEnvMode(0), _zOrder(0.0f),
			_sourceTransparentMesh(0), _scissorX(0), _scissorY(0),
			_scissorWidth(0), _scissorHeight(0) {}

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

	void addTransparentMesh(const TeMesh &mesh, uint i1, uint i2, uint i3);
	void checkError(const Common::String &str) {};
	virtual void clearBuffer(Buffer buf) = 0;
	virtual void colorMask(bool r, bool g, bool b, bool a) = 0;
	void create();
	TeMatrix4x4 currentMatrix();
	virtual void disableAllLights() = 0;
	virtual void disableTexture() = 0;
	virtual void disableWireFrame() = 0;
	virtual void disableZBuffer() = 0;
	virtual void drawLine(const TeVector3f32 &from, const TeVector3f32 &to) = 0;
	virtual void enableAllLights() = 0;
	virtual void enableTexture() = 0;
	virtual void enableWireFrame() = 0;
	virtual void enableZBuffer() = 0;
	//void extractFrameBufferToImg(const TeVector2s32 &from, const TeVector2s32 &to, TeImage &output);
	virtual void init(uint width, uint height) = 0;
	void loadIdentityMatrix();
	void loadMatrix(const TeMatrix4x4 &matrix);
	void loadCurrentMatrixToGL();
	virtual void loadProjectionMatrix(const TeMatrix4x4 &matrix) = 0;
	void multiplyMatrix(const TeMatrix4x4 &matrix);
	void optimiseTransparentMeshProperties();
	void popMatrix();
	void pushMatrix();
	virtual Common::String renderer() = 0;
	virtual void renderTransparentMeshes() = 0;
	virtual void reset() = 0;
	void rotate(const TeQuaternion &rot);
	void rotate(float angle, float rx, float ry, float rz);
	void scale(float xs, float ys, float zs);
	bool scissorEnabled() const { return _scissorEnabled; }
	int scissorHeight() const { return _scissorHeight; }
	int scissorWidth() const { return _scissorWidth; }
	int scissorX() const { return _scissorX; }
	int scissorY() const { return _scissorY; }
	void sendModelMatrix(const TeMatrix4x4 &matrix) {}
	virtual void setClearColor(const TeColor &col) = 0;
	void setCurrentCamera(TeCamera *camera) {
		_currentCamera = camera;
	}
	virtual void setCurrentColor(const TeColor &col) = 0;
	virtual void setMatrixMode(enum MatrixMode mode) = 0;
	void setScissor(int x, int y, int w, int h);
	void setScissorEnabled(bool val) { _scissorEnabled = val; }
	virtual void setViewport(int x, int y, int w, int h) = 0;
	virtual void shadowMode(enum ShadowMode mode) = 0;
	enum ShadowMode shadowMode() const { return _shadowMode; }
	void translate(float x, float y, float z);
	virtual Common::String vendor() = 0;

	void dumpTransparentMeshProps() const;
	void dumpTransparentMeshData() const;
	const TeColor &currentColor() const { return _currentColor; }

	virtual void updateScreen() = 0;
	virtual void updateGlobalLight() = 0;
	virtual void applyMaterial(const TeMaterial &m) = 0;

	static TeRenderer *makeInstance();

protected:
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

	uint _numTransparentMeshes;
	Common::Array<TeVector3f32> _transparentMeshVertexes;
	Common::Array<TeVector3f32> _transparentMeshNormals;
	Common::Array<TeVector2f32> _transparentMeshCoords;
	Common::Array<TeColor> _transparentMeshColors;
	Common::Array<unsigned short> _transparentMeshVertexNums;

	int _pendingTransparentMeshProperties;
	Common::Array<TransparentMeshProperties> _transparentMeshProps;

	TeMatriciesStack _matriciesStacks[3];  // one per matrix mode.

	virtual void loadMatrixToGL(const TeMatrix4x4 &matrix) = 0;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_RENDERER_H
