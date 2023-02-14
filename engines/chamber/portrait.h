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

extern volatile byte vblank_ticks;

int16 drawPortrait(byte **desc, byte *x, byte *y, byte *width, byte *height);
void animPortrait(byte layer, byte index, byte delay);

void drawBoxAroundSpot(void);

void mergeImageAndSpriteData(byte *target, int16 pitch, byte *source, uint16 w, uint16 h);
void mergeImageAndSpriteDataFlip(byte *target, int16 pitch, byte *source, uint16 w, uint16 h);

void blinkToRed(void);
void blinkToWhite(void);


} // End of namespace Chamber

#endif
