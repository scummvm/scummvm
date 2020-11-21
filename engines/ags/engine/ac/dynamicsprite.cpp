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

#include <math.h>
#include "ac/dynamicsprite.h"
#include "ac/common.h"
#include "ac/charactercache.h"
#include "ac/draw.h"
#include "ac/gamesetupstruct.h"
#include "ac/global_dynamicsprite.h"
#include "ac/global_game.h"
#include "ac/math.h"    // M_PI
#include "ac/objectcache.h"
#include "ac/path_helper.h"
#include "ac/roomobject.h"
#include "ac/roomstatus.h"
#include "ac/system.h"
#include "debug/debug_log.h"
#include "game/roomstruct.h"
#include "gui/guibutton.h"
#include "ac/spritecache.h"
#include "gfx/graphicsdriver.h"
#include "script/runtimescriptvalue.h"

using namespace Common;
using namespace Engine;

extern GameSetupStruct game;
extern SpriteCache spriteset;
extern RoomStruct thisroom;
extern RoomObject*objs;
extern RoomStatus*croom;
extern CharacterCache *charcache;
extern ObjectCache objcache[MAX_ROOM_OBJECTS];

extern color palette[256];
extern AGS::Engine::IGraphicsDriver *gfxDriver;

char check_dynamic_sprites_at_exit = 1;

// ** SCRIPT DYNAMIC SPRITE

void DynamicSprite_Delete(ScriptDynamicSprite *sds) {
    if (sds->slot) {
        free_dynamic_sprite(sds->slot);
        sds->slot = 0;
    }
}

ScriptDrawingSurface* DynamicSprite_GetDrawingSurface(ScriptDynamicSprite *dss)
{
    ScriptDrawingSurface *surface = new ScriptDrawingSurface();
    surface->dynamicSpriteNumber = dss->slot;

    if ((game.SpriteInfos[dss->slot].Flags & SPF_ALPHACHANNEL) != 0)
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
    return game_to_data_coord(game.SpriteInfos[sds->slot].Width);
}

int DynamicSprite_GetHeight(ScriptDynamicSprite *sds) {
    return game_to_data_coord(game.SpriteInfos[sds->slot].Height);
}

int DynamicSprite_GetColorDepth(ScriptDynamicSprite *sds) {
    int depth = spriteset[sds->slot]->GetColorDepth();
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
    Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, spriteset[sds->slot]->GetColorDepth());
    newPic->StretchBlt(spriteset[sds->slot],
        RectWH(0, 0, game.SpriteInfos[sds->slot].Width, game.SpriteInfos[sds->slot].Height),
        RectWH(0, 0, width, height));

    delete spriteset[sds->slot];

    // replace the bitmap in the sprite set
    add_dynamic_sprite(sds->slot, newPic, (game.SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
}

void DynamicSprite_Flip(ScriptDynamicSprite *sds, int direction) {
    if ((direction < 1) || (direction > 3))
        quit("!DynamicSprite.Flip: invalid direction");
    if (sds->slot == 0)
        quit("!DynamicSprite.Flip: sprite has been deleted");

    // resize the sprite to the requested size
    Bitmap *newPic = BitmapHelper::CreateTransparentBitmap(game.SpriteInfos[sds->slot].Width, game.SpriteInfos[sds->slot].Height, spriteset[sds->slot]->GetColorDepth());

    if (direction == 1)
        newPic->FlipBlt(spriteset[sds->slot], 0, 0, Common::kBitmap_HFlip);
    else if (direction == 2)
        newPic->FlipBlt(spriteset[sds->slot], 0, 0, Common::kBitmap_VFlip);
    else if (direction == 3)
        newPic->FlipBlt(spriteset[sds->slot], 0, 0, Common::kBitmap_HVFlip);

    delete spriteset[sds->slot];

    // replace the bitmap in the sprite set
    add_dynamic_sprite(sds->slot, newPic, (game.SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
}

void DynamicSprite_CopyTransparencyMask(ScriptDynamicSprite *sds, int sourceSprite) {
    if (sds->slot == 0)
        quit("!DynamicSprite.CopyTransparencyMask: sprite has been deleted");

    if ((game.SpriteInfos[sds->slot].Width != game.SpriteInfos[sourceSprite].Width) ||
        (game.SpriteInfos[sds->slot].Height != game.SpriteInfos[sourceSprite].Height))
    {
        quit("!DynamicSprite.CopyTransparencyMask: sprites are not the same size");
    }

    Bitmap *target = spriteset[sds->slot];
    Bitmap *source = spriteset[sourceSprite];

    if (target->GetColorDepth() != source->GetColorDepth())
    {
        quit("!DynamicSprite.CopyTransparencyMask: sprites are not the same colour depth");
    }

    // set the target's alpha channel depending on the source
    bool dst_has_alpha = (game.SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0;
    bool src_has_alpha = (game.SpriteInfos[sourceSprite].Flags & SPF_ALPHACHANNEL) != 0;
    game.SpriteInfos[sds->slot].Flags &= ~SPF_ALPHACHANNEL;
    if (src_has_alpha)
    {
        game.SpriteInfos[sds->slot].Flags |= SPF_ALPHACHANNEL;
    }

    BitmapHelper::CopyTransparency(target, source, dst_has_alpha, src_has_alpha);
}

void DynamicSprite_ChangeCanvasSize(ScriptDynamicSprite *sds, int width, int height, int x, int y) 
{
    if (sds->slot == 0)
        quit("!DynamicSprite.ChangeCanvasSize: sprite has been deleted");
    if ((width < 1) || (height < 1))
        quit("!DynamicSprite.ChangeCanvasSize: new size is too small");

    data_to_game_coords(&x, &y);
    data_to_game_coords(&width, &height);

    Bitmap *newPic = BitmapHelper::CreateTransparentBitmap(width, height, spriteset[sds->slot]->GetColorDepth());
    // blit it into the enlarged image
    newPic->Blit(spriteset[sds->slot], 0, 0, x, y, game.SpriteInfos[sds->slot].Width, game.SpriteInfos[sds->slot].Height);

    delete spriteset[sds->slot];

    // replace the bitmap in the sprite set
    add_dynamic_sprite(sds->slot, newPic, (game.SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
}

void DynamicSprite_Crop(ScriptDynamicSprite *sds, int x1, int y1, int width, int height) {
    if ((width < 1) || (height < 1))
        quit("!DynamicSprite.Crop: co-ordinates do not make sense");
    if (sds->slot == 0)
        quit("!DynamicSprite.Crop: sprite has been deleted");

    data_to_game_coords(&x1, &y1);
    data_to_game_coords(&width, &height);

    if ((width > game.SpriteInfos[sds->slot].Width) || (height > game.SpriteInfos[sds->slot].Height))
        quit("!DynamicSprite.Crop: requested to crop an area larger than the source");

    Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, spriteset[sds->slot]->GetColorDepth());
    // blit it cropped
    newPic->Blit(spriteset[sds->slot], x1, y1, 0, 0, newPic->GetWidth(), newPic->GetHeight());

    delete spriteset[sds->slot];

    // replace the bitmap in the sprite set
    add_dynamic_sprite(sds->slot, newPic, (game.SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
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

        width = (cosVal * (double)game.SpriteInfos[sds->slot].Width + sinVal * (double)game.SpriteInfos[sds->slot].Height);
        height = (sinVal * (double)game.SpriteInfos[sds->slot].Width + cosVal * (double)game.SpriteInfos[sds->slot].Height);
    }
    else {
        data_to_game_coords(&width, &height);
    }

    // convert to allegro angle
    angle = (angle * 256) / 360;

    // resize the sprite to the requested size
    Bitmap *newPic = BitmapHelper::CreateTransparentBitmap(width, height, spriteset[sds->slot]->GetColorDepth());

    // rotate the sprite about its centre
    // (+ width%2 fixes one pixel offset problem)
    newPic->RotateBlt(spriteset[sds->slot], width / 2 + width % 2, height / 2,
        game.SpriteInfos[sds->slot].Width / 2, game.SpriteInfos[sds->slot].Height / 2, itofix(angle));

    delete spriteset[sds->slot];

    // replace the bitmap in the sprite set
    add_dynamic_sprite(sds->slot, newPic, (game.SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
}

void DynamicSprite_Tint(ScriptDynamicSprite *sds, int red, int green, int blue, int saturation, int luminance) 
{
    Bitmap *source = spriteset[sds->slot];
    Bitmap *newPic = BitmapHelper::CreateBitmap(source->GetWidth(), source->GetHeight(), source->GetColorDepth());

    tint_image(newPic, source, red, green, blue, saturation, (luminance * 25) / 10);

    delete source;
    // replace the bitmap in the sprite set
    add_dynamic_sprite(sds->slot, newPic, (game.SpriteInfos[sds->slot].Flags & SPF_ALPHACHANNEL) != 0);
}

int DynamicSprite_SaveToFile(ScriptDynamicSprite *sds, const char* namm)
{
    if (sds->slot == 0)
        quit("!DynamicSprite.SaveToFile: sprite has been deleted");

    auto filename = String(namm);
    if (filename.FindChar('.') == -1)
        filename.Append(".bmp");

    ResolvedPath rp;
    if (!ResolveWritePathAndCreateDirs(filename, rp))
        return 0;
    return spriteset[sds->slot]->SaveToFile(rp.FullPath, palette) ? 1 : 0;
}

ScriptDynamicSprite* DynamicSprite_CreateFromSaveGame(int sgslot, int width, int height) {
    int slotnum = LoadSaveSlotScreenshot(sgslot, width, height);
    if (slotnum) {
        ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(slotnum);
        return new_spr;
    }
    return nullptr;
}

ScriptDynamicSprite* DynamicSprite_CreateFromFile(const char *filename) {
    int slotnum = LoadImageFile(filename);
    if (slotnum) {
        ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(slotnum);
        return new_spr;
    }
    return nullptr;
}

ScriptDynamicSprite* DynamicSprite_CreateFromScreenShot(int width, int height) {

    // TODO: refactor and merge with create_savegame_screenshot()

    int gotSlot = spriteset.GetFreeIndex();
    if (gotSlot <= 0)
        return nullptr;

    const Rect &viewport = play.GetMainViewport();
    if (width <= 0)
        width = viewport.GetWidth();
    else
        width = data_to_game_coord(width);

    if (height <= 0)
        height = viewport.GetHeight();
    else
        height = data_to_game_coord(height);

    Bitmap *newPic = CopyScreenIntoBitmap(width, height);

    update_polled_stuff_if_runtime();

    // replace the bitmap in the sprite set
    add_dynamic_sprite(gotSlot, ReplaceBitmapWithSupportedFormat(newPic));
    ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(gotSlot);
    return new_spr;
}

ScriptDynamicSprite* DynamicSprite_CreateFromExistingSprite(int slot, int preserveAlphaChannel) {

    int gotSlot = spriteset.GetFreeIndex();
    if (gotSlot <= 0)
        return nullptr;

    if (!spriteset.DoesSpriteExist(slot))
        quitprintf("DynamicSprite.CreateFromExistingSprite: sprite %d does not exist", slot);

    // create a new sprite as a copy of the existing one
    Bitmap *newPic = BitmapHelper::CreateBitmapCopy(spriteset[slot]);
    if (newPic == nullptr)
        return nullptr;

    bool hasAlpha = (preserveAlphaChannel) && ((game.SpriteInfos[slot].Flags & SPF_ALPHACHANNEL) != 0);

    // replace the bitmap in the sprite set
    add_dynamic_sprite(gotSlot, newPic, hasAlpha);
    ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(gotSlot);
    return new_spr;
}

ScriptDynamicSprite* DynamicSprite_CreateFromDrawingSurface(ScriptDrawingSurface *sds, int x, int y, int width, int height) 
{
    int gotSlot = spriteset.GetFreeIndex();
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

ScriptDynamicSprite* DynamicSprite_Create(int width, int height, int alphaChannel) 
{
    data_to_game_coords(&width, &height);

    int gotSlot = spriteset.GetFreeIndex();
    if (gotSlot <= 0)
        return nullptr;

    Bitmap *newPic = BitmapHelper::CreateTransparentBitmap(width, height, game.GetColorDepth());
    if (newPic == nullptr)
        return nullptr;

    if ((alphaChannel) && (game.GetColorDepth() < 32))
        alphaChannel = false;

    add_dynamic_sprite(gotSlot, ReplaceBitmapWithSupportedFormat(newPic), alphaChannel != 0);
    ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(gotSlot);
    return new_spr;
}

ScriptDynamicSprite* DynamicSprite_CreateFromExistingSprite_Old(int slot) 
{
    return DynamicSprite_CreateFromExistingSprite(slot, 0);
}

ScriptDynamicSprite* DynamicSprite_CreateFromBackground(int frame, int x1, int y1, int width, int height) {

    if (frame == SCR_NO_VALUE) {
        frame = play.bg_frame;
    }
    else if ((frame < 0) || ((size_t)frame >= thisroom.BgFrameCount))
        quit("!DynamicSprite.CreateFromBackground: invalid frame specified");

    if (x1 == SCR_NO_VALUE) {
        x1 = 0;
        y1 = 0;
        width = play.room_width;
        height = play.room_height;
    }
    else if ((x1 < 0) || (y1 < 0) || (width < 1) || (height < 1) ||
        (x1 + width > play.room_width) || (y1 + height > play.room_height))
        quit("!DynamicSprite.CreateFromBackground: invalid co-ordinates specified");

    data_to_game_coords(&x1, &y1);
    data_to_game_coords(&width, &height);

    int gotSlot = spriteset.GetFreeIndex();
    if (gotSlot <= 0)
        return nullptr;

    // create a new sprite as a copy of the existing one
    Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, thisroom.BgFrames[frame].Graphic->GetColorDepth());
    if (newPic == nullptr)
        return nullptr;

    newPic->Blit(thisroom.BgFrames[frame].Graphic.get(), x1, y1, 0, 0, width, height);

    // replace the bitmap in the sprite set
    add_dynamic_sprite(gotSlot, newPic);
    ScriptDynamicSprite *new_spr = new ScriptDynamicSprite(gotSlot);
    return new_spr;
}

//=============================================================================

void add_dynamic_sprite(int gotSlot, Bitmap *redin, bool hasAlpha) {

  spriteset.SetSprite(gotSlot, redin);

  game.SpriteInfos[gotSlot].Flags = SPF_DYNAMICALLOC;

  if (redin->GetColorDepth() > 8)
    game.SpriteInfos[gotSlot].Flags |= SPF_HICOLOR;
  if (redin->GetColorDepth() > 16)
    game.SpriteInfos[gotSlot].Flags |= SPF_TRUECOLOR;
  if (hasAlpha)
    game.SpriteInfos[gotSlot].Flags |= SPF_ALPHACHANNEL;

  game.SpriteInfos[gotSlot].Width = redin->GetWidth();
  game.SpriteInfos[gotSlot].Height = redin->GetHeight();
}

void free_dynamic_sprite (int gotSlot) {
  int tt;

  if ((gotSlot < 0) || (gotSlot >= spriteset.GetSpriteSlotCount()))
    quit("!FreeDynamicSprite: invalid slot number");

  if ((game.SpriteInfos[gotSlot].Flags & SPF_DYNAMICALLOC) == 0)
    quitprintf("!DeleteSprite: Attempted to free static sprite %d that was not loaded by the script", gotSlot);

  spriteset.RemoveSprite(gotSlot, true);

  game.SpriteInfos[gotSlot].Flags = 0;
  game.SpriteInfos[gotSlot].Width = 0;
  game.SpriteInfos[gotSlot].Height = 0;

  // ensure it isn't still on any GUI buttons
  for (tt = 0; tt < numguibuts; tt++) {
    if (guibuts[tt].IsDeleted())
      continue;
    if (guibuts[tt].Image == gotSlot)
      guibuts[tt].Image = 0;
    if (guibuts[tt].CurrentImage == gotSlot)
      guibuts[tt].CurrentImage = 0;
    if (guibuts[tt].MouseOverImage == gotSlot)
      guibuts[tt].MouseOverImage = 0;
    if (guibuts[tt].PushedImage == gotSlot)
      guibuts[tt].PushedImage = 0;
  }

  // force refresh of any object caches using the sprite
  if (croom != nullptr) 
  {
    for (tt = 0; tt < croom->numobj; tt++) 
    {
      if (objs[tt].num == gotSlot)
      {
        objs[tt].num = 0;
        objcache[tt].sppic = -1;
      }
      else if (objcache[tt].sppic == gotSlot)
        objcache[tt].sppic = -1;
    }
  }
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

#include "debug/out.h"
#include "script/script_api.h"
#include "script/script_runtime.h"

// void (ScriptDynamicSprite *sds, int width, int height, int x, int y)
RuntimeScriptValue Sc_DynamicSprite_ChangeCanvasSize(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT4(ScriptDynamicSprite, DynamicSprite_ChangeCanvasSize);
}

// void (ScriptDynamicSprite *sds, int sourceSprite)
RuntimeScriptValue Sc_DynamicSprite_CopyTransparencyMask(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptDynamicSprite, DynamicSprite_CopyTransparencyMask);
}

// void (ScriptDynamicSprite *sds, int x1, int y1, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_Crop(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT4(ScriptDynamicSprite, DynamicSprite_Crop);
}

// void (ScriptDynamicSprite *sds)
RuntimeScriptValue Sc_DynamicSprite_Delete(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID(ScriptDynamicSprite, DynamicSprite_Delete);
}

// void (ScriptDynamicSprite *sds, int direction)
RuntimeScriptValue Sc_DynamicSprite_Flip(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptDynamicSprite, DynamicSprite_Flip);
}

// ScriptDrawingSurface* (ScriptDynamicSprite *dss)
RuntimeScriptValue Sc_DynamicSprite_GetDrawingSurface(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_OBJAUTO(ScriptDynamicSprite, ScriptDrawingSurface, DynamicSprite_GetDrawingSurface);
}

// void (ScriptDynamicSprite *sds, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_Resize(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT2(ScriptDynamicSprite, DynamicSprite_Resize);
}

// void (ScriptDynamicSprite *sds, int angle, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_Rotate(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT3(ScriptDynamicSprite, DynamicSprite_Rotate);
}

// int (ScriptDynamicSprite *sds, const char* namm)
RuntimeScriptValue Sc_DynamicSprite_SaveToFile(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT_POBJ(ScriptDynamicSprite, DynamicSprite_SaveToFile, const char);
}

// void (ScriptDynamicSprite *sds, int red, int green, int blue, int saturation, int luminance)
RuntimeScriptValue Sc_DynamicSprite_Tint(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT5(ScriptDynamicSprite, DynamicSprite_Tint);
}

// int (ScriptDynamicSprite *sds)
RuntimeScriptValue Sc_DynamicSprite_GetColorDepth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDynamicSprite, DynamicSprite_GetColorDepth);
}

// int (ScriptDynamicSprite *sds)
RuntimeScriptValue Sc_DynamicSprite_GetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDynamicSprite, DynamicSprite_GetGraphic);
}

// int (ScriptDynamicSprite *sds)
RuntimeScriptValue Sc_DynamicSprite_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDynamicSprite, DynamicSprite_GetHeight);
}

// int (ScriptDynamicSprite *sds)
RuntimeScriptValue Sc_DynamicSprite_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDynamicSprite, DynamicSprite_GetWidth);
}

// ScriptDynamicSprite* (int width, int height, int alphaChannel)
RuntimeScriptValue Sc_DynamicSprite_Create(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJAUTO_PINT3(ScriptDynamicSprite, DynamicSprite_Create);
}

// ScriptDynamicSprite* (int frame, int x1, int y1, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_CreateFromBackground(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJAUTO_PINT5(ScriptDynamicSprite, DynamicSprite_CreateFromBackground);
}

// ScriptDynamicSprite* (ScriptDrawingSurface *sds, int x, int y, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_CreateFromDrawingSurface(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJAUTO_POBJ_PINT4(ScriptDynamicSprite, DynamicSprite_CreateFromDrawingSurface, ScriptDrawingSurface);
}

// ScriptDynamicSprite* (int slot)
RuntimeScriptValue Sc_DynamicSprite_CreateFromExistingSprite_Old(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJAUTO_PINT(ScriptDynamicSprite, DynamicSprite_CreateFromExistingSprite_Old);
}

// ScriptDynamicSprite* (int slot, int preserveAlphaChannel)
RuntimeScriptValue Sc_DynamicSprite_CreateFromExistingSprite(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJAUTO_PINT2(ScriptDynamicSprite, DynamicSprite_CreateFromExistingSprite);
}

// ScriptDynamicSprite* (const char *filename)
RuntimeScriptValue Sc_DynamicSprite_CreateFromFile(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJAUTO_POBJ(ScriptDynamicSprite, DynamicSprite_CreateFromFile, const char);
}

// ScriptDynamicSprite* (int sgslot, int width, int height)
RuntimeScriptValue Sc_DynamicSprite_CreateFromSaveGame(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJAUTO_PINT3(ScriptDynamicSprite, DynamicSprite_CreateFromSaveGame);
}

// ScriptDynamicSprite* (int width, int height)
RuntimeScriptValue Sc_DynamicSprite_CreateFromScreenShot(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJAUTO_PINT2(ScriptDynamicSprite, DynamicSprite_CreateFromScreenShot);
}


void RegisterDynamicSpriteAPI()
{
    ccAddExternalObjectFunction("DynamicSprite::ChangeCanvasSize^4",        Sc_DynamicSprite_ChangeCanvasSize);
    ccAddExternalObjectFunction("DynamicSprite::CopyTransparencyMask^1",    Sc_DynamicSprite_CopyTransparencyMask);
    ccAddExternalObjectFunction("DynamicSprite::Crop^4",                    Sc_DynamicSprite_Crop);
    ccAddExternalObjectFunction("DynamicSprite::Delete",                    Sc_DynamicSprite_Delete);
    ccAddExternalObjectFunction("DynamicSprite::Flip^1",                    Sc_DynamicSprite_Flip);
    ccAddExternalObjectFunction("DynamicSprite::GetDrawingSurface^0",       Sc_DynamicSprite_GetDrawingSurface);
    ccAddExternalObjectFunction("DynamicSprite::Resize^2",                  Sc_DynamicSprite_Resize);
    ccAddExternalObjectFunction("DynamicSprite::Rotate^3",                  Sc_DynamicSprite_Rotate);
    ccAddExternalObjectFunction("DynamicSprite::SaveToFile^1",              Sc_DynamicSprite_SaveToFile);
    ccAddExternalObjectFunction("DynamicSprite::Tint^5",                    Sc_DynamicSprite_Tint);
    ccAddExternalObjectFunction("DynamicSprite::get_ColorDepth",            Sc_DynamicSprite_GetColorDepth);
    ccAddExternalObjectFunction("DynamicSprite::get_Graphic",               Sc_DynamicSprite_GetGraphic);
    ccAddExternalObjectFunction("DynamicSprite::get_Height",                Sc_DynamicSprite_GetHeight);
    ccAddExternalObjectFunction("DynamicSprite::get_Width",                 Sc_DynamicSprite_GetWidth);
    ccAddExternalStaticFunction("DynamicSprite::Create^3",                  Sc_DynamicSprite_Create);
    ccAddExternalStaticFunction("DynamicSprite::CreateFromBackground",      Sc_DynamicSprite_CreateFromBackground);
    ccAddExternalStaticFunction("DynamicSprite::CreateFromDrawingSurface^5", Sc_DynamicSprite_CreateFromDrawingSurface);
    ccAddExternalStaticFunction("DynamicSprite::CreateFromExistingSprite^1", Sc_DynamicSprite_CreateFromExistingSprite_Old);
    ccAddExternalStaticFunction("DynamicSprite::CreateFromExistingSprite^2", Sc_DynamicSprite_CreateFromExistingSprite);
    ccAddExternalStaticFunction("DynamicSprite::CreateFromFile",            Sc_DynamicSprite_CreateFromFile);
    ccAddExternalStaticFunction("DynamicSprite::CreateFromSaveGame",        Sc_DynamicSprite_CreateFromSaveGame);
    ccAddExternalStaticFunction("DynamicSprite::CreateFromScreenShot",      Sc_DynamicSprite_CreateFromScreenShot);

    /* ----------------------- Registering unsafe exports for plugins -----------------------*/

    ccAddExternalFunctionForPlugin("DynamicSprite::ChangeCanvasSize^4",        (void*)DynamicSprite_ChangeCanvasSize);
    ccAddExternalFunctionForPlugin("DynamicSprite::CopyTransparencyMask^1",    (void*)DynamicSprite_CopyTransparencyMask);
    ccAddExternalFunctionForPlugin("DynamicSprite::Crop^4",                    (void*)DynamicSprite_Crop);
    ccAddExternalFunctionForPlugin("DynamicSprite::Delete",                    (void*)DynamicSprite_Delete);
    ccAddExternalFunctionForPlugin("DynamicSprite::Flip^1",                    (void*)DynamicSprite_Flip);
    ccAddExternalFunctionForPlugin("DynamicSprite::GetDrawingSurface^0",       (void*)DynamicSprite_GetDrawingSurface);
    ccAddExternalFunctionForPlugin("DynamicSprite::Resize^2",                  (void*)DynamicSprite_Resize);
    ccAddExternalFunctionForPlugin("DynamicSprite::Rotate^3",                  (void*)DynamicSprite_Rotate);
    ccAddExternalFunctionForPlugin("DynamicSprite::SaveToFile^1",              (void*)DynamicSprite_SaveToFile);
    ccAddExternalFunctionForPlugin("DynamicSprite::Tint^5",                    (void*)DynamicSprite_Tint);
    ccAddExternalFunctionForPlugin("DynamicSprite::get_ColorDepth",            (void*)DynamicSprite_GetColorDepth);
    ccAddExternalFunctionForPlugin("DynamicSprite::get_Graphic",               (void*)DynamicSprite_GetGraphic);
    ccAddExternalFunctionForPlugin("DynamicSprite::get_Height",                (void*)DynamicSprite_GetHeight);
    ccAddExternalFunctionForPlugin("DynamicSprite::get_Width",                 (void*)DynamicSprite_GetWidth);
    ccAddExternalFunctionForPlugin("DynamicSprite::Create^3",                  (void*)DynamicSprite_Create);
    ccAddExternalFunctionForPlugin("DynamicSprite::CreateFromBackground",      (void*)DynamicSprite_CreateFromBackground);
    ccAddExternalFunctionForPlugin("DynamicSprite::CreateFromDrawingSurface^5", (void*)DynamicSprite_CreateFromDrawingSurface);
    ccAddExternalFunctionForPlugin("DynamicSprite::CreateFromExistingSprite^1", (void*)DynamicSprite_CreateFromExistingSprite_Old);
    ccAddExternalFunctionForPlugin("DynamicSprite::CreateFromExistingSprite^2", (void*)DynamicSprite_CreateFromExistingSprite);
    ccAddExternalFunctionForPlugin("DynamicSprite::CreateFromFile",            (void*)DynamicSprite_CreateFromFile);
    ccAddExternalFunctionForPlugin("DynamicSprite::CreateFromSaveGame",        (void*)DynamicSprite_CreateFromSaveGame);
    ccAddExternalFunctionForPlugin("DynamicSprite::CreateFromScreenShot",      (void*)DynamicSprite_CreateFromScreenShot);
}
