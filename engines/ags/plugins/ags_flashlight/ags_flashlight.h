/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGSFLASHLIGHT_AGSFLASHLIGHT_H
#define AGS_PLUGINS_AGSFLASHLIGHT_AGSFLASHLIGHT_H

#include "ags/plugins/plugin_base.h"
#include "ags/lib/allegro.h"

namespace AGS3 {
namespace Plugins {
namespace AGSFlashlight {

/**
 * This is not the AGS Flashlight plugin,
 * but a workalike plugin originally created for the AGS engine PSP port.
 */
class AGSFlashlight : public PluginBase {
private:
	static IAGSEngine *_engine;
	static int32 screen_width;
	static int32 screen_height;
	static int32 screen_color_depth;
	static bool g_BitmapMustBeUpdated;
	static int g_RedTint;
	static int g_GreenTint;
	static int g_BlueTint;
	static int g_DarknessLightLevel;
	static int g_BrightnessLightLevel;
	static int g_DarknessSize;
	static int g_DarknessDiameter;
	static int g_BrightnessSize;
	static int32 g_FlashlightX;
	static int32 g_FlashlightY;
	static int32 g_FlashlightDrawAtX;
	static int32 g_FlashlightDrawAtY;
	static bool g_FlashlightFollowMouse;
	static int g_FollowCharacterId;
	static int g_FollowCharacterDx;
	static int g_FollowCharacterDy;
	static int g_FollowCharacterHorz;
	static int g_FollowCharacterVert;
	static AGSCharacter *g_FollowCharacter;
	static BITMAP *g_LightBitmap;
	static unsigned long flashlight_x, flashlight_n;

private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);
	static void AGS_EngineShutdown();
	static NumberPtr AGS_EngineOnEvent(int event, NumberPtr data);

private:
	/**
	 * This function is from Allegro, split for more performance.
	 *  Combines a 32 bit RGBA sprite with a 16 bit RGB destination, optimised
	 *  for when one pixel is in an RGB layout and the other is BGR.
	 */
	static inline unsigned long _blender_alpha16_bgr(unsigned long y);
	static inline void calc_x_n(unsigned long x);
	static inline void setPixel(int x, int y, int color, unsigned int *pixel);
	static void plotCircle(int xm, int ym, int r, unsigned int color);
	static void ClipToRange(int &variable, int min, int max);
	static void AlphaBlendBitmap();
	static void DrawTint();
	static void DrawDarkness();
	static void CreateLightBitmap();
	static void Update();
	static size_t engineFileRead(void *ptr, size_t size, size_t count, long fileHandle);
	static size_t engineFileWrite(const void *ptr, size_t size, size_t count, long fileHandle);
	static void RestoreGame(long file);
	static void SaveGame(long file);
	static void SetFlashlightTint(int RedTint, int GreenTint, int BlueTint);
	static int GetFlashlightTintRed();
	static int GetFlashlightTintGreen();
	static int GetFlashlightTintBlue();
	static int GetFlashlightMinLightLevel();
	static int GetFlashlightMaxLightLevel();
	static void SetFlashlightDarkness(int LightLevel);
	static int GetFlashlightDarkness();
	static void SetFlashlightDarknessSize(int Size);
	static int GetFlashlightDarknessSize();
	static void SetFlashlightBrightness(int LightLevel);
	static int GetFlashlightBrightness();
	static void SetFlashlightBrightnessSize(int Size);
	static int GetFlashlightBrightnessSize();
	static void SetFlashlightPosition(int X, int Y);
	static int GetFlashlightPositionX();
	static int GetFlashlightPositionY();
	static void SetFlashlightFollowMouse(int OnOff);
	static int GetFlashlightFollowMouse();
	static void SetFlashlightFollowCharacter(int CharacterId, int dx, int dy, int horz, int vert);
	static int GetFlashlightFollowCharacter();
	static int GetFlashlightCharacterDX();
	static int GetFlashlightCharacterDY();
	static int GetFlashlightCharacterHorz();
	static int GetFlashlightCharacterVert();
	static void SetFlashlightMask(int SpriteSlot);
	static int GetFlashlightMask();
public:
	AGSFlashlight();
};

} // namespace AGSFlashlight
} // namespace Plugins
} // namespace AGS3

#endif
