/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGSFLASHLIGHT_AGSFLASHLIGHT_H
#define AGS_PLUGINS_AGSFLASHLIGHT_AGSFLASHLIGHT_H

#include "ags/plugins/ags_plugin.h"
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
	SCRIPT_HASH(AGSFlashlight)
private:
	int32 screen_width = 320;
	int32 screen_height = 200;
	int32 screen_color_depth = 16;
	bool g_BitmapMustBeUpdated = true;
	int g_RedTint = 0;
	int g_GreenTint = 0;
	int g_BlueTint = 0;
	int g_DarknessLightLevel = 100;
	int g_BrightnessLightLevel = 100;
	int g_DarknessSize = 0;
	int g_DarknessDiameter = 0;
	int g_BrightnessSize = 0;
	int g_ScaleFactor =  1;
	int32 g_FlashlightX = 0;
	int32 g_FlashlightY = 0;
	int32 g_FlashlightDrawAtX = 0;
	int32 g_FlashlightDrawAtY = 0;
	bool g_FlashlightFollowMouse = false;
	int g_FollowCharacterId = 0;
	int g_FollowCharacterDx = 0;
	int g_FollowCharacterDy = 0;
	int g_FollowCharacterHorz = 0;
	int g_FollowCharacterVert = 0;
	AGSCharacter *g_FollowCharacter = nullptr;
	BITMAP *g_LightBitmap = nullptr;
	uint32 flashlight_x = 0, flashlight_n = 0;

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
	AGSFlashlight() : PluginBase() {}
	virtual ~AGSFlashlight() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *engine) override;
	int64 AGS_EngineOnEvent(int event, NumberPtr data) override;
	int AGS_PluginV2() const override { return 1; };
};

} // namespace AGSFlashlight
} // namespace Plugins
} // namespace AGS3

#endif
