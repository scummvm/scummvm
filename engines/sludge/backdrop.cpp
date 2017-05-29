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
#if 0
#if defined __unix__ && !(defined __APPLE__)
#include <png.h>
#else
#include <libpng/png.h>
#endif

#include <stdlib.h>
#include <stdint.h>

#if !defined(HAVE_GLES2)
#include "GLee.h"
#else
#include <GLES2/gl2.h>
#endif
#endif

#include "allfiles.h"
#include "debug.h"
#include "newfatal.h"
#include "colours.h"
#include "fileset.h"
#include "cursors.h"
#include "backdrop.h"
#include "language.h"
#include "moreio.h"
#include "variable.h"
#include "zbuffer.h"
#include "graphics.h"
#include "line.h"
#include "people.h"
#include "talk.h"
#include "sludger.h"
#include "statusba.h"
#include "variable.h"
#include "CommonCode/version.h"
#include "common/debug.h"
#include "image/png.h"
#include "graphics/surface.h"
#include "sludge.h"

namespace Sludge {

extern inputType input;

bool freeze();
void unfreeze(bool);    // Because FREEZE.H needs a load of other includes

#if 0
GLubyte *backdropTexture = NULL;
GLuint backdropTextureName = 0;
bool backdropExists = false;
GLfloat backdropTexW = 1.0;
GLfloat backdropTexH = 1.0;

texture lightMap;

GLuint snapshotTextureName = 0;
#endif

Graphics::Surface backdropSurface;

float snapTexW = 1.0;
float snapTexH = 1.0;

int lightMapMode = LIGHTMAPMODE_PIXEL;
parallaxLayer *parallaxStuff = NULL;
int cameraPX = 0, cameraPY = 0;

unsigned int sceneWidth, sceneHeight;
int lightMapNumber;
unsigned int currentBlankColour = makeColour(0, 0, 0);

extern int cameraX, cameraY;
extern float cameraZoom;

void nosnapshot() {
#if 0
	deleteTextures(1, &snapshotTextureName);
	snapshotTextureName = 0;
#endif
}

void saveSnapshot(Common::WriteStream *stream) {
#if 0
	if (snapshotTextureName) {
		putch(1, stream);               // 1 for snapshot follows
		saveCoreHSI(stream, snapshotTextureName, winWidth, winHeight);
	} else {
		putch(0, stream);
	}
#endif
}

bool snapshot() {

	nosnapshot();
	if (!freeze())
		return false;
#if 0
	setPixelCoords(true);
	glGenTextures(1, &snapshotTextureName);

	int w = winWidth;
	int h = winHeight;
	if (!NPOT_textures) {
		w = getNextPOT(winWidth);
		h = getNextPOT(winHeight);
		snapTexW = ((double)winWidth) / w;
		snapTexH = ((double)winHeight) / h;
	}

	glBindTexture(GL_TEXTURE_2D, snapshotTextureName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0, snapshotTextureName);

	// Render scene
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear The Screen
	glDepthMask(GL_FALSE);

	drawBackDrop();// Draw the room
	drawZBuffer(cameraX, cameraY, false);

	glEnable(GL_DEPTH_TEST);

	drawPeople();// Then add any moving characters...

	glDisable(GL_DEPTH_TEST);

	viewSpeech();// ...and anything being said
	drawStatusBar();
				 // Copy Our ViewPort To The Texture
	copyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, viewportOffsetX, viewportOffsetY, winWidth, winHeight, snapshotTextureName);

	setPixelCoords(false);
#endif
	unfreeze(false);
	return true;
}

bool restoreSnapshot(Common::SeekableReadStream *stream) {
	unsigned int picWidth = get2bytes(stream);
	unsigned int picHeight = get2bytes(stream);

	if ((picWidth != winWidth) || (picHeight != winHeight))
		return false;

	unsigned int t1, t2, n;
	unsigned short c;
#if 0
	GLubyte *target;
	if (!NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
		snapTexW = ((double)winWidth) / picWidth;
		snapTexH = ((double)winHeight) / picHeight;
	}
	GLubyte *snapshotTexture = new GLubyte [picHeight * picWidth * 4];
	if (!snapshotTexture) return fatal("Out of memory while restoring snapshot.");
#endif

	for (t2 = 0; t2 < winHeight; t2++) {
		t1 = 0;
		while (t1 < winWidth) {
			c = (unsigned short) get2bytes(stream);
			if (c & 32) {
				n = getch(stream) + 1;
				c -= 32;
			} else {
				n = 1;
			}
#if 0
			while (n --) {
				target = snapshotTexture + 4 * picWidth * t2 + t1 * 4;
				target[0] = (GLubyte) redValue(c);
				target[1] = (GLubyte) greenValue(c);
				target[2] = (GLubyte) blueValue(c);
				target[3] = (GLubyte) 255;
				t1++;
			}
#endif
		}
	}
#if 0
	if (!snapshotTextureName) glGenTextures(1, &snapshotTextureName);
	glBindTexture(GL_TEXTURE_2D, snapshotTextureName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, snapshotTexture, snapshotTextureName);

	delete snapshotTexture;
	snapshotTexture = NULL;
#endif
	return true;
}

void killBackDrop() {
#if 0
	deleteTextures(1, &backdropTextureName);
	backdropTextureName = 0;
	backdropExists = false;
#endif
}

void killLightMap() {
#if 0
	deleteTextures(1, &lightMap.name);
	lightMap.name = 0;
	if (lightMap.data) {
		delete lightMap.data;
		lightMap.data = NULL;
	}
	lightMapNumber = 0;
#endif
}

void killParallax() {
#if 0
	while (parallaxStuff) {

		parallaxLayer *k = parallaxStuff;
		parallaxStuff = k->next;

		// Now kill the image
		deleteTextures(1, &k->textureName);
		delete k->texture;
		delete k;
		k = NULL;
	}
#endif
}

bool reserveBackdrop() {
	cameraX = 0;
	cameraY = 0;
	input.mouseX = (int) ((float) input.mouseX * cameraZoom);
	input.mouseY = (int) ((float) input.mouseY * cameraZoom);
	cameraZoom = 1.0;
	input.mouseX = (int) ((float) input.mouseX / cameraZoom);
	input.mouseY = (int) ((float) input.mouseY / cameraZoom);
	setPixelCoords(false);
	int picWidth = sceneWidth;
	int picHeight = sceneHeight;
#if 0
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

bool resizeBackdrop(int x, int y) {
	killLightMap();
	killBackDrop();
	killParallax();
	killZBuffer();
	sceneWidth = x;
	sceneHeight = y;
	return reserveBackdrop();
}

void loadBackDrop(int fileNum, int x, int y) {
	debug(kSludgeDebugGraphics, "Load back drop");
	setResourceForFatal(fileNum);
	if (!openFileFromNum(fileNum)) {
		fatal("Can't load overlay image");
		return;
	}

	if (!loadHSI(bigDataFile, x, y, false)) {
		char mess[200];
		sprintf(mess,
				"Can't paste overlay image outside scene dimensions\n\nX = %i\nY = %i\nWidth = %i\nHeight = %i",
				x, y, sceneWidth, sceneHeight);
		fatal(mess);
	}

	finishAccess();
	setResourceForFatal(-1);
}

void mixBackDrop(int fileNum, int x, int y) {
#if 0
	setResourceForFatal(fileNum);
	if (!openFileFromNum(fileNum)) {
		fatal("Can't load overlay image");
		return;
	}

	if (!mixHSI(bigDataFile, x, y)) {
		fatal("Can't paste overlay image outside screen dimensions");
	}

	finishAccess();
#endif
	setResourceForFatal(-1);
}

void blankScreen(int x1, int y1, int x2, int y2) {

	if (y1 < 0)
		y1 = 0;
	if (x1 < 0)
		x1 = 0;
	if (x2 > (int) sceneWidth)
		x2 = (int) sceneWidth;
	if (y2 > (int) sceneHeight)
		y2 = (int) sceneHeight;

	int picWidth = x2 - x1;
	int picHeight = y2 - y1;
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
	if (abs(distance) >= sceneHeight) {
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

	unsigned int xoffset = 0;
	while (xoffset < sceneWidth) {
		int w = (sceneWidth - xoffset < viewportWidth) ? sceneWidth - xoffset : viewportWidth;

		unsigned int yoffset = 0;
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

void drawVerticalLine(unsigned int x, unsigned int y1, unsigned int y2) {
	drawLine(x, y1, x, y2);
}

void drawHorizontalLine(unsigned int x1, unsigned int y, unsigned int x2) {
	drawLine(x1, y, x2, y);
}

void darkScreen() {
	setPixelCoords(true);

	int xoffset = 0;
	while (xoffset < sceneWidth) {
		int w = (sceneWidth - xoffset < viewportWidth) ?
				sceneWidth - xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < sceneHeight) {
			int h = (sceneHeight - yoffset < viewportHeight) ?
					sceneHeight - yoffset : viewportHeight;

			// Render the scene - first the old backdrop
#if 0
			//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			const GLfloat vertices[] = {
				(GLfloat) - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat)sceneWidth - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat) - xoffset, (GLfloat)sceneHeight - yoffset, 0.,
				(GLfloat)sceneWidth - xoffset, (GLfloat)sceneHeight - yoffset, 0.
			};

			const GLfloat texCoords[] = {
				0.0f, 0.0f,
				backdropTexW, 0.0f,
				0.0f, backdropTexH,
				backdropTexW, backdropTexH
			};

			glUseProgram(shader.texture);

			setPMVMatrix(shader.texture);

			drawQuad(shader.texture, vertices, 1, texCoords);

			// Then the darkness
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

			glEnable(GL_BLEND);

			glUseProgram(shader.color);

			setPMVMatrix(shader.color);
			setPrimaryColor(0.0f, 0.0f, 0.0f, 0.5f);
			drawQuad(shader.color, vertices, 0);

			glUseProgram(0);

			glDisable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			// Copy Our ViewPort To The Texture
			copyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, viewportOffsetX, viewportOffsetY, w, h, backdropTextureName);

			yoffset += h;

			glClear(GL_COLOR_BUFFER_BIT);
#endif
		}
		xoffset += w;
	}

	setPixelCoords(false);
}

inline int sortOutPCamera(int cX, int fX, int sceneMax, int boxMax) {
	return (fX == 65535) ?
			(sceneMax ? ((cX * boxMax) / sceneMax) : 0) : ((cX * fX) / 100);
}

void drawBackDrop() {
#if 0
	setPrimaryColor(1.0, 1.0, 1.0, 1.0);

	//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);

	glUseProgram(shader.smartScaler);
	GLuint uniform = glGetUniformLocation(shader.smartScaler, "useLightTexture");
	if (uniform >= 0) glUniform1i(uniform, 0);

	setPMVMatrix(shader.smartScaler);

	if (gameSettings.antiAlias == 1) {
		glUniform1i(glGetUniformLocation(shader.smartScaler, "antialias"), 1);
	} else {
		glUniform1i(glGetUniformLocation(shader.smartScaler, "antialias"), 0);
	}

	if (parallaxStuff) {
		parallaxLayer *ps = parallaxStuff;

		while (ps->next) ps = ps->next;

		while (ps) {
			ps->cameraX = sortOutPCamera(cameraX, ps->fractionX, (int)(sceneWidth - (float)winWidth / cameraZoom), (int)(ps->width - (float)winWidth / cameraZoom));
			ps->cameraY = sortOutPCamera(cameraY, ps->fractionY, (int)(sceneHeight - (float)winHeight / cameraZoom), (int)(ps->height - (float)winHeight / cameraZoom));
			glBindTexture(GL_TEXTURE_2D, ps->textureName);

			float w = (ps->wrapS) ? sceneWidth : ps->width;
			float h = (ps->wrapT) ? sceneHeight : ps->height;
			float texw;
			float texh;
			if (!NPOT_textures) {
				texw = (ps->wrapS) ? (float) sceneWidth / ps->width : (float) ps->width / getNextPOT(ps->width);
				texh = (ps->wrapT) ? (float) sceneHeight / ps->height : (float) ps->height / getNextPOT(ps->height);
			} else {
				texw = (ps->wrapS) ? (float) sceneWidth / ps->width : 1.0;
				texh = (ps->wrapT) ? (float) sceneHeight / ps->height : 1.0;
			}

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
	}

	glBindTexture(GL_TEXTURE_2D, backdropTextureName);

	const GLfloat backdropTexCoords[] = {
		0.0f, 0.0f,
		backdropTexW, 0.0f,
		0.0f, backdropTexH,
		backdropTexW, backdropTexH
	};

	const GLfloat vertices[] = {
		(GLfloat) - cameraX, (GLfloat) - cameraY, 0.,
		(GLfloat)sceneWidth - (GLfloat)cameraX, (GLfloat) - cameraY, 0.,
		(GLfloat) - cameraX, (GLfloat)sceneHeight - (GLfloat)cameraY, 0.,
		(GLfloat)sceneWidth - (GLfloat)cameraX, (GLfloat)sceneHeight - (GLfloat)cameraY, 0.
	};

	drawQuad(shader.smartScaler, vertices, 1, backdropTexCoords);

	glDisable(GL_BLEND);

	glUseProgram(0);
#endif
}

bool loadLightMap(int v) {
	int newPicWidth, newPicHeight;
#if 0
	setResourceForFatal(v);
	if (!openFileFromNum(v)) return fatal("Can't open light map.");
	long file_pointer = ftell(bigDataFile);

	png_structp png_ptr;
	png_infop info_ptr, end_info;

	int fileIsPNG = true;

	// Is this a PNG file?

	char tmp[10];
	size_t bytes_read = fread(tmp, 1, 8, bigDataFile);
	if (bytes_read != 8 && ferror(bigDataFile)) {
		debugOut("Reading error in loadLightMap.\n");
	}

	if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
		// No, it's old-school HSI
		fileIsPNG = false;
		fseek(bigDataFile, file_pointer, SEEK_SET);

		newPicWidth = lightMap.w = get2bytes(bigDataFile);
		newPicHeight = lightMap.h = get2bytes(bigDataFile);
	} else {
		// Read the PNG header

		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) {
			return false;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
			return false;
		}

		end_info = png_create_info_struct(png_ptr);
		if (!end_info) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return false;
		}
		png_init_io(png_ptr, bigDataFile);     // Tell libpng which file to read
		png_set_sig_bytes(png_ptr, 8);// 8 bytes already read

		png_read_info(png_ptr, info_ptr);

		png_uint_32 width, height;
		int bit_depth, color_type, interlace_type, compression_type, filter_method;
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		newPicWidth = lightMap.w = width;
		newPicHeight = lightMap.h = height;

		if (bit_depth < 8) png_set_packing(png_ptr);
		png_set_expand(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
		if (bit_depth == 16) png_set_strip_16(png_ptr);

		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		//int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	}

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

	killLightMap();
	lightMapNumber = v;
#if 0
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif
	if (lightMap.data) delete [] lightMap.data;

	lightMap.data = new GLubyte [newPicWidth * newPicHeight * 4];
	if (!lightMap.data) {
		return fatal("Out of memory loading light map.");
	}

	int t1, t2, n;
	unsigned short c;
	GLubyte *target;

	if (fileIsPNG) {
		unsigned char *row_pointers[lightMap.h];
		for (int i = 0; i < lightMap.h; i++)
		row_pointers[i] = lightMap.data + 4 * i * newPicWidth;

		png_read_image(png_ptr, (png_byte **) row_pointers);
		png_read_end(png_ptr, NULL);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	} else {

		for (t2 = 0; t2 < lightMap.h; t2 ++) {
			t1 = 0;
			while (t1 < lightMap.w) {
				c = (unsigned short) get2bytes(bigDataFile);
				if (c & 32) {
					n = fgetc(bigDataFile) + 1;
					c -= 32;
				} else {
					n = 1;
				}
				while (n --) {
					target = lightMap.data + 4 * newPicWidth * t2 + t1 * 4;
					target[0] = (GLubyte) redValue(c);
					target[1] = (GLubyte) greenValue(c);
					target[2] = (GLubyte) blueValue(c);
					target[3] = (GLubyte) 255;
					t1++;
				}
			}
		}
	}
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

#endif
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

bool loadParallax(unsigned short v, unsigned short fracX,
		unsigned short fracY) {

#if 0
	setResourceForFatal(v);
	if (!openFileFromNum(v)) return fatal("Can't open parallax image");

	parallaxLayer *nP = new parallaxLayer;
	if (!checkNew(nP)) return false;

	nP->next = parallaxStuff;
	parallaxStuff = nP;
	if (nP->next) {
		nP->next->prev = nP;
	}
	nP->prev = NULL;

	int picWidth;
	int picHeight;

	long file_pointer = ftell(bigDataFile);

	png_structp png_ptr;
	png_infop info_ptr, end_info;

	int fileIsPNG = true;

	// Is this a PNG file?

	char tmp[10];
	size_t bytes_read = fread(tmp, 1, 8, bigDataFile);
	if (bytes_read != 8 && ferror(bigDataFile)) {
		debugOut("Reading error in loadParallax.\n");
	}
	if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
		// No, it's old-school HSI
		fileIsPNG = false;
		fseek(bigDataFile, file_pointer, SEEK_SET);

		picWidth = nP->width = get2bytes(bigDataFile);
		picHeight = nP->height = get2bytes(bigDataFile);
	} else {
		// Read the PNG header

		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) {
			return false;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
			return false;
		}

		end_info = png_create_info_struct(png_ptr);
		if (!end_info) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return false;
		}
		png_init_io(png_ptr, bigDataFile);     // Tell libpng which file to read
		png_set_sig_bytes(png_ptr, 8);// 8 bytes already read

		png_read_info(png_ptr, info_ptr);

		png_uint_32 width, height;
		int bit_depth, color_type, interlace_type, compression_type, filter_method;
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		picWidth = nP->width = width;
		picHeight = nP->height = height;

		if (bit_depth < 8) png_set_packing(png_ptr);
		png_set_expand(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
		if (bit_depth == 16) png_set_strip_16(png_ptr);

		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		//int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	}

	if (!NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
	}

	nP->fileNum = v;
	nP->fractionX = fracX;
	nP->fractionY = fracY;

	if (fracX == 65535) {
		nP->wrapS = false;
		if (nP->width < winWidth) {
			fatal("For AUTOFIT parallax backgrounds, the image must be at least as wide as the game window/screen.");
			return false;
		}
	} else {
		nP->wrapS = true;
	}

	if (fracY == 65535) {
		nP->wrapT = false;
		if (nP->height < winHeight) {
			fatal("For AUTOFIT parallax backgrounds, the image must be at least as tall as the game window/screen.");
			return false;
		}
	} else {
		nP->wrapT = true;
	}

	nP->texture = new GLubyte [picHeight * picWidth * 4];
	if (!checkNew(nP->texture)) return false;

	if (fileIsPNG) {
		unsigned char *row_pointers[nP->height];
		for (int i = 0; i < nP->height; i++)
		row_pointers[i] = nP->texture + 4 * i * picWidth;

		png_read_image(png_ptr, (png_byte **) row_pointers);
		png_read_end(png_ptr, NULL);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	} else {

		int t1, t2, n;
		unsigned short c;
		GLubyte *target;

		for (t2 = 0; t2 < nP->height; t2 ++) {
			t1 = 0;
			while (t1 < nP->width) {
				c = (unsigned short) get2bytes(bigDataFile);
				if (c & 32) {
					n = fgetc(bigDataFile) + 1;
					c -= 32;
				} else {
					n = 1;
				}
				while (n--) {
					target = nP->texture + 4 * picWidth * t2 + t1 * 4;
					if (c == 63519 || c == 2015) {
						target[0] = (GLubyte) 0;
						target[1] = (GLubyte) 0;
						target[2] = (GLubyte) 0;
						target[3] = (GLubyte) 0;
					} else {
						target[0] = (GLubyte) redValue(c);
						target[1] = (GLubyte) greenValue(c);
						target[2] = (GLubyte) blueValue(c);
						target[3] = (GLubyte) 255;
					}
					t1 ++;
				}
			}
		}
	}

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

	texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nP->texture, nP->textureName);

	finishAccess();
#endif
	setResourceForFatal(-1);
	return true;
}

extern int viewportOffsetX, viewportOffsetY;

bool loadPng(int &picWidth, int &picHeight, int &realPicWidth,
		int &realPicHeight, Common::SeekableReadStream *stream, bool reserve) {
	debug(kSludgeDebugGraphics, "Loading back drop png.");
	::Image::PNGDecoder png;
	if (!png.loadStream(*stream)) {
		debug(kSludgeDebugGraphics, "Back drop is not a png");
		return false;
	}
	backdropSurface.copyFrom(*(png.getSurface()));
	picWidth = realPicWidth = backdropSurface.w;
	picHeight = realPicHeight = backdropSurface.h;
	return true;
#if 0
	long file_pointer = stream->pos();
	png_structp png_ptr;
	png_infop info_ptr, end_info;

	char tmp[10];
	size_t bytes_read = stream->read(tmp, 8);

	if (bytes_read != 8 && stream->err()) {
		stream->seek(file_pointer, SEEK_SET);
		return false;
	}

	if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
		stream->seek(file_pointer, SEEK_SET);
		return false;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		return false;
	}

	end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		return false;
	}
	png_init_io(png_ptr, stream);       // Tell libpng which file to read
	png_set_sig_bytes(png_ptr, 8);// 8 bytes already read

	png_read_info(png_ptr, info_ptr);

	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, compression_type, filter_method;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

	picWidth = realPicWidth = width;
	picHeight = realPicHeight = height;

	if (bit_depth < 8) png_set_packing(png_ptr);
	png_set_expand(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
	if (bit_depth == 16) png_set_strip_16(png_ptr);

	png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

	png_read_update_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

	if (reserve) {
		if (!resizeBackdrop(realPicWidth, realPicHeight)) return false;
	}

	unsigned char *row_pointers[realPicHeight];

	for (int i = 0; i < realPicHeight; i++)
	row_pointers[i] = loadhere + 4 * i * picWidth;

	png_read_image(png_ptr, (png_byte **) row_pointers);
	png_read_end(png_ptr, NULL);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
#endif
}

bool loadByteArray(int &picWidth, int &picHeight, int &realPicWidth,
		int &realPicHeight, Common::SeekableReadStream *stream, bool reserve) {
#if 0
	int32_t transCol = reserve ? -1 : 63519;
	int t1, t2, n;
	unsigned short c;
	picWidth = realPicWidth = get2bytes(stream);
	picHeight = realPicHeight = get2bytes(stream);

	if (reserve) {
		if (!resizeBackdrop(realPicWidth, realPicHeight)) return false;
	}

	for (t2 = 0; t2 < realPicHeight; t2 ++) {
		t1 = 0;
		while (t1 < realPicWidth) {
			c = (unsigned short) get2bytes(stream);
			if (c & 32) {
				n = getch(stream) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n --) {
				GLubyte *target = loadhere + 4 * picWidth * t2 + t1 * 4;
				if (c == transCol || c == 2015) {
					target[0] = (GLubyte)0;
					target[1] = (GLubyte)0;
					target[2] = (GLubyte)0;
					target[3] = (GLubyte)0;
				} else {
					target[0] = (GLubyte)redValue(c);
					target[1] = (GLubyte)greenValue(c);
					target[2] = (GLubyte)blueValue(c);
					target[3] = (GLubyte)255;
				}
				t1++;
			}
		}
	}
#endif
}

bool loadImage(int &picWidth, int &picHeight, int &realPicWidth,
		int &realPicHeight, Common::SeekableReadStream *stream, int x, int y,
		bool reserve) {
	debug(kSludgeDebugGraphics, "Loading back drop image.");
	if (!loadPng(picWidth, picHeight, realPicWidth, realPicHeight, stream,
			reserve)) {
		if (!loadByteArray(picWidth, picHeight, realPicWidth, realPicHeight,
				stream, reserve)) {
			return false;
		}
	}

	if (x == IN_THE_CENTRE)
		x = (sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE)
		y = (sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > sceneWidth || y < 0
			|| y + realPicHeight > sceneHeight)
		return false;

	return true;
}

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
	int picWidth, picHeight;
	int realPicWidth, realPicHeight;

	if (!loadImage(picWidth, picHeight, realPicWidth, realPicHeight, stream, x,
			y, reserve))
		return false;
#if 0
	GLuint tmpTex;
	makeGlArray(tmpTex, backdropTexture, picWidth, picHeight);

	renderToTexture(tmpTex, x, y, picWidth, picHeight, realPicWidth, realPicHeight);

	deleteTextures(1, &tmpTex);

	backdropExists = true;
#endif
	return true;
}

bool mixHSI(Common::SeekableReadStream *stream, int x, int y) {
#if 0
	int realPicWidth, realPicHeight;
	int picWidth;
	int picHeight;

	long file_pointer = stream->pos();

	png_structp png_ptr;
	png_infop info_ptr, end_info;

	int fileIsPNG = true;

	// Is this a PNG file?
	char tmp[10];
	size_t bytes_read = stream->read(tmp, 8);
	if (bytes_read != 8 && stream->err()) {
		debugOut("Reading error in mixHSI.\n");
	}
	if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
		// No, it's old-school HSI
		fileIsPNG = false;
		stream->seek(file_pointer, SEEK_SET);

		picWidth = realPicWidth = get2bytes(stream);
		picHeight = realPicHeight = get2bytes(stream);
	} else {
		// Read the PNG header

		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) {
			return false;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
			return false;
		}

		end_info = png_create_info_struct(png_ptr);
		if (!end_info) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return false;
		}
		png_init_io(png_ptr, stream);       // Tell libpng which file to read
		png_set_sig_bytes(png_ptr, 8);// 8 bytes already read

		png_read_info(png_ptr, info_ptr);

		png_uint_32 width, height;
		int bit_depth, color_type, interlace_type, compression_type, filter_method;
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		picWidth = realPicWidth = width;
		picHeight = realPicHeight = height;

		if (bit_depth < 8) png_set_packing(png_ptr);
		png_set_expand(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
		if (bit_depth == 16) png_set_strip_16(png_ptr);

		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		//int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	}

	if (x == IN_THE_CENTRE) x = (sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE) y = (sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > sceneWidth || y < 0 || y + realPicHeight > sceneHeight) return false;

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

	int t1, t2, n;
	unsigned short c;
	GLubyte *target;
	int32_t transCol = 63519;

	if (fileIsPNG) {
		unsigned char *row_pointers[realPicHeight];
		for (int i = 0; i < realPicHeight; i++)
		row_pointers[i] = backdropTexture + 4 * i * picWidth;

		png_read_image(png_ptr, (png_byte **) row_pointers);
		png_read_end(png_ptr, NULL);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	} else {
		for (t2 = 0; t2 < realPicHeight; t2 ++) {
			t1 = 0;
			while (t1 < realPicWidth) {
				c = (unsigned short) get2bytes(stream);
				if (c & 32) {
					n = getch(stream) + 1;
					c -= 32;
				} else {
					n = 1;
				}
				while (n --) {
					target = backdropTexture + 4 * picWidth * t2 + t1 * 4;
					if (c == transCol || c == 2015) {
						target[0] = (GLubyte) 0;
						target[1] = (GLubyte) 0;
						target[2] = (GLubyte) 0;
						target[3] = (GLubyte) 0;
					} else {
						target[0] = (GLubyte) redValue(c);
						target[1] = (GLubyte) greenValue(c);
						target[2] = (GLubyte) blueValue(c);
						target[3] = (GLubyte) 255;
					}
					t1++;
				}
			}
		}
	}

	GLuint tmpTex;

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

	texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture, tmpTex);

	//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

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
void saveCorePNG(Common::WriteStream *stream, GLuint texture, int w, int h) {
	GLint tw, th;

	glBindTexture(GL_TEXTURE_2D, texture);

	getTextureDimensions(texture, &tw, &th);

	GLubyte *image = new GLubyte [tw * th * 4];
	if (!checkNew(image)) return;

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

#ifdef HAVE_GLES2
	GLuint old_fbo, new_fbo;
	GLint old_vp[4];
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&old_fbo);
	glGetIntegerv(GL_VIEWPORT, old_vp);
	glGenFramebuffers(1, &new_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, new_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	glViewport(0, 0, tw, th);
	glReadPixels(0, 0, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
	glViewport(old_vp[0], old_vp[1], old_vp[2], old_vp[3]);
	glDeleteFramebuffers(1, &new_fbo);
#else
	setPixelCoords(true);
	const GLfloat texCoords[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};

	int xoffset = 0;
	while (xoffset < tw) {
		int w = (tw - xoffset < viewportWidth) ? tw - xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < th) {
			int h = (th - yoffset < viewportHeight) ? th - yoffset : viewportHeight;

			glClear(GL_COLOR_BUFFER_BIT);   // Clear The Screen
			const GLfloat vertices[] = {
				(GLfloat) - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat)tw - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat) - xoffset, (GLfloat) - yoffset + th, 0.,
				(GLfloat)tw - xoffset, (GLfloat) - yoffset + th, 0.
			};

			glUseProgram(shader.texture);

			setPMVMatrix(shader.texture);

			drawQuad(shader.texture, vertices, 1, texCoords);

			glUseProgram(0);

			for (int i = 0; i < h; i++) {
				glReadPixels(viewportOffsetX, viewportOffsetY + i, w, 1, GL_RGBA, GL_UNSIGNED_BYTE, image + xoffset * 4 + (yoffset + i) * 4 * tw);
			}
			yoffset += viewportHeight;
		}

		xoffset += viewportWidth;
	}
	setPixelCoords(false);

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fatal("Error saving image!");
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fatal("Error saving image!");
		return;
	}

	png_init_io(png_ptr, writer);

	png_set_IHDR(png_ptr, info_ptr, w, h,
			8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	unsigned char *row_pointers[h];

	for (int i = 0; i < h; i++) {
		row_pointers[i] = image + 4 * i * tw;
	}

	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	delete [] image;
	image = NULL;
#endif
}
#endif

#if 0
void saveCoreHSI(Common::WriteStream *stream, GLuint texture, int w, int h) {
	GLint tw, th;
	glBindTexture(GL_TEXTURE_2D, texture);
	getTextureDimensions(texture, &tw, &th);

	GLushort *image = new GLushort [tw * th];
	if (!checkNew(image)) return;
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, image);
	setPixelCoords(true);

	//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	const GLfloat texCoords[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};

	int xoffset = 0;
	while (xoffset < tw) {
		int w = (tw - xoffset < viewportWidth) ? tw - xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < th) {
			int h = (th - yoffset < viewportHeight) ? th - yoffset : viewportHeight;
			glClear(GL_COLOR_BUFFER_BIT);   // Clear The Screen
			const GLfloat vertices[] = {
				(GLfloat) - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat)w - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat) - xoffset, (GLfloat) - yoffset + h, 0.,
				(GLfloat)w - xoffset, (GLfloat) - yoffset + h, 0.
			};

			glUseProgram(shader.texture);
			setPMVMatrix(shader.texture);
			drawQuad(shader.texture, vertices, 1, texCoords);
			glUseProgram(0);

			for (int i = 0; i < h; i++) {
				glReadPixels(viewportOffsetX, viewportOffsetY + i, w, 1, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, image + xoffset + (yoffset + i)*tw);
			}
			yoffset += viewportHeight;
		}

		xoffset += viewportWidth;
	}
	//glReadPixels(viewportOffsetX, viewportOffsetY, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, data);
	setPixelCoords(false);

	int x, y, lookAhead;
	unsigned short int *fromHere, * lookPointer;

	put2bytes(w, stream);
	put2bytes(h, stream);

	for (y = 0; y < h; y ++) {
		fromHere = image + (y * tw);
		x = 0;
		while (x < w) {
			lookPointer = fromHere + 1;
			for (lookAhead = x + 1; lookAhead < w; lookAhead ++) {
				if (lookAhead - x == 256) break;
				if (* fromHere != * lookPointer) break;
				lookPointer ++;
			}
			if (lookAhead == x + 1) {
				put2bytes((* fromHere) & 65503, stream);
			} else {
				put2bytes(* fromHere | 32, stream);
				putch(lookAhead - x - 1, stream);
			}
			fromHere = lookPointer;
			x = lookAhead;
		}
	}
	delete [] image;
	image = NULL;
}
#endif

#if 0
void saveHSI(Common::WriteStream *stream) {
	saveCorePNG(stream, backdropTextureName, sceneWidth, sceneHeight);
}
#endif

void saveParallaxRecursive(parallaxLayer *me, Common::WriteStream *stream) {
	if (me) {
		saveParallaxRecursive(me->next, stream);
		putch(1, stream);
		put2bytes(me->fileNum, stream);
		put2bytes(me->fractionX, stream);
		put2bytes(me->fractionY, stream);
	}
}

bool getRGBIntoStack(unsigned int x, unsigned int y, stackHandler *sH) {
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
