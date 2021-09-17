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

extern byte *cur_image_pixels;
extern byte cur_image_size_w;
extern byte cur_image_size_h;
extern byte cur_image_coords_x;
extern byte cur_image_coords_y;
extern uint16 cur_image_offs;
extern uint16 cur_image_end;
extern byte cur_image_idx;
extern byte cur_image_anim1;
extern byte cur_image_anim2;
extern uint16 cur_frame_width;

int16 DrawPortrait(byte **desc, byte *x, byte *y, byte *width, byte *height);
void AnimPortrait(byte layer, byte index, byte delay);

void DrawBoxAroundSpot(void);

void MergeImageAndSpriteData(byte *target, int16 pitch, byte *source, uint16 w, uint16 h);
void MergeImageAndSpriteDataFlip(byte *target, int16 pitch, byte *source, uint16 w, uint16 h);

void BlinkToRed(void);
void BlinkToWhite(void);


} // End of namespace Chamber

#endif
