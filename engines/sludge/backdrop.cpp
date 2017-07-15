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
#include "common/rect.h"
#include "image/png.h"
#include "graphics/surface.h"
#include "graphics/transparent_surface.h"
#include "graphics/palette.h"

#include "sludge/allfiles.h"
#include "sludge/newfatal.h"
#include "sludge/fileset.h"
#include "sludge/backdrop.h"
#include "sludge/moreio.h"
#include "sludge/statusba.h"
#include "sludge/talk.h"
#include "sludge/zbuffer.h"
#include "sludge/graphics.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/variable.h"
#include "sludge/imgloader.h"

namespace Sludge {

extern inputType input;
extern Graphics::Surface renderSurface;

bool freeze();
void unfreeze(bool);    // Because FREEZE.H needs a load of other includes

#if 0
GLubyte *backdropTexture = NULL;
GLuint backdropTextureName = 0;
GLfloat backdropTexW = 1.0;
GLfloat backdropTexH = 1.0;

GLuint snapshotTextureName = 0;
#endif

bool backdropExists = false;
extern int zBufferToSet;

Graphics::Surface lightMap;
Graphics::Surface backdropSurface;
Graphics::Surface snapshotSurface;

float snapTexW = 1.0;
float snapTexH = 1.0;

int lightMapMode = LIGHTMAPMODE_PIXEL;
parallaxLayer *parallaxStuff = NULL;
int cameraPX = 0, cameraPY = 0;

uint sceneWidth, sceneHeight;
int lightMapNumber;
uint currentBlankColour = TS_ARGB(255, 0, 0, 0);

extern int cameraX, cameraY;
extern float cameraZoom;

void nosnapshot() {
	if (snapshotSurface.getPixels())
		snapshotSurface.free();
}

void saveSnapshot(Common::WriteStream *stream) {
	if (snapshotSurface.getPixels()) {
		stream->writeByte(1);               // 1 for snapshot follows
		Image::writePNG(*stream, snapshotSurface);
	} else {
		stream->writeByte(0);
	}
}

bool snapshot() {
	nosnapshot();
	if (!freeze())
		return false;

	// draw snapshot to backdrop
	displayBase();
	viewSpeech(); // ...and anything being said
	drawStatusBar();

	// copy backdrop to snapshot
	snapshotSurface.copyFrom(backdropSurface);

	unfreeze(false);
	return true;
}

bool restoreSnapshot(Common::SeekableReadStream *stream) {
	if (!(ImgLoader::loadImage(stream, &snapshotSurface))) {
		return false;
	}
	return true;
}

void killBackDrop() {
	if (backdropSurface.getPixels())
		backdropSurface.free();
#if 0
	deleteTextures(1, &backdropTextureName);
	backdropTextureName = 0;
#endif
	backdropExists = false;
}

void killLightMap() {
	if (lightMap.getPixels()) {
		lightMap.free();
	}
	lightMapNumber = 0;
#if 0
	deleteTextures(1, &lightMap.name);
	lightMap.name = 0;
#endif
}

void killParallax() {

	while (parallaxStuff) {

		parallaxLayer *k = parallaxStuff;
		parallaxStuff = k->next;
#if 0
		// Now kill the image
		deleteTextures(1, &k->textureName);
#endif
		k->surface.free();
		delete k;
		k = NULL;
	}

}

bool reserveBackdrop() {
	cameraX = 0;
	cameraY = 0;
	input.mouseX = (int)((float)input.mouseX * cameraZoom);
	input.mouseY = (int)((float)input.mouseY * cameraZoom);
	cameraZoom = 1.0;
	input.mouseX = (int)((float)input.mouseX / cameraZoom);
	input.mouseY = (int)((float)input.mouseY / cameraZoom);
	setPixelCoords(false);
#if 0
	int picWidth = sceneWidth;
	int picHeight = sceneHeight;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (backdropTexture) delete backdropTexture;
	if (!NPOT_textures) {
		picWidth = getNextPOT(sceneWidth);
		picHeight = getNextPOT(sceneHeight);
		backdropTexW = ((double)sceneWidth) / picWidth;
		backdropTexH = ((double)sceneHeight) / picHeight;
	}
	backdropTexture = new GLubyte [picWidth * picHeight * 4];
	if (!checkNew(backdropTexture)) return false;

	if (!backdropTextureName) glGenTextures(1, &backdropTextureName);
	glBindTexture(GL_TEXTURE_2D, backdropTextureName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (gameSettings.antiAlias < 0) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture, backdropTextureName);
#endif
	return true;
}

void killAllBackDrop() {
	killLightMap();
	killBackDrop();
	killParallax();
	killZBuffer();
}

bool resizeBackdrop(int x, int y) {
	sceneWidth = x;
	sceneHeight = y;
	return reserveBackdrop();
}

bool killResizeBackdrop(int x, int y) {
	killAllBackDrop();
	return resizeBackdrop(x, y);
}

void loadBackDrop(int fileNum, int x, int y) {
	debug(kSludgeDebugGraphics, "Load back drop");
	setResourceForFatal(fileNum);
	if (!openFileFromNum(fileNum)) {
		fatal("Can't load overlay image");
		return;
	}

	if (!loadHSI(bigDataFile, x, y, false)) {
		Common::String mess = Common::String::format("Can't paste overlay image outside scene dimensions\n\nX = %i\nY = %i\nWidth = %i\nHeight = %i", x, y, sceneWidth, sceneHeight);
		fatal(mess);
	}

	finishAccess();
	setResourceForFatal(-1);

	// set zBuffer if it's not set
	if (zBufferToSet >= 0) {
		setZBuffer(zBufferToSet);
		zBufferToSet = -1;
	}
}

void mixBackDrop(int fileNum, int x, int y) {
	setResourceForFatal(fileNum);
	if (!openFileFromNum(fileNum)) {
		fatal("Can't load overlay image");
		return;
	}

	if (!mixHSI(bigDataFile, x, y)) {
		fatal("Can't paste overlay image outside screen dimensions");
	}

	finishAccess();
	setResourceForFatal(-1);
}

void blankScreen(int x1, int y1, int x2, int y2) {
	// in case of no backdrop added at all
	if (!backdropSurface.getPixels()) {
		return;
	}

	if (y1 < 0)
		y1 = 0;
	if (x1 < 0)
		x1 = 0;
	if (x2 > (int)sceneWidth)
		x2 = (int)sceneWidth;
	if (y2 > (int)sceneHeight)
		y2 = (int)sceneHeight;

	backdropSurface.fillRect(Common::Rect(x1, y1, x2, y2), currentBlankColour);

#if 0
	setPixelCoords(true);

	int xoffset = 0;
	while (xoffset < picWidth) {
		int w = (picWidth - xoffset < viewportWidth) ? picWidth - xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < picHeight) {
			int h = (picHeight - yoffset < viewportHeight) ? picHeight - yoffset : viewportHeight;

			// Render the scene

			const GLfloat vertices[] = {
				-10.325f, -1.325f, 0.0f,
				w + 1.325f, -1.325f, 0.0f,
				-10.325f, h + 1.325f, 0.0f,
				w + 1.325f, h + 1.325f, 0.0f
			};

			glUseProgram(shader.color);

			setPMVMatrix(shader.color);
			setPrimaryColor(redValue(currentBlankColour) / 255.0f, greenValue(currentBlankColour) / 255.0f, blueValue(currentBlankColour) / 255.0f, 1.0f);
			drawQuad(shader.color, vertices, 0);

			glUseProgram(0);

			// Copy Our ViewPort To The Texture
			copyTexSubImage2D(GL_TEXTURE_2D, 0, x1 + xoffset, y1 + yoffset, viewportOffsetX, viewportOffsetY, w, h, backdropTextureName);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}

	setPixelCoords(false);
#endif
}

void hardScroll(int distance) {
	if (ABS(distance) >= (int)sceneHeight) {
		blankScreen(0, 0, sceneWidth, sceneHeight);
		return;
	}

	if (!distance)
		return;
#if 0
	const GLfloat backdropTexCoords[] = {
		0.0f, 0.0f,
		backdropTexW, 0.0f,
		0.0f, backdropTexH,
		backdropTexW, backdropTexH
	};

	setPixelCoords(true);

	uint xoffset = 0;
	while (xoffset < sceneWidth) {
		int w = (sceneWidth - xoffset < viewportWidth) ? sceneWidth - xoffset : viewportWidth;

		uint yoffset = 0;
		while (yoffset < sceneHeight) {
			int h = (sceneHeight - yoffset < viewportHeight) ? sceneHeight - yoffset : viewportHeight;

			glClear(GL_COLOR_BUFFER_BIT);   // Clear The Screen

			// Render the backdrop
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			const GLfloat vertices[] = {
				(GLfloat) - xoffset, (GLfloat) - distance - yoffset, 0.,
				(GLfloat)sceneWidth - xoffset, (GLfloat) - distance - yoffset, 0.,
				(GLfloat) - xoffset, (GLfloat)sceneHeight - distance - yoffset, 0.,
				(GLfloat)sceneWidth - xoffset, (GLfloat)sceneHeight - distance - yoffset, 0.
			};

			glUseProgram(shader.texture);

			setPMVMatrix(shader.texture);

			drawQuad(shader.texture, vertices, 1, backdropTexCoords);

			glUseProgram(0);

			// Copy Our ViewPort To The Texture
			copyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, viewportOffsetX, viewportOffsetY, w, h, backdropTextureName);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}
	setPixelCoords(false);
#endif
}

void drawVerticalLine(uint x, uint y1, uint y2) {
	backdropSurface.drawLine(x, y1, x, y2, backdropSurface.format.ARGBToColor(255, 0, 0, 0));
}

void drawHorizontalLine(uint x1, uint y, uint x2) {
	backdropSurface.drawLine(x1, y, x2, y, backdropSurface.format.ARGBToColor(255, 0, 0, 0));
}

void darkScreen() {
	Graphics::TransparentSurface tmp(backdropSurface, false);
	tmp.blit(backdropSurface, 0, 0, Graphics::FLIP_NONE, nullptr, TS_ARGB(0, 255 >> 1, 0, 0));
}

inline int sortOutPCamera(int cX, int fX, int sceneMax, int boxMax) {
	return (fX == 65535) ? (sceneMax ? ((cX * boxMax) / sceneMax) : 0) : ((cX * fX) / 100);
}

void drawBackDrop() {
	if (!backdropExists)
		return;

	// TODO: apply lightmap shader

	// draw parallaxStuff
	if (parallaxStuff) {
		// TODO: simulate image repeating effect
		warning("Drawing parallaxStuff");
#if 0
		parallaxLayer *ps = parallaxStuff;

		// go to the parallax at bottom
		while (ps->next) ps = ps->next;

		// draw parallax one by one
		while (ps) {
			ps->cameraX = sortOutPCamera(cameraX, ps->fractionX, (int)(sceneWidth - (float)winWidth / cameraZoom), (int)(ps->surface.w - (float)winWidth / cameraZoom));
			ps->cameraY = sortOutPCamera(cameraY, ps->fractionY, (int)(sceneHeight - (float)winHeight / cameraZoom), (int)(ps->surface.h - (float)winHeight / cameraZoom));

			uint w = (ps->wrapS) ? sceneWidth : ps->surface.w;
			uint h = (ps->wrapT) ? sceneHeight : ps->surface.h;

			const GLfloat vertices[] = {
				(GLfloat) - ps->cameraX, (GLfloat) - ps->cameraY, 0.1f,
				w - ps->cameraX, (GLfloat) - ps->cameraY, 0.1f,
				(GLfloat) - ps->cameraX, h - ps->cameraY, 0.1f,
				w - ps->cameraX, h - ps->cameraY, 0.1f
			};

			const GLfloat texCoords[] = {
				0.0f, 0.0f,
				texw, 0.0f,
				0.0f, texh,
				texw, texh
			};
			drawQuad(shader.smartScaler, vertices, 1, texCoords);

			ps = ps->prev;
		}
#endif
	}

	// draw backdrop
	Graphics::TransparentSurface tmp(backdropSurface, false);
	tmp.blit(renderSurface, 0, 0);
}

bool loadLightMap(int v) {
	setResourceForFatal(v);
	if (!openFileFromNum(v))
		return fatal("Can't open light map.");

	killLightMap();
	lightMapNumber = v;

	if (!ImgLoader::loadImage(bigDataFile, &lightMap))
		return false;

#if 0
	int newPicWidth = lightMap.w;
	int newPicHeight = lightMap.h;

	if (lightMapMode == LIGHTMAPMODE_HOTSPOT) {
		if (lightMap.w != sceneWidth || lightMap.h != sceneHeight) {
			return fatal("Light map width and height don't match scene width and height. That is required for lightmaps in HOTSPOT mode.");
		}
	}

	if (!NPOT_textures) {
		newPicWidth = getNextPOT(lightMap.w);
		newPicHeight = getNextPOT(lightMap.h);
		lightMap.texW = (double) lightMap.w / newPicWidth;
		lightMap.texH = (double) lightMap.h / newPicHeight;
	} else {
		lightMap.texW = 1.0;
		lightMap.texH = 1.0;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif
#if 0
	if (!lightMap.name) glGenTextures(1, &lightMap.name);
	glBindTexture(GL_TEXTURE_2D, lightMap.name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newPicWidth, newPicHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, lightMap.data, lightMap.name);
#endif
	finishAccess();

	setResourceForFatal(-1);

	return true;
}

void reloadParallaxTextures() {
#if 0
	parallaxLayer *nP = parallaxStuff;
	if (!nP) return;

	while (nP) {
		//fprintf (stderr, "Reloading parallax. (%d, %d) ", nP->width, nP->height);
		nP->textureName = 0;

		glGenTextures(1, &nP->textureName);
		glBindTexture(GL_TEXTURE_2D, nP->textureName);
		if (nP->wrapS)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		if (nP->wrapT)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (gameSettings.antiAlias < 0) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		if (!NPOT_textures) {
			texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getNextPOT(nP->width), getNextPOT(nP->height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nP->texture, nP->textureName);
		} else {
			texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nP->width, nP->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nP->texture, nP->textureName);
		}

		nP = nP->next;
	}
#endif
}

bool loadParallax(uint16 v, uint16 fracX, uint16 fracY) {
	setResourceForFatal(v);
	if (!openFileFromNum(v))
		return fatal("Can't open parallax image");

	parallaxLayer *nP = new parallaxLayer;
	if (!checkNew(nP))
		return false;

	nP->next = parallaxStuff;
	parallaxStuff = nP;
	if (nP->next) {
		nP->next->prev = nP;
	}
	nP->prev = NULL;

	if (!ImgLoader::loadImage(bigDataFile, &nP->surface, 0))
		return false;

	nP->fileNum = v;
	nP->fractionX = fracX;
	nP->fractionY = fracY;

	// 65535 is the value of AUTOFIT constant in Sludge
	if (fracX == 65535) {
		nP->wrapS = false;
		if (nP->surface.w < winWidth) {
			fatal("For AUTOFIT parallax backgrounds, the image must be at least as wide as the game window/screen.");
			return false;
		}
	} else {
		nP->wrapS = true;
	}

	if (fracY == 65535) {
		nP->wrapT = false;
		if (nP->surface.h < winHeight) {
			fatal("For AUTOFIT parallax backgrounds, the image must be at least as tall as the game window/screen.");
			return false;
		}
	} else {
		nP->wrapT = true;
	}

	// TODO: reinterpret this part
#if 0
	if (nP->wrapS)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	if (nP->wrapT)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif

	finishAccess();
	setResourceForFatal(-1);
	return true;
}

extern int viewportOffsetX, viewportOffsetY;

#if 0
void makeGlArray(GLuint &tmpTex, const GLubyte *texture, int picWidth, int picHeight) {
	glGenTextures(1, &tmpTex);
	glBindTexture(GL_TEXTURE_2D, tmpTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (gameSettings.antiAlias < 0) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture, tmpTex);

}

void renderToTexture(GLuint tmpTex, int x, int y, int picWidth, int picHeight, int realPicWidth, int realPicHeight) {
	GLfloat texCoordW = 1.0;
	GLfloat texCoordH = 1.0;
	if (!NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
		texCoordW = ((double)realPicWidth) / picWidth;
		texCoordH = ((double)realPicHeight) / picHeight;
	}

	float btx1;
	float btx2;
	float bty1;
	float bty2;
	if (!NPOT_textures) {
		btx1 = backdropTexW * x / sceneWidth;
		btx2 = backdropTexW * (x + realPicWidth) / sceneWidth;
		bty1 = backdropTexH * y / sceneHeight;
		bty2 = backdropTexH * (y + realPicHeight) / sceneHeight;
	} else {
		btx1 = (float) x / sceneWidth;
		btx2 = (float)(x + realPicWidth) / sceneWidth;
		bty1 = (float) y / sceneHeight;
		bty2 = (float)(y + realPicHeight) / sceneHeight;
	}

	const GLfloat btexCoords[] = {
		btx1, bty1,
		btx2, bty1,
		btx1, bty2,
		btx2, bty2
	};

	setPixelCoords(true);

	int xoffset = 0;
	while (xoffset < realPicWidth) {
		int w = (realPicWidth - xoffset < viewportWidth) ? realPicWidth - xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < realPicHeight) {
			int h = (realPicHeight - yoffset < viewportHeight) ? realPicHeight - yoffset : viewportHeight;

			glClear(GL_COLOR_BUFFER_BIT);   // Clear The Screen

			const GLfloat vertices[] = {
				(GLfloat) - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat)realPicWidth - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat) - xoffset, (GLfloat) - yoffset + realPicHeight, 0.,
				(GLfloat)realPicWidth - xoffset, (GLfloat) - yoffset + realPicHeight, 0.
			};

			const GLfloat texCoords[] = {
				0.0f, 0.0f,
				texCoordW, 0.0f,
				0.0f, texCoordH,
				texCoordW, texCoordH
			};

			if (backdropExists) {
				// Render the sprite to the backdrop
				// (using mulitexturing, so the old backdrop is seen where alpha < 1.0)
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, backdropTextureName);
				glActiveTexture(GL_TEXTURE0);

				glUseProgram(shader.paste);
				GLint uniform = glGetUniformLocation(shader.paste, "useLightTexture");
				if (uniform >= 0) glUniform1i(uniform, 0);// No lighting

				setPMVMatrix(shader.paste);

				setPrimaryColor(1.0, 1.0, 1.0, 1.0);
				glBindTexture(GL_TEXTURE_2D, tmpTex);
				//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

				drawQuad(shader.paste, vertices, 3, texCoords, NULL, btexCoords);

				glUseProgram(0);

			} else {
				// It's all new - nothing special to be done.
				glUseProgram(shader.texture);
				setPMVMatrix(shader.texture);

				glBindTexture(GL_TEXTURE_2D, tmpTex);

				setPrimaryColor(1.0, 0.0, 0.0, 0.0);

				drawQuad(shader.texture, vertices, 1, texCoords);

				glUseProgram(0);
			}

			// Copy Our ViewPort To The Texture
			copyTexSubImage2D(GL_TEXTURE_2D, 0, x + xoffset, y + yoffset, viewportOffsetX, viewportOffsetY, w, h, backdropTextureName);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}
	setPixelCoords(false);
}
#endif
bool loadHSI(Common::SeekableReadStream *stream, int x, int y, bool reserve) {
	debug(kSludgeDebugGraphics, "Load HSI");
	if (reserve) {
		killAllBackDrop(); // kill all
	}

	if (!ImgLoader::loadImage(stream, &backdropSurface, (int)reserve))
		return false;

	uint realPicWidth = backdropSurface.w;
	uint realPicHeight = backdropSurface.h;

	if (reserve) { // resize backdrop
		if (!resizeBackdrop(realPicWidth, realPicHeight))
			return false;
	}

	if (x == IN_THE_CENTRE)
		x = (sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE)
		y = (sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > sceneWidth || y < 0 || y + realPicHeight > sceneHeight) {
		debug(kSludgeDebugGraphics, "Illegal back drop size");
		return false;
	}
#if 0
	GLuint tmpTex;
	makeGlArray(tmpTex, backdropTexture, picWidth, picHeight);

	renderToTexture(tmpTex, x, y, picWidth, picHeight, realPicWidth, realPicHeight);

	deleteTextures(1, &tmpTex);
#endif
	backdropExists = true;

	return true;
}

bool mixHSI(Common::SeekableReadStream *stream, int x, int y) {
	debug(kSludgeDebugGraphics, "Load mixHSI");
	Graphics::Surface mixSurface;
	if (!ImgLoader::loadImage(stream, &mixSurface, 0))
		return false;

	uint realPicWidth = mixSurface.w;
	uint realPicHeight = mixSurface.h;

	if (x == IN_THE_CENTRE)
		x = (sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE)
		y = (sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > sceneWidth || y < 0 || y + realPicHeight > sceneHeight)
		return false;

	Graphics::TransparentSurface tmp(mixSurface, false);
	tmp.blit(backdropSurface, x, y, Graphics::FLIP_NONE, nullptr, TS_ARGB(255, 255 >> 1, 255, 255));
	mixSurface.free();
;
#if 0
	float btx1, tx1;
	float btx2, tx2;
	float bty1, ty1;
	float bty2, ty2;

	if (!NPOT_textures) {
		tx1 = 0.0;
		ty1 = 0.0;
		tx2 = ((double)picWidth) / getNextPOT(picWidth);
		ty2 = ((double)picHeight) / getNextPOT(picHeight);
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
		btx1 = backdropTexW * x / sceneWidth;
		btx2 = backdropTexW * (x + realPicWidth) / sceneWidth;
		bty1 = backdropTexH * y / sceneHeight;
		bty2 = backdropTexH * (y + realPicHeight) / sceneHeight;
	} else {
		tx1 = 0.0;
		ty1 = 0.0;
		tx2 = 1.0;
		ty2 = 1.0;
		btx1 = (float) x / sceneWidth;
		btx2 = (float)(x + picWidth) / sceneWidth;
		bty1 = (float) y / sceneHeight;
		bty2 = (float)(y + picHeight) / sceneHeight;
	}

	const GLfloat texCoords[] = {
		tx1, ty1,
		tx2, ty1,
		tx1, ty2,
		tx2, ty2
	};

	const GLfloat btexCoords[] = {
		btx1, bty1,
		btx2, bty1,
		btx1, bty2,
		btx2, bty2
	};

	GLuint tmpTex;
	makeGlArray(tmpTex, backdropTexture, picWidth, picHeight);

	setPixelCoords(true);

	int xoffset = 0;
	while (xoffset < realPicWidth) {
		int w = (realPicWidth - xoffset < viewportWidth) ? realPicWidth - xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < realPicHeight) {
			int h = (realPicHeight - yoffset < viewportHeight) ? realPicHeight - yoffset : viewportHeight;

			glClear(GL_COLOR_BUFFER_BIT);   // Clear The Screen

			// Render the sprite to the backdrop
			// (using mulitexturing, so the backdrop is seen where alpha < 1.0)
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glActiveTexture(GL_TEXTURE0);

			glUseProgram(shader.paste);
			GLint uniform = glGetUniformLocation(shader.paste, "useLightTexture");
			if (uniform >= 0) glUniform1i(uniform, 0);// No lighting

			setPMVMatrix(shader.paste);

			setPrimaryColor(1.0, 1.0, 1.0, 0.5);

			glBindTexture(GL_TEXTURE_2D, tmpTex);
			//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			const GLfloat vertices[] = {
				(GLfloat) - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat)realPicWidth - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat) - xoffset, (GLfloat) - yoffset + realPicHeight, 0.,
				(GLfloat)realPicWidth - xoffset, (GLfloat) - yoffset + realPicHeight, 0.
			};

			drawQuad(shader.paste, vertices, 3, texCoords, NULL, btexCoords);

			// Copy Our ViewPort To The Texture
			glUseProgram(0);

			copyTexSubImage2D(GL_TEXTURE_2D, 0, (int)((x < 0) ? xoffset : x + xoffset), (int)((y < 0) ? yoffset : y + yoffset), (int)((x < 0) ? viewportOffsetX - x : viewportOffsetX), (int)((y < 0) ? viewportOffsetY - y : viewportOffsetY), w, h, backdropTextureName);

			yoffset += viewportHeight;
		}

		xoffset += viewportWidth;
	}
	deleteTextures(1, &tmpTex);
	setPixelCoords(false);
#endif
	return true;
}

#if 0
void saveCoreHSI(Common::WriteStream *stream, GLuint texture, int w, int h) {
	GLint tw, th;
	glBindTexture(GL_TEXTURE_2D, texture);
	getTextureDimensions(texture, &tw, &th);

	GLushort *image = new GLushort[tw * th];
	if (!checkNew(image))
		return;
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, image);
	setPixelCoords(true);

	//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	const GLfloat texCoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

	int xoffset = 0;
	while (xoffset < tw) {
		int w = (tw - xoffset < viewportWidth) ? tw - xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < th) {
			int h = (th - yoffset < viewportHeight) ? th - yoffset : viewportHeight;
			glClear (GL_COLOR_BUFFER_BIT);   // Clear The Screen
			const GLfloat vertices[] = { (GLfloat)-xoffset, (GLfloat)-yoffset, 0., (GLfloat)w - xoffset, (GLfloat)-yoffset, 0., (GLfloat)-xoffset, (GLfloat)-yoffset + h, 0., (GLfloat)w - xoffset,
					(GLfloat)-yoffset + h, 0. };

			glUseProgram(shader.texture);
			setPMVMatrix(shader.texture);
			drawQuad(shader.texture, vertices, 1, texCoords);
			glUseProgram(0);

			for (int i = 0; i < h; i++) {
				glReadPixels(viewportOffsetX, viewportOffsetY + i, w, 1, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, image + xoffset + (yoffset + i) * tw);
			}
			yoffset += viewportHeight;
		}

		xoffset += viewportWidth;
	}
	//glReadPixels(viewportOffsetX, viewportOffsetY, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, data);
	setPixelCoords(false);

	int x, y, lookAhead;
	uint16 *fromHere, *lookPointer;

	stream->writeUint16BE(w);
	stream->writeUint16BE(h);

	for (y = 0; y < h; y++) {
		fromHere = image + (y * tw);
		x = 0;
		while (x < w) {
			lookPointer = fromHere + 1;
			for (lookAhead = x + 1; lookAhead < w; lookAhead++) {
				if (lookAhead - x == 256)
					break;
				if (*fromHere != *lookPointer)
					break;
				lookPointer++;
			}
			if (lookAhead == x + 1) {
				put2bytes((*fromHere) & 65503, stream);
			} else {
				stream->writeUint16BE(*fromHere | 32);
				stream->writeByte(lookAhead - x - 1);
			}
			fromHere = lookPointer;
			x = lookAhead;
		}
	}
	delete[] image;
	image = NULL;
}
#endif

void saveHSI(Common::WriteStream *stream) {
	Image::writePNG(*stream, backdropSurface);
}

void saveParallaxRecursive(parallaxLayer *me, Common::WriteStream *stream) {
	if (me) {
		saveParallaxRecursive(me->next, stream);
		stream->writeByte(1);
		stream->writeUint16BE(me->fileNum);
		stream->writeUint16BE(me->fractionX);
		stream->writeUint16BE(me->fractionY);
	}
}

bool getRGBIntoStack(uint x, uint y, stackHandler *sH) {
#if 0
	if (x >= sceneWidth || y >= sceneHeight) {
		return fatal("Co-ordinates are outside current scene!");
	}

	variable newValue;

	newValue.varType = SVT_NULL;

	saveTexture(backdropTextureName, backdropTexture);

	GLubyte *target;
	if (!NPOT_textures) {
		target = backdropTexture + 4 * getNextPOT(sceneWidth) * y + x * 4;
	} else {
		target = backdropTexture + 4 * sceneWidth * y + x * 4;
	}

	setVariable(newValue, SVT_INT, target[2]);
	if (!addVarToStackQuick(newValue, sH->first)) return false;
	sH->last = sH->first;

	setVariable(newValue, SVT_INT, target[1]);
	if (!addVarToStackQuick(newValue, sH->first)) return false;

	setVariable(newValue, SVT_INT, target[0]);
	if (!addVarToStackQuick(newValue, sH->first)) return false;
#endif
	return true;
}

} // End of namespace Sludge
