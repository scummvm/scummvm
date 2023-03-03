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

#include "ags/lib/std/math.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_dynamic_sprite.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/math.h"    // M_PI
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/ac/room_object.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/game/room_struct.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace Shared;
using namespace Engine;

// ** SCRIPT DYNAMIC SPRITE

void DynamicSprite_Delete(ScriptDynamicSprite *sds) {
	if (sds->slot) {
		free_dynamic_sprite(sds->slot);
		sds->slot = 0;
	}
}

ScriptDrawingSurface *DynamicSprite_GetDrawingSurface(ScriptDynamicSprite *dss) {
	ScriptDrawingSurface *surface = new ScriptDrawingSurface();
	surface->dynamicSpriteNumber = dss->slot;

	if ((_GP(game).SpriteInfos[dss->slot].Flags & SPF_ALPHACHANNEL) != 0)
		surface->hasAlphaChannel = true;

	ccRegisterManagedObject(surface, surface);
	return surface;
}

int DynamicSprite_GetGraphic(ScriptDynamicSprite *sds) {
	if (sds->slot == 0)
		quit("!DynamicSprite.Graphic: Cannot get graphic, sprite has been deleted");
	return sds->slot;
}

int DynamicSprite_GetWidth(ScriptDynamicSprite *sds) {
	return game_to_data_coord(_GP(game).SpriteInfos[sds->slot].Width);
}

int DynamicSprite_GetHeight(ScriptDynamicSprite *sds) {
	return game_to_data_coord(_GP(game).SpriteInfos[sds->slot].Height);
}

int DynamicSprite_GetColorDepth(ScriptDynamicSprite *sds) {
	int depth = _GP(spriteset)[sds->slot]->GetColorDepth();
	if (depth == 15)
		depth = 16;
	if (depth == 24)
		depth = 32;
	return depth;
}

void DynamicSprite_Resize(ScriptDynamicSprite *sds, int width, int height) {
	if ((width < 1) || (height < 1))
		quit("!DynamicSprite.Resize: width and height must be greater than zero");
	if (sds->slot == 0)
		quit("!DynamicSprite.Resize: sprite has been deleted");

	data_to_game_coords(&width, &height);

	if (width * height >= 25000000)
		quitprintf("!DynamicSprite.Resize: new size is too large: %d x %d", width, height);

	// resize the sprite to the requested size
	Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, _GP(spriteset)[sds->slot]->GetColorDepth());
	newPic->StretchBlt(_GP(spriteset)[sds->slot],
	                   RectWH(0, 0, _GP(game).SpriteInfos[sds->slot].Width, _GP(game).SpriteInfos[sds->slot].Height),
	                   RectWH(0, 0, width, height));

	delete _GP(spriteset)[sds->slot];

	// replace the bitmap in the sprite set
	add_dynamic_sprite(sds->slot, newPic, (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
	game_sprite_updated(sds->slot);
}

void DynamicSprite_Flip(ScriptDynamicSprite *sds, int direction) {
	if ((direction < 1) || (direction > 3))
		quit("!DynamicSprite.Flip: invalid direction");
	if (sds->slot == 0)
		quit("!DynamicSprite.Flip: sprite has been deleted");

	// resize the sprite to the requested size
	Bitmap *newPic = BitmapHelper::CreateTransparentBitmap(_GP(game).SpriteInfos[sds->slot].Width, _GP(game).SpriteInfos[sds->slot].Height, _GP(spriteset)[sds->slot]->GetColorDepth());

	// AGS script FlipDirection corresponds to internal GraphicFlip
	newPic->FlipBlt(_GP(spriteset)[sds->slot], 0, 0, static_cast<GraphicFlip>(direction));
	delete _GP(spriteset)[sds->slot];

	// replace the bitmap in the sprite set
	add_dynamic_sprite(sds->slot, newPic, (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
	game_sprite_updated(sds->slot);
}

void DynamicSprite_CopyTransparencyMask(ScriptDynamicSprite *sds, int sourceSprite) {
	if (sds->slot == 0)
		quit("!DynamicSprite.CopyTransparencyMask: sprite has been deleted");

	if ((_GP(game).SpriteInfos[sds->slot].Width != _GP(game).SpriteInfos[sourceSprite].Width) ||
	        (_GP(game).SpriteInfos[sds->slot].Height != _GP(game).SpriteInfos[sourceSprite].Height)) {
		quit("!DynamicSprite.CopyTransparencyMask: sprites are not the same size");
	}

	Bitmap *target = _GP(spriteset)[sds->slot];
	Bitmap *source = _GP(spriteset)[sourceSprite];

	if (target->GetColorDepth() != source->GetColorDepth()) {
		quit("!DynamicSprite.CopyTransparencyMask: sprites are not the same colour depth");
	}

	// set the target's alpha channel depending on the source
	bool dst_has_alpha = (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0;
	bool src_has_alpha = (_GP(game).SpriteInfos[sourceSprite].Flags & SPF_ALPHACHANNEL) != 0;
	_GP(game).SpriteInfos[sds->slot].Flags &= ~SPF_ALPHACHANNEL;
	if (src_has_alpha) {
		_GP(game).SpriteInfos[sds->slot].Flags |= SPF_ALPHACHANNEL;
	}

	BitmapHelper::CopyTransparency(target, source, dst_has_alpha, src_has_alpha);
	game_sprite_updated(sds->slot);
}

void DynamicSprite_ChangeCanvasSize(ScriptDynamicSprite *sds, int width, int height, int x, int y) {
	if (sds->slot == 0)
		quit("!DynamicSprite.ChangeCanvasSize: sprite has been deleted");
	if ((width < 1) || (height < 1))
		quit("!DynamicSprite.ChangeCanvasSize: new size is too small");

	data_to_game_coords(&x, &y);
	data_to_game_coords(&width, &height);

	Bitmap *newPic = BitmapHelper::CreateTransparentBitmap(width, height, _GP(spriteset)[sds->slot]->GetColorDepth());
	// blit it into the enlarged image
	newPic->Blit(_GP(spriteset)[sds->slot], 0, 0, x, y, _GP(game).SpriteInfos[sds->slot].Width, _GP(game).SpriteInfos[sds->slot].Height);

	delete _GP(spriteset)[sds->slot];

	// replace the bitmap in the sprite set
	add_dynamic_sprite(sds->slot, newPic, (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
	game_sprite_updated(sds->slot);
}

void DynamicSprite_Crop(ScriptDynamicSprite *sds, int x1, int y1, int width, int height) {
	if ((width < 1) || (height < 1))
		quit("!DynamicSprite.Crop: co-ordinates do not make sense");
	if (sds->slot == 0)
		quit("!DynamicSprite.Crop: sprite has been deleted");

	data_to_game_coords(&x1, &y1);
	data_to_game_coords(&width, &height);

	if ((width > _GP(game).SpriteInfos[sds->slot].Width) || (height > _GP(game).SpriteInfos[sds->slot].Height))
		quit("!DynamicSprite.Crop: requested to crop an area larger than the source");

	Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, _GP(spriteset)[sds->slot]->GetColorDepth());
	// blit it cropped
	newPic->Blit(_GP(spriteset)[sds->slot], x1, y1, 0, 0, newPic->GetWidth(), newPic->GetHeight());

	delete _GP(spriteset)[sds->slot];

	// replace the bitmap in the sprite set
	add_dynamic_sprite(sds->slot, newPic, (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
	game_sprite_updated(sds->slot);
}

void DynamicSprite_Rotate(ScriptDynamicSprite *sds, int angle, int width, int height) {
	if ((angle < 1) || (angle > 359))
		quit("!DynamicSprite.Rotate: invalid angle (must be 1-359)");
	if (sds->slot == 0)
		quit("!DynamicSprite.Rotate: sprite has been deleted");

	if ((width == SCR_NO_VALUE) || (height == SCR_NO_VALUE)) {
		// calculate the new image size automatically
		// 1 degree = 181 degrees in terms of x/y size, so % 180
		int useAngle = angle % 180;
		// and 0..90 is the same as 180..90
		if (useAngle > 90)
			useAngle = 180 - useAngle;
		// useAngle is now between 0 and 90 (otherwise the sin/cos stuff doesn't work)
		double angleInRadians = (double)useAngle * (M_PI / 180.0);
		double sinVal = sin(angleInRadians);
		double cosVal = cos(angleInRadians);

		width = (cosVal * (double)_GP(game).SpriteInfos[sds->slot].Width + sinVal * (double)_GP(game).SpriteInfos[sds->slot].Height);
		height = (sinVal * (double)_GP(game).SpriteInfos[sds->slot].Width + cosVal * (double)_GP(game).SpriteInfos[sds->slot].Height);
	} else {
		data_to_game_coords(&width, &height);
	}

	// convert to allegro angle
	angle = (angle * 256) / 360;

	// resize the sprite to the requested size
	Bitmap *newPic = BitmapHelper::CreateTransparentBitmap(width, height, _GP(spriteset)[sds->slot]->GetColorDepth());

	// rotate the sprite about its centre
	// (+ width%2 fixes one pixel offset problem)
	newPic->RotateBlt(_GP(spriteset)[sds->slot], width / 2 + width % 2, height / 2,
	                  _GP(game).SpriteInfos[sds->slot].Width / 2, _GP(game).SpriteInfos[sds->slot].Height / 2, itofix(angle));

	delete _GP(spriteset)[sds->slot];

	// replace the bitmap in the sprite set
	add_dynamic_sprite(sds->slot, newPic, (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
	game_sprite_updated(sds->slot);
}

void DynamicSprite_Tint(ScriptDynamicSprite *sds, int red, int green, int blue, int saturation, int luminance) {
	Bitmap *source = _GP(spriteset)[sds->slot];
	Bitmap *newPic = BitmapHelper::CreateBitmap(source->GetWidth(), source->GetHeight(), source->GetColorDepth());

	tint_image(newPic, source, red, green, blue, saturation, (luminance * 25) / 10);

	delete source;
	// replace the bitmap in the sprite set
	add_dynamic_sprite(sds->slot, newPic, (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
	game_sprite_updated(sds->slot);
}

int DynamicSprite_SaveToFile(ScriptDynamicSprite *sds, const char *namm) {
	if (sds->slot == 0)
		quit("!DynamicSprite.SaveToFile: sprite has been deleted");

	auto filename = String(namm);
	if (filename.FindChar('.') == String::NoIndex)
		filename.Append(".bmp");

	ResolvedPath rp;
	if (!ResolveWritePathAndCreateDirs(filename, rp))
		return 0;
	return _GP(spriteset)[sds->slot]->SaveToFile(rp.FullPath, _G(palette)) ? 1 : 0;
}

ScriptDynamicSprite *DynamicSprite_CreateFromSaveGame(int sgslot, int width, int height) {
	int slotnum = LoadSaveSlotScreenshot(sgslot, width, height);
	if (slotnum) {
		ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(slotnum);
		return new_spr;
	}
	return nullptr;
}

ScriptDynamicSprite *DynamicSprite_CreateFromFile(const char *filename) {
	int slotnum = LoadImageFile(filename);
	if (slotnum) {
		ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(slotnum);
		return new_spr;
	}
	return nullptr;
}

ScriptDynamicSprite *DynamicSprite_CreateFromScreenShot(int width, int height) {

	// TODO: refactor and merge with create_savegame_screenshot()

	int gotSlot = _GP(spriteset).GetFreeIndex();
	if (gotSlot <= 0)
		return nullptr;

	const Rect &viewport = _GP(play).GetMainViewport();
	if (width <= 0)
		width = viewport.GetWidth();
	else
		width = data_to_game_coord(width);

	if (height <= 0)
		height = viewport.GetHeight();
	else
		height = data_to_game_coord(height);

	Bitmap *newPic = CopyScreenIntoBitmap(width, height);

	// replace the bitmap in the sprite set
	add_dynamic_sprite(gotSlot, newPic);
	ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(gotSlot);
	return new_spr;
}

ScriptDynamicSprite *DynamicSprite_CreateFromExistingSprite(int slot, int preserveAlphaChannel) {

	int gotSlot = _GP(spriteset).GetFreeIndex();
	if (gotSlot <= 0)
		return nullptr;

	if (!_GP(spriteset).DoesSpriteExist(slot))
		quitprintf("DynamicSprite.CreateFromExistingSprite: sprite %d does not exist", slot);

	// create a new sprite as a copy of the existing one
	Bitmap *newPic = BitmapHelper::CreateBitmapCopy(_GP(spriteset)[slot]);
	if (newPic == nullptr)
		return nullptr;

	bool hasAlpha = (preserveAlphaChannel) && ((_GP(game).SpriteInfos[slot].Flags & SPF_ALPHACHANNEL) != 0);

	// replace the bitmap in the sprite set
	add_dynamic_sprite(gotSlot, newPic, hasAlpha);
	ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(gotSlot);
	return new_spr;
}

ScriptDynamicSprite *DynamicSprite_CreateFromDrawingSurface(ScriptDrawingSurface *sds, int x, int y, int width, int height) {
	int gotSlot = _GP(spriteset).GetFreeIndex();
	if (gotSlot <= 0)
		return nullptr;

	// use DrawingSurface resolution
	sds->PointToGameResolution(&x, &y);
	sds->SizeToGameResolution(&width, &height);

	Bitmap *ds = sds->StartDrawing();

	if ((x < 0) || (y < 0) || (x + width > ds->GetWidth()) || (y + height > ds->GetHeight()))
		quit("!DynamicSprite.CreateFromDrawingSurface: requested area is outside the surface");

	int colDepth = ds->GetColorDepth();

	Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, colDepth);
	if (newPic == nullptr)
		return nullptr;

	newPic->Blit(ds, x, y, 0, 0, width, height);

	sds->FinishedDrawingReadOnly();

	add_dynamic_sprite(gotSlot, newPic, (sds->hasAlphaChannel != 0));
	ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(gotSlot);
	return new_spr;
}

ScriptDynamicSprite *DynamicSprite_Create(int width, int height, int alphaChannel) {
	data_to_game_coords(&width, &height);

	int gotSlot = _GP(spriteset).GetFreeIndex();
	if (gotSlot <= 0)
		return nullptr;

	Bitmap *newPic = CreateCompatBitmap(width, height);
	if (newPic == nullptr)
		return nullptr;

	newPic->ClearTransparent();
	if ((alphaChannel) && (_GP(game).GetColorDepth() < 32))
		alphaChannel = false;

	add_dynamic_sprite(gotSlot, newPic, alphaChannel != 0);
	ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(gotSlot);
	return new_spr;
}

ScriptDynamicSprite *DynamicSprite_CreateFromExistingSprite_Old(int slot) {
	return DynamicSprite_CreateFromExistingSprite(slot, 0);
}

ScriptDynamicSprite *DynamicSprite_CreateFromBackground(int frame, int x1, int y1, int width, int height) {

	if (frame == SCR_NO_VALUE) {
		frame = _GP(play).bg_frame;
	} else if ((frame < 0) || ((size_t)frame >= _GP(thisroom).BgFrameCount))
		quit("!DynamicSprite.CreateFromBackground: invalid frame specified");

	if (x1 == SCR_NO_VALUE) {
		x1 = 0;
		y1 = 0;
		width = _GP(play).room_width;
		height = _GP(play).room_height;
	} else if ((x1 < 0) || (y1 < 0) || (width < 1) || (height < 1) ||
	           (x1 + width > _GP(play).room_width) || (y1 + height > _GP(play).room_height))
		quit("!DynamicSprite.CreateFromBackground: invalid co-ordinates specified");

	data_to_game_coords(&x1, &y1);
	data_to_game_coords(&width, &height);

	int gotSlot = _GP(spriteset).GetFreeIndex();
	if (gotSlot <= 0)
		return nullptr;

	// create a new sprite as a copy of the existing one
	Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, _GP(thisroom).BgFrames[frame].Graphic->GetColorDepth());
	if (newPic == nullptr)
		return nullptr;

	newPic->Blit(_GP(thisroom).BgFrames[frame].Graphic.get(), x1, y1, 0, 0, width, height);

	// replace the bitmap in the sprite set
	add_dynamic_sprite(gotSlot, newPic);
	ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(gotSlot);
	return new_spr;
}

//=============================================================================

void add_dynamic_sprite(int gotSlot, Bitmap *redin, bool hasAlpha) {

	_GP(spriteset).SetSprite(gotSlot, redin);

	_GP(game).SpriteInfos[gotSlot].Flags = SPF_DYNAMICALLOC;

	if (redin->GetColorDepth() > 8)
		_GP(game).SpriteInfos[gotSlot].Flags |= SPF_HICOLOR;
	if (redin->GetColorDepth() > 16)
		_GP(game).SpriteInfos[gotSlot].Flags |= SPF_TRUECOLOR;
	if (hasAlpha)
		_GP(game).SpriteInfos[gotSlot].Flags |= SPF_ALPHACHANNEL;

	_GP(game).SpriteInfos[gotSlot].Width = redin->GetWidth();
	_GP(game).SpriteInfos[gotSlot].Height = redin->GetHeight();
}

void free_dynamic_sprite(int gotSlot) {

	if ((gotSlot < 0) || ((size_t)gotSlot >= _GP(spriteset).GetSpriteSlotCount()))
		quit("!FreeDynamicSprite: invalid slot number");

	if ((_GP(game).SpriteInfos[gotSlot].Flags & SPF_DYNAMICALLOC) == 0)
		quitprintf("!DeleteSprite: Attempted to free static sprite %d that was not loaded by the script", gotSlot);

	_GP(spriteset).RemoveSprite(gotSlot, true);

	_GP(game).SpriteInfos[gotSlot].Flags = 0;
	_GP(game).SpriteInfos[gotSlot].Width = 0;
	_GP(game).SpriteInfos[gotSlot].Height = 0;

	game_sprite_deleted(gotSlot);
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// void (ScriptDynamicSprite *sds, int width, int height, int x, int y)
RuntimeScriptValue Sc_DynamicSprite_ChangeCanvasSize(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(ScriptDynamicSprite, DynamicSprite_ChangeCanvasSize);
}

// void (ScriptDynamicSprite *sds, int sourceSprite)
RuntimeScriptValue Sc_DynamicSprite_CopyTransparencyMask(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptDynamicSprite, DynamicSprite_CopyTransparencyMask);
}

// void (ScriptDynamicSprite *sds, int x1, int y1, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_Crop(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(ScriptDynamicSprite, DynamicSprite_Crop);
}

// void (ScriptDynamicSprite *sds)
RuntimeScriptValue Sc_DynamicSprite_Delete(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptDynamicSprite, DynamicSprite_Delete);
}

// void (ScriptDynamicSprite *sds, int direction)
RuntimeScriptValue Sc_DynamicSprite_Flip(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptDynamicSprite, DynamicSprite_Flip);
}

// ScriptDrawingSurface* (ScriptDynamicSprite *dss)
RuntimeScriptValue Sc_DynamicSprite_GetDrawingSurface(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJAUTO(ScriptDynamicSprite, ScriptDrawingSurface, DynamicSprite_GetDrawingSurface);
}

// void (ScriptDynamicSprite *sds, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_Resize(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(ScriptDynamicSprite, DynamicSprite_Resize);
}

// void (ScriptDynamicSprite *sds, int angle, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_Rotate(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT3(ScriptDynamicSprite, DynamicSprite_Rotate);
}

// int (ScriptDynamicSprite *sds, const char* namm)
RuntimeScriptValue Sc_DynamicSprite_SaveToFile(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(ScriptDynamicSprite, DynamicSprite_SaveToFile, const char);
}

// void (ScriptDynamicSprite *sds, int red, int green, int blue, int saturation, int luminance)
RuntimeScriptValue Sc_DynamicSprite_Tint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT5(ScriptDynamicSprite, DynamicSprite_Tint);
}

// int (ScriptDynamicSprite *sds)
RuntimeScriptValue Sc_DynamicSprite_GetColorDepth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDynamicSprite, DynamicSprite_GetColorDepth);
}

// int (ScriptDynamicSprite *sds)
RuntimeScriptValue Sc_DynamicSprite_GetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDynamicSprite, DynamicSprite_GetGraphic);
}

// int (ScriptDynamicSprite *sds)
RuntimeScriptValue Sc_DynamicSprite_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDynamicSprite, DynamicSprite_GetHeight);
}

// int (ScriptDynamicSprite *sds)
RuntimeScriptValue Sc_DynamicSprite_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDynamicSprite, DynamicSprite_GetWidth);
}

// ScriptDynamicSprite* (int width, int height, int alphaChannel)
RuntimeScriptValue Sc_DynamicSprite_Create(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT3(ScriptDynamicSprite, DynamicSprite_Create);
}

// ScriptDynamicSprite* (int frame, int x1, int y1, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_CreateFromBackground(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT5(ScriptDynamicSprite, DynamicSprite_CreateFromBackground);
}

// ScriptDynamicSprite* (ScriptDrawingSurface *sds, int x, int y, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_CreateFromDrawingSurface(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_POBJ_PINT4(ScriptDynamicSprite, DynamicSprite_CreateFromDrawingSurface, ScriptDrawingSurface);
}

// ScriptDynamicSprite* (int slot)
RuntimeScriptValue Sc_DynamicSprite_CreateFromExistingSprite_Old(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT(ScriptDynamicSprite, DynamicSprite_CreateFromExistingSprite_Old);
}

// ScriptDynamicSprite* (int slot, int preserveAlphaChannel)
RuntimeScriptValue Sc_DynamicSprite_CreateFromExistingSprite(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT2(ScriptDynamicSprite, DynamicSprite_CreateFromExistingSprite);
}

// ScriptDynamicSprite* (const char *filename)
RuntimeScriptValue Sc_DynamicSprite_CreateFromFile(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_POBJ(ScriptDynamicSprite, DynamicSprite_CreateFromFile, const char);
}

// ScriptDynamicSprite* (int sgslot, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_CreateFromSaveGame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT3(ScriptDynamicSprite, DynamicSprite_CreateFromSaveGame);
}

// ScriptDynamicSprite* (int width, int height)
RuntimeScriptValue Sc_DynamicSprite_CreateFromScreenShot(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT2(ScriptDynamicSprite, DynamicSprite_CreateFromScreenShot);
}


void RegisterDynamicSpriteAPI() {
	ccAddExternalObjectFunction("DynamicSprite::ChangeCanvasSize^4", Sc_DynamicSprite_ChangeCanvasSize);
	ccAddExternalObjectFunction("DynamicSprite::CopyTransparencyMask^1", Sc_DynamicSprite_CopyTransparencyMask);
	ccAddExternalObjectFunction("DynamicSprite::Crop^4", Sc_DynamicSprite_Crop);
	ccAddExternalObjectFunction("DynamicSprite::Delete", Sc_DynamicSprite_Delete);
	ccAddExternalObjectFunction("DynamicSprite::Flip^1", Sc_DynamicSprite_Flip);
	ccAddExternalObjectFunction("DynamicSprite::GetDrawingSurface^0", Sc_DynamicSprite_GetDrawingSurface);
	ccAddExternalObjectFunction("DynamicSprite::Resize^2", Sc_DynamicSprite_Resize);
	ccAddExternalObjectFunction("DynamicSprite::Rotate^3", Sc_DynamicSprite_Rotate);
	ccAddExternalObjectFunction("DynamicSprite::SaveToFile^1", Sc_DynamicSprite_SaveToFile);
	ccAddExternalObjectFunction("DynamicSprite::Tint^5", Sc_DynamicSprite_Tint);
	ccAddExternalObjectFunction("DynamicSprite::get_ColorDepth", Sc_DynamicSprite_GetColorDepth);
	ccAddExternalObjectFunction("DynamicSprite::get_Graphic", Sc_DynamicSprite_GetGraphic);
	ccAddExternalObjectFunction("DynamicSprite::get_Height", Sc_DynamicSprite_GetHeight);
	ccAddExternalObjectFunction("DynamicSprite::get_Width", Sc_DynamicSprite_GetWidth);
	ccAddExternalStaticFunction("DynamicSprite::Create^3", Sc_DynamicSprite_Create);
	ccAddExternalStaticFunction("DynamicSprite::CreateFromBackground", Sc_DynamicSprite_CreateFromBackground);
	ccAddExternalStaticFunction("DynamicSprite::CreateFromDrawingSurface^5", Sc_DynamicSprite_CreateFromDrawingSurface);
	ccAddExternalStaticFunction("DynamicSprite::CreateFromExistingSprite^1", Sc_DynamicSprite_CreateFromExistingSprite_Old);
	ccAddExternalStaticFunction("DynamicSprite::CreateFromExistingSprite^2", Sc_DynamicSprite_CreateFromExistingSprite);
	ccAddExternalStaticFunction("DynamicSprite::CreateFromFile", Sc_DynamicSprite_CreateFromFile);
	ccAddExternalStaticFunction("DynamicSprite::CreateFromSaveGame", Sc_DynamicSprite_CreateFromSaveGame);
	ccAddExternalStaticFunction("DynamicSprite::CreateFromScreenShot", Sc_DynamicSprite_CreateFromScreenShot);
}

} // namespace AGS3
