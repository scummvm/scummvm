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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRIM_GFX_BASE_H
#define GRIM_GFX_BASE_H

#include "math/vector3d.h"
#include "math/quat.h"

#include "graphics/pixelformat.h"

#include "engines/grim/material.h"

namespace Graphics {
	struct Surface;
}

namespace Grim {

struct Shadow;
class SaveGame;
class BitmapData;
class Bitmap;
class CMap;
class Color;
class PrimitiveObject;
class Font;
class TextObject;
class Material;
class EMIModel;
class EMIMeshFace;
class ModelNode;
class Mesh;
class MeshFace;
class Sprite;
class Light;
class Texture;

class SpecialtyMaterial : public Material {
public:
	SpecialtyMaterial() { _texture = NULL; }
	~SpecialtyMaterial() { delete _texture; }
	void create(const char *data, int width, int height);
	virtual void select() const;
	Texture *_texture;
};

/**
 * The Color-formats used for bitmaps in Grim Fandango/Escape From Monkey Island
 */
enum colorFormat {
	BM_RGB565 = 1,    // Grim Fandango
	BM_RGB1555 = 2,   // EMI-PS2
	BM_RGBA = 3,      // EMI-PC (Also internal Material-format for Grim)
	BM_BGR888 = 4,    // EMI-TGA-materials (888)
	BM_BGRA = 5       // EMI-TGA-materials with alpha
};
class GfxBase {
public:
	GfxBase();
	virtual ~GfxBase() { ; }

	/**
	 * Creates a render-context.
	 *
	 * @param screenW       the width of the context
	 * @param screenH       the height of the context
	 * @param fullscreen    true if fullscreen is desired, false otherwise.
	 */
	virtual byte *setupScreen(int screenW, int screenH, bool fullscreen) = 0;

	/**
	 * Query whether the current context is hardware-accelerated
	 *
	 * @return true if hw-accelerated, false otherwise
	 */
	virtual bool isHardwareAccelerated() = 0;
	/**
	 * Query whether the current context is fullscreen.
	 *
	 * @return true if fullscreen, false otherwise
	 */
	virtual bool isFullscreen() { return _isFullscreen; }
	virtual uint getScreenWidth() { return _screenWidth; }
	virtual uint getScreenHeight() { return _screenHeight; }

	virtual void setupCamera(float fov, float nclip, float fclip, float roll) = 0;
	virtual void positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float roll) = 0;

	virtual void clearScreen() = 0;

	/**
	 *  Swap the buffers, making the drawn screen visible
	 */
	virtual void flipBuffer() = 0;

	virtual void getBoundingBoxPos(const Mesh *mesh, int *x1, int *y1, int *x2, int *y2) = 0;
	virtual void startActorDraw(const Math::Vector3d &pos, float scale, const Math::Quaternion &quat,
								const bool inOverworld, const float alpha, const bool depthOnly) = 0;

	virtual void finishActorDraw() = 0;
	virtual void setShadow(Shadow *shadow) = 0;
	virtual void drawShadowPlanes() = 0;
	virtual void setShadowMode();
	virtual void clearShadowMode();
	bool isShadowModeActive();
	virtual void setShadowColor(byte r, byte g, byte b) = 0;
	virtual void getShadowColor(byte *r, byte *g, byte *b) = 0;

	virtual void set3DMode() = 0;

	virtual void translateViewpointStart() = 0;
	virtual void translateViewpoint(const Math::Vector3d &vec) = 0;
	virtual void rotateViewpoint(const Math::Angle &angle, const Math::Vector3d &axis) = 0;
	virtual void translateViewpointFinish() = 0;

	virtual void drawEMIModelFace(const EMIModel *model, const EMIMeshFace *face) = 0;
	virtual void drawModelFace(const Mesh *mesh, const MeshFace *face) = 0;
	virtual void drawSprite(const Sprite *sprite) = 0;
	virtual void drawMesh(const Mesh *mesh);

	virtual void enableLights() = 0;
	virtual void disableLights() = 0;
	virtual void setupLight(Light *light, int lightId) = 0;
	virtual void turnOffLight(int lightId) = 0;

	virtual void createMaterial(Texture *material, const char *data, const CMap *cmap) = 0;
	virtual void selectMaterial(const Texture *material) = 0;
	virtual void destroyMaterial(Texture *material) = 0;

	/**
	 * Prepares a bitmap for drawing
	 * performs any format conversions needed for the renderer,
	 * and might create an internal representation of the bitmap
	 * external changes to the bitmap may not be visible after this
	 * is called. Must be called before drawBitmap can be used.
	 *
	 * the external bitmap might have its data changed by this function,
	 *
	 * @param bitmap    the bitmap to be prepared
	 * @see destroyBitmap
	 * @see drawBitmap
	 */
	virtual void createBitmap(BitmapData *bitmap) = 0;

	/**
	 * Draws a bitmap
	 * before this is safe to use, createBitmap MUST have been called
	 *
	 * @param bitmap    the bitmap to be drawn
	 * @see createBitmap
	 * @see destroyBitmap
	 */
	virtual void drawBitmap(const Bitmap *bitmap, int x, int y, uint32 layer = 0) = 0;

	/**
	 * Deletes any internal references and representations of a bitmap
	 * after this is called, it is safe to dispose of or change the external
	 * bitmapdata.
	 *
	 * @param bitmap    the bitmap to be destroyed
	 * @see createBitmap
	 * @see drawBitmap
	 */
	virtual void destroyBitmap(BitmapData *bitmap) = 0;

	virtual void createFont(Font *font) = 0;
	virtual void destroyFont(Font *font) = 0;

	virtual void createTextObject(TextObject *text) = 0;
	virtual void drawTextObject(const TextObject *text) = 0;
	virtual void destroyTextObject(TextObject *text) = 0;

	virtual Bitmap *getScreenshot(int w, int h) = 0;
	virtual void storeDisplay() = 0;
	virtual void copyStoredToDisplay() = 0;

	/**
	 * Dims the entire screen
	 * Sets the entire screen to 10% of its current brightness,
	 * and converts it to grayscale.
	 */
	virtual void dimScreen() = 0;
	virtual void dimRegion(int x, int y, int w, int h, float level) = 0;
	virtual void setDimLevel(float dimLevel) { _dimLevel = dimLevel; }

	/**
	 * Draw a completely opaque Iris around the specified rectangle.
	 * the arguments specify the distance from the screen-edge to the first
	 * non-iris pixel.
	 *
	 * @param x     the width of the Iris
	 * @param y     the height of the Iris
	 */
	virtual void irisAroundRegion(int x1, int y1, int x2, int y2) = 0;

	virtual void drawEmergString(int x, int y, const char *text, const Color &fgColor) = 0;
	virtual void loadEmergFont() = 0;

	virtual void drawRectangle(const PrimitiveObject *primitive) = 0;
	virtual void drawLine(const PrimitiveObject *primitive) = 0;
	virtual void drawPolygon(const PrimitiveObject *primitive) = 0;

	/**
	 * Prepare a movie-frame for drawing
	 * performing any necessary conversion
	 *
	 * @param width         the width of the movie-frame.
	 * @param height        the height of the movie-frame.
	 * @param bitmap        a pointer to the data for the movie-frame.
	 * @see drawMovieFrame
	 * @see releaseMovieFrame
	 */
	virtual void prepareMovieFrame(Graphics::Surface *frame) = 0;
	virtual void drawMovieFrame(int offsetX, int offsetY) = 0;

	/**
	 * Release the currently prepared movie-frame, if one exists.
	 *
	 * @see drawMovieFrame
	 * @see prepareMovieFrame
	 */
	virtual void releaseMovieFrame() = 0;

	virtual const char *getVideoDeviceName() = 0;

	virtual void saveState(SaveGame *state);
	virtual void restoreState(SaveGame *state);

	void renderBitmaps(bool render);
	void renderZBitmaps(bool render);

	virtual void createSpecialtyTextures() = 0;
	virtual Material *getSpecialtyTexture(int n) { return &_specialty[n]; }

	virtual void createModel(Mesh *mesh) {}
	virtual void createEMIModel(EMIModel *model) {}
	virtual void updateEMIModel(const EMIModel *model) {}

	virtual int genBuffer() { return 0; }
	virtual void delBuffer(int buffer) {}
	virtual void selectBuffer(int buffer) {}
	virtual void clearBuffer(int buffer) {}
	virtual void drawBuffers() {}
	virtual void refreshBuffers() {}

protected:
	static const int _gameHeight = 480;
	static const int _gameWidth = 640;
	float _scaleW, _scaleH;
	int _screenWidth, _screenHeight, _screenSize;
	bool _isFullscreen;
	Shadow *_currentShadowArray;
	unsigned char _shadowColorR;
	unsigned char _shadowColorG;
	unsigned char _shadowColorB;
	bool _renderBitmaps;
	bool _renderZBitmaps;
	bool _shadowModeActive;
	Graphics::PixelFormat _pixelFormat;
	SpecialtyMaterial _specialty[8];
	Math::Vector3d _currentPos;
	Math::Quaternion _currentQuat;
	float _dimLevel;
};

// Factory-like functions:

GfxBase *CreateGfxOpenGL();
GfxBase *CreateGfxTinyGL();

extern GfxBase *g_driver;

} // end of namespace Grim

#endif
