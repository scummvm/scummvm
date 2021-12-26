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

#include "ags/plugins/ags_nickenstien_gfx/ags_nickenstien_gfx.h"

namespace AGS3 {
namespace Plugins {
namespace AGSNickenstienGFX {

const char *AGSNickenstienGFX::AGS_GetPluginName() {
	return "Nickenstien\'s NGFX Graphics Library.";
}

void AGSNickenstienGFX::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	if (_engine->version < 3)
		_engine->AbortGame("Engine interface is too old, need newer version of AGS.");

	SCRIPT_METHOD(NGFX_GRAPHICS_Initialise, AGSNickenstienGFX::NGFX_GRAPHICS_Initialise);
	SCRIPT_METHOD(NGFX_GRAPHICS_Enable, AGSNickenstienGFX::NGFX_GRAPHICS_Enable);
	SCRIPT_METHOD(NGFX_GRAPHICS_SetTimeScalar, AGSNickenstienGFX::NGFX_GRAPHICS_SetTimeScalar);
	SCRIPT_METHOD(NGFX_GRAPHICS_FullScreenFadeOut, AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeOut);
	SCRIPT_METHOD(NGFX_GRAPHICS_FullScreenFadeIn, AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeIn);
	SCRIPT_METHOD(NGFX_GRAPHICS_FullScreenFadeOut_2, AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeOut_2);
	SCRIPT_METHOD(NGFX_GRAPHICS_FullScreenFadeOut_2_SetBackgroundColour, AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeOut_2_SetBackgroundColour);
	SCRIPT_METHOD(NGFX_GRAPHICS_FullScreenFadeIn_2, AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeIn_2);
	SCRIPT_METHOD(NGFX_GRAPHICS_SetAnisotropicFilter, AGSNickenstienGFX::NGFX_GRAPHICS_SetAnisotropicFilter);
	SCRIPT_METHOD(NGFX_TEXTURE_Load, AGSNickenstienGFX::NGFX_TEXTURE_Load);
	SCRIPT_METHOD(NGFX_TEXTURE_Release, AGSNickenstienGFX::NGFX_TEXTURE_Release);
	SCRIPT_METHOD(NGFX_TEXTURE_GetWidth, AGSNickenstienGFX::NGFX_TEXTURE_GetWidth);
	SCRIPT_METHOD(NGFX_TEXTURE_GetHeight, AGSNickenstienGFX::NGFX_TEXTURE_GetHeight);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_Create, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Create);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_Release, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Release);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetType, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetType);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetLife, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetLife);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEmittionRate, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEmittionRate);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetParticlesPerEmittion, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetParticlesPerEmittion);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetPosition1, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetPosition1);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetPosition2, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetPosition2);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartVelocity, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartVelocity);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartVelocity_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartVelocity_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndVelocity, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndVelocity);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndVelocity_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndVelocity_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartWidth, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartWidth);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartWidth_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartWidth_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndWidth, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndWidth);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndWidth_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndWidth_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartHeight, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartHeight);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartHeight_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartHeight_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndHeight, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndHeight);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndHeight_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndHeight_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartAngle, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartAngle);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartAngle_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartAngle_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetRotation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetRotation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetRotation_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetRotation_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartColour, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartColour);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartColour_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartColour_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndColour, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndColour);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndColour_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndColour_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetBlendMode, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetBlendMode);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetTexture, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetTexture);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetForce, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetForce);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetParticleLife, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetParticleLife);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetParticleLife_Deviation, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetParticleLife_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetNoTimeOut, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetNoTimeOut);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetDrawOrderReversed, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetDrawOrderReversed);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetProcessWhenOffScreen, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetProcessWhenOffScreen);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetUseVelocityParticles, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetUseVelocityParticles);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetChannelID, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetChannelID);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_Start, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Start);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_Clone, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Clone);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_ReleaseAll, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_ReleaseAll);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_Evolve, AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Evolve);
	SCRIPT_METHOD(NGFX_HasGameRoomChanged, AGSNickenstienGFX::NGFX_HasGameRoomChanged);
	SCRIPT_METHOD(NGFX_HasGameRoomChanged_SecondTest, AGSNickenstienGFX::NGFX_HasGameRoomChanged_SecondTest);
	SCRIPT_METHOD(NGFX_SPRITE_ResetForNewRoom, AGSNickenstienGFX::NGFX_SPRITE_ResetForNewRoom);
	SCRIPT_METHOD(NGFX_SPRITE_Create, AGSNickenstienGFX::NGFX_SPRITE_Create);
	SCRIPT_METHOD(NGFX_SPRITE_Release, AGSNickenstienGFX::NGFX_SPRITE_Release);
	SCRIPT_METHOD(NGFX_SPRITE_SetChannelID, AGSNickenstienGFX::NGFX_SPRITE_SetChannelID);
	SCRIPT_METHOD(NGFX_SPRITE_SetPosition, AGSNickenstienGFX::NGFX_SPRITE_SetPosition);
	SCRIPT_METHOD(NGFX_SPRITE_SetPivot, AGSNickenstienGFX::NGFX_SPRITE_SetPivot);
	SCRIPT_METHOD(NGFX_SPRITE_SetTexture, AGSNickenstienGFX::NGFX_SPRITE_SetTexture);
	SCRIPT_METHOD(NGFX_SPRITE_SetBlendMode, AGSNickenstienGFX::NGFX_SPRITE_SetBlendMode);
	SCRIPT_METHOD(NGFX_SPRITE_SetWidth, AGSNickenstienGFX::NGFX_SPRITE_SetWidth);
	SCRIPT_METHOD(NGFX_SPRITE_SetHeight, AGSNickenstienGFX::NGFX_SPRITE_SetHeight);
	SCRIPT_METHOD(NGFX_SPRITE_SetAngle, AGSNickenstienGFX::NGFX_SPRITE_SetAngle);
	SCRIPT_METHOD(NGFX_SPRITE_SetColour_1, AGSNickenstienGFX::NGFX_SPRITE_SetColour_1);
	SCRIPT_METHOD(NGFX_SPRITE_SetColour_2, AGSNickenstienGFX::NGFX_SPRITE_SetColour_2);
	SCRIPT_METHOD(NGFX_SPRITE_SetColour_3, AGSNickenstienGFX::NGFX_SPRITE_SetColour_3);
	SCRIPT_METHOD(NGFX_SPRITE_SetColour_4, AGSNickenstienGFX::NGFX_SPRITE_SetColour_4);
	SCRIPT_METHOD(NGFX_SPRITE_SetClipRectangle, AGSNickenstienGFX::NGFX_SPRITE_SetClipRectangle);
	SCRIPT_METHOD(NGFX_SPRITE_SetGourard, AGSNickenstienGFX::NGFX_SPRITE_SetGourard);
	SCRIPT_METHOD(NGFX_SPRITE_SetFlipped_H, AGSNickenstienGFX::NGFX_SPRITE_SetFlipped_H);
	SCRIPT_METHOD(NGFX_SPRITE_SetFlipped_V, AGSNickenstienGFX::NGFX_SPRITE_SetFlipped_V);
	SCRIPT_METHOD(NGFX_SPRITE_AddToDrawList, AGSNickenstienGFX::NGFX_SPRITE_AddToDrawList);
	SCRIPT_METHOD(NGFX_InitForNewGameLoop, AGSNickenstienGFX::NGFX_InitForNewGameLoop);
	// TODO unnamed functions?
}

void AGSNickenstienGFX::NGFX_GRAPHICS_Initialise(ScriptMethodParams &params) {
	//PARAMS2(int, GameNativeScreenWidth, int, GameNativeScreenHeight);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_Enable(ScriptMethodParams &params) {
	//PARAMS1(bool, OnOff);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_SetTimeScalar(ScriptMethodParams &params) {
	//PARAMS1(float, TimeScalar);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeOut(ScriptMethodParams &params) {
	//PARAMS1(int, Time);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeIn(ScriptMethodParams &params) {
	//PARAMS1(int, Time);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeOut_2(ScriptMethodParams &params) {
	//PARAMS9(int, Time, int, BlendMode, int, R, int, G, int, B, int, NumLayers, float, BackSpeed, float, FrontSpeed, Common::String, TextureFilename);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeOut_2_SetBackgroundColour(ScriptMethodParams &params) {
	//PARAMS3(int, Back_R, int, Back_G, int, Back_B);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeIn_2(ScriptMethodParams &params) {
	//PARAMS1(int, Time);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_SetAnisotropicFilter(ScriptMethodParams &params) {
	//PARAMS1(int, TrueFalse);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_TEXTURE_Load(ScriptMethodParams &params) {
	//PARAMS1(Common::String, Filename);
	// TODO rest of the owl
	params._result = 0; // HACK stub
}

void AGSNickenstienGFX::NGFX_TEXTURE_Release(ScriptMethodParams &params) {
	//PARAMS1(int, TextureHandle);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_TEXTURE_GetWidth(ScriptMethodParams &params) {
	//PARAMS1(int, TextureHandle);
	// TODO rest of the owl
	params._result = 0; // HACK stub
}

void AGSNickenstienGFX::NGFX_TEXTURE_GetHeight(ScriptMethodParams &params) {
	//PARAMS1(int, TextureHandle);
	// TODO rest of the owl
	params._result = 0; // HACK stub
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Create(ScriptMethodParams &params) {
	//PARAMS1(int, MaxParticlesForThisEmitter);
	// TODO rest of the owl
	params._result = 0; // HACK stub
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Release(ScriptMethodParams &params) {
	//PARAMS1(int, ParticleEmitterHandle);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetType(ScriptMethodParams &params) {
	//PARAMS1(int, ParticleEmitterHandle, int, EmitterType);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetLife(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, EmitterLife);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEmittionRate(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, EmittionRate);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetParticlesPerEmittion(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, int, ParticlesPerEmittion);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetPosition1(ScriptMethodParams &params) {
	//PARAMS3(int, ParticleEmitterHandle, float, PosX, float, PosY);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetPosition2(ScriptMethodParams &params) {
	//PARAMS3(int, ParticleEmitterHandle, float, PosX, float, PosY);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartVelocity(ScriptMethodParams &params) {
	//PARAMS3(int, ParticleEmitterHandle, float, VelocityX, float, VelocityY);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartVelocity_Deviation(ScriptMethodParams &params) {
	//PARAMS3(int, ParticleEmitterHandle, float, VelocityDeviationX, float, VelocityDeviationY);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndVelocity(ScriptMethodParams &params) {
	//PARAMS3(int, ParticleEmitterHandle, float, VelocityX, float, VelocityY);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndVelocity_Deviation(ScriptMethodParams &params) {
	//PARAMS3(int, ParticleEmitterHandle, float, VelocityDeviationX, float, VelocityDeviationY);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartWidth(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, Width);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartWidth_Deviation(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, WidthDeviation);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndWidth(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, Width);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndWidth_Deviation(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, WidthDeviation);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartHeight(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, Height);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartHeight_Deviation(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, HeightDeviation);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndHeight(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, Height);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndHeight_Deviation(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, HeightDeviation);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartAngle(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, Angle);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartAngle_Deviation(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, AngleDeviation);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetRotation(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, Rotation);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetRotation_Deviation(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, RotationDeviation);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartColour(ScriptMethodParams &params) {
	//PARAMS5(int, ParticleEmitterHandle, int, R, int, G, int, B, int, A);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartColour_Deviation(ScriptMethodParams &params) {
	//PARAMS5(int, ParticleEmitterHandle, int, R, int, G, int, B, int, A);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndColour(ScriptMethodParams &params) {
	//PARAMS5(int, ParticleEmitterHandle, int, R, int, G, int, B, int, A);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndColour_Deviation(ScriptMethodParams &params) {
	//PARAMS5(int, ParticleEmitterHandle, int, R, int, G, int, B, int, A);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetBlendMode(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, int, BlendMode);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetTexture(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, int, TextureHandle);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetForce(ScriptMethodParams &params) {
	//PARAMS3(int, ParticleEmitterHandle, float, ForceX, float, ForceY);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetParticleLife(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, Life);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetParticleLife_Deviation(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, LifeDeviation);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetNoTimeOut(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, int, TrueFalse);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetDrawOrderReversed(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, int, TrueFalse);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetProcessWhenOffScreen(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, int, TrueFalse);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetUseVelocityParticles(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, int, TrueFalse);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetChannelID(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, int, ChannelID);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Start(ScriptMethodParams &params) {
	//PARAMS1(int, ParticleEmitterHandle);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Clone(ScriptMethodParams &params) {
	//PARAMS1(int, SourceParticleEmitterHandle);
	// TODO rest of the owl
	params._result = 0; // HACK stub
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_ReleaseAll(ScriptMethodParams &params) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Evolve(ScriptMethodParams &params) {
	//PARAMS2(int, ParticleEmitterHandle, float, TimeToEvolveBy);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_HasGameRoomChanged(ScriptMethodParams &params) {
	//PARAMS1(int, CurrentGameRoom);
	// TODO rest of the owl
	params._result = true; // HACK stub
}

void AGSNickenstienGFX::NGFX_HasGameRoomChanged_SecondTest(ScriptMethodParams &params) {
	//PARAMS1(int, CurrentGameRoom);
	// TODO rest of the owl
	params._result = true; // HACK stub
}

void AGSNickenstienGFX::NGFX_SPRITE_ResetForNewRoom(ScriptMethodParams &params) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_Create(ScriptMethodParams &params) {
	// TODO rest of the owl
	params._result = 0; // HACK stub
}

void AGSNickenstienGFX::NGFX_SPRITE_Release(ScriptMethodParams &params) {
	//PARAMS1(int, SpriteHandle);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetChannelID(ScriptMethodParams &params) {
	//PARAMS2(int, SpriteHandle, int, ChannelID);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetPosition(ScriptMethodParams &params) {
	//PARAMS3(int, SpriteHandle, float, x, float, y);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetPivot(ScriptMethodParams &params) {
	//PARAMS3(int, SpriteHandle, float, x, float, y);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetTexture(ScriptMethodParams &params) {
	//PARAMS2(int, SpriteHandle, int, TextureHandle);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetBlendMode(ScriptMethodParams &params) {
	//PARAMS2(int, SpriteHandle, int, BlendMode);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetWidth(ScriptMethodParams &params) {
	//PARAMS2(int, SpriteHandle, float, Width);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetHeight(ScriptMethodParams &params) {
	//PARAMS2(int, SpriteHandle, float, Height);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetAngle(ScriptMethodParams &params) {
	//PARAMS2(int, SpriteHandle, float, Angle);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetColour_1(ScriptMethodParams &params) {
	//PARAMS5(int, SpriteHandle, int, R, int, G, int, B, int, A);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetColour_2(ScriptMethodParams &params) {
	//PARAMS5(int, SpriteHandle, int, R, int, G, int, B, int, A);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetColour_3(ScriptMethodParams &params) {
	//PARAMS5(int, SpriteHandle, int, R, int, G, int, B, int, A);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetColour_4(ScriptMethodParams &params) {
	//PARAMS5(int, SpriteHandle, int, R, int, G, int, B, int, A);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetClipRectangle(ScriptMethodParams &params) {
	//PARAMS5(int, SpriteHandle, float, x, float, y, float, x2, float, y2);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetGourard(ScriptMethodParams &params) {
	//PARAMS2(int, SpriteHandle, int, TrueFalse);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetFlipped_H(ScriptMethodParams &params) {
	//PARAMS2(int, SpriteHandle, int, TrueFalse);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetFlipped_V(ScriptMethodParams &params) {
	//PARAMS2(int, SpriteHandle, int, TrueFalse);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_AddToDrawList(ScriptMethodParams &params) {
	//PARAMS1(int, SpriteHandle);
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_InitForNewGameLoop(ScriptMethodParams &params) {
	// TODO rest of the owl
}

} // namespace AGSNickenstienGFX
} // namespace Plugins
} // namespace AGS3
