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

#include "watchmaker/extraLS.h"
#include "watchmaker/struct.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/ll/ll_util.h"

namespace Watchmaker {

char *ExtraLS[MAX_EXTRALS];

#define EXTRALS_BKG_COLOR_R     50
#define EXTRALS_BKG_COLOR_G     0
#define EXTRALS_BKG_COLOR_B     224

uint32 extraLS_BMP = 0;
uint32 extraLS_currentID = 0;
int32 extraLS_bmpPosX = 0;
int32 extraLS_bmpPosY = 0;

struct SRect Console3Save           = { 228 + 22, 11 + 15, 277 + 22, 30 + 15 };
struct SRect Console3Load           = { 340 + 22, 11 + 15, 396 + 22, 30 + 15 };
struct SRect Console3Options    = { 444 + 22, 11 + 15, 527 + 22, 30 + 15 };
struct SRect Console3Quit           = { 565 + 22, 11 + 15, 613 + 22, 30 + 15 };
struct SRect Console3Close      = { 665 + 22, 11 + 15, 724 + 22, 30 + 15 };

void CheckExtraLocalizationStrings(Renderer &renderer, uint32 id) {
	uint32 newID = 0;

	if (!bShowExtraLocalizationStrings) {
		if (extraLS_BMP) {
			rReleaseBitmap(extraLS_BMP);
			extraLS_BMP = 0;
		}
		extraLS_currentID = 0;
		return;
	}

	if (!id) {
		if (InvStatus & INV_MODE2) {
			for (;;) {
				if (CheckRect(renderer, Console3Save, mPosx, mPosy))         {
					newID = EXTRALS_SAVE;
					break;
				}
				if (CheckRect(renderer, Console3Load, mPosx, mPosy))         {
					newID = EXTRALS_LOAD;
					break;
				}
				if (CheckRect(renderer, Console3Options, mPosx, mPosy))  {
					newID = EXTRALS_OPTIONS;
					break;
				}
				if (CheckRect(renderer, Console3Quit, mPosx, mPosy))         {
					newID = EXTRALS_QUIT;
					break;
				}
				if (CheckRect(renderer, Console3Close, mPosx, mPosy))        {
					newID = EXTRALS_CLOSE;
					break;
				}

				break;
			}
		}
	} else    newID = id;

	if (extraLS_currentID != newID) {
		if (extraLS_BMP) {
			rReleaseBitmap(extraLS_BMP);
			extraLS_BMP = 0;
		}

		if (newID) {
			int x, y;
			renderer._fonts->getTextDim(ExtraLS[newID], FontKind::Standard, &x, &y);
			extraLS_BMP = CreateTooltipBitmap(renderer, ExtraLS[newID], WHITE_FONT, EXTRALS_BKG_COLOR_R, EXTRALS_BKG_COLOR_G, EXTRALS_BKG_COLOR_B);
			extraLS_bmpPosX = (800 - x) / 2;
			extraLS_bmpPosY = 600 - (y + 12);
		}

		extraLS_currentID = newID;
	}

	if (extraLS_BMP) renderer._2dStuff.displayDDBitmap(extraLS_BMP, extraLS_bmpPosX, extraLS_bmpPosY, 0, 0, 0, 0);
}

} // End of namespace Watchmaker
