/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002  Marcus Comstedt
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
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "dc.h"

#define SCREEN_W 320
#define SCREEN_H 240
#define MOUSE_W 64
#define MOUSE_H 64

#define TOP_OFFSET (240.0-_screen_h)

#define QACR0 (*(volatile unsigned int *)(void *)0xff000038)
#define QACR1 (*(volatile unsigned int *)(void *)0xff00003c)


#define COPYPIXEL(n) do {			\
  unsigned short _tmp = pal[*s++];		\
  d[n] = _tmp|(pal[*s++]<<16);			\
} while(0)

static void texture_memcpy64_pal(void *dest, void *src, int cnt, unsigned short *pal)
{
  unsigned char *s = (unsigned char *)src;
  unsigned int *d = (unsigned int *)(void *)
    (0xe0000000 | (((unsigned long)dest) & 0x03ffffc0));
  QACR0 = ((0xa4000000>>26)<<2)&0x1c;
  QACR1 = ((0xa4000000>>26)<<2)&0x1c;
  while(cnt--) {
    COPYPIXEL(0);
    COPYPIXEL(1);
    COPYPIXEL(2);
    COPYPIXEL(3);
    asm("pref @%0" : : "r" (s+4*16));
    COPYPIXEL(4);
    COPYPIXEL(5);
    COPYPIXEL(6);
    COPYPIXEL(7);
    asm("pref @%0" : : "r" (d));
    d += 8;
    COPYPIXEL(0);
    COPYPIXEL(1);
    COPYPIXEL(2);
    COPYPIXEL(3);
    asm("pref @%0" : : "r" (s+4*16));
    COPYPIXEL(4);
    COPYPIXEL(5);
    COPYPIXEL(6);
    COPYPIXEL(7);
    asm("pref @%0" : : "r" (d));
    d += 8;
  }
}

void commit_dummy_transpoly()
{
  struct polygon_list mypoly;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_ENABLE_ALPHA;
  mypoly.texture = 0;
  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;
  ta_commit_list(&mypoly);
}


void OSystem_Dreamcast::set_palette(const byte *colors, uint start, uint num)
{
  unsigned short *dst = palette + start;
  if(num>0)
    while( num-- ) {
      *dst++ = ((colors[0]<<7)&0x7c00)|
	((colors[1]<<2)&0x03e0)|
	((colors[2]>>3)&0x001f);
      colors += 4;
    }
}

void OSystem_Dreamcast::init_size(uint w, uint h)
{
  assert(w == SCREEN_W && h <= SCREEN_H);

  _screen_h = h;
  ta_sync();
  if(!screen)
    screen = new unsigned char[SCREEN_W*SCREEN_H];
  for(int i=0; i<NUM_BUFFERS; i++)
    if(!screen_tx[i])
      screen_tx[i] = ta_txalloc(SCREEN_W*SCREEN_H*2);
  for(int i=0; i<NUM_BUFFERS; i++)
    if(!mouse_tx[i])
      mouse_tx[i] = ta_txalloc(MOUSE_W*MOUSE_H*2);
  current_buffer = 0;
  *(volatile unsigned int *)(0xa05f80e4) = SCREEN_W/32; //stride
  //  dc_reset_screen(0, 0);
}

void OSystem_Dreamcast::copy_rect(const byte *buf, int pitch, int x, int y,
				  int w, int h)
{
  unsigned char *dst = screen + y*SCREEN_W + x;
  do {
    memcpy(dst, buf, w);
    dst += SCREEN_W;
    buf += pitch;
  } while (--h);
}

bool OSystem_Dreamcast::show_mouse(bool visible)
{	
  bool last = _ms_visible;
  _ms_visible = visible;

  return last;
}

void OSystem_Dreamcast::set_mouse_pos(int x, int y)
{
  _ms_cur_x = x;
  _ms_cur_y = y;
}

void OSystem_Dreamcast::set_mouse_cursor(const byte *buf, uint w, uint h,
					 int hotspot_x, int hotspot_y)
{
  _ms_cur_w = w;
  _ms_cur_h = h;
  
  _ms_hotspot_x = hotspot_x;
  _ms_hotspot_y = hotspot_y;
  
  _ms_buf = (byte*)buf;
}

void OSystem_Dreamcast::set_shake_pos(int shake_pos)
{
  _current_shake_pos = shake_pos;
}

void OSystem_Dreamcast::update_screen(void)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  unsigned short *dst = (unsigned short *)screen_tx[current_buffer];
  unsigned char *src = screen;

  // while((*((volatile unsigned int *)(void*)0xa05f810c) & 0x3ff) != 200);
  // *((volatile unsigned int *)(void*)0xa05f8040) = 0xff0000;
  
  for( int y = 0; y<_screen_h; y++ )
  {
    texture_memcpy64_pal( dst, src, SCREEN_W>>5, palette );
    src += SCREEN_W;
    dst += SCREEN_W;
  }

  // *((volatile unsigned int *)(void*)0xa05f8040) = 0x00ff00;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|TA_CMD_POLYGON_TEXTURED;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_TEXTURE_REPLACE|
    TA_POLYMODE2_U_SIZE_512|TA_POLYMODE2_V_SIZE_512;
  mypoly.texture = TA_TEXTUREMODE_ARGB1555|TA_TEXTUREMODE_NON_TWIDDLED|
    TA_TEXTUREMODE_STRIDE|TA_TEXTUREMODE_ADDRESS(screen_tx[current_buffer]);

  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;
  
  ta_begin_frame();
  // *((volatile unsigned int *)(void*)0xa05f8040) = 0x0000ff;
  ta_commit_list(&mypoly);
  
  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.ocolour = 0;
  myvertex.colour = 0;
  myvertex.z = 0.5;
  myvertex.u = 0.0;
  myvertex.v = 0.0;

  myvertex.x = 0.0;
  myvertex.y = _current_shake_pos*2.0+TOP_OFFSET;
  ta_commit_list(&myvertex);

  myvertex.x = SCREEN_W*2.0;
  myvertex.u = SCREEN_W/512.0;
  ta_commit_list(&myvertex);

  myvertex.x = 0.0;
  myvertex.y += _screen_h*2.0;
  myvertex.u = 0.0;
  myvertex.v = _screen_h*(1/512.0);
  ta_commit_list(&myvertex);

  myvertex.x = SCREEN_W*2.0;
  myvertex.u = SCREEN_W/512.0;
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);

  ta_commit_end();
  // *((volatile unsigned int *)(void*)0xa05f8040) = 0xffff00;
  drawMouse(_ms_cur_x, _ms_cur_y, _ms_cur_w, _ms_cur_h, _ms_buf, _ms_visible);
  // *((volatile unsigned int *)(void*)0xa05f8040) = 0xff00ff;
  ta_commit_frame();

  current_buffer++;
  current_buffer &= NUM_BUFFERS-1;
  // *((volatile unsigned int *)(void*)0xa05f8040) = 0x0;
}

void OSystem_Dreamcast::drawMouse(int xdraw, int ydraw, int w, int h,
				  unsigned char *buf, bool visible)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  unsigned short *dst = (unsigned short *)mouse_tx[current_buffer];
  int y=0;

  if(visible && w<=MOUSE_W && h<=MOUSE_H)
    for(int y=0; y<h; y++) {
      int x;
      for(x=0; x<w; x++)
	if(*buf == 0xff) {
	  *dst++ = 0;
	  buf++;
	} else
	  *dst++ = palette[*buf++]|0x8000;
      dst += MOUSE_W-x;
    }
  else
    w = h = 0;
  
  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|TA_CMD_POLYGON_TEXTURED;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_TEXTURE_REPLACE|
    TA_POLYMODE2_U_SIZE_64|TA_POLYMODE2_V_SIZE_64;
  mypoly.texture = TA_TEXTUREMODE_ARGB1555|TA_TEXTUREMODE_NON_TWIDDLED|
    TA_TEXTUREMODE_ADDRESS(mouse_tx[current_buffer]);

  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;
  
  ta_commit_list(&mypoly);
  
  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.ocolour = 0;
  myvertex.colour = 0xffff00;
  myvertex.z = 0.25;
  myvertex.u = 0.0;
  myvertex.v = 0.0;

  myvertex.x = (xdraw-_ms_hotspot_y)*2.0;
  myvertex.y = (ydraw+_current_shake_pos-_ms_hotspot_x)*2.0 + TOP_OFFSET;
  ta_commit_list(&myvertex);

  myvertex.x += w*2.0;
  myvertex.u = w*(1.0/MOUSE_W);
  ta_commit_list(&myvertex);

  myvertex.x = (xdraw-_ms_hotspot_y)*2.0;
  myvertex.y += h*2.0;
  myvertex.u = 0.0;
  myvertex.v = h*(1.0/MOUSE_H);
  ta_commit_list(&myvertex);

  myvertex.x += w*2.0;
  myvertex.u = w*(1.0/MOUSE_W);
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);
}

