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
#include "sludge/graphics.h"
#include "sludge/language.h"
#include "sludge/newfatal.h"
#include "sludge/sprbanks.h"
#include "sludge/zbuffer.h"
#include "sludge/backdrop.h"
#include "sludge/movie.h"
#include "sludge/stringy.h"
#include "sludge/CommonCode/specialsettings.h"

namespace Sludge {

uint winWidth, winHeight;
int viewportHeight, viewportWidth;
int viewportOffsetX = 0, viewportOffsetY = 0;

extern float cameraZoom;

bool NPOT_textures = true;

extern int specialSettings;

void setMovieViewport();

extern uint sceneWidth, sceneHeight;
extern zBufferData zBuffer;
extern int lightMapNumber;

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

		videoflags = SDL_SWSURFACE | SDL_FULLSCREEN;

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
		videoflags = SDL_SWSURFACE;

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

	if (EGL_Open()) {
		msgBox("Startup Error", "Couldn't initialize EGL.");
		SDL_Quit();
		exit(1);
	}
	EGL_Init();

	GLint uniform;
	const char *Vertex;
	const char *Fragment;

	Vertex = shaderFileRead("scale.vert");

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

	Fragment = joinStrings("precision mediump float;\n", Fragment);

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

	Fragment = joinStrings("precision mediump float;\n", Fragment);

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

	Fragment = joinStrings("precision mediump float;\n", Fragment);

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

	Fragment = joinStrings("precision mediump float;\n", Fragment);

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

} // End of namespace Sludge
