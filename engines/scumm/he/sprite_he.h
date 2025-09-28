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

#define SPRDEF_DUMB    0
#define SPRDEF_SMART   1
#define SPRDEF_SIMPLE  2

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
	kSFIgnoreErase       = 0x40000000,

	// Defaults
	kSFDefaultFlagInactive = (kSFErase  | kSFDontCombineErase),
	kSFDefaultFlagActive   = (kSFActive | kSFAutoAnimate | kSFSmartRender | kSFDontCombineErase)
};

enum SpriteGroupFlags {
	kSGFUseClipRect      = 0x00000001
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

enum SpriteProperties {
	 SPRPROP_SPECIAL_RENDER_FLAGS = 123,
	 SPRPROP_SPECIAL_BLEND_FLAGS  = 124,
	 SPRPROP_CONDITION_BITS       = 125,
	 SPRPROP_ANIMATION_SUB_STATE  = 126
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

struct SpriteImageList {
	int16 list[32];
	int16 count;
};

struct SpriteGroup {
	Common::Rect clipRect;
	int32 priority;
	int32 flags;
	int32 posX;
	int32 posY;
	int32 image;
	int32 isScaled;
	float xScale;
	float yScale;
	int32 xMul;
	int32 xDiv;
	int32 yMul;
	int32 yDiv;

	void reset() {
		clipRect.top = clipRect.left = clipRect.bottom = clipRect.right = 0;
		priority = 0;
		flags = 0;
		posX = 0;
		posY = 0;
		image = 0;
		isScaled = 0;
		xScale = 0.0F;
		yScale = 0.0F;
		xMul = 0;
		xDiv = 0;
		yMul = 0;
		yDiv = 0;
	}
};

class ScummEngine_v90he;

class Sprite : public Common::Serializable {
public:
	Sprite(ScummEngine_v90he *vm);
	~Sprite() override;

	SpriteInfo *_spriteTable;
	SpriteGroup *_groupTable;
	SpriteInfo **_activeSprites;
	SpriteImageList *_imageLists;
	int16 *_imageListStack;

	int32 _imageListStackIndex = 0;
	int32 _activeSpriteCount = 0;
	int32 _maxSpriteGroups = 0;
	int32 _maxSprites = 0;
	int32 _maxImageLists = 0;

	void saveLoadWithSerializer(Common::Serializer &s) override;
	void eraseSprites();
	bool doesRectIntersectUpdateAreas(const Common::Rect *rectPtr);
	void checkForForcedRedraws(bool checkOnlyPositivePriority);
	void qsortSpriteArray(SpriteInfo **base, uint num);
	void shortsortSpriteArray(SpriteInfo **lo, SpriteInfo **hi);
	void buildActiveSpriteList();
	void renderSprites(bool negativeOrPositiveRender);
	void runSpriteEngines();

	int spriteFromPoint(int x, int y, int spriteGroupId, int d, int num, int *args);
	int getSpriteClass(int spriteId, int num);
	int checkSpriteClassAgaintClassSet(int sprite, int classCount, int *classCheckTable);
	int getSpriteRenderToBackground(int spriteId);
	int getSpriteVertFlip(int spriteId);
	int getSpriteHorzFlip(int spriteId);
	int getSpriteActiveFlag(int spriteId);
	int getSpriteImageRemapFlag(int spriteId);
	int getSpriteAutoAnimFlag(int spriteId);
	int getSpriteUpdateType(int spriteId);
	int getSpriteEraseType(int spriteId);
	int getSpriteImage(int spriteId);
	int getSpriteImageState(int spriteId);
	int getSpriteGroup(int spriteId);
	int getSpritePalette(int spriteId);
	int getSpritePriority(int spriteId);
	int getSpriteDisplayX(int spriteId);
	int getSpriteDisplayY(int spriteId);
	int getUserValue(int spriteId);
	int getSpriteShadow(int spriteId);
	int getSpriteImageStateCount(int spriteId);
	int getSpriteScale(int spriteId);
	int getSpriteAnimSpeed(int spriteId);
	int getSourceImage(int spriteId);
	int getMaskImage(int spriteId);
	int getSpriteGeneralProperty(int spriteId, int type);
	int getDestImageForSprite(const SpriteInfo *spritePtr);
	int getSpriteAnimSpeedState(int sprite);
	void getSpriteLogicalRect(int sprite, Common::Rect *rectPtr);
	void getSpriteDrawRect(int sprite, Common::Rect *rectPtr);
	void getSpriteImageDim(int spriteId, int32 &w, int32 &h);
	void getSpritePosition(int spriteId, int32 &tx, int32 &ty);
	void getSpriteRectPrim(const SpriteInfo *spritePtr, Common::Rect *rectPtr, bool includeGroupTransform, const Common::Point *spotPtr);
	void getDelta(int spriteId, int32 &dx, int32 &dy);
	void calcSpriteSpot(const SpriteInfo *spritePtr, bool includeGroupTransform, int32 &x, int32 &y);

	int getGroupPriority(int spriteGroupId);
	int getGroupImage(int spriteGroupId);
	int getGroupXMul(int spriteGroupId);
	int getGroupXDiv(int spriteGroupId);
	int getGroupYMul(int spriteGroupId);
	int getGroupYDiv(int spriteGroupId);
	void getGroupPoint(int spriteGroupId, int32 &tx, int32 &ty);
	int getGroupGeneralProperty(int group, int property);

	void setSpritePalette(int spriteId, int value);
	void setSourceImage(int spriteId, int value);
	void setMaskImage(int spriteId, int value);
	void newSprite(int spriteId);
	void setSpriteImageState(int spriteId, int value);
	void setSpritePosition(int spriteId, int value1, int value2);
	void setSpriteGroup(int spriteId, int value);
	void setDelta(int spriteId, int value1, int value2);
	void setSpriteShadow(int spriteId, int value);
	void setUserValue(int spriteId, int value1, int value2);
	void setSpritePriority(int spriteId, int value);
	void moveSprite(int spriteId, int value1, int value2);
	void setSpriteScale(int spriteId, int value);
	void setSpriteAngle(int spriteId, int value);
	void setSpriteRenderToBackground(int spriteId, int value);
	void setSpriteVertFlip(int spriteId, int value);
	void setSpriteHorzFlip(int spriteId, int value);
	void setSpriteActiveFlag(int spriteId, int value);
	void setSpriteImageRemapFlag(int spriteId, int value);
	void setSpriteAutoAnimFlag(int spriteId, int value);
	void setSpriteUpdateType(int spriteId, int value);
	void setSpriteEraseType(int spriteId, int value);
	void setSpriteAnimSpeed(int spriteId, int value);
	void setSpriteClass(int spriteId, int classId, int toggle);
	void clearSpriteClasses(int spriteId);
	void setSpriteZBuffer(int spriteId, int value);
	void setSpriteAnimSpeedState(int spriteId, int animState);
	void setSpriteGeneralProperty(int spriteId, int type, int value);
	void setImageList(int sprite, int count, const int *list);

	void moveGroupMembers(int spriteGroupId, int value1, int value2);
	void orInGroupMembersFlags(int spriteGroupId, int32 flags);
	void setGroupMembersPriority(int spriteGroupId, int value);
	void changeGroupMembersGroup(int spriteGroupId, int value);
	void setGroupMembersUpdateType(int spriteGroupId, int value);
	void performNewOnGroupMembers(int spriteGroupId);
	void setGroupMembersAnimationSpeed(int spriteGroupId, int value);
	void setGroupMembersAutoAnimFlag(int spriteGroupId, int value);
	void setGroupMembersShadow(int spriteGroupId, int value);

	void moveGroup(int spriteGroupId, int value1, int value2);
	void setGroupClipRect(int spriteGroupId, int x1, int y1, int x2, int y2);
	void setGroupPriority(int spriteGroupId, int value);
	void setGroupPoint(int spriteGroupId, int value1, int value2);
	void setGroupImage(int spriteGroupId, int value);
	void setGroupScaling(int spriteGroupId);
	void setGroupXMul(int spriteGroupId, int value);
	void setGroupXDiv(int spriteGroupId, int value);
	void setGroupYMul(int spriteGroupId, int value);
	void setGroupYDiv(int spriteGroupId, int value);

	int pixelPerfectSpriteCollisionCheck(int spriteA, int deltaAX, int deltaAY, int spriteB, int deltaBX, int deltaBY);

	void clearGroupClipRect(int spriteGroupId);
	void clearGroupScaleInfo(int group);

	void resetImageLists();
	SpriteImageList *getImageListPtr(int imageList);
	int getFreeImageList(int imageCount);
	void releaseImageList(int imageList);

	void initializeStuff(int numSprites, int numGroups, int numMaxSprites);
	void newGroup(int spriteGroupId);
	void resetSpriteSystem(bool refreshScreen);

	void setSpriteImage(int spriteId, int imageNum);
private:
	ScummEngine_v90he *_vm;
};

} // End of namespace Scumm

#endif
