/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

/* FrameBuffer renderer in an OpenGL texture
 Andre Souza <asouza@olinux.com.br> */

#include <GL/gl.h>
#include <SDL.h>
#include <stdlib.h>
#include <string.h>

/* FLAGS */
#define FB2GL_FS 1      /* FULLSCREEN */
#define FB2GL_RGBA 2    /* Use RGBA (else use palette) */
#define FB2GL_320 4     /* 320x256 texture (else use 256x256) */
#define FB2GL_AUDIO 8   /* Activate SDL Audio */
#define FB2GL_PITCH	16  /* On fb2l_update, use pitch (else bytes per pixel) */
#define FB2GL_EXPAND 32 /* Create a RGB fb with the color lookup table*/

/* Framebuffer for 8 bpp */ 
unsigned char ogl_fb[256][256];
unsigned char ogl_fbb[256][64];
/* Framebuffer for RGBA */ 
unsigned char ogl_fb1[256][256][4];
unsigned char ogl_fb2[256][64][4];
/* Texture(s) */ 
GLuint texture;
GLuint textureb;
/* Display list */ 
GLuint dlist;
/* Color Table (256 colors, RGB) */ 
char ogl_ctable[256][3];
char ogl_temp_ctable[256][3]; /* Support for OpenGL 1.1 */
/* Use RGBA? */ 
char fb2gl_RGBA=0;
/* If so, expand the 8 bit fb to RGB? */
char fb2gl_expand=0;
/* 320x256? */
char fb2gl_t320=0;
/* Use pitch? Else, bytes per pixel */
char use_pitch=0;
/* Methods */
void fb2gl_maketex();
void fb2gl_makedlist(int xf, int yf);
void fb2gl_display();

/* Public */
SDL_Surface *screen;
void fb2gl_init(int width, int height, int xfix, int yfix, char flags);
void fb2gl_update(void *fb, int width, int height, int pitch, int xskip, int yskip);
void fb2gl_palette(int index, int r, int g, int b);
void fb2gl_set_palette(int first, int ncolors);


void fb2gl_maketex()
{
  glGenTextures(0,&texture);
  glBindTexture(GL_TEXTURE_2D,texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
/*
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
*/
  if (fb2gl_RGBA) {
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256,256,0,GL_RGBA, GL_UNSIGNED_BYTE, ogl_fb1);
  }
  else {
    glTexImage2D(GL_TEXTURE_2D,0,GL_COLOR_INDEX,256,256,0,GL_COLOR_INDEX, GL_UNSIGNED_BYTE, ogl_fb);
  }

  if (fb2gl_t320) {
    glGenTextures(1,&textureb);
    glBindTexture(GL_TEXTURE_2D,textureb);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
/*
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
*/
    if (fb2gl_RGBA) {
      glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,64,256,0,GL_RGBA, 
	GL_UNSIGNED_BYTE, ogl_fb2);
    }
    else {
      glTexImage2D(GL_TEXTURE_2D,0,GL_COLOR_INDEX,64,256,0,GL_COLOR_INDEX,
	GL_UNSIGNED_BYTE, ogl_fbb);
    }
  }

}

void fb2gl_makedlist(int xf, int yf)
{
  double xfix=(double)xf/128; /* 128 = 256/2 (half texture => 0.0 to 1.0) */
  double yfix=(double)yf/128;
  double texend = (double)615/1024; /* End of 256x256 (from -1.0 to 1.0) */

  dlist=glGenLists(1);
  glNewList(dlist,GL_COMPILE);

  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, texture);

  if (!fb2gl_t320) { /* Normal 256x256 */
    glBegin(GL_QUADS);
      glTexCoord2f(0.0,1.0); glVertex2f(-1.0,-1.0-yfix); /* upper left */
      glTexCoord2f(0.0,0.0); glVertex2f(-1.0,1.0); /* lower left */
      glTexCoord2f(1.0,0.0); glVertex2f(1.0+xfix,1.0); /* lower right */
      glTexCoord2f(1.0,1.0); glVertex2f(1.0+xfix,-1.0-yfix); /* upper right */
    glEnd();
  }
  else { /* 320x256 */

    /* First, the 256x256 texture */
    glBegin(GL_QUADS);
      glTexCoord2f(0.0,1.0); glVertex2f(-1.0,-1.0-yfix); /* upper left */
      glTexCoord2f(0.0,0.0); glVertex2f(-1.0,1.0); /* lower left */
      glTexCoord2f(1.0,0.0); glVertex2f(texend+xfix,1.0); /* lower right */
      glTexCoord2f(1.0,1.0); glVertex2f(texend+xfix,-1.0-yfix); /* upper right*/
    glEnd();

    /* 64x256 */
    glBindTexture(GL_TEXTURE_2D, textureb);

    glBegin(GL_QUADS);
      glTexCoord2f(0.0,1.0); glVertex2f(texend+xfix,-1.0-yfix); /* upper left */
      glTexCoord2f(0.0,0.0); glVertex2f(texend+xfix,1.0); /* lower left */
      glTexCoord2f(1.0,0.0); glVertex2f(1.0+xfix,1.0); /* lower right */
      glTexCoord2f(1.0,1.0); glVertex2f(1.0+xfix,-1.0-yfix); /* upper right */
    glEnd();
  }

  glDisable(GL_TEXTURE_2D);

  glEndList();
}

void fb2gl_init(int width, int height, int xfix, int yfix, char flags)
{
  char fs=0;
  char audio=0;
  char gl_ext[4096];

  /* Test the flags */
  if (flags & FB2GL_FS) fs=1; else fs=0;
  if (flags & FB2GL_RGBA) fb2gl_RGBA=1; else fb2gl_RGBA=0;
  if (flags & FB2GL_320) fb2gl_t320=1; else fb2gl_t320=0;
  if (flags & FB2GL_AUDIO) audio=1; else audio=0;
  if (flags & FB2GL_PITCH) use_pitch=1; else use_pitch=0;
  if (flags & FB2GL_EXPAND) fb2gl_expand=1; else fb2gl_expand=0;
/*
  if (audio) SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  else SDL_Init(SDL_INIT_VIDEO);

  atexit(SDL_Quit);
*/


  if (fs) {
    screen = SDL_SetVideoMode(width, height, 0, SDL_HWSURFACE | SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_FULLSCREEN);
  }
  else {
    screen = SDL_SetVideoMode(width, height, 0, SDL_HWPALETTE | SDL_HWSURFACE | SDL_OPENGL | SDL_GL_DOUBLEBUFFER);
  }

  if (!screen) {
    fprintf(stderr, "Couldn't start video res %dx%d\n", width, height);
  }

#ifndef OGL_1_1
  if (!fb2gl_RGBA) {
    /* Paletted Texture Extension */
    strcpy(gl_ext, (char *)glGetString(GL_EXTENSIONS));
	
    if ( strstr( gl_ext , "GL_EXT_paletted_texture") )
      glEnable(GL_EXT_paletted_texture);
    else {
      fprintf(stderr,"Your OpenGL version doesn't support paletted texture\n");
      exit(0);
    }
  }
#endif

  fb2gl_maketex();
  fb2gl_makedlist(xfix, yfix);

}

void fb2gl_display()
{
  glCallList(dlist);
  SDL_GL_SwapBuffers();
}

/* Changed the way xskip and yskip work for use with scummvm (shaking) */
void fb2gl_update(void *fb, int w, int h, int pitch, int xskip, int yskip) {
  unsigned char *fb1=(unsigned char *)fb;
  int x,y,scr_pitch,byte=0;

  if (use_pitch) scr_pitch=pitch;
  else {
    scr_pitch=w*pitch;
    byte = pitch; /* Bytes perl pixel (for RGBA mode) */
  }

  if (fb2gl_RGBA) {
    
    if (fb2gl_expand) { /* Expand the 8 bit fb into a RGB fb */
  
      for (y=yskip; y<h; y++) {
	for (x=xskip; x<w; x++) {
	  if (x<256) { 
	    ogl_fb1[y][x][0] = ogl_ctable[*(fb1+x)][0];
	    ogl_fb1[y][x][1] = ogl_ctable[*(fb1+x)][1]; 
	    ogl_fb1[y][x][2] = ogl_ctable[*(fb1+x)][2]; 
	    /*  ogl_fb1[y-yskip][x-xskip][3]=255; */
	  }
	  else {
	    ogl_fb2[y][x-256][0] = ogl_ctable[*(fb1+x)][0]; 
	    ogl_fb2[y][x-256][1] = ogl_ctable[*(fb1+x)][1]; 
	    ogl_fb2[y][x-256][2] = ogl_ctable[*(fb1+x)][2]; 
	    /*  ogl_fb2[y-yskip][x-256][3]=255; */
	  }
	}
	fb1 += scr_pitch;
      }
    }
    else { /* No expansion */
      for (y=yskip; y<h; y++) {
	for (x=xskip; x<w; x++) {
	  if (x<256) { 
	    ogl_fb1[y-yskip][x-xskip][0] = *(fb1+(x*byte)); 
	    ogl_fb1[y-yskip][x-xskip][1] = *(fb1+(x*byte)+1); 
	    ogl_fb1[y-yskip][x-xskip][2] = *(fb1+(x*byte)+2); 
	    /* ogl_fb1[y-yskip][x-xskip][3]=255; */
	  }
	  else {
	    ogl_fb2[y-yskip][x-256][0] = *(fb1+(x*byte)); 
	    ogl_fb2[y-yskip][x-256][1] = *(fb1+(x*byte)+1); 
	    ogl_fb2[y-yskip][x-256][2] = *(fb1+(x*byte)+2); 
	    /* ogl_fb2[y-yskip][x-256][3]=255; */
	  }
	}
	fb1 += scr_pitch;
      }
    }

    /* Update 256x256 texture */
    glBindTexture(GL_TEXTURE_2D,texture);
    glFlush();
    glTexSubImage2D(GL_TEXTURE_2D,0,xskip,yskip,256-xskip,256-yskip,GL_RGBA,
	GL_UNSIGNED_BYTE,ogl_fb1);

    if (fb2gl_t320) {
      /* Update 64x256 texture */
      glBindTexture(GL_TEXTURE_2D,textureb);
      glFlush();
      glTexSubImage2D(GL_TEXTURE_2D,0,xskip,yskip,64-xskip,256-yskip,GL_RGBA,
	GL_UNSIGNED_BYTE,ogl_fb2);
    }

  }
  else { /* non RGBA */

    for (y=0; y<h; y++)
      for (x=0; x<w; x++) {
	if (x<256) { 
	  ogl_fb[ y ][ x ] = *(fb1 + (y)*scr_pitch + x);
	}
	else { 
	  ogl_fbb[ y ][ x - 256 ] = *(fb1 + y*scr_pitch + x);
	}
      }

    /* Update 256x256 texture */
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexSubImage2D(GL_TEXTURE_2D,0,xskip,yskip,256-xskip,256-yskip,
	GL_COLOR_INDEX, GL_UNSIGNED_BYTE,ogl_fb);
		
    if (fb2gl_t320) {
      /* Update 64x256 texture */
      glBindTexture(GL_TEXTURE_2D,textureb);
      glTexSubImage2D(GL_TEXTURE_2D,0,xskip,yskip,64-xskip,256-yskip,
	  GL_COLOR_INDEX, GL_UNSIGNED_BYTE,ogl_fbb);
    }

  }

  fb2gl_display();

}

void fb2gl_palette(int i, int r, int g, int b) {
#ifdef OGL_1_1
  ogl_temp_ctable[i][0]=r;
  ogl_temp_ctable[i][1]=g;
  ogl_temp_ctable[i][2]=b;
#else
  ogl_ctable[i][0]=r;
  ogl_ctable[i][1]=g;
  ogl_ctable[i][2]=b;
#endif
}

void fb2gl_set_palette(int f, int n) {
  char temp[256][3];
  int i;
 
#ifdef OGL_1_1
  /* No ColorTable Extension. Expand option MUST be set. */
  for (i=f; i<n; i++) {
    ogl_ctable[i][0] = ogl_temp_ctable[i][0];
    ogl_ctable[i][1] = ogl_temp_ctable[i][1];
    ogl_ctable[i][2] = ogl_temp_ctable[i][2];
  }
#else	
  if (!fb2gl_expand) 
  {

    glBindTexture(GL_TEXTURE_2D,texture);
    glGetColorTable(GL_TEXTURE_2D,GL_RGB,GL_UNSIGNED_BYTE,&temp);
	
    for (i=f; i<n; i++) {
      temp[i][0] = ogl_ctable[i][0];
      temp[i][1] = ogl_ctable[i][1];
      temp[i][2] = ogl_ctable[i][2];
    }
	
    glColorTable(GL_TEXTURE_2D,GL_RGB,256,GL_RGB,GL_UNSIGNED_BYTE,&temp);

    if (fb2gl_t320) {
      glBindTexture(GL_TEXTURE_2D,textureb);
      glColorTable(GL_TEXTURE_2D,GL_RGB,256,GL_RGB,GL_UNSIGNED_BYTE,&temp);
    }

  }
#endif	
}
