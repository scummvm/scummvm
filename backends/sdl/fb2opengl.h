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

// FrameBuffer renderer in an OpenGL texture
// Andre Souza <asouza@olinux.com.br>

#include <SDL.h>
#include <SDL_opengl.h>
//#include <GL/gl.h>	NOTE! Before anybody comments out SDL_opengl.h and enables this again, talk to Fingolfin first! GL/gl.h is NOT portable!
#include <stdlib.h>
#include <string.h>

// FLAGS 
#define FB2GL_FS 1      // FULLSCREEN
#define FB2GL_RGBA 2    // Use RGBA (else use palette)
#define FB2GL_320 4     // 320x256 texture (else use 256x256)
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
    // Framebuffer for 8 bpp
    unsigned char ogl_fb[256][256];
    unsigned char ogl_fbb[256][64];
    // Framebuffer for RGBA
    unsigned char ogl_fb1[256][256][4];
    unsigned char ogl_fb2[256][64][4];
    // Framebuffer for the blit function (SDL Blitting)
    unsigned char fb1[256*256*4]; // Enough room for RGBA
    unsigned char fb2[64*256*4]; // Enough room for RGBA
    // Texture(s)
    GLuint texture;
    GLuint textureb;
    // Display list
    GLuint dlist;
    // Color Table (256 colors, RGB)
    char ogl_ctable[256][3];
    char ogl_temp_ctable[256][3]; // Support for OpenGL 1.1
    char flags;
    void maketex();
    void makedlist(int xf, int yf);

  public:
    SDL_Surface *screen;
    FB2GL() { 
      flags=0;
      screen=NULL;
    }
    int init(int width, int height, int xfix, int yfix, char _flags);
    void update(void *fb, int width, int height, int pitch, int xskip, int yskip);
    void palette(int index, int r, int g, int b);
    void setPalette(int first, int ncolors);
    void blit16(SDL_Surface *fb, int num_rect, SDL_Rect *rectlist, int xskip, int yskip);
    void display();
};

void FB2GL::maketex()
{
  glGenTextures(0,&texture);
  glBindTexture(GL_TEXTURE_2D,texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // Bilinear filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
/*
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
*/
  
  if (flags & FB2GL_RGBA) {
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256,256,0,GL_RGBA, GL_UNSIGNED_BYTE, ogl_fb1);
  }
  else {
    glTexImage2D(GL_TEXTURE_2D,0,GL_COLOR_INDEX,256,256,0,GL_COLOR_INDEX, GL_UNSIGNED_BYTE, ogl_fb);
  }

  if (flags & FB2GL_320) {
    glGenTextures(1,&textureb);
    glBindTexture(GL_TEXTURE_2D,textureb);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Bilinear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
/*
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
*/

    if (flags & FB2GL_RGBA) {
      glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,64,256,0,GL_RGBA, 
	GL_UNSIGNED_BYTE, ogl_fb2);
    }
    else {
      glTexImage2D(GL_TEXTURE_2D,0,GL_COLOR_INDEX,64,256,0,GL_COLOR_INDEX,
	GL_UNSIGNED_BYTE, ogl_fbb);
    }
  }

}

void FB2GL::makedlist(int xf, int yf)
{
  double xfix=(double)xf/128; // 128 = 256/2 (half texture => 0.0 to 1.0)
  double yfix=(double)yf/128;
  // End of 256x256 (from -1.0 to 1.0)
  double texend = (double)96/160; // 160=320/2 (== 0.0), 256-160=96.
 
  if (glIsList(dlist)) 
    glDeleteLists(dlist,1);
      
  dlist=glGenLists(1);
  glNewList(dlist,GL_COMPILE);

  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, texture);

  if (!(flags & FB2GL_320)) { // Normal 256x256
    glBegin(GL_QUADS);
      glTexCoord2f(0.0,1.0); glVertex2f(-1.0,-1.0-yfix); // upper left 
      glTexCoord2f(0.0,0.0); glVertex2f(-1.0,1.0); // lower left 
      glTexCoord2f(1.0,0.0); glVertex2f(1.0+xfix,1.0); // lower right 
      glTexCoord2f(1.0,1.0); glVertex2f(1.0+xfix,-1.0-yfix); // upper right 
    glEnd();
  }
  else { // 320x256 

    // First, the 256x256 texture 
    glBegin(GL_QUADS);
      glTexCoord2f(0.0,1.0); glVertex2f(-1.0,-1.0-yfix); // upper left 
      glTexCoord2f(0.0,0.0); glVertex2f(-1.0,1.0); // lower left 
      glTexCoord2f(1.0,0.0); glVertex2f(texend+xfix,1.0); // lower right 
      glTexCoord2f(1.0,1.0); glVertex2f(texend+xfix,-1.0-yfix); // upper right
    glEnd();

    // 64x256 
    glBindTexture(GL_TEXTURE_2D, textureb);

    glBegin(GL_QUADS);
      glTexCoord2f(0.0,1.0); glVertex2f(texend+xfix,-1.0-yfix); // upper left 
      glTexCoord2f(0.0,0.0); glVertex2f(texend+xfix,1.0); // lower left 
      glTexCoord2f(1.0,0.0); glVertex2f(1.0+xfix,1.0); // lower right 
      glTexCoord2f(1.0,1.0); glVertex2f(1.0+xfix,-1.0-yfix); // upper right 
    glEnd();
  }

  glDisable(GL_TEXTURE_2D);

  glEndList();
}

int FB2GL::init(int width, int height, int xfix, int yfix, char _flags)
{
  char gl_ext[4096];
  gl_ext[0]='\0';

  flags = _flags;
  
  // Fullscreen?
  if (!screen) {
    screen = SDL_SetVideoMode(width, height, (flags & FB2GL_16BIT? 16: 0), 
	SDL_HWPALETTE | SDL_HWSURFACE | SDL_OPENGL | SDL_GL_DOUBLEBUFFER | 
	(flags & FB2GL_FS? SDL_FULLSCREEN: 0));
  }

  fprintf(stderr,"Screen BitsPerPixel: %d\n",screen->format->BitsPerPixel);
  
  if (!screen) {
    fprintf(stderr, "Couldn't start video res %dx%d\n", width, height);
    return 0;
  }


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
  
  if (width>0 && height>0) 
    maketex();
  makedlist(xfix, yfix);

/*  glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glAlphaFunc(GL_GREATER,0);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);*/
  
  return 1;
}

void FB2GL::display()
{
  glCallList(dlist);
  SDL_GL_SwapBuffers();
}

void FB2GL::update(void *fb, int w, int h, int pitch, int xskip, int yskip) {
  unsigned char *fb1=(unsigned char *)fb;
  int x,y,scr_pitch,byte=0;

  if (flags & FB2GL_PITCH) scr_pitch=pitch;
  else {
    scr_pitch=w*pitch;
    byte = pitch; // Bytes perl pixel (for RGBA mode)
  }

  if (flags & FB2GL_RGBA) {
    
    if (flags & FB2GL_EXPAND) { // Expand the 8 bit fb into a RGB fb
  
      for (y=yskip; y<h; y++) {
	for (x=xskip; x<w; x++) {
	  if (x<256) { 
	    ogl_fb1[y][x][0] = ogl_ctable[*(fb1+x)][0];
	    ogl_fb1[y][x][1] = ogl_ctable[*(fb1+x)][1]; 
	    ogl_fb1[y][x][2] = ogl_ctable[*(fb1+x)][2]; 
	    ogl_fb1[y][x][3] = 255;
	  }
	  else {
	    ogl_fb2[y][x-256][0] = ogl_ctable[*(fb1+x)][0]; 
	    ogl_fb2[y][x-256][1] = ogl_ctable[*(fb1+x)][1]; 
	    ogl_fb2[y][x-256][2] = ogl_ctable[*(fb1+x)][2]; 
	    ogl_fb2[y][x-256][3] = 255;
	  }
	}
	fb1 += scr_pitch;
      }
    }
    else { // No expansion
      for (y=yskip; y<h; y++) {
	for (x=xskip; x<w; x++) {
	  if (x<256) { 
	    ogl_fb1[y-yskip][x-xskip][0] = *(fb1+(x*byte)); 
	    ogl_fb1[y-yskip][x-xskip][1] = *(fb1+(x*byte)+1); 
	    ogl_fb1[y-yskip][x-xskip][2] = *(fb1+(x*byte)+2); 
	  }
	  else {
	    ogl_fb2[y-yskip][x-256][0] = *(fb1+(x*byte)); 
	    ogl_fb2[y-yskip][x-256][1] = *(fb1+(x*byte)+1); 
	    ogl_fb2[y-yskip][x-256][2] = *(fb1+(x*byte)+2); 
	  }
	}
	fb1 += scr_pitch;
      }
    }

    // Update 256x256 texture
    glBindTexture(GL_TEXTURE_2D,texture);
    glFlush();
    glTexSubImage2D(GL_TEXTURE_2D,0,xskip,yskip,256-xskip,256-yskip,GL_RGBA,
	GL_UNSIGNED_BYTE,ogl_fb1);

    if (flags & FB2GL_320) {
      // Update 64x256 texture
      glBindTexture(GL_TEXTURE_2D,textureb);
      glFlush();
      glTexSubImage2D(GL_TEXTURE_2D,0,xskip,yskip,64-xskip,256-yskip,GL_RGBA,
	GL_UNSIGNED_BYTE,ogl_fb2);
    }

  }
  else { // non RGBA (paletted)

    for (y=0; y<h; y++)
      for (x=0; x<w; x++) {
	if (x<256) { 
	  ogl_fb[ y ][ x ] = *(fb1 + (y)*scr_pitch + x);
	}
	else { 
	  ogl_fbb[ y ][ x - 256 ] = *(fb1 + y*scr_pitch + x);
	}
      }

    // Update 256x256 texture
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexSubImage2D(GL_TEXTURE_2D,0,xskip,yskip,256-xskip,256-yskip,
	GL_COLOR_INDEX, GL_UNSIGNED_BYTE,ogl_fb);
		
    if (flags & FB2GL_320) {
      // Update 64x256 texture
      glBindTexture(GL_TEXTURE_2D,textureb);
      glTexSubImage2D(GL_TEXTURE_2D,0,xskip,yskip,64-xskip,256-yskip,
	  GL_COLOR_INDEX, GL_UNSIGNED_BYTE,ogl_fbb);
    }

  }

  display();

}

void FB2GL::blit16(SDL_Surface *fb, int num_rect, SDL_Rect *rect, int xskip, int yskip) {
  int x, y, i;
  int rx, ry, rw, rh;
  int xend=0, yend=0;
  int pitch = fb->pitch/2; // 16 bit pointer access (not char *)
  int tex1_w = 0, tex2_w = 0, tex2_x = 0;

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

    if (rx < 256) { // Begins before the end of the 1st texture
      if (xend >= 256) { // Ends after the first texture
	tex2_w = xend-256; // For the 2nd texture
	tex1_w = rw - tex2_w; // For the 1st texture
      }
      else tex1_w = rw; 
    }
    else {
      tex2_w = rw;
      tex2_x = rx - 256;
    }

    for (y = ry; y < yend; y++) {
      for (x = rx; x < xend; x++) {
      
        if (x < 256 && tex1_w) { 
	  int pos = (x-rx+(y-ry)*tex1_w)*4; // RGBA
	  SDL_GetRGB(((Uint16 *)fb->pixels)[x+y*(pitch)],fb->format,
	    &fb1[pos],
	    &fb1[pos+1],
	    &fb1[pos+2]);
	}
	else if (x >= 256 && tex2_w) {
	  int rx2 = rx < 256? 256: rx;
	  int pos = (x-rx2+(y-ry)*tex2_w)*4; // RGBA
	  SDL_GetRGB(((Uint16 *)fb->pixels)[x+y*(pitch)],fb->format,
	    &fb2[pos],
	    &fb2[pos+1],
	    &fb2[pos+2]);
	}
      }
    }

    if (tex1_w > 0) {
      // Update 256x256 texture
      glBindTexture(GL_TEXTURE_2D,texture);
      glFlush();
      glTexSubImage2D(GL_TEXTURE_2D,0,rx+xskip,ry+yskip,tex1_w,rh,GL_RGBA,
        GL_UNSIGNED_BYTE,fb1);
    }
    if (tex2_w > 0) { // What was left for this texture
      // Update 64x256 texture
      glBindTexture(GL_TEXTURE_2D,textureb);
      glFlush();
      glTexSubImage2D(GL_TEXTURE_2D,0,tex2_x+xskip,ry+yskip,tex2_w,rh,GL_RGBA,
        GL_UNSIGNED_BYTE,fb2);
    }
  }
}

void FB2GL::palette(int i, int r, int g, int b) {
  if (flags & FB2GL_EXPAND) {
    ogl_temp_ctable[i][0]=r;
    ogl_temp_ctable[i][1]=g;
    ogl_temp_ctable[i][2]=b;
  }
  else { // Paletted texture
    ogl_ctable[i][0]=r;
    ogl_ctable[i][1]=g;
    ogl_ctable[i][2]=b;
  }
}

void FB2GL::setPalette(int f, int n) {
  char temp[256][3];
  int i;
 
  if (flags & FB2GL_EXPAND) {
    for (i=f; i<n; i++) {
      ogl_ctable[i][0] = ogl_temp_ctable[i][0];
      ogl_ctable[i][1] = ogl_temp_ctable[i][1];
      ogl_ctable[i][2] = ogl_temp_ctable[i][2];
    }
  }
  else { // Paletted texture
    glBindTexture(GL_TEXTURE_2D,texture);
    glGetColorTable(GL_TEXTURE_2D,GL_RGB,GL_UNSIGNED_BYTE,&temp);
	
    for (i=f; i<n; i++) {
      temp[i][0] = ogl_ctable[i][0];
      temp[i][1] = ogl_ctable[i][1];
      temp[i][2] = ogl_ctable[i][2];
    }
	
    glColorTable(GL_TEXTURE_2D,GL_RGB,256,GL_RGB,GL_UNSIGNED_BYTE,&temp);

    if (flags & FB2GL_320) {
      glBindTexture(GL_TEXTURE_2D,textureb);
      glColorTable(GL_TEXTURE_2D,GL_RGB,256,GL_RGB,GL_UNSIGNED_BYTE,&temp);
    }

  }

}
