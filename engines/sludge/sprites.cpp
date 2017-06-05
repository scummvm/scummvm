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

#include "graphics/surface.h"
#include "graphics/transparent_surface.h"
#include "allfiles.h"
#include "fileset.h"
#include "people.h"
#include "sprites.h"
#include "moreio.h"
#include "newfatal.h"
#include "colours.h"
#include "backdrop.h"
#include "sludger.h"
#include "zbuffer.h"
#include "debug.h"
#include "graphics.h"
#include "imgloader.h"
#include "shaders.h"

namespace Sludge {

extern zBufferData zBuffer;

#if 0
extern GLuint backdropTextureName;
#endif

extern inputType input;
extern int cameraX, cameraY;
extern float cameraZoom;
extern Graphics::Surface renderSurface;

unsigned char currentBurnR = 0, currentBurnG = 0, currentBurnB = 0;

void forgetSpriteBank(spriteBank &forgetme) {
#if 0
	deleteTextures(forgetme.myPalette.numTextures, forgetme.myPalette.tex_names);
	if (forgetme.isFont) {
		deleteTextures(forgetme.myPalette.numTextures, forgetme.myPalette.burnTex_names);
		delete [] forgetme.myPalette.burnTex_names;
		forgetme.myPalette.burnTex_names = NULL;
	}

	delete [] forgetme.myPalette.tex_names;
	forgetme.myPalette.tex_names = NULL;
	delete[] forgetme.myPalette.tex_w;
	forgetme.myPalette.tex_w = NULL;
	delete[] forgetme.myPalette.tex_h;
	forgetme.myPalette.tex_h = NULL;
#endif
	if (forgetme.myPalette.pal) {
		delete[] forgetme.myPalette.pal;
		forgetme.myPalette.pal = NULL;
		delete[] forgetme.myPalette.r;
		forgetme.myPalette.r = NULL;
		delete[] forgetme.myPalette.g;
		forgetme.myPalette.g = NULL;
		delete[] forgetme.myPalette.b;
		forgetme.myPalette.b = NULL;
	}

	delete forgetme.sprites;
	forgetme.sprites = NULL;

	// TODO: also remove sprite bank from allLoadedBanks
	// And add a function call for this function to the scripting language
}

bool reserveSpritePal(spritePalette &sP, int n) {
	if (sP.pal) {
		delete[] sP.pal;
		delete[] sP.r;
		delete[] sP.g;
		delete[] sP.b;
	}

	sP.pal = new unsigned short int[n];
	if (!checkNew(sP.pal))
		return false;

	sP.r = new unsigned char[n];
	if (!checkNew(sP.r))
		return false;
	sP.g = new unsigned char[n];
	if (!checkNew(sP.g))
		return false;
	sP.b = new unsigned char[n];
	if (!checkNew(sP.b))
		return false;
	sP.total = n;
	return (bool) (sP.pal != NULL) && (sP.r != NULL) && (sP.g != NULL)
			&& (sP.b != NULL);
}

bool loadSpriteBank(int fileNum, spriteBank &loadhere, bool isFont) {

	int total, spriteBankVersion = 0, howmany = 0, startIndex = 0;
	byte *data;

	setResourceForFatal(fileNum);
	if (!openFileFromNum(fileNum)) return fatal("Can't open sprite bank / font");

	loadhere.isFont = isFont;

	total = bigDataFile->readUint16BE();
	if (!total) {
		spriteBankVersion = bigDataFile->readByte();
		if (spriteBankVersion == 1) {
			total = 0;
		} else {
			total = bigDataFile->readUint16BE();
		}
	}

	if (total <= 0) return fatal("No sprites in bank or invalid sprite bank file");
	if (spriteBankVersion > 3) return fatal("Unsupported sprite bank file format");

	loadhere.total = total;
	loadhere.sprites = new sprite [total];
	if (! checkNew(loadhere.sprites)) return false;
	byte **spriteData = new byte * [total];
	if (! checkNew(spriteData)) return false;

	// version 1, 2, read how many now
	if (spriteBankVersion && spriteBankVersion < 3) {
		howmany = bigDataFile->readByte();
		startIndex = 1;
	}

	// version 3, sprite is png
	if (spriteBankVersion == 3) {
		for (int i = 0; i < total; i ++) {
			howmany = bigDataFile->readByte();
			startIndex = 1;

			loadhere.sprites[i].xhot = getSigned(bigDataFile);
			loadhere.sprites[i].yhot = getSigned(bigDataFile);

			if (!ImgLoader::loadPNGImage(bigDataFile, &loadhere.sprites[i].surface)) {
				return false;
			}
		}
		return true;
	}

	// version 0, 1, 2
	for (int i = 0; i < total; i ++) {
		int picwidth, picheight;
		// load sprite width, height, relative position
		if (spriteBankVersion == 2) {
			picwidth = bigDataFile->readUint16BE();
			picheight = bigDataFile->readUint16BE();
			loadhere.sprites[i].xhot = getSigned(bigDataFile);
			loadhere.sprites[i].yhot = getSigned(bigDataFile);
		} else {
			picwidth = (byte)bigDataFile->readByte();
			picheight = (byte)bigDataFile->readByte();
			loadhere.sprites[i].xhot = bigDataFile->readByte();
			loadhere.sprites[i].yhot = bigDataFile->readByte();
		}

		// init data
		loadhere.sprites[i].surface.create(picwidth, picheight, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
		if (isFont) {
			loadhere.sprites[i].burnSurface.create(picwidth, picheight, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
		}
		data = (byte *)new byte[picwidth * (picheight + 1)];
		if (!checkNew(data)) return false;
		int ooo = picwidth * picheight;
		for (int tt = 0; tt < picwidth; tt++) {
			data[ooo++] = 0;
		}
		spriteData[i] = data;

		// read color
		if (spriteBankVersion == 2) { // RUN LENGTH COMPRESSED DATA
			unsigned size = picwidth * picheight;
			unsigned pip = 0;

			while (pip < size) {
				byte col = bigDataFile->readByte();
				int looper;
				if (col > howmany) {
					col -= howmany + 1;
					looper = bigDataFile->readByte() + 1;
				} else looper = 1;

				while (looper --) {
					data[pip ++] = col;
				}
			}
		} else { // RAW DATA
			size_t bytes_read = bigDataFile->read(data, picwidth * picheight);
			if (bytes_read != picwidth * picheight && bigDataFile->err()) {
				debugOut("Reading error in loadSpriteBank.\n");
			}
		}
	}

	// read howmany for version 0
	if (!spriteBankVersion) {
		howmany = bigDataFile->readByte();
		startIndex = bigDataFile->readByte();
	}

	// Make palette for version 0, 1, 2
	if (!reserveSpritePal(loadhere.myPalette, howmany + startIndex)) return false;
	for (int i = 0; i < howmany; i++) {
		loadhere.myPalette.r[i + startIndex] = (byte)bigDataFile->readByte();
		loadhere.myPalette.g[i + startIndex] = (byte)bigDataFile->readByte();
		loadhere.myPalette.b[i + startIndex] = (byte)bigDataFile->readByte();
		loadhere.myPalette.pal[i + startIndex] = makeColour(loadhere.myPalette.r[i + startIndex], loadhere.myPalette.g[i + startIndex], loadhere.myPalette.b[i + startIndex]);
	}
	loadhere.myPalette.originalRed = loadhere.myPalette.originalGreen = loadhere.myPalette.originalBlue = 255;

	// convert
	for (int i = 0; i < total; i ++) {
		int fromhere = 0;
		int transColour = -1;
		int size = loadhere.sprites[i].surface.w * loadhere.sprites[i].surface.h;
		while (fromhere < size) {
			unsigned char s = spriteData[i][fromhere++];
			if (s) {
				transColour = s;
				break;
			}
		}
		fromhere = 0;
		for (int y = 0; y < loadhere.sprites[i].surface.h; y++) {
			for (int x = 0; x < loadhere.sprites[i].surface.w; x++) {
				byte *target = (byte *)loadhere.sprites[i].surface.getBasePtr(x, y);
				unsigned char s = spriteData[i][fromhere++];
				if (s) {
					target[0] = (byte)255;
					target[1] = (byte)loadhere.myPalette.b[s];
					target[2] = (byte)loadhere.myPalette.g[s];
					target[3] = (byte)loadhere.myPalette.r[s];
					transColour = s;
				} else if (transColour >= 0) {
					target[0] = (byte)0;
					target[1] = (byte)loadhere.myPalette.b[transColour];
					target[2] = (byte)loadhere.myPalette.g[transColour];
					target[3] = (byte)loadhere.myPalette.r[transColour];
				}
				if (isFont) {
					target = (byte *)loadhere.sprites[i].burnSurface.getBasePtr(x, y);
					if (s)
						target[0] = loadhere.myPalette.r[s];
					target[1] = (byte)255;
					target[2] = (byte)255;
					target[3] = (byte)255;
				}
			}
		}
		delete[] spriteData[i];
	}
	delete[] spriteData;
	spriteData = NULL;

	finishAccess();

	setResourceForFatal(-1);

	return true;
}

void pasteSpriteToBackDrop(int x1, int y1, sprite &single,
		const spritePalette &fontPal) {
#if 0
	float tx1 = (float)(single.tex_x) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height) / fontPal.tex_h[single.texNum];

	float btx1;
	float btx2;
	float bty1;
	float bty2;

	int diffX = single.width;
	int diffY = single.height;

	x1 -= single.xhot;
	y1 -= single.yhot;

	if (! NPOT_textures) {
		btx1 = backdropTexW * x1 / sceneWidth;
		btx2 = backdropTexW * (x1 + single.width) / sceneWidth;
		bty1 = backdropTexH * y1 / sceneHeight;
		bty2 = backdropTexH * (y1 + single.height) / sceneHeight;
	} else {
		btx1 = (float) x1 / sceneWidth;
		btx2 = (float)(x1 + single.width) / sceneWidth;
		bty1 = (float) y1 / sceneHeight;
		bty2 = (float)(y1 + single.height) / sceneHeight;
	}

	const GLfloat btexCoords[] = {
		btx1, bty1,
		btx2, bty1,
		btx1, bty2,
		btx2, bty2
	};

	if (x1 < 0) diffX += x1;
	if (y1 < 0) diffY += y1;
	if (x1 + diffX > sceneWidth) diffX = sceneWidth - x1;
	if (y1 + diffY > sceneHeight) diffY = sceneHeight - y1;
	if (diffX < 0) return;
	if (diffY < 0) return;

	setPixelCoords(true);

	int xoffset = 0;
	while (xoffset < diffX) {
		int w = (diffX - xoffset < viewportWidth) ? diffX - xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < diffY) {
			int h = (diffY - yoffset < viewportHeight) ? diffY - yoffset : viewportHeight;

			// Render the sprite to the backdrop
			// (using mulitexturing, so the backdrop is seen where alpha < 1.0)
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glActiveTexture(GL_TEXTURE0);

			glUseProgram(shader.paste);
			GLint uniform = glGetUniformLocation(shader.paste, "useLightTexture");
			if (uniform >= 0) glUniform1i(uniform, 0);// No lighting

			setPMVMatrix(shader.paste);

			setPrimaryColor(fontPal.originalRed / 255.f, fontPal.originalGreen / 255.f, fontPal.originalBlue / 255.f, 1.0f);
			glBindTexture(GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);
			//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			const GLfloat vertices[] = {
				(GLfloat) - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat)single.width - xoffset, (GLfloat) - yoffset, 0.,
				(GLfloat) - xoffset, (GLfloat)single.height - yoffset, 0.,
				(GLfloat)single.width - xoffset, (GLfloat)single.height - yoffset, 0.
			};

			const GLfloat texCoords[] = {
				tx1, ty1,
				tx2, ty1,
				tx1, ty2,
				tx2, ty2
			};

			drawQuad(shader.paste, vertices, 3, texCoords, NULL, btexCoords);

			// Copy Our ViewPort To The Texture
			glUseProgram(0);

			copyTexSubImage2D(GL_TEXTURE_2D, 0, (int)((x1 < 0) ? xoffset : x1 + xoffset), (int)((y1 < 0) ? yoffset : y1 + yoffset), (int)((x1 < 0) ? viewportOffsetX - x1 : viewportOffsetX), (int)((y1 < 0) ? viewportOffsetY - y1 : viewportOffsetY), w, h, backdropTextureName);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}
#endif
	setPixelCoords(false);
}

void burnSpriteToBackDrop(int x1, int y1, sprite &single,
		const spritePalette &fontPal) {
#if 0
	float tx1 = (float)(single.tex_x - 0.5) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width + 0.5) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height + 2) / fontPal.tex_h[single.texNum];

	const GLfloat spriteTexCoords[] = {
		tx1, ty1,
		tx2, ty1,
		tx1, ty2,
		tx2, ty2
	};

	x1 -= single.xhot;
	y1 -= single.yhot - 1;

	float bx1 = (float)x1 * backdropTexW / sceneWidth;
	float by1 = (float)y1 * backdropTexH / sceneHeight;
	float bx2 = (float)(x1 + single.width - 1) * backdropTexW / sceneWidth;
	float by2 = (float)(y1 + single.height - 1) * backdropTexH / sceneHeight;

	const GLfloat backdropTexCoords[] = {
		bx1, by1,
		bx2, by1,
		bx1, by2,
		bx2, by2
	};

	const GLfloat backdropTexCoords2[] = {
		0.0f, 0.0f,
		backdropTexW, 0.0f,
		0.0f, backdropTexH,
		backdropTexW, backdropTexH
	};

	int diffX = single.width + 1;
	int diffY = single.height + 2;

	if (x1 < 0) diffX += x1;
	if (y1 < 0) diffY += y1;
	if (x1 + diffX > sceneWidth) diffX = sceneWidth - x1;
	if (y1 + diffY > sceneHeight) diffY = sceneHeight - y1;
	if (diffX < 0) return;
	if (diffY < 0) return;

	setPixelCoords(true);
	setPrimaryColor(currentBurnR / 255.f, currentBurnG / 255.f, currentBurnB / 255.f, 1.0f);

	GLfloat xoffset = 0.0f;
	while (xoffset < diffX) {
		int w = (diffX - xoffset < viewportWidth) ? diffX - xoffset : viewportWidth;

		GLfloat yoffset = 0.0f;
		while (yoffset < diffY) {
			int h = (diffY - yoffset < viewportHeight) ? diffY - yoffset : viewportHeight;

			const GLfloat backdropVertices[] = {
				-x1 - xoffset, -y1 + yoffset, 0.0f,
				sceneWidth - 1.0f - x1 - xoffset, -y1 + yoffset, 0.0f,
				-x1 - xoffset, sceneHeight - 1.0f - y1 + yoffset, 0.0f,
				sceneWidth - 1.0f - x1 - xoffset, sceneHeight - 1.0f - y1 + yoffset, 0.0f
			};

			const GLfloat spriteVertices[] = {
				-xoffset, -yoffset, 0.0f,
				single.width - 1 - xoffset, -yoffset, 0.0f,
				-xoffset, single.height - 1 - yoffset, 0.0f,
				single.width - 1 - xoffset, single.height - 1 - yoffset, 0.0f
			};

			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glUseProgram(shader.texture);
			setPMVMatrix(shader.texture);

			drawQuad(shader.texture, backdropVertices, 1, backdropTexCoords2);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glActiveTexture(GL_TEXTURE0);

			glUseProgram(shader.paste);
			GLint uniform = glGetUniformLocation(shader.paste, "useLightTexture");
			if (uniform >= 0) glUniform1i(uniform, 0); // No lighting

			setPMVMatrix(shader.paste);

			glBindTexture(GL_TEXTURE_2D, fontPal.burnTex_names[single.texNum]);

//FIXME: Test this some more. Also pasting the backdrop again is not strictly necessary but allows using the paste shader.
			drawQuad(shader.paste, spriteVertices, 3, spriteTexCoords, NULL, backdropTexCoords);
			glUseProgram(0);

			// Copy Our ViewPort To The Texture
			copyTexSubImage2D(GL_TEXTURE_2D, 0, (x1 < 0) ? xoffset : x1 + xoffset, (y1 < 0) ? yoffset : y1 + yoffset, viewportOffsetX, viewportOffsetY, w, h, backdropTextureName);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}
	setPixelCoords(false);
#endif
}

#if 0
extern GLuint backdropTextureName;
#endif

void fontSprite(bool flip, int x, int y, sprite &single, const spritePalette &fontPal) {
	float x1 = (float) x - (float) single.xhot / cameraZoom;
	float y1 = (float) y - (float) single.yhot / cameraZoom;

	// Use Transparent surface to scale and blit
	Graphics::TransparentSurface tmp(single.surface, false);
	Graphics::TransparentSurface tmp2(single.burnSurface, false);
	tmp.blit(renderSurface, x1, y1, (flip? Graphics::FLIP_H : Graphics::FLIP_NONE));
	tmp2.blit(renderSurface, x1, y1, (flip? Graphics::FLIP_H : Graphics::FLIP_NONE), 0, TS_RGB(fontPal.originalRed, fontPal.originalGreen, fontPal.originalBlue));

#if 0
	float x2 = x1 + (float) single.surface.w / cameraZoom;
	float y2 = y1 + (float) single.surface.h / cameraZoom;
	float tx1 = (float) (single.tex_x - 0.5) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float) (single.tex_x + single.width + (flip ? 1.0 : 0.5))
			/ fontPal.tex_w[single.texNum];
	float ty2 = (float) (single.height + 2) / fontPal.tex_h[single.texNum];

	GLfloat vertices[] = {
		x1, y1, 0.0f,
		x2, y1, 0.0f,
		x1, y2, 0.0f,
		x2, y2, 0.0f
	};
	if (flip) {
		vertices[0] = x2;
		vertices[3] = x1;
		vertices[6] = x2;
		vertices[9] = x1;
	}

	const GLfloat texCoords[] = {
		tx1, ty1,
		tx2, ty1,
		tx1, ty2,
		tx2, ty2
	};

	//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_MODULATE instead of decal mixes the colours!
	setPrimaryColor(fontPal.originalRed / 255.f, fontPal.originalGreen / 255.f, fontPal.originalBlue / 255.f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);

	glUseProgram(shader.smartScaler);
	GLuint uniform = glGetUniformLocation(shader.smartScaler, "useLightTexture");
	if (uniform >= 0) glUniform1i(uniform, 0);

	setPMVMatrix(shader.smartScaler);

	if (gameSettings.antiAlias == 1) {
		glUniform1i(glGetUniformLocation(shader.smartScaler, "antialias"), 1);
	} else {
		glUniform1i(glGetUniformLocation(shader.smartScaler, "antialias"), 0);
	}

	glEnable(GL_BLEND);

	drawQuad(shader.smartScaler, vertices, 1, texCoords);

	glDisable(GL_BLEND);
	glUseProgram(0);
#endif
}

void fontSprite(int x, int y, sprite &single, const spritePalette &fontPal) {
	fontSprite(false, x, y, single, fontPal);
}

void flipFontSprite(int x, int y, sprite &single,
		const spritePalette &fontPal) {
	fontSprite(true, x, y, single, fontPal);
}

unsigned char curLight[3];

void setDrawMode(onScreenPerson *thisPerson) {
#if 0
	if (thisPerson->colourmix) {
		//glEnable(GL_COLOR_SUM); FIXME: replace line?
		setSecondaryColor(curLight[0]*thisPerson->r * thisPerson->colourmix / 65025 / 255.f, curLight[1]*thisPerson->g * thisPerson->colourmix / 65025 / 255.f, curLight[2]*thisPerson->b * thisPerson->colourmix / 65025 / 255.f, 1.0f);
	}

	//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	setPrimaryColor(curLight[0] * (255 - thisPerson->colourmix) / 65025.f, curLight[1] * (255 - thisPerson->colourmix) / 65025.f, curLight[2] * (255 - thisPerson->colourmix) / 65025.f, 1.0f - thisPerson->transparency / 255.f);
#endif
}

#if 0
extern GLuint backdropTextureName;
#endif

bool checkColourChange(bool reset);

bool scaleSprite(sprite &single, const spritePalette &fontPal, onScreenPerson *thisPerson, bool mirror) {
	float x = thisPerson->x;
	float y = thisPerson->y;

	float scale = thisPerson->scale;

	if (scale <= 0.05) return false;

	int diffX = (int)(((float)single.surface.w) * scale);
	int diffY = (int)(((float)single.surface.h) * scale);

	float x1, y1, x2, y2;

	if (thisPerson->extra & EXTRA_FIXTOSCREEN) {
		x = x / cameraZoom;
		y = y / cameraZoom;
		if (single.xhot < 0)
			x1 = x - (int)((mirror ? (float)(single.surface.w - single.xhot) : (float)(single.xhot + 1)) * scale / cameraZoom);
		else
			x1 = x - (int)((mirror ? (float)(single.surface.w - (single.xhot + 1)) : (float)single.xhot) * scale / cameraZoom);
		y1 = y - (int)((single.yhot - thisPerson->floaty) * scale / cameraZoom);
		x2 = x1 + (int)(diffX / cameraZoom);
		y2 = y1 + (int)(diffY / cameraZoom);
	} else {
		x -= cameraX;
		y -= cameraY;
		if (single.xhot < 0)
		x1 = x - (int)((mirror ? (float)(single.surface.w - single.xhot) : (float)(single.xhot + 1)) * scale);
		else
		x1 = x - (int)((mirror ? (float)(single.surface.w - (single.xhot + 1)) : (float)single.xhot) * scale);
		y1 = y - (int)((single.yhot - thisPerson->floaty) * scale);
		x2 = x1 + diffX;
		y2 = y1 + diffY;
	}

	// Use Transparent surface to scale and blit
	Graphics::TransparentSurface tmp(single.surface, false);
	tmp.blit(renderSurface, x1, y1, (mirror? Graphics::FLIP_H : Graphics::FLIP_NONE), nullptr, TS_ARGB(255, 255, 255, 255), diffX, diffY);

#if 0
	GLfloat z;

	if ((!(thisPerson->extra & EXTRA_NOZB)) && zBuffer.numPanels) {
		int i;
		for (i = 1; i < zBuffer.numPanels; i++) {
			if (zBuffer.panel[i] >= y + cameraY) {
				i--;
				break;
			}
		}
		z = 0.999 - (double) i * (1.0 / 128.0);
	} else {
		z = -0.5;
	}

	float ltx1, ltx2, lty1, lty2;
	if (! NPOT_textures) {
		ltx1 = lightMap.texW * (x1 + cameraX) / sceneWidth;
		ltx2 = lightMap.texW * (x2 + cameraX) / sceneWidth;
		lty1 = lightMap.texH * (y1 + cameraY) / sceneHeight;
		lty2 = lightMap.texH * (y2 + cameraY) / sceneHeight;
	} else {
		ltx1 = (float)(x1 + cameraX) / sceneWidth;
		ltx2 = (float)(x2 + cameraX) / sceneWidth;
		lty1 = (float)(y1 + cameraY) / sceneHeight;
		lty2 = (float)(y2 + cameraY) / sceneHeight;
	}

	const GLfloat ltexCoords[] = {
		ltx1, lty1,
		ltx2, lty1,
		ltx1, lty2,
		ltx2, lty2
	};

	bool light = !(thisPerson->extra & EXTRA_NOLITE);
	if (light && lightMap.data) {
		if (lightMapMode == LIGHTMAPMODE_HOTSPOT) {
			int lx = (int)(x + cameraX);
			int ly = (int)(y + cameraY);

			if (lx < 0) lx = 0;
			else if (lx >= sceneWidth) lx = sceneWidth - 1;
			if (ly < 0) ly = 0;
			else if (ly >= sceneHeight) ly = sceneHeight - 1;

			GLubyte *target;
			if (! NPOT_textures) {
				target = lightMap.data + (ly * getNextPOT(sceneWidth) + lx) * 4;
			} else {
				target = lightMap.data + (ly * sceneWidth + lx) * 4;
			}
			curLight[0] = target[0];
			curLight[1] = target[1];
			curLight[2] = target[2];
		} else if (lightMapMode == LIGHTMAPMODE_PIXEL) {
			curLight[0] = curLight[1] = curLight[2] = 255;

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, lightMap.name);
			glActiveTexture(GL_TEXTURE0);

		}
	} else {
		curLight[0] = curLight[1] = curLight[2] = 255;
	}
#ifndef HAVE_GLES2
	if (!(thisPerson->extra & EXTRA_RECTANGULAR))
	checkColourChange(true);
#endif
	setDrawMode(thisPerson);

	glBindTexture(GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);

	glEnable(GL_BLEND);

	glUseProgram(shader.smartScaler);
	GLuint uniform = glGetUniformLocation(shader.smartScaler, "useLightTexture");
	if (uniform >= 0) glUniform1i(uniform, light && lightMapMode == LIGHTMAPMODE_PIXEL && lightMap.data);

	setPMVMatrix(shader.smartScaler);

	if (gameSettings.antiAlias == 1) {
		glUniform1i(glGetUniformLocation(shader.smartScaler, "antialias"), 1);
	} else {
		glUniform1i(glGetUniformLocation(shader.smartScaler, "antialias"), 0);
	}

	const GLfloat vertices[] = {
		x1, y1, z,
		x2, y1, z,
		x1, y2, z,
		x2, y2, z
	};

	if (! mirror) {
		GLfloat tx3 = tx1;
		tx1 = tx2;
		tx2 = tx3;
	}
	const GLfloat texCoords[] = {
		tx2, ty1,
		tx1, ty1,
		tx2, ty2,
		tx1, ty2
	};

	drawQuad(shader.smartScaler, vertices, 2, texCoords, ltexCoords);

	glDisable(GL_BLEND);
	glUseProgram(0);

	if (light && lightMapMode == LIGHTMAPMODE_PIXEL) {
		glActiveTexture(GL_TEXTURE1);
		glActiveTexture(GL_TEXTURE0);
	}

	setSecondaryColor(0., 0., 0., 1.);

	// Are we pointing at the sprite?
	if (input.mouseX >= x1 && input.mouseX <= x2 && input.mouseY >= y1 && input.mouseY <= y2) {
		if (thisPerson->extra & EXTRA_RECTANGULAR) return true;
#ifdef HAVE_GLES2
		return true;
#else
		return checkColourChange(false);
#endif
	}
	return false;
#endif
}

// Paste a scaled sprite onto the backdrop
void fixScaleSprite(int x, int y, sprite &single, const spritePalette &fontPal,
		onScreenPerson *thisPerson, int camX, int camY, bool mirror) {
#if 0
	float scale = thisPerson-> scale;
	bool useZB = !(thisPerson->extra & EXTRA_NOZB);
	bool light = !(thisPerson->extra & EXTRA_NOLITE);

	if (scale <= 0.05) return;

	float tx1 = (float)(single.tex_x) / fontPal.tex_w[single.texNum];
	float ty1 = (float) 1.0 / fontPal.tex_h[single.texNum]; //0.0;
	float tx2 = (float)(single.tex_x + single.width) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height + 1) / fontPal.tex_h[single.texNum];

	int diffX = (int)(((float)single.width) * scale);
	int diffY = (int)(((float)single.height) * scale);
	int x1;
	if (single.xhot < 0)
	x1 = x - (int)((mirror ? (float)(single.width - single.xhot) : (float)(single.xhot + 1)) * scale);
	else
	x1 = x - (int)((mirror ? (float)(single.width - (single.xhot + 1)) : (float)single.xhot) * scale);
	int y1 = y - (int)((single.yhot - thisPerson->floaty) * scale);

	float spriteWidth = diffX;
	float spriteHeight = diffY;
	if (x1 < 0) diffX += x1;
	if (y1 < 0) diffY += y1;
	if (x1 + diffX > sceneWidth) diffX = sceneWidth - x1;
	if (y1 + diffY > sceneHeight) diffY = sceneHeight - y1;
	if (diffX < 0) return;
	if (diffY < 0) return;

	GLfloat z;

	if (useZB && zBuffer.numPanels) {
		int i;
		for (i = 1; i < zBuffer.numPanels; i++) {
			if (zBuffer.panel[i] >= y + cameraY) {
				i--;
				break;
			}
		}
		z = 0.999 - (double) i * (1.0 / 128.0);
	} else {
		z = -0.5;
	}

	float ltx1, btx1;
	float ltx2, btx2;
	float lty1, bty1;
	float lty2, bty2;
	if (! NPOT_textures) {
		ltx1 = lightMap.texW * x1 / sceneWidth;
		ltx2 = lightMap.texW * (x1 + spriteWidth) / sceneWidth;
		lty1 = lightMap.texH * y1 / sceneHeight;
		lty2 = lightMap.texH * (y1 + spriteHeight) / sceneHeight;
		btx1 = backdropTexW * x1 / sceneWidth;
		btx2 = backdropTexW * (x1 + spriteWidth) / sceneWidth;
		bty1 = backdropTexH * y1 / sceneHeight;
		bty2 = backdropTexH * (y1 + spriteHeight) / sceneHeight;
	} else {
		btx1 = ltx1 = (float) x1 / sceneWidth;
		btx2 = ltx2 = (float)(x1 + spriteWidth) / sceneWidth;
		bty1 = lty1 = (float) y1 / sceneHeight;
		bty2 = lty2 = (float)(y1 + spriteHeight) / sceneHeight;
	}

	const GLfloat ltexCoords[] = {
		ltx1, lty1,
		ltx2, lty1,
		ltx1, lty2,
		ltx2, lty2
	};

	const GLfloat btexCoords[] = {
		btx1, bty1,
		btx2, bty1,
		btx1, bty2,
		btx2, bty2
	};

	if (light && lightMap.data) {
		if (lightMapMode == LIGHTMAPMODE_HOTSPOT) {
			int lx = x + cameraX;
			int ly = y + cameraY;
			if (lx < 0 || ly < 0 || lx >= sceneWidth || ly >= sceneHeight) {
				curLight[0] = curLight[1] = curLight[2] = 255;
			} else {
				GLubyte *target = lightMap.data + (ly * sceneWidth + lx) * 4;
				curLight[0] = target[0];
				curLight[1] = target[1];
				curLight[2] = target[2];
			}
		} else if (lightMapMode == LIGHTMAPMODE_PIXEL) {
			curLight[0] = curLight[1] = curLight[2] = 255;

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, lightMap.name);

		}
	} else {
		curLight[0] = curLight[1] = curLight[2] = 255;
	}

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, backdropTextureName);
	glActiveTexture(GL_TEXTURE0);

	setPixelCoords(true);
	GLfloat xoffset = 0.0f;
	while (xoffset < diffX) {
		int w = (diffX - xoffset < viewportWidth) ? (int)(diffX - xoffset) : viewportWidth;

		GLfloat yoffset = 0.0f;
		while (yoffset < diffY) {

			int h = (diffY - yoffset < viewportHeight) ? (int)(diffY - yoffset) : viewportHeight;

			// Render the scene - first the old backdrop (so that it'll show through when the z-buffer is active
			//glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			const GLfloat vertices[] = {
				-x1 - xoffset, -y1 - yoffset, 0.0f,
				sceneWidth - x1 - xoffset, -y1 - yoffset, 0.0f,
				-x1 - xoffset, sceneHeight - y1 - yoffset, 0.0f,
				sceneWidth - x1 - xoffset, sceneHeight - y1 - yoffset, 0.0f
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

			// The z-buffer
			if (useZB) {
				glDepthMask(GL_TRUE);
				glClear(GL_DEPTH_BUFFER_BIT);
				drawZBuffer((int)(x1 + xoffset + camX), (int)(y1 + yoffset + camY), false);

				glDepthMask(GL_FALSE);
				glEnable(GL_DEPTH_TEST);
			}

			// Then the sprite
			glUseProgram(shader.paste);
			GLint uniform = glGetUniformLocation(shader.paste, "useLightTexture");
			if (uniform >= 0) glUniform1i(uniform, light && lightMapMode == LIGHTMAPMODE_PIXEL && lightMap.data);
			setPMVMatrix(shader.paste);

			setDrawMode(thisPerson);

			glBindTexture(GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);

			const GLfloat vertices2[] = {
				-xoffset, -yoffset, z,
				spriteWidth - xoffset, -yoffset, z,
				-xoffset, spriteHeight - yoffset, z,
				spriteWidth - xoffset, spriteHeight - yoffset, z
			};

			if (! mirror) {
				GLfloat tx3 = tx1;
				tx1 = tx2;
				tx2 = tx3;
			}
			const GLfloat texCoords2[] = {
				tx2, ty1,
				tx1, ty1,
				tx2, ty2,
				tx1, ty2
			};

			drawQuad(shader.paste, vertices2, 3, texCoords2, ltexCoords, btexCoords);

			setSecondaryColor(0., 0., 0., 1.);
			//glDisable(GL_COLOR_SUM); FIXME: replace line?
			// Copy Our ViewPort To The Texture
			glUseProgram(0);
			copyTexSubImage2D(GL_TEXTURE_2D, 0, (int)((x1 < 0) ? xoffset : x1 + xoffset), (int)((y1 < 0) ? yoffset : y1 + yoffset), (int)((x1 < 0) ? viewportOffsetX - x1 : viewportOffsetX), (int)((y1 < 0) ? viewportOffsetY - y1 : viewportOffsetY), w, h, backdropTextureName);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}

	setPixelCoords(false);
	glUseProgram(0);
#endif
}

} // End of namespace Sludge
