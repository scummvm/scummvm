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

#ifndef CHAMBER_PORTRAIT_H
#define CHAMBER_PORTRAIT_H

namespace Chamber {

extern unsigned char *cur_image_pixels;
extern unsigned char cur_image_size_w;
extern unsigned char cur_image_size_h;
extern unsigned char cur_image_coords_x;
extern unsigned char cur_image_coords_y;
extern unsigned int cur_image_offs;
extern unsigned int cur_image_end;
extern unsigned char cur_image_idx;
extern unsigned char cur_image_anim1;
extern unsigned char cur_image_anim2;
extern unsigned int cur_frame_width;

int DrawPortrait(unsigned char **desc, unsigned char *x, unsigned char *y, unsigned char *width, unsigned char *height);
void AnimPortrait(unsigned char layer, unsigned char index, unsigned char delay);

void DrawBoxAroundSpot(void);

void MergeImageAndSpriteData(unsigned char *target, signed int pitch, unsigned char *source, unsigned int w, unsigned int h);
void MergeImageAndSpriteDataFlip(unsigned char *target, signed int pitch, unsigned char *source, unsigned int w, unsigned int h);

void BlinkToRed(void);
void BlinkToWhite(void);


} // End of namespace Chamber

#endif
