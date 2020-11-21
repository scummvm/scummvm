//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__DYNAMICSPRITE_H
#define __AGS_EE_AC__DYNAMICSPRITE_H

#include "ac/dynobj/scriptdynamicsprite.h"
#include "ac/dynobj/scriptdrawingsurface.h"

void	DynamicSprite_Delete(ScriptDynamicSprite *sds);
ScriptDrawingSurface* DynamicSprite_GetDrawingSurface(ScriptDynamicSprite *dss);
int		DynamicSprite_GetGraphic(ScriptDynamicSprite *sds);
int		DynamicSprite_GetWidth(ScriptDynamicSprite *sds);
int		DynamicSprite_GetHeight(ScriptDynamicSprite *sds);
int		DynamicSprite_GetColorDepth(ScriptDynamicSprite *sds);
void	DynamicSprite_Resize(ScriptDynamicSprite *sds, int width, int height);
void	DynamicSprite_Flip(ScriptDynamicSprite *sds, int direction);
void	DynamicSprite_CopyTransparencyMask(ScriptDynamicSprite *sds, int sourceSprite);
void	DynamicSprite_ChangeCanvasSize(ScriptDynamicSprite *sds, int width, int height, int x, int y);
void	DynamicSprite_Crop(ScriptDynamicSprite *sds, int x1, int y1, int width, int height);
void	DynamicSprite_Rotate(ScriptDynamicSprite *sds, int angle, int width, int height);
void	DynamicSprite_Tint(ScriptDynamicSprite *sds, int red, int green, int blue, int saturation, int luminance);
int		DynamicSprite_SaveToFile(ScriptDynamicSprite *sds, const char* namm);
ScriptDynamicSprite* DynamicSprite_CreateFromSaveGame(int sgslot, int width, int height);
ScriptDynamicSprite* DynamicSprite_CreateFromFile(const char *filename);
ScriptDynamicSprite* DynamicSprite_CreateFromScreenShot(int width, int height);
ScriptDynamicSprite* DynamicSprite_CreateFromExistingSprite(int slot, int preserveAlphaChannel);
ScriptDynamicSprite* DynamicSprite_CreateFromDrawingSurface(ScriptDrawingSurface *sds, int x, int y, int width, int height);
ScriptDynamicSprite* DynamicSprite_Create(int width, int height, int alphaChannel);
ScriptDynamicSprite* DynamicSprite_CreateFromExistingSprite_Old(int slot);
ScriptDynamicSprite* DynamicSprite_CreateFromBackground(int frame, int x1, int y1, int width, int height);


void	add_dynamic_sprite(int gotSlot, Common::Bitmap *redin, bool hasAlpha = false);
void	free_dynamic_sprite (int gotSlot);

#endif // __AGS_EE_AC__DYNAMICSPRITE_H
