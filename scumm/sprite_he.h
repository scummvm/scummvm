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

#if !defined(SPRITE_HE_H) && !defined(DISABLE_HE)
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
	int32 curImgFlags;
	int32 field_74;
	int32 delayAmount;
	int32 maskImgResNum;
	int32 field_80;
	int32 field_84;
	int32 classFlags;
	int32 imgFlags;
	int32 field_90;
};

struct SpriteGroup {
	Common::Rect bbox;
	int32 zorderPriority;
	int32 flags;
	int32 tx;
	int32 ty;
	int32 dstResNum;
	int32 scaling;
	int32 scale_x_ratio_mul;
	int32 scale_x_ratio_div;
	int32 scale_y_ratio_mul;
	int32 scale_y_ratio_div;
};

class ScummEngine_v90he;

class Sprite {
public:
	Sprite(ScummEngine_v90he *vm);

	SpriteInfo *_spriteTable;
	SpriteGroup *_spriteGroups;
	SpriteInfo **_activeSpritesTable;

	int32 _numSpritesToProcess;
	int32 _varNumSpriteGroups;
	int32 _varNumSprites;
	int32 _varMaxSprites;

	void saveOrLoadSpriteData(Serializer *s, uint32 savegameVersion);
	void spritesBlitToScreen();
	void spritesMarkDirty(bool checkZOrder);
	void spritesSortActiveSprites();
	void spritesProcessWiz(bool arg);
	void spritesUpdateImages();

	void getSpriteBounds(int spriteId, bool checkGroup, Common::Rect &bound);
	int findSpriteWithClassOf(int x, int y, int spriteGroupId, int d, int num, int *args);
	int getSpriteclassFlags(int spriteId, int num, int *args);
	int getSpriteFlagDoubleBuffered(int spriteId);
	int getSpriteFlagYFlipped(int spriteId);
	int getSpriteFlagXFlipped(int spriteId);
	int getSpriteFlagActive(int spriteId);
	int getSpriteFlagNeedPaletteRemap(int spriteId);
	int getSpriteFlagDelayed(int spriteId);
	int getSpriteFlagMarkDirty(int spriteId);	
	int getSpriteFlagHasImage(int spriteId);
	int getSpriteResId(int spriteId);
	int getSpriteResState(int spriteId);
	int getSpriteGroupNum(int spriteId);
	int getSpritePaletteNum(int spriteId);
	int getSpriteZorderPriority(int spriteId);
	int getSpritegrp_tx(int spriteId);
	int getSpritegrp_ty(int spriteId);
	int getSpriteField_44(int spriteId);
	int getSpriteXmapNum(int spriteId);
	int getSpritewizSize(int spriteId);
	int getSpritezoom(int spriteId);
	int getSpritedelayAmount(int spriteId);
	int getSpritemaskImgResNum(int spriteId);
	int getSpriteField_80(int spriteId);
	int getSpriteField_8C_90(int spriteId, int type);
	void getSpriteImageDim(int spriteId, int32 &w, int32 &h);
	void getSpritetx_ty(int spriteId, int32 &tx, int32 &ty);
	void getSpritedx_dy(int spriteId, int32 &dx, int32 &dy);

	int getGroupzorderPriority(int spriteGroupId);
	int getGroupdstResNum(int spriteGroupId);
	int getGroupScale_x_ratio_mul(int spriteGroupId);
	int getGroupScale_x_ratio_div(int spriteGroupId);
	int getGroupScale_y_ratio_mul(int spriteGroupId);
	int getGroupScale_y_ratio_div(int spriteGroupId);
	void getGrouptx_ty(int spriteGroupId, int32 &tx, int32 &ty);

	void setSpritePaletteNum(int spriteId, int value);
	void setSpriteMaskImgResNum(int spriteId, int value);
	void setSpriteField80(int spriteId, int value);
	void setSpriteResetSprite(int spriteId);
	void setSpriteResState(int spriteId, int value);
	void setSpritetx_ty(int spriteId, int value1, int value2);
	void setSpriteGroupNum(int spriteId, int value);
	void setSpritedx_dy(int spriteId, int value1, int value2);
	void setSpriteXmapNum(int spriteId, int value);
	void setSpriteField44(int spriteId, int value1, int value2);
	void setSpriteZorderPriority(int spriteId, int value);
	void setSpriteInc_tx_ty(int spriteId, int value1, int value2);
	void setSpriteZoom(int spriteId, int value);
	void setSpriteAngle(int spriteId, int value);
	void setSpriteFlagDoubleBuffered(int spriteId, int value);
	void setSpriteFlagYFlipped(int spriteId, int value);
	void setSpriteFlagXFlipped(int spriteId, int value);
	void setSpriteFlagActive(int spriteId, int value);
	void setSpriteFlagNeedPaletteRemap(int spriteId, int value);
	void setSpriteFlagDelayed(int spriteId, int value);
	void setSpriteFlagMarkDirty(int spriteId, int value);
	void setSpriteFlagHasImage(int spriteId, int value);
	void setSpriteDelay(int spriteId, int value);
	void setSpriteSetClassFlag(int spriteId, int classId, int toggle);
	void setSpriteResetClassFlags(int spriteId);
	void setSpriteField84(int spriteId, int value);
	void setSpriteField8C_90(int spriteId, int type, int value);

	void redrawSpriteGroup(int spriteGroupId);
	void setGroupCase0_0(int spriteGroupId, int value1, int value2);
	void setGroupCase0_1(int spriteGroupId, int value);
	void setGroupCase0_2(int spriteGroupId, int value);
	void setGroupCase0_3(int spriteGroupId, int value);
	void setGroupCase0_4(int spriteGroupId);
	void setGroupCase0_5(int spriteGroupId, int value);
	void setGroupCase0_6(int spriteGroupId, int value);
	void setGroupCase0_7(int spriteGroupId, int value);
	void setGroupbbox(int spriteGroupId, int x1, int y1, int x2, int y2);
	void setGroupzorderPriority(int spriteGroupId, int value);
	void setGrouptx_ty(int spriteGroupId, int value1, int value2);
	void setGroupinc_tx_ty(int spriteGroupId, int value1, int value2);
	void setGroupdstResNum(int spriteGroupId, int value);
	void setGroupscaling(int spriteGroupId);
	void setGroupScale_x_ratio_mul(int spriteGroupId, int value);
	void setGroupScale_x_ratio_div(int spriteGroupId, int value);
	void setGroupScale_y_ratio_mul(int spriteGroupId, int value);
	void setGroupScale_y_ratio_div(int spriteGroupId, int value);
	void setGroupflagClipBoxAnd(int spriteGroupId);

	void spritesAllocTables(int numSprites, int numGroups, int numMaxSprites);
	void spritesResetGroup(int spriteGroupId);
	void spritesResetTables(bool refreshScreen);
	void spriteAddImageToList(int spriteId, int imageNum, int *spriteIdptr);
private:
	ScummEngine_v90he *_vm;
};

} // End of namespace Scumm

#endif

