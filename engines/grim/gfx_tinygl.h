/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_GFX_TINYGL_H
#define GRIM_GFX_TINYGL_H

#include "engines/grim/gfx_base.h"
#include "math/quat.h"

#include "graphics/tinygl/zgl.h"

namespace Grim {

class ModelNode;
class Mesh;
class MeshFace;
class BlitImage;

class GfxTinyGL : public GfxBase {
public:
	GfxTinyGL();
	virtual ~GfxTinyGL();

	byte *setupScreen(int screenW, int screenH, bool fullscreen);

	const char *getVideoDeviceName();

	void setupCamera(float fov, float nclip, float fclip, float roll);
	void positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float roll);

	void clearScreen();
	void flipBuffer();

	bool isHardwareAccelerated();

	void getBoundingBoxPos(const Mesh *model, int *x1, int *y1, int *x2, int *y2);

	void startActorDraw(const Math::Vector3d &pos, float scale, const Math::Angle &yaw,
						const Math::Angle &pitch, const Math::Angle &roll, const bool inOverworld,
						const float alpha);
	void finishActorDraw();
	void setShadow(Shadow *shadow);
	void drawShadowPlanes();
	void setShadowMode();
	void clearShadowMode();
	void setShadowColor(byte r, byte g, byte b);
	void getShadowColor(byte *r, byte *g, byte *b);

	void set3DMode();

	void translateViewpointStart();
	void translateViewpoint(const Math::Vector3d &vec);
	void rotateViewpoint(const Math::Angle &angle, const Math::Vector3d &axis);
	void translateViewpointFinish();

	void drawEMIModelFace(const EMIModel* model, const EMIMeshFace* face);
	void drawModelFace(const MeshFace *face, float *vertices, float *vertNormals, float *textureVerts);
	void drawSprite(const Sprite *sprite);

	void enableLights();
	void disableLights();
	void setupLight(Light *light, int lightId);
	void turnOffLight(int lightId);

	void createMaterial(Texture *material, const char *data, const CMap *cmap);
	void selectMaterial(const Texture *material);
	void destroyMaterial(Texture *material);

	void createBitmap(BitmapData *bitmap);
	void drawBitmap(const Bitmap *bitmap, int x, int y);
	void destroyBitmap(BitmapData *bitmap);

	void createFont(Font *font);
	void destroyFont(Font *font);

	void drawTextObject(const TextObject *text);
	void createTextObject(TextObject *text);
	void destroyTextObject(TextObject *text);

	void dimScreen();
	void dimRegion(int x, int y, int w, int h, float level);
	void irisAroundRegion(int x1, int y1, int x2, int y2);

	Bitmap *getScreenshot(int w, int h);
	void storeDisplay();
	void copyStoredToDisplay();

	void drawEmergString(int x, int y, const char *text, const Color &fgColor);
	void loadEmergFont();

	void drawRectangle(const PrimitiveObject *primitive);
	void drawLine(const PrimitiveObject *primitive);
	void drawPolygon(const PrimitiveObject *primitive);

	void prepareMovieFrame(Graphics::Surface* frame);
	void drawMovieFrame(int offsetX, int offsetY);
	void releaseMovieFrame();

	void selectScreenBuffer();
	void selectCleanBuffer();
	void clearCleanBuffer();
	void drawCleanBuffer();

	void createSpecialtyTextures();

protected:

private:
	TinyGL::ZBuffer *_zb;
	Graphics::PixelBuffer _smushBitmap;
	int _smushWidth;
	int _smushHeight;
	Graphics::PixelBuffer _storedDisplay;
	float _alpha;

	Math::Vector3d _currentPos;
	Math::Quaternion _currentQuat;

	void readPixels(int x, int y, int width, int height, uint8 *buffer);
	void blit(const Graphics::PixelFormat &format, BlitImage *blit, byte *dst, byte *src, int x, int y, int width, int height, bool trans);
};

} // end of namespace Grim

#endif
