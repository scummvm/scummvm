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

#ifndef WINTERMUTE_BASE_RENDER_OPENGL3D_H
#define WINTERMUTE_BASE_RENDER_OPENGL3D_H

#include "engines/wintermute/base/gfx/base_renderer3d.h"
#include "engines/wintermute/dctypes.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/math/vector2.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"
#include "graphics/transform_struct.h"
#include "math/matrix4.h"
#include "math/ray.h"

#if defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

#include "graphics/opengl/shader.h"

namespace Wintermute {

class BaseSurfaceOpenGL3D;

class BaseRenderOpenGL3DShader : public BaseRenderer3D {
public:
	BaseRenderOpenGL3DShader(BaseGame *inGame = nullptr);
	~BaseRenderOpenGL3DShader() override;

	void setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode) override;

	void setAmbientLight() override;

	int maximumLightsCount() override;
	void enableLight(int index) override;
	void disableLight(int index) override;
	void setLightParameters(int index, const Math::Vector3d &position, const Math::Vector3d &direction, const Math::Vector4d &diffuse, bool spotlight) override;

	void enableCulling() override;
	void disableCulling() override;

	bool enableShadows() override;
	bool disableShadows() override;
	void displayShadow(BaseObject *object, const Math::Vector3d &lightPos, bool lightPosRelative) override;
	bool stencilSupported() override;

	void dumpData(const char *filename) override {}
	BaseImage *takeScreenshot() override;
	bool saveScreenShot(const Common::String &filename, int sizeX = 0, int sizeY = 0) override;
	void setWindowed(bool windowed) override;
	void fadeToColor(byte r, byte g, byte b, byte a) override;
	bool fill(byte r, byte g, byte b, Common::Rect *rect = nullptr) override;

	bool setViewport(int left, int top, int right, int bottom) override;
	bool drawLine(int x1, int y1, int x2, int y2, uint32 color) override;                // Unused outside indicator-display
	bool drawRect(int x1, int y1, int x2, int y2, uint32 color, int width = 1) override; // Unused outside indicator-display

	bool setProjection() override;
	bool setProjection2D() override;
	void setWorldTransform(const Math::Matrix4 &transform) override;

	bool windowedBlt() override;

	void onWindowChange() override;
	bool initRenderer(int width, int height, bool windowed) override;
	bool flip() override;
	bool indicatorFlip() override;
	bool forcedFlip() override;
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

	bool startSpriteBatch() override {
		return STATUS_OK;
	};
	bool endSpriteBatch() override {
		return STATUS_OK;
	};

	bool drawSpriteEx(BaseSurfaceOpenGL3D &tex, const Rect32 &rect, const Vector2 &pos, const Vector2 &rot, const Vector2 &scale,
	                  float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) override;

	void renderSceneGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
	                         const BaseArray<AdGeneric *> &generics, const BaseArray<Light3D *> &lights, Camera3D *camera) override;
	void renderShadowGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks, const BaseArray<AdGeneric *> &generics, Camera3D *camera) override;

	Mesh3DS *createMesh3DS() override;
	MeshX *createMeshX() override;
	ShadowVolume *createShadowVolume() override;

private:
	Math::Matrix4 _projectionMatrix2d;

	Common::Array<Math::Matrix4> _transformStack;

	float _fov;
	float _nearPlane;
	float _farPlane;
	TRendererState _renderState;
	bool _spriteBatchMode;

	Math::Vector4d _flatShadowColor;
	int _shadowTextureWidth;
	int _shadowTextureHeight;

	GLuint _spriteVBO;
	GLuint _fadeVBO;
	GLuint _lineVBO;
	GLuint _flatShadowMaskVBO;
	GLuint _flatShadowFrameBuffer;
	GLuint _flatShadowRenderTexture;
	GLuint _flatShadowDepthBuffer;
	OpenGL::ShaderGL *_spriteShader;
	OpenGL::ShaderGL *_fadeShader;
	OpenGL::ShaderGL *_modelXShader;
	OpenGL::ShaderGL *_geometryShader;
	OpenGL::ShaderGL *_shadowVolumeShader;
	OpenGL::ShaderGL *_shadowMaskShader;
	OpenGL::ShaderGL *_lineShader;
	OpenGL::ShaderGL *_flatShadowModelXShader;
	OpenGL::ShaderGL *_flatShadowMaskShader;
};

} // namespace Wintermute

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#endif
