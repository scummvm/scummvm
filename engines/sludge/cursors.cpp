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

#include "sludge/allfiles.h"
#include "sludge/cursors.h"
#include "sludge/event.h"
#include "sludge/graphics.h"
#include "sludge/people.h"
#include "sludge/sprites.h"
#include "sludge/sprbanks.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"

namespace Sludge {

PersonaAnimation  *mouseCursorAnim;
int mouseCursorFrameNum = 0;
int mouseCursorCountUp = 0;

void pickAnimCursor(PersonaAnimation  *pp) {
	deleteAnim(mouseCursorAnim);
	mouseCursorAnim = pp;
	mouseCursorFrameNum = 0;
	mouseCursorCountUp = 0;
}

void displayCursor() {
	if (mouseCursorAnim && mouseCursorAnim->numFrames) {

		int spriteNum = mouseCursorAnim->frames[mouseCursorFrameNum].frameNum;
		int flipMe = 0;

		if (spriteNum < 0) {
			spriteNum = -spriteNum;
			flipMe = 1;
			if (spriteNum >= mouseCursorAnim->theSprites->bank.total)
				spriteNum = 0;
		} else {
			if (spriteNum >= mouseCursorAnim->theSprites->bank.total)
				flipMe = 2;
		}

		if (flipMe != 2) {
			if (flipMe) {
				g_sludge->_gfxMan->flipFontSprite(
						g_sludge->_evtMan->mouseX(), g_sludge->_evtMan->mouseY(),
						mouseCursorAnim->theSprites->bank.sprites[spriteNum],
						mouseCursorAnim->theSprites->bank.myPalette /* ( spritePalette&) NULL*/);
			} else {
				g_sludge->_gfxMan->fontSprite(
						g_sludge->_evtMan->mouseX(), g_sludge->_evtMan->mouseY(),
						mouseCursorAnim->theSprites->bank.sprites[spriteNum],
						mouseCursorAnim->theSprites->bank.myPalette /* ( spritePalette&) NULL*/);
			}
		}

		if (++mouseCursorCountUp >= mouseCursorAnim->frames[mouseCursorFrameNum].howMany) {
			mouseCursorCountUp = 0;
			mouseCursorFrameNum++;
			mouseCursorFrameNum %= mouseCursorAnim->numFrames;
		}
	}
}

void pasteCursor(int x, int y, PersonaAnimation  *c) {
	if (c->numFrames)
		g_sludge->_gfxMan->pasteSpriteToBackDrop(x, y, c->theSprites->bank.sprites[c->frames[0].frameNum], c->theSprites->bank.myPalette);
}

} // End of namespace Sludge
