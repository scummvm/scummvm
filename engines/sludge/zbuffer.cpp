/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "graphics/pixelformat.h"
#include "graphics/transparent_surface.h"

#include "sludge/allfiles.h"
#include "sludge/zbuffer.h"
#include "sludge/fileset.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/sprites.h"

namespace Sludge {

int zBufferToSet = -1;
zBufferData zBuffer;
extern uint sceneWidth, sceneHeight;
extern Graphics::Surface backdropSurface;
extern Graphics::Surface renderSurface;

void killZBuffer() {
	if (zBuffer.sprites) {
		for (int i = 0; i < zBuffer.numPanels; ++i) {
			zBuffer.sprites[i].free();
		}
		delete []zBuffer.sprites;
		zBuffer.sprites = nullptr;
	}
	zBuffer.numPanels = 0;
	zBuffer.originalNum = 0;
}

void sortZPal(int *oldpal, int *newpal, int size) {
	int i, tmp;

	for (i = 0; i < size; i++) {
		newpal[i] = i;
	}

	if (size < 2)
		return;

	for (i = 1; i < size; i++) {
		if (oldpal[newpal[i]] < oldpal[newpal[i - 1]]) {
			tmp = newpal[i];
			newpal[i] = newpal[i - 1];
			newpal[i - 1] = tmp;
			i = 0;
		}
	}
}

bool setZBuffer(int num) {
	// if the backdrop has not been set yet
	// set zbuffer later
	if (!backdropSurface.getPixels()) {
		zBufferToSet = num;
		return true;
	}

	debug (kSludgeDebugGraphics, "Setting zBuffer");
	uint32 stillToGo = 0;
	int yPalette[16], sorted[16], sortback[16];

	killZBuffer();

	setResourceForFatal(num);

	zBuffer.originalNum = num;
	if (!openFileFromNum(num))
		return false;
	if (bigDataFile->readByte() != 'S')
		return fatal("Not a Z-buffer file");
	if (bigDataFile->readByte() != 'z')
		return fatal("Not a Z-buffer file");
	if (bigDataFile->readByte() != 'b')
		return fatal("Not a Z-buffer file");

	uint width, height;
	switch (bigDataFile->readByte()) {
		case 0:
			width = 640;
			height = 480;
			break;

		case 1:
			width = bigDataFile->readUint16BE();
			height = bigDataFile->readUint16BE();
			break;

		default:
			return fatal("Extended Z-buffer format not supported in this version of the SLUDGE engine");
	}
	if (width != sceneWidth || height != sceneHeight) {
		Common::String tmp = Common::String::format("Z-w: %d Z-h:%d w: %d, h:%d", width, height, sceneWidth, sceneHeight);
		return fatal("Z-buffer width and height don't match scene width and height", tmp);
	}

	zBuffer.numPanels = bigDataFile->readByte();
	for (int y = 0; y < zBuffer.numPanels; y++) {
		yPalette[y] = bigDataFile->readUint16BE();
	}
	sortZPal(yPalette, sorted, zBuffer.numPanels);
	for (int y = 0; y < zBuffer.numPanels; y++) {
		zBuffer.panel[y] = yPalette[sorted[y]];
		sortback[sorted[y]] = y;
	}

	int picWidth = sceneWidth;
	int picHeight = sceneHeight;

	zBuffer.sprites = nullptr;
	zBuffer.sprites = new Graphics::Surface[zBuffer.numPanels];

	for (int i = 0; i < zBuffer.numPanels; ++i) {
		zBuffer.sprites[i].create(picWidth, picHeight, *g_sludge->getScreenPixelFormat());
	}

	for (uint y = 0; y < sceneHeight; y++) {
		for (uint x = 0; x < sceneWidth; x++) {
			int n;
			if (stillToGo == 0) {
				n = bigDataFile->readByte();
				stillToGo = n >> 4;
				if (stillToGo == 15)
					stillToGo = bigDataFile->readUint16BE() + 16l;
				else
					stillToGo++;
				n &= 15;
			}
			for (int i = 0; i < zBuffer.numPanels; ++i) {
				byte *target = (byte *)zBuffer.sprites[i].getBasePtr(x, y);
				if (n && (sortback[i] == n || i == 0)) {
					byte *source = (byte *)backdropSurface.getBasePtr(x, y);
					target[0] = source[0];
					target[1] = source[1];
					target[2] = source[2];
					target[3] = source[3];
				} else {
					target[0] = 0;
					target[1] = 0;
					target[2] = 0;
					target[3] = 0;
				}
			}
			stillToGo--;
		}
	}
	finishAccess();
	setResourceForFatal(-1);
	return true;
}

void drawZBuffer(int x, int y, bool upsidedown) {
	if (!zBuffer.numPanels || !zBuffer.sprites)
		return;

	resetSpriteLayers(&zBuffer, x, y, upsidedown);

#if 0
	glEnable (GL_DEPTH_TEST);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_TRUE);

		glUseProgram(shader.texture);

		setPMVMatrix(shader.texture);

		//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, zBuffer.texName);

		setPrimaryColor(1.0, 1.0, 1.0, 1.0);

		for (i = 1; i < zBuffer.numPanels; i++) {
			GLfloat z = 1.0 - (double) i * (1.0 / 128.0);

			GLfloat vy1 = -y, vy2 = zBuffer.height - y;
			if (upsidedown) {
				vy1 += zBuffer.height;
				vy2 -= zBuffer.height;
			}

			const GLfloat vertices[] = {
				(GLfloat) - x, vy1, z,
				(GLfloat)zBuffer.width - x, vy1, z,
				(GLfloat) - x, vy2, z,
				(GLfloat)zBuffer.width - x, vy2, z
			};

			const GLfloat texCoords[] = {
				0.0f, 0.0f,
				backdropTexW, 0.0f,
				0.0f, backdropTexH,
				backdropTexW, backdropTexH
			};

			glUniform1i(glGetUniformLocation(shader.texture, "zBuffer"), 1);
			glUniform1f(glGetUniformLocation(shader.texture, "zBufferLayer"), i);

			drawQuad(shader.texture, vertices, 1, texCoords);
			glUniform1i(glGetUniformLocation(shader.texture, "zBuffer"), 0);
		}

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_FALSE);
		glDisable(GL_BLEND);
		glUseProgram(0);
#endif
	}

	} // End of namespace Sludge
