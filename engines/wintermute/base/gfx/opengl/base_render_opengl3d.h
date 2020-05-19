#ifndef WINTERMUTE_BASE_RENDER_OPENGL3D_H
#define WINTERMUTE_BASE_RENDER_OPENGL3D_H

#include "../base_renderer.h"
#include "../../../math/rect32.h"
#include "../../../math/vector2.h"
#include "../../../dctypes.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"
#include "graphics/transform_struct.h"

namespace Wintermute {

class BaseRenderOpenGL3D : public BaseRenderer {
public:
	BaseRenderOpenGL3D(BaseGame *inGame = nullptr);
	~BaseRenderOpenGL3D() override;

	virtual void dumpData(const char *filename) {}
	/**
	 * Take a screenshot of the current screenstate
	 *
	 * @return a BaseImage containing the current screen-buffer.
	 */
	virtual BaseImage *takeScreenshot();
	virtual bool saveScreenShot(const Common::String &filename, int sizeX = 0, int sizeY = 0);
	virtual bool setViewport(int left, int top, int right, int bottom);
	virtual bool setViewport(Rect32 *rect);
	virtual Rect32 getViewPort();
	virtual bool setScreenViewport();
	virtual void setWindowed(bool windowed);

	virtual Graphics::PixelFormat getPixelFormat() const;
	/**
	 * Fade the screen to black
	 *
	 * @param alpha amount to fade by (alpha value of black)
	 */
	virtual void fade(uint16 alpha);
	/**
	 * Fade a portion of the screen to a specific color
	 *
	 * @param r the red component to fade too.
	 * @param g the green component to fade too.
	 * @param b the blue component to fade too.
	 * @param a the alpha component to fade too.
	 * @param rect the portion of the screen to fade (if nullptr, the entire screen will be faded).
	 */
	virtual void fadeToColor(byte r, byte g, byte b, byte a);

	virtual bool drawLine(int x1, int y1, int x2, int y2, uint32 color); 	// Unused outside indicator-display
	virtual bool drawRect(int x1, int y1, int x2, int y2, uint32 color, int width = 1); 	// Unused outside indicator-display

	virtual bool setProjection();

	virtual bool windowedBlt();
	/**
	 * Fill a portion of the screen with a specified color
	 *
	 * @param r the red component to fill with.
	 * @param g the green component to fill with.
	 * @param b the blue component to fill with.
	 */
	virtual bool fill(byte r, byte g, byte b, Common::Rect *rect = nullptr);
	virtual void onWindowChange();
	virtual bool initRenderer(int width, int height, bool windowed);
	/**
	 * Flip the backbuffer onto the screen-buffer
	 * The screen will NOT be updated before calling this function.
	 *
	 * @return true if successfull, false on error.
	 */
	virtual bool flip();
	/**
	 * Special flip for the indicator drawn during save/load
	 * essentially, just copies the region defined by the _indicator-variables.
	 */
	virtual bool indicatorFlip();
	virtual bool forcedFlip();
	virtual void initLoop();
	virtual bool setup2D(bool force = false);
	virtual bool setup3D(bool force = false);
	virtual bool setupLines();

	/**
	 * Get the name of the current renderer
	 *
	 * @return the name of the renderer.
	 */
	virtual Common::String getName() const {
		return "OpenGL 3D renderer";
	};
	virtual bool displayDebugInfo() {
		return STATUS_FAILED;
	};
	virtual bool drawShaderQuad() {
		return STATUS_FAILED;
	}

	virtual float getScaleRatioX() const {
		return 1.0f;
	}
	virtual float getScaleRatioY() const {
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
	virtual BaseSurface *createSurface();

	virtual bool startSpriteBatch() {
		return STATUS_OK;
	};
	virtual bool endSpriteBatch() {
		return STATUS_OK;
	};

	virtual void endSaveLoad();

	bool drawSprite(const OpenGL::Texture& tex, const Rect32& rect, float zoomX, float zoomY, const Vector2& pos,
					float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY);
	bool drawSpriteEx(const OpenGL::Texture& tex, const Rect32& rect, const Vector2& pos, const Vector2& rot, const Vector2& scale,
					  float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY);

private:
	bool _spriteBatchMode;
};

} // wintermute namespace

#endif
