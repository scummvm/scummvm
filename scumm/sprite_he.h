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
	kSFChanged           = 0x1,
	kSFNeedRedraw        = 0x2,
	kSFZoomed            = 0x10,
	kSFRotated           = 0x20,
	kSFDoubleBuffered    = 0x1000,
	kSFYFlipped          = 0x2000,
	kSFXFlipped          = 0x4000,
	kSFActive            = 0x8000,
	kSFNeedPaletteRemap  = 0x80000,
	kSFDelayed           = 0x200000,
	kSFMarkDirty         = 0x400000,
	kSFBlitDirectly      = 0x2000000,
	kSF30                = 0x20000000,
	kSFImageless         = 0x40000000
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
	int paletteNum;
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

