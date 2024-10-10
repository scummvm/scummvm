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
#include "ags/engine/ac/dynobj/dynobj_manager.h"
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
	// Dynamic sprite ensures the sprite exists always
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
	Bitmap *sprite = _GP(spriteset)[sds->slot];
	std::unique_ptr<Bitmap> new_pic(BitmapHelper::CreateBitmap(width, height, sprite->GetColorDepth()));
	new_pic->StretchBlt(sprite,
					   RectWH(0, 0, _GP(game).SpriteInfos[sds->slot].Width, _GP(game).SpriteInfos[sds->slot].Height),
					   RectWH(0, 0, width, height));

	add_dynamic_sprite(sds->slot, std::move(new_pic), (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
	game_sprite_updated(sds->slot);
}

void DynamicSprite_Flip(ScriptDynamicSprite *sds, int direction) {
	if ((direction < 1) || (direction > 3))
		quit("!DynamicSprite.Flip: invalid direction");
	if (sds->slot == 0)
		quit("!DynamicSprite.Flip: sprite has been deleted");

	// resize the sprite to the requested size
	Bitmap *sprite = _GP(spriteset)[sds->slot];
	std::unique_ptr<Bitmap> new_pic(BitmapHelper::CreateBitmap(sprite->GetWidth(), sprite->GetHeight(), sprite->GetColorDepth()));

	// AGS script FlipDirection corresponds to internal GraphicFlip
	new_pic->FlipBlt(sprite, 0, 0, static_cast<GraphicFlip>(direction));

	add_dynamic_sprite(sds->slot, std::move(new_pic), (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
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

	Bitmap *sprite = _GP(spriteset)[sds->slot];
	std::unique_ptr<Bitmap> new_pic(BitmapHelper::CreateTransparentBitmap(width, height, sprite->GetColorDepth()));
	// blit it into the enlarged image
	new_pic->Blit(sprite, 0, 0, x, y, sprite->GetWidth(), sprite->GetHeight());

	// replace the bitmap in the sprite set
	add_dynamic_sprite(sds->slot, std::move(new_pic), (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
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

	Bitmap *sprite = _GP(spriteset)[sds->slot];
	std::unique_ptr<Bitmap> new_pic(BitmapHelper::CreateBitmap(width, height, sprite->GetColorDepth()));
	// blit it cropped
	new_pic->Blit(sprite, x1, y1, 0, 0, new_pic->GetWidth(), new_pic->GetHeight());

	// replace the bitmap in the sprite set
	add_dynamic_sprite(sds->slot, std::move(new_pic), (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
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
	Bitmap *sprite = _GP(spriteset)[sds->slot];
	std::unique_ptr<Bitmap> new_pic(BitmapHelper::CreateTransparentBitmap(width, height, sprite->GetColorDepth()));

	// rotate the sprite about its centre
	// (+ width%2 fixes one pixel offset problem)
	new_pic->RotateBlt(sprite, width / 2 + width % 2, height / 2,
					  sprite->GetWidth() / 2, sprite->GetHeight() / 2, itofix(angle));

	// replace the bitmap in the sprite set
	add_dynamic_sprite(sds->slot, std::move(new_pic), (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
	game_sprite_updated(sds->slot);
}

void DynamicSprite_Tint(ScriptDynamicSprite *sds, int red, int green, int blue, int saturation, int luminance) {
	Bitmap *source = _GP(spriteset)[sds->slot];
	std::unique_ptr<Bitmap> new_pic(BitmapHelper::CreateBitmap(source->GetWidth(), source->GetHeight(), source->GetColorDepth()));

	tint_image(new_pic.get(), source, red, green, blue, saturation, (luminance * 25) / 10);

	add_dynamic_sprite(sds->slot, std::move(new_pic), (_GP(game).SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
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

	if (!_GP(spriteset).HasFreeSlots())
		return nullptr;

	// NOTE: be aware that by the historical logic AGS makes a screenshot
	// of a "main viewport", that may be smaller in legacy "letterbox" mode.
	const Rect &viewport = _GP(play).GetMainViewport();
	if (width <= 0)
		width = viewport.GetWidth();
	else
		width = data_to_game_coord(width);

	if (height <= 0)
		height = viewport.GetHeight();
	else
		height = data_to_game_coord(height);

	std::unique_ptr<Bitmap> new_pic(CopyScreenIntoBitmap(width, height, &viewport));

	// replace the bitmap in the sprite set
	int new_slot = add_dynamic_sprite(std::move(new_pic));
	return new ScriptDynamicSprite(new_slot);
}

ScriptDynamicSprite *DynamicSprite_CreateFromExistingSprite(int slot, int preserveAlphaChannel) {

	if (!_GP(spriteset).HasFreeSlots())
		return nullptr;

	if (!_GP(spriteset).DoesSpriteExist(slot))
		quitprintf("DynamicSprite.CreateFromExistingSprite: sprite %d does not exist", slot);

	// create a new sprite as a copy of the existing one
	std::unique_ptr<Bitmap> new_pic(BitmapHelper::CreateBitmapCopy(_GP(spriteset)[slot]));
	if (!new_pic)
		return nullptr;

	bool hasAlpha = (preserveAlphaChannel) && ((_GP(game).SpriteInfos[slot].Flags & SPF_ALPHACHANNEL) != 0);
	int new_slot = add_dynamic_sprite(std::move(new_pic), hasAlpha);
	return new ScriptDynamicSprite(new_slot);
}

ScriptDynamicSprite *DynamicSprite_CreateFromDrawingSurface(ScriptDrawingSurface *sds, int x, int y, int width, int height) {
	if (!_GP(spriteset).HasFreeSlots())
		return nullptr;

	if (width <= 0 || height <= 0) {
		debug_script_warn("WARNING: DynamicSprite.CreateFromDrawingSurface: invalid size %d x %d, will adjust", width, height);
		width = std::max(1, width);
		height = std::max(1, height);
	}

	// use DrawingSurface resolution
	sds->PointToGameResolution(&x, &y);
	sds->SizeToGameResolution(&width, &height);

	Bitmap *ds = sds->StartDrawing();
	if ((x < 0) || (y < 0) || (x + width > ds->GetWidth()) || (y + height > ds->GetHeight()))
		quit("!DynamicSprite.CreateFromDrawingSurface: requested area is outside the surface");

	std::unique_ptr<Bitmap> new_pic(BitmapHelper::CreateBitmap(width, height, ds->GetColorDepth()));
	if (!new_pic)
		return nullptr;

	new_pic->Blit(ds, x, y, 0, 0, width, height);

	sds->FinishedDrawingReadOnly();

	int new_slot = add_dynamic_sprite(std::move(new_pic), (sds->hasAlphaChannel != 0));
	return new ScriptDynamicSprite(new_slot);
}

ScriptDynamicSprite *DynamicSprite_Create(int width, int height, int alphaChannel) {
	if (width <= 0 || height <= 0) {
		debug_script_warn("WARNING: DynamicSprite.Create: invalid size %d x %d, will adjust", width, height);
		width = MAX(1, width);
		height = MAX(1, height);
	}

	data_to_game_coords(&width, &height);

	if (!_GP(spriteset).HasFreeSlots())
		return nullptr;

	std::unique_ptr<Bitmap> new_pic(CreateCompatBitmap(width, height));
	if (!new_pic)
		return nullptr;

	new_pic->ClearTransparent();
	if ((alphaChannel) && (_GP(game).GetColorDepth() < 32))
		alphaChannel = false;

	int new_slot = add_dynamic_sprite(std::move(new_pic), alphaChannel != 0);
	return new ScriptDynamicSprite(new_slot);
}

ScriptDynamicSprite *DynamicSprite_CreateFromExistingSprite_Old(int slot) {
	return DynamicSprite_CreateFromExistingSprite(slot, 0);
}

ScriptDynamicSprite *DynamicSprite_CreateFromBackground(int frame, int x1, int y1, int width, int height) {
	if (frame == SCR_NO_VALUE) {
		frame = _GP(play).bg_frame;
	} else if ((frame < 0) || ((size_t)frame >= _GP(thisroom).BgFrameCount))
		quit("!DynamicSprite.CreateFromBackground: invalid frame specified");

	if (x1 == SCR_NO_VALUE)
		x1 = 0;
	if (y1 == SCR_NO_VALUE)
		y1 = 0;
	if (width == SCR_NO_VALUE)
		width = _GP(play).room_width;
	if (height == SCR_NO_VALUE)
		height = _GP(play).room_height;

	if (width <= 0 || height <= 0) {
		debug_script_warn("WARNING: DynamicSprite.CreateFromBackground: invalid size %d x %d, will adjust", width, height);
		width = std::max(1, width);
		height = std::max(1, height);
	}

	if ((x1 < 0) || (y1 < 0) || (x1 + width > _GP(play).room_width) || (y1 + height > _GP(play).room_height))
		quit("!DynamicSprite.CreateFromBackground: invalid co-ordinates specified");

	data_to_game_coords(&x1, &y1);
	data_to_game_coords(&width, &height);

	if (!_GP(spriteset).HasFreeSlots())
		return nullptr;

	// create a new sprite as a copy of the existing one
	std::unique_ptr<Bitmap> new_pic(BitmapHelper::CreateBitmap(width, height, _GP(thisroom).BgFrames[frame].Graphic->GetColorDepth()));
	if (!new_pic)
		return nullptr;

	new_pic->Blit(_GP(thisroom).BgFrames[frame].Graphic.get(), x1, y1, 0, 0, width, height);

	int new_slot = add_dynamic_sprite(std::move(new_pic));
	return new ScriptDynamicSprite(new_slot);
}

//=============================================================================

int add_dynamic_sprite(std::unique_ptr<Bitmap> image, bool has_alpha, uint32_t extra_flags) {
	int slot = _GP(spriteset).GetFreeIndex();
	if (slot <= 0)
		return 0;

	return add_dynamic_sprite(slot, std::move(image), has_alpha, extra_flags);
}

int add_dynamic_sprite(int slot, std::unique_ptr<Bitmap> image, bool has_alpha, uint32_t extra_flags) {
	assert(slot > 0 && !_GP(spriteset).IsAssetSprite(slot));
	if (slot <= 0 || _GP(spriteset).IsAssetSprite(slot))
		return 0; // invalid slot, or reserved for the static sprite

	uint32_t flags = SPF_DYNAMICALLOC | (SPF_ALPHACHANNEL * has_alpha) | extra_flags;

	if(!_GP(spriteset).SetSprite(slot, std::move(image), flags))
		return 0; // failed to add the sprite, bad image or realloc failed
	return slot;
}

void free_dynamic_sprite(int slot, bool notify_all) {
	assert((slot > 0) && (static_cast<size_t>(slot) < _GP(game).SpriteInfos.size()) &&
		   (_GP(game).SpriteInfos[slot].Flags & SPF_DYNAMICALLOC));
	if ((slot <= 0) || (static_cast<size_t>(slot) >= _GP(game).SpriteInfos.size()) ||
		(_GP(game).SpriteInfos[slot].Flags & SPF_DYNAMICALLOC) == 0)
		return;

	_GP(spriteset).DisposeSprite(slot);
	if (notify_all)
		game_sprite_updated(slot, true);
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
	ScFnRegister dynsprite_api[] = {
		{"DynamicSprite::Create^3", API_FN_PAIR(DynamicSprite_Create)},
		{"DynamicSprite::CreateFromBackground", API_FN_PAIR(DynamicSprite_CreateFromBackground)},
		{"DynamicSprite::CreateFromDrawingSurface^5", API_FN_PAIR(DynamicSprite_CreateFromDrawingSurface)},
		{"DynamicSprite::CreateFromExistingSprite^1", API_FN_PAIR(DynamicSprite_CreateFromExistingSprite_Old)},
		{"DynamicSprite::CreateFromExistingSprite^2", API_FN_PAIR(DynamicSprite_CreateFromExistingSprite)},
		{"DynamicSprite::CreateFromFile", API_FN_PAIR(DynamicSprite_CreateFromFile)},
		{"DynamicSprite::CreateFromSaveGame", API_FN_PAIR(DynamicSprite_CreateFromSaveGame)},
		{"DynamicSprite::CreateFromScreenShot", API_FN_PAIR(DynamicSprite_CreateFromScreenShot)},

		{"DynamicSprite::ChangeCanvasSize^4", API_FN_PAIR(DynamicSprite_ChangeCanvasSize)},
		{"DynamicSprite::CopyTransparencyMask^1", API_FN_PAIR(DynamicSprite_CopyTransparencyMask)},
		{"DynamicSprite::Crop^4", API_FN_PAIR(DynamicSprite_Crop)},
		{"DynamicSprite::Delete", API_FN_PAIR(DynamicSprite_Delete)},
		{"DynamicSprite::Flip^1", API_FN_PAIR(DynamicSprite_Flip)},
		{"DynamicSprite::GetDrawingSurface^0", API_FN_PAIR(DynamicSprite_GetDrawingSurface)},
		{"DynamicSprite::Resize^2", API_FN_PAIR(DynamicSprite_Resize)},
		{"DynamicSprite::Rotate^3", API_FN_PAIR(DynamicSprite_Rotate)},
		{"DynamicSprite::SaveToFile^1", API_FN_PAIR(DynamicSprite_SaveToFile)},
		{"DynamicSprite::Tint^5", API_FN_PAIR(DynamicSprite_Tint)},
		{"DynamicSprite::get_ColorDepth", API_FN_PAIR(DynamicSprite_GetColorDepth)},
		{"DynamicSprite::get_Graphic", API_FN_PAIR(DynamicSprite_GetGraphic)},
		{"DynamicSprite::get_Height", API_FN_PAIR(DynamicSprite_GetHeight)},
		{"DynamicSprite::get_Width", API_FN_PAIR(DynamicSprite_GetWidth)},
	};

	ccAddExternalFunctions361(dynsprite_api);
}

} // namespace AGS3
