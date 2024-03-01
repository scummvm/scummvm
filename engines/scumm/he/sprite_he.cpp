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

#ifdef ENABLE_HE

#include "scumm/he/intern_he.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/he/sprite_he.h"
#include "scumm/usage_bits.h"
#include "scumm/util.h"
#include "scumm/he/wiz_he.h"

namespace Scumm {

Sprite::Sprite(ScummEngine_v90he *vm)
	:
	_vm(vm),
	_spriteGroups(0),
	_spriteTable(0),
	_activeSpritesTable(0),
	_numSpritesToProcess(0),
	_varNumSpriteGroups(0),
	_varNumSprites(0),
	_varMaxSprites(0) {
}

Sprite::~Sprite() {
	free(_spriteGroups);
	free(_spriteTable);
	free(_activeSpritesTable);
}

void ScummEngine_v90he::allocateArrays() {
	ScummEngine_v70he::allocateArrays();
	_sprite->allocTables(_numSprites, MAX(64, _numSprites / 4), 64);
}

void Sprite::getSpriteBounds(int spriteId, bool checkGroup, Common::Rect &bound) {
	assertRange(1, spriteId, _varNumSprites, "sprite");
	int32 spr_wiz_x, spr_wiz_y;
	int angle, scale, x1, y1;
	int32 w, h;

	SpriteInfo *spi = &_spriteTable[spriteId];

	_vm->_wiz->getWizImageSpot(spi->image, spi->state, spr_wiz_x, spr_wiz_y);
	if (checkGroup && spi->group) {
		SpriteGroup *spg = &_spriteGroups[spi->group];

		if (spg->scaling) {
			x1 = spi->posX * spg->scale_x_ratio_mul / spg->scale_x_ratio_div - spr_wiz_x + spg->tx;
			y1 = spi->posY * spg->scale_y_ratio_mul / spg->scale_y_ratio_div - spr_wiz_y + spg->ty;
		} else {
			x1 = spi->posX - spr_wiz_x + spg->tx;
			y1 = spi->posY - spr_wiz_y + spg->ty;
		}
	} else {
		x1 = spi->posX - spr_wiz_x;
		y1 = spi->posY - spr_wiz_y;
	}

	if (spi->image) {
		angle = spi->angle;
		scale = spi->scale;
		_vm->_wiz->getWizImageDim(spi->image, spi->state, w, h);
		if (spi->flags & (kSFScaleSpecified | kSFAngleSpecified)) {
			Common::Point pts[4];
			_vm->_wiz->polygonTransform(spi->image, spi->state, x1, y1, angle, scale, pts);
			_vm->_wiz->polyBuildBoundingRect(pts, 4, bound);
		} else {
			bound.left = x1;
			bound.top = y1;
			bound.right = x1 + w;
			bound.bottom = y1 + h;
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
int Sprite::findSpriteWithClassOf(int x_pos, int y_pos, int spriteGroupId, int type, int num, int *args) {
	debug(7, "findSprite: x %d, y %d, spriteGroup %d, type %d, num %d", x_pos, y_pos, spriteGroupId, type, num);
	Common::Point pos[1];
	bool cond;
	int code, classId;

	for (int i = (_numSpritesToProcess - 1); i >= 0; i--) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!spi->lastImage)
			continue;

		if (spriteGroupId && spi->group != spriteGroupId)
			continue;

		cond = true;
		for (int j = 0; j < num; j++) {
			code = classId = args[j];
			classId &= 0x7F;
			assertRange(1, classId, 32, "class");
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
			if (spi->lastRect.left > spi->lastRect.right)
				continue;
			if (spi->lastRect.top > spi->lastRect.bottom)
				continue;
			if (spi->lastRect.left > x_pos)
				continue;
			if (spi->lastRect.top > y_pos)
				continue;
			if (spi->lastRect.right < x_pos)
				continue;
			if (spi->lastRect.bottom < y_pos)
				continue;
			return spi->id;
		} else {
			int image, imageState, angle, scale;
			int32 w, h;

			image = spi->lastImage;
			if (spi->maskImage) {
				int32 x1, x2, y1, y2;

				image = spi->maskImage;
				imageState = spi->lastState % _vm->_wiz->getWizImageStates(spi->maskImage);

				pos[0].x = x_pos - spi->lastSpot.x;
				pos[0].y = y_pos - spi->lastSpot.y;

				_vm->_wiz->getWizImageSpot(spi->lastImage, imageState, x1, y1);
				_vm->_wiz->getWizImageSpot(spi->maskImage, imageState, x2, y2);

				pos[0].x += (x2 - x1);
				pos[0].y += (y2 - y1);
			} else {
				if (spi->lastRect.left > spi->lastRect.right)
					continue;
				if (spi->lastRect.top > spi->lastRect.bottom)
					continue;
				if (spi->lastRect.left > x_pos)
					continue;
				if (spi->lastRect.top > y_pos)
					continue;
				if (spi->lastRect.right < x_pos)
					continue;
				if (spi->lastRect.bottom < y_pos)
					continue;

				pos[0].x = x_pos - spi->lastSpot.x;
				pos[0].y = y_pos - spi->lastSpot.y;
				imageState = spi->lastState;
			}

			angle = spi->lastAngle;
			scale = spi->lastScale;
			if ((spi->flags & kSFScaleSpecified) || (spi->flags & kSFAngleSpecified)) {
				if (spi->flags & kSFScaleSpecified && scale) {
					pos[0].x = pos[0].x * 256 / scale;
					pos[0].y = pos[0].y * 256 / scale;
				}
				if (spi->flags & kSFAngleSpecified && angle) {
					angle = (360 - angle) % 360;
					_vm->_wiz->polyRotatePoints(pos, 1, angle);
				}

				_vm->_wiz->getWizImageDim(image, imageState, w, h);
				pos[0].x += w / 2;
				pos[0].y += h / 2;
			}

			if (_vm->_wiz->isWizPixelNonTransparent(image, imageState, pos[0].x, pos[0].y, spi->lastRenderFlags))
				return spi->id;
		}
	}

	return 0;
}

int Sprite::getSpriteClass(int spriteId, int num, int *args) {
	assertRange(1, spriteId, _varNumSprites, "sprite");
	int code, classId;

	if (num == 0)
		return _spriteTable[spriteId].classFlags;

	for (int i = 0; i < num; i++) {
		code = classId = args[i];
		classId &= 0x7F;
		assertRange(1, classId, 32, "class");
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

int Sprite::getSpriteFlagDoubleBuffered(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFBackgroundRender) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagYFlipped(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFVFlip) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagXFlipped(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFHFlip) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagActive(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFActive) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagRemapPalette(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFUseImageRemap) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagAutoAnim(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	// WORKAROUND: Two scripts (room 2 script 2070/2071) compare against
	// a return value of one, but the original game returned the flag value
	// (0x200000) for true. These scripts bugs caused problems (infinite loop
	// and beans not appearing) in the Jumping Beans mini games under ScummVM.
	if (_vm->_game.id == GID_PJGAMES)
		return 0;

	return ((_spriteTable[spriteId].flags & kSFAutoAnimate) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagUpdateType(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFSmartRender) != 0) ? 1 : 0;
}

int Sprite::getSpriteFlagEraseType(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFIgnoreErase) != 0) ? 1 : 0;
}

int Sprite::getSpriteImage(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].image;
}

int Sprite::getSpriteImageState(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].state;
}

int Sprite::getSpriteGroup(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].group;
}

int Sprite::getSpritePalette(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].palette;
}

int Sprite::getSpritePriority(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].priority;
}

int Sprite::getSpriteDisplayX(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	if (_spriteTable[spriteId].group)
		return _spriteTable[spriteId].posX + _spriteGroups[_spriteTable[spriteId].group].tx;
	else
		return _spriteTable[spriteId].posX;
}

int Sprite::getSpriteDisplayY(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	if (_spriteTable[spriteId].group)
		return _spriteTable[spriteId].posY + _spriteGroups[_spriteTable[spriteId].group].ty;
	else
		return _spriteTable[spriteId].posY;
}

int Sprite::getSpriteUserValue(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].userValue;
}

int Sprite::getSpriteShadow(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].shadow;
}

int Sprite::getSpriteImageStateCount(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].maxStates;
}

int Sprite::getSpriteScale(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].scale;
}

int Sprite::getSpriteAnimSpeed(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].animSpeed;
}

int Sprite::getSpriteSourceImage(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].sourceImage;
}

int Sprite::getSpriteMaskImage(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	return _spriteTable[spriteId].maskImage;
}

int Sprite::getSpriteGeneralProperty(int spriteId, int type) {
	debug(7, "getSpriteGeneralProperty: spriteId %d type 0x%x", spriteId, type);
	assertRange(1, spriteId, _varNumSprites, "sprite");

	// XXX U32 related check

	switch (type) {
	case 0x7B:
		return _spriteTable[spriteId].specialRenderFlags;
	case 0x7D:
		return _spriteTable[spriteId].conditionBits;
	case 0x7E:
		return _spriteTable[spriteId].animState;
	default:
		error("getSpriteGeneralProperty: Invalid type %d", type);
	}
}

void Sprite::getSpriteImageDim(int spriteId, int32 &w, int32 &h) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	if (_spriteTable[spriteId].image) {
		_vm->_wiz->getWizImageDim(_spriteTable[spriteId].image, _spriteTable[spriteId].state, w, h);
	} else {
		w = 0;
		h = 0;
	}
}

void Sprite::getSpritePosition(int spriteId, int32 &tx, int32 &ty) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	tx = _spriteTable[spriteId].posX;
	ty = _spriteTable[spriteId].posY;
}

void Sprite::getSpriteDist(int spriteId, int32 &dx, int32 &dy) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	dx = _spriteTable[spriteId].deltaPosX;
	dy = _spriteTable[spriteId].deltaPosY;
}

//
// spriteGroupGet functions
//
int ScummEngine_v90he::getGroupSpriteArray(int spriteGroupId) {
	int i, numSprites = 0;

	assertRange(1, spriteGroupId, _sprite->_varNumSpriteGroups, "sprite group");

	for (i = (_sprite->_varNumSprites - 1); i > 0; i--) {
		if (_sprite->_spriteTable[i].group == spriteGroupId)
			numSprites++;
	}

	if (!numSprites)
		return 0;

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, numSprites);
	writeArray(0, 0, 0, numSprites);

	numSprites = 1;
	for (i = (_sprite->_varNumSprites - 1); i > 0; i--) {
		if (_sprite->_spriteTable[i].group == spriteGroupId) {
			writeArray(0, 0, numSprites, i);
			numSprites++;
		}
	}

	return readVar(0);
}

int Sprite::getGroupPriority(int spriteGroupId) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	return _spriteGroups[spriteGroupId].priority;
}

int Sprite::getGroupDstResNum(int spriteGroupId) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	return _spriteGroups[spriteGroupId].image;
}

int Sprite::getGroupXMul(int spriteGroupId) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	return _spriteGroups[spriteGroupId].scale_x_ratio_mul;
}

int Sprite::getGroupXDiv(int spriteGroupId) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	return _spriteGroups[spriteGroupId].scale_x_ratio_div;
}

int Sprite::getGroupYMul(int spriteGroupId) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	return _spriteGroups[spriteGroupId].scale_y_ratio_mul;
}

int Sprite::getGroupYDiv(int spriteGroupId) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	return _spriteGroups[spriteGroupId].scale_y_ratio_div;
}

void Sprite::getGroupPosition(int spriteGroupId, int32 &tx, int32 &ty) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	tx = _spriteGroups[spriteGroupId].tx;
	ty = _spriteGroups[spriteGroupId].ty;
}

//
// spriteInfoSet functions
//
void Sprite::setSpritePalette(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	if (_spriteTable[spriteId].palette != value) {
		_spriteTable[spriteId].palette = value;
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
	}
}

void Sprite::setSpriteSourceImage(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	if (_spriteTable[spriteId].sourceImage != value) {
		_spriteTable[spriteId].sourceImage = value;
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
	}
}

void Sprite::setSpriteMaskImage(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].maskImage = value;
}

void Sprite::setSpriteImageState(int spriteId, int state) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	if (_spriteTable[spriteId].image) {
		int imageStateCount = _spriteTable[spriteId].maxStates - 1;
		state = MAX(0, state);
		state = MIN(state, imageStateCount);

		if (_spriteTable[spriteId].state != state) {
			_spriteTable[spriteId].state = state;
			_spriteTable[spriteId].flags |= kSFErase | kSFRender;
		}
	}
}

void Sprite::setSpritePosition(int spriteId, int tx, int ty) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	if (_spriteTable[spriteId].posX != tx || _spriteTable[spriteId].posY != ty) {
		_spriteTable[spriteId].posX = tx;
		_spriteTable[spriteId].posY = ty;
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
	}
}

void Sprite::setSpriteGroup(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");
	assertRange(0, value, _varNumSpriteGroups, "sprite group");

	_spriteTable[spriteId].group = value;
	_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteDist(int spriteId, int value1, int value2) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].deltaPosX = value1;
	_spriteTable[spriteId].deltaPosY = value2;
}

void Sprite::setSpriteShadow(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].shadow = value;
	if (_spriteTable[spriteId].image)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteUserValue(int spriteId, int value1, int value2) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].userValue = value2;
}

void Sprite::setSpritePriority(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].priority = value;
}

void Sprite::moveSprite(int spriteId, int value1, int value2) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].posX += value1;
	_spriteTable[spriteId].posY += value2;

	if  (value1 || value2)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteScale(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].flags |= kSFScaleSpecified;

	if (_spriteTable[spriteId].scale != value) {
		_spriteTable[spriteId].scale = value;

		if (_spriteTable[spriteId].image)
			_spriteTable[spriteId].flags |= kSFErase | kSFRender;
	}
}

void Sprite::setSpriteAngle(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].flags |= kSFAngleSpecified;

	if (_spriteTable[spriteId].angle != value) {
		_spriteTable[spriteId].angle = value;

		if (_spriteTable[spriteId].image)
			_spriteTable[spriteId].flags |= kSFErase | kSFRender;
	}
}

void Sprite::setSpriteFlagDoubleBuffered(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFBackgroundRender;
	else
		_spriteTable[spriteId].flags &= ~kSFBackgroundRender;

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteFlagYFlipped(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFVFlip;
	else
		_spriteTable[spriteId].flags &= ~kSFVFlip;

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteFlagXFlipped(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFHFlip;
	else
		_spriteTable[spriteId].flags &= ~kSFHFlip;

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteFlagActive(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	if (value)
		_spriteTable[spriteId].flags |= kSFActive;
	else
		_spriteTable[spriteId].flags &= ~kSFActive;
}

void Sprite::setSpriteFlagRemapPalette(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value)
		_spriteTable[spriteId].flags |= kSFUseImageRemap;
	else
		_spriteTable[spriteId].flags &= ~kSFUseImageRemap;

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteFlagAutoAnim(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	if (value)
		_spriteTable[spriteId].flags |= kSFAutoAnimate;
	else
		_spriteTable[spriteId].flags &= ~kSFAutoAnimate;
}

void Sprite::setSpriteFlagUpdateType(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	switch (value) {
	case 2:
		_spriteTable[spriteId].flags &= ~(kSFSmartRender);
		_spriteTable[spriteId].flags |= kSFDontCombineErase;
		break;
	case 1:
		_spriteTable[spriteId].flags |= kSFSmartRender | kSFDontCombineErase;
		break;
	case 0:
		_spriteTable[spriteId].flags &= ~(kSFSmartRender | kSFDontCombineErase);
		break;
	default:
		error("setSpriteFlagUpdateType: Invalid value %d", value);
	}
}

void Sprite::setSpriteFlagEraseType(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	// Note that condition is inverted
	if (!value)
		_spriteTable[spriteId].flags |= kSFIgnoreErase;
	else
		_spriteTable[spriteId].flags &= ~kSFIgnoreErase;
}

void Sprite::setSpriteAnimSpeed(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].animSpeed = value;
	_spriteTable[spriteId].animState = value;
}

void Sprite::setSpriteSetClass(int spriteId, int classId, int toggle) {
	assertRange(1, spriteId, _varNumSprites, "sprite");
	assertRange(1, classId, 32, "class");

	if (toggle) {
		_spriteTable[spriteId].classFlags |= (1 << (classId - 1));
	} else {
		_spriteTable[spriteId].classFlags &= ~(1 << (classId - 1));
	}
}

void Sprite::setSpriteResetClass(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].classFlags = 0;
}

void Sprite::setSpriteZBuffer(int spriteId, int value) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].zbufferImage = value;
}

void Sprite::setSpriteGeneralProperty(int spriteId, int type, int value) {
	debug(7, "setSpriteGeneralProperty: spriteId %d type 0x%x value 0x%x", spriteId, type, value);
	assertRange(1, spriteId, _varNumSprites, "sprite");
	int32 delay;

	switch (type) {
	case 0x7B:
		_spriteTable[spriteId].specialRenderFlags = value;
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
		break;
	case 0x7D:
		_spriteTable[spriteId].conditionBits = value;
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
		break;
	case 0x7E:
		delay = MAX(0, value);
		delay = MIN(delay, _spriteTable[spriteId].animSpeed);

		_spriteTable[spriteId].animState = delay;
		break;
	default:
		error("setSpriteGeneralProperty: Invalid value %d", type);
	}
}

void Sprite::resetSprite(int spriteId) {
	assertRange(1, spriteId, _varNumSprites, "sprite");

	_spriteTable[spriteId].angle = 0;
	_spriteTable[spriteId].scale = 0;

	setSpriteImage(spriteId, 0);

	_spriteTable[spriteId].shadow = 0;
	_spriteTable[spriteId].posX = 0;
	_spriteTable[spriteId].posY = 0;

	_spriteTable[spriteId].flags &= ~(kSFVFlip | kSFHFlip);
	_spriteTable[spriteId].flags |= kSFErase | kSFRender;
	_spriteTable[spriteId].deltaPosX = 0;
	_spriteTable[spriteId].deltaPosY = 0;
	_spriteTable[spriteId].userValue = 0;
	_spriteTable[spriteId].group = 0;
	_spriteTable[spriteId].animSpeed = 0;
	_spriteTable[spriteId].animState = 0;
	_spriteTable[spriteId].classFlags = 0;
	_spriteTable[spriteId].palette = 0;
	_spriteTable[spriteId].sourceImage = 0;
	_spriteTable[spriteId].maskImage = 0;
	_spriteTable[spriteId].priority = 0;
	_spriteTable[spriteId].zbufferImage = 0;
	_spriteTable[spriteId].specialRenderFlags = 0;
	_spriteTable[spriteId].conditionBits = 0;

	if (_vm->_game.heversion >= 100) {
		_spriteTable[spriteId].flags &= ~kSFSmartRender;
		_spriteTable[spriteId].flags |= kSFAutoAnimate | kSFDontCombineErase;
	}
}

void Sprite::setSpriteImage(int spriteId, int imageNum) {
	int origResId, origResWizStates;

	assertRange(1, spriteId, _varNumSprites, "sprite");

	origResId = _spriteTable[spriteId].image;
	origResWizStates = _spriteTable[spriteId].maxStates;

	_spriteTable[spriteId].image = imageNum;
	_spriteTable[spriteId].animIndex = 0;
	_spriteTable[spriteId].state = 0;

	if (_spriteTable[spriteId].image) {
		_spriteTable[spriteId].maxStates = _vm->_wiz->getWizImageStates(_spriteTable[spriteId].image);

		if (_vm->VAR(139))
			_spriteTable[spriteId].flags |= kSFActive;
		else
			_spriteTable[spriteId].flags |= kSFActive | kSFAutoAnimate | kSFSmartRender | kSFDontCombineErase;

		if (_spriteTable[spriteId].image != origResId || _spriteTable[spriteId].maxStates != origResWizStates)
			_spriteTable[spriteId].flags |= kSFErase | kSFRender;
	} else {
		if (_vm->VAR(139))
			_spriteTable[spriteId].flags &= ~kSFActive;
		else if (_vm->_game.heversion >= 100 && origResId == 0)
			_spriteTable[spriteId].flags = 0;
		else if (_spriteTable[spriteId].flags & kSFIgnoreErase)
			_spriteTable[spriteId].flags = 0;
		else
			_spriteTable[spriteId].flags = kSFErase | kSFDontCombineErase;
		_spriteTable[spriteId].lastImage = 0;
		_spriteTable[spriteId].lastState = 0;
		_spriteTable[spriteId].maxStates = 0;
	}
}

//
// spriteGroupSet functions
//
void Sprite::redrawSpriteGroup(int spriteGroupId) {
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (spi->group == spriteGroupId) {
			spi->flags |= kSFErase | kSFRender;
		}
	}
}

void Sprite::moveGroupMembers(int spriteGroupId, int value1, int value2) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].posX += value1;
			_spriteTable[i].posY += value2;

			if (value1 || value2)
				_spriteTable[i].flags |= kSFErase | kSFRender;
		}
	}
}

void Sprite::setGroupMembersPriority(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId)
			_spriteTable[i].priority = value;
	}
}

void Sprite::setGroupMembersGroup(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].group = value;
			_spriteTable[i].flags |= kSFErase | kSFRender;
		}
	}
}

void Sprite::setGroupMembersUpdateType(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId)
			setSpriteFlagUpdateType(i, value);
	}
}

void Sprite::setGroupMembersResetSprite(int spriteGroupId) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId)
			resetSprite(i);
	}
}

void Sprite::setGroupMembersAnimationSpeed(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].animSpeed = value;
			_spriteTable[i].animState = value;
		}
	}
}

void Sprite::setGroupMembersAutoAnimFlag(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			if (value)
				_spriteTable[i].flags |= kSFAutoAnimate;
			else
				_spriteTable[i].flags &= ~kSFAutoAnimate;
		}
	}
}

void Sprite::setGroupMembersShadow(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	for (int i = 1; i < _varNumSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].shadow = value;
			if (_spriteTable[i].image)
				_spriteTable[i].flags |= kSFErase | kSFRender;
		}
	}
}

void Sprite::setGroupBounds(int spriteGroupId, int x1, int y1, int x2, int y2) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	_spriteGroups[spriteGroupId].flags |= kSGFUseClipRect;
	_spriteGroups[spriteGroupId].bbox.left = x1;
	_spriteGroups[spriteGroupId].bbox.top = y1;
	_spriteGroups[spriteGroupId].bbox.right = x2;
	_spriteGroups[spriteGroupId].bbox.bottom = y2;
	_vm->adjustRect(_spriteGroups[spriteGroupId].bbox);

	redrawSpriteGroup(spriteGroupId);
}

void Sprite::setGroupPriority(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	if (_spriteGroups[spriteGroupId].priority != value) {
		_spriteGroups[spriteGroupId].priority = value;
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupPosition(int spriteGroupId, int value1, int value2) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	if (_spriteGroups[spriteGroupId].tx != value1 || _spriteGroups[spriteGroupId].ty != value2) {
		_spriteGroups[spriteGroupId].tx = value1;
		_spriteGroups[spriteGroupId].ty = value2;
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::moveGroup(int spriteGroupId, int value1, int value2) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	if (value1 || value2) {
		_spriteGroups[spriteGroupId].tx += value1;
		_spriteGroups[spriteGroupId].ty += value2;
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupImage(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	if (_spriteGroups[spriteGroupId].image != value) {
		_spriteGroups[spriteGroupId].image = value;
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupScaling(int spriteGroupId) {
	if ((_spriteGroups[spriteGroupId].scale_x_ratio_mul != _spriteGroups[spriteGroupId].scale_x_ratio_div) || (_spriteGroups[spriteGroupId].scale_y_ratio_mul != _spriteGroups[spriteGroupId].scale_y_ratio_div))
		_spriteGroups[spriteGroupId].scaling = 1;
	else
		_spriteGroups[spriteGroupId].scaling = 0;

}

void Sprite::setGroupXMul(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	if (_spriteGroups[spriteGroupId].scale_x_ratio_mul != value) {
		_spriteGroups[spriteGroupId].scale_x_ratio_mul = value;
		setGroupScaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupXDiv(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	if (value == 0)
		error("setGroupXDiv: Divisor must not be 0");

	if (_spriteGroups[spriteGroupId].scale_x_ratio_div != value) {
		_spriteGroups[spriteGroupId].scale_x_ratio_div = value;
		setGroupScaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupYMul(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	if (_spriteGroups[spriteGroupId].scale_y_ratio_mul != value) {
		_spriteGroups[spriteGroupId].scale_y_ratio_mul = value;
		setGroupScaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::setGroupYDiv(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	if (value == 0)
		error("setGroupYDiv: Divisor must not be 0");

	if (_spriteGroups[spriteGroupId].scale_y_ratio_div != value) {
		_spriteGroups[spriteGroupId].scale_y_ratio_div = value;
		setGroupScaling(spriteGroupId);
		redrawSpriteGroup(spriteGroupId);
	}
}

void Sprite::resetGroupBounds(int spriteGroupId) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");

	_spriteGroups[spriteGroupId].flags &= ~(kSGFUseClipRect);
	redrawSpriteGroup(spriteGroupId);
}

void Sprite::allocTables(int numSprites, int numGroups, int numMaxSprites) {
	_varNumSpriteGroups = numGroups;
	_numSpritesToProcess = 0;
	_varNumSprites = numSprites;
	_varMaxSprites = numMaxSprites;
	_spriteGroups = (SpriteGroup *)malloc((_varNumSpriteGroups + 1) * sizeof(SpriteGroup));
	_spriteTable = (SpriteInfo *)malloc((_varNumSprites + 1) * sizeof(SpriteInfo));
	_activeSpritesTable = (SpriteInfo **)malloc((_varNumSprites + 1) * sizeof(SpriteInfo *));
}

void Sprite::resetGroup(int spriteGroupId) {
	assertRange(1, spriteGroupId, _varNumSpriteGroups, "sprite group");
	SpriteGroup *spg = &_spriteGroups[spriteGroupId];

	spg->priority = 0;
	spg->tx = spg->ty = 0;

	spg->flags &= ~kSGFUseClipRect;
	redrawSpriteGroup(spriteGroupId);

	spg->image = 0;
	spg->scaling = 0;
	spg->scale_x_ratio_mul = 1;
	spg->scale_x_ratio_div = 1;
	spg->scale_y_ratio_mul = 1;
	spg->scale_y_ratio_div = 1;
}

void Sprite::resetTables(bool refreshScreen) {
	for (int i = 0; i < _varNumSprites; i++) {
		_spriteTable[i].reset();
	}
	for (int i = 0; i < _varNumSpriteGroups; i++) {
		_spriteGroups[i].reset();
	}

	for (int curGrp = 1; curGrp < _varNumSpriteGroups; ++curGrp)
		resetGroup(curGrp);

	if (refreshScreen) {
		_vm->backgroundToForegroundBlit(Common::Rect(_vm->_screenWidth, _vm->_screenHeight));
	}
	_numSpritesToProcess = 0;
}

void Sprite::resetBackground() {
	int xmin, xmax, ymin, ymax;
	xmin = ymin = 1234;
	xmax = ymax = -1234;
	bool firstLoop = true;
	bool refreshScreen = false;

	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!(spi->flags & kSFIgnoreErase) && (spi->flags & kSFErase)) {
			spi->flags &= ~kSFErase;
			if (spi->lastRect.left <= spi->lastRect.right && spi->lastRect.top <= spi->lastRect.bottom) {
				if (spi->flags & kSFDontCombineErase) {
					_vm->backgroundToForegroundBlit(spi->lastRect, USAGE_BIT_RESTORED);
				} else if (firstLoop) {
					xmin = spi->lastRect.left;
					ymin = spi->lastRect.top;
					xmax = spi->lastRect.right;
					ymax = spi->lastRect.bottom;
					firstLoop = false;
					refreshScreen = true;
				} else {
					if (xmin > spi->lastRect.left) {
						xmin = spi->lastRect.left;
					}
					if (ymin > spi->lastRect.top) {
						ymin = spi->lastRect.top;
					}
					if (xmax < spi->lastRect.right) {
						xmax = spi->lastRect.right;
					}
					if (ymax < spi->lastRect.bottom) {
						ymax = spi->lastRect.bottom;
					}
					refreshScreen = true;
				}
				if (!(spi->flags & kSFRender) && spi->image)
					spi->flags |= kSFRender;
			}
		}
	}
	if (refreshScreen) {
		_vm->backgroundToForegroundBlit(Common::Rect(xmin, ymin, xmax, ymax), USAGE_BIT_RESTORED);
	}
}

void Sprite::checkForForcedRedraws(bool checkZOrder) {
	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (!(spi->flags & kSFRender)) {
			if ((!checkZOrder || spi->priority >= 0) && (spi->flags & kSFSmartRender)) {
				int32 lp = spi->lastRect.left / 8;
				lp = MAX((int32)0, lp);
				lp = MIN(lp, _vm->_gdi->_numStrips);
				int32 rp = (spi->lastRect.right + 7) / 8;
				rp = MAX((int32)0, rp);
				rp = MIN(rp, _vm->_gdi->_numStrips);
				for (; lp < rp; lp++) {
					if (vs->tdirty[lp] < vs->h && spi->lastRect.bottom >= vs->tdirty[lp] && spi->lastRect.top <= vs->bdirty[lp]) {
						spi->flags |= kSFRender;
						break;
					}
				}
			}
		}
	}
}

void Sprite::updateImages() {
	for (int i = 0; i < _numSpritesToProcess; ++i) {
		SpriteInfo *spi = _activeSpritesTable[i];
		if (spi->deltaPosX || spi->deltaPosY) {
			int tx = spi->posX;
			int ty = spi->posY;
			spi->posX += spi->deltaPosX;
			spi->posY += spi->deltaPosY;
			if (tx != spi->posX || ty != spi->posY) {
				spi->flags |= kSFErase | kSFRender;
			}
		}
		if (spi->flags & kSFAutoAnimate) {
			if (spi->animSpeed) {
				--spi->animState;
				if (spi->animState)
					continue;

				spi->animState = spi->animSpeed;
			}
			int imageState = spi->state;
			++spi->state;
			if (spi->state >= spi->maxStates) {
				spi->state = 0;
				if (imageState == 0)
					continue;
			}
			spi->flags |= kSFErase | kSFRender;
		}
	}
}

static int compareSprTable(const void *a, const void *b) {
	const SpriteInfo *spr1 = *(const SpriteInfo *const*)a;
	const SpriteInfo *spr2 = *(const SpriteInfo *const*)b;

	if (spr1->combinedPriority > spr2->combinedPriority)
		return 1;

	if (spr1->combinedPriority < spr2->combinedPriority)
		return -1;

	return 0;
}

void Sprite::sortActiveSprites() {
	int groupZorder;

	_numSpritesToProcess = 0;

	if (_varNumSprites <= 1)
		return;

	for (int i = 1; i < _varNumSprites; i++) {
		SpriteInfo *spi = &_spriteTable[i];

		if (spi->flags & kSFActive) {
			if (!(spi->flags & kSFSmartRender)) {
				spi->flags |= kSFRender;
				if (!(spi->flags & kSFIgnoreErase))
					spi->flags |= kSFErase;
			}
			if (spi->group)
				groupZorder = _spriteGroups[spi->group].priority;
			else
				groupZorder = 0;

			spi->id = i;
			spi->combinedPriority = spi->priority + groupZorder;

			_activeSpritesTable[_numSpritesToProcess++] = spi;
		}
	}

	if (_numSpritesToProcess < 2)
		return;

	qsort(_activeSpritesTable, _numSpritesToProcess, sizeof(SpriteInfo *), compareSprTable);
}

void Sprite::renderSprites(bool arg) {
	int spr_flags;
	int32 spr_wiz_x, spr_wiz_y;
	int image, imageState;
	Common::Rect *bboxPtr;
	int angle, scale;
	int32 w, h;
	WizImageCommand wiz;

	for (int i = 0; i < _numSpritesToProcess; i++) {
		SpriteInfo *spi = _activeSpritesTable[i];

		if (!(spi->flags & kSFRender))
			continue;

		spr_flags = spi->flags;

		if (arg) {
			if (spi->combinedPriority >= 0)
				return;
		} else {
			if (spi->combinedPriority < 0)
				continue;
		}

		spi->flags &= ~kSFRender;
		image = spi->image;
		imageState = spi->state;
		_vm->_wiz->getWizImageSpot(spi->image, spi->state, spr_wiz_x, spr_wiz_y);

		if (spi->group) {
			SpriteGroup *spg = &_spriteGroups[spi->group];

			if (spg->scaling) {
				wiz.xPos = spi->posX * spg->scale_x_ratio_mul / spg->scale_x_ratio_div - spr_wiz_x + spg->tx;
				wiz.yPos = spi->posY * spg->scale_y_ratio_mul / spg->scale_y_ratio_div - spr_wiz_y + spg->ty;
			} else {
				wiz.xPos = spi->posX - spr_wiz_x + spg->tx;
				wiz.yPos = spi->posY - spr_wiz_y + spg->ty;
			}
		} else {
			wiz.xPos = spi->posX - spr_wiz_x;
			wiz.yPos = spi->posY - spr_wiz_y;
		}

		wiz.extendedRenderInfo.sprite = spi->id;
		wiz.extendedRenderInfo.group = spi->group;
		wiz.extendedRenderInfo.conditionBits = spi->conditionBits;
		spi->lastState = wiz.state = imageState;
		spi->lastImage = wiz.image = image;
		wiz.actionFlags = kWAFState | kWAFSpot;
		spi->lastAngle = spi->angle;
		spi->lastScale = spi->scale;
		spi->lastSpot.x = wiz.xPos;
		spi->lastSpot.y = wiz.yPos;
		bboxPtr = &spi->lastRect;
		if (image) {
			angle = spi->angle;
			scale = spi->scale;
			_vm->_wiz->getWizImageDim(image, imageState, w, h);
			if (spi->flags & (kSFScaleSpecified | kSFAngleSpecified)) {
				Common::Point pts[4];
				_vm->_wiz->polygonTransform(image, imageState, wiz.xPos, wiz.yPos, angle, scale, pts);
				_vm->_wiz->polyBuildBoundingRect(pts, 4, spi->lastRect);
			} else {
				bboxPtr->left = wiz.xPos;
				bboxPtr->top = wiz.yPos;
				bboxPtr->right = wiz.xPos + w;
				bboxPtr->bottom = wiz.yPos + h;
			}
		} else {
			bboxPtr->left = 1234;
			bboxPtr->top = 1234;
			bboxPtr->right = -1234;
			bboxPtr->bottom = -1234;
		}

		wiz.flags = kWRFForeground;
		wiz.zPos = 0;
		if (spr_flags & kSFHFlip)
			wiz.flags |= kWRFHFlip;
		if (spr_flags & kSFVFlip)
			wiz.flags |= kWRFVFlip;
		if (spr_flags & kSFBackgroundRender) {
			wiz.flags &= ~kWRFForeground;
			wiz.flags |= kWRFBackground;
		}
		if (spi->shadow) {
			wiz.flags |= kWRFUseShadow;
			wiz.actionFlags |= kWAFShadow;
			wiz.shadow = spi->shadow;
		}
		if (spr_flags & kSFUseImageRemap)
			wiz.flags |= kWRFRemap;
		if (spi->zbufferImage) {
			wiz.actionFlags |= kWAFZBufferImage;
			wiz.zbufferImage = spi->zbufferImage;
			wiz.zPos = spi->priority;
		}
		if (spi->sourceImage) {
			wiz.actionFlags |= kWAFSourceImage;
			wiz.sourceImage = spi->sourceImage;
		}
		wiz.actionFlags |= kWAFFlags;
		wiz.flags |= spi->specialRenderFlags;

		if (spr_flags & kSFAngleSpecified) {
			wiz.actionFlags |= kWAFAngle;
			wiz.angle = spi->angle;
		}
		if (spr_flags & kSFScaleSpecified) {
			wiz.actionFlags |= kWAFScale;
			wiz.scale = spi->scale;
		}
		spi->lastRenderFlags = wiz.flags;

		if (spi->group && (_spriteGroups[spi->group].flags & kSGFUseClipRect)) {
			Common::Rect &spgBbox = _spriteGroups[spi->group].bbox;
			if (spgBbox.isValidRect() && spi->lastRect.intersects(spgBbox)) {
				spi->lastRect.clip(spgBbox);
				wiz.actionFlags |= kWAFRect;
				wiz.box = spi->lastRect;
			} else {
				bboxPtr->left = 1234;
				bboxPtr->top = 1234;
				bboxPtr->right = -1234;
				bboxPtr->bottom = -1234;
				continue;
			}
		}
		if (spi->palette) {
			wiz.actionFlags |= kWAFPalette;
			wiz.palette = spi->palette;
		}
		if (spi->image && spi->group && _spriteGroups[spi->group].image) {
			wiz.actionFlags |= kWAFDestImage;
			wiz.destImageNumber = _spriteGroups[spi->group].image;
		}
		_vm->_wiz->processWizImageDrawCmd(&wiz);
	}
}

static void syncWithSerializer(Common::Serializer &s, SpriteInfo &si) {
	s.syncAsSint32LE(si.id, VER(48));
	s.syncAsSint32LE(si.combinedPriority, VER(48));
	s.syncAsSint32LE(si.flags, VER(48));
	s.syncAsSint32LE(si.image, VER(48));
	s.syncAsSint32LE(si.state, VER(48));
	s.syncAsSint32LE(si.group, VER(48));
	s.syncAsSint32LE(si.palette, VER(48));
	s.syncAsSint32LE(si.priority, VER(48));
	s.syncAsSint32LE(si.lastRect.left, VER(48));
	s.syncAsSint32LE(si.lastRect.top, VER(48));
	s.syncAsSint32LE(si.lastRect.right, VER(48));
	s.syncAsSint32LE(si.lastRect.bottom, VER(48));
	s.syncAsSint32LE(si.deltaPosX, VER(48));
	s.syncAsSint32LE(si.deltaPosY, VER(48));
	s.syncAsSint32LE(si.lastSpot.x, VER(48));
	s.syncAsSint32LE(si.lastSpot.y, VER(48));
	s.syncAsSint32LE(si.posX, VER(48));
	s.syncAsSint32LE(si.posY, VER(48));
	s.syncAsSint32LE(si.userValue, VER(48));
	s.syncAsSint32LE(si.lastState, VER(48));
	s.syncAsSint32LE(si.lastImage, VER(48));
	s.syncAsSint32LE(si.imageList, VER(48));
	s.syncAsSint32LE(si.shadow, VER(48));
	s.syncAsSint32LE(si.maxStates, VER(48));
	s.syncAsSint32LE(si.angle, VER(48));
	s.syncAsSint32LE(si.scale, VER(48));
	s.syncAsSint32LE(si.animState, VER(48));
	s.syncAsSint32LE(si.lastAngle, VER(48));
	s.syncAsSint32LE(si.lastScale, VER(48));
	s.syncAsSint32LE(si.lastRenderFlags, VER(48));
	s.syncAsSint32LE(si.animIndex, VER(48));
	s.syncAsSint32LE(si.animSpeed, VER(48));
	s.syncAsSint32LE(si.sourceImage, VER(48));
	s.syncAsSint32LE(si.maskImage, VER(48));
	s.syncAsSint32LE(si.zbufferImage, VER(48));
	s.syncAsSint32LE(si.classFlags, VER(48));
	s.syncAsSint32LE(si.specialRenderFlags, VER(48));
	s.syncAsSint32LE(si.conditionBits, VER(48));
}

static void syncWithSerializer(Common::Serializer &s, SpriteGroup &sg) {
	s.syncAsSint32LE(sg.bbox.left, VER(48));
	s.syncAsSint32LE(sg.bbox.top, VER(48));
	s.syncAsSint32LE(sg.bbox.right, VER(48));
	s.syncAsSint32LE(sg.bbox.bottom, VER(48));
	s.syncAsSint32LE(sg.priority, VER(48));
	s.syncAsSint32LE(sg.flags, VER(48));
	s.syncAsSint32LE(sg.tx, VER(48));
	s.syncAsSint32LE(sg.ty, VER(48));
	s.syncAsSint32LE(sg.image, VER(48));
	s.syncAsSint32LE(sg.scaling, VER(48));
	s.syncAsSint32LE(sg.scale_x_ratio_mul, VER(48));
	s.syncAsSint32LE(sg.scale_x_ratio_div, VER(48));
	s.syncAsSint32LE(sg.scale_y_ratio_mul, VER(48));
	s.syncAsSint32LE(sg.scale_y_ratio_div, VER(48));
}

void Sprite::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.getVersion() >= VER(64)) {
		s.syncArray(_spriteTable, _varNumSprites + 1, syncWithSerializer);
		s.syncArray(_spriteGroups, _varNumSpriteGroups + 1, syncWithSerializer);
	} else {
		// TODO: This had been bogus, what is it really supposed to do?
//		s->saveLoadArrayOf(_activeSpritesTable, _varNumSprites, sizeof(_activeSpritesTable[0]), spriteEntries);
		s.syncArray(*_activeSpritesTable, _varNumSprites, syncWithSerializer);
		s.syncArray(_spriteTable, _varNumSprites, syncWithSerializer);
		s.syncArray(_spriteGroups, _varNumSpriteGroups, syncWithSerializer);
	}

	// Reset active sprite table
	if (s.isLoading())
		_numSpritesToProcess = 0;
}

} // End of namespace Scumm

#endif // ENABLE_HE
