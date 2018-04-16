/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGI_SPRITE_H
#define AGI_SPRITE_H

namespace Agi {

/**
 * Sprite structure.
 * This structure holds information on visible and priority data of
 * a rectangular area of the AGI screen. Sprites are chained in two
 * circular lists, one for updating and other for non-updating sprites.
 */
struct Sprite {
	uint16 givenOrderNr;
	uint16 sortOrder;
	ScreenObjEntry *screenObjPtr; /**< pointer to view table entry */
	int16 xPos;                   /**< x coordinate of the sprite */
	int16 yPos;                   /**< y coordinate of the sprite */
	int16 xSize;                  /**< width of the sprite */
	int16 ySize;                  /**< height of the sprite */
	byte *backgroundBuffer;       /**< buffer to store background data */
};

typedef Common::List<Sprite> SpriteList;
typedef Common::List<Sprite *> SpritePtrList;

class AgiEngine;
class GfxMgr;
class Obejcts;

class SpritesMgr {
private:
	GfxMgr *_gfx;
	AgiEngine *_vm;

	//
	// Sprite management functions
	//

	SpriteList _spriteRegularList;
	SpriteList _spriteStaticList;

public:
	void buildRegularSpriteList();
	void buildStaticSpriteList();
	void buildAllSpriteLists();
	void buildSpriteListAdd(uint16 givenOrderNr, ScreenObjEntry *screenObj, SpriteList &spriteList);
	void freeList(SpriteList &spriteList);
	void freeRegularSprites();
	void freeStaticSprites();
	void freeAllSprites();

	void eraseSprites(SpriteList &spriteList);
	void eraseRegularSprites();
	void eraseStaticSprites();
	void eraseSprites();

	void drawSprites(SpriteList &spriteList);
	void drawRegularSpriteList();
	void drawStaticSpriteList();
	void drawAllSpriteLists();

	void drawCel(ScreenObjEntry *screenObj);

	void showSprite(ScreenObjEntry *screenObj);
	void showSprites(SpriteList &spriteList);
	void showRegularSpriteList();
	void showStaticSpriteList();
	void showAllSpriteLists();

	void showObject(int16 viewNr);

public:
	SpritesMgr(AgiEngine *agi, GfxMgr *gfx);
	~SpritesMgr();

	int initSprites();
	void deinitSprites();
	void addToPic(int16 viewNr, int16 loopNr, int16 celNr, int16 xPos, int16 yPos, int16 priority, int16 border);
	void addToPicDrawPriorityBox(ScreenObjEntry *screenObj, int16 border);
};

} // End of namespace Agi

#endif /* AGI_SPRITE_H */
