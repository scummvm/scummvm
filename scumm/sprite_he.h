/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project 
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

#ifndef SPRITE_HE_H
#define SPRITE_HE_H

namespace Scumm {

enum SpriteFlags {
	kSF01      = (1 << 0),
	kSF02      = (1 << 1),
	kSF03      = (1 << 2),
	kSF04      = (1 << 3),
	kSFZoomed  = (1 << 4),
	kSFRotated = (1 << 5),
	kSF07      = (1 << 6),
	kSF08      = (1 << 7),
	kSF09      = (1 << 8),
	kSF10      = (1 << 9),
	kSF11      = (1 << 10),
	kSF12      = (1 << 11),
	kSF13      = (1 << 12),
	kSF14      = (1 << 13),
	kSF15      = (1 << 14),
	kSF16      = (1 << 15),
	kSF17      = (1 << 16),
	kSF18      = (1 << 17),
	kSF19      = (1 << 18),
	kSF20      = (1 << 19),
	kSF21      = (1 << 20),
	kSF22      = (1 << 21),
	kSF23      = (1 << 22),
	kSF24      = (1 << 23),
	kSF25      = (1 << 24),
	kSF26      = (1 << 25),
	kSF27      = (1 << 26),
	kSF28      = (1 << 27),
	kSF29      = (1 << 28),
	kSF30      = (1 << 29),
	kSF31      = (1 << 30),
	kSF32      = (1 << 31)
};

enum SpriteGroupFlags {
	kSGF01  = 1 << 0,
	kSGF02  = 1 << 1
};

struct SpriteInfo {
	int field_0;
	int flags;
	int res_id;
	int res_state;
	int group_num;
	int field_14;
	int field_18;
	int bbox_xmin;
	int bbox_ymin;
	int bbox_xmax;
	int bbox_ymax;
	int field_2C;
	int field_30;
	int field_34;
	int field_38;
	int tx;
	int ty;
	int field_44;
	int field_48;
	int field_4C;
	int imglist_num;
	int field_54;
	int res_wiz_states;
	int rot_angle;
	int zoom;
	int field_64;
	int field_68;
	int field_6C;
	int field_70;
	int field_74;
	int field_78;
	int field_7C;
	int field_80;
	int class_flags;
};

struct SpriteGroup {
	int field_0;
	int field_4;
	int field_8;
	int field_C;
	int field_10;
	int flags;
	int tx;
	int ty;
	int field_20;
	int scaling;
	int scale_x;
	int scale_y;
	int field_30;
	int field_34;
	int field_38;
	int field_3C;
};

	int _varNumSpriteGroups;
	int _numSpritesToProcess;
	int _varNumSprites;
	int _varNumImgLists;
	SpriteInfo *_spriteTable;
	SpriteGroup *_spriteGroups;
	SpriteInfo **_activeSpritesTable;
	uint32 *_imageListTable; // XXX wrong type
	uint16 *_imageListStack;
	int _curSprImageListNum;


} // End of namespace Scumm

#endif

