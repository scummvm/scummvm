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

#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/math/vector2.h"
#include "engines/wintermute/dctypes.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"
#include "graphics/transform_struct.h"
#include "math/matrix4.h"
#include "math/ray.h"

namespace Wintermute {

class BaseSurfaceOpenGL3D;

class BaseRenderOpenGL3D : public BaseRenderer {
public:
	BaseRenderOpenGL3D(BaseGame *inGame = nullptr);
	~BaseRenderOpenGL3D() override;

	bool setAmbientLightColor(uint32 color);
	bool setDefaultAmbientLightColor();

	void setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode);

	void dumpData(const char *filename) override {}
	/**
	 * Take a screenshot of the current screenstate
	 *
	 * @return a BaseImage containing the current screen-buffer.
	 */
	BaseImage *takeScreenshot() override;
	bool saveScreenShot(const Common::String &filename, int sizeX = 0, int sizeY = 0) override;
	bool setViewport(int left, int top, int right, int bottom) override;
	bool setViewport(Rect32 *rect) override;
	Rect32 getViewPort() override;
	void setWindowed(bool windowed) override;

	Graphics::PixelFormat getPixelFormat() const override;
	/**
	 * Fade the screen to black
	 *
	 * @param alpha amount to fade by (alpha value of black)
	 */
	void fade(uint16 alpha) override;
	/**
	 * Fade a portion of the screen to a specific color
	 *
	 * @param r the red component to fade too.
	 * @param g the green component to fade too.
	 * @param b the blue component to fade too.
	 * @param a the alpha component to fade too.
	 * @param rect the portion of the screen to fade (if nullptr, the entire screen will be faded).
	 */
	void fadeToColor(byte r, byte g, byte b, byte a) override;

	bool drawLine(int x1, int y1, int x2, int y2, uint32 color) override; 	// Unused outside indicator-display
	bool drawRect(int x1, int y1, int x2, int y2, uint32 color, int width = 1) override; 	// Unused outside indicator-display

	bool setProjection() override;
	bool setProjection2D();
	void resetModelViewTransform();
	void pushWorldTransform(const Math::Matrix4 &transform);
	void popWorldTransform();

	bool windowedBlt() override;
	/**
	 * Fill a portion of the screen with a specified color
	 *
	 * @param r the red component to fill with.
	 * @param g the green component to fill with.
	 * @param b the blue component to fill with.
	 */
	bool fill(byte r, byte g, byte b, Common::Rect *rect = nullptr) override;
	void onWindowChange() override;
	bool initRenderer(int width, int height, bool windowed) override;
	/**
	 * Flip the backbuffer onto the screen-buffer
	 * The screen will NOT be updated before calling this function.
	 *
	 * @return true if successfull, false on error.
	 */
	bool flip() override;
	/**
	 * Special flip for the indicator drawn during save/load
	 * essentially, just copies the region defined by the _indicator-variables.
	 */
	bool indicatorFlip() override;
	bool forcedFlip() override;
	void initLoop() override;
	bool setup2D(bool force = false) override;
	bool setup3D(Camera3D *camera, bool force = false) override;
	bool setupLines() override;

	void project(const Math::Matrix4 &worldMatrix, const Math::Vector3d &point, int &x, int &y);
	Math::Ray rayIntoScene(int x, int y);

	Math::Matrix4 lastProjectionMatrix() {
		return _lastProjectionMatrix;
	}

	/**
	 * Get the name of the current renderer
	 *
	 * @return the name of the renderer.
	 */
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

	/**
	 * Create a Surface fit for use with the renderer.
	 * As diverse implementations of BaseRenderer might have different solutions for storing surfaces
	 * this allows for a common interface for creating surface-handles. (Mostly usefull to ease future
	 * implementation of hw-accelerated rendering, or readding 3D-support at some point).
	 *
	 * @return a surface that can be used with this renderer
	 */
	BaseSurface *createSurface() override;

	bool startSpriteBatch() override {
		return STATUS_OK;
	};
	bool endSpriteBatch() override {
		return STATUS_OK;
	};

	void endSaveLoad() override;

	bool drawSprite(BaseSurfaceOpenGL3D &tex, const Rect32 &rect, float zoomX, float zoomY, const Vector2 &pos,
	                uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY);
	bool drawSpriteEx(BaseSurfaceOpenGL3D &tex, const Rect32 &rect, const Vector2 &pos, const Vector2 &rot, const Vector2 &scale,
	                  float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY);

private:
	Math::Matrix4 _lastProjectionMatrix;
	Math::Matrix4 _lastViewMatrix;
	float _fov;
	bool _spriteBatchMode;
	bool _state3D;
};

} // wintermute namespace

#endif
