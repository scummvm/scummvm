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
	_groupTable(0),
	_spriteTable(0),
	_activeSprites(0),
	_activeSpriteCount(0),
	_maxSpriteGroups(0),
	_maxSprites(0),
	_maxImageLists(0) {
}

Sprite::~Sprite() {
	free(_groupTable);
	free(_spriteTable);
	free(_activeSprites);
}

void ScummEngine_v90he::allocateArrays() {
	ScummEngine_v70he::allocateArrays();
	_sprite->initializeStuff(_numSprites, MAX(64, _numSprites / 4), 64);
}

void Sprite::getSpriteLogicalRect(int spriteId, bool checkGroup, Common::Rect &bound) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (_vm->_game.heversion <= 98) {
		int32 x, y, w, h;
		Common::Point spot;

		int image = _spriteTable[spriteId].image;

		if (image != 0) {
			int state = _spriteTable[spriteId].state;
			_vm->_wiz->getWizSpot(image, state, x, y);
			_vm->_wiz->getWizImageDim(image, state, w, h);

			spot.x = (int16)(_spriteTable[spriteId].posX - spot.x);
			spot.y = (int16)(_spriteTable[spriteId].posY - spot.y);

			bound.left = spot.x;
			bound.top = spot.y;
			bound.right = spot.x + (int16)w - 1,
			bound.bottom = spot.y + (int16)h - 1;

		} else {
			bound.left = 1234;
			bound.top = 1234;
			bound.right = -1234;
			bound.bottom = -1234;
		}
	} else {
		int32 x, y;
		calcSpriteSpot(&_spriteTable[spriteId], false, x, y);

		// Let's hope it doesn't overflow...
		Common::Point spot((int16)x, (int16)y);

		getSpriteRectPrim(&_spriteTable[spriteId], &bound, false, &spot);
	}
}

//
// spriteInfoGet functions
//
int Sprite::spriteFromPoint(int x, int y, int groupCheck, int quickCheck, int classCount, int *classCheckTable) {
	SpriteInfo **spritePtr;
	const Common::Rect *r;
	int image;

	if (!_activeSpriteCount)
		return 0;

	spritePtr = &_activeSprites [_activeSpriteCount - 1];

	if (quickCheck != 0) {
		for (int counter = 0; counter < _activeSpriteCount; counter++, spritePtr--) {
			if (_vm->_game.heversion > 90 && groupCheck != 0) {
				if (groupCheck != (*spritePtr)->group)
					continue;
			}

			if (_vm->_game.heversion >= 99 && classCount != 0) {
				if (!checkSpriteClassAgaintClassSet(((*spritePtr) - _spriteTable), classCount, classCheckTable)) {
					continue;
				}
			}

			r = &(*spritePtr)->lastRect;
			image = (*spritePtr)->lastImage;
			if ((!image) || (!_vm->_wiz->isRectValid(*r)))
				continue;

			if ((r->left > x) || (r->top > y) || (r->right < x) || (r->bottom < y)) {
				continue;
			}

			return ((*spritePtr) - _spriteTable);
		}
	} else {
		for (int counter = 0; counter < _activeSpriteCount; counter++, spritePtr--) {
			r = &(*spritePtr)->lastRect;
			image = (*spritePtr)->lastImage;

			if ((!image))
				continue;

			if (_vm->_game.heversion > 90 && groupCheck) {
				if (groupCheck != (*spritePtr)->group)
					continue;
			}

			if (_vm->_game.heversion >= 99 && classCount) {
				if (!checkSpriteClassAgaintClassSet(((*spritePtr) - _spriteTable), classCount, classCheckTable)) {
					continue;
				}
			}

			if (_vm->_game.heversion > 99) {
				int state = 0;
				int32 testPointX, testPointY;
				if ((*spritePtr)->maskImage) {
					int32 maskSpotX, maskSpotY, imageSpotX, imageSpotY;
					int maskStateCount;

					// Change to using the the mask image instead of the display image
					image = (*spritePtr)->maskImage;

					// Get the state for the mask (wrap if necessary)
					maskStateCount = _vm->_wiz->getWizStateCount(image);
					state = ((*spritePtr)->lastState % maskStateCount);

					// Convert the coords to "image" relative coords.
					testPointX = (x - (*spritePtr)->lastSpot.x);
					testPointY = (y - (*spritePtr)->lastSpot.y);

					// Get the spot's so we can adjust the "lastSpot" to
					// be relative to the mask's link point...
					_vm->_wiz->getWizSpot((*spritePtr)->lastImage, state, imageSpotX, imageSpotY);
					_vm->_wiz->getWizSpot(image, state, maskSpotX, maskSpotY);

					// Convert the coords to "Mask" relative coords.
					testPointX += (maskSpotX - imageSpotX);
					testPointY += (maskSpotY - imageSpotY);
				} else {
					r = &(*spritePtr)->lastRect;

					if (!_vm->_wiz->isRectValid(*r) || (r->left > x) || (r->top > y) || (r->right < x) || (r->bottom < y)) {
						continue;
					}

					// Convert the coords to image relative coords.
					testPointX = (x - (*spritePtr)->lastSpot.x);
					testPointY = (y - (*spritePtr)->lastSpot.y);

					// Get the last active image state
					state = (*spritePtr)->lastState;
				}

				if (!_vm->_wiz->hitTestWiz(image, state, testPointX, testPointY, (*spritePtr)->lastRenderFlags)) {
					continue;
				}

				return ((*spritePtr) - _spriteTable);
			} else {
				if (!_vm->_wiz->isRectValid(*r))
					continue;

				if ((r->left > x) || (r->top > y) || (r->right < x) || (r->bottom < y))
					continue;

				int32 state = _vm->_game.heversion > 80 ? (*spritePtr)->lastState : 0; // HE80 doesn't have states

				if (!_vm->_wiz->hitTestWiz(image, state, (x - (*spritePtr)->lastSpot.x), (y - (*spritePtr)->lastSpot.y), 0)) {
					continue;
				}

				return ((*spritePtr) - _spriteTable);
			}
		}
	}

	return 0;
}

int Sprite::getSpriteClass(int spriteId, int classId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (classId == -1) {
		return _spriteTable[spriteId].classFlags;
	} else {
		assertRange(1, classId, 32, "sprite");
		return _spriteTable[spriteId].classFlags & (1 << (classId - 1));
	}
}

int Sprite::checkSpriteClassAgaintClassSet(int sprite, int classCount, int *classCheckTable) {
	int classId, classBit;

	while (classCount--) {
		classId = classCheckTable[classCount];
		classBit = getSpriteClass(sprite, (classId & 0x7f));

		if (((classId & 0x80) == 0x80) && (classBit == 0))
			return 0;

		if (((classId & 0x80) == 0x00) && (classBit != 0))
			return 0;
	}

	return 1;
}

int Sprite::getSpriteRenderToBackground(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFBackgroundRender) != 0) ? 1 : 0;
}

int Sprite::getSpriteVertFlip(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFVFlip) != 0) ? 1 : 0;
}

int Sprite::getSpriteHorzFlip(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFHFlip) != 0) ? 1 : 0;
}

int Sprite::getSpriteActiveFlag(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFActive) != 0) ? 1 : 0;
}

int Sprite::getSpriteImageRemapFlag(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFUseImageRemap) != 0) ? 1 : 0;
}

int Sprite::getSpriteAutoAnimFlag(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return (_spriteTable[spriteId].flags & kSFAutoAnimate);
}

int Sprite::getSpriteUpdateType(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");
	if (_vm->_game.heversion > 99) {
		if (_spriteTable[spriteId].flags & kSFSmartRender) {
			return SPRDEF_SMART;
		} else {
			return (_spriteTable[spriteId].flags & kSFDontCombineErase) ? SPRDEF_SIMPLE : SPRDEF_DUMB;
		}
	} else {
		return ((_spriteTable[spriteId].flags & kSFSmartRender) != 0) ? 1 : 0;
	}

}

int Sprite::getSpriteEraseType(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return ((_spriteTable[spriteId].flags & kSFIgnoreErase) != 0) ? 0 : 1;
}

int Sprite::getSpriteImage(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].image;
}

int Sprite::getSpriteImageState(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].state;
}

int Sprite::getSpriteGroup(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].group;
}

int Sprite::getSpritePalette(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].palette;
}

int Sprite::getSpritePriority(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].priority;
}

int Sprite::getSpriteDisplayX(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (_spriteTable[spriteId].group)
		return _spriteTable[spriteId].posX + _groupTable[_spriteTable[spriteId].group].posX;
	else
		return _spriteTable[spriteId].posX;
}

int Sprite::getSpriteDisplayY(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (_spriteTable[spriteId].group)
		return _spriteTable[spriteId].posY + _groupTable[_spriteTable[spriteId].group].posY;
	else
		return _spriteTable[spriteId].posY;
}

int Sprite::getUserValue(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].userValue;
}

int Sprite::getSpriteShadow(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].shadow;
}

int Sprite::getSpriteImageStateCount(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].maxStates;
}

int Sprite::getSpriteScale(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].scale;
}

int Sprite::getSpriteAnimSpeed(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].animSpeed;
}

int Sprite::getSourceImage(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].sourceImage;
}

int Sprite::getMaskImage(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	return _spriteTable[spriteId].maskImage;
}

int Sprite::getSpriteGeneralProperty(int spriteId, int type) {
	debug(7, "getSpriteGeneralProperty: spriteId %d type 0x%x", spriteId, type);
	assertRange(1, spriteId, _maxSprites, "sprite");

	// TODO U32

	switch (type) {
	case SPRPROP_SPECIAL_RENDER_FLAGS:
		return _spriteTable[spriteId].specialRenderFlags;
	case SPRPROP_CONDITION_BITS:
		return _spriteTable[spriteId].conditionBits;
	case SPRPROP_ANIMATION_SUB_STATE:
		return getSpriteAnimSpeedState(spriteId);
	default:
		error("getSpriteGeneralProperty: Invalid type %d", type);
	}
}

int Sprite::getDestImageForSprite(const SpriteInfo *spritePtr) {
	if (spritePtr->image) {
		if (spritePtr->group) {
			return _groupTable[spritePtr->group].image;
		}
	}

	return 0;
}

int Sprite::getSpriteAnimSpeedState(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");
	return _spriteTable[spriteId].animState;
}

void Sprite::getSpriteImageDim(int spriteId, int32 &w, int32 &h) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (_spriteTable[spriteId].image) {
		_vm->_wiz->getWizImageDim(_spriteTable[spriteId].image, _spriteTable[spriteId].state, w, h);
	} else {
		w = 0;
		h = 0;
	}
}

void Sprite::getSpritePosition(int spriteId, int32 &tx, int32 &ty) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	tx = _spriteTable[spriteId].posX;
	ty = _spriteTable[spriteId].posY;
}

void Sprite::getSpriteRectPrim(const SpriteInfo *spritePtr, Common::Rect *rectPtr, bool includeGroupTransform, const Common::Point *spotPtr) {
	bool angleSpecified, scaleSpecified;
	int state, scale, angle;
	int32 x, y;
	int image = spritePtr->image;

	if (image != 0) {
		Common::Point tmpPt(spotPtr->x, spotPtr->y);

		if (_vm->_game.heversion < 100) {
			calcSpriteSpot(spritePtr, includeGroupTransform, x, y);
			tmpPt.x = x;
			tmpPt.y = y;
		}

		state = spritePtr->state;
		angle = spritePtr->angle;
		scale = spritePtr->scale;

		scaleSpecified = (kSFScaleSpecified & spritePtr->flags);
		angleSpecified = (kSFAngleSpecified & spritePtr->flags);

		if (angleSpecified || scaleSpecified) {
			Common::Point listOfPoints[4];
			int w, h;

			_vm->_wiz->getWizImageDim(image, state, w, h);

			listOfPoints[0].x = -w / 2;
			listOfPoints[0].y = -h / 2;

			listOfPoints[1].x = listOfPoints[0].x + w - 1;
			listOfPoints[1].y = listOfPoints[0].y;

			listOfPoints[2].x = listOfPoints[1].x;
			listOfPoints[2].y = listOfPoints[0].y + h - 1;

			listOfPoints[3].x = listOfPoints[0].x;
			listOfPoints[3].y = listOfPoints[2].y;

			if (scaleSpecified) {
				for (int i = 0; i < 4; i++) {
					listOfPoints[i].x = (scale * listOfPoints[i].x) / 256;
					listOfPoints[i].y = (scale * listOfPoints[i].y) / 256;
				}
			}

			// Rotate the points
			if (angleSpecified) {
				_vm->_wiz->polyRotatePoints(listOfPoints, 4, angle);
			}

			// Offset the points
			_vm->_wiz->polyMovePolygonPoints(listOfPoints, 4, tmpPt.x, tmpPt.y);

			// Finally get down the point and get the bounding rect of this
			_vm->_wiz->polyBuildBoundingRect(listOfPoints, 4, *rectPtr);

		} else {
			int w, h;
			_vm->_wiz->getWizImageDim(image, state, w, h);

			rectPtr->left = tmpPt.x;
			rectPtr->top = tmpPt.y;
			rectPtr->right = tmpPt.x + w - 1;
			rectPtr->bottom = tmpPt.y + h - 1;
		}

	} else {
		rectPtr->left = 1234;
		rectPtr->top = 1234;
		rectPtr->right = -1234;
		rectPtr->bottom = -1234;
	}
}

void Sprite::getDelta(int spriteId, int32 &dx, int32 &dy) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	dx = _spriteTable[spriteId].deltaPosX;
	dy = _spriteTable[spriteId].deltaPosY;
}

void Sprite::calcSpriteSpot(const SpriteInfo *spritePtr, bool includeGroupTransform, int32 &x, int32 &y) {
	if (_vm->_game.heversion <= 90) {
		if (_vm->_game.heversion == 80) {
			_vm->_wiz->getWizSpot(spritePtr->image, x, y);
		} else {
			_vm->_wiz->getWizSpot(spritePtr->image, spritePtr->state, x, y);
		}

		x = spritePtr->posX - x;
		y = spritePtr->posY - y;

		if (spritePtr->group != 0) {
			x += _groupTable[spritePtr->group].posX;
			y += _groupTable[spritePtr->group].posY;
		}

	} else if (_vm->_game.heversion >= 98) {
		if (_vm->_game.heversion >= 100) {
			if (spritePtr->image == 0) {
				if (includeGroupTransform && spritePtr->group) {
					x = _groupTable[spritePtr->group].posX;
					y = _groupTable[spritePtr->group].posY;
				} else {
					x = 0;
					y = 0;
				}

				return;
			}
		}

		_vm->_wiz->getWizSpot(spritePtr->image, spritePtr->state, x, y);

		int group = spritePtr->group;

		if ((includeGroupTransform || _vm->_game.heversion == 98) && group != 0) {
			if (_groupTable[group].isScaled) {
				x = (int)((float)spritePtr->posX * _groupTable[group].xScale) - x;
				y = (int)((float)spritePtr->posY * _groupTable[group].yScale) - y;

				x += _groupTable[group].posX;
				y += _groupTable[group].posY;
			} else {
				x = spritePtr->posX - x;
				y = spritePtr->posY - y;
				x += _groupTable[group].posX;
				y += _groupTable[group].posY;
			}
		} else {
			x = spritePtr->posX - x;
			y = spritePtr->posY - y;
		}
	}
}

//
// spriteGroupGet functions
//
int ScummEngine_v90he::getGroupSpriteArray(int spriteGroupId) {
	int i, numSprites = 0;

	assertRange(1, spriteGroupId, _sprite->_maxSpriteGroups, "sprite group");

	for (i = (_sprite->_maxSprites - 1); i > 0; i--) {
		if (_sprite->_spriteTable[i].group == spriteGroupId)
			numSprites++;
	}

	if (!numSprites)
		return 0;

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, numSprites);
	writeArray(0, 0, 0, numSprites);

	numSprites = 1;
	for (i = (_sprite->_maxSprites - 1); i > 0; i--) {
		if (_sprite->_spriteTable[i].group == spriteGroupId) {
			writeArray(0, 0, numSprites, i);
			numSprites++;
		}
	}

	return readVar(0);
}

int Sprite::getGroupPriority(int spriteGroupId) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	return _groupTable[spriteGroupId].priority;
}

int Sprite::getGroupImage(int spriteGroupId) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	return _groupTable[spriteGroupId].image;
}

int Sprite::getGroupXMul(int spriteGroupId) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	return _groupTable[spriteGroupId].xMul;
}

int Sprite::getGroupXDiv(int spriteGroupId) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	return _groupTable[spriteGroupId].xDiv;
}

int Sprite::getGroupYMul(int spriteGroupId) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	return _groupTable[spriteGroupId].yMul;
}

int Sprite::getGroupYDiv(int spriteGroupId) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	return _groupTable[spriteGroupId].yDiv;
}

void Sprite::getGroupPoint(int spriteGroupId, int32 &tx, int32 &ty) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	tx = _groupTable[spriteGroupId].posX;
	ty = _groupTable[spriteGroupId].posY;
}

//
// spriteInfoSet functions
//
void Sprite::setSpritePalette(int spriteId, int value) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (_spriteTable[spriteId].palette != value) {
		_spriteTable[spriteId].palette = value;
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
	}
}

void Sprite::setSourceImage(int spriteId, int value) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (_spriteTable[spriteId].sourceImage != value) {
		_spriteTable[spriteId].sourceImage = value;
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
	}
}

void Sprite::setMaskImage(int spriteId, int value) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	_spriteTable[spriteId].maskImage = value;
}

void Sprite::setSpriteImageState(int spriteId, int state) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (_spriteTable[spriteId].image) {
		int imageStateCount = _spriteTable[spriteId].maxStates - 1;
		state = MAX(0, MIN(state, imageStateCount));

		if (_spriteTable[spriteId].state != state) {
			_spriteTable[spriteId].state = state;
			_spriteTable[spriteId].flags |= kSFErase | kSFRender;
		}
	}
}

void Sprite::setSpritePosition(int spriteId, int tx, int ty) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	int32 oldX, oldY;
	oldX = _spriteTable[spriteId].posX;
	oldY = _spriteTable[spriteId].posY;

	_spriteTable[spriteId].posX = tx;
	_spriteTable[spriteId].posY = ty;

	if (oldX != tx || oldY != ty) {
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
	}
}

void Sprite::setSpriteGroup(int spriteId, int group) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (group != 0)
		assertRange(1, group, _maxSpriteGroups, "sprite group");

	_spriteTable[spriteId].group = group;
	_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setDelta(int spriteId, int dx, int dy) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	_spriteTable[spriteId].deltaPosX = dx;
	_spriteTable[spriteId].deltaPosY = dy;
}

void Sprite::setSpriteShadow(int spriteId, int shadow) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	_spriteTable[spriteId].shadow = shadow;
	if (_spriteTable[spriteId].image)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setUserValue(int spriteId, int value1, int value2) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	_spriteTable[spriteId].userValue = value2;
}

void Sprite::setSpritePriority(int spriteId, int priority) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	_spriteTable[spriteId].priority = priority;
}

void Sprite::moveSprite(int spriteId, int dx, int dy) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	int32 oldX, oldY;
	oldX = _spriteTable[spriteId].posX;
	oldY = _spriteTable[spriteId].posY;

	_spriteTable[spriteId].posX += dx;
	_spriteTable[spriteId].posY += dy;

	if ((oldX != _spriteTable[spriteId].posX) || (oldY != _spriteTable[spriteId].posY))
		_spriteTable[spriteId].flags |= (kSFErase | kSFRender);
}

void Sprite::setSpriteScale(int spriteId, int scale) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	_spriteTable[spriteId].flags |= kSFScaleSpecified;

	if (_spriteTable[spriteId].scale != scale) {
		_spriteTable[spriteId].scale = scale;

		if (_spriteTable[spriteId].image)
			_spriteTable[spriteId].flags |= (kSFErase | kSFRender);
	}
}

void Sprite::setSpriteAngle(int spriteId, int angle) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	_spriteTable[spriteId].flags |= kSFAngleSpecified;

	if (_spriteTable[spriteId].angle != angle) {
		_spriteTable[spriteId].angle = angle;

		if (_spriteTable[spriteId].image)
			_spriteTable[spriteId].flags |= (kSFErase | kSFRender);
	}
}

void Sprite::setSpriteRenderToBackground(int spriteId, int value) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value) {
		_spriteTable[spriteId].flags |= kSFBackgroundRender;
	} else {
		_spriteTable[spriteId].flags &= ~kSFBackgroundRender;
	}

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteVertFlip(int spriteId, int flip) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	int oldFlags = _spriteTable[spriteId].flags;
	if (flip) {
		_spriteTable[spriteId].flags |= kSFVFlip;
	} else {
		_spriteTable[spriteId].flags &= ~kSFVFlip;
	}

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteHorzFlip(int spriteId, int flip) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	int oldFlags = _spriteTable[spriteId].flags;
	if (flip) {
		_spriteTable[spriteId].flags |= kSFHFlip;
	} else {
		_spriteTable[spriteId].flags &= ~kSFHFlip;
	}

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteActiveFlag(int spriteId, int value) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (value) {
		_spriteTable[spriteId].flags |= kSFActive;
	} else {
		_spriteTable[spriteId].flags &= ~kSFActive;
	}
}

void Sprite::setSpriteImageRemapFlag(int spriteId, int value) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	int oldFlags = _spriteTable[spriteId].flags;
	if (value) {
		_spriteTable[spriteId].flags |= kSFUseImageRemap;
	} else {
		_spriteTable[spriteId].flags &= ~kSFUseImageRemap;
	}

	if (_spriteTable[spriteId].image && _spriteTable[spriteId].flags != oldFlags)
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
}

void Sprite::setSpriteAutoAnimFlag(int spriteId, int value) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (value) {
		_spriteTable[spriteId].flags |= kSFAutoAnimate;
	} else {
		_spriteTable[spriteId].flags &= ~kSFAutoAnimate;
	}
}

void Sprite::setSpriteUpdateType(int spriteId, int eraseType) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	if (_vm->_game.heversion > 99) {
		switch (eraseType) {
		default:
		case SPRDEF_SMART:
			_spriteTable[spriteId].flags |= kSFSmartRender | kSFDontCombineErase;
			break;
		case SPRDEF_DUMB:
			_spriteTable[spriteId].flags &= ~(kSFSmartRender | kSFDontCombineErase);
			break;
		case SPRDEF_SIMPLE:
			_spriteTable[spriteId].flags &= ~(kSFSmartRender);
			_spriteTable[spriteId].flags |= kSFDontCombineErase;
			break;
		}
	} else {
		if (eraseType != 0) {
			_spriteTable[spriteId].flags |= (kSFSmartRender | kSFDontCombineErase);
		} else {
			_spriteTable[spriteId].flags &= ~(kSFSmartRender | kSFDontCombineErase);
		}
	}

}

void Sprite::setSpriteEraseType(int spriteId, int eraseType) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	// Note that condition is inverted
	if (!eraseType) {
		_spriteTable[spriteId].flags |= kSFIgnoreErase;
	} else {
		_spriteTable[spriteId].flags &= ~kSFIgnoreErase;
	}
}

void Sprite::setSpriteAnimSpeed(int spriteId, int value) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	_spriteTable[spriteId].animSpeed = value;
	_spriteTable[spriteId].animState = value;
}

void Sprite::setSpriteClass(int spriteId, int classId, int toggle) {
	assertRange(1, spriteId, _maxSprites, "sprite");
	assertRange(1, classId, 32, "class");

	if (toggle) {
		_spriteTable[spriteId].classFlags |= (1 << (classId - 1));
	} else {
		_spriteTable[spriteId].classFlags &= ~(1 << (classId - 1));
	}
}

void Sprite::clearSpriteClasses(int spriteId) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	_spriteTable[spriteId].classFlags = 0;
}

void Sprite::setSpriteZBuffer(int spriteId, int value) {
	assertRange(1, spriteId, _maxSprites, "sprite");

	_spriteTable[spriteId].zbufferImage = value;
}

void Sprite::setSpriteAnimSpeedState(int spriteId, int animState) {
	assertRange(1, spriteId, _maxSprites, "sprite");
	_spriteTable[spriteId].animState = MAX(0, MIN(animState, _spriteTable[spriteId].animSpeed));
}

void Sprite::setSpriteGeneralProperty(int spriteId, int property, int value) {
	// TODO U32
	debug(7, "setSpriteGeneralProperty: spriteId %d type 0x%x value 0x%x", spriteId, property, value);
	assertRange(1, spriteId, _maxSprites, "sprite");

	switch (property) {
	case SPRPROP_SPECIAL_RENDER_FLAGS:
		_spriteTable[spriteId].specialRenderFlags = value;
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
		break;
	case SPRPROP_CONDITION_BITS:
		_spriteTable[spriteId].conditionBits = value;
		_spriteTable[spriteId].flags |= kSFErase | kSFRender;
		break;
	case SPRPROP_ANIMATION_SUB_STATE:
		setSpriteAnimSpeedState(spriteId, value);
		break;
	default:
		warning("Sprite::setSpriteGeneralProperty(): Unknown sprite property %d", property);
	}
}

void Sprite::newSprite(int sprite) {
	assertRange(1, sprite, _maxSprites, "sprite");

	_spriteTable[sprite].angle = 0;
	_spriteTable[sprite].scale = 0;

	setSpriteImage(sprite, 0);
	setSpriteShadow(sprite, 0);
	setSpritePosition(sprite, 0, 0);
	setSpriteHorzFlip(sprite, 0);
	setSpriteVertFlip(sprite, 0);
	setDelta(sprite, 0, 0);
	setUserValue(sprite, 0, 0);
	setSpriteGroup(sprite, 0);
	setSpriteAnimSpeed(sprite, 0);
	clearSpriteClasses(sprite);

	if (_vm->_game.heversion > 98) {
		setSpritePalette(sprite, 0);
		setSourceImage(sprite, 0);
		setMaskImage(sprite, 0);
	}

	if (_vm->_game.heversion > 99) {
		setSpriteUpdateType(sprite, SPRDEF_SIMPLE);
		setSpritePriority(sprite, 0);
		setSpriteZBuffer(sprite, 0);

		_spriteTable[sprite].flags |= kSFAutoAnimate;
		_spriteTable[sprite].conditionBits = 0;

		_spriteTable[sprite].specialRenderFlags = 0;
		// TODO U32 PU_SpriteNewHook(sprite);
	}
}

void Sprite::setImageList(int sprite, int count, const int *list) {
	SpriteImageList *imageListPtr;
	int counter;

	assertRange(1, sprite, _maxSprites, "sprite");

	// Release the last image list if one...
	if (_spriteTable[sprite].imageList) {
		releaseImageList(_spriteTable[sprite].imageList);
		_spriteTable[sprite].imageList = 0;
	}

	// HE90+
	int32 lastMaxStates = _spriteTable[sprite].maxStates;
	int32 lastImage = _spriteTable[sprite].image;

	if (count == 1) {
		_spriteTable[sprite].image = *list;
	} else {
		_spriteTable[sprite].imageList = getFreeImageList(count);
		imageListPtr = getImageListPtr(_spriteTable[sprite].imageList);
		imageListPtr->count = count;

		for (counter = 0; counter < count; counter++) {
			imageListPtr->list[counter] = *list++;
		}

		_spriteTable[sprite].image = imageListPtr->list[0];
	}

	_spriteTable[sprite].animIndex = 0;
	_spriteTable[sprite].state = 0; // HE90+

	if (_spriteTable[sprite].image) {
		if (_vm->_game.heversion == 80) {
			_spriteTable[sprite].flags |=
				(kSFActive | kSFAutoAnimate |
				 kSFRender | kSFErase);
		} else {
			_spriteTable[sprite].maxStates = _vm->_wiz->getWizStateCount(_spriteTable[sprite].image);

			if (_vm->_game.heversion < 100) {
				_spriteTable[sprite].flags |= kSFDefaultFlagActive;
			} else {
				if (_vm->VAR(_vm->VAR_SPRITE_IMAGE_CHANGE_DOES_NOT_RESET_SETTINGS)) {
					_spriteTable[sprite].flags |= kSFActive;
				} else {
					_spriteTable[sprite].flags |= kSFDefaultFlagActive;
				}
			}

			if (_vm->_game.heversion > 80 &&
				(lastImage != _spriteTable[sprite].image) || (_spriteTable[sprite].maxStates != lastMaxStates)) {
				_spriteTable[sprite].flags |= (kSFRender | kSFErase);
			}
		}

	} else {
		if (_vm->_game.heversion >= 95 && _vm->_game.heversion < 100) {
			if (_spriteTable[sprite].flags & kSFIgnoreErase) {
				_spriteTable[sprite].flags = 0;
			} else {
				_spriteTable[sprite].flags = kSFDefaultFlagInactive;
			}
		} else if (_vm->_game.heversion >= 100) {
			if (_vm->VAR(_vm->VAR_SPRITE_IMAGE_CHANGE_DOES_NOT_RESET_SETTINGS)) {
				_spriteTable[sprite].flags &= ~kSFActive;
			} else {
				if (lastImage) {
					if (_spriteTable[sprite].flags & kSFIgnoreErase) {
						_spriteTable[sprite].flags = 0;
					} else {
						_spriteTable[sprite].flags = kSFDefaultFlagInactive;
					}
				} else {
					_spriteTable[sprite].flags = 0;
				}
			}
		} else {
			_spriteTable[sprite].flags = (_vm->_game.heversion == 80 ? kSFErase : kSFDefaultFlagInactive);
		}

		_spriteTable[sprite].lastImage = 0;

		// HE90+
		_spriteTable[sprite].lastState = 0;
		_spriteTable[sprite].maxStates = 0;
	}
}
void Sprite::setSpriteImage(int sprite, int imageNum) {
	setImageList(sprite, 1, &imageNum);
}

//
// spriteGroupSet functions
//
void Sprite::orInGroupMembersFlags(int spriteGroupId, int32 flags) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	for (int i = 0; i < _activeSpriteCount; ++i) {
		SpriteInfo *spi = _activeSprites[i];
		if (spi->group == spriteGroupId) {
			spi->flags |= flags;
		}
	}
}

void Sprite::moveGroupMembers(int spriteGroupId, int value1, int value2) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	for (int i = 1; i < _maxSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].posX += value1;
			_spriteTable[i].posY += value2;

			if (value1 || value2)
				_spriteTable[i].flags |= kSFErase | kSFRender;
		}
	}
}

void Sprite::setGroupMembersPriority(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	for (int i = 1; i < _maxSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId)
			_spriteTable[i].priority = value;
	}
}

void Sprite::changeGroupMembersGroup(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	for (int i = 1; i < _maxSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].group = value;
			_spriteTable[i].flags |= kSFErase | kSFRender;
		}
	}
}

void Sprite::setGroupMembersUpdateType(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	for (int i = 1; i < _maxSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId)
			setSpriteUpdateType(i, value);
	}
}

void Sprite::performNewOnGroupMembers(int spriteGroupId) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	for (int i = 1; i < _maxSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId)
			newSprite(i);
	}
}

void Sprite::setGroupMembersAnimationSpeed(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	for (int i = 1; i < _maxSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].animSpeed = value;
			_spriteTable[i].animState = value;
		}
	}
}

void Sprite::setGroupMembersAutoAnimFlag(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	for (int i = 1; i < _maxSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			if (value)
				_spriteTable[i].flags |= kSFAutoAnimate;
			else
				_spriteTable[i].flags &= ~kSFAutoAnimate;
		}
	}
}

void Sprite::setGroupMembersShadow(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	for (int i = 1; i < _maxSprites; i++) {
		if (_spriteTable[i].group == spriteGroupId) {
			_spriteTable[i].shadow = value;
			if (_spriteTable[i].image)
				_spriteTable[i].flags |= kSFErase | kSFRender;
		}
	}
}

void Sprite::setGroupClipRect(int spriteGroupId, int x1, int y1, int x2, int y2) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	_groupTable[spriteGroupId].flags |= kSGFUseClipRect;
	_groupTable[spriteGroupId].clipRect.left = x1;
	_groupTable[spriteGroupId].clipRect.top = y1;
	_groupTable[spriteGroupId].clipRect.right = x2;
	_groupTable[spriteGroupId].clipRect.bottom = y2;

	orInGroupMembersFlags(spriteGroupId, (kSFRender | kSFErase));
}

void Sprite::setGroupPriority(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	if (_groupTable[spriteGroupId].priority != value) {
		_groupTable[spriteGroupId].priority = value;
		orInGroupMembersFlags(spriteGroupId, (kSFRender | kSFErase));
	}
}

void Sprite::setGroupPoint(int spriteGroupId, int x, int y) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	if (_groupTable[spriteGroupId].posX != x || _groupTable[spriteGroupId].posY != y) {
		_groupTable[spriteGroupId].posX = x;
		_groupTable[spriteGroupId].posY = y;
		orInGroupMembersFlags(spriteGroupId, (kSFRender | kSFErase));
	}
}

void Sprite::moveGroup(int spriteGroupId, int dx, int dy) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	if (dx || dy) {
		_groupTable[spriteGroupId].posX += dx;
		_groupTable[spriteGroupId].posY += dy;
		orInGroupMembersFlags(spriteGroupId, (kSFRender | kSFErase));
	}
}

void Sprite::setGroupImage(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	if (_groupTable[spriteGroupId].image != value) {
		_groupTable[spriteGroupId].image = value;
		orInGroupMembersFlags(spriteGroupId, (kSFRender | kSFErase));
	}
}

void Sprite::setGroupScaling(int spriteGroupId) {
	if ((_groupTable[spriteGroupId].xMul != _groupTable[spriteGroupId].xDiv) || (_groupTable[spriteGroupId].yMul != _groupTable[spriteGroupId].yDiv))
		_groupTable[spriteGroupId].isScaled = 1;
	else
		_groupTable[spriteGroupId].isScaled = 0;

}

void Sprite::setGroupXMul(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	if (_groupTable[spriteGroupId].xMul != value) {
		_groupTable[spriteGroupId].xMul = value;
		_groupTable[spriteGroupId].xScale = (float)_groupTable[spriteGroupId].xMul / (float)_groupTable[spriteGroupId].xDiv;

		setGroupScaling(spriteGroupId);
		orInGroupMembersFlags(spriteGroupId, (kSFRender | kSFErase));
	}
}

void Sprite::setGroupXDiv(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	if (value == 0)
		error("setGroupXDiv: Divisor must not be 0");

	if (_groupTable[spriteGroupId].xDiv != value) {
		_groupTable[spriteGroupId].xDiv = value;
		_groupTable[spriteGroupId].xScale = (float)_groupTable[spriteGroupId].xMul / (float)_groupTable[spriteGroupId].xDiv;

		setGroupScaling(spriteGroupId);
		orInGroupMembersFlags(spriteGroupId, (kSFRender | kSFErase));
	}
}

void Sprite::setGroupYMul(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	if (_groupTable[spriteGroupId].yMul != value) {
		_groupTable[spriteGroupId].yMul = value;
		_groupTable[spriteGroupId].yScale = (float)_groupTable[spriteGroupId].yMul / (float)_groupTable[spriteGroupId].yDiv;

		setGroupScaling(spriteGroupId);
		orInGroupMembersFlags(spriteGroupId, (kSFRender | kSFErase));
	}
}

void Sprite::setGroupYDiv(int spriteGroupId, int value) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	if (value == 0)
		error("setGroupYDiv: Divisor must not be 0");

	if (_groupTable[spriteGroupId].yDiv != value) {
		_groupTable[spriteGroupId].yDiv = value;
		_groupTable[spriteGroupId].yScale = (float)_groupTable[spriteGroupId].yMul / (float)_groupTable[spriteGroupId].yDiv;

		setGroupScaling(spriteGroupId);
		orInGroupMembersFlags(spriteGroupId, (kSFRender | kSFErase));
	}
}

void Sprite::clearGroupClipRect(int spriteGroupId) {
	assertRange(1, spriteGroupId, _maxSpriteGroups, "sprite group");

	_groupTable[spriteGroupId].flags &= ~(kSGFUseClipRect);
	orInGroupMembersFlags(spriteGroupId, (kSFRender | kSFErase));
}

void Sprite::clearGroupScaleInfo(int group) {
	assertRange(1, group, _maxSpriteGroups, "sprite group");

	_groupTable[group].isScaled = 0;
	_groupTable[group].xScale = 1.0;
	_groupTable[group].xMul = 1;
	_groupTable[group].xDiv = 1;
	_groupTable[group].yScale = 1.0;
	_groupTable[group].yMul = 1;
	_groupTable[group].yDiv = 1;
}

void Sprite::initializeStuff(int spriteCount, int groupCount, int imageListCount) {
	_activeSpriteCount = 0;
	_maxSpriteGroups = groupCount;
	_maxSprites = spriteCount;
	_maxImageLists = imageListCount;

	_groupTable = (SpriteGroup *)malloc((_maxSpriteGroups + 1) * sizeof(SpriteGroup));
	_spriteTable = (SpriteInfo *)malloc((_maxSprites + 1) * sizeof(SpriteInfo));
	_activeSprites = (SpriteInfo **)malloc((_maxSprites + 1) * sizeof(SpriteInfo *));
	_imageLists = (SpriteImageList *)malloc((_maxImageLists + 1) * sizeof(SpriteImageList));
	_imageListStack = (int16 *)malloc((_maxImageLists + 1) * sizeof(int16));

	resetSpriteSystem(false);
}

void Sprite::newGroup(int group) {
	assertRange(1, group, _maxSpriteGroups, "sprite group");

	setGroupPriority(group, 0);
	setGroupPoint(group, 0, 0);
	clearGroupClipRect(group);
	setGroupImage(group, 0);
	clearGroupScaleInfo(group);

	// TODO U32
	// if (_vm->_game.heversion > 99)
	// PU_GroupNewHook(group);
}

void Sprite::resetSpriteSystem(bool refreshScreen) {
	resetImageLists();

	for (int i = 0; i < _maxSprites; i++) {
		_spriteTable[i].reset();
	}

	for (int i = 0; i < _maxSpriteGroups; i++) {
		_groupTable[i].reset();
	}

	if (_vm->_game.heversion >= 98) {
		for (int i = 1; i < _maxSpriteGroups; i++) {
			newGroup(i);
		}
	}

	if (_vm->_game.heversion > 80) {
		if (refreshScreen) {
			_vm->backgroundToForegroundBlit(Common::Rect(_vm->_screenWidth - 1, _vm->_screenHeight - 1));
		}
	} else {
		_vm->_completeScreenRedraw = true;
	}

	_activeSpriteCount = 0;
}

void Sprite::resetImageLists() {
	_imageListStackIndex = _maxImageLists;

	for (int i = 0; i < _maxImageLists; i++) {
		_imageListStack[i] = i;
	}
}

SpriteImageList *Sprite::getImageListPtr(int imageList) {
	assertRange(1, imageList, _maxImageLists, "sprite image list");
	return &_imageLists[imageList - 1];
}

int Sprite::getFreeImageList(int imageCount) {
	if (!_imageListStackIndex) {
		error("Sprite::getFreeImageList(): Out of image lists");
	}

	if (imageCount > 32) {
		error("Sprite::getFreeImageList(): Too many images in image list (%d)!", imageCount);
	}

	--_imageListStackIndex;
	return (_imageListStack[_imageListStackIndex] + 1);
}

void Sprite::releaseImageList(int imageList) {
	assertRange(1, imageList, _maxImageLists, "sprite image list");
	_imageListStack[_imageListStackIndex++] = (imageList - 1);
}

void Sprite::eraseSprites() {
	Common::Rect *lastRectPtr;
	Common::Rect eraseRect;
	SpriteInfo **spritePtr;
	bool first, valid;
	int32 flags;

	eraseRect.left = 1234;
	eraseRect.top = 1234;
	eraseRect.left = -1234;
	eraseRect.bottom = -1234;

	spritePtr = _activeSprites;
	first = true;
	valid = false;

	for (int i = 0; i < _activeSpriteCount; i++) {
		// Get the current flags & clear the erase flag...
		flags = spritePtr[i]->flags;
		spritePtr[i]->flags &= ~kSFErase;

		// Check to see if this sprite should erase it's last rect...
		if (!(flags & kSFErase))
			continue;

		if (_vm->_game.heversion >= 98) {
			if (flags & kSFIgnoreErase)
				continue;
		}

		// Combine the rect with the erase rect...
		lastRectPtr = &spritePtr[i]->lastRect;

		if (_vm->_wiz->isRectValid(*lastRectPtr)) {
			if (_vm->_game.heversion < 90 || !(flags & kSFDontCombineErase)) {
				if (!first) {
					_vm->_wiz->combineRects(&eraseRect, &eraseRect, lastRectPtr);
				} else {
					eraseRect = *lastRectPtr;
					first = false;
				}

				valid = true;
			} else {
				_vm->backgroundToForegroundBlit(*lastRectPtr, USAGE_BIT_RESTORED);
			}

			if (_vm->_game.version >= 90) {
				if (!(flags & (kSFIgnoreErase | kSFRender))) {
					if (spritePtr[i]->image) {
						spritePtr[i]->flags |= kSFRender;
					}
				}
			}
		}
	}

	// Erase the sprites as a bunch, hopefuly this will be faster
	// Also set the bits so that any "actors" that are in the area
	// will also be updated!
	if (valid) {
		_vm->backgroundToForegroundBlit(eraseRect, USAGE_BIT_RESTORED);
	}
}


bool Sprite::doesRectIntersectUpdateAreas(const Common::Rect *rectPtr) {
	int sMin, sMax, y1, y2, minValue;
	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
	int strips = _vm->_gdi->_numStrips;
	int stripsBytes = 8;

	y1 = rectPtr->top;
	y2 = rectPtr->bottom;

	sMin = rectPtr->left / stripsBytes;
	_vm->_gdi->_numStrips;
	sMin = MAX(0, MIN(sMin, (strips - 1)));
	sMax = (rectPtr->right + stripsBytes - 1) / stripsBytes;
	sMax = MAX(0, MIN(sMax, (strips - 1)));

	for (int i = sMin; i <= sMax; i++) {
		minValue = vs->tdirty[i];
		if (minValue >= vs->h)
			continue;

		if (y2 < minValue)
			continue;

		if (y1 > vs->bdirty[i])
			continue;

		return true;
	}

	return false;
}

void Sprite::checkForForcedRedraws(bool checkOnlyPositivePriority) {
	if (_vm->_game.heversion < 90)
		return;

	SpriteInfo **spritePtr;
	int32 flags;

	spritePtr = _activeSprites;

	for (int i = 0; i < _activeSpriteCount; i++) {
		flags = spritePtr[i]->flags;
		if (flags & (kSFRender | kSFIgnoreRender)) {
			continue;
		}

		if (_vm->_game.heversion > 90) {
			if (checkOnlyPositivePriority && (spritePtr[i]->priority < 0)) {
				continue;
			}
		}

		if (kSFSmartRender & flags) {
			if (doesRectIntersectUpdateAreas(&spritePtr[i]->lastRect)) {
				spritePtr[i]->flags |= kSFRender;
			}
		}
	}
}

void Sprite::runSpriteEngines() {
	int lastIndex, index, lastState;
	SpriteImageList *imageListPtr;
	SpriteInfo **spritePtr;
	int32 flags;

	spritePtr = _activeSprites;

	for (int i = 0; i < _activeSpriteCount; i++) {
		// Handle movement
		if (spritePtr[i]->deltaPosX || spritePtr[i]->deltaPosY) {
			moveSprite(
				spritePtr[i] - _spriteTable,
				spritePtr[i]->deltaPosX, spritePtr[i]->deltaPosY);
		}

		// Handle animation...
		if ((flags = spritePtr[i]->flags) & kSFAutoAnimate) {
			// Check to see if the speed is set!
			if (_vm->_game.heversion >= 95) {
				if (spritePtr[i]->animSpeed) {
					if (--spritePtr[i]->animState) {
						continue;
					}

					spritePtr[i]->animState = spritePtr[i]->animSpeed;
				}
			}

			// Handle "state" animation...
			lastState = spritePtr[i]->state;
			spritePtr[i]->state++;

			if ((spritePtr[i]->state >= spritePtr[i]->maxStates)) {
				spritePtr[i]->state = 0;
				if (!spritePtr[i]->imageList) {
					if (lastState) {
						spritePtr[i]->flags |= (kSFRender | kSFErase);
					}
				} else {
					// Handle image list animation...
					if (!(flags & kSFDontAnimImageList)) {
						imageListPtr = getImageListPtr(spritePtr[i]->imageList);
						index = lastIndex = spritePtr[i]->animIndex;

						if ((++index) >= imageListPtr->count) {
							index = 0;
						}

						if (lastIndex != index) {
							spritePtr[i]->animIndex = index;
							spritePtr[i]->image = imageListPtr->list[index];
							spritePtr[i]->flags |= (kSFRender | kSFErase);
							spritePtr[i]->maxStates = _vm->_wiz->getWizStateCount(spritePtr[i]->image);
						}
					}
				}
			} else {
				spritePtr[i]->flags |= (kSFRender | kSFErase);
			}
		}
	}
}

static int compareSpritePriority(const void *a, const void *b) {
	const SpriteInfo *spr1 = *(const SpriteInfo *const*)a;
	const SpriteInfo *spr2 = *(const SpriteInfo *const*)b;

	if (spr1->combinedPriority > spr2->combinedPriority)
		return 1;

	if (spr1->combinedPriority < spr2->combinedPriority)
		return -1;

	return 0;
}

void Sprite::buildActiveSpriteList() {
	SpriteInfo **spritePtr;

	// Build the list of active sprites
	_activeSpriteCount = 0;
	spritePtr = _activeSprites;

	for (int i = 1; i < _maxSprites; i++) {
		if (!(_spriteTable[i].flags & kSFActive))
			continue;

		if (!(_spriteTable[i].flags & kSFSmartRender)) {
			if (!(_spriteTable[i].flags & kSFIgnoreRender)) {
				_spriteTable[i].flags |= kSFRender;
			}

			if (!(_spriteTable[i].flags & kSFIgnoreErase)) {
				_spriteTable[i].flags |= kSFErase;
			}
		}

		if (_vm->_game.heversion > 90) {
			_spriteTable[i].combinedPriority =  (_spriteTable[i].priority +
				(((_spriteTable[i].group) ? _groupTable[_spriteTable[i].group].priority : 0)));
		}

		*spritePtr++ = &_spriteTable[i];
		_activeSpriteCount++;
	}

	// Sort the list of active sprites...
	if (_activeSpriteCount) {
		qsort(_activeSprites, _activeSpriteCount, sizeof(SpriteInfo *), compareSpritePriority);
	}
}

void Sprite::renderSprites(bool negativeOrPositiveRender) {
	int image, group, shadow, state, angle;
	int sourceImage, scale, destImageNumber;
	WizImageCommand imageRenderCmd;
	Common::Rect clippedLastRect;
	Common::Rect *clipRectPtr;
	SpriteInfo **spritePtr;
	int32 flags, renderFlags;
	bool angleSpecified;
	bool scaleSpecified;
	Common::Point spot;

	spritePtr = _activeSprites;

	for (int i = 0; i < _activeSpriteCount; i++) {
		// Get the current flags & clear the render flag...
		flags = spritePtr[i]->flags;

		// Check to see if this sprite needs to render...
		if (!(flags & kSFRender)) {
			continue;
		}

		// This code is how we draw the negative priority sprites first.
		// If it reaches a positive priority it stops, because the active list
		// should be sorted by this point...
		if (negativeOrPositiveRender) {
			if (spritePtr[i]->combinedPriority >= 0) {
				break;
			}
		} else {
			if (spritePtr[i]->combinedPriority < 0) {
				continue;
			}
		}

		// Clear the sprite render flag...
		spritePtr[i]->flags &= ~kSFRender;

		// Get the current image and the rect it will render to...
		image = spritePtr[i]->image;
		state = spritePtr[i]->state;

		int32 spotX, spotY;
		calcSpriteSpot(spritePtr[i], true, spotX, spotY);

		// In the hope that it never overflows...
		spot.x = (int16)spotX;
		spot.y = (int16)spotY;

		// Setup the image render command...
		imageRenderCmd.extendedRenderInfo.sprite = (int32)(spritePtr[i] - _spriteTable);
		imageRenderCmd.extendedRenderInfo.group = spritePtr[i]->group;
		imageRenderCmd.extendedRenderInfo.conditionBits = spritePtr[i]->conditionBits;

		imageRenderCmd.actionFlags = kWAFSpot;
		imageRenderCmd.xPos = spot.x;
		imageRenderCmd.yPos = spot.y;

		imageRenderCmd.actionFlags |= kWAFState;
		imageRenderCmd.image = image;
		imageRenderCmd.state = state;

		spritePtr[i]->lastAngle = spritePtr[i]->angle;
		spritePtr[i]->lastScale = spritePtr[i]->scale;
		spritePtr[i]->lastImage = image;
		spritePtr[i]->lastState = state;
		spritePtr[i]->lastSpot = spot;

		// The the potential update rect (we'll clip it later)
		getSpriteRectPrim(spritePtr[i], &spritePtr[i]->lastRect, true, &spot);

		// Setup the renderFlags
		renderFlags = kWRFForeground;

		if (flags & kSFHFlip) {
			renderFlags |= kWRFHFlip;
		}

		if (flags & kSFVFlip) {
			renderFlags |= kWRFVFlip;
		}

		if (flags & kSFBackgroundRender) {
			renderFlags &= ~kWRFForeground;
			renderFlags |= kWRFBackground;
		}

		// Check to see if there is any shadow attached to this sprite...
		shadow = spritePtr[i]->shadow;
		if (shadow != 0) {
			renderFlags |= kWRFUseShadow;
			imageRenderCmd.actionFlags |= kWAFShadow;
			imageRenderCmd.shadow = shadow;
		}

		// Check to see if the sprite is supposed to remap...
		if (flags & kSFUseImageRemap) {
			renderFlags |= kWRFRemap;
		}

		// Handle Z-Clipping
		if (spritePtr[i]->zbufferImage != 0) {
			imageRenderCmd.actionFlags |= kWAFZBufferImage;
			imageRenderCmd.zbufferImage = spritePtr[i]->zbufferImage;
			imageRenderCmd.zPos = spritePtr[i]->priority;
		}

		// Set the source image...
		sourceImage = spritePtr[i]->sourceImage;
		if (sourceImage != 0) {
			imageRenderCmd.actionFlags |= kWAFSourceImage;
			imageRenderCmd.sourceImage = sourceImage;
		}

		renderFlags |= spritePtr[i]->specialRenderFlags;

		// Finally set the image render flags
		imageRenderCmd.actionFlags |= kWAFFlags;
		imageRenderCmd.flags = renderFlags;

		// Read the angle/scale variables
		angle = spritePtr[i]->angle;
		scale = spritePtr[i]->scale;

		scaleSpecified = (spritePtr[i]->flags & kSFScaleSpecified);
		angleSpecified = (spritePtr[i]->flags & kSFAngleSpecified);

		// Check for "complex" image draw mode...
		if (angleSpecified) {
			imageRenderCmd.actionFlags |= kWAFAngle;
			imageRenderCmd.angle = angle;
		}

		if (scaleSpecified) {
			imageRenderCmd.actionFlags |= kWAFScale;
			imageRenderCmd.scale = scale;
		}

		// Store off the render flags
		spritePtr[i]->lastRenderFlags = renderFlags;

		// Check to see if the group has a clipping rect.
		group = spritePtr[i]->group;
		if (group != 0) {
			if (_groupTable[group].flags & kSGFUseClipRect) {
				if (!_vm->_wiz->findRectOverlap(&spritePtr[i]->lastRect, &_groupTable[group].clipRect)) {
					spritePtr[i]->lastRect.left = 1234;
					spritePtr[i]->lastRect.top = 1234;
					spritePtr[i]->lastRect.right = -1234;
					spritePtr[i]->lastRect.bottom = -1234;

					continue;
				}

				// Setup the clipping rect to the overlap rect.
				// This will eventually be clipped down to the
				// limits of the bitmap
				imageRenderCmd.actionFlags |= kWAFRect;
				imageRenderCmd.box.left= spritePtr[i]->lastRect.left;
				imageRenderCmd.box.top = spritePtr[i]->lastRect.top;
				imageRenderCmd.box.right = spritePtr[i]->lastRect.right;
				imageRenderCmd.box.bottom = spritePtr[i]->lastRect.bottom;

				clippedLastRect = spritePtr[i]->lastRect;
				clipRectPtr = &clippedLastRect;
			} else {
				clipRectPtr = nullptr;
			}
		} else {
			clipRectPtr = nullptr;
		}

		// Finally get down to business and render the wiz
		// get the palette
		if (spritePtr[i]->palette != 0) {
			imageRenderCmd.actionFlags |= kWAFPalette;
			imageRenderCmd.palette = spritePtr[i]->palette;
		}

		// get the associated dest image if any
		destImageNumber = getDestImageForSprite(spritePtr[i]);

		if (destImageNumber) {
			imageRenderCmd.actionFlags |= kWAFDestImage;
			imageRenderCmd.destImageNumber = destImageNumber;
		}

		// Finally actually do something by calling the command parser
		// this function is the same core that renders images via the
		// "image" draw command.
		if (_vm->_game.heversion > 99) {
			imageRenderCmd.actionType = kWADraw;
			_vm->_wiz->processWizImageCmd(&imageRenderCmd);
		} else {
			_vm->_wiz->processWizImageDrawCmd(&imageRenderCmd);
		}
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
	s.syncAsSint32LE(sg.clipRect.left, VER(48));
	s.syncAsSint32LE(sg.clipRect.top, VER(48));
	s.syncAsSint32LE(sg.clipRect.right, VER(48));
	s.syncAsSint32LE(sg.clipRect.bottom, VER(48));
	s.syncAsSint32LE(sg.priority, VER(48));
	s.syncAsSint32LE(sg.flags, VER(48));
	s.syncAsSint32LE(sg.posX, VER(48));
	s.syncAsSint32LE(sg.posY, VER(48));
	s.syncAsSint32LE(sg.image, VER(48));
	s.syncAsSint32LE(sg.isScaled, VER(48));
	s.syncAsSint32LE(sg.xMul, VER(48));
	s.syncAsSint32LE(sg.xDiv, VER(48));
	s.syncAsSint32LE(sg.yMul, VER(48));
	s.syncAsSint32LE(sg.yDiv, VER(48));
}

void Sprite::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.getVersion() >= VER(64)) {
		s.syncArray(_spriteTable, _maxSprites + 1, syncWithSerializer);
		s.syncArray(_groupTable, _maxSpriteGroups + 1, syncWithSerializer);
	} else {
		// TODO: This had been bogus, what is it really supposed to do?
//		s->saveLoadArrayOf(_activeSprites, _maxSprites, sizeof(_activeSprites[0]), spriteEntries);
		s.syncArray(*_activeSprites, _maxSprites, syncWithSerializer);
		s.syncArray(_spriteTable, _maxSprites, syncWithSerializer);
		s.syncArray(_groupTable, _maxSpriteGroups, syncWithSerializer);
	}

	// Reset active sprite table
	if (s.isLoading())
		_activeSpriteCount = 0;
}

} // End of namespace Scumm

#endif // ENABLE_HE
