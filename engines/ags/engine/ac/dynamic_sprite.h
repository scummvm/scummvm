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

#ifndef AGS_ENGINE_AC_DYNAMICSPRITE_H
#define AGS_ENGINE_AC_DYNAMICSPRITE_H

#include "ags/engine/ac/dynobj/script_dynamic_sprite.h"
#include "ags/engine/ac/dynobj/script_drawing_surface.h"

namespace AGS3 {

void    DynamicSprite_Delete(ScriptDynamicSprite *sds);
ScriptDrawingSurface *DynamicSprite_GetDrawingSurface(ScriptDynamicSprite *dss);
int     DynamicSprite_GetGraphic(ScriptDynamicSprite *sds);
int     DynamicSprite_GetWidth(ScriptDynamicSprite *sds);
int     DynamicSprite_GetHeight(ScriptDynamicSprite *sds);
int     DynamicSprite_GetColorDepth(ScriptDynamicSprite *sds);
void    DynamicSprite_Resize(ScriptDynamicSprite *sds, int width, int height);
void    DynamicSprite_Flip(ScriptDynamicSprite *sds, int direction);
void    DynamicSprite_CopyTransparencyMask(ScriptDynamicSprite *sds, int sourceSprite);
void    DynamicSprite_ChangeCanvasSize(ScriptDynamicSprite *sds, int width, int height, int x, int y);
void    DynamicSprite_Crop(ScriptDynamicSprite *sds, int x1, int y1, int width, int height);
void    DynamicSprite_Rotate(ScriptDynamicSprite *sds, int angle, int width, int height);
void    DynamicSprite_Tint(ScriptDynamicSprite *sds, int red, int green, int blue, int saturation, int luminance);
int     DynamicSprite_SaveToFile(ScriptDynamicSprite *sds, const char *namm);
ScriptDynamicSprite *DynamicSprite_CreateFromSaveGame(int sgslot, int width, int height);
ScriptDynamicSprite *DynamicSprite_CreateFromFile(const char *filename);
ScriptDynamicSprite *DynamicSprite_CreateFromScreenShot(int width, int height);
ScriptDynamicSprite *DynamicSprite_CreateFromExistingSprite(int slot, int preserveAlphaChannel);
ScriptDynamicSprite *DynamicSprite_CreateFromDrawingSurface(ScriptDrawingSurface *sds, int x, int y, int width, int height);
ScriptDynamicSprite *DynamicSprite_Create(int width, int height, int alphaChannel);
ScriptDynamicSprite *DynamicSprite_CreateFromExistingSprite_Old(int slot);
ScriptDynamicSprite *DynamicSprite_CreateFromBackground(int frame, int x1, int y1, int width, int height);


void    add_dynamic_sprite(int gotSlot, AGS::Shared::Bitmap *redin, bool hasAlpha = false);
void    free_dynamic_sprite(int gotSlot);

} // namespace AGS3

#endif
