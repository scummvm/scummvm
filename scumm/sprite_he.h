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
	int32 id;
	int32 zorder;
	int32 flags;
	int32 resId;
	int32 resState;
	int32 groupNum;
	int32 paletteNum;
	int32 zorderPriority;
	Common::Rect bbox;
	int32 dx;
	int32 dy;
	Common::Point pos;
	int32 tx;
	int32 ty;
	int32 field_44;
	int32 curImageState;
	int32 curResId;
	int32 imglistNum;
	int32 xmapNum;
	int32 res_wiz_states;
	int32 angle;
	int32 zoom;
	int32 delayCount;
	int32 curAngle;
	int32 curZoom;
	int32 imgFlags;
	int32 field_74;
	int32 delayAmount;
	int32 field_7C;
	int32 field_80;
	int32 field_84;
	int32 classFlags;
	int32 field_8C;
	int32 field_90;
	int32 field_94;
};

struct SpriteGroup {
	Common::Rect bbox;
	int32 zorderPriority;
	int32 flags;
	int32 tx;
	int32 ty;
	int32 dstResNum;
	int32 scaling;
//	int32 scaleX;
//	int32 scaleY;
	int32 scale_x_ratio_mul;
	int32 scale_x_ratio_div;
	int32 scale_y_ratio_mul;
	int32 scale_y_ratio_div;
};

} // End of namespace Scumm

#endif

