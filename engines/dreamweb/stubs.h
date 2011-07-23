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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

	void multidump();
	void multidump(uint16 x, uint16 y, uint8 width, uint8 height);
	void frameoutv(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameoutnm(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameoutbh(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameoutfx(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void worktoscreen();
	void multiget();
	void multiget(uint16 x, uint16 y, uint8 width, uint8 height);
	void convertkey();
	void cls();
	void printsprites();
	void quickquit();
	void readoneblock();
	void printundermon();
	void seecommandtail();
	void randomnumber();
	void quickquit2();
	void getnextword();
	uint8 getnextword(const uint8 *string, uint8 *totalWidth, uint8 *charCount);
	void printboth();
	void printboth(uint16 dst, uint16 src, uint16 *x, uint16 y, uint8 c);
	void printchar();
	void printchar(uint16 dst, uint16 src, uint16 *x, uint16 y, uint8 c, uint8 *width, uint8 *height);
	void printdirect();
	void printdirect(uint16 x, uint16 *y, uint8 maxWidth, bool centered);
	void usetimedtext();
	void getundertimed();
	void putundertimed();
	uint8 printslow(uint16 x, uint16 y, uint8 maxWidth, bool centered);
	void printslow();
	void dumptextline();
	void getnumber();
	uint8 getnumber(uint16 index, uint16 maxWidth, bool centered, uint16* offset);
	void kernchars();
	uint8 kernchars(uint8 firstChar, uint8 secondChar, uint8 width);
	Sprite *spritetable();
	void showframe();
	void showframe(uint16 dst, uint16 src, uint16 x, uint16 y, uint8 frameNumber, uint8 effectsFlag, uint8 *width, uint8 *height);
	void printasprite(const Sprite *sprite);
	void width160();
	void multiput(uint16 x, uint16 y, uint8 width, uint8 height);
	void multiput();
	void eraseoldobs();
	void clearsprites();
	void makesprite();
	Sprite *makesprite(uint8 x, uint8 y, uint16 updateCallback, uint16 somethingInDx, uint16 somethingInDi);
	void spriteupdate();
	void initman();
	void mainmanCPP(Sprite *sprite);
	void walking();
	void aboutturn(Sprite *sprite);
	void backobject(Sprite *sprite);
	void constant(Sprite *sprite, ObjData *objData);
	void steady(Sprite *sprite, ObjData *objData);
	void random(Sprite *sprite, ObjData *objData);
	void dodoor();
	void dodoor(Sprite *sprite, ObjData *objData);
	void doorway(Sprite *sprite, ObjData *objData);
	void widedoor(Sprite *sprite, ObjData *objData);
	void liftsprite();
	void liftsprite(Sprite *sprite, ObjData *objData);
	void turnpathonCPP(uint8 param);
	void turnpathoffCPP(uint8 param);
	void modifychar();
	void lockmon();
	void cancelch0();
	void cancelch1();
