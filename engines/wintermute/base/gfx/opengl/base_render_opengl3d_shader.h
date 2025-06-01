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

#ifndef WINTERMUTE_BASE_RENDER_OPENGL3D_H
#define WINTERMUTE_BASE_RENDER_OPENGL3D_H

#include "engines/wintermute/base/gfx/base_renderer3d.h"
#include "engines/wintermute/dctypes.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/math/vector2.h"

#include "graphics/opengl/system_headers.h"
#include "graphics/transform_struct.h"

#if defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/shader.h"

namespace Wintermute {

class BaseSurfaceOpenGL3D;

class BaseRenderOpenGL3DShader : public BaseRenderer3D {
	friend class BaseSurfaceOpenGL3DShader;
	friend class Mesh3DSOpenGLShader;
	friend class XMeshOpenGLShader;
	friend class ShadowVolumeOpenGLShader;

	struct SpriteVertex {
		float x;
		float y;
		float z;
		float u;
		float v;
		float r;
		float g;
		float b;
		float a;
	};

	struct LineVertex {
		float x;
		float y;
		float z;
	};

	struct SimpleShadowVertex {
		float x;
		float y;
		float z;
		float nx;
		float ny;
		float nz;
		float u;
		float v;
	};


public:
	BaseRenderOpenGL3DShader(BaseGame *inGame = nullptr);
	~BaseRenderOpenGL3DShader() override;

	bool invalidateTexture(BaseSurfaceOpenGL3D *texture) override;

	bool invalidateDeviceObjects() override;
	bool restoreDeviceObjects() override;

	bool resetDevice() override;

	void setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode, bool forceChange = false) override;

	void setAmbientLightRenderState() override;

	int getMaxActiveLights() override;
	void lightEnable(int index, bool enable) override;
	void setLightParameters(int index, const DXVector3 &position, const DXVector3 &direction, const DXVector4 &diffuse, bool spotlight) override;

	void enableCulling() override;
	void disableCulling() override;

	bool enableShadows() override;
	bool disableShadows() override;
	bool stencilSupported() override;

	BaseImage *takeScreenshot(int newWidth = 0, int newHeight = 0) override;
	void fadeToColor(byte r, byte g, byte b, byte a) override;
	bool flip() override;
	bool fill(byte r, byte g, byte b, Common::Rect *rect = nullptr) override;

	bool setViewport(int left, int top, int right, int bottom) override;
	bool drawLine(int x1, int y1, int x2, int y2, uint32 color) override;

	DXMatrix *buildMatrix(DXMatrix* out, const DXVector2 *centre, const DXVector2 *scaling, float angle);
	void transformVertices(struct SpriteVertex *vertices, const DXVector2 *centre, const DXVector2 *scaling, float angle);

	bool setProjection() override;
	bool setProjection2D(OpenGL::Shader *);
	bool setWorldTransform(const DXMatrix &transform) override;
	bool setViewTransform(const DXMatrix &transform) override;
	bool setProjectionTransform(const DXMatrix &transform) override;

	bool initRenderer(int width, int height, bool windowed) override;
	bool setup2D(bool force = false) override;
	bool setup3D(Camera3D *camera, bool force = false) override;
	bool setupLines() override;

	Common::String getName() const override {
		return "OpenGL 3D renderer";
	};
	bool displayDebugInfo() override {
		return STATUS_FAILED;
	};
	bool drawShaderQuad() override {
		return STATUS_FAILED;
	}

	float getScaleRatioX() const override {
		return 1.0f;
	}
	float getScaleRatioY() const override {
		return 1.0f;
	}

	BaseSurface *createSurface() override;

	bool startSpriteBatch() override;
	bool endSpriteBatch() override;
	bool commitSpriteBatch() override;

	bool drawSpriteEx(BaseSurface *texture, const Rect32 &rect, const Vector2 &pos, const Vector2 &rot, const Vector2 &scale,
	                  float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) override;

	void renderSceneGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
	                         const BaseArray<AdGeneric *> &generics, const BaseArray<Light3D *> &lights, Camera3D *camera) override;
	void renderShadowGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks, const BaseArray<AdGeneric *> &generics, Camera3D *camera) override;

	Mesh3DS *createMesh3DS() override;
	XMesh *createXMesh() override;
	ShadowVolume *createShadowVolume() override;

	bool setViewport3D(DXViewport *viewport) override;

	void postfilter() override;
	void setPostfilter(PostFilter postFilter) override { _postFilterMode = postFilter; };

	OpenGL::Shader *_shadowMaskShader;

private:
	void displaySimpleShadow(BaseObject *object) override;

	SimpleShadowVertex _simpleShadow[4];

	DXMatrix _glProjectionMatrix;
	float _alphaRef;

	Common::Array<DXMatrix> _transformStack;

	Math::Vector4d _flatShadowColor;

	GLuint _spriteVBO{};
	GLuint _fadeVBO{};
	GLuint _lineVBO{};
	GLuint _simpleShadowVBO{};
	GLuint _postfilterVBO{};
	OpenGL::Shader *_spriteShader{};
	OpenGL::Shader *_fadeShader{};
	OpenGL::Shader *_xmodelShader{};
	OpenGL::Shader *_geometryShader{};
	OpenGL::Shader *_simpleShadowShader{};
	OpenGL::Shader *_flatShadowShader{};
	OpenGL::Shader *_shadowVolumeShader{};
	OpenGL::Shader *_lineShader{};
	OpenGL::Shader *_postfilterShader{};
	GLuint _postfilterTexture;
};

} // namespace Wintermute

#endif // defined(USE_OPENGL_SHADERS)

#endif
