/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_chdir

#include "dc.h"
#include "dcutils.h"
#include <ronin/gddrive.h>


int getCdState()
{
  unsigned int param[4];
  gdGdcGetDrvStat(param);
  return param[0];
}

extern "C" {
  int dummy_cdfs_get_volume_id(char *, unsigned int) {
    return -1;
  }
  int cdfs_get_volume_id(char *, unsigned int) __attribute__ ((weak, alias ("dummy_cdfs_get_volume_id")));
}

DiscLabel::DiscLabel() {
  if (cdfs_get_volume_id(buf, 32) < 0)
    memset(buf, '*', 32);
}

bool DiscLabel::operator==(const DiscLabel &other) const {
  return !memcmp(buf, other.buf, 32);
}

void DiscLabel::get(char *p) const {
  memcpy(p, buf, 32);
  p[32] = 0;
}


void draw_solid_quad(float x1, float y1, float x2, float y2,
		     int c0, int c1, int c2, int c3)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  mypoly.cmd =
	TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
	TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|
	TA_CMD_POLYGON_GOURAUD_SHADING;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
	TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED;
  mypoly.texture = 0;

  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.ocolour = 0;
  myvertex.z = 0.5;
  myvertex.u = 0.0;
  myvertex.v = 0.0;

  myvertex.colour = c0;
  myvertex.x = x1;
  myvertex.y = y1;
  ta_commit_list(&myvertex);

  myvertex.colour = c1;
  myvertex.x = x2;
  ta_commit_list(&myvertex);

  myvertex.colour = c2;
  myvertex.x = x1;
  myvertex.y = y2;
  ta_commit_list(&myvertex);

  myvertex.colour = c3;
  myvertex.x = x2;
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);
}

void draw_trans_quad(float x1, float y1, float x2, float y2,
		     int c0, int c1, int c2, int c3)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  mypoly.cmd =
	TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
	TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|
	TA_CMD_POLYGON_GOURAUD_SHADING;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
	TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
	TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_ENABLE_ALPHA;
  mypoly.texture = 0;

  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.ocolour = 0;
  myvertex.z = 0.5;
  myvertex.u = 0.0;
  myvertex.v = 0.0;

  myvertex.colour = c0;
  myvertex.x = x1;
  myvertex.y = y1;
  ta_commit_list(&myvertex);

  myvertex.colour = c1;
  myvertex.x = x2;
  ta_commit_list(&myvertex);

  myvertex.colour = c2;
  myvertex.x = x1;
  myvertex.y = y2;
  ta_commit_list(&myvertex);

  myvertex.colour = c3;
  myvertex.x = x2;
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);
}

DiscSwap::DiscSwap(const char *label, unsigned int argb_) : argb(argb_) {
  x = 320 - 7 * strlen(label);
  lab.create_texture(label);
}

void DiscSwap::run()
{
  int wasopen = 0;
  for (;;) {
	int s = getCdState();
	if (s >= 6)
	  wasopen = 1;
	if (s > 0 && s < 6 && wasopen) {
	  cdfs_reinit();
	  chdir("/");  // Expect this one to fail with ERR_DISKCHG
	  chdir("/");  // but this one to succeed
	  return;
	}

	ta_begin_frame();
	background();
	ta_commit_end();
	lab.draw(x, 200.0, argb);
	ta_commit_frame();

	interact();
  }
}

namespace DC_Flash {

  static int syscall_info_flash(int sect, int *info)
  {
	return (*(int (**)(int, void*, int, int))0x8c0000b8)(sect,info,0,0);
  }

  static int syscall_read_flash(int offs, void *buf, int cnt)
  {
	return (*(int (**)(int, void*, int, int))0x8c0000b8)(offs,buf,cnt,1);
  }

  static int flash_crc(const char *buf, int size)
  {
	int i, c, n = -1;
	for(i=0; i<size; i++) {
	  n ^= (buf[i]<<8);
	  for(c=0; c<8; c++)
	if(n & 0x8000)
	  n = (n << 1) ^ 4129;
	else
	  n <<= 1;
	}
	return (unsigned short)~n;
  }

  int flash_read_sector(int partition, int sec, unsigned char *dst)
  {
	int s, r, n, b, bmb, got=0;
	int info[2];
	char buf[64];
	char bm[64];

	if((r = syscall_info_flash(partition, info))<0)
	  return r;

	if((r = syscall_read_flash(info[0], buf, 64))<0)
	  return r;

	if(memcmp(buf, "KATANA_FLASH", 12) ||
	   buf[16] != partition || buf[17] != 0)
	  return -2;

	n = (info[1]>>6)-1-((info[1] + 0x7fff)>>15);
	bmb = n+1;
	for(b = 0; b < n; b++) {
	  if(!(b&511)) {
	if((r = syscall_read_flash(info[0] + (bmb++ << 6), bm, 64))<0)
	  return r;
	  }
	  if(!(bm[(b>>3)&63] & (0x80>>(b&7)))) {
	if((r = syscall_read_flash(info[0] + ((b+1) << 6), buf, 64))<0)
	  return r;
	else if((s=READ_LE_UINT16(buf+0)) == sec &&
		flash_crc(buf, 62) == READ_LE_UINT16(buf+62)) {
	  memcpy(dst+(s-sec)*60, buf+2, 60);
	  got=1;
	}
	  }
	}
	return got;
  }

  int get_locale_setting()
  {
	unsigned char data[60];
	if (flash_read_sector(2,5,data) == 1)
	  return data[5];
	else
	  return -1;
  }

} // End of namespace DC_Flash
