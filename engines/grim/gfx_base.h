/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_GFX_BASE_H
#define GRIM_GFX_BASE_H

#include "engines/grim/model.h"
#include "engines/grim/scene.h"

namespace Grim {

struct Shadow;
class SaveGame;
class BitmapData;
class PrimitiveObject;
class Font;
class TextObject;

enum colorFormat {
	BM_RGB565 = 1,    // Grim Fandango
	BM_RGB1555 = 2,   // EMI-PS2
	BM_RGBA = 3      // EMI-PC
};
class GfxBase {
public:
	GfxBase() { ; }
	virtual ~GfxBase() { ; }

	virtual byte *setupScreen(int screenW, int screenH, bool fullscreen) = 0;

	virtual bool isHardwareAccelerated() = 0;

	virtual void setupCamera(float fov, float nclip, float fclip, float roll) = 0;
	virtual void positionCamera(Graphics::Vector3d pos, Graphics::Vector3d interest) = 0;

	virtual void clearScreen() = 0;
	virtual void flipBuffer() = 0;

	virtual void getBoundingBoxPos(const Model::Mesh *model, int *x1, int *y1, int *x2, int *y2) = 0;
	virtual void startActorDraw(Graphics::Vector3d pos, float scale, float yaw, float pitch, float roll) = 0;
	virtual void finishActorDraw() = 0;
	virtual void setShadow(Shadow *shadow) = 0;
	virtual void drawShadowPlanes() = 0;
	virtual void setShadowMode() = 0;
	virtual void clearShadowMode() = 0;
	virtual void setShadowColor(byte r, byte g, byte b) = 0;
	virtual void getShadowColor(byte *r, byte *g, byte *b) = 0;

	virtual void set3DMode() = 0;

	virtual void translateViewpointStart(Graphics::Vector3d pos, float pitch, float yaw, float roll) = 0;
	virtual void translateViewpointFinish() = 0;

	virtual void drawHierachyNode(const Model::HierNode *node) = 0;
	virtual void drawModelFace(const Model::Face *face, float *vertices, float *vertNormals, float *textureVerts) = 0;
	virtual void drawSprite(const Sprite *sprite) = 0;

	virtual void disableLights() = 0;
	virtual void setupLight(Scene::Light *light, int lightId) = 0;

	virtual void createMaterial(Material *material, const char *data, const CMap *cmap) = 0;
	virtual void selectMaterial(const Material *material) = 0;
	virtual void destroyMaterial(Material *material) = 0;

	virtual void createBitmap(BitmapData *bitmap) = 0;
	virtual void drawBitmap(const Bitmap *bitmap) = 0;
	virtual void destroyBitmap(BitmapData *bitmap) = 0;

	virtual void createFont(Font *font) = 0;
	virtual void destroyFont(Font *font) = 0;

	virtual void createTextObject(TextObject *text) = 0;
	virtual void drawTextObject(TextObject *text) = 0;
	virtual void destroyTextObject(TextObject *text) = 0;

	virtual Bitmap *getScreenshot(int w, int h) = 0;
	virtual void storeDisplay() = 0;
	virtual void copyStoredToDisplay() = 0;
	virtual void dimScreen() = 0;
	virtual void dimRegion(int x, int y, int w, int h, float level) = 0;

	virtual void drawEmergString(int x, int y, const char *text, const Color &fgColor) = 0;
	virtual void loadEmergFont() = 0;

	virtual void drawRectangle(PrimitiveObject *primitive) = 0;
	virtual void drawLine(PrimitiveObject *primitive) = 0;
	virtual void drawPolygon(PrimitiveObject *primitive) = 0;

	virtual void prepareSmushFrame(int width, int height, byte *bitmap) = 0;
	virtual void drawSmushFrame(int offsetX, int offsetY) = 0;
	virtual void releaseSmushFrame() = 0;

	virtual const char *getVideoDeviceName() = 0;

	virtual void saveState(SaveGame *state);
	virtual void restoreState(SaveGame *state);

protected:
	int _screenWidth, _screenHeight, _screenBPP;
	bool _isFullscreen;
	Shadow *_currentShadowArray;
	unsigned char _shadowColorR;
	unsigned char _shadowColorG;
	unsigned char _shadowColorB;
};

// Factory-like functions:

GfxBase *CreateGfxOpenGL();
GfxBase *CreateGfxTinyGL();

extern GfxBase *g_driver;

} // end of namespace Grim

#endif
