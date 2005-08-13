// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef DRIVER_GL_H
#define DRIVER_GL_H

#include "bits.h"
#include "vector3d.h"
#include "color.h"
#include "model.h"
#include "colormap.h"
#include "bitmap.h"
#include "driver.h"

#include <SDL.h>
#include <SDL_opengl.h>

class DriverGL : public Driver {
public:
	DriverGL(int screenW, int screenH, int screenBPP, bool fullscreen = false);

	void setupCamera(float fov, float nclip, float fclip, float roll);
	void positionCamera(Vector3d pos, Vector3d interest);

	void toggleFullscreenMode();

	void clearScreen(); 
	void flipBuffer();

	bool isHardwareAccelerated();

	void startActorDraw(Vector3d pos, float yaw, float pitch, float roll);
	void finishActorDraw();
	
	void set3DMode();

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

	Bitmap *getScreenshot(int w, int h);
	void storeDisplay() {}
	void flushStoredDisplay() {}
	void enableDim(int /*x*/, int /*y*/, int /*w*/, int /*h*/) { _dim = true; }
	void disableDim(int /*x*/, int /*y*/, int /*w*/, int /*h*/) { _dim = false; }

	void drawEmergString(int x, int y, const char *text, const Color &fgColor);
	void loadEmergFont();
	TextObjectHandle *createTextBitmap(uint8 *bitmap, int width, int height, const Color &fgColor);
	void drawTextBitmap(int x, int y, TextObjectHandle *handle);
	void destroyTextBitmap(TextObjectHandle *handle);

	void drawRectangle(PrimitiveObject *primitive);
	void drawLine(PrimitiveObject *primitive);

	void prepareSmushFrame(int width, int height, byte *bitmap);
	void drawSmushFrame(int offsetX, int offsetY);

protected:
	void drawDim();

private:
	GLuint _emergFont;
	int _smushNumTex;
	GLuint *_smushTexIds;
	int _smushWidth;
	int _smushHeight;
};

#endif
