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
	// FIXME Check real _varNumSpriteGroups value
	spritesAllocTables(_numSprites, 640, 64);
}

//
// spriteInfoGet functions
//
int ScummEngine_v90he::spriteInfoGet_case15(int a, int b, int c, int d, int num, int *args) {
	// TODO
	return 0;
}

int ScummEngine_v90he::spriteInfoGet_classFlags(int spriteId, int num) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	// TODO
	return 0;
}

int ScummEngine_v90he::spriteInfoGet_classFlags2(int spriteId, int num, int *args) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	// TODO
	return 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_1(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSF15) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_2(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSF14) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_3(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSF16) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_4(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSF13) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_5(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSF20) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_6(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].flags & kSF22;
}

int ScummEngine_v90he::spriteInfoGet_flags_7(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSF23) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_8(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return ((_spriteTable[spriteId].flags & kSF31) != 0) ? 1 : 0;
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

int ScummEngine_v90he::spriteInfoGet_field_18(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_18;
}

int ScummEngine_v90he::spriteInfoGet_grp_tx(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].group_num)
		return _spriteTable[spriteId].tx + _spriteGroups[spriteId].tx;
	else
		return _spriteTable[spriteId].tx;
}

int ScummEngine_v90he::spriteInfoGet_grp_ty(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].group_num)
		return _spriteTable[spriteId].ty + _spriteGroups[spriteId].ty;
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

int ScummEngine_v90he::spriteInfoGet_field_78(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_78;
}

int ScummEngine_v90he::spriteInfoGet_field_7C(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_7C;
}

int ScummEngine_v90he::spriteInfoGet_field_80(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	return _spriteTable[spriteId].field_80;
}

void ScummEngine_v90he::getSpriteImageDim(int spriteId, int32 &w, int32 &h) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	getWizImageDim(_spriteTable[spriteId].res_id, _spriteTable[spriteId].res_state, w, h);
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
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	// TODO
	return 0;
}

int ScummEngine_v90he::spriteGroupGet_field_10(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].field_10;
}

int ScummEngine_v90he::spriteGroupGet_field_20(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].field_20;
}

int ScummEngine_v90he::spriteGroupGet_field_30(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].field_30;
}

int ScummEngine_v90he::spriteGroupGet_field_34(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].field_34;
}

int ScummEngine_v90he::spriteGroupGet_field_38(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].field_38;
}

int ScummEngine_v90he::spriteGroupGet_field_3C(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	return _spriteGroups[spriteGroupId].field_3C;
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
		_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_field_7C(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].field_7C != value) {
		_spriteTable[spriteId].field_7C = value;
		_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_field_80(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].field_80 = value;
}

void ScummEngine_v90he::spriteInfoSet_resState(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].res_id) {
		int state;

		state = MAX(value, _spriteTable[spriteId].res_wiz_states - 1);
		if (state < 0)
			state = 0;
	
		if (_spriteTable[spriteId].res_state != state) {
			_spriteTable[spriteId].res_state = state;
			_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
		}
	}
}

void ScummEngine_v90he::spriteInfoSet_tx_ty(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].tx != value1 || _spriteTable[spriteId].ty != value2) {
		_spriteTable[spriteId].tx = value1;
		_spriteTable[spriteId].ty = value2;
		_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_groupNum(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	checkRange(_varNumSpriteGroups, 0, value, "Invalid sprite group %d");

	_spriteTable[spriteId].group_num = value;
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
		_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_field_44(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].field_44 = value2;
}

void ScummEngine_v90he::spriteInfoSet_field_18(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].field_18 = value;
}

void ScummEngine_v90he::spriteInfoSet_Inc_tx_ty(int spriteId, int value1, int value2) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].tx += value1;
	_spriteTable[spriteId].ty += value2;

	if  (_spriteTable[spriteId].tx || _spriteTable[spriteId].ty)
		_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_zoom(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].flags |= kSFZoomed;

	if (_spriteTable[spriteId].zoom != value) {
		_spriteTable[spriteId].zoom = value;

		if (_spriteTable[spriteId].res_id)
			_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_rotAngle(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].flags |= kSFRotated;

	if (_spriteTable[spriteId].rot_angle != value) {
		_spriteTable[spriteId].rot_angle = value;

		if (_spriteTable[spriteId].res_id)
			_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
	}
}

void ScummEngine_v90he::spriteInfoSet_flag22(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSF22;
	else
		_spriteTable[spriteId].flags &= ~(kSF01 | kSF22);
}

void ScummEngine_v90he::spriteInfoSet_flag7(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSF07;
	else
		_spriteTable[spriteId].flags &= ~(kSF01 | kSF07);

	if (_spriteTable[spriteId].res_id)
		_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flagRotated(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSFRotated;
	else
		_spriteTable[spriteId].flags &= ~(kSF01 | kSFRotated);

	if (_spriteTable[spriteId].res_id)
		_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flag8(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSF08;
	else
		_spriteTable[spriteId].flags &= ~(kSF01 | kSF08);
}

void ScummEngine_v90he::spriteInfoSet_flagZoomed(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSFZoomed;
	else
		_spriteTable[spriteId].flags &= ~(kSF01 | kSFZoomed);

	if (_spriteTable[spriteId].res_id)
		_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flag20(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSF20;
	else
		_spriteTable[spriteId].flags &= ~(kSF01 | kSF20);

	if (_spriteTable[spriteId].res_id)
		_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
}

void ScummEngine_v90he::spriteInfoSet_flags23_26(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (value)
		_spriteTable[spriteId].flags |= kSF23 | kSFBlitDirectly;
	else
		_spriteTable[spriteId].flags &= ~(kSF01 | kSF23 | kSFBlitDirectly);
}

void ScummEngine_v90he::spriteInfoSet_flag31(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	// Note that condition is inverted
	if (!value)
		_spriteTable[spriteId].flags |= kSF31;
	else
		_spriteTable[spriteId].flags &= ~(kSF01 | kSF31);
}

void ScummEngine_v90he::spriteInfoSet_field_78_64(int spriteId, int value) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].field_78 = value;
	_spriteTable[spriteId].field_64 = value;
}

void ScummEngine_v90he::spriteInfoSet_setClassFlags(int spriteId, int classId, int toggle) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");
	checkRange(32, 1, classId, "class %d out of range in statement");
	
	if (toggle) {
		_spriteTable[spriteId].flags |= 1 << (classId - 1);
	} else {
		_spriteTable[spriteId].flags &= ~(1 << (classId - 1));
	}
}

void ScummEngine_v90he::spriteInfoSet_resetClassFlags(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].class_flags = 0;
}

void ScummEngine_v90he::spriteInfoSet_resetSprite(int spriteId) {
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	_spriteTable[spriteId].rot_angle = 0;
	_spriteTable[spriteId].zoom = 0;

	spriteInfoSet_addImageToList(spriteId, 1, &spriteId);

	_spriteTable[spriteId].field_54 = 0;
	_spriteTable[spriteId].tx = 0;
	_spriteTable[spriteId].ty = 0;

	_spriteTable[spriteId].flags &= ~(kSF07 | kSFRotated);
	_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
	_spriteTable[spriteId].dx = 0;
	_spriteTable[spriteId].dy = 0;
	_spriteTable[spriteId].field_44 = 0;
	_spriteTable[spriteId].group_num = 0;
	_spriteTable[spriteId].field_78 = 0;
	_spriteTable[spriteId].field_64 = 0;
	_spriteTable[spriteId].class_flags = 0;
	_spriteTable[spriteId].field_14 = 0;
	_spriteTable[spriteId].field_7C = 0;
	_spriteTable[spriteId].field_80 = 0;
}

void ScummEngine_v90he::spriteInfoSet_addImageToList(int spriteId, int imageNum, int *spriteIdptr) {
	int listNum;
	int *ptr;
	int origResId;

	// XXX needs review
	checkRange(_varNumSprites, 1, spriteId, "Invalid sprite %d");

	if (_spriteTable[spriteId].imglist_num) {
		checkRange(_varMaxSprites, 1, _spriteTable[spriteId].imglist_num,
				   "Image list %d out of range");
		_imageListStack[_curSprImageListNum++] = _spriteTable[spriteId].imglist_num - 1;
		_spriteTable[spriteId].imglist_num = 0;
	}

	origResId = _spriteTable[spriteId].res_id;

	if (imageNum == 1)
		_spriteTable[spriteId].res_id = *spriteIdptr;
	else {
		if (!_curSprImageListNum)
			error("Out of image lists");

		if (imageNum > 32)
			error("Too many images in image list (%d)!", imageNum);

		_curSprImageListNum--;
		_spriteTable[spriteId].imglist_num = _imageListStack[_curSprImageListNum] + 1;

		listNum = _spriteTable[spriteId].imglist_num;

		checkRange(_varMaxSprites, 1, listNum, "Image list %d out of range");

		_imageListTable[0x21 * listNum - 1] = imageNum;
		
		ptr = spriteIdptr;
		for (int i = 0; i < listNum; i++) {
			_imageListTable[0x21 * listNum - 0x21 + i] = *ptr++;
		}
		_spriteTable[spriteId].res_id = *spriteIdptr;
	}

	_spriteTable[spriteId].field_74 = 0;
	_spriteTable[spriteId].res_state = 0;

	if (_spriteTable[spriteId].res_id) {
		_spriteTable[spriteId].res_wiz_states = getWizImageStates(_spriteTable[spriteId].res_id);
		_spriteTable[spriteId].flags |= kSF16 | kSF22 | kSF23 | kSFBlitDirectly;

		if (_spriteTable[spriteId].res_id == origResId &&
			_spriteTable[spriteId].res_wiz_states == spriteId)
			return;

		_spriteTable[spriteId].flags |= kSF01 | kSFNeedRedraw;
	} else {
		_spriteTable[spriteId].flags &= ~(kSF31);
		_spriteTable[spriteId].flags |= kSF01 | kSFBlitDirectly;
		_spriteTable[spriteId].field_4C = 0;
		_spriteTable[spriteId].field_48 = 0;
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
			spi->flags |= kSF01 | kSFNeedRedraw;
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
				_spriteTable[i].flags |= kSF01 | kSFNeedRedraw;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_1(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId)
			_spriteTable[i].field_18 = value;
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_2(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId) {
			_spriteTable[i].group_num = value;
			_spriteTable[i].flags |= kSF01 | kSFNeedRedraw;
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
				_spriteTable[i].flags &= ~(kSF01 | kSF23 | kSFBlitDirectly);
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
			_spriteTable[i].field_78 = value;
			_spriteTable[i].field_64 = value;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_6(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId) {
			if (value)
				_spriteTable[i].flags |= kSF22;
			else
				_spriteTable[i].flags &= ~(kSF01 | kSF22);
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case0_7(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group_num == spriteGroupId) {
			_spriteTable[i].field_54 = value;
			if (_spriteTable[i].res_id)
				_spriteTable[i].flags |= kSF01 | kSFNeedRedraw;
		}
	}
}

void ScummEngine_v90he::spriteGroupSet_case5_0(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].field_30 == value)
		return;

	_spriteGroups[spriteGroupId].field_30 = value;
	_spriteGroups[spriteGroupId].scale_x = _spriteGroups[spriteGroupId].field_30 / _spriteGroups[spriteGroupId].field_34;

	if ((_spriteGroups[spriteGroupId].field_30 != _spriteGroups[spriteGroupId].field_34) || (_spriteGroups[spriteGroupId].field_38 != _spriteGroups[spriteGroupId].field_3C))
		_spriteGroups[spriteGroupId].scaling = 1;
	else
		_spriteGroups[spriteGroupId].scaling = 0;

	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spriteGroupSet_case5_1(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].field_34 == value)
		return;

	_spriteGroups[spriteGroupId].field_34 = value;
	_spriteGroups[spriteGroupId].scale_x = _spriteGroups[spriteGroupId].field_30 / _spriteGroups[spriteGroupId].field_34;

	if ((_spriteGroups[spriteGroupId].field_30 != _spriteGroups[spriteGroupId].field_34) || (_spriteGroups[spriteGroupId].field_38 != _spriteGroups[spriteGroupId].field_3C))
		_spriteGroups[spriteGroupId].scaling = 1;
	else
		_spriteGroups[spriteGroupId].scaling = 0;

	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spriteGroupSet_case5_2(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].field_38 == value)
		return;

	_spriteGroups[spriteGroupId].field_38 = value;
	_spriteGroups[spriteGroupId].scale_y = _spriteGroups[spriteGroupId].field_38 / _spriteGroups[spriteGroupId].field_3C;

	if ((_spriteGroups[spriteGroupId].field_30 != _spriteGroups[spriteGroupId].field_34) || (_spriteGroups[spriteGroupId].field_38 != _spriteGroups[spriteGroupId].field_3C))
		_spriteGroups[spriteGroupId].scaling = 1;
	else
		_spriteGroups[spriteGroupId].scaling = 0;

	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spriteGroupSet_case5_3(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].field_3C == value)
		return;

	_spriteGroups[spriteGroupId].field_3C = value;
	_spriteGroups[spriteGroupId].scale_y = _spriteGroups[spriteGroupId].field_38 / _spriteGroups[spriteGroupId].field_3C;

	if ((_spriteGroups[spriteGroupId].field_30 != _spriteGroups[spriteGroupId].field_34) || (_spriteGroups[spriteGroupId].field_38 != _spriteGroups[spriteGroupId].field_3C))
		_spriteGroups[spriteGroupId].scaling = 1;
	else
		_spriteGroups[spriteGroupId].scaling = 0;

	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spriteGroupSet_field_10(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].field_10 == value)
		return;

	_spriteGroups[spriteGroupId].field_10 = value;

	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spriteGroupSet_tx_ty(int spriteGroupId, int value1, int value2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	_spriteGroups[spriteGroupId].tx += value1;
	_spriteGroups[spriteGroupId].ty += value2;

	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spriteGroupSet_case26(int spriteGroupId, int value) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].field_20 == value)
		return;

	_spriteGroups[spriteGroupId].field_20 = value;

	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spriteGroupSet_case28(int spriteGroupId, int value1, int value2) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	if (_spriteGroups[spriteGroupId].tx == value1 && _spriteGroups[spriteGroupId].ty == value2)
		return;

	_spriteGroups[spriteGroupId].tx = value1;
	_spriteGroups[spriteGroupId].ty = value2;

	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spriteGroupSet_fields_0_4_8_C(int spriteGroupId, int value1, int value2, int value3, int value4) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	_spriteGroups[spriteGroupId].flags |= kSGF01;
	_spriteGroups[spriteGroupId].field_0 = value1;
	_spriteGroups[spriteGroupId].field_4 = value2;
	_spriteGroups[spriteGroupId].field_8 = value3;
	_spriteGroups[spriteGroupId].field_C = value4;

	redrawSpriteGroup(spriteGroupId);
}

void ScummEngine_v90he::spriteGroupSet_case56(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");

	_spriteGroups[spriteGroupId].flags &= ~(kSGF02);

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
	_imageListTable = (uint16 *)malloc((_varMaxSprites + 1) * sizeof(uint16) * 2 + 1);
	_imageListStack = (uint16 *)malloc((_varMaxSprites + 1) * sizeof(uint16));
}

void ScummEngine_v90he::spritesResetGroup(int spriteGroupId) {
	int i;

	SpriteGroup *spg = &_spriteGroups[spriteGroupId];
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");
	if (spg->field_10 != 0) {
		spg->field_10 = 0;
		spriteGroupCheck(spriteGroupId);
		for (i = 0; i < _numSpritesToProcess; ++i) {
			SpriteInfo *spi = _activeSpritesTable[i];
			if (spi->group_num == spriteGroupId) {
				spi->flags |= kSF01 | kSFNeedRedraw;
			}
		}
	}
	if (spg->tx != 0 || spg->ty != 0) {
		spg->tx = spg->ty = 0;
		spriteGroupCheck(spriteGroupId);
		for (i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
			if (spi->group_num == spriteGroupId) {
				spi->flags |= kSF01 | kSFNeedRedraw;
			}
		}
	}
	spg->flags &= ~kSGF01;
	spriteMarkIfInGroup(spriteGroupId, kSF01 | kSFNeedRedraw);
	if (spg->field_20 != 0) {
		spriteGroupCheck(spriteGroupId);
		for (i = 0; i < _numSpritesToProcess; ++i) {
			SpriteInfo *spi = _activeSpritesTable[i];
			if (spi->group_num == spriteGroupId) {
				spi->flags |= kSF01 | kSFNeedRedraw;
			}
		}
	}
	spriteGroupCheck(spriteGroupId);
	spg->scaling = 0;
	spg->scale_x = 0x3F800000;
	spg->field_30 = 0;
	spg->field_34 = 0;
	spg->scale_y = 0x3F800000;
	spg->field_38 = 0;
	spg->field_3C = 0;
}

void ScummEngine_v90he::spritesResetTables(bool refreshScreen) {
	int i;

	for (i = 0; i < _varMaxSprites; ++i) {
		_imageListStack[i] = i;
	}
	memset(_spriteTable, 0, (_varNumSprites + 1) * sizeof(SpriteInfo));
	memset(_spriteGroups, 0, (_varNumSpriteGroups + 1) * sizeof(SpriteGroup));
	for (int curGrp = 1; curGrp < _varNumSpriteGroups; ++curGrp)
		spritesResetGroup(curGrp);

	if (refreshScreen) {
		gdi.copyVirtScreenBuffers(Common::Rect(_screenWidth, _screenHeight));
	}
	_numSpritesToProcess = 0;
}

void ScummEngine_v90he::spriteGroupCheck(int spriteGroupId) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");
}

void ScummEngine_v90he::spriteMarkIfInGroup(int spriteGroupId, uint32 flags) {
	checkRange(_varNumSpriteGroups, 1, spriteGroupId, "Invalid sprite group %d");
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (spi->group_num == spriteGroupId) {
			spi->flags |= flags;
		}		
	}
}

void ScummEngine_v90he::spritesBlitToScreen() {
	int xmin, xmax, ymin, ymax;
	xmin = ymin = 1234;
	xmax = ymax = -1234; 
	bool firstLoop = true;
	bool refreshScreen = false;
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!(spi->flags & kSF31) && (spi->flags & kSF01)) {
			spi->flags &= ~kSF01;
			if (spi->bbox_xmin <= spi->bbox_xmax && spi->bbox_ymin <= spi->bbox_ymax) {
				if (spi->flags & kSFBlitDirectly) {
					gdi.copyVirtScreenBuffers(Common::Rect(spi->bbox_xmin, spi->bbox_ymin, spi->bbox_ymin, spi->bbox_ymax)); // XXX 0, 0x40000000);
				}
			} else if (firstLoop) {
				xmin = spi->bbox_xmin;
				ymin = spi->bbox_ymin;
				xmax = spi->bbox_xmax;
				ymax = spi->bbox_ymax;
				firstLoop = false;
			} else {
				if (xmin < spi->bbox_xmin) {
					xmin = spi->bbox_xmin;
				}
				if (ymin < spi->bbox_ymin) {
					ymin = spi->bbox_ymin;
				}
				if (xmax > spi->bbox_xmax) {
					xmax = spi->bbox_xmax;
				}
				if (ymax > spi->bbox_ymax) {
					ymax = spi->bbox_ymax;
				}
				refreshScreen = true;
			}
			if (!(spi->flags & (kSFNeedRedraw | kSF30)) && (spi->res_id != 0)) {
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
			if ((!unkFlag || spi->field_18 >= 0) && (spi->flags & kSF23)) {
				bool needRedraw = false;
				int lp = MIN(79, spi->bbox_xmin / 8);
				int rp = MIN(79, (spi->bbox_xmax + 7) / 8);
				for (; lp <= rp; ++lp) {
					if (vs0->tdirty[lp] < vs0->h && spi->bbox_ymax >= vs0->bdirty[lp] && spi->bbox_ymin <= vs0->tdirty[lp]) {
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
		if (spi->dx != 0 || spi->dy != 0) {
			checkRange(_varNumSprites, 1, i, "Invalid sprite %d");
			int tx = spi->tx;
			int ty = spi->ty;
			spi->tx += spi->dx;
			spi->ty += spi->dy;
			if (tx != spi->tx || ty != spi->ty) {
				spi->flags |= kSF01 | kSFNeedRedraw;
			}			
		}
		if (spi->flags & kSF22) {
			if (spi->field_78 != 0) {
				--spi->field_64;
				if (spi->field_64 != 0) {
					continue;
				}
				spi->field_64 = spi->field_78;
			}
			int state = spi->res_state;
			++spi->res_state;
			if (spi->res_state >= spi->res_wiz_states) {
				spi->res_state = 0;
				if (spi->imglist_num != 0) {
					if (!(spi->flags & kSF25)) {
						// XXX
						checkRange(_varMaxSprites, 1, spi->imglist_num, "Image list %d out of range");
						uint16 img1 = _imageListTable[0x21 * spi->imglist_num - 1];
						uint16 img2 = spi->field_74 + 1;
						if (img2 >= img1) {
							img2 = 0;
						}
						if (spi->field_74 != img2) {
							spi->field_74 = img2;
							spi->res_id = _imageListTable[0x21 * (img2 - 1)];
							spi->flags |= kSF01 | kSFNeedRedraw;
							spi->res_wiz_states = getWizImageStates(spi->res_id);
						}
					}
					continue;
				} else if (state == 0) {
					continue;
				}
			}
			spi->flags |= kSF01 | kSFNeedRedraw;
		}
	}
}

} // End of namespace Scumm
