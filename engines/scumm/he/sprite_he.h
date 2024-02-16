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

#if !defined(SCUMM_HE_SPRITE_HE_H) && defined(ENABLE_HE)
#define SCUMM_HE_SPRITE_HE_H

#include "common/serializer.h"

namespace Scumm {

enum SpriteFlags {
	kSFErase             = 0x00000001,
	kSFRender            = 0x00000002,
	kSFScaleSpecified    = 0x00000010,
	kSFAngleSpecified    = 0x00000020,
	kSFBackgroundRender  = 0x00001000,
	kSFVFlip             = 0x00002000,
	kSFHFlip             = 0x00004000,
	kSFActive            = 0x00008000,
	kSFUseImageRemap     = 0x00080000,
	kSFAutoAnimate       = 0x00200000,
	kSFSmartRender       = 0x00400000,
	kSFDontAnimImageList = 0x01000000,
	kSFDontCombineErase  = 0x02000000,
	kSFIgnoreRender      = 0x20000000,
	kSFIgnoreErase       = 0x40000000
};

enum SpriteGroupFlags {
	kSGFUseClipRect     = (1 << 0)
};

enum SpritePropertySubOps {
	SPRPROP_HFLIP = 0,
	SPRPROP_VFLIP = 1,
	SPRPROP_ACTIVE = 2,
	SPRPROP_BACKGROUND_RENDER = 3,
	SPRPROP_USE_IMAGE_REMAP_TABLE = 4,
};

enum SpriteGroupPropertySubOps {
	SPRGRPPROP_XMUL = 0,
	SPRGRPPROP_XDIV = 1,
	SPRGRPPROP_YMUL = 2,
	SPRGRPPROP_YDIV = 3,
};

enum SpriteGroupSubOps {
	SPRGRPOP_MOVE = 1,
	SPRGRPOP_ORDER = 2,
	SPRGRPOP_NEW_GROUP = 3,
	SPRGRPOP_UPDATE_TYPE = 4,
	SPRGRPOP_NEW = 5,
	SPRGRPOP_ANIMATION_SPEED = 6,
	SPRGRPOP_ANIMATION_TYPE = 7,
	SPRGRPOP_SHADOW = 8,
};

enum PolygonOverlapSubOps {
	OVERLAP_POINT_TO_RECT = 1,
	OVERLAP_POINT_TO_CIRCLE = 2,
	OVERLAP_RECT_TO_RECT = 3,
	OVERLAP_CIRCLE_TO_CIRCLE = 4,
	OVERLAP_POINT_N_SIDED_POLYGON = 5,
	OVERLAP_SPRITE_TO_SPRITE = 6,
	OVERLAP_SPRITE_TO_RECT = 7,
	OVERLAP_DRAW_POS_SPRITE_TO_SPRITE = 8,
	OVERLAP_DRAW_POS_SPRITE_TO_RECT = 9,
	OVERLAP_SPRITE_TO_SPRITE_PIXEL_PERFECT = 10,
};

struct SpriteInfo {
	int32 id;
	int32 combinedPriority;
	int32 flags;
	int32 image;
	int32 state;
	int32 group;
	int32 palette;
	int32 priority;
	Common::Rect lastRect;
	int32 deltaPosX;
	int32 deltaPosY;
	Common::Point lastSpot;
	int32 posX;
	int32 posY;
	int32 userValue;
	int32 lastState;
	int32 lastImage;
	int32 imageList;
	int32 shadow;
	int32 maxStates;
	int32 angle;
	int32 scale;
	int32 animState;
	int32 lastAngle;
	int32 lastScale;
	int32 lastRenderFlags;
	int32 animIndex;
	int32 animSpeed;
	int32 sourceImage;
	int32 maskImage;
	int32 zbufferImage;
	int32 classFlags;
	int32 specialRenderFlags;
	int32 conditionBits;

	void reset() {
		id = 0;
		combinedPriority = 0;
		flags = 0;
		image = 0;
		state = 0;
		group = 0;
		palette = 0;
		priority = 0;
		lastRect.top = lastRect.left = lastRect.bottom = lastRect.right = 0;
		deltaPosX = 0;
		deltaPosY = 0;
		lastSpot.x = lastSpot.y = 0;
		posX = 0;
		posY = 0;
		userValue = 0;
		lastState = 0;
		lastImage = 0;
		imageList = 0;
		shadow = 0;
		maxStates = 0;
		angle = 0;
		scale = 0;
		animState = 0;
		lastAngle = 0;
		lastScale = 0;
		lastRenderFlags = 0;
		animIndex = 0;
		animSpeed = 0;
		sourceImage = 0;
		maskImage = 0;
		zbufferImage = 0;
		classFlags = 0;
		specialRenderFlags = 0;
		conditionBits = 0;
	}
};

struct SpriteGroup {
	Common::Rect bbox;
	int32 priority;
	int32 flags;
	int32 tx;
	int32 ty;
	int32 image;
	int32 scaling;
	int32 scale_x_ratio_mul;
	int32 scale_x_ratio_div;
	int32 scale_y_ratio_mul;
	int32 scale_y_ratio_div;

	void reset() {
		bbox.top = bbox.left = bbox.bottom = bbox.right = 0;
		priority = 0;
		flags = 0;
		tx = 0;
		ty = 0;
		image = 0;
		scaling = 0;
		scale_x_ratio_mul = 0;
		scale_x_ratio_div = 0;
		scale_y_ratio_mul = 0;
		scale_y_ratio_div = 0;
	}
};

class ScummEngine_v90he;

class Sprite : public Common::Serializable {
public:
	Sprite(ScummEngine_v90he *vm);
	~Sprite() override;

	SpriteInfo *_spriteTable;
	SpriteGroup *_spriteGroups;
	SpriteInfo **_activeSpritesTable;

	int32 _numSpritesToProcess;
	int32 _varNumSpriteGroups;
	int32 _varNumSprites;
	int32 _varMaxSprites;

	void saveLoadWithSerializer(Common::Serializer &s) override;
	void resetBackground();
	void checkForForcedRedraws(bool checkZOrder);
	void sortActiveSprites();
	void renderSprites(bool arg);
	void updateImages();

	int findSpriteWithClassOf(int x, int y, int spriteGroupId, int d, int num, int *args);
	int getSpriteClass(int spriteId, int num, int *args);
	int getSpriteFlagDoubleBuffered(int spriteId);
	int getSpriteFlagYFlipped(int spriteId);
	int getSpriteFlagXFlipped(int spriteId);
	int getSpriteFlagActive(int spriteId);
	int getSpriteFlagRemapPalette(int spriteId);
	int getSpriteFlagAutoAnim(int spriteId);
	int getSpriteFlagUpdateType(int spriteId);
	int getSpriteFlagEraseType(int spriteId);
	int getSpriteImage(int spriteId);
	int getSpriteImageState(int spriteId);
	int getSpriteGroup(int spriteId);
	int getSpritePalette(int spriteId);
	int getSpritePriority(int spriteId);
	int getSpriteDisplayX(int spriteId);
	int getSpriteDisplayY(int spriteId);
	int getSpriteUserValue(int spriteId);
	int getSpriteShadow(int spriteId);
	int getSpriteImageStateCount(int spriteId);
	int getSpriteScale(int spriteId);
	int getSpriteAnimSpeed(int spriteId);
	int getSpriteSourceImage(int spriteId);
	int getSpriteMaskImage(int spriteId);
	int getSpriteGeneralProperty(int spriteId, int type);
	void getSpriteBounds(int spriteId, bool checkGroup, Common::Rect &bound);
	void getSpriteImageDim(int spriteId, int32 &w, int32 &h);
	void getSpritePosition(int spriteId, int32 &tx, int32 &ty);
	void getSpriteDist(int spriteId, int32 &dx, int32 &dy);

	int getGroupPriority(int spriteGroupId);
	int getGroupDstResNum(int spriteGroupId);
	int getGroupXMul(int spriteGroupId);
	int getGroupXDiv(int spriteGroupId);
	int getGroupYMul(int spriteGroupId);
	int getGroupYDiv(int spriteGroupId);
	void getGroupPosition(int spriteGroupId, int32 &tx, int32 &ty);

	void setSpritePalette(int spriteId, int value);
	void setSpriteSourceImage(int spriteId, int value);
	void setSpriteMaskImage(int spriteId, int value);
	void resetSprite(int spriteId);
	void setSpriteImageState(int spriteId, int value);
	void setSpritePosition(int spriteId, int value1, int value2);
	void setSpriteGroup(int spriteId, int value);
	void setSpriteDist(int spriteId, int value1, int value2);
	void setSpriteShadow(int spriteId, int value);
	void setSpriteUserValue(int spriteId, int value1, int value2);
	void setSpritePriority(int spriteId, int value);
	void moveSprite(int spriteId, int value1, int value2);
	void setSpriteScale(int spriteId, int value);
	void setSpriteAngle(int spriteId, int value);
	void setSpriteFlagDoubleBuffered(int spriteId, int value);
	void setSpriteFlagYFlipped(int spriteId, int value);
	void setSpriteFlagXFlipped(int spriteId, int value);
	void setSpriteFlagActive(int spriteId, int value);
	void setSpriteFlagRemapPalette(int spriteId, int value);
	void setSpriteFlagAutoAnim(int spriteId, int value);
	void setSpriteFlagUpdateType(int spriteId, int value);
	void setSpriteFlagEraseType(int spriteId, int value);
	void setSpriteAnimSpeed(int spriteId, int value);
	void setSpriteSetClass(int spriteId, int classId, int toggle);
	void setSpriteResetClass(int spriteId);
	void setSpriteZBuffer(int spriteId, int value);
	void setSpriteGeneralProperty(int spriteId, int type, int value);

	void moveGroupMembers(int spriteGroupId, int value1, int value2);
	void redrawSpriteGroup(int spriteGroupId);
	void setGroupMembersPriority(int spriteGroupId, int value);
	void setGroupMembersGroup(int spriteGroupId, int value);
	void setGroupMembersUpdateType(int spriteGroupId, int value);
	void setGroupMembersResetSprite(int spriteGroupId);
	void setGroupMembersAnimationSpeed(int spriteGroupId, int value);
	void setGroupMembersAutoAnimFlag(int spriteGroupId, int value);
	void setGroupMembersShadow(int spriteGroupId, int value);

	void moveGroup(int spriteGroupId, int value1, int value2);
	void setGroupBounds(int spriteGroupId, int x1, int y1, int x2, int y2);
	void setGroupPriority(int spriteGroupId, int value);
	void setGroupPosition(int spriteGroupId, int value1, int value2);
	void setGroupImage(int spriteGroupId, int value);
	void setGroupScaling(int spriteGroupId);
	void setGroupXMul(int spriteGroupId, int value);
	void setGroupXDiv(int spriteGroupId, int value);
	void setGroupYMul(int spriteGroupId, int value);
	void setGroupYDiv(int spriteGroupId, int value);
	void resetGroupBounds(int spriteGroupId);

	void allocTables(int numSprites, int numGroups, int numMaxSprites);
	void resetGroup(int spriteGroupId);
	void resetTables(bool refreshScreen);
	void setSpriteImage(int spriteId, int imageNum);
private:
	ScummEngine_v90he *_vm;
};

} // End of namespace Scumm

#endif
