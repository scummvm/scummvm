// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
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

// Driver:
#include <SDL.h>
#include <SDL_opengl.h>

// Residual:
#include "bits.h"
#include "vector3d.h"
#include "color.h"

#define BITMAP_TEXTURE_SIZE 256

class Driver {
	public:
		Driver(int screenW, int screenH, int screenBPP);

		void setupCamera(float fov, float nclip, float fclip, float roll);
		void positionCamera(Vector3d pos, Vector3d interest);

		void clearScreen(); 
		void flipBuffer();

		void startActorDraw(Vector3d pos, float yaw, float pitch, float roll);
		void finishActorDraw();

		void drawDepthBitmap(int num, int x, int y, int w, int h, char **data);
		void drawBitmap();

		void drawHackFont(int x, int y, const char *text, Color &fgColor);

		void prepareSmushFrame(int width, int height, byte *bitmap);
		void drawSmushFrame(int offsetX, int offsetY);

	private:
		GLuint hackFont;  // FIXME: Temporary font drawing hack
		int _smushNumTex;
		GLuint *_smushTexIds;
		int _smushWidth;
		int _smushHeight;
};

extern Driver *g_driver;

