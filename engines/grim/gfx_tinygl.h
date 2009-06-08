/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_GFX_TINYGL_H
#define GRIM_GFX_TINYGL_H

#include "engines/grim/gfx_base.h"

#include "graphics/tinygl/zgl.h"

namespace Grim {

class GfxTinyGL : public GfxBase {
public:
	GfxTinyGL();
	virtual ~GfxTinyGL();

	byte *setupScreen(int screenW, int screenH, bool fullscreen);

	const char *getVideoDeviceName();

	void setupCamera(float fov, float nclip, float fclip, float roll);
	void positionCamera(Graphics::Vector3d pos, Graphics::Vector3d interest);

	void clearScreen();
	void flipBuffer();

	bool isHardwareAccelerated();

	void getBoundingBoxPos(const Model::Mesh *model, int *x1, int *y1, int *x2, int *y2);

	void startActorDraw(Graphics::Vector3d pos, float yaw, float pitch, float roll);
	void finishActorDraw();
	void setShadow(Shadow *shadow);
	void drawShadowPlanes();
	void setShadowMode();
	void clearShadowMode();
	void setShadowColor(byte r, byte g, byte b);

	void set3DMode();

	void translateViewpointStart(Graphics::Vector3d pos, float pitch, float yaw, float roll);
	void translateViewpointFinish();

	void drawHierachyNode(const Model::HierNode *node);
	void drawModelFace(const Model::Face *face, float *vertices, float *vertNormals, float *textureVerts);

	void disableLights();
	void setupLight(Scene::Light *light, int lightId);

	void createMaterial(Material *material, const char *data, const CMap *cmap);
	void selectMaterial(const Material *material);
	void destroyMaterial(Material *material);

	void createBitmap(Bitmap *bitmap);
	void drawBitmap(const Bitmap *bitmap);
	void destroyBitmap(Bitmap *bitmap);

	void drawDepthBitmap(int x, int y, int w, int h, char *data);
	void drawBitmap();
	void dimScreen();
	void dimRegion(int x, int y, int w, int h, float level);

	Bitmap *getScreenshot(int w, int h);
	void storeDisplay();
	void copyStoredToDisplay();

	void drawEmergString(int x, int y, const char *text, const Color &fgColor);
	void loadEmergFont();
	TextObjectHandle *createTextBitmap(uint8 *bitmap, int width, int height, const Color &fgColor);
	void drawTextBitmap(int x, int y, TextObjectHandle *handle);
	void destroyTextBitmap(TextObjectHandle *handle);

	void drawRectangle(PrimitiveObject *primitive);
	void drawLine(PrimitiveObject *primitive);
	void drawPolygon(PrimitiveObject *primitive);

	void prepareSmushFrame(int width, int height, byte *bitmap);
	void drawSmushFrame(int offsetX, int offsetY);
	void releaseSmushFrame();

protected:

private:
	TinyGL::ZBuffer *_zb;
	byte *_screen;
	byte *_smushBitmap;
	int _smushWidth;
	int _smushHeight;
	byte *_storedDisplay;
};

} // end of namespace Grim

#endif
