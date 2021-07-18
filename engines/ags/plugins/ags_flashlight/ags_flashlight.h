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
IAGSEngine *_engine;
int32 screen_width;
int32 screen_height;
int32 screen_color_depth;
bool g_BitmapMustBeUpdated;
int g_RedTint;
int g_GreenTint;
int g_BlueTint;
int g_DarknessLightLevel;
int g_BrightnessLightLevel;
int g_DarknessSize;
int g_DarknessDiameter;
int g_BrightnessSize;
int32 g_FlashlightX;
int32 g_FlashlightY;
int32 g_FlashlightDrawAtX;
int32 g_FlashlightDrawAtY;
bool g_FlashlightFollowMouse;
int g_FollowCharacterId;
int g_FollowCharacterDx;
int g_FollowCharacterDy;
int g_FollowCharacterHorz;
int g_FollowCharacterVert;
AGSCharacter *g_FollowCharacter;
BITMAP *g_LightBitmap;
uint32 flashlight_x, flashlight_n;

private:
const char *AGS_GetPluginName();
void AGS_EngineStartup(IAGSEngine *engine) override;
void AGS_EngineShutdown();
int64 AGS_EngineOnEvent(int event, NumberPtr data);

private:
/**
 * This function is from Allegro, split for more performance.
 *  Combines a 32 bit RGBA sprite with a 16 bit RGB destination, optimised
 *  for when one pixel is in an RGB layout and the other is BGR.
 */
inline uint32 _blender_alpha16_bgr(uint32 y);
inline void calc_x_n(uint32 x);
inline void setPixel(int x, int y, uint32 color, uint32 *pixel);
void plotCircle(int xm, int ym, int r, uint32 color);
void ClipToRange(int &variable, int min, int max);
void AlphaBlendBitmap();
void DrawTint();
void DrawDarkness();
void CreateLightBitmap();
void Update();
uint32 blendPixel(uint32 col, bool isAlpha24, int light);
void syncGame(Serializer &s);

void SetFlashlightTint(ScriptMethodParams &params);
void GetFlashlightTintRed(ScriptMethodParams &params);
void GetFlashlightTintGreen(ScriptMethodParams &params);
void GetFlashlightTintBlue(ScriptMethodParams &params);
void GetFlashlightMinLightLevel(ScriptMethodParams &params);
void GetFlashlightMaxLightLevel(ScriptMethodParams &params);
void SetFlashlightDarkness(ScriptMethodParams &params);
void GetFlashlightDarkness(ScriptMethodParams &params);
void SetFlashlightDarknessSize(ScriptMethodParams &params);
void GetFlashlightDarknessSize(ScriptMethodParams &params);
void SetFlashlightBrightness(ScriptMethodParams &params);
void GetFlashlightBrightness(ScriptMethodParams &params);
void SetFlashlightBrightnessSize(ScriptMethodParams &params);
void GetFlashlightBrightnessSize(ScriptMethodParams &params);
void SetFlashlightPosition(ScriptMethodParams &params);
void GetFlashlightPositionX(ScriptMethodParams &params);
void GetFlashlightPositionY(ScriptMethodParams &params);
void SetFlashlightFollowMouse(ScriptMethodParams &params);
void GetFlashlightFollowMouse(ScriptMethodParams &params);
void SetFlashlightFollowCharacter(ScriptMethodParams &params);
void GetFlashlightFollowCharacter(ScriptMethodParams &params);
void GetFlashlightCharacterDX(ScriptMethodParams &params);
void GetFlashlightCharacterDY(ScriptMethodParams &params);
void GetFlashlightCharacterHorz(ScriptMethodParams &params);
void GetFlashlightCharacterVert(ScriptMethodParams &params);
void SetFlashlightMask(ScriptMethodParams &params);
void GetFlashlightMask(ScriptMethodParams &params);
public:
AGSFlashlight();
};

} // namespace AGSFlashlight
} // namespace Plugins
} // namespace AGS3

#endif
