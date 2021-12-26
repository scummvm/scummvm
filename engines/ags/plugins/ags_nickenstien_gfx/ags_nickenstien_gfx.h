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

#ifndef AGS_PLUGINS_AGS_NICKENSTIEN_GFX_AGS_NICKENSTIEN_GFX_H
#define AGS_PLUGINS_AGS_NICKENSTIEN_GFX_AGS_NICKENSTIEN_GFX_H

#include "common/str.h"
#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace AGSNickenstienGFX {

class AGSNickenstienGFX : public PluginBase {
	SCRIPT_HASH(AGSNickenstienGFX)
private:
	void NGFX_GRAPHICS_Initialise(ScriptMethodParams &params);
	void NGFX_GRAPHICS_Enable(ScriptMethodParams &params);
	void NGFX_GRAPHICS_SetTimeScalar(ScriptMethodParams &params);
	void NGFX_GRAPHICS_FullScreenFadeOut(ScriptMethodParams &params);
	void NGFX_GRAPHICS_FullScreenFadeIn(ScriptMethodParams &params);
	void NGFX_GRAPHICS_FullScreenFadeOut_2(ScriptMethodParams &params);
	void NGFX_GRAPHICS_FullScreenFadeOut_2_SetBackgroundColour(ScriptMethodParams &params);
	void NGFX_GRAPHICS_FullScreenFadeIn_2(ScriptMethodParams &params);
	void NGFX_GRAPHICS_SetAnisotropicFilter(ScriptMethodParams &params);
	void NGFX_TEXTURE_Load(ScriptMethodParams &params);
	void NGFX_TEXTURE_Release(ScriptMethodParams &params);
	void NGFX_TEXTURE_GetWidth(ScriptMethodParams &params);
	void NGFX_TEXTURE_GetHeight(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_Create(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_Release(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetType(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetLife(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetEmittionRate(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetParticlesPerEmittion(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetPosition1(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetPosition2(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetStartVelocity(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetStartVelocity_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetEndVelocity(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetEndVelocity_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetStartWidth(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetStartWidth_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetEndWidth(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetEndWidth_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetStartHeight(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetStartHeight_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetEndHeight(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetEndHeight_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetStartAngle(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetStartAngle_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetRotation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetRotation_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetStartColour(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetStartColour_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetEndColour(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetEndColour_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetBlendMode(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetTexture(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetForce(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetParticleLife(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetParticleLife_Deviation(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetNoTimeOut(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetDrawOrderReversed(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetProcessWhenOffScreen(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetUseVelocityParticles(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_SetChannelID(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_Start(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_Clone(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_ReleaseAll(ScriptMethodParams &params);
	void NGFX_PARTICLE_EMITTER_Evolve(ScriptMethodParams &params);
	void NGFX_HasGameRoomChanged(ScriptMethodParams &params);
	void NGFX_HasGameRoomChanged_SecondTest(ScriptMethodParams &params);
	void NGFX_SPRITE_ResetForNewRoom(ScriptMethodParams &params);
	void NGFX_SPRITE_Create(ScriptMethodParams &params);
	void NGFX_SPRITE_Release(ScriptMethodParams &params);
	void NGFX_SPRITE_SetChannelID(ScriptMethodParams &params);
	void NGFX_SPRITE_SetPosition(ScriptMethodParams &params);
	void NGFX_SPRITE_SetPivot(ScriptMethodParams &params);
	void NGFX_SPRITE_SetTexture(ScriptMethodParams &params);
	void NGFX_SPRITE_SetBlendMode(ScriptMethodParams &params);
	void NGFX_SPRITE_SetWidth(ScriptMethodParams &params);
	void NGFX_SPRITE_SetHeight(ScriptMethodParams &params);
	void NGFX_SPRITE_SetAngle(ScriptMethodParams &params);
	void NGFX_SPRITE_SetColour_1(ScriptMethodParams &params);
	void NGFX_SPRITE_SetColour_2(ScriptMethodParams &params);
	void NGFX_SPRITE_SetColour_3(ScriptMethodParams &params);
	void NGFX_SPRITE_SetColour_4(ScriptMethodParams &params);
	void NGFX_SPRITE_SetClipRectangle(ScriptMethodParams &params);
	void NGFX_SPRITE_SetGourard(ScriptMethodParams &params);
	void NGFX_SPRITE_SetFlipped_H(ScriptMethodParams &params);
	void NGFX_SPRITE_SetFlipped_V(ScriptMethodParams &params);
	void NGFX_SPRITE_AddToDrawList(ScriptMethodParams &params);
	void NGFX_InitForNewGameLoop(ScriptMethodParams &params);

public:
	AGSNickenstienGFX() : PluginBase() {}
	virtual ~AGSNickenstienGFX() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *engine) override;
};

} // namespace AGSNickenstienGFX
} // namespace Plugins
} // namespace AGS3

#endif
