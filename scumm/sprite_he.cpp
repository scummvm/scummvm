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

#include "stdafx.h"

#include "scumm/intern.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sprite_he.h"
#include "scumm/wiz_he.h"

namespace Scumm {

void ScummEngine_v90he::allocateArrays() {
	ScummEngine::allocateArrays();
	spritesAllocTables(_numSprites, MAX(64, _numSprites / 4), 64);
}

void ScummEngine_v90he::getSpriteBounds(int spriteId, bool checkGroup, Common::Rect &bound) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	int16 spr_wiz_x, spr_wiz_y;
	int rot_angle, zoom, x1, y1;
	int32 w, h;

	SpriteInfo *spi = &_spriteTable[spriteId];

	loadImgSpot(spi->res_id, spi->res_state, spr_wiz_x, spr_wiz_y);
	if (checkGroup && spi->group_num) {
		SpriteGroup *spg = &_spriteGroups[spi->group_num];

		if (spg->scaling) {
			x1 = spi->tx * spg->scale_x - spr_wiz_x + spg->tx;
			y1 = spi->ty * spg->scale_y - spr_wiz_y + spg->ty;
		} else {
			x1 = spi->tx - spr_wiz_x + spg->tx;
			y1 = spi->ty - spr_wiz_y + spg->ty;
		}
	} else {
		x1 = spi->tx - spr_wiz_x;
		y1 = spi->ty - spr_wiz_y;
	}

	if (spi->res_id) {
		rot_angle = spi->rot_angle;
		zoom = spi->zoom;
		getWizImageDim(spi->res_id, spi->res_state, w, h);
		if (!(spi->flags & (kSFZoomed | kSFRotated))) {
			bound.left = x1;
			bound.top = y1;
			bound.right = x1 + w;
			bound.bottom = y1 + h;
		} else {
			Common::Point pts[4];

			pts[1].x = pts[2].x = w / 2 - 1;
			pts[0].x = pts[0].y = pts[1].y = pts[3].x = -w / 2;
			pts[2].y = pts[3].y = h / 2 - 1;

			if ((spi->flags & kSFZoomed) && zoom) {
				for (int j = 0; j < 4; ++j) {
					pts[j].x = pts[j].x * zoom / 256;
					pts[j].y = pts[j].y * zoom / 256;
				}
			}
			if ((spi->flags & kSFRotated) && rot_angle)
				_wiz.polygonRotatePoints(pts, 4, rot_angle);

			for (int j = 0; j < 4; ++j) {
				pts[j].x += x1;
				pts[j].y += y1;
			}

			_wiz.polygonCalcBoundBox(pts, 4, bound);
		}
	} else {
		bound.left = 1234;
		bound.top = 1234;
		bound.right = -1234;
		bound.bottom = -1234;
	}
}

//
// spriteInfoGet functions
//
int ScummEngine_v90he::findSpriteWithClassOf(int x_pos, int y_pos, int spriteGroupId, int type, int num, int *args) {
	bool cond;
	int code, classId, x, y;
	debug(1, "findSprite: x %d, y %d, spriteGroup %d, type %d, num %d", x_pos, y_pos, spriteGroupId, type, num);

	for (int i = (_numSpritesToProcess - 1); i >= 0; i--) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!spi->cur_res_id)
			continue;

		if (spriteGroupId && spi->group_num != spriteGroupId)
			continue;

		cond = true;
		for (int j = 0; j < num; j++) {
			code = classId = args[j];
			classId &= 0x7F;
			checkRange(32, 1, classId, "class %d out of range in statement");
			if (code & 0x80) {
				if (!(spi->class_flags & (1 << (classId - 1))))
					cond = 0;
			} else {
				if ((spi->class_flags & (1 << (classId - 1))))
					cond = 0;
			}
		}
		if (!cond)
			continue;

		if (type) {
			if (spi->bbox.left > spi->bbox.right)
				continue;
			if (spi->bbox.top > spi->bbox.bottom)
				continue;
			if (spi->bbox.left > x_pos)
				continue;
			if (spi->bbox.top > y_pos)
				continue;
			if (spi->bbox.right < x_pos)
				continue;
			if (spi->bbox.bottom < y_pos)
				continue;
			return spi->id;
		} else {
			int resId, state, rot_angle, zoom;
			int32 w, h;

			resId = spi->cur_res_id;
			if (spi->field_80) {
				int16 x1, x2, y1, y2;

				state = spi->cur_img_state / getWizImageStates(spi->field_80);

				x = x_pos - spi->pos.x;
				y = y_pos - spi->pos.y;

				loadImgSpot(spi->cur_res_id, state, x1, y1);
				loadImgSpot(spi->field_80, state, x2, y2);

				x += (x2 - x1);
				y += (y2 - y1);
			} else {
				if (spi->bbox.left > spi->bbox.right)
					continue;
				if (spi->bbox.top > spi->bbox.bottom)
					continue;
				if (spi->bbox.left > x_pos)
					continue;
				if (spi->bbox.top > y_pos)
					continue;
				if (spi->bbox.right < x_pos)
					continue;
				if (spi->bbox.bottom < y_pos)
					continue;

				x = x_pos - spi->pos.x;
				y = y_pos - spi->pos.y;
				state = spi->cur_img_state;
			}

			rot_angle = spi->cur_rot_angle;
			zoom = spi->cur_zoom;
			if ((spi->flags & kSFZoomed) || (spi->flags & kSFRotated)) {
				if (spi->flags & kSFZoomed && zoom) {
					x = x * 256 / zoom;
					y = y * 256 / zoom;
				}
				if (spi->flags & kSFRotated && rot_angle) {
					rot_angle = (360 - rot_angle) % 360;
					Common::Point pts[1];
					_wiz.polygonRotatePoints(pts, 1, rot_angle);
				}

				getWizImageDim(resId, state, w, h);
				x += w / 2;
				y += h / 2;
			}

			if(isWizPixelNonTransparent(rtImage, resId, state, x, y, spi->imgFlags))
				return spi->id;
		}
	}

	return 0;
}

int ScummEngine_v90he::spriteInfoGet_classFlags(int spriteId, int num, int *args) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	int code, classId;

	if (num == 0)
		return _spriteTable[spriteId].class_flags; 

	for (int i = 0; i < num; i++) {
		code = classId = args[i];
		classId &= 0x7F;
		checkRange(32, 1, classId, "class %d out of range in statement");
		if (code & 0x80) {
			if (!(_spriteTable[spriteId].class_flags & (1 << (classId - 1))))
				return 0;
		} else {
			if ((_spriteTable[spriteId].class_flags & (1 << (classId - 1))))
				return 0;
		}
	}

	return 1;
}

int ScummEngine_v90he::spriteInfoGet_flagDoubleBuffered(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFDoubleBuffered) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flagYFlipped(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFYFlipped) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flagXFlipped(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFXFlipped) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flagActive(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFActive) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flagNeedPaletteRemap(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFNeedPaletteRemap) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_22(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFDelayed) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_23(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSF23) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flagHasImage(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFImageless) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_resId(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].res_id;
}

int ScummEngine_v90he::spriteInfoGet_resState(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].res_state;
}

int ScummEngine_v90he::spriteInfoGet_groupNum(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].group_num;
}

int ScummEngine_v90he::spriteInfoGet_field_14(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_14;
}

int ScummEngine_v90he::spriteInfoGet_zorderPriority(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].zorderPriority;
}

int ScummEngine_v90he::spriteInfoGet_grp_tx(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].group_num)
		return _spriteTable[spriteId].tx + _spriteGroups[_spriteTable[spriteId].group_num].tx;
	else
		return _spriteTable[spriteId].tx;
}

int ScummEngine_v90he::spriteInfoGet_grp_ty(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].group_num)
		return _spriteTable[spriteId].ty + _spriteGroups[_spriteTable[spriteId].group_num].ty;
	else
		return _spriteTable[spriteId].ty;
}

int ScummEngine_v90he::spriteInfoGet_field_44(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_44;
}

int ScummEngine_v90he::spriteInfoGet_field_54(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_54;
}

int ScummEngine_v90he::spriteInfoGet_wizSize(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].res_wiz_states;
}

int ScummEngine_v90he::spriteInfoGet_zoom(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].zoom;
}

int ScummEngine_v90he::spriteInfoGet_delayAmount(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].delayAmount;
}

int ScummEngine_v90he::spriteInfoGet_field_7C(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_7C;
}

int ScummEngine_v90he::spriteInfoGet_field_80(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_80;
}

int ScummEngine_v90he::spriteInfoGet_field_88(int spriteId, int type) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (type == 0x7B)
		return _spriteTable[spriteId].field_88;
	else
		return 0;
}

void ScummEngine_v90he::getSpriteImageDim(int spriteId, int32 &w, int32 &h) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].res_id) {
		getWizImageDim(_spriteTable[spriteId].res_id, _spriteTable[spriteId].res_state, w, h);
	} else {
		w = 0;
		h = 0;
	}
}

void ScummEngine_v90he::spriteInfoGet_tx_ty(int spriteId, int32 &tx, int32 &ty) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	tx = _spriteTable[spriteId].tx;
	ty = _spriteTable[spriteId].ty;
}

void ScummEngine_v90he::spriteInfoGet_dx_dy(int spriteId, int32 &dx, int32 &dy) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	dx = _spriteTable[spriteId].dx;
	dy = _spriteTable[spriteId].dy;
}

//
// spriteGroupGet functions
//
int ScummEngine_v90he::spriteGroupGet_allocateGroupSpritesList(int spriteGroupId) {
	int i, j = 0, sprites = 0;

	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId)
			sprites++;
	}

	if (!sprites)
		return 0;

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, sprites);
	writeArray(0, 0, 0, sprites);
	
	for (i = (_varNumSprites - 1); i > 0; i--) {
		if (_spriteTable[i].group_num == spriteGroupId)
			writeArray(0, 0, ++j, i);
	}
	
	return readVar(0);
}

int ScummEngine_v90he::spriteGroupGet_zorderPriority(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].zorderPriority;
}

int ScummEngine_v90he::spriteGroupGet_field_20(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].field_20;
}

int ScummEngine_v90he::spriteGroupGet_scale_x_ratio_mul(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].scale_x_ratio_mul;
}

int ScummEngine_v90he::spriteGroupGet_scale_x_ratio_div(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].scale_x_ratio_div;
}

int ScummEngine_v90he::spriteGroupGet_scale_y_ratio_mul(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].scale_y_ratio_mul;
}

int ScummEngine_v90he::spriteGroupGet_scale_y_ratio_div(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].scale_y_ratio_div;
}

void ScummEngine_v90he::spriteGroupGet_tx_ty(int spriteGroupId, int32 &tx, int32 &ty) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	tx = _spriteGroups[spriteGroupId].tx;
	ty = _spriteGroups[spriteGroupId].ty;
}

//
// spriteInfoSet functions
//
void ScummEngine_v90he::spriteInfoSet_field_14(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].field_14 != value) {
		_spriteTable[spriteId].field_14 = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_field_7C(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].field_7C != value) {
		_spriteTable[spriteId].field_7C = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_field_80(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].field_80 = value;
}

void ScummEngine_v90he::spriteInfoSet_resState(int spriteId, int state) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].res_id) {
		int res_wiz_states = _spriteTable[spriteId].res_wiz_states - 1;
		state = MAX(0, state);
		state = MIN(state, res_wiz_states);
	
		if (_spriteTable[spriteId].res_state != state) {
			_spriteTable[spriteId].res_state = state;
			_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void ScummEngine_v90he::spriteInfoSet_tx_ty(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].tx != value1 || _spriteTable[spriteId].ty != value2) {
		_spriteTable[spriteId].tx = value1;
		_spriteTable[spriteId].ty = value2;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_groupNum(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	checkRange(_varNumSpriteGroups, 0, value, "Invalid sprite group %d");

	_spriteTable[spriteId].group_num = value;
	_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_dx_dy(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].dx = value1;
	_spriteTable[spriteId].dy = value2;
}

void ScummEngine_v90he::spriteInfoSet_field_54(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].field_54 = value;
	if (_spriteTable[spriteId].res_id)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_field_44(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].field_44 = value2;
}

void ScummEngine_v90he::spriteInfoSet_zorderPriority(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].zorderPriority = value;
}

void ScummEngine_v90he::spriteInfoSet_Inc_tx_ty(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].tx += value1;
	_spriteTable[spriteId].ty += value2;

	if  (value1 || value2)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_zoom(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].flags |= kSFZoomed;

	if (_spriteTable[spriteId].zoom != value) {
		_spriteTable[spriteId].zoom = value;

		if (_spriteTable[spriteId].res_id)
			_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_rotAngle(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].flags |= kSFRotated;

	if (_spriteTable[spriteId].rot_angle != value) {
		_spriteTable[spriteId].rot_angle = value;

		if (_spriteTable[spriteId].res_id)
			_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_flagDoubleBuffered(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFDoubleBuffered;
	else
		_spriteTable[spriteId].flags &= ~kSFDoubleBuffered;

	if (_spriteTable[spriteId].res_id && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flagYFlipped(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFYFlipped;
	else
		_spriteTable[spriteId].flags &= ~kSFYFlipped;

	if (_spriteTable[spriteId].res_id && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flagXFlipped(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFXFlipped;
	else
		_spriteTable[spriteId].flags &= ~kSFXFlipped;

	if (_spriteTable[spriteId].res_id && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flagActive(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSFActive;
	else
		_spriteTable[spriteId].flags &= ~kSFActive;
}

void ScummEngine_v90he::spriteInfoSet_flagNeedPaletteRemap(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFNeedPaletteRemap;
	else
		_spriteTable[spriteId].flags &= ~kSFNeedPaletteRemap;

	if (_spriteTable[spriteId].res_id && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flag22(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSFDelayed;
	else
		_spriteTable[spriteId].flags &= ~kSFDelayed;
}

void ScummEngine_v90he::spriteInfoSet_flag23(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSF23 | kSFBlitDirectly;
	else
		_spriteTable[spriteId].flags &= ~(kSF23 | kSFBlitDirectly);
}

void ScummEngine_v90he::spriteInfoSet_flagHasImage(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	// Note that condition is inverted
	if (!value)
		_spriteTable[spriteId].flags |= kSFImageless;
	else
		_spriteTable[spriteId].flags &= ~kSFImageless;
}

void ScummEngine_v90he::spriteInfoSet_delay(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].delayAmount = value;
	_spriteTable[spriteId].delayCount = value;
}

void ScummEngine_v90he::spriteInfoSet_setClassFlags(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	
	_spriteTable[spriteId].class_flags = value;
}

void ScummEngine_v90he::spriteInfoSet_setClassFlag(int spriteId, int classId, int toggle) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	checkRange(32, 1, classId, "class %d out of range in statement");
	
	if (toggle) {
		_spriteTable[spriteId].class_flags |= (1 << (classId - 1));
	} else {
		_spriteTable[spriteId].class_flags &= ~(1 << (classId - 1));
	}
}

void ScummEngine_v90he::spriteInfoSet_resetClassFlags(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].class_flags = 0;
}

void ScummEngine_v90he::spriteInfoSet_field_88(int spriteId, int type, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (type == 0x7B) {
		_spriteTable[spriteId].field_88 = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_resetSprite(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].rot_angle = 0;
	_spriteTable[spriteId].zoom = 0;

	int tmp = 0;
	spriteAddImageToList(spriteId, 1, &tmp);

	_spriteTable[spriteId].field_54 = 0;
	_spriteTable[spriteId].tx = 0;
	_spriteTable[spriteId].ty = 0;

	_spriteTable[spriteId].flags &= ~(kSFYFlipped | kSFXFlipped);
	_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	_spriteTable[spriteId].dx = 0;
	_spriteTable[spriteId].dy = 0;
	_spriteTable[spriteId].field_44 = 0;
	_spriteTable[spriteId].group_num = 0;
	_spriteTable[spriteId].delayAmount = 0;
	_spriteTable[spriteId].delayCount = 0;
	_spriteTable[spriteId].class_flags = 0;
	_spriteTable[spriteId].field_14 = 0;
	_spriteTable[spriteId].field_7C = 0;
	_spriteTable[spriteId].field_80 = 0;
	_spriteTable[spriteId].zorderPriority = 0;
	_spriteTable[spriteId].field_88 = 0;
}

void ScummEngine_v90he::spriteAddImageToList(int spriteId, int imageNum, int *spriteIdptr) {
	int origResId, origResWizStates;

	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	origResId = _spriteTable[spriteId].res_id;
	origResWizStates = _spriteTable[spriteId].res_wiz_states;

	_spriteTable[spriteId].res_id = *spriteIdptr;
	_spriteTable[spriteId].field_74 = 0;
	_spriteTable[spriteId].res_state = 0;

	if (_spriteTable[spriteId].res_id) {
		_spriteTable[spriteId].res_wiz_states = getWizImageStates(_spriteTable[spriteId].res_id);
		_spriteTable[spriteId].flags |= kSFActive | kSFDelayed | kSF23 | kSFBlitDirectly;

		if (_spriteTable[spriteId].res_id != origResId || _spriteTable[spriteId].res_wiz_states != origResWizStates)
			_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	} else {
		if (_spriteTable[spriteId].flags & kSFImageless)
			_spriteTable[spriteId].flags = 0;
		else
			_spriteTable[spriteId].flags = kSFChanged | kSFBlitDirectly;
		_spriteTable[spriteId].cur_res_id = 0;
		_spriteTable[spriteId].cur_img_state = 0;
		_spriteTable[spriteId].res_wiz_states = 0;
	}
}

//
// spriteGroupSet functions
//
void ScummEngine_v90he::redrawSpriteGroup(int spriteGroupId) {
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (spi->group_num == spriteGroupId) {
			spi->flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_0(int spriteGroupId, int value1, int value2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId) {
			_spriteTable[i].tx += value1;
			_spriteTable[i].ty += value2;

			if (value1 || value2)
				_spriteTable[i].flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_1(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId)
			_spriteTable[i].zorderPriority = value;
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_2(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId) {
			_spriteTable[i].group_num = value;
			_spriteTable[i].flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_3(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId) {
			if (value)
				_spriteTable[i].flags |= kSF23 | kSFBlitDirectly;
			else
				_spriteTable[i].flags &= ~(kSF23 | kSFBlitDirectly);
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_4(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId)
			spriteInfoSet_resetSprite(i);
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_5(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId) {
			_spriteTable[i].delayAmount = value;
			_spriteTable[i].delayCount = value;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_6(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId) {
			if (value)
				_spriteTable[i].flags |= kSFDelayed;
			else
				_spriteTable[i].flags &= ~kSFDelayed;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_7(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId) {
			_spriteTable[i].field_54 = value;
			if (_spriteTable[i].res_id)
				_spriteTable[i].flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_bbox(int spriteGroupId, int x1, int y1, int x2, int y2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	_spriteGroups[spriteGroupId].flags |= kSGFClipBox;
	_spriteGroups[spriteGroupId].bbox.left = x1;
	_spriteGroups[spriteGroupId].bbox.top = y1;
	_spriteGroups[spriteGroupId].bbox.right = x2;
	_spriteGroups[spriteGroupId].bbox.bottom = y2;

	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spriteGroupSet_zorderPriority(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].zorderPriority != value) {
		_spriteGroups[spriteGroupId].zorderPriority = value;
		redrawSpriteGroup(spriteGroupId);
	}
}

void ScummEngine_v90he::spriteGroupSet_tx_ty(int spriteGroupId, int value1, int value2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].tx != value1 || _spriteGroups[spriteGroupId].ty != value2) {
		_spriteGroups[spriteGroupId].tx = value1;
		_spriteGroups[spriteGroupId].ty = value2;
		redrawSpriteGroup(spriteGroupId);
	}
}

void ScummEngine_v90he::spriteGroupSet_inc_tx_ty(int spriteGroupId, int value1, int value2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (value1 || value2) {
		_spriteGroups[spriteGroupId].tx += value1;
		_spriteGroups[spriteGroupId].ty += value2;
		redrawSpriteGroup(spriteGroupId);
	}
}

void ScummEngine_v90he::spriteGroupSet_field_20(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].field_20 != value) {
		_spriteGroups[spriteGroupId].field_20 = value;
		redrawSpriteGroup(spriteGroupId);
	}
}

void ScummEngine_v90he::spriteGroupSet_scaling(int spriteGroupId) {
	if ((_spriteGroups[spriteGroupId].scale_x_ratio_mul != _spriteGroups[spriteGroupId].scale_x_ratio_div) || (_spriteGroups[spriteGroupId].scale_y_ratio_mul != _spriteGroups[spriteGroupId].scale_y_ratio_div))
		_spriteGroups[spriteGroupId].scaling = 1;
	else
		_spriteGroups[spriteGroupId].scaling = 0;

}

void ScummEngine_v90he::spriteGroupSet_scale_x_ratio_mul(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].scale_x_ratio_mul != value) {
		_spriteGroups[spriteGroupId].scale_x_ratio_mul = value;
		_spriteGroups[spriteGroupId].scale_x = _spriteGroups[spriteGroupId].scale_x_ratio_mul / _spriteGroups[spriteGroupId].scale_x_ratio_div;

		spriteGroupSet_scaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void ScummEngine_v90he::spriteGroupSet_scale_x_ratio_div(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (value == 0)
		error("spriteGroupSet_scale_x_ratio_div: Divisor must not be 0");

	if (_spriteGroups[spriteGroupId].scale_x_ratio_div != value) {
		_spriteGroups[spriteGroupId].scale_x_ratio_div = value;
		_spriteGroups[spriteGroupId].scale_x = _spriteGroups[spriteGroupId].scale_x_ratio_mul / _spriteGroups[spriteGroupId].scale_x_ratio_div;

		spriteGroupSet_scaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void ScummEngine_v90he::spriteGroupSet_scale_y_ratio_mul(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].scale_y_ratio_mul != value) {
		_spriteGroups[spriteGroupId].scale_y_ratio_mul = value;
		_spriteGroups[spriteGroupId].scale_y = _spriteGroups[spriteGroupId].scale_y_ratio_mul / _spriteGroups[spriteGroupId].scale_y_ratio_div;

		spriteGroupSet_scaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void ScummEngine_v90he::spriteGroupSet_scale_y_ratio_div(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (value == 0)
		error("spriteGroupSet_scale_y_ratio_div: Divisor must not be 0");

	if (_spriteGroups[spriteGroupId].scale_y_ratio_div != value) {
		_spriteGroups[spriteGroupId].scale_y_ratio_div = value;
		_spriteGroups[spriteGroupId].scale_y = _spriteGroups[spriteGroupId].scale_y_ratio_mul / _spriteGroups[spriteGroupId].scale_y_ratio_div;

		spriteGroupSet_scaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void ScummEngine_v90he::spriteGroupSet_flagClipBoxAnd(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	_spriteGroups[spriteGroupId].flags &= ~(kSGFClipBox);
	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spritesAllocTables(int numSprites, int numGroups, int numMaxSprites) {
	_varNumSpriteGroups = numGroups;
	_numSpritesToProcess = 0;
	_varNumSprites = numSprites;
	_varMaxSprites = numMaxSprites;
	_spriteGroups = (SpriteGroup *)malloc((_varNumSpriteGroups + 1) * sizeof(SpriteGroup));
	_spriteTable = (SpriteInfo *)malloc((_varNumSprites + 1) * sizeof(SpriteInfo));
	_activeSpritesTable = (SpriteInfo **)malloc((_varNumSprites + 1) * sizeof(SpriteInfo *));
}

void ScummEngine_v90he::spritesResetGroup(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");
	SpriteGroup *spg = &_spriteGroups[spriteGroupId];

	spg->zorderPriority = 0;
	spg->tx = spg->ty = 0;

	spg->flags &= ~kSGFClipBox;
	redrawSpriteGroup(spriteGroupId);

	spg->field_20 = 0;
	spg->scaling = 0;
	spg->scale_x = 0x3F800000;
	spg->scale_x_ratio_mul = 1;
	spg->scale_x_ratio_div = 1;
	spg->scale_y = 0x3F800000;
	spg->scale_y_ratio_mul = 1;
	spg->scale_y_ratio_div = 1;
}

void ScummEngine_v90he::spritesResetTables(bool refreshScreen) {
	memset(_spriteTable, 0, (_varNumSprites + 1) * sizeof(SpriteInfo));
	memset(_spriteGroups, 0, (_varNumSpriteGroups + 1) * sizeof(SpriteGroup));
	for (int curGrp = 1; curGrp < _varNumSpriteGroups; ++curGrp)
		spritesResetGroup(curGrp);

	if (refreshScreen) {
		gdi.copyVirtScreenBuffers(Common::Rect(_screenWidth, _screenHeight));
	}
	_numSpritesToProcess = 0;
}

void ScummEngine_v90he::spritesBlitToScreen() {
	int xmin, xmax, ymin, ymax;
	xmin = ymin = 1234;
	xmax = ymax = -1234; 
	bool firstLoop = true;
	bool refreshScreen = false;

	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!(spi->flags & kSFImageless) && (spi->flags & kSFChanged)) {
			spi->flags &= ~kSFChanged;
			if (spi->bbox.left <= spi->bbox.right && spi->bbox.top <= spi->bbox.bottom) {
				if (spi->flags & kSFBlitDirectly) {
					gdi.copyVirtScreenBuffers(spi->bbox); // XXX 0, 0x40000000);
				} else if (firstLoop) {
					xmin = spi->bbox.left;
					ymin = spi->bbox.top;
					xmax = spi->bbox.right;
					ymax = spi->bbox.bottom;
					firstLoop = false;
					refreshScreen = true;
				} else {
					if (xmin > spi->bbox.left) {
						xmin = spi->bbox.left;
					}
					if (ymin > spi->bbox.top) {
						ymin = spi->bbox.top;
					}
					if (xmax < spi->bbox.right) {
						xmax = spi->bbox.right;
					}
					if (ymax < spi->bbox.bottom) {
						ymax = spi->bbox.bottom;
					}
					refreshScreen = true;
				}
				if (!(spi->flags & (kSFNeedRedraw | kSF30)) && spi->res_id)
					spi->flags |= kSFNeedRedraw;
			}
		}
	}
	if (refreshScreen) {
		gdi.copyVirtScreenBuffers(Common::Rect(xmin, ymin, xmax, ymax)); // , 0, 0x40000000);
	}
}

void ScummEngine_v90he::spritesMarkDirty(bool unkFlag) {
	VirtScreen *vs0 = &virtscr[kMainVirtScreen];
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!(spi->flags & (kSFNeedRedraw | kSF30))) {
			if ((!unkFlag || spi->zorderPriority >= 0) && (spi->flags & kSF23)) {
				bool needRedraw = false;
				int lp = MIN(79, spi->bbox.left / 8);
				int rp = MIN(79, (spi->bbox.right + 7) / 8);
				for (; lp <= rp; ++lp) {
					if (vs0->tdirty[lp] < vs0->h && spi->bbox.bottom >= vs0->bdirty[lp] && spi->bbox.top <= vs0->tdirty[lp]) {
						needRedraw = true;
						break;
					}
				}
				if (needRedraw) {
					spi->flags |= kSFNeedRedraw;
				}
			}
		}
	}
}

void ScummEngine_v90he::spritesUpdateImages() {
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (spi->dx || spi->dy) {
			int tx = spi->tx;
			int ty = spi->ty;
			spi->tx += spi->dx;
			spi->ty += spi->dy;
			if (tx != spi->tx || ty != spi->ty) {
				spi->flags |= kSFChanged | kSFNeedRedraw;
			}			
		}
		if (spi->flags & kSFDelayed) {
			if (spi->delayAmount) {
				--spi->delayCount;
				if (spi->delayCount) 
					continue;

				spi->delayCount = spi->delayAmount;
			}
			int state = spi->res_state;
			++spi->res_state;
			if (spi->res_state >= spi->res_wiz_states) {
				spi->res_state = 0;
				if (state == 0)
					continue;
			}
			spi->flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

static int compareSprTable(const void *a, const void *b) {
	const SpriteInfo *spr1 = *(const SpriteInfo *const*)a;
	const SpriteInfo *spr2 = *(const SpriteInfo *const*)b;

	if (spr1->zorder > spr2->zorder)
		return 1;

	if (spr1->zorder < spr2->zorder)
		return -1;

	return 0;
}

void ScummEngine_v90he::spritesSortActiveSprites() {
	int groupZorder;

	_numSpritesToProcess = 0;

	if (_varNumSprites <= 1)
		return;

	for (int i = 1; i < _varNumSprites; i++) {
		SpriteInfo *spi = &_spriteTable[i];

		if (spi->flags & kSFActive) {
			if (!(spi->flags & kSF23)) {
				if (!(spi->flags & kSF30))
					spi->flags |= kSFNeedRedraw;
				if (!(spi->flags & kSFImageless))
					spi->flags |= kSFChanged;
			}
			if (spi->group_num)
				groupZorder = _spriteGroups[spi->group_num].zorderPriority;
			else
				groupZorder = 0;

			spi->id = i;
			spi->zorder = spi->zorderPriority + groupZorder;

			_activeSpritesTable[_numSpritesToProcess++] = spi;
		}
	}

	if (_numSpritesToProcess < 2)
		return;

	qsort(_activeSpritesTable, _numSpritesToProcess, sizeof(SpriteInfo *), compareSprTable);
}

void ScummEngine_v90he::spritesProcessWiz(bool arg) {
	int spr_flags;
	int16 spr_wiz_x, spr_wiz_y;
	int res_id, res_state;
	Common::Rect *bboxPtr;
	int rot_angle, zoom;
	int32 w, h;
	WizParameters wiz;

	for (int i = 0; i < _numSpritesToProcess; i++) {
		SpriteInfo *spi = _activeSpritesTable[i];

		// HACK Sprite redraws aren't always been triggered
		// so leave disabled for now.
		//if (!(spi->flags & kSFNeedRedraw))
		//	continue;

		spr_flags = spi->flags;

		if (arg) {
			if (spi->zorder >= 0)
				return;
		} else {
			if (spi->zorder < 0)
				continue;
		}
		
		spi->flags &= ~kSFNeedRedraw;
		res_id = spi->res_id;
		res_state = spi->res_state;
		loadImgSpot(spi->res_id, spi->res_state, spr_wiz_x, spr_wiz_y);

		if (spi->group_num) {
			SpriteGroup *spg = &_spriteGroups[spi->group_num];

			if (spg->scaling) {
				wiz.img.x1 = spi->tx * spg->scale_x - spr_wiz_x + spg->tx;
				wiz.img.y1 = spi->ty * spg->scale_y - spr_wiz_y + spg->ty;
			} else {
				wiz.img.x1 = spi->tx - spr_wiz_x + spg->tx;
				wiz.img.y1 = spi->ty - spr_wiz_y + spg->ty;
			}
		} else {
			wiz.img.x1 = spi->tx - spr_wiz_x;
			wiz.img.y1 = spi->ty - spr_wiz_y;
		}

		spi->cur_img_state = wiz.img.state = res_state;
		spi->cur_res_id = wiz.img.resNum = res_id;
		wiz.processFlags = kWPFNewState | kWPFSetPos;
		spi->cur_rot_angle = spi->rot_angle;
		spi->cur_zoom = spi->zoom;
		spi->pos.x = wiz.img.x1;
		spi->pos.y = wiz.img.y1;
		bboxPtr = &spi->bbox;
		if (res_id) {
			rot_angle = spi->rot_angle;
			zoom = spi->zoom;
			getWizImageDim(res_id, res_state, w, h);
			if (!(spi->flags & (kSFZoomed | kSFRotated))) {
				bboxPtr->left = wiz.img.x1;
				bboxPtr->top = wiz.img.y1;
				bboxPtr->right = wiz.img.x1 + w;
				bboxPtr->bottom = wiz.img.y1 + h;
			} else {
				Common::Point pts[4];

				pts[1].x = pts[2].x = w / 2 - 1;
				pts[0].x = pts[0].y = pts[1].y = pts[3].x = -w / 2;
				pts[2].y = pts[3].y = h / 2 - 1;

				if ((spi->flags & kSFZoomed) && zoom) {
					for (int j = 0; j < 4; ++j) {
						pts[j].x = pts[j].x * zoom / 256;
						pts[j].y = pts[j].y * zoom / 256;
					}
				}
				if ((spi->flags & kSFRotated) && rot_angle)
					_wiz.polygonRotatePoints(pts, 4, rot_angle);

				for (int j = 0; j < 4; ++j) {
					pts[j].x += wiz.img.x1;
					pts[j].y += wiz.img.y1;
				}

				_wiz.polygonCalcBoundBox(pts, 4, spi->bbox);
			}
		} else {
			bboxPtr->left = 1234;
			bboxPtr->top = 1234;
			bboxPtr->right = -1234;
			bboxPtr->bottom = -1234;
		}

		wiz.img.flags = kWIFMarkBufferDirty;
		if (spr_flags & kSFXFlipped)
			wiz.img.flags |= kWIFFlipX;
		if (spr_flags & kSFYFlipped)
			wiz.img.flags |= kWIFFlipY;
		if (spr_flags & kSFDoubleBuffered) {
			wiz.img.flags &= ~kWIFMarkBufferDirty;
			wiz.img.flags |= kWIFBlitToFrontVideoBuffer;
		}
		if (spi->field_54) {
			wiz.img.flags |= 0x200;
			wiz.processFlags |= 4;
			wiz.unk_15C = spi->field_54;
		}
		if (spr_flags & kSFNeedPaletteRemap)
			wiz.img.flags |= kWIFRemapPalette;
		if (spi->field_7C) {
			wiz.processFlags |= 0x80000;
			wiz.unk_178 = spi->field_7C;
		}
		wiz.processFlags |= kWPFNewFlags;
		
		if (spr_flags & kSFRotated) {
			wiz.processFlags |= kWPFRotate;
			wiz.angle = spi->rot_angle;
		}
		if (spr_flags & kSFZoomed) {
			wiz.processFlags |= kWPFZoom;
			wiz.zoom = spi->zoom;
		}
		spi->imgFlags = wiz.img.flags;
		
		if (spi->group_num && (_spriteGroups[spi->group_num].flags & kSGFClipBox)) {
			if (spi->bbox.intersects(_spriteGroups[spi->group_num].bbox)) {
				spi->bbox.clip(_spriteGroups[spi->group_num].bbox);
				wiz.processFlags |= kWPFClipBox;
				wiz.box = spi->bbox;
			} else {
				bboxPtr->left = 1234;
				bboxPtr->top = 1234;
				bboxPtr->right = -1234;
				bboxPtr->bottom = -1234;
				continue;
			}
		}
		if (spi->field_14) {
			wiz.processFlags |= 0x8000;
			wiz.unk_174 = spi->field_14;
		}
		if (spi->res_id && spi->group_num && _spriteGroups[spi->group_num].field_20) {
			wiz.processFlags |= 0x10000;
			wiz.unk_380 = _spriteGroups[spi->group_num].field_20;
		}
		displayWizComplexImage(&wiz);
	}
}

} // End of namespace Scumm
