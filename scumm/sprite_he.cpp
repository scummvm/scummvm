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
#include "scumm/saveload.h"
#include "scumm/scumm.h"
#include "scumm/sprite_he.h"
#include "scumm/usage_bits.h"
#include "scumm/util.h"
#include "scumm/wiz_he.h"

namespace Scumm {

void ScummEngine_v90he::allocateArrays() {
	ScummEngine::allocateArrays();
	spritesAllocTables(_numSprites, MAX(64, _numSprites / 4), 64);
}

void ScummEngine_v90he::getSpriteBounds(int spriteId, bool checkGroup, Common::Rect &bound) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	int16 spr_wiz_x, spr_wiz_y;
	int angle, zoom, x1, y1;
	int32 w, h;

	SpriteInfo *spi = &_spriteTable[spriteId];

	loadImgSpot(spi->resId, spi->resState, spr_wiz_x, spr_wiz_y);
	if (checkGroup && spi->groupNum) {
		SpriteGroup *spg = &_spriteGroups[spi->groupNum];

		if (spg->scaling) {
			x1 = spi->tx * spg->scale_x_ratio_mul / spg->scale_x_ratio_div - spr_wiz_x + spg->tx;
			y1 = spi->ty * spg->scale_y_ratio_mul / spg->scale_y_ratio_div - spr_wiz_y + spg->ty;
		} else {
			x1 = spi->tx - spr_wiz_x + spg->tx;
			y1 = spi->ty - spr_wiz_y + spg->ty;
		}
	} else {
		x1 = spi->tx - spr_wiz_x;
		y1 = spi->ty - spr_wiz_y;
	}

	if (spi->resId) {
		angle = spi->angle;
		zoom = spi->zoom;
		getWizImageDim(spi->resId, spi->resState, w, h);
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
			if ((spi->flags & kSFRotated) && angle)
				_wiz.polygonRotatePoints(pts, 4, angle);

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
		if (!spi->curResId)
			continue;

		if (spriteGroupId && spi->groupNum != spriteGroupId)
			continue;

		cond = true;
		for (int j = 0; j < num; j++) {
			code = classId = args[j];
			classId &= 0x7F;
			checkRange(32, 1, classId, "class %d out of range in statement");
			if (code & 0x80) {
				if (!(spi->classFlags & (1 << (classId - 1))))
					cond = 0;
			} else {
				if ((spi->classFlags & (1 << (classId - 1))))
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
			int resId, resState, angle, zoom;
			int32 w, h;

			resId = spi->curResId;
			if (spi->field_80) {
				int16 x1, x2, y1, y2;

				resState = spi->curImageState / getWizImageStates(spi->field_80);

				x = x_pos - spi->pos.x;
				y = y_pos - spi->pos.y;

				loadImgSpot(spi->curResId, resState, x1, y1);
				loadImgSpot(spi->field_80, resState, x2, y2);

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
				resState = spi->curImageState;
			}

			angle = spi->curAngle;
			zoom = spi->curZoom;
			if ((spi->flags & kSFZoomed) || (spi->flags & kSFRotated)) {
				if (spi->flags & kSFZoomed && zoom) {
					x = x * 256 / zoom;
					y = y * 256 / zoom;
				}
				if (spi->flags & kSFRotated && angle) {
					angle = (360 - angle) % 360;
					Common::Point pts[1];
					_wiz.polygonRotatePoints(pts, 1, angle);
				}

				getWizImageDim(resId, resState, w, h);
				x += w / 2;
				y += h / 2;
			}

			if (isWizPixelNonTransparent(resId, resState, x, y, spi->curImgFlags))
				return spi->id;
		}
	}

	return 0;
}

int ScummEngine_v90he::spriteInfoGet_classFlags(int spriteId, int num, int *args) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	int code, classId;

	if (num == 0)
		return _spriteTable[spriteId].classFlags; 

	for (int i = 0; i < num; i++) {
		code = classId = args[i];
		classId &= 0x7F;
		checkRange(32, 1, classId, "class %d out of range in statement");
		if (code & 0x80) {
			if (!(_spriteTable[spriteId].classFlags & (1 << (classId - 1))))
				return 0;
		} else {
			if ((_spriteTable[spriteId].classFlags & (1 << (classId - 1))))
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

int ScummEngine_v90he::spriteInfoGet_flagDelayed(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFDelayed) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flagMarkDirty(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFMarkDirty) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flagHasImage(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSFImageless) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_resId(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].resId;
}

int ScummEngine_v90he::spriteInfoGet_resState(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].resState;
}

int ScummEngine_v90he::spriteInfoGet_groupNum(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].groupNum;
}

int ScummEngine_v90he::spriteInfoGet_paletteNum(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].paletteNum;
}

int ScummEngine_v90he::spriteInfoGet_zorderPriority(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].zorderPriority;
}

int ScummEngine_v90he::spriteInfoGet_grp_tx(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].groupNum)
		return _spriteTable[spriteId].tx + _spriteGroups[_spriteTable[spriteId].groupNum].tx;
	else
		return _spriteTable[spriteId].tx;
}

int ScummEngine_v90he::spriteInfoGet_grp_ty(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].groupNum)
		return _spriteTable[spriteId].ty + _spriteGroups[_spriteTable[spriteId].groupNum].ty;
	else
		return _spriteTable[spriteId].ty;
}

int ScummEngine_v90he::spriteInfoGet_field_44(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_44;
}

int ScummEngine_v90he::spriteInfoGet_xmapNum(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].xmapNum;
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

int ScummEngine_v90he::spriteInfoGet_maskImgResNum(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].maskImgResNum;
}

int ScummEngine_v90he::spriteInfoGet_field_80(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_80;
}

int ScummEngine_v90he::spriteInfoGet_field_8C_90(int spriteId, int type) {
	debug(0, "spriteInfoGet_field_8C_90: spriteId %d type 0x%x", spriteId, type);
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	// XXX U32 related check

	switch(type) {
	case 0x7B:
		return _spriteTable[spriteId].imgFlags;
	case 0x7D:
		return _spriteTable[spriteId].field_90;
	case 0x7E:
		return _spriteTable[spriteId].delayCount;
	default:
		error("spriteInfoGet_field_8C_90: Invalid type %d", type);
	}
}

void ScummEngine_v90he::getSpriteImageDim(int spriteId, int32 &w, int32 &h) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].resId) {
		getWizImageDim(_spriteTable[spriteId].resId, _spriteTable[spriteId].resState, w, h);
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
	int i, numSprites = 0;

	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (i = (_varNumSprites - 1); i > 0; i--) {
		if (_spriteTable[i].groupNum == spriteGroupId)
			numSprites++;
	}

	if (!numSprites)
		return 0;

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, numSprites);
	writeArray(0, 0, 0, numSprites);
	
	numSprites = 1;
	for (i = (_varNumSprites - 1); i > 0; i--) {
		if (_spriteTable[i].groupNum == spriteGroupId) {
			writeArray(0, 0, numSprites, i);
			numSprites++;
		}
	}
	
	return readVar(0);
}

int ScummEngine_v90he::spriteGroupGet_zorderPriority(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].zorderPriority;
}

int ScummEngine_v90he::spriteGroupGet_dstResNum(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].dstResNum;
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
void ScummEngine_v90he::spriteInfoSet_paletteNum(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].paletteNum != value) {
		_spriteTable[spriteId].paletteNum = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_maskImgResNum(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].maskImgResNum != value) {
		_spriteTable[spriteId].maskImgResNum = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_field_80(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].field_80 = value;
}

void ScummEngine_v90he::spriteInfoSet_resState(int spriteId, int state) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].resId) {
		int res_wiz_states = _spriteTable[spriteId].res_wiz_states - 1;
		state = MAX(0, state);
		state = MIN(state, res_wiz_states);
	
		if (_spriteTable[spriteId].resState != state) {
			_spriteTable[spriteId].resState = state;
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

	_spriteTable[spriteId].groupNum = value;
	_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_dx_dy(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].dx = value1;
	_spriteTable[spriteId].dy = value2;
}

void ScummEngine_v90he::spriteInfoSet_xmapNum(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].xmapNum = value;
	if (_spriteTable[spriteId].resId)
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

		if (_spriteTable[spriteId].resId)
			_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_angle(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].flags |= kSFRotated;

	if (_spriteTable[spriteId].angle != value) {
		_spriteTable[spriteId].angle = value;

		if (_spriteTable[spriteId].resId)
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

	if (_spriteTable[spriteId].resId && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flagYFlipped(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFYFlipped;
	else
		_spriteTable[spriteId].flags &= ~kSFYFlipped;

	if (_spriteTable[spriteId].resId && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flagXFlipped(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFXFlipped;
	else
		_spriteTable[spriteId].flags &= ~kSFXFlipped;

	if (_spriteTable[spriteId].resId && _spriteTable[spriteId].flags != oldFlags)
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

	if (_spriteTable[spriteId].resId && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flagDelayed(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSFDelayed;
	else
		_spriteTable[spriteId].flags &= ~kSFDelayed;
}

void ScummEngine_v90he::spriteInfoSet_flagMarkDirty(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	switch(value) {
	case 2:
		_spriteTable[spriteId].flags &= ~(kSFMarkDirty);
		_spriteTable[spriteId].flags |= kSFBlitDirectly;
		break;
	case 1:
		_spriteTable[spriteId].flags |= kSFMarkDirty | kSFBlitDirectly;
		break;
	case 0:
		_spriteTable[spriteId].flags &= ~(kSFMarkDirty | kSFBlitDirectly);
		break;
	default:
		error("spriteInfoSet_flagMarkDirty: Invalid value %d", value);
	}
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

void ScummEngine_v90he::spriteInfoSet_setClassFlag(int spriteId, int classId, int toggle) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	checkRange(32, 1, classId, "class %d out of range in statement");
	
	if (toggle) {
		_spriteTable[spriteId].classFlags |= (1 << (classId - 1));
	} else {
		_spriteTable[spriteId].classFlags &= ~(1 << (classId - 1));
	}
}

void ScummEngine_v90he::spriteInfoSet_resetClassFlags(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].classFlags = 0;
}

void ScummEngine_v90he::spriteInfoSet_field_84(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].field_84 = value;
}

void ScummEngine_v90he::spriteInfoSet_field_8C_90(int spriteId, int type, int value) {
	debug(0, "spriteInfoSet_field_8C_90: spriteId %d type 0x%x", spriteId, type);
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	int delay;

	// XXX U32 related check

	switch(type) {
	case 0x7B:
		_spriteTable[spriteId].imgFlags = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
		break;
	case 0x7D:
		_spriteTable[spriteId].field_90 = value;
		_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
		break;
	case 0x7E:
		delay = MAX(0, value);
		delay = MIN((int32)value, _spriteTable[spriteId].delayAmount);

		_spriteTable[spriteId].delayCount = value;
		break;
	default:
		error("spriteInfoSet_field_8C_90: Invalid value %d", type);
	}
}

void ScummEngine_v90he::spriteInfoSet_resetSprite(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].angle = 0;
	_spriteTable[spriteId].zoom = 0;

	int tmp = 0;
	spriteAddImageToList(spriteId, 1, &tmp);

	_spriteTable[spriteId].xmapNum = 0;
	_spriteTable[spriteId].tx = 0;
	_spriteTable[spriteId].ty = 0;

	_spriteTable[spriteId].flags &= ~(kSFYFlipped | kSFXFlipped);
	_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	_spriteTable[spriteId].dx = 0;
	_spriteTable[spriteId].dy = 0;
	_spriteTable[spriteId].field_44 = 0;
	_spriteTable[spriteId].groupNum = 0;
	_spriteTable[spriteId].delayAmount = 0;
	_spriteTable[spriteId].delayCount = 0;
	_spriteTable[spriteId].classFlags = 0;
	_spriteTable[spriteId].paletteNum = 0;
	_spriteTable[spriteId].maskImgResNum = 0;
	_spriteTable[spriteId].field_80 = 0;
	_spriteTable[spriteId].zorderPriority = 0;
	_spriteTable[spriteId].field_84 = 0;
	_spriteTable[spriteId].imgFlags = 0;
	_spriteTable[spriteId].field_90 = 0;
}

void ScummEngine_v90he::spriteAddImageToList(int spriteId, int imageNum, int *spriteIdptr) {
	int origResId, origResWizStates;

	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	origResId = _spriteTable[spriteId].resId;
	origResWizStates = _spriteTable[spriteId].res_wiz_states;

	_spriteTable[spriteId].resId = *spriteIdptr;
	_spriteTable[spriteId].field_74 = 0;
	_spriteTable[spriteId].resState = 0;

	if (_spriteTable[spriteId].resId) {
		_spriteTable[spriteId].res_wiz_states = getWizImageStates(_spriteTable[spriteId].resId);
		_spriteTable[spriteId].flags |= kSFActive | kSFDelayed | kSFMarkDirty | kSFBlitDirectly;

		if (_spriteTable[spriteId].resId != origResId || _spriteTable[spriteId].res_wiz_states != origResWizStates)
			_spriteTable[spriteId].flags |= kSFChanged | kSFNeedRedraw;
	} else {
		if (_spriteTable[spriteId].flags & kSFImageless)
			_spriteTable[spriteId].flags = 0;
		else
			_spriteTable[spriteId].flags = kSFChanged | kSFBlitDirectly;
		_spriteTable[spriteId].curResId = 0;
		_spriteTable[spriteId].curImageState = 0;
		_spriteTable[spriteId].res_wiz_states = 0;
	}
}

//
// spriteGroupSet functions
//
void ScummEngine_v90he::redrawSpriteGroup(int spriteGroupId) {
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (spi->groupNum == spriteGroupId) {
			spi->flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_0(int spriteGroupId, int value1, int value2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].groupNum == spriteGroupId) {
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
		if (_spriteTable[i].groupNum == spriteGroupId)
			_spriteTable[i].zorderPriority = value;
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_2(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].groupNum == spriteGroupId) {
			_spriteTable[i].groupNum = value;
			_spriteTable[i].flags |= kSFChanged | kSFNeedRedraw;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_3(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].groupNum == spriteGroupId)
			spriteInfoSet_flagMarkDirty(i, value);
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_4(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].groupNum == spriteGroupId)
			spriteInfoSet_resetSprite(i);
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_5(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].groupNum == spriteGroupId) {
			_spriteTable[i].delayAmount = value;
			_spriteTable[i].delayCount = value;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_6(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].groupNum == spriteGroupId) {
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
		if (_spriteTable[i].groupNum == spriteGroupId) {
			_spriteTable[i].xmapNum = value;
			if (_spriteTable[i].resId)
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

void ScummEngine_v90he::spriteGroupSet_dstResNum(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].dstResNum != value) {
		_spriteGroups[spriteGroupId].dstResNum = value;
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
		spriteGroupSet_scaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void ScummEngine_v90he::spriteGroupSet_scale_y_ratio_mul(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].scale_y_ratio_mul != value) {
		_spriteGroups[spriteGroupId].scale_y_ratio_mul = value;
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

	spg->dstResNum = 0;
	spg->scaling = 0;
	spg->scale_x_ratio_mul = 1;
	spg->scale_x_ratio_div = 1;
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
					gdi.copyVirtScreenBuffers(spi->bbox, USAGE_BIT_RESTORED);
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
				if (!(spi->flags & (kSFNeedRedraw | kSF30)) && spi->resId)
					spi->flags |= kSFNeedRedraw;
			}
		}
	}
	if (refreshScreen) {
		gdi.copyVirtScreenBuffers(Common::Rect(xmin, ymin, xmax, ymax), USAGE_BIT_RESTORED);
	}
}

void ScummEngine_v90he::spritesMarkDirty(bool checkZOrder) {
	VirtScreen *vs = &virtscr[kMainVirtScreen];
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!(spi->flags & (kSFNeedRedraw | kSF30))) {
			if ((!checkZOrder || spi->zorderPriority >= 0) && (spi->flags & kSFMarkDirty)) {
				int lp = spi->bbox.left / 8;
				if (lp < 0) {
					lp = 0;
				} else if (lp > 79) {
					lp = 79;
				}
				int rp = (spi->bbox.right + 7) / 8;
				if (rp < 0) {
					rp = 0;
				} else {
					rp = 79;
				}
				for (; lp <= rp; ++lp) {
					if (vs->tdirty[lp] < vs->h && spi->bbox.bottom >= vs->tdirty[lp] && spi->bbox.top <= vs->bdirty[lp]) {
						spi->flags |= kSFNeedRedraw;
						break;
					}
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
			int resState = spi->resState;
			++spi->resState;
			if (spi->resState >= spi->res_wiz_states) {
				spi->resState = 0;
				if (resState == 0)
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
			if (!(spi->flags & kSFMarkDirty)) {
				if (!(spi->flags & kSF30))
					spi->flags |= kSFNeedRedraw;
				if (!(spi->flags & kSFImageless))
					spi->flags |= kSFChanged;
			}
			if (spi->groupNum)
				groupZorder = _spriteGroups[spi->groupNum].zorderPriority;
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
	int resId, resState;
	Common::Rect *bboxPtr;
	int angle, zoom;
	int32 w, h;
	WizParameters wiz;

	for (int i = 0; i < _numSpritesToProcess; i++) {
		SpriteInfo *spi = _activeSpritesTable[i];

		if (!(spi->flags & kSFNeedRedraw))
			continue;

		spr_flags = spi->flags;

		if (arg) {
			if (spi->zorder >= 0)
				return;
		} else {
			if (spi->zorder < 0)
				continue;
		}
		
		spi->flags &= ~kSFNeedRedraw;
		resId = spi->resId;
		resState = spi->resState;
		loadImgSpot(spi->resId, spi->resState, spr_wiz_x, spr_wiz_y);

		if (spi->groupNum) {
			SpriteGroup *spg = &_spriteGroups[spi->groupNum];

			if (spg->scaling) {
				wiz.img.x1 = spi->tx * spg->scale_x_ratio_mul / spg->scale_x_ratio_div - spr_wiz_x + spg->tx;
				wiz.img.y1 = spi->ty * spg->scale_y_ratio_mul / spg->scale_y_ratio_div - spr_wiz_y + spg->ty;
			} else {
				wiz.img.x1 = spi->tx - spr_wiz_x + spg->tx;
				wiz.img.y1 = spi->ty - spr_wiz_y + spg->ty;
			}
		} else {
			wiz.img.x1 = spi->tx - spr_wiz_x;
			wiz.img.y1 = spi->ty - spr_wiz_y;
		}

		wiz.spriteId = spi->id;
		wiz.groupNum = spi->groupNum;
		wiz.field_23EA = spi->field_90;
		spi->curImageState = wiz.img.state = resState;
		spi->curResId = wiz.img.resNum = resId;
		wiz.processFlags = kWPFNewState | kWPFSetPos;
		spi->curAngle = spi->angle;
		spi->curZoom = spi->zoom;
		spi->pos.x = wiz.img.x1;
		spi->pos.y = wiz.img.y1;
		bboxPtr = &spi->bbox;
		if (resId) {
			angle = spi->angle;
			zoom = spi->zoom;
			getWizImageDim(resId, resState, w, h);
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
				if ((spi->flags & kSFRotated) && angle)
					_wiz.polygonRotatePoints(pts, 4, angle);

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
		if (spi->xmapNum) {
			wiz.img.flags |= 0x200;
			wiz.processFlags |= kWPFXmapNum;
			wiz.xmapNum = spi->xmapNum;
		}
		if (spr_flags & kSFNeedPaletteRemap)
			wiz.img.flags |= kWIFRemapPalette;
		if (spi->field_84) {
			wiz.processFlags |= 0x200000;
			wiz.img.field_390 = spi->field_84;
			wiz.img.zorder = spi->zorderPriority;
		}
		if (spi->maskImgResNum) {
			wiz.processFlags |= kWPFMaskImg;
			wiz.maskImgResNum = spi->maskImgResNum;
		}
		wiz.processFlags |= kWPFNewFlags;
		wiz.img.flags |= spi->imgFlags;
		
		if (spr_flags & kSFRotated) {
			wiz.processFlags |= kWPFRotate;
			wiz.angle = spi->angle;
		}
		if (spr_flags & kSFZoomed) {
			wiz.processFlags |= kWPFZoom;
			wiz.zoom = spi->zoom;
		}
		spi->curImgFlags = wiz.img.flags;
		
		if (spi->groupNum && (_spriteGroups[spi->groupNum].flags & kSGFClipBox)) {
			if (spi->bbox.intersects(_spriteGroups[spi->groupNum].bbox)) {
				spi->bbox.clip(_spriteGroups[spi->groupNum].bbox);
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
		if (spi->paletteNum) {
			wiz.processFlags |= kWPFPaletteNum;
			wiz.img.paletteNum = spi->paletteNum;
		}
		if (spi->resId && spi->groupNum && _spriteGroups[spi->groupNum].dstResNum) {
			wiz.processFlags |= kWPFDstResNum;
			wiz.dstResNum = _spriteGroups[spi->groupNum].dstResNum;
		}
		displayWizComplexImage(&wiz);
	}
}

void ScummEngine_v90he::saveOrLoadSpriteData(Serializer *s, uint32 savegameVersion) {
	static const SaveLoadEntry spriteEntries[] = {
		MKLINE(SpriteInfo, id, sleInt32, VER(48)),
		MKLINE(SpriteInfo, zorder, sleInt32, VER(48)),
		MKLINE(SpriteInfo, flags, sleInt32, VER(48)),
		MKLINE(SpriteInfo, resId, sleInt32, VER(48)),
		MKLINE(SpriteInfo, resState, sleInt32, VER(48)),
		MKLINE(SpriteInfo, groupNum, sleInt32, VER(48)),
		MKLINE(SpriteInfo, paletteNum, sleInt32, VER(48)),
		MKLINE(SpriteInfo, zorderPriority, sleInt32, VER(48)),
		MKLINE(SpriteInfo, bbox.left, sleInt32, VER(48)),
		MKLINE(SpriteInfo, bbox.top, sleInt32, VER(48)),
		MKLINE(SpriteInfo, bbox.right, sleInt32, VER(48)),
		MKLINE(SpriteInfo, bbox.bottom, sleInt32, VER(48)),
		MKLINE(SpriteInfo, dx, sleInt32, VER(48)),
		MKLINE(SpriteInfo, dy, sleInt32, VER(48)),
		MKLINE(SpriteInfo, pos.x, sleInt32, VER(48)),
		MKLINE(SpriteInfo, pos.y, sleInt32, VER(48)),
		MKLINE(SpriteInfo, tx, sleInt32, VER(48)),
		MKLINE(SpriteInfo, ty, sleInt32, VER(48)),
		MKLINE(SpriteInfo, field_44, sleInt32, VER(48)),
		MKLINE(SpriteInfo, curImageState, sleInt32, VER(48)),
		MKLINE(SpriteInfo, curResId, sleInt32, VER(48)),
		MKLINE(SpriteInfo, imglistNum, sleInt32, VER(48)),
		MKLINE(SpriteInfo, xmapNum, sleInt32, VER(48)),
		MKLINE(SpriteInfo, res_wiz_states, sleInt32, VER(48)),
		MKLINE(SpriteInfo, angle, sleInt32, VER(48)),
		MKLINE(SpriteInfo, zoom, sleInt32, VER(48)),
		MKLINE(SpriteInfo, delayCount, sleInt32, VER(48)),
		MKLINE(SpriteInfo, curAngle, sleInt32, VER(48)),
		MKLINE(SpriteInfo, curZoom, sleInt32, VER(48)),
		MKLINE(SpriteInfo, curImgFlags, sleInt32, VER(48)),
		MKLINE(SpriteInfo, field_74, sleInt32, VER(48)),
		MKLINE(SpriteInfo, delayAmount, sleInt32, VER(48)),
		MKLINE(SpriteInfo, maskImgResNum, sleInt32, VER(48)),
		MKLINE(SpriteInfo, field_80, sleInt32, VER(48)),
		MKLINE(SpriteInfo, field_84, sleInt32, VER(48)),
		MKLINE(SpriteInfo, classFlags, sleInt32, VER(48)),
		MKLINE(SpriteInfo, imgFlags, sleInt32, VER(48)),
		MKLINE(SpriteInfo, field_90, sleInt32, VER(48)),
		MKEND()
	};
	
	static const SaveLoadEntry spriteGroupEntries[] = {
		MKLINE(SpriteGroup, bbox.left, sleInt32, VER(48)),
		MKLINE(SpriteGroup, bbox.top, sleInt32, VER(48)),
		MKLINE(SpriteGroup, bbox.right, sleInt32, VER(48)),
		MKLINE(SpriteGroup, bbox.bottom, sleInt32, VER(48)),
		MKLINE(SpriteGroup, zorderPriority, sleInt32, VER(48)),
		MKLINE(SpriteGroup, flags, sleInt32, VER(48)),
		MKLINE(SpriteGroup, tx, sleInt32, VER(48)),
		MKLINE(SpriteGroup, ty, sleInt32, VER(48)),
		MKLINE(SpriteGroup, dstResNum, sleInt32, VER(48)),
		MKLINE(SpriteGroup, scaling, sleInt32, VER(48)),
		MKLINE(SpriteGroup, scale_x_ratio_mul, sleInt32, VER(48)),
		MKLINE(SpriteGroup, scale_x_ratio_div, sleInt32, VER(48)),
		MKLINE(SpriteGroup, scale_y_ratio_mul, sleInt32, VER(48)),
		MKLINE(SpriteGroup, scale_y_ratio_div, sleInt32, VER(48)),
		MKEND()
	};

	s->saveLoadArrayOf(_activeSpritesTable, _varNumSprites, sizeof(_activeSpritesTable[0]), spriteEntries);
	s->saveLoadArrayOf(_spriteTable, _varNumSprites, sizeof(_spriteTable[0]), spriteEntries);
	s->saveLoadArrayOf(_spriteGroups, _varNumSpriteGroups, sizeof(_spriteGroups[0]), spriteGroupEntries);
}

} // End of namespace Scumm
