/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

// FrameBuffer renderer in an OpenGL texture
// Andre Souza <asouza_luke@yahoo.com.br>

#include <SDL.h>
#include <SDL_opengl.h>
//#include <GL/gl.h>	NOTE! Before anybody comments out SDL_opengl.h and enables this again, talk to Fingolfin first! GL/gl.h is NOT portable!
#include <stdlib.h>
#include <string.h>

// FLAGS 
#define FB2GL_FS 1      // FULLSCREEN
#define FB2GL_RGBA 2    // Use RGBA (else use palette)
#define FB2GL_NO_320 4     // Don't use 320x256 texture, rather emulate with a 256x256 plus a 64x256 texture
#define FB2GL_AUDIO 8   // Activate SDL Audio
#define FB2GL_PITCH 16  // On fb2l_update, use pitch (else bytes per pixel)
#define FB2GL_EXPAND 32 // Create a RGB fb with the color lookup table
#define FB2GL_16BIT 64  // 16 BIT Color Depth

// This extension isn't defined in OpenGL 1.1
#ifndef GL_EXT_paletted_texture
#define GL_EXT_paletted_texture 1
#endif

class FB2GL {
	private:
		SDL_Surface *screen;
		// Framebuffer for 8 bpp
		unsigned char palettedFrameBuffer1[256*320];
		unsigned char *palettedFrameBuffer2;
//		unsigned char palettedFrameBuffer1[256][256];
//		unsigned char palettedFrameBuffer2[256][64];
		// Framebuffer for RGBA
		unsigned char RGBAFrameBuffer1[256*320*4];
		unsigned char *RGBAFrameBuffer2;
//		unsigned char RGBAFrameBuffer1[256][256][4];
//		unsigned char RGBAFrameBuffer2[256][64][4];
		// Texture(s)
		GLuint texture1;
		GLuint texture2;
		// Display list
		GLuint displayList;
		// Color Table (256 colors, RGB)
		char colorTable[256][3];
		char tempColorTable[256][3]; // Support for OpenGL 1.1
		char flags;
		void makeTextures();
		void makeDisplayList(int xf, int yf);

	public:
		
		FB2GL() { 
			flags = 0;
			screen = NULL;
			
			palettedFrameBuffer2 = palettedFrameBuffer1 + 256*256;
			RGBAFrameBuffer2 = RGBAFrameBuffer1 + 256*256*4;
		}
		
		SDL_Surface *getScreen() {
		  return screen; 
		};
		
		void setScreen(SDL_Surface *s) {
		  screen = s;
		};
		
		int init(int width, int height, int xfix, int yfix, char _flags);
		void update(void *fb, int width, int height, int pitch, int xskip, int yskip);
		void palette(int index, int r, int g, int b);
		void setPalette(int first, int ncolors);
		void blit16(SDL_Surface *fb, int num_rect, SDL_Rect *rectlist, int xskip, int yskip);
		void display();
		void setBilinearMode(bool bilinear);
};

void FB2GL::setBilinearMode(bool bilinear) {
	const GLuint mode = bilinear ? GL_LINEAR : GL_NEAREST;
	glBindTexture(GL_TEXTURE_2D, texture1);
	// No borders
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	// Bilinear filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode);

	if (flags & FB2GL_NO_320) {
		glBindTexture(GL_TEXTURE_2D, texture2);
		// No borders
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		// Bilinear filter
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode);
	}
}

void FB2GL::makeTextures() {
	glGenTextures(0,&texture1);
	glBindTexture(GL_TEXTURE_2D,texture1);

	// No borders
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Bilinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	GLuint mode;
	const GLvoid *buffer1, *buffer2;

	if (flags & FB2GL_RGBA) {
		mode = GL_RGBA;
		buffer1 = RGBAFrameBuffer1;
		buffer2 = RGBAFrameBuffer2;
	} else {
		mode = GL_COLOR_INDEX;
		buffer1 = palettedFrameBuffer1;
		buffer2 = palettedFrameBuffer2;
	}

	if (flags & FB2GL_NO_320) {
		glTexImage2D(GL_TEXTURE_2D, 0, mode, 256, 256, 0, mode, GL_UNSIGNED_BYTE, buffer1);

		glGenTextures(1, &texture2);
		glBindTexture(GL_TEXTURE_2D, texture2);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		// Bilinear filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, mode, 64, 256, 0, mode, GL_UNSIGNED_BYTE, buffer2);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, mode, 320, 256, 0, mode, GL_UNSIGNED_BYTE, buffer1);
	}
}


void FB2GL::makeDisplayList(int xf, int yf) {
	GLfloat xfix = xf / 128.0f; // 128 = 256/2 (half texture => 0.0 to 1.0)
	GLfloat yfix = yf / 128.0f;
	GLfloat texture1_end;	// End of 256x256 texture (from -1.0 to 1.0)

	if (flags & FB2GL_NO_320) {
		// Game screen width = 320
		// GL coordinates (-1.0 <= x <= 1.0, -1.0 <= y <= 1.0)
		// X axis center: GL => 0.0, Game => 320/2 = 160
		// GL texture1 width = 256 (texture2 width = 64. 256 + 64 = 320)
		// 160 = 320/2 (half width). 256 - 160 = 96 
		texture1_end = 96.0f / 160.0f; // between 0.0 (center) and 1.0
		// Note: wrong value may cause black vertical line (gap 
		// between texture1 and texture2 in the X axis)
	}
	else {
		texture1_end = 1.0f;
	}
	
	if (glIsList(displayList)) 
		glDeleteLists(displayList, 1);

	displayList = glGenLists(1);
	glNewList(displayList, GL_COMPILE);

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, texture1);

	glBegin(GL_QUADS);
	// lower left
	glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f - yfix);
	// upper left
	glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);
	// upper right
	glTexCoord2f(1.0f, 0.0f); glVertex2f(texture1_end + xfix, 1.0f);
	// lower right
	glTexCoord2f(1.0f, 1.0f); glVertex2f(texture1_end + xfix, -1.0f - yfix);
	glEnd();

	if (flags & FB2GL_NO_320) {
		// 64x256 (texture2)
		glBindTexture(GL_TEXTURE_2D, texture2);

		glBegin(GL_QUADS);
		// lower left	
		glTexCoord2f(0.0f, 1.0f); glVertex2f(texture1_end + xfix, -1.0f - yfix);
		// upper left
		glTexCoord2f(0.0f, 0.0f); glVertex2f(texture1_end + xfix, 1.0f);
		// upper right
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f + xfix, 1.0f);
		// lower right
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f + xfix, -1.0f - yfix);
		glEnd();
	}

	glDisable(GL_TEXTURE_2D);

	glEndList();
}

int FB2GL::init(int width, int height, int xfix, int yfix, char _flags) {
	char gl_ext[4096];
	gl_ext[0]='\0';

	flags = _flags;

	// Fullscreen?
	if (!screen) {
		screen = SDL_SetVideoMode(width, height, (flags & FB2GL_16BIT ? 16: 0),
				SDL_HWPALETTE | SDL_HWSURFACE | SDL_OPENGL | SDL_GL_DOUBLEBUFFER |
				(flags & FB2GL_FS? SDL_FULLSCREEN: 0));
	}

//	warning("Screen BitsPerPixel: %d\n",screen->format->BitsPerPixel);

	if (!screen) {
		warning("Couldn't start video res %dx%d", width, height);
		return 0;
	}

/*
	if (!(flags & FB2GL_RGBA)) { // Check for Paletted Texture Extension 
		
		strcpy(gl_ext, (char *)glGetString(GL_EXTENSIONS));
	fprintf(stderr,"gl_ext= %s\n",gl_ext);

		if ( strstr( gl_ext , "GL_EXT_paletted_texture") )
			glEnable(GL_EXT_paletted_texture);
		else {
			fprintf(stderr,"Your OpenGL version doesn't support paletted texture\n");
			return 0;
		}
	}
*/

	if (width>0 && height>0) 
		makeTextures();
	makeDisplayList(xfix, yfix);

	return 1;
}

void FB2GL::display()
{
	glCallList(displayList);
	SDL_GL_SwapBuffers();
}

void FB2GL::update(void *fb, int w, int h, int pitch, int xskip, int yskip) {
	unsigned char *tempFrameBuffer = (unsigned char *)fb;
	int x, y, scr_pitch, _byte;

	if (flags & FB2GL_PITCH) {
		scr_pitch = pitch;
		_byte = 0;
	} else {
		scr_pitch = w * pitch;
		_byte = pitch; // Bytes perl pixel (for RGBA mode)
	}

	if (flags & FB2GL_RGBA) {
		
		if (flags & FB2GL_EXPAND) { // Expand the 8 bit fb into a RGB fb
	
			for (y = yskip; y < h; y++) {
				for (x = xskip; x < w; x++) {
					if (!(flags & FB2GL_NO_320)) {
						RGBAFrameBuffer1[y*320*4 + x*4 + 0] = colorTable[*(tempFrameBuffer+x)][0];
						RGBAFrameBuffer1[y*320*4 + x*4 + 1] = colorTable[*(tempFrameBuffer+x)][1]; 
						RGBAFrameBuffer1[y*320*4 + x*4 + 2] = colorTable[*(tempFrameBuffer+x)][2]; 
						RGBAFrameBuffer1[y*320*4 + x*4 + 3] = 255;
					} else if (x < 256) { 
						RGBAFrameBuffer1[y*256*4 + x*4 + 0] = colorTable[*(tempFrameBuffer+x)][0];
						RGBAFrameBuffer1[y*256*4 + x*4 + 1] = colorTable[*(tempFrameBuffer+x)][1]; 
						RGBAFrameBuffer1[y*256*4 + x*4 + 2] = colorTable[*(tempFrameBuffer+x)][2]; 
						RGBAFrameBuffer1[y*256*4 + x*4 + 3] = 255;
					} else {
						RGBAFrameBuffer2[y*64*4 + (x-256)*4 + 0] = colorTable[*(tempFrameBuffer+x)][0]; 
						RGBAFrameBuffer2[y*64*4 + (x-256)*4 + 1] = colorTable[*(tempFrameBuffer+x)][1]; 
						RGBAFrameBuffer2[y*64*4 + (x-256)*4 + 2] = colorTable[*(tempFrameBuffer+x)][2]; 
						RGBAFrameBuffer2[y*64*4 + (x-256)*4 + 3] = 255;
					}
				}
				tempFrameBuffer += scr_pitch; // Next row (like y++)
			}
		} else { // No expansion
			for (y = yskip; y < h; y++) {
				for (x = xskip; x < w; x++) {
					if (!(flags & FB2GL_NO_320)) {
						RGBAFrameBuffer1[(y-yskip)*320*4 + (x-xskip)*4 + 0] = *(tempFrameBuffer+(x*_byte)); 
						RGBAFrameBuffer1[(y-yskip)*320*4 + (x-xskip)*4 + 1] = *(tempFrameBuffer+(x*_byte)+1); 
						RGBAFrameBuffer1[(y-yskip)*320*4 + (x-xskip)*4 + 2] = *(tempFrameBuffer+(x*_byte)+2); 
					} else if (x < 256) { 
						RGBAFrameBuffer1[(y-yskip)*256*4 + (x-xskip)*4 + 0] = *(tempFrameBuffer+(x*_byte)); 
						RGBAFrameBuffer1[(y-yskip)*256*4 + (x-xskip)*4 + 1] = *(tempFrameBuffer+(x*_byte)+1); 
						RGBAFrameBuffer1[(y-yskip)*256*4 + (x-xskip)*4 + 2] = *(tempFrameBuffer+(x*_byte)+2); 
					} else {
						RGBAFrameBuffer2[(y-yskip)*64*4 + (x-256)*4 + 0] = *(tempFrameBuffer+(x*_byte)); 
						RGBAFrameBuffer2[(y-yskip)*64*4 + (x-256)*4 + 1] = *(tempFrameBuffer+(x*_byte)+1); 
						RGBAFrameBuffer2[(y-yskip)*64*4 + (x-256)*4 + 2] = *(tempFrameBuffer+(x*_byte)+2); 
					}
				}
				tempFrameBuffer += scr_pitch; // Next row (like y++)
			}
		}

		if (flags & FB2GL_NO_320) {
			// Update 256x256 texture
			glBindTexture(GL_TEXTURE_2D, texture1);
			glFlush();
			glTexSubImage2D(GL_TEXTURE_2D, 0, xskip, yskip, 
				256-xskip, 256-yskip, GL_RGBA,
				GL_UNSIGNED_BYTE, RGBAFrameBuffer1);

			// Update 64x256 texture
			glBindTexture(GL_TEXTURE_2D, texture2);
			glFlush();
			glTexSubImage2D(GL_TEXTURE_2D, 0, xskip, yskip,
			    64-xskip, 256-yskip, GL_RGBA,
			    GL_UNSIGNED_BYTE, RGBAFrameBuffer2);
		} else {
			// Update 320x256 texture
			glBindTexture(GL_TEXTURE_2D, texture1);
			glFlush();
			glTexSubImage2D(GL_TEXTURE_2D, 0, xskip, yskip, 
			    320-xskip, 256-yskip, GL_RGBA,
			    GL_UNSIGNED_BYTE, RGBAFrameBuffer1);
		}

	} else { // non RGBA (paletted)

		for (y=0; y<h; y++)
			for (x=0; x<w; x++) {
				if (!(flags & FB2GL_NO_320)) {
					palettedFrameBuffer1[y * 320 + x] = *(tempFrameBuffer + y*scr_pitch + x);
				} else if (x<256) { 
					palettedFrameBuffer1[y * 256 + x] = *(tempFrameBuffer + y*scr_pitch + x);
				} else { 
					palettedFrameBuffer2[y * 64 + (x - 256)] = *(tempFrameBuffer + y*scr_pitch + x);
				}
			}

		if (flags & FB2GL_NO_320) {
			// Update 256x256 texture
			glBindTexture(GL_TEXTURE_2D, texture1);
			glTexSubImage2D(GL_TEXTURE_2D, 0, xskip, yskip, 
				256-xskip, 256-yskip, GL_COLOR_INDEX, 
				GL_UNSIGNED_BYTE, palettedFrameBuffer1);
			
			// Update 64x256 texture
			glBindTexture(GL_TEXTURE_2D, texture2);
			glTexSubImage2D(GL_TEXTURE_2D, 0, xskip, yskip, 
			    64-xskip, 256-yskip, GL_COLOR_INDEX, 
			    GL_UNSIGNED_BYTE, palettedFrameBuffer2);
		} else {
			// Update 320x256 texture
			glBindTexture(GL_TEXTURE_2D, texture1);
			glTexSubImage2D(GL_TEXTURE_2D, 0, xskip, yskip, 
				320-xskip, 256-yskip, GL_COLOR_INDEX, 
				GL_UNSIGNED_BYTE, palettedFrameBuffer1);
		}

	}

	display();
}

void FB2GL::blit16(SDL_Surface *fb, int num_rect, SDL_Rect *rect, int xskip, int yskip) {
	int x, y, i;
	int rx, ry, rw, rh; // rect[i].x, .y, .w, .h 
	// (0 <= rx <= 320) and (0 <= ry <= 240). 
	// Note: rx may be larger than texture1 width (256).
	int xend=0, yend=0; // (x + width) and (y + height)
	int pitch = fb->pitch/2; // 16 bit pointer access (not char *)
	
	// Width of the rectangle draw in texture1. (256 - rx) > 0
	unsigned int tex1_w = 0;
	// Width of the rectangle draw in texture2. (xend - 256) > 0
	unsigned int tex2_w = 0;
	// Rectangle's x coordinate in texture2. (rx - 256) > 0
	unsigned int tex2_x = 0;
	
	for (i=0; i<num_rect; i++) {
		tex1_w = tex2_w = tex2_x = 0;
		rx = rect[i].x;
		ry = rect[i].y;
		rw = rect[i].w;
		rh = rect[i].h;
		xend = rx + rw;
		yend = ry + rh;
		if (xend > fb->w) continue;
		if (yend > fb->h) continue;

		if (rx < 256) { // Begins before the end of texture1
			if (xend >= 256) { // Ends after texture1
			  tex2_w = xend - 256; //Rectangle width on texture2
			  tex1_w = rw - tex2_w; // Width left for texture1
			} else
				tex1_w = rw;  // This rectangle is inside texture1
		} else {
			tex2_w = rw;       // This rectangle is inside texture2
			tex2_x = rx - 256; // Relative x coordinate on texture2
		}

		// Note: if (tex1_w == 0) then there is no rectangle to draw 
		// in texture1. The same is true for tex2_w.

		for (y = ry; y < yend; y++) {
			for (x = rx; x < xend; x++) {
			
				if (tex1_w && x < 256) { 
					int pos = (x-rx+(y-ry)*tex1_w)*4; // (x + (y*pitch)) * RGBAsize
					SDL_GetRGB(
						((Uint16 *)fb->pixels)[x+y*(pitch)], 
						fb->format,
						&RGBAFrameBuffer1[pos],
						&RGBAFrameBuffer1[pos+1],
						&RGBAFrameBuffer1[pos+2]
					);
				} else if (tex2_w && x >= 256) {
					int rx2 = rx < 256? 256: rx;
					int pos = (x-rx2+(y-ry)*tex2_w)*4; // (x + (y*pitch)) * RGBAsize
					SDL_GetRGB(
						((Uint16 *)fb->pixels)[x+y*(pitch)], 
						fb->format,
						&RGBAFrameBuffer2[pos],
						&RGBAFrameBuffer2[pos+1],
						&RGBAFrameBuffer2[pos+2]
					);
				}
			}
		}

		if (tex1_w > 0) {
			// Update 256x256 texture
			glBindTexture(GL_TEXTURE_2D, texture1);
			glFlush();
			glTexSubImage2D(GL_TEXTURE_2D, 0, 
			    rx + xskip,
			    ry + yskip, 
			    tex1_w, rh, GL_RGBA,
			    GL_UNSIGNED_BYTE, RGBAFrameBuffer1);
		}
		if (tex2_w > 0) { // What was left for this texture
			// Update 64x256 texture
			glBindTexture(GL_TEXTURE_2D, texture2);
			glFlush();
			glTexSubImage2D(GL_TEXTURE_2D, 0, 
			    tex2_x + xskip, 
			    ry + yskip, 
			    tex2_w, rh, GL_RGBA,
			    GL_UNSIGNED_BYTE, RGBAFrameBuffer2);
		}
	}
}

void FB2GL::palette(int i, int r, int g, int b) {
	if (flags & FB2GL_EXPAND) {
		tempColorTable[i][0] = r;
		tempColorTable[i][1] = g;
		tempColorTable[i][2] = b;
	} else { // Paletted texture
		colorTable[i][0] = r;
		colorTable[i][1] = g;
		colorTable[i][2] = b;
	}
}

void FB2GL::setPalette(int first, int n) {
	char temp[256][3];
	int i;
 
	if (flags & FB2GL_EXPAND) {
		for (i = first; i < n; i++) {
			colorTable[i][0] = tempColorTable[i][0];
			colorTable[i][1] = tempColorTable[i][1];
			colorTable[i][2] = tempColorTable[i][2];
		}
	} else { // Paletted texture
		glBindTexture(GL_TEXTURE_2D, texture1);
		glGetColorTable(GL_TEXTURE_2D, GL_RGB, GL_UNSIGNED_BYTE, &temp);
	
		for (i = first; i < n; i++) {
			temp[i][0] = colorTable[i][0];
			temp[i][1] = colorTable[i][1];
			temp[i][2] = colorTable[i][2];
		}
	
		glColorTable(GL_TEXTURE_2D, GL_RGB, 256, GL_RGB, 
		    GL_UNSIGNED_BYTE, &temp);

		if (flags & FB2GL_NO_320) {
			glBindTexture(GL_TEXTURE_2D, texture2);
			glColorTable(GL_TEXTURE_2D, GL_RGB, 256, GL_RGB, 
			    GL_UNSIGNED_BYTE, &temp);
		}

	}
}
