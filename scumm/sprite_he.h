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
	kSFChanged           = (1 << 0),
	kSFNeedRedraw        = (1 << 1),
	kSF03                = (1 << 2),
	kSF04                = (1 << 3),
	kSFZoomed            = (1 << 4),
	kSFRotated           = (1 << 5),
	kSF07                = (1 << 6),
	kSF08                = (1 << 7),
	kSF09                = (1 << 8),
	kSF10                = (1 << 9),
	kSF11                = (1 << 10),
	kSF12                = (1 << 11),
	kSFDoubleBuffered    = (1 << 12),
	kSFYFlipped          = (1 << 13),
	kSFXFlipped          = (1 << 14),
	kSFActive            = (1 << 15),
	kSF17                = (1 << 16),
	kSF18                = (1 << 17),
	kSF19                = (1 << 18),
	kSFNeedPaletteRemap  = (1 << 19),
	kSF21                = (1 << 20),
	kSFDelayed           = (1 << 21),
	kSF23                = (1 << 22),
	kSF24                = (1 << 23),
	kSF25                = (1 << 24),
	kSFBlitDirectly      = (1 << 25),
	kSF27                = (1 << 26),
	kSF28                = (1 << 27),
	kSF29                = (1 << 28),
	kSF30                = (1 << 29),
	kSFImageless         = (1 << 30),
	kSF32                = (1 << 31)
};

enum SpriteGroupFlags {
	kSGFClipBox     = (1 << 0)
};

struct SpriteInfo {
	int id;
	int zorder;
	int flags;
	int res_id;
	int res_state;
	int group_num;
	int field_14;
	int zorderPriority;
	Common::Rect bbox;
	int dx;
	int dy;
	Common::Point pos;
	int tx;
	int ty;
	int field_44;
	int cur_img_state;
	int cur_res_id;
	int imglist_num;
	int field_54;
	int res_wiz_states;
	int rot_angle;
	int zoom;
	int delayCount;
	int cur_rot_angle;
	int cur_zoom;
	int imgFlags;
	int field_74;
	int delayAmount;
	int field_7C;
	int field_80;
	int class_flags;
	int field_88;
	int field_8C;
	int field_90;
};

struct SpriteGroup {
	Common::Rect bbox;
	int zorderPriority;
	int flags;
	int tx;
	int ty;
	int field_20;
	int scaling;
	int scale_x;
	int scale_y;
	int scale_x_ratio_mul;
	int scale_x_ratio_div;
	int scale_y_ratio_mul;
	int scale_y_ratio_div;
};

} // End of namespace Scumm

#endif

