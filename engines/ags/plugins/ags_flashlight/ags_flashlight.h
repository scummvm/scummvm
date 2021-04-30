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
#include "ags/plugins/serializer.h"
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
	static uint32 flashlight_x, flashlight_n;

private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);
	static void AGS_EngineShutdown();
	static int64 AGS_EngineOnEvent(int event, NumberPtr data);

private:
	/**
	 * This function is from Allegro, split for more performance.
	 *  Combines a 32 bit RGBA sprite with a 16 bit RGB destination, optimised
	 *  for when one pixel is in an RGB layout and the other is BGR.
	 */
	static inline uint32 _blender_alpha16_bgr(uint32 y);
	static inline void calc_x_n(uint32 x);
	static inline void setPixel(int x, int y, uint32 color, uint32 *pixel);
	static void plotCircle(int xm, int ym, int r, uint32 color);
	static void ClipToRange(int &variable, int min, int max);
	static void AlphaBlendBitmap();
	static void DrawTint();
	static void DrawDarkness();
	static void CreateLightBitmap();
	static void Update();
	static uint32 blendPixel(uint32 col, bool isAlpha24, int light);
	static void syncGame(Serializer &s);

	static void SetFlashlightTint(ScriptMethodParams &params);
	static void GetFlashlightTintRed(ScriptMethodParams &params);
	static void GetFlashlightTintGreen(ScriptMethodParams &params);
	static void GetFlashlightTintBlue(ScriptMethodParams &params);
	static void GetFlashlightMinLightLevel(ScriptMethodParams &params);
	static void GetFlashlightMaxLightLevel(ScriptMethodParams &params);
	static void SetFlashlightDarkness(ScriptMethodParams &params);
	static void GetFlashlightDarkness(ScriptMethodParams &params);
	static void SetFlashlightDarknessSize(ScriptMethodParams &params);
	static void GetFlashlightDarknessSize(ScriptMethodParams &params);
	static void SetFlashlightBrightness(ScriptMethodParams &params);
	static void GetFlashlightBrightness(ScriptMethodParams &params);
	static void SetFlashlightBrightnessSize(ScriptMethodParams &params);
	static void GetFlashlightBrightnessSize(ScriptMethodParams &params);
	static void SetFlashlightPosition(ScriptMethodParams &params);
	static void GetFlashlightPositionX(ScriptMethodParams &params);
	static void GetFlashlightPositionY(ScriptMethodParams &params);
	static void SetFlashlightFollowMouse(ScriptMethodParams &params);
	static void GetFlashlightFollowMouse(ScriptMethodParams &params);
	static void SetFlashlightFollowCharacter(ScriptMethodParams &params);
	static void GetFlashlightFollowCharacter(ScriptMethodParams &params);
	static void GetFlashlightCharacterDX(ScriptMethodParams &params);
	static void GetFlashlightCharacterDY(ScriptMethodParams &params);
	static void GetFlashlightCharacterHorz(ScriptMethodParams &params);
	static void GetFlashlightCharacterVert(ScriptMethodParams &params);
	static void SetFlashlightMask(ScriptMethodParams &params);
	static void GetFlashlightMask(ScriptMethodParams &params);
public:
	AGSFlashlight();
};

} // namespace AGSFlashlight
} // namespace Plugins
} // namespace AGS3

#endif
