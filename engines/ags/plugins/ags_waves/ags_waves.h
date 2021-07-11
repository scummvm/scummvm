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

#ifndef AGS_PLUGINS_AGS_WAVES_AGS_WAVES_H
#define AGS_PLUGINS_AGS_WAVES_AGS_WAVES_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {

class AGSWaves : public PluginBase {
private:
	static IAGSEngine *_engine;
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);

private:
	static void DrawScreenEffect(ScriptMethodParams &params);
	static void SFX_Play(ScriptMethodParams &params);
	static void SFX_SetVolume(ScriptMethodParams &params);
	static void SFX_GetVolume(ScriptMethodParams &params);
	static void Music_Play(ScriptMethodParams &params);
	static void Music_GetVolume(ScriptMethodParams &params);
	static void Music_SetVolume(ScriptMethodParams &params);
	static void SFX_Stop(ScriptMethodParams &params);
	static void SFX_SetPosition(ScriptMethodParams &params);
	static void SFX_SetGlobalVolume(ScriptMethodParams &params);
	static void Load_SFX(ScriptMethodParams &params);
	static void Audio_Apply_Filter(ScriptMethodParams &params);
	static void Audio_Remove_Filter(ScriptMethodParams &params);
	static void SFX_AllowOverlap(ScriptMethodParams &params);
	static void SFX_Filter(ScriptMethodParams &params);
	static void DrawBlur(ScriptMethodParams &params);
	static void DrawTunnel(ScriptMethodParams &params);
	static void DrawCylinder(ScriptMethodParams &params);
	static void DrawForceField(ScriptMethodParams &params);
	static void Grayscale(ScriptMethodParams &params);
	static void ReadWalkBehindIntoSprite(ScriptMethodParams &params);
	static void AdjustSpriteFont(ScriptMethodParams &params);
	static void SpriteGradient(ScriptMethodParams &params);
	static void Outline(ScriptMethodParams &params);
	static void OutlineOnly(ScriptMethodParams &params);
	static void SaveVariable(ScriptMethodParams &params);
	static void ReadVariable(ScriptMethodParams &params);
	static void GameDoOnceOnly(ScriptMethodParams &params);
	static void SetGDState(ScriptMethodParams &params);
	static void GetGDState(ScriptMethodParams &params);
	static void ResetAllGD(ScriptMethodParams &params);
	static void SpriteSkew(ScriptMethodParams &params);
	static void FireUpdate(ScriptMethodParams &params);
	static void WindUpdate(ScriptMethodParams &params);
	static void SetWindValues(ScriptMethodParams &params);
	static void ReturnWidth(ScriptMethodParams &params);
	static void ReturnHeight(ScriptMethodParams &params);
	static void ReturnNewHeight(ScriptMethodParams &params);
	static void ReturnNewWidth(ScriptMethodParams &params);
	static void Warper(ScriptMethodParams &params);
	static void SetWarper(ScriptMethodParams &params);
	static void RainUpdate(ScriptMethodParams &params);
	static void BlendTwoSprites(ScriptMethodParams &params);
	static void Blend(ScriptMethodParams &params);
	static void Dissolve(ScriptMethodParams &params);
	static void ReverseTransparency(ScriptMethodParams &params);
	static void NoiseCreator(ScriptMethodParams &params);
	static void TintProper(ScriptMethodParams &params);
	static void GetWalkbehindBaserine(ScriptMethodParams &params);
	static void SetWalkbehindBaserine(ScriptMethodParams &params);
public:
	AGSWaves();
};

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3

#endif
