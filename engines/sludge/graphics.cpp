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

#include "sludge/allfiles.h"
#include "sludge/debug.h"
#include "sludge/platform-dependent.h"
#include "sludge/graphics.h"
#include "sludge/language.h"
#include "sludge/newfatal.h"
#include "sludge/sprbanks.h"
#include "sludge/zbuffer.h"
#include "sludge/backdrop.h"
#include "sludge/shaders.h"
#include "sludge/movie.h"
#include "sludge/stringy.h"
#include "sludge/CommonCode/specialsettings.h"

namespace Sludge {

unsigned int winWidth, winHeight;
int viewportHeight, viewportWidth;
int viewportOffsetX = 0, viewportOffsetY = 0;

extern float cameraZoom;

bool NPOT_textures = true;

extern int specialSettings;

void setMovieViewport();

#if 0
extern GLuint backdropTextureName;
extern GLuint snapshotTextureName;
#endif

extern unsigned int sceneWidth, sceneHeight;
extern zBufferData zBuffer;
extern int lightMapNumber;
#if 0
extern GLuint yTextureName;
extern GLuint uTextureName;
extern GLuint vTextureName;
//extern GLubyte * ytex, * utex, * vtex;

shaders shader;
GLfloat aPMVMatrix[16];

void sludgeDisplay();

GLfloat primaryColor[4];
GLfloat secondaryColor[4];
#endif

struct textureList *firstTexture = NULL;

textureList *addTexture() {
	textureList *newTexture = new textureList;
	newTexture->next = firstTexture;
	firstTexture = newTexture;
	return newTexture;
}

#if 0
void deleteTextures(GLsizei n, const GLuint *textures) {
	if (firstTexture == NULL) {
		//debugOut("Deleting texture while list is already empty.\n");
	} else {
		for (int i = 0; i < n; i++) {
			bool found = false;
			textureList *list = firstTexture;
			if (list->name == textures[i]) {
				found = true;
				firstTexture = list->next;
				delete list;
				continue;
			}

			while (list->next) {
				if (list->next->name == textures[i]) {
					found = true;
					textureList *deleteMe = list->next;
					list->next = list->next->next;
					delete deleteMe;
					break;
				}
				list = list->next;
			}
			//if (!found)
			//  debugOut("Deleting texture that was not in list.\n");
		}
	}

	glDeleteTextures(n, textures);

}

void getTextureDimensions(GLuint name, GLint *width, GLint *height) {
	textureList *list = firstTexture;
	while (list) {
		if (list->name == name) {
			*width = list->width;
			*height = list->height;
#if !defined(HAVE_GLES2)
			//For the following test it is assumed that glBindTexture is always
			//called for the right texture before getTextureDimensions.
			GLint tw, th;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tw);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
			if (tw != *width || th != *height) {
				debugOut("Warning: Texture dimensions don't match: They are %ix%i, but SLUDGEs bookkeeping says %ix%i.\n", tw, th, *width, *height);
			}
#endif
			return;
		}
		list = list->next;
	}
	fatal("Texture not found in list.\n");
}

void storeTextureDimensions(GLuint name, GLsizei width, GLsizei height, const char *file, int line) {
	if (! NPOT_textures && !(((height & (height - 1)) == 0) || ((width & (width - 1)) == 0))) {
		debugOut("I was told to create a texture with dimensions %ix%i in %s @ line %d although NPOT textures are disabled.\n", width, height, file, line);
		//height = getNextPOT(height);
		//width = getNextPOT(width);
	}

	textureList *list = firstTexture;
	while (list) {
		if (list->name == name) {
			//debugOut("Texture dimensions are overwritten.\n");
			break;
		}
		list = list->next;
	}
	if (list == NULL) {
		list = addTexture();
	}
	list->name = name;
	list->width = width;
	list->height = height;

}
#endif

#ifdef HAVE_GLES2
void glesCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
	// Work around for broken glCopy(Sub)TexImage2D...
	void *tmp = malloc(width * height * 4);
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
	glTexSubImage2D(target, level, xoffset, yoffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
	free(tmp);
}
void glesCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
	// Work around for broken glCopy(Sub)TexImage2D...
	void *tmp = malloc(width * height * 4);
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
	glTexImage2D(target, level, GL_RGBA, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
	free(tmp);
}
#endif

#if 0
void dcopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border, GLuint name, const char *file, int line) {

	glBindTexture(GL_TEXTURE_2D, name);
#ifdef HAVE_GLES2_
	glesCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
#else
	glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
#endif
}

void dcopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height, GLuint name, const char *file, int line) {
	glBindTexture(GL_TEXTURE_2D, name);
#ifdef HAVE_GLES2_
	glesCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
#else
	glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
#endif
}

void dtexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
		GLint border, GLenum format, GLenum type, const GLvoid *data, GLuint name, const char *file, int line) {
	storeTextureDimensions(name, width, height, file, line);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
}

void dtexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
		GLenum format, GLenum type, const GLvoid *data, GLuint name, const char *file, int line) {
	storeTextureDimensions(name, width, height, file, line);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, data);
}

void setPrimaryColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	primaryColor[0] = r;
	primaryColor[1] = g;
	primaryColor[2] = b;
	primaryColor[3] = a;
}

void setSecondaryColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	secondaryColor[0] = r;
	secondaryColor[1] = g;
	secondaryColor[2] = b;
	secondaryColor[3] = a;
}

void drawQuad(GLint program, const GLfloat *vertices, int numTexCoords, ...) {
	int i, vertexLoc, texCoordLocs[numTexCoords];
	const GLfloat *texCoords[numTexCoords];

	va_list vl;
	va_start(vl, numTexCoords);
	for (i = 0; i < numTexCoords; i++) {
		texCoords[i] = va_arg(vl, const GLfloat *);
	}
	va_end(vl);

	glUniform4f(glGetUniformLocation(program, "myColor"), primaryColor[0], primaryColor[1], primaryColor[2], primaryColor[3]);
	if (program == shader.smartScaler || program == shader.paste) {
		glUniform4f(glGetUniformLocation(program, "mySecondaryColor"), secondaryColor[0], secondaryColor[1], secondaryColor[2], secondaryColor[3]);
	}

	vertexLoc = glGetAttribLocation(program, "myVertex");
	texCoordLocs[0] = glGetAttribLocation(program, "myUV0");
	if (numTexCoords > 1) texCoordLocs[1] = glGetAttribLocation(program, "myUV1");
	if (numTexCoords > 2) texCoordLocs[2] = glGetAttribLocation(program, "myUV2");
	if (numTexCoords > 3) texCoordLocs[3] = glGetAttribLocation(program, "myUV3");

	glEnableVertexAttribArray(vertexLoc);
	glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

	for (i = 0; i < numTexCoords; i++) {
		if (texCoords[i]) {
			glEnableVertexAttribArray(texCoordLocs[i]);
			glVertexAttribPointer(texCoordLocs[i], 2, GL_FLOAT, GL_FALSE, 0, texCoords[i]);
		}
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	for (i = 0; i < numTexCoords; i++) {
		if (texCoords[i]) {
			glDisableVertexAttribArray(texCoordLocs[i]);
		}
	}
	glDisableVertexAttribArray(vertexLoc);

}

void setPMVMatrix(GLint program) {
	glUniformMatrix4fv(glGetUniformLocation(program, "myPMVMatrix"), 1, GL_FALSE, aPMVMatrix);
}
#endif
// This is for swapping settings between rendering to texture or to the screen
void setPixelCoords(bool pixels) {
	static int current = -1;
//	if (current == pixels) return;
	current = pixels;
#if 0
	glBindTexture(GL_TEXTURE_2D, backdropTextureName);

	if (pixels) {

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glClear(GL_COLOR_BUFFER_BIT);

		const GLfloat bPMVMatrix[] = {
			2.0f / viewportWidth, .0, .0, .0,
			.0, 2.0f / viewportHeight, .0, .0,
			.0, .0, 1.0f, .0,
			-1.0, -1.0f, .0, 1.0f

		};
		for (int i = 0; i < 16; i++) {
			aPMVMatrix[i] = bPMVMatrix[i];
		}
	} else {
		if (gameSettings.antiAlias < 0) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		GLfloat w = (GLfloat) winWidth / cameraZoom;
		GLfloat h = (GLfloat) winHeight / cameraZoom;

		const GLfloat bPMVMatrix[] = {
			2.0f / w, .0, .0, .0,
			.0, -2.0f / h, .0, .0,
			.0, .0, 1.0f, .0,
			-1.0, 1.0f, .0, 1.0f

		};
		for (int i = 0; i < 16; i++) {
			aPMVMatrix[i] = bPMVMatrix[i];
		}
	}
#endif
}

int desktopW = 0, desktopH = 0;
bool runningFullscreen = false;

#if defined(HAVE_GLES2)
void saveTexture(GLuint tex, GLubyte *data) {
	// use an FBO to easily grab the texture...
	static GLuint fbo = 0;
	GLuint old_fbo;
	GLint tw, th;
	GLint old_vp[4];
	if (fbo == 0) {
		glGenFramebuffers(1, &fbo);
	}
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	getTextureDimensions(tex, &tw, &th);
	glGetIntegerv(GL_VIEWPORT, old_vp);
	glViewport(0, 0, tw, th);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glReadPixels(0, 0, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glViewport(old_vp[0], old_vp[1], old_vp[2], old_vp[3]);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}
#else
#if 0
void saveTexture(GLuint tex, GLubyte *data) {

	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}
#endif
#endif

// This is for setting windowed or fullscreen graphics.
// Used for switching, and for initial window creation.
void setGraphicsWindow(bool fullscreen, bool restoreGraphics, bool resize) {
#if defined(PANDORA)
	fullscreen = true;
#endif
#if 0
	GLubyte *snapTexture = NULL;

	Uint32 videoflags = 0;

	if (! desktopW) {

		// Get video hardware information
		const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
		desktopW = videoInfo->current_w;
		desktopH = videoInfo->current_h;

	} else if (restoreGraphics && fullscreen == runningFullscreen & ! resize) return;

	runningFullscreen = fullscreen;

	if (restoreGraphics) {
		/*
		 * Save the textures
		 */
		if (backdropTextureName) {
			if (backdropTexture) delete backdropTexture;
			int picWidth = sceneWidth;
			int picHeight = sceneHeight;
			if (! NPOT_textures) {
				picWidth = getNextPOT(picWidth);
				picHeight = getNextPOT(picHeight);
			}
			backdropTexture = new GLubyte [picHeight * picWidth * 4];
			if (! checkNew(backdropTexture)) return;

			saveTexture(backdropTextureName, backdropTexture);
		}
		if (snapshotTextureName) {
			int picWidth = winWidth;
			int picHeight = winHeight;
			if (! NPOT_textures) {
				picWidth = getNextPOT(picWidth);
				picHeight = getNextPOT(picHeight);
			}
			snapTexture = new GLubyte [picHeight * picWidth * 4];
			if (! checkNew(snapTexture)) return;

			saveTexture(snapshotTextureName, snapTexture);
		}
	}

	/*
	 * Set the graphics mode
	 */
	float winAspect = (float) winWidth / winHeight;

	if (fullscreen) {
		specialSettings &= ~SPECIAL_INVISIBLE;
#if !defined(HAVE_GLES2)
		videoflags = SDL_OPENGL | SDL_FULLSCREEN;
#else
		videoflags = SDL_SWSURFACE | SDL_FULLSCREEN;
#endif

		if (gameSettings.fixedPixels) {
			viewportWidth = realWinWidth = winWidth;
			viewportHeight = realWinHeight = winHeight;
			viewportOffsetY = 0;
			viewportOffsetX = 0;
		} else {
			realWinWidth = desktopW;
			realWinHeight = desktopH;

			float realAspect = (float) realWinWidth / realWinHeight;

			if (realAspect > winAspect) {
				viewportHeight = realWinHeight;
				viewportWidth = (int)(realWinHeight * winAspect);
				viewportOffsetY = 0;
				viewportOffsetX = (realWinWidth - viewportWidth) / 2;
			} else {
				viewportWidth = realWinWidth;
				viewportHeight = (int)((float) realWinWidth / winAspect);
				viewportOffsetY = (realWinHeight - viewportHeight) / 2;
				viewportOffsetX = 0;
			}
		}

	} else {
#if !defined(HAVE_GLES2)
		videoflags = SDL_OPENGL/* | SDL_RESIZABLE*/;
#else
		videoflags = SDL_SWSURFACE;
#endif

		if (resize) {
			float realAspect = (float) desktopW / desktopH;

			if (realAspect > winAspect) {
				realWinWidth = (int)(realWinHeight * winAspect);
			} else {
				realWinHeight = (int)(realWinWidth / winAspect);
			}

			realAspect = (float) realWinWidth / realWinHeight;

			if (realAspect > winAspect) {
				viewportHeight = realWinHeight;
				viewportWidth = (int)(realWinHeight * winAspect);
				viewportOffsetY = 0;
				viewportOffsetX = (realWinWidth - viewportWidth) / 2;
			} else {
				viewportWidth = realWinWidth;
				viewportHeight = (int)((float) realWinWidth / winAspect);
				viewportOffsetY = (realWinHeight - viewportHeight) / 2;
				viewportOffsetX = 0;
			}
		} else {

			if (gameSettings.fixedPixels) {
				viewportWidth = realWinWidth = winWidth;
				viewportHeight = realWinHeight = winHeight;
				viewportOffsetY = 0;
				viewportOffsetX = 0;
			} else {
				realWinHeight = desktopH * 3 / 4;
				realWinWidth = (int)(realWinHeight * winAspect);

				if (realWinWidth > desktopW) {
					realWinWidth = desktopW;
					realWinHeight = (int)((float) realWinWidth / winAspect);
				}

				viewportHeight = realWinHeight;
				viewportWidth = realWinWidth;
				viewportOffsetY = 0;
				viewportOffsetX = 0;
			}
		}
	}

	debugHeader();

	if (SDL_SetVideoMode(realWinWidth, realWinHeight, 32, videoflags) == 0) {
		msgBox("Startup Error: Couldn't set video mode.", SDL_GetError());
		SDL_Quit();
		exit(2);
	}
	debugOut("Video mode %d %d set successfully.\n", realWinWidth, realWinHeight);

#if defined(HAVE_GLES2)
	if (EGL_Open()) {
		msgBox("Startup Error", "Couldn't initialize EGL.");
		SDL_Quit();
		exit(1);
	}
	EGL_Init();
#endif

	GLint uniform;
	const char *Vertex;
	const char *Fragment;

	Vertex = shaderFileRead("scale.vert");

#if !defined(HAVE_GLES2)
	Fragment = shaderFileRead("scale.frag");
#else
	/*  const GLubyte *str;
	 int glDerivativesAvailable;
	 str = glGetString (GL_EXTENSIONS);
	 glDerivativesAvailable = (strstr((const char *)str, "GL_OES_standard_derivatives") != NULL);
	 if (!glDerivativesAvailable) {
	 debugOut("Extension \"GL_OES_standard_derivatives\" not available. Advanced anti-aliasing is not possible. Using linear anti-aliasing instead.");
	 gameSettings.antiAlias = -1;
	 */
	Fragment = shaderFileRead("scale_noaa.frag");
//	}

	Fragment = joinStrings("precision mediump float;\n", Fragment);
#endif

	if (! Vertex || ! Fragment) {
		fatal("Error loading \"scale\" shader program!", "Try re-installing the game. (scale.frag, scale_noaa.frag or scale.vert was not found.)");
		gameSettings.antiAlias = -1;
		shader.smartScaler = 0;
	} else {

		shader.smartScaler = buildShaders(Vertex, Fragment);

		if (! shader.smartScaler) {
			fatal("Error building \"scale\" shader program!");
			gameSettings.antiAlias = -1;
			shader.smartScaler = 0;
		} else {
			debugOut("Built shader program: %d (smartScaler)\n", shader.smartScaler);

			glUseProgram(shader.smartScaler);
			uniform = glGetUniformLocation(shader.smartScaler, "Texture");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.smartScaler, "lightTexture");
			if (uniform >= 0) glUniform1i(uniform, 1);
			uniform = glGetUniformLocation(shader.smartScaler, "useLightTexture");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.smartScaler, "antialias");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.smartScaler, "scale");

			float scale = (float)realWinWidth / (float)winWidth * 0.25;
			if (scale > 1.0) scale = 1.0;
			if (uniform >= 0) glUniform1f(uniform, scale);

		}
	}

	Vertex = shaderFileRead("fixScaleSprite.vert");
	Fragment = shaderFileRead("fixScaleSprite.frag");

#if defined(HAVE_GLES2)
	Fragment = joinStrings("precision mediump float;\n", Fragment);
#endif

	if (! Vertex || ! Fragment) {
		fatal("Error loading \"fixScaleSprite\" shader program!", "Try re-installing the game. (fixScaleSprite.frag or fixScaleSprite.vert was not found.)");
		shader.paste = 0;
	} else {

		shader.paste = buildShaders(Vertex, Fragment);
		if (! shader.paste) {
			fatal("Error building \"fixScaleSprite\" shader program!");
		} else {
			debugOut("Built shader program: %d (fixScaleSprite)\n", shader.paste);

			glUseProgram(shader.paste);
			uniform = glGetUniformLocation(shader.paste, "tex0");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.paste, "tex1");
			if (uniform >= 0) glUniform1i(uniform, 1);
			uniform = glGetUniformLocation(shader.paste, "tex2");
			if (uniform >= 0) glUniform1i(uniform, 2);
			uniform = glGetUniformLocation(shader.paste, "useLightTexture");
			if (uniform >= 0) glUniform1i(uniform, 0);

		}
	}

	Vertex = shaderFileRead("yuv.vert");
	Fragment = shaderFileRead("yuv.frag");

#if defined(HAVE_GLES2)
	Fragment = joinStrings("precision mediump float;\n", Fragment);
#endif

	if (! Vertex || ! Fragment) {
		fatal("Error loading \"yuv\" shader program!", "Try re-installing the game. (yuv.frag or yuv.vert was not found.)");
		shader.yuv = 0;
	} else {

		shader.yuv = buildShaders(Vertex, Fragment);
		if (! shader.yuv) {
			fatal("Error building \"yuv\" shader program!");
		} else {
			debugOut("Built shader program: %d (yuv)\n", shader.yuv);

			glUseProgram(shader.yuv);
			uniform = glGetUniformLocation(shader.yuv, "Ytex");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.yuv, "Utex");
			if (uniform >= 0) glUniform1i(uniform, 1);
			uniform = glGetUniformLocation(shader.yuv, "Vtex");
			if (uniform >= 0) glUniform1i(uniform, 2);

		}
	}

	Vertex = shaderFileRead("texture.vert");
	Fragment = shaderFileRead("texture.frag");

#if defined(HAVE_GLES2)
	Fragment = joinStrings("precision mediump float;\n", Fragment);
#endif

	if (! Vertex || ! Fragment) {
		fatal("Error loading \"texture\" shader program!", "Try re-installing the game. (texture.frag or texture.vert was not found.)");
		shader.texture = 0;
	} else {

		shader.texture = buildShaders(Vertex, Fragment);
		if (! shader.texture) {
			fatal("Error building \"texture\" shader program!");
		} else {
			debugOut("Built shader program: %d (texture)\n", shader.texture);

			glUseProgram(shader.texture);
			uniform = glGetUniformLocation(shader.texture, "sampler2d");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.texture, "zBuffer");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.texture, "zBufferLayer");
			if (uniform >= 0) glUniform1f(uniform, 0.);
			uniform = glGetUniformLocation(shader.texture, "modulateColor");
			if (uniform >= 0) glUniform1i(uniform, 0);
		}
	}

	Vertex = shaderFileRead("color.vert");
	Fragment = shaderFileRead("color.frag");

#if defined(HAVE_GLES2)
	Fragment = joinStrings("precision mediump float;\n", Fragment);
#endif

	if (! Vertex || ! Fragment) {
		fatal("Error loading \"color\" shader program!", "Try re-installing the game. (color.frag or color.vert was not found.)");
		shader.color = 0;
	} else {

		shader.color = buildShaders(Vertex, Fragment);
		if (! shader.color) {
			fatal("Error building \"color\" shader program!");
		} else {
			debugOut("Built shader program: %d (color)\n", shader.color);
			glUseProgram(shader.color);
		}
	}
	glUseProgram(0);

	glViewport(viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);

	/*
	 * Set up OpenGL for 2D rendering.
	 */
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setPixelCoords(false);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	if (restoreGraphics) {
		/*
		 * Restore the textures
		 */
		if (backdropTextureName) {
			if (!glIsTexture(backdropTextureName)) {
				glGenTextures(1, &backdropTextureName);
			}
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
			// Restore the backdrop
			texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sceneWidth, sceneHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture, backdropTextureName);

		}
		if (snapshotTextureName) {
			if (!glIsTexture(snapshotTextureName)) {
				glGenTextures(1, &snapshotTextureName);
			}
			glBindTexture(GL_TEXTURE_2D, snapshotTextureName);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			// Restore the backdrop
			texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, winWidth, winHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, snapTexture, snapshotTextureName);
			delete snapTexture;
		}

		if (yTextureName) {
			if (!glIsTexture(yTextureName)) {
				glGenTextures(1, &yTextureName);
				glGenTextures(1, &uTextureName);
				glGenTextures(1, &vTextureName);
			}
		}

		reloadSpriteTextures();
		reloadParallaxTextures();
		zBuffer.texName = 0;
		if (zBuffer.numPanels) {
			setZBuffer(zBuffer.originalNum);
		}
		lightMap.name = 0;
		if (lightMapNumber) {
			loadLightMap(lightMapNumber);
		}

		sludgeDisplay();
	}

	if (movieIsPlaying)
	setMovieViewport();
#endif
}

void setupOpenGLStuff() {

	/*
	 * Time to setup our requested window attributes for our OpenGL window.
	 * We want *at least* 8 bits of red, green and blue. We also want at least a 16-bit
	 * depth buffer.
	 *
	 * The last thing we do is request a double buffered window. '1' turns on double
	 * buffering, '0' turns it off.
	 */
#if 0
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	setGraphicsWindow(gameSettings.userFullScreen, false);
#endif
#if !defined(HAVE_GLES2)
#if 0
	/* Check for graphics capabilities... */
	if (GLEE_VERSION_2_0) {
		// Yes! Textures can be any size!
		NPOT_textures = true;
		debugOut("OpenGL 2.0! All is good.\n");
	} else {
		if (GLEE_VERSION_1_5) {
			debugOut("OpenGL 1.5!\n");
		} else if (GLEE_VERSION_1_4) {
			debugOut("OpenGL 1.4!\n");
		} else if (GLEE_VERSION_1_3) {
			debugOut("OpenGL 1.3!\n");
		} else if (GLEE_VERSION_1_2) {
			debugOut("OpenGL 1.2!\n");
		}

		if (GLEE_ARB_texture_non_power_of_two) {
			// Yes! Textures can be any size!
			NPOT_textures = true;
		} else {
			// Workaround needed for lesser graphics cards. Let's hope this works...
			NPOT_textures = false;
			debugOut("Warning: Old graphics card! GLEE_ARB_texture_non_power_of_two not supported.\n");
		}

		if (GLEE_ARB_shading_language_100) {
			debugOut("ARB_shading_language_100 supported.\n");
		} else {
			debugOut("Warning: Old graphics card! ARB_shading_language_100 not supported. Try updating your drivers.\n");
		}
		if (GLEE_ARB_shader_objects) {
			debugOut("ARB_shader_objects supported.\n");
		} else {
			fatal("Error: Old graphics card! ARB_shader_objects not supported.\n");
		}
		if (GLEE_ARB_vertex_shader) {
			debugOut("ARB_vertex_shader supported.\n");
		} else {
			fatal("Error: Old graphics card! ARB_vertex_shader not supported.\n");
		}
		if (GLEE_ARB_fragment_shader) {
			debugOut("ARB_fragment_shader supported.\n");
		} else {
			fatal("Error: Old graphics card! ARB_fragment_shader not supported.\n");
		}
	}
#else
	NPOT_textures = false;
#endif
#endif
	int n;
#if 0
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint *) &n);
#endif
	debugOut("Max texture image units: %d\n", n);

}

// I found this function on a coding forum on the 'net.
// Looks a bit weird, but it should work.
int getNextPOT(int n) {
	--n;
	n |= n >> 16;
	n |= n >> 8;
	n |= n >> 4;
	n |= n >> 2;
	n |= n >> 1;
	++n;
	return n;
}

int printOglError(const char *file, int line) {
	/* Returns 1 if an OpenGL error occurred, 0 otherwise. */
	int retCode = 0;
#if 0
	GLenum glErr;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
#if !defined(HAVE_GLES2)
		debugOut("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
#else
		debugOut("glError in file %s @ line %d: error code %i\n", file, line, glErr);
#endif
		retCode = 1;
		glErr = glGetError();
	}
#endif
	return retCode;
}

} // End of namespace Sludge
