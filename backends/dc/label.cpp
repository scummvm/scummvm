/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002-2003  Marcus Comstedt
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

#include <ronin/ronin.h>
#include <string.h>

#include "label.h"

static void *get_romfont_address()
{
  void *ret;
  __asm__("jsr @%1; mov #0,r1; mov r0,%0" :
	  "=r" (ret) : "r" (*(void **)0x8c0000b4) :
	  "pr", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
  return ret;
}

static void draw_char(unsigned short *dst, int mod, int c, void *font_base)
{
  unsigned char *src;
  int i, j;
  if(c<=32 || c>255 || (c>=127 && c<160)) c=160;
  if(c<128) c -= 32; else c -= 64;
  src = c*36 + (unsigned char *)font_base;
  for(i=0; i<12; i++) {
    int n = (src[0]<<16)|(src[1]<<8)|src[2];
    for(j=0; j<12; j++, n<<=1)
      if(n & (1<<23)) {
	dst[j] = 0xffff;
	dst[j+1] = 0xffff;
	dst[j+2] = 0xa108;
	dst[j+mod] = 0xa108;
	dst[j+mod+1] = 0xa108;
      }
    dst += mod;
    for(j=0; j<12; j++, n<<=1)
      if(n & (1<<23)) {
	dst[j] = 0xffff;
	dst[j+1] = 0xffff;
	dst[j+2] = 0xa108;
	dst[j+mod] = 0xa108;
	dst[j+mod+1] = 0xa108;
      }
    dst += mod;
    src += 3;
  }
}

void Label::create_texture(const char *text)
{
  void *font = get_romfont_address();
  int l = strlen(text);
  if(l>64) l=64;
  int w = 14*l;
  for(tex_u=TA_POLYMODE2_U_SIZE_8, u=8; u<w; u<<=1, tex_u += 1<<3);
  int tsz = u*32;
  unsigned short *tex = (unsigned short *)ta_txalloc(tsz*2);
  for(int i=0; i<tsz; i++)
    tex[i] = 0;
  int p=l*14;
  while(l>0)
    draw_char(tex+(p-=14), u, text[--l], font);
  texture = tex;
}

void Label::draw(float x, float y, unsigned int argb, float scale)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|TA_CMD_POLYGON_TEXTURED;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_ENABLE_ALPHA|
    TA_POLYMODE2_TEXTURE_MODULATE_ALPHA|TA_POLYMODE2_V_SIZE_32|tex_u;
  mypoly.texture = TA_TEXTUREMODE_ARGB1555|TA_TEXTUREMODE_NON_TWIDDLED|
    TA_TEXTUREMODE_ADDRESS(texture);

  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.ocolour = 0;
  myvertex.colour = argb;
  myvertex.z = 0.5;
  myvertex.u = 0.0;
  myvertex.v = 0.0;

  myvertex.x = x;
  myvertex.y = y;
  ta_commit_list(&myvertex);

  myvertex.x = x+u*scale;
  myvertex.u = 1.0;
  ta_commit_list(&myvertex);

  myvertex.x = x;
  myvertex.y = y+25.0*scale;
  myvertex.u = 0.0;
  myvertex.v = 25.0/32.0;
  ta_commit_list(&myvertex);

  myvertex.x = x+u*scale;
  myvertex.u = 1.0;
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);  
}

