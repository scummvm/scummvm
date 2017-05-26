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
#include "allfiles.h"
#include "errors.h"
#include "moreio.h"
#include "CommonCode/version.h"
#include "sludger.h"
#include "colours.h"
#include "backdrop.h"
#include "graphics.h"
#include "newfatal.h"

namespace Sludge {

bool freeze();
void unfreeze(bool);    // Because FREEZE.H needs a load of other includes

int thumbWidth = 0, thumbHeight = 0;

#if 0
extern GLuint backdropTextureName;
#endif

#if ALLOW_FILE
bool saveThumbnail(FILE *fp) {
	GLuint thumbnailTextureName = 0;

	put4bytes(thumbWidth, fp);
	put4bytes(thumbHeight, fp);

	if (thumbWidth && thumbHeight) {
		if (! freeze()) return false;

		setPixelCoords(true);
#if 0
		glUseProgram(shader.texture);

		setPMVMatrix(shader.texture);

		glGenTextures(1, &thumbnailTextureName);
		glBindTexture(GL_TEXTURE_2D, thumbnailTextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, thumbWidth, thumbHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0, thumbnailTextureName);

		// Render the backdrop (scaled)
		//glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, backdropTextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


		const GLfloat vertices[] = {
			0., 0., 0.,
			thumbWidth - 1.f, 0., 0.,
			0., thumbHeight - 1.f, 0.,
			thumbWidth - 1.f, thumbHeight - 1.f, 0.
		};

		const GLfloat texCoords[] = {
			0.0f, 0.0f,
			backdropTexW, 0.0f,
			0.0f, backdropTexH,
			backdropTexW, backdropTexH
		};

		drawQuad(shader.texture, vertices, 1, texCoords);

		if (gameSettings.antiAlias < 0) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		deleteTextures(1, &thumbnailTextureName);
		thumbnailTextureName = 0;

		// Save Our ViewPort
#ifdef HAVE_GLES2
		GLushort *image = new GLushort [thumbWidth * thumbHeight];
		GLuint *tmp = new GLuint [thumbWidth * thumbHeight];
		if (! checkNew(image)) return false;
		glReadPixels(viewportOffsetX, viewportOffsetY, thumbWidth, thumbHeight, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
		for (int y = 0; y < thumbHeight; y ++) {
			for (int x = 0; x < thumbWidth; x ++) {
				const GLuint a = tmp[y * thumbWidth + x];
				image[y * thumbWidth + x] = ((a & 0x00f80000) >> (16 + 3)) | ((a & 0x0000fc00) >> (8 + 2 - 5)) | ((a & 0x000000f8) << (11 - 3));
			}
		}
		delete[] tmp;
#else
		GLushort *image = new GLushort [thumbWidth * thumbHeight];
		if (! checkNew(image)) return false;

		glReadPixels(viewportOffsetX, viewportOffsetY, thumbWidth, thumbHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, image);
#endif



		glUseProgram(0);
#endif
		setPixelCoords(false);

		for (int y = 0; y < thumbHeight; y ++) {
			for (int x = 0; x < thumbWidth; x ++) {
				put2bytes((*(image + y * thumbWidth + x)), fp);
			}
		}
		delete[] image;
		image = NULL;
		unfreeze(true);
	}
	fputc('!', fp);
	return true;
}
#endif

void showThumbnail(char *filename, int atX, int atY) {
#if 0
	GLubyte *thumbnailTexture = NULL;
	GLuint thumbnailTextureName = 0;

	GLfloat texCoordW = 1.0;
	GLfloat texCoordH = 1.0;

	int ssgVersion;
	FILE *fp = openAndVerify(filename, 'S', 'A', ERROR_GAME_LOAD_NO, ssgVersion);
	if (ssgVersion >= VERSION(1, 4)) {
		if (fp == NULL) return;
		int fileWidth = get4bytes(fp);
		int fileHeight = get4bytes(fp);

		int picWidth = fileWidth;
		int picHeight = fileHeight;
		if (! NPOT_textures) {
			picWidth = getNextPOT(picWidth);
			picHeight = getNextPOT(picHeight);
			texCoordW = ((double)fileWidth) / picWidth;
			texCoordH = ((double)fileHeight) / picHeight;

		}

		thumbnailTexture = new GLubyte [picHeight * picWidth * 4];
		if (thumbnailTexture == NULL) return;

		int t1, t2;
		unsigned short c;
		GLubyte *target;
		for (t2 = 0; t2 < fileHeight; t2 ++) {
			t1 = 0;
			while (t1 < fileWidth) {
				c = (unsigned short) get2bytes(fp);
				target = thumbnailTexture + 4 * picWidth * t2 + t1 * 4;
				target[0] = (GLubyte) redValue(c);
				target[1] = (GLubyte) greenValue(c);
				target[2] = (GLubyte) blueValue(c);
				target[3] = (GLubyte) 255;
				t1++;
			}
		}

		fclose(fp);

		glGenTextures(1, &thumbnailTextureName);
		glBindTexture(GL_TEXTURE_2D, thumbnailTextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, thumbnailTexture, thumbnailTextureName);

		delete thumbnailTexture;
		thumbnailTexture = NULL;

		if (atX < 0) {
			fileWidth += atX;
			atX = 0;
		}
		if (atY < 0) {
			fileHeight += atY;
			atY = 0;
		}
		if (fileWidth + atX > sceneWidth) fileWidth = sceneWidth - atX;
		if (fileHeight + atY > sceneHeight) fileHeight = sceneHeight - atY;

		setPixelCoords(true);

		glUseProgram(shader.texture);
		setPMVMatrix(shader.texture);

		int xoffset = 0;
		while (xoffset < fileWidth) {
			int w = (fileWidth - xoffset < viewportWidth) ? fileWidth - xoffset : viewportWidth;

			int yoffset = 0;
			while (yoffset < fileHeight) {
				int h = (fileHeight - yoffset < viewportHeight) ? fileHeight - yoffset : viewportHeight;
				glBindTexture(GL_TEXTURE_2D, thumbnailTextureName);
				const GLfloat vertices[] = {
					(GLfloat)fileWidth - 1.f - xoffset, (GLfloat) - yoffset, 0.,
					(GLfloat) - xoffset, (GLfloat) - yoffset, 0.,
					(GLfloat)fileWidth - 1.f - xoffset, (GLfloat)fileHeight - 1.f - yoffset, 0.,
					(GLfloat) - xoffset, (GLfloat)fileHeight - 1.f - yoffset, 0.
				};

				const GLfloat texCoords[] = {
					texCoordW, 0.0f,
					0.0f, 0.0f,
					texCoordW, texCoordH,
					0.0f, texCoordH
				};

				drawQuad(shader.texture, vertices, 1, texCoords);
				glDisable(GL_BLEND);
				// Copy Our ViewPort To The Texture
				copyTexSubImage2D(GL_TEXTURE_2D, 0, atX + xoffset, atY + yoffset, viewportOffsetX, viewportOffsetY, w, h, backdropTextureName);

				yoffset += viewportHeight;
			}
			xoffset += viewportWidth;
		}
		glUseProgram(0);

		setPixelCoords(false);
		deleteTextures(1, &thumbnailTextureName);

		thumbnailTextureName = 0;
	}
#endif
}

#if ALLOW_FILE
bool skipThumbnail(FILE *fp) {
	thumbWidth = get4bytes(fp);
	thumbHeight = get4bytes(fp);
	uint32_t skippy = thumbWidth;
	skippy *= thumbHeight << 1;
	fseek(fp, skippy, 1);
	return (fgetc(fp) == '!');
}
#endif

} // End of namespace Sludge
