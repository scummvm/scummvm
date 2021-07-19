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

#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/ags_waves/vars.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {

class AGSWaves : public PluginBase, public Vars {
	SCRIPT_HASH(AGSWaves)
private:
	void DrawScreenEffect(ScriptMethodParams &params);
	void SFX_Play(ScriptMethodParams &params);
	void SFX_SetVolume(ScriptMethodParams &params);
	void SFX_GetVolume(ScriptMethodParams &params);
	void Music_Play(ScriptMethodParams &params);
	void Music_GetVolume(ScriptMethodParams &params);
	void Music_SetVolume(ScriptMethodParams &params);
	void SFX_Stop(ScriptMethodParams &params);
	void SFX_SetPosition(ScriptMethodParams &params);
	void SFX_SetGlobalVolume(ScriptMethodParams &params);
	void Load_SFX(ScriptMethodParams &params);
	void Audio_Apply_Filter(ScriptMethodParams &params);
	void Audio_Remove_Filter(ScriptMethodParams &params);
	void SFX_AllowOverlap(ScriptMethodParams &params);
	void SFX_Filter(ScriptMethodParams &params);
	void DrawBlur(ScriptMethodParams &params);
	void DrawTunnel(ScriptMethodParams &params);
	void DrawCylinder(ScriptMethodParams &params);
	void DrawForceField(ScriptMethodParams &params);
	void Grayscale(ScriptMethodParams &params);
	void ReadWalkBehindIntoSprite(ScriptMethodParams &params);
	void AdjustSpriteFont(ScriptMethodParams &params);
	void SpriteGradient(ScriptMethodParams &params);
	void Outline(ScriptMethodParams &params);
	void OutlineOnly(ScriptMethodParams &params);
	void SaveVariable(ScriptMethodParams &params);
	void ReadVariable(ScriptMethodParams &params);
	void GameDoOnceOnly(ScriptMethodParams &params);
	void SetGDState(ScriptMethodParams &params);
	void GetGDState(ScriptMethodParams &params);
	void ResetAllGD(ScriptMethodParams &params);
	void SpriteSkew(ScriptMethodParams &params);
	void FireUpdate(ScriptMethodParams &params);
	void WindUpdate(ScriptMethodParams &params);
	void SetWindValues(ScriptMethodParams &params);
	void ReturnWidth(ScriptMethodParams &params);
	void ReturnHeight(ScriptMethodParams &params);
	void ReturnNewHeight(ScriptMethodParams &params);
	void ReturnNewWidth(ScriptMethodParams &params);
	void Warper(ScriptMethodParams &params);
	void SetWarper(ScriptMethodParams &params);
	void RainUpdate(ScriptMethodParams &params);
	void BlendTwoSprites(ScriptMethodParams &params);
	void Blend(ScriptMethodParams &params);
	void Dissolve(ScriptMethodParams &params);
	void ReverseTransparency(ScriptMethodParams &params);
	void NoiseCreator(ScriptMethodParams &params);
	void TintProper(ScriptMethodParams &params);
	void GetWalkbehindBaserine(ScriptMethodParams &params);
	void SetWalkbehindBaserine(ScriptMethodParams &params);

private:
	void StartingValues();

	void CastWave(int delayMax, int PixelsWide, int n);
	void DrawEffect(int sprite_a, int sprite_b, int id, int n);

public:
	AGSWaves() : PluginBase(), Vars() {}
	virtual ~AGSWaves() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *engine) override;
};

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3

#endif
