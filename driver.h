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

#ifndef DRIVER_H
#define DRIVER_H

#include "bits.h"
#include "vector3d.h"
#include "color.h"
#include "model.h"
#include "colormap.h"
#include "bitmap.h"

class Driver {
public:
	Driver() { ; }
	Driver(int screenW, int screenH, int screenBPP) { ; }

	virtual void setupCamera(float fov, float nclip, float fclip, float roll) = NULL;
	virtual void positionCamera(Vector3d pos, Vector3d interest) = NULL;

	virtual void clearScreen() = NULL;
	virtual void flipBuffer() = NULL;

	virtual void startActorDraw(Vector3d pos, float yaw, float pitch, float roll) = NULL;
	virtual void finishActorDraw() = NULL;
	
	virtual void set3DMode() = NULL;

	virtual void drawHierachyNode(const Model::HierNode *node) = NULL;
	virtual void drawModelFace(const Model::Face *face, float *vertices, float *vertNormals, float *textureVerts) = NULL;

	virtual void createMaterial(Material *material, const char *data, const CMap *cmap) = NULL;
	virtual void selectMaterial(const Material *material) = NULL;
	virtual void destroyMaterial(Material *material) = NULL;

	virtual void createBitmap(Bitmap *bitmap) = NULL;
	virtual void drawBitmap(const Bitmap *bitmap) = NULL;
	virtual void destroyBitmap(Bitmap *bitmap) = NULL;

	virtual void drawDepthBitmap(int x, int y, int w, int h, char *data) = NULL;

	virtual void drawEmergString(int x, int y, const char *text, const Color &fgColor) = NULL;
	virtual void loadEmergFont() = NULL;

	virtual void prepareSmushFrame(int width, int height, byte *bitmap) = NULL;
	virtual void drawSmushFrame(int offsetX, int offsetY) = NULL;
};

extern Driver *g_driver;

#endif
