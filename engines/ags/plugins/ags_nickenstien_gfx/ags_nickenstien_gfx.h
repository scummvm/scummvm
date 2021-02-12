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

#ifndef AGS_PLUGINS_AGS_NICKENSTIEN_GFX_AGS_NICKENSTIEN_GFX_H
#define AGS_PLUGINS_AGS_NICKENSTIEN_GFX_AGS_NICKENSTIEN_GFX_H

#include "common/str.h"
#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace AGSNickenstienGFX {

class AGSNickenstienGFX : public PluginBase {
private:
	static IAGSEngine *_engine;
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);

	static void NGFX_GRAPHICS_Initialise(ScriptMethodParams &params);
	static void NGFX_GRAPHICS_Enable(ScriptMethodParams &params);
	static void NGFX_GRAPHICS_SetTimeScalar(ScriptMethodParams &params);
	static void NGFX_GRAPHICS_FullScreenFadeOut(ScriptMethodParams &params);
	static void NGFX_GRAPHICS_FullScreenFadeIn(ScriptMethodParams &params);
	static void NGFX_GRAPHICS_FullScreenFadeOut_2(ScriptMethodParams &params);
	static void NGFX_GRAPHICS_FullScreenFadeOut_2_SetBackgroundColour(ScriptMethodParams &params);
	static void NGFX_GRAPHICS_FullScreenFadeIn_2(ScriptMethodParams &params);
	static void NGFX_GRAPHICS_SetAnisotropicFilter(ScriptMethodParams &params);
	static void NGFX_TEXTURE_Load(ScriptMethodParams &params);
	static void NGFX_TEXTURE_Release(ScriptMethodParams &params);
	static void NGFX_TEXTURE_GetWidth(ScriptMethodParams &params);
	static void NGFX_TEXTURE_GetHeight(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_Create(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_Release(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetType(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetLife(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetEmittionRate(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetParticlesPerEmittion(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetPosition1(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetPosition2(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetStartVelocity(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetStartVelocity_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetEndVelocity(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetEndVelocity_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetStartWidth(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetStartWidth_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetEndWidth(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetEndWidth_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetStartHeight(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetStartHeight_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetEndHeight(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetEndHeight_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetStartAngle(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetStartAngle_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetRotation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetRotation_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetStartColour(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetStartColour_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetEndColour(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetEndColour_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetBlendMode(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetTexture(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetForce(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetParticleLife(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetParticleLife_Deviation(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetNoTimeOut(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetDrawOrderReversed(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetProcessWhenOffScreen(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetUseVelocityParticles(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_SetChannelID(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_Start(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_Clone(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_ReleaseAll(ScriptMethodParams &params);
	static void NGFX_PARTICLE_EMITTER_Evolve(ScriptMethodParams &params);
	static void NGFX_HasGameRoomChanged(ScriptMethodParams &params);
	static void NGFX_HasGameRoomChanged_SecondTest(ScriptMethodParams &params);
	static void NGFX_SPRITE_ResetForNewRoom(ScriptMethodParams &params);
	static void NGFX_SPRITE_Create(ScriptMethodParams &params);
	static void NGFX_SPRITE_Release(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetChannelID(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetPosition(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetPivot(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetTexture(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetBlendMode(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetWidth(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetHeight(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetAngle(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetColour_1(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetColour_2(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetColour_3(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetColour_4(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetClipRectangle(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetGourard(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetFlipped_H(ScriptMethodParams &params);
	static void NGFX_SPRITE_SetFlipped_V(ScriptMethodParams &params);
	static void NGFX_SPRITE_AddToDrawList(ScriptMethodParams &params);
	static void NGFX_InitForNewGameLoop(ScriptMethodParams &params);

public:
	AGSNickenstienGFX();
};

} // namespace AGSNickenstienGFX
} // namespace Plugins
} // namespace AGS3

#endif
