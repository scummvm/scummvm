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
	spritesAllocTables(_numSprites, 64, 64);
}

int ScummEngine_v90he::spriteInfoGet_flags_1(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_1: Invalid sprite %d");

	return ((_spriteTable[spriteNum].flags & kSF15) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_2(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_2: Invalid sprite %d");

	return ((_spriteTable[spriteNum].flags & kSF14) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_3(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_3: Invalid sprite %d");

	return ((_spriteTable[spriteNum].flags & kSF16) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_4(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_4: Invalid sprite %d");

	return ((_spriteTable[spriteNum].flags & kSF13) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_5(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_5: Invalid sprite %d");

	return ((_spriteTable[spriteNum].flags & kSF20) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_6(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_6: Invalid sprite %d");

	return _spriteTable[spriteNum].flags & kSF22;
}

int ScummEngine_v90he::spriteInfoGet_flags_7(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_7: Invalid sprite %d");

	return ((_spriteTable[spriteNum].flags & kSF23) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_flags_8(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_8: Invalid sprite %d");

	return ((_spriteTable[spriteNum].flags & kSF31) != 0) ? 1 : 0;
}

int ScummEngine_v90he::spriteInfoGet_resId(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_resId: Invalid sprite %d");

	return _spriteTable[spriteNum].res_id;
}

int ScummEngine_v90he::spriteInfoGet_resState(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_resState: Invalid sprite %d");

	return _spriteTable[spriteNum].res_state;
}

int ScummEngine_v90he::spriteInfoGet_groupNum(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_groupNum: Invalid sprite %d");

	return _spriteTable[spriteNum].group_num;
}

int ScummEngine_v90he::spriteInfoGet_field_18(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_field_18: Invalid sprite %d");

	return _spriteTable[spriteNum].field_18;
}

int ScummEngine_v90he::spriteInfoGet_grp_tx(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_grp_tx: Invalid sprite %d");

	if (_spriteTable[spriteNum].group_num)
		return _spriteTable[spriteNum].tx + _spriteGroups[spriteNum].tx;
	else
		return _spriteTable[spriteNum].tx;
}

int ScummEngine_v90he::spriteInfoGet_grp_ty(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_grp_ty: Invalid sprite %d");

	if (_spriteTable[spriteNum].group_num)
		return _spriteTable[spriteNum].ty + _spriteGroups[spriteNum].ty;
	else
		return _spriteTable[spriteNum].ty;
}

int ScummEngine_v90he::spriteInfoGet_field_44(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_field_44: Invalid sprite %d");

	return _spriteTable[spriteNum].field_44;
}

int ScummEngine_v90he::spriteInfoGet_field_54(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_field_54: Invalid sprite %d");

	return _spriteTable[spriteNum].field_54;
}

int ScummEngine_v90he::spriteInfoGet_wizSize(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_wizSize: Invalid sprite %d");

	return _spriteTable[spriteNum].res_wiz_states;
}

int ScummEngine_v90he::spriteInfoGet_zoom(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_zoom: Invalid sprite %d");

	return _spriteTable[spriteNum].zoom;
}

int ScummEngine_v90he::spriteInfoGet_field_78(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_field_78: Invalid sprite %d");

	return _spriteTable[spriteNum].field_78;
}

int ScummEngine_v90he::spriteInfoGet_field_80(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_field_80: Invalid sprite %d");

	return _spriteTable[spriteNum].field_80;
}

void ScummEngine_v90he::getSpriteImageDim(int spriteNum, int32 &w, int32 &h) {
	checkRange(_numSprites, 1, spriteNum, "getSpriteImageDim: Invalid sprite %d");

	getWizImageDim(_spriteTable[spriteNum].res_id, _spriteTable[spriteNum].res_state, w, h);
}

void ScummEngine_v90he::spriteInfoGet_tx_ty(int spriteNum, int32 &tx, int32 &ty) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_tx_ty: Invalid sprite %d");

	tx = _spriteTable[spriteNum].tx;
	ty = _spriteTable[spriteNum].ty;
}

void ScummEngine_v90he::spriteInfoGet_field_2C_30(int spriteNum, int32 &field_2C, int32 &field_30) {
	checkRange(_numSprites, 1, spriteNum, "spriteInfoGet_field_2C_30: Invalid sprite %d");

	field_2C = _spriteTable[spriteNum].field_2C;
	field_30 = _spriteTable[spriteNum].field_30;
}

//
// spriteInfoSet functions
//
void ScummEngine_v90he::spriteInfoSet_field_14(int spriteNum, int value) {
	checkRange(_varNumSprites, 1, spriteNum, "Invalid sprite %d");

	if (_spriteTable[spriteNum].field_14 != value) {
		_spriteTable[spriteNum].field_14 = value;
		_spriteTable[spriteNum].flags |= kSF01 | kSF02;
	}
}

void ScummEngine_v90he::spriteInfoSet_field_7C(int spriteNum, int value) {
	checkRange(_varNumSprites, 1, spriteNum, "Invalid sprite %d");

	if (_spriteTable[spriteNum].field_7C != value) {
		_spriteTable[spriteNum].field_7C = value;
		_spriteTable[spriteNum].flags |= kSF01 | kSF02;
	}
}

void ScummEngine_v90he::spriteInfoSet_field_80(int spriteNum, int value) {
	checkRange(_varNumSprites, 1, spriteNum, "Invalid sprite %d");

	_spriteTable[spriteNum].field_80 = value;
}

void ScummEngine_v90he::spriteInfoSet_case183(int spriteNum) {
	checkRange(_varNumSprites, 1, spriteNum, "Invalid sprite %d");

	// TODO
}

void ScummEngine_v90he::spritesAllocTables(int numSprites, int numGroups, int numImgLists) {
	_varNumSpriteGroups = numGroups;
	_numSpritesToProcess = 0;
	_varNumSprites = numSprites;
	_varNumImgLists = numImgLists;
	_spriteGroups = (SpriteGroup *)malloc((_varNumSpriteGroups + 1) * sizeof(SpriteGroup));
	_spriteTable = (SpriteInfo *)malloc((_varNumSprites + 1) * sizeof(SpriteInfo));
	_activeSpritesTable = (SpriteInfo **)malloc((_varNumSprites + 1) * sizeof(SpriteInfo *));
	_imageListTable = (uint32 *)malloc((_varNumImgLists + 1) * sizeof(uint32)); // XXX
	_imageListStack = (uint16 *)malloc((_varNumImgLists + 1) * sizeof(uint16));
}

void ScummEngine_v90he::spritesResetTables(bool refreshScreen) {
	int i;
	for (i = 0; i < _varNumImgLists; ++i) {
		_imageListStack[i] = i;
	}
	memset(_spriteTable, 0, (_varNumSprites + 1) * sizeof(SpriteInfo));
	memset(_spriteGroups, 0, (_varNumSpriteGroups + 1) * sizeof(SpriteGroup));
	for (int curGrp = 1; curGrp < _varNumSpriteGroups; ++curGrp) {
		SpriteGroup *spg = &_spriteGroups[curGrp];
		checkRange(_varNumSpriteGroups, 1, curGrp, "Invalid sprite group %d");
		if (spg->field_10 != 0) {
			spg->field_10 = 0;
			spriteGroupCheck(curGrp);
			for (i = 0; i < _numSpritesToProcess; ++i) {
				SpriteInfo *spi = _activeSpritesTable[i];
				if (spi->group_num == curGrp) {
					spi->flags |= kSF01 | kSF02;
				}
			}
		}
		if (spg->tx != 0 || spg->ty != 0) {
			spg->tx = spg->ty = 0;
			spriteGroupCheck(curGrp);
			for (i = 0; i < _numSpritesToProcess; ++i) {
				SpriteInfo *spi = _activeSpritesTable[i];
				if (spi->group_num == curGrp) {
					spi->flags |= kSF01 | kSF02;
				}
			}
		}
		spg->flags &= ~kSGF01;
		spriteMarkIfInGroup(curGrp, kSF01 | kSF02);
		if (spg->field_20 != 0) {
			spriteGroupCheck(curGrp);
			for (i = 0; i < _numSpritesToProcess; ++i) {
				SpriteInfo *spi = _activeSpritesTable[i];
				if (spi->group_num == curGrp) {
					spi->flags |= kSF01 | kSF02;
				}
			}
		}
		spriteGroupCheck(curGrp);
		spg->scaling = 0;
		spg->scale_x = 0x3F800000;
		spg->field_30 = 0;
		spg->field_34 = 0;
		spg->scale_y = 0x3F800000;
		spg->field_38 = 0;
		spg->field_3C = 0;
	}
	if (refreshScreen) {
		gdi.copyVirtScreenBuffers(Common::Rect(_screenWidth, _screenHeight));
	}
	_numSpritesToProcess = 0;
}

void ScummEngine_v90he::spriteGroupCheck(int sprGrpId) {
	// XXX
}

void ScummEngine_v90he::spriteMarkIfInGroup(int sprGrpId, uint32 flags) {
	// XXX
}

} // End of namespace Scumm
