/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002-2004  Marcus Comstedt
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

class Icon
{
 private:
  unsigned char bitmap[32*32/2];
  unsigned int palette[16];
  void *texture;

  int find_unused_pixel();
  bool load_image1(const void *data, int len, int offs);
  bool load_image2(const void *data, int len);

 public:
  bool load(const void *data, int len, int offs = 0);
  bool load(const char *filename);
  void create_texture();
  void set_palette(int pal);
  void draw(float x1, float y1, float x2, float y2, int pal,
	    unsigned argb = 0xffffffff);
  void create_vmicon(void *buffer);
};
