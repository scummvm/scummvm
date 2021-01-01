/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"
#include "engines/icb/surface_manager.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"

#include "common/config-manager.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/tinygl/zgl.h"
#include "graphics/tinygl/zblit.h"

namespace ICB {

#if defined (SDL_BACKEND) && defined (ENABLE_OPENGL)
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffers;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffers;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatus;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2D;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffers;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffers;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbuffer;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorage;

GLuint g_RGBFrameBuffer;

#endif

#define FIRST_CLIENT_SURFACE 2

uint32 working_buffer_id;
uint32 bg_buffer_id;
uint32 effect_time; // Time spent doing postprocessing effects (fades ect)
uint32 flipTime;

_surface::~_surface() {
	// Is the surface there
	if (m_dds) {
		// Make sure it's unlocked
		if (m_locked) {
			//SDL_UnlockSurface(m_dds);
		}
		m_dds->free();
		delete m_dds;
	}
}

_surface::_surface() {
	m_dds = NULL;
	m_name = "Created";
	m_width = 0;
	m_height = 0;
	m_locked = FALSE8;
}

void _surface_manager::PrintDebugLabel(const char *mess, uint32 c) {
	static int y = 100;

	if (mess == NULL) {
		y = c;
	} else {
		/*      Get_surface_DC( working_buffer_id, dc );
		        SetBkColor( dc, 0x00000000 );
		        SetTextColor( dc, c );
		        TextOut( dc, 10, y, mess, strlen( mess ) );
		        Release_surface_DC( working_buffer_id, dc);*/
		y += 15;
	}
}

void _surface_manager::PrintTimer(char label, uint32 time, uint32 limit) {
/*  static uint32 colours[] = { 0x0000ff00, // 0x - 1x
                                  0x0000ffff,   // 1x - 2x
                                  0x000000ff,   // 2x - 3x
                                  0x00ffff00,   // 3x - 4x
                                  0x00ff0000,   // 4x - 5x
                                  0x00ffffff }; // 5x +
*/
	static int x = 20;

	if (limit == 0) {
		x = 20;
	} else {
		float perc = (float)time / (float)limit;
		int percIndex = (int)perc;
		if (percIndex < 0)
			percIndex = 0;
		if (percIndex > 5)
			percIndex = 5;
		char message[64];
		sprintf(message, "%c%3.1f", label, perc);
		/*      Get_surface_DC( working_buffer_id, dc );
		        SetBkColor( dc, colours[percIndex] );
		        SetTextColor( dc, 0x01010101 );
		        TextOut( dc, x, 0, message, strlen( message ) );
		        Release_surface_DC( working_buffer_id, dc);*/
		x += 54;
	}
}

_surface_manager::_surface_manager() {
	// Setup uninitialized pointers
	sdl_screen = NULL;

	// set these up only once
	full_rect.left = 0;
	full_rect.right = SCREEN_WIDTH;
	full_rect.top = 0;
	full_rect.bottom = SCREEN_DEPTH;

	m_screen_rect.left = m_screen_rect.top = 0;
	m_screen_rect.right = SCREEN_WIDTH;
	m_screen_rect.bottom = SCREEN_DEPTH;

	// Set up border properties
	m_borders = m_screen_rect;
	BorderRed() = (uint8)0x00;
	BorderGreen() = (uint8)0x00;
	BorderBlue() = (uint8)0x00;
	BorderRed() = (uint8)0x00;
	m_borderMode = 0;

	// Set up the fade properties
	FadeFromRed() = FadeFromGreen() = FadeFromBlue() = FadeToRed() = FadeToGreen() = FadeToBlue() = FadeAlpha() = (uint8)0x00;
	FadeMode() = 0;

	// Make sure the list of surfaces is empty
	m_Surfaces.Reset();

	// Set/Clear the MMX flag
	m_hasMMX = TRUE8;
}

_surface_manager::~_surface_manager() {
	// Print surface info
	Zdebug("*SURFACE_MANAGER* Destroying Surface Manager with %d surfaces still active", m_Surfaces.GetNoItems());

	// Release the surfaces ( the Reset call calls the destructor for each non null surface )
	m_Surfaces.Reset();
	//sdl_screen->free();
	//delete sdl_screen;

#ifndef ENABLE_OPENGL
#ifdef USE_SDL_DIRECTLY
	if (sdl_screen_texture)
		SDL_DestroyTexture(sdl_screen_texture);
	if (sdl_renderer)
		SDL_DestroyRenderer(sdl_renderer);
#endif
#else
	glDeleteFramebuffers(1, &RGBFrameBufferTexture);
	glDeleteTextures(1, &RGBFrameBufferTexture);
	glDeleteTextures(1, &sdlTextureId);
#endif
	// Finished
	Zdebug("*SURFACE_MANAGER* Surface Manager Destroyed");
}

uint32 _surface_manager::Init_direct_draw() {
	// Debug info
	Zdebug("*SURFACE_MANAGER* Initalizing the SDL video interface");

	g_system->setWindowCaption(Common::U32String("In Cold Blood (C)2000 Revolution Software Ltd"));
	initGraphics(640, 480, nullptr);

	_zb = new TinyGL::FrameBuffer(640, 480, g_system->getScreenFormat()); // TODO: delete
	TinyGL::glInit(_zb, 256);

	sdl_screen = new Graphics::Surface();
	sdl_screen->create(640, 480, Graphics::PixelFormat(4, 8, 8, 8, 0, 24, 16, 8, 0));

	if (!sdl_screen->getBasePtr(0, 0)) {
		//		SDL_Quit();
		Fatal_error("Initialise Graphics::Surface::create failed");
	}

#ifndef ENABLE_OPENGL
#ifdef USE_SDL_DIRECTLY
	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
	if (!sdl_renderer) {
		SDL_Quit();
		Fatal_error("Initialise SDL_CreateRenderer failed");
	}

	sdl_screen_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 640, 480);
	if (!sdl_screen_texture) {
		SDL_Quit();
		Fatal_error("Initialise SDL_CreateTexture failed");
	}
#endif
#else
	if (!SDL_GL_ExtensionSupported("GL_EXT_framebuffer_object")) {
		SDL_Quit();
		Fatal_error("GL_EXT_framebuffer_object not supported!");
	}
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenRenderbuffers");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffers");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindRenderbuffer");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)SDL_GL_GetProcAddress("glRenderbufferStorage");

	glGenTextures(1, &RGBFrameBufferTexture);
	glBindTexture(GL_TEXTURE_2D, RGBFrameBufferTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glGenFramebuffers(1, &RGBFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, RGBFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RGBFrameBufferTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		SDL_Quit();
		Fatal_error("glCheckFramebufferStatus: status is not complete!");
	}
	glGenRenderbuffers(1, &renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	g_RGBFrameBuffer = RGBFrameBuffer;

	glGenTextures(1, &sdlTextureId);
	glBindTexture(GL_TEXTURE_2D, sdlTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, sdl_screen->w, sdl_screen->h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sdl_screen->getBasePtr(0, 0));

#endif

	// Make our back buffer type affair
	m_Surfaces[0] = new _surface;
	m_Surfaces[0]->m_locked = false;
	m_Surfaces[0]->m_width = SCREEN_WIDTH;
	m_Surfaces[0]->m_height = SCREEN_DEPTH;
	m_Surfaces[0]->m_name = "backbuffer";
	m_Surfaces[0]->m_dds = sdl_screen;

	working_buffer_id = 0;

	// Finished
	Zdebug("*SURFACE_MANAGER* SDL video interface Ready");

	return true;
}

void _surface_manager::Reset_Effects() {
	// Reset the border properties
	m_borders = m_screen_rect;
	BorderRed() = (uint8)0x00;
	BorderGreen() = (uint8)0x00;
	BorderBlue() = (uint8)0x00;
	BorderRed() = (uint8)0x00;
	m_borderMode = 0;

	// Reset the fade properties
	FadeFromRed() = (uint8)0x00;
	FadeFromGreen() = (uint8)0x00;
	FadeFromBlue() = (uint8)0x00;
	FadeToRed() = (uint8)0x00;
	FadeToGreen() = (uint8)0x00;
	FadeToBlue() = (uint8)0x00;
	FadeAlpha() = (uint8)0x00;
	FadeMode() = 0;
}

void _surface_manager::Flip() {
	// Draw Frame rate monitor if it's switched on
	static uint32 g_fpsCounter = 0;
	static float g_fpsTotalTime = 0.0f;
	static float g_fpsEndTime = 0.0f;
	static float g_fpsStartTime = 0.0f;

	g_fpsEndTime = (float)GetMicroTimer();
	float nowTime = (g_fpsEndTime - g_fpsStartTime) / 1000.0f;
	g_fpsStartTime = g_fpsEndTime;

	if ((nowTime > 1000.0f) || (nowTime < 1.0f))
		nowTime = 83.0f;

	g_fpsTotalTime += nowTime;
	//float averageFps = (float)(g_fpsTotalTime / ((double)g_fpsCounter + 1.0f));

	//float currentFPS = (float)(1000.0f / (double)nowTime);
	//float averageFPS = (float)(1000.0f / (double)averageFps);

	if ((++g_fpsCounter) > 0xffffff00) {
		g_fpsTotalTime = 0.0f;
		g_fpsCounter = 0;
	}

	// Print the timers
	//char fpsMessage[64];
	//HDC dc;
	//sprintf( fpsMessage,"%3.1f FPS (%3.1f Average FPS)", currentFPS, averageFPS);
	//surface_manager->Get_surface_DC( working_buffer_id, dc );
	//SetBkColor( dc, 0x00000000 );
	//SetTextColor( dc, 0x00c08020 );
	//TextOut( dc, 420, 460, fpsMessage, strlen( fpsMessage ) );
	//surface_manager->Release_surface_DC( working_buffer_id, dc);

	flipTime = GetMicroTimer();

#ifdef ENABLE_OPENGL
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1.0, 1.0, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor4f(1.0, 1.0, 1.0, 1.0);

	glDepthMask(GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, sdlTextureId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sdl_screen->w, sdl_screen->h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sdl_screen->getBasePtr(0, 0));

	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 0);

	glTexCoord2i(sdl_screen->w - 1, 0);
	glVertex2i(sdl_screen->w - 1, 0);

	glTexCoord2i(sdl_screen->w - 1, sdl_screen->h - 1);
	glVertex2i(sdl_screen->w - 1, sdl_screen->h - 1);

	glTexCoord2i(0, sdl_screen->h - 1);
	glVertex2i(0, sdl_screen->h - 1);
	glEnd();

	glDepthMask(GL_TRUE);
	glDisable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	SDL_GL_SwapWindow(sdl_window);
#else
#ifdef USE_SDL_DIRECTLY
	void *pixels;
	int pitch;
	if (SDL_LockTexture(sdl_screen_texture, NULL, &pixels, &pitch) < 0) {
		Fatal_error("_surface_manager::Flip() Failed Lock texture");
	}
	memcpy(pixels, sdl_screen->pixels, pitch * sdl_screen->h);

	SDL_UnlockTexture(sdl_screen_texture);
	SDL_RenderCopy(sdl_renderer, sdl_screen_texture, NULL, NULL);
	SDL_RenderPresent(sdl_renderer);
#else
	Graphics::BlitImage *blitImage = Graphics::tglGenBlitImage();
	Graphics::tglUploadBlitImage(blitImage, *sdl_screen, 0, false);
	Graphics::tglBlitFast(blitImage, 0, 0);
	TinyGL::tglPresentBuffer();
	g_system->copyRectToScreen(_zb->getPixelBuffer(), _zb->linesize,
	                           0, 0, _zb->xsize, _zb->ysize);
	g_system->updateScreen();
	Graphics::tglDeleteBlitImage(blitImage);
#endif
#endif
	flipTime = GetMicroTimer() - flipTime;

	PrintDebugLabel(NULL, 0x00000000);
	PrintTimer('\0', 0, 0);
}

// Returns 0 on error
uint32 _surface_manager::Create_new_surface(const char *name, uint32 width, uint32 height, uint32 /*type*/) {
	// Find the next free slot
	uint32 slot;
	for (slot = FIRST_CLIENT_SURFACE; slot < m_Surfaces.GetNoItems() && m_Surfaces[slot] != NULL; slot++)
		;

	// Create the new surface structure
	m_Surfaces[slot] = new _surface;

	// Initalize it
	m_Surfaces[slot]->m_locked = false;
	m_Surfaces[slot]->m_width = width;
	m_Surfaces[slot]->m_height = height;
	m_Surfaces[slot]->m_name = name;
	m_Surfaces[slot]->m_dds = new Graphics::Surface;
	m_Surfaces[slot]->m_dds->create(width, height, Graphics::PixelFormat(4, 8, 8, 8, 0, 24, 16, 8, 0));

	if (m_Surfaces[slot]->m_dds)
		return slot;

	delete m_Surfaces[slot];
	m_Surfaces[slot] = NULL;

	return 0;
}

void _surface_manager::Kill_surface(uint32 s_id) {
	if (m_Surfaces[s_id] == NULL)
		return; // Already killed

	delete m_Surfaces[s_id];
	m_Surfaces[s_id] = NULL;
}

uint8 *_surface_manager::Lock_surface(uint32 s_id) {
	_surface *pSurface = m_Surfaces[s_id];

	if (pSurface->m_locked)
		Fatal_error("_surface_manager::Lock_surface( %s ) - Already locked and doesn't support multiple locking", (const char *)pSurface->m_name);

	if (!m_Surfaces[s_id]->m_dds) {
		Fatal_error("_surface_manager::Lock_surface( %s ) - surface is null :O", (const char *)m_Surfaces[s_id]->m_name);
		error("Should exit with error-code -1");
	}

	// lock the surface
	/*	if (SDL_LockSurface(pSurface->m_dds) < 0) {
	                Fatal_error("_surface_manager::Lock_surface( %s ) - SDL_LockSurface failed", (const char *)pSurface->m_name);
	                exit(-1);
	        }*/
	pSurface->m_locked = TRUE8;

	return ((uint8 *)pSurface->m_dds->getBasePtr(0, 0));
}

int _surface_manager::Get_pitch(uint32 s_id) {
	if (m_Surfaces[s_id]->m_locked == false) {
		Fatal_error("**Get_pitch %s - surface is unlocked :O", (const char *)m_Surfaces[s_id]->m_name);
		error("Should exit with error-code -1");
	}

	if (!m_Surfaces[s_id]->m_dds) {
		Fatal_error("**Get_pitch %s - surface is null :O", (const char *)m_Surfaces[s_id]->m_name);
		error("Should exit with error-code -1");
	}
	return (m_Surfaces[s_id]->m_dds->pitch);
}

void _surface_manager::Unlock_surface(uint32 s_id) {
	_surface *pSurface = m_Surfaces[s_id];
	if (!pSurface->m_dds) {
		Fatal_error("**Unlock_surface %s - surface is null :O", (const char *)m_Surfaces[s_id]->m_name);
		error("Should exit with error-code -1");
	}
	// SDL_UnlockSurface(pSurface->m_dds);
	m_Surfaces[s_id]->m_locked = FALSE8;
}

void _surface_manager::Fill_surface(uint32 s_id, uint32 rgb_value) {
	m_Surfaces[s_id]->m_dds->fillRect(Common::Rect(0, 0, m_Surfaces[s_id]->m_dds->h, m_Surfaces[s_id]->m_dds->w), rgb_value);
}

void _surface_manager::Blit_surface_to_surface(uint32 from_id, uint32 to_id, LRECT *pSrcRect, LRECT *pDestRect, uint32 dwFlags) {
	Common::Rect srcRect, dstRect;
	if (pSrcRect) {
		if ((pSrcRect->right - pSrcRect->left) < 1)
			return;
		if ((pSrcRect->bottom - pSrcRect->top) < 1)
			return;
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	}

	if (pDestRect) {
		dstRect.left = pDestRect->left;
		dstRect.top = pDestRect->top;
		dstRect.right = pDestRect->right;
		dstRect.bottom = pDestRect->bottom;
	}

	Graphics::Surface *destSurface = m_Surfaces[to_id]->m_dds;
	Graphics::Surface *srcSurface = m_Surfaces[from_id]->m_dds;

	if (dwFlags == 0) {
		//SDL_SetColorKey(m_Surfaces[from_id]->m_dds, SDL_FALSE, 0);
		//printf("------------ Blit_surface_to_surface: dwFlags is 0 --------!!!!!!\n");
	}

	// TODO: Check that the sizes match.
	if (pDestRect) {
		if (pSrcRect) {
			destSurface->copyRectToSurface(*srcSurface, dstRect.left, dstRect.top, srcRect);
		} else {
			destSurface->copyRectToSurface(*srcSurface, dstRect.left, dstRect.top, Common::Rect(0, 0, srcSurface->w, srcSurface->h));
		}
	} else {
		if (pSrcRect) {
			destSurface->copyRectToSurface(*srcSurface, 0, 0, srcRect);
		} else {
			destSurface->copyRectToSurface(*srcSurface, 0, 0, Common::Rect(0, 0, srcSurface->w, srcSurface->h));
		}
	}
	//SDL_BlitSurface(srcSurface, pSrcRect ? &srcRect : NULL, destSurface, pDestRect ? &dstRect : NULL);
}

void _surface_manager::Blit_fillfx(uint32 surface_id, LRECT *rect, uint32 col) {
	Common::Rect r;
	if (rect) {
		r.left = rect->left;
		r.top = rect->top;
		r.right = rect->right;
		r.bottom = rect->bottom;
	}

	if (rect) {
		m_Surfaces[surface_id]->m_dds->fillRect(r, col);
	} else {
		m_Surfaces[surface_id]->m_dds->fillRect(Common::Rect(0, 0, m_Surfaces[surface_id]->m_dds->w, m_Surfaces[surface_id]->m_dds->h), col);
	}
}

void _surface_manager::Set_transparent_colour_key(uint32 nSurfaceID, uint32 nKey) {
	warning("TODO: Handle color-key");
	//SDL_SetColorKey(m_Surfaces[nSurfaceID]->m_dds, SDL_TRUE, nKey);
}

void _surface_manager::DrawEffects(uint32 surface_id) {
	effect_time = GetMicroTimer();

	// Are we fading ?
	if (FadeMode() && FadeAlpha() != 0) {
		if (FadeMode() == 1) {
			/* Additive Fade
			    S'xy = Sxy + ( Alpha * Blend Colour )
			*/

			/* Linear Interpolation between FromColour and ToColour */
			uint8 r = (uint8)(FadeFromRed() + ((FadeAlpha() * (FadeToRed() - FadeFromRed())) >> 8));
			uint8 g = (uint8)(FadeFromGreen() + ((FadeAlpha() * (FadeToGreen() - FadeFromGreen())) >> 8));
			uint8 b = (uint8)(FadeFromBlue() + ((FadeAlpha() * (FadeToBlue() - FadeFromBlue())) >> 8));

			/* Move blend colours into MMX friendly structure */
			uint8 additive[8];
			additive[4] = additive[0] = b;
			additive[5] = additive[1] = g;
			additive[6] = additive[2] = r;
			additive[7] = additive[3] = 0x00;

			// Yep.  Lock the working buffer
			uint8 *pixels = Lock_surface(surface_id);
			uint32 pitch = Get_pitch(surface_id);

#if 1
			for (uint32 lines = 0; lines < SCREEN_DEPTH; lines++) {
				for (int xPos = 0; xPos < SCREEN_WIDTH; xPos++) {
					// 32-bit BGRA pixel
					uint8 *pixel = &pixels[xPos * 4];
					// Add from RGB components
					for (int i = 0; i < 3; i++) {
						pixel[i] = MIN(255, pixel[i] + additive[i]);
					}
				}
				// Next line
				pixels += pitch;
			}
#else
			_asm {
				lea  edi, additive          ; // Get the address of the blend colour block
				movq MM0, [edi]             ; // Load the blend colour
				mov  esi, pixels            ; // Fill esi with the address for the first row of screen memory
				mov  edx, SCREEN_DEPTH      ; // Row counter
				sub  esi, 8                 ; // Ecx is our index and counter, so move the pointer back to compensate for it's 1 out ness
				mov  eax, pitch         ; // Distance to move from the end of one line to the start of the next
				mov  ebx, (SCREEN_WIDTH/2)  ;

				additive_fade_row_loop:
				mov  ecx, ebx               ; // Pixel Counter ( 2 pixels at a time )
				additive_fade_loop:
				movq    MM1, [esi + ecx * 8]    ; // Load 2 pixels
				paddusb MM1, MM0            ; // Add the blend colour (with byte saturation)
				movq    [esi + ecx * 8], MM1    ; // Store 2 pixels
				dec     ecx                 ; // Reduce counter/index
				jne     additive_fade_loop  ; // On to the next 2 pixels
				add esi, eax                ; // Add the increment to get to the start of the next row
				dec edx                     ; // Reduce the row counter
				jne additive_fade_row_loop // And off again
				EMMS; /* Reset FPU/MMX */
			}
#endif
			Unlock_surface(surface_id);
		} else if (FadeMode() == 2) { /* SUBTRACTIVE FADE */
			uint8 r = (uint8)(FadeFromRed() + ((FadeAlpha() * (FadeToRed() - FadeFromRed())) >> 8));
			uint8 g = (uint8)(FadeFromGreen() + ((FadeAlpha() * (FadeToGreen() - FadeFromGreen())) >> 8));
			uint8 b = (uint8)(FadeFromBlue() + ((FadeAlpha() * (FadeToBlue() - FadeFromBlue())) >> 8));

			uint8 subtractive[8];
			subtractive[4] = subtractive[0] = b;
			subtractive[5] = subtractive[1] = g;
			subtractive[6] = subtractive[2] = r;
			subtractive[7] = subtractive[3] = 0x00;

			// Yep.  Lock the working buffer
			uint8 *pixels = Lock_surface(surface_id);
			uint32 pitch = Get_pitch(surface_id);

#if 1
			for (uint32 lines = 0; lines < SCREEN_DEPTH; lines++) {
				for (int xPos = 0; xPos < SCREEN_WIDTH; xPos++) {
					// 32-bit BGRA pixel
					uint8 *pixel = &pixels[xPos * 4];
					// Subtract from RGB components
					for (int i = 0; i < 3; i++) {
						pixel[i] = MAX(0, pixel[i] - subtractive[i]);
					}
				}
				// Next line
				pixels += pitch;
			}
#else
			_asm {
				lea  edi, subtractive       ; // Get the address of the blend colour block
				movq MM0, [edi]             ; // Load the blend colour
				mov  esi, pixels            ; // Fill esi with the address for the first row of screen memory
				mov  edx, SCREEN_DEPTH      ; // Row counter
				sub  esi, 8                 ; // Ecx is our index and counter, so move the pointer back to compensate for it's 1 out ness
				mov  eax, pitch             ; // Distance to move from the end of one line to the start of the next
				mov  ebx, (SCREEN_WIDTH/2)  ;

				subtractive_fade_row_loop:
				mov  ecx, ebx               ; // Pixel Counter ( 2 pixels at a time )
				subtractive_fade_loop:
				movq    MM1, [esi + ecx * 8]        ; // Load 2 pixels
				psubusb MM1, MM0                ; // Do the add
				movq    [esi + ecx * 8], MM1        ; // Store 2 pixels
				dec     ecx                     ; // Reduce counter/index
				jne     subtractive_fade_loop   ; // On to the next 2 pixels
				add esi, eax                ; // Add the increment to get to the start of the next row
				dec edx                     ; // Reduce the row counter
				jne subtractive_fade_row_loop // And off again
				EMMS; /* Reset FPU/MMX */
			}
#endif

			Unlock_surface(surface_id);
		} else if (FadeMode() == 3) { /* ALPHA FADE */
			// Flip FadeAlpha() so it's 0 for fully blended and 255 for no blend
			uint8 fA = (uint8)(255 - FadeAlpha());
			// Create multiplier mask
			uint8 multiplier[8];
			multiplier[4] = multiplier[0] = fA;
			multiplier[5] = multiplier[1] = 0x00;
			multiplier[6] = multiplier[2] = fA;
			multiplier[7] = multiplier[3] = 0x00;

			uint16 r = (uint16)(fA * FadeFromRed() + ((255 - fA) * FadeToRed()));
			uint16 g = (uint16)(fA * FadeFromGreen() + ((255 - fA) * FadeToGreen()));
			uint16 b = (uint16)(fA * FadeFromBlue() + ((255 - fA) * FadeToBlue()));

			// Pre calculate the fade colour component ... 0 <= alpha <= 128
			uint8 blendColour[8];
			*((uint16 *)&blendColour[0]) = b;
			*((uint16 *)&blendColour[2]) = g;
			*((uint16 *)&blendColour[4]) = r;
			*((uint16 *)&blendColour[6]) = (uint16)0x0000;

			// Yep.  Lock the working buffer
			uint8 *pixels = Lock_surface(surface_id);
			uint32 pitch = Get_pitch(surface_id);

#if 1
			for (uint32 lines = 0; lines < SCREEN_DEPTH; lines++) {
				for (int xPos = 0; xPos < SCREEN_WIDTH; xPos++) {
					// 32-bit BGRA pixel
					uint8 *pixel = &pixels[xPos * 4];
					// Subtract from RGB components
					for (int i = 0; i < 3; i++) {
						pixel[i] = (pixel[i] * multiplier[i] + blendColour[i * 2]) >> 8;
					}
				}
				// Next line
				pixels += pitch;
			}
#else
			_asm {
				mov  ebx, (SCREEN_WIDTH/2);
				lea  esi, multiplier;
				lea  edi, blendColour;
				movq MM0, [esi]         ; // Load the fade subtractive thingy
				movq MM1, [edi]         ; // Load the fade subtractive thingy
				mov  esi, pixels        ;
				mov  edx, SCREEN_DEPTH  ; // Row counter
				sub  esi, 8             ; // Ecx is our index and counter, so move the pointer back to compensate for it's 1 out ness
				pxor MM2, MM2           ; // Clear MM2
				mov  eax, pitch     ; // Distance to move from the end of one line to the start of the next

				alpha_fade_row_loop:
				mov  ecx, ebx           ; // Pixel Counter ( 2 pixels at a time )
				alpha_fade_loop:
				movq      MM3, [esi + ecx * 8]   ; // Load 2 pixels
				movq      MM4, MM3
				punpckhbw MM3, MM2          ; // Unpack the pixels into words
				punpcklbw MM4, MM2          ; // Unpack the pixels into words
				pmullw    MM3, MM0
				pmullw    MM4, MM0
				paddusw   MM3, MM1
				paddusw   MM4, MM1
				psrlw     MM3, 8
				psrlw     MM4, 8
				packuswb  MM4, MM3
				movq      [esi + ecx * 8], MM4   ; // Store the
			                                                                                                                                                // result
				dec       ecx               ; // Reduce counter
				jne       alpha_fade_loop   ; // On to the next 2 pixels
				add esi, eax                ; // Add the increment to get to the start of the next row
				dec edx                     ; // Reduce the row counter
				jne alpha_fade_row_loop // And off again
				EMMS                    ; // Clear/Set MMX/FPU flag
			}
#endif
			Unlock_surface(surface_id);
			effect_time = (GetMicroTimer() - effect_time);
			return;
		}
	}

	if (FadeMode() == 4) { /* COLOUR FADE */
		// Create the colour for this frame
		uint8 r = (uint8)(FadeFromRed() + ((FadeToRed() - FadeFromRed()) * FadeAlpha()) / 255);
		uint8 g = (uint8)(FadeFromGreen() + ((FadeToGreen() - FadeFromGreen()) * FadeAlpha()) / 255);
		uint8 b = (uint8)(FadeFromBlue() + ((FadeToBlue() - FadeFromBlue()) * FadeAlpha()) / 255);

		uint32 color = (r << 16) + (g << 8) + b;
		Graphics::Surface *pSurf = m_Surfaces[surface_id]->m_dds;
		pSurf->fillRect(Common::Rect(0, 0, pSurf->w, pSurf->h), color);
	}

	/* ADD Top/Bottom/Left/Right BORDERS */
	if (BorderMode() == 0) {
		// Add the borders ( if there are any )
		LRECT rect;
		rect.top = 0;
		rect.left = 0;
		rect.right = 640;

		uint32 color = (BorderRed() << 16) + (BorderGreen() << 8) + BorderBlue();
		Graphics::Surface *pSurf = m_Surfaces[surface_id]->m_dds;

		// Top
		rect.bottom = m_borders.top;
		if (rect.top != rect.bottom) {
			Common::Rect r;
			r.left = rect.left;
			r.top = rect.top;
			r.right = rect.right;
			r.bottom = rect.bottom;
			pSurf->fillRect(r, color);
			pSurf->fillRect(r, color);
		}

		// Bottom
		rect.bottom = 480;
		rect.top = m_borders.bottom;
		if (rect.top != rect.bottom) {
			Common::Rect r;
			r.left = rect.left;
			r.top = rect.top;
			r.right = rect.right;
			r.bottom = rect.bottom;
			pSurf->fillRect(r, color);
		}
	} else if ((BorderMode() == 1 || BorderMode() == 2) && m_borders.top != 0) {
		// Do we have MMX ?
		if (HasMMX()) {
			/* Alpha Blend
			    S'xy = ( Sxy * multiplier + blendedColour ) / 256
			*/

			// Flip FadeAlpha() so it's 0 for fully blended and 255 for no blend
			uint8 fA = (uint8)(255 - BorderAlpha());

			// Create multiplier mask
			uint8 multiplier[8];
			multiplier[4] = multiplier[0] = fA;
			multiplier[5] = multiplier[1] = 0x00;
			multiplier[6] = multiplier[2] = fA;
			multiplier[7] = multiplier[3] = 0x00;

			uint16 r = (uint16)(((255 - fA) * BorderRed()));
			uint16 g = (uint16)(((255 - fA) * BorderGreen()));
			uint16 b = (uint16)(((255 - fA) * BorderBlue()));

			// Pre calculate the fade colour component ... 0 <= alpha <= 128
			uint8 blendColour[8];
			*((uint16 *)&blendColour[0]) = b;
			*((uint16 *)&blendColour[2]) = g;
			*((uint16 *)&blendColour[4]) = r;
			*((uint16 *)&blendColour[6]) = (uint16)0x0000;

			// Yep.  Lock the working buffer
			uint8 *pixels = Lock_surface(surface_id);
			uint32 pitch = Get_pitch(surface_id);
#if 1
			for (int xPos = 0; xPos < SCREEN_WIDTH; xPos++) {
				// 32-bit BGRA pixel
				uint8 *pixel = &pixels[xPos * 4];
				// Subtract from RGB components
				for (int i = 0; i < 3; i++) {
					pixel[i] = (pixel[i] * multiplier[i] + blendColour[i * 2]) >> 8;
				}
			}
#else
			uint32 rowCount = m_borders.top;
			_asm {
				mov  ebx, (SCREEN_WIDTH/2)  ; // How many operations per row ?
				lea  esi, multiplier        ; // Get the address of the mulitplier array
				lea  edi, blendColour       ; // Get the address of the blendcolour array
				pxor MM2, MM2               ; // Clear MM2
				movq MM0, [esi]             ; // Load the multiplier
				movq MM1, [edi]             ; // Load the blended colour
				mov  esi, pPixels           ; // Get a pointer to the pixels
				mov  edx, rowCount          ; // Setup a count for the number of rows.
				sub  esi, 8                 ; // Ecx is our index and counter, so move the pointer back to compensate for it's 1 out ness
				pxor MM2, MM2               ; // Clear MM2 read for packing/unpacking
				mov  eax, rowStep           ; // Distance to move from the start of one line to the start of the next

				top_border_fade_row_loop:
				mov  ecx, ebx               ; // Pixel Counter ( 2 pixels at a time )
				top_border_fade_loop:
				movq      MM3, [esi + ecx * 8]   ; // Load 2 pixels into MM3 & MM4
				movq      MM4, [esi + ecx * 8]   ;
				punpckhbw MM3, MM2          ; // Unpack the 2 pixels from bytes into words
				punpcklbw MM4, MM2
				pmullw    MM3, MM0          ; // Multiply the pixels
				pmullw    MM4, MM0
				paddusw   MM3, MM1          ; // Add the blend colour
				paddusw   MM4, MM1
				psrlw     MM3, 8            ; // Divide by 256
				psrlw     MM4, 8
				packuswb  MM4, MM3          ; // Repack back from words to bytes
				movq     [esi + ecx * 8], MM4    ; // Store the 2 pixels
				dec     ecx                 ; // Reduce counter/index
				jne     top_border_fade_loop; // On to the next 2 pixels
				add esi, eax                ; // Move the base pointer on to the next line
				dec edx                     ; // Decrement the row counter
				jne top_border_fade_row_loop
			}
#endif
			// Blend the bottom half
			pixels += pitch * m_borders.bottom;

#if 1
			for (int xPos = 0; xPos < SCREEN_WIDTH; xPos++) {
				// 32-bit BGRA pixel
				uint8 *pixel = &pixels[xPos * 4];
				// Subtract from RGB components
				for (int i = 0; i < 3; i++) {
					pixel[i] = (pixel[i] * multiplier[i] + blendColour[i * 2]) >> 8;
				}
			}
#else
			_asm {
				mov  ebx, (SCREEN_WIDTH/2)  ;
				lea  esi, multiplier        ;
				lea  edi, blendColour       ;
				pxor MM2, MM2               ; // Clear MM2
				movq MM0, [esi]             ; // Load the multiplier
				movq MM1, [edi]             ; // Load the blended colour
				mov  esi, pPixels           ;
				mov  edx, rowCount          ;
				sub  esi, 8                 ; // Ecx is our index and counter, so move the pointer back to compensate for it's 1 out ness
				pxor MM2, MM2               ; // Clear MM2
				mov  eax, rowStep           ; // Distance to move from the end of one line to the start of the next

				bottom_border_fade_row_loop:
				mov  ecx, ebx               ; // Pixel Counter ( 2 pixels at a time )
				bottom_border_fade_loop:
				movq      MM3, [esi + ecx * 8]   ; // Load 2 pixels into MM3 & MM4
				movq      MM4, [esi + ecx * 8]   ;
				punpckhbw MM3, MM2          ; // Unpack the 2 pixels from bytes into words
				punpcklbw MM4, MM2
				pmullw    MM3, MM0          ; // Multiply the pixels
				pmullw    MM4, MM0
				paddusw   MM3, MM1          ; // Add the blend colour
				paddusw   MM4, MM1
				psrlw     MM3, 8            ; // Divide by 256
				psrlw     MM4, 8
				packuswb  MM4, MM3          ; // Repack back from words to bytes
				movq     [esi + ecx * 8], MM4    ; // Store the 2 pixels
				dec     ecx                 ; // Reduce counter -- optimize with decrement_and_jump ..... if I remember it.
				jne     bottom_border_fade_loop; // On to the next 2 pixels
				add esi, eax
				dec edx
				jne bottom_border_fade_row_loop
				EMMS;
			}
#endif
			Unlock_surface(surface_id);
		}
	}

	effect_time = (GetMicroTimer() - effect_time);
}

void _surface_manager::RecordFrame(const char *path) {
	warning("TODO: Implement RecordFrame");
#if 0
	if (SDL_SaveBMP(sdl_screen, path) < 0)
		Fatal_error("_surface_manager::RecordSurface( \"%s\" ) - Failed to create the output file", path);
#endif
}

} // End of namespace ICB
