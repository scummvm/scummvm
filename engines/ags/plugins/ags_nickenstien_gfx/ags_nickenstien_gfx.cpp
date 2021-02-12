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

#include "ags/plugins/ags_nickenstien_gfx/ags_nickenstien_gfx.h"

namespace AGS3 {
namespace Plugins {
namespace AGSNickenstienGFX {

IAGSEngine *AGSNickenstienGFX::_engine;

AGSNickenstienGFX::AGSNickenstienGFX() : DLL() {
	_engine = nullptr;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSNickenstienGFX::AGS_GetPluginName() {
	return "Nickenstien\'s NGFX Graphics Library.";
}

void AGSNickenstienGFX::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

	if (_engine->version < 3)
		_engine->AbortGame("Engine interface is too old, need newer version of AGS.");

	SCRIPT_METHOD(NGFX_GRAPHICS_Initialise);
	SCRIPT_METHOD(NGFX_GRAPHICS_Enable);
	SCRIPT_METHOD(NGFX_GRAPHICS_SetTimeScalar);
	SCRIPT_METHOD(NGFX_GRAPHICS_FullScreenFadeOut);
	SCRIPT_METHOD(NGFX_GRAPHICS_FullScreenFadeIn);
	SCRIPT_METHOD(NGFX_GRAPHICS_FullScreenFadeOut_2);
	SCRIPT_METHOD(NGFX_GRAPHICS_FullScreenFadeOut_2_SetBackgroundColour);
	SCRIPT_METHOD(NGFX_GRAPHICS_FullScreenFadeIn_2);
	SCRIPT_METHOD(NGFX_GRAPHICS_SetAnisotropicFilter);
	SCRIPT_METHOD(NGFX_TEXTURE_Load);
	SCRIPT_METHOD(NGFX_TEXTURE_Release);
	SCRIPT_METHOD(NGFX_TEXTURE_GetWidth);
	SCRIPT_METHOD(NGFX_TEXTURE_GetHeight);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_Create);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_Release);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetType);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetLife);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEmittionRate);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetParticlesPerEmittion);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetPosition1);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetPosition2);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartVelocity);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartVelocity_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndVelocity);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndVelocity_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartWidth);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartWidth_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndWidth);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndWidth_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartHeight);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartHeight_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndHeight);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndHeight_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartAngle);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartAngle_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetRotation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetRotation_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartColour);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetStartColour_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndColour);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetEndColour_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetBlendMode);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetTexture);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetForce);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetParticleLife);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetParticleLife_Deviation);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetNoTimeOut);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetDrawOrderReversed);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetProcessWhenOffScreen);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetUseVelocityParticles);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_SetChannelID);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_Start);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_Clone);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_ReleaseAll);
	SCRIPT_METHOD(NGFX_PARTICLE_EMITTER_Evolve);
	SCRIPT_METHOD(NGFX_HasGameRoomChanged);
	SCRIPT_METHOD(NGFX_HasGameRoomChanged_SecondTest);
	SCRIPT_METHOD(NGFX_SPRITE_ResetForNewRoom);
	SCRIPT_METHOD(NGFX_SPRITE_Create);
	SCRIPT_METHOD(NGFX_SPRITE_Release);
	SCRIPT_METHOD(NGFX_SPRITE_SetChannelID);
	SCRIPT_METHOD(NGFX_SPRITE_SetPosition);
	SCRIPT_METHOD(NGFX_SPRITE_SetPivot);
	SCRIPT_METHOD(NGFX_SPRITE_SetTexture);
	SCRIPT_METHOD(NGFX_SPRITE_SetBlendMode);
	SCRIPT_METHOD(NGFX_SPRITE_SetWidth);
	SCRIPT_METHOD(NGFX_SPRITE_SetHeight);
	SCRIPT_METHOD(NGFX_SPRITE_SetAngle);
	SCRIPT_METHOD(NGFX_SPRITE_SetColour_1);
	SCRIPT_METHOD(NGFX_SPRITE_SetColour_2);
	SCRIPT_METHOD(NGFX_SPRITE_SetColour_3);
	SCRIPT_METHOD(NGFX_SPRITE_SetColour_4);
	SCRIPT_METHOD(NGFX_SPRITE_SetClipRectangle);
	SCRIPT_METHOD(NGFX_SPRITE_SetGourard);
	SCRIPT_METHOD(NGFX_SPRITE_SetFlipped_H);
	SCRIPT_METHOD(NGFX_SPRITE_SetFlipped_V);
	SCRIPT_METHOD(NGFX_SPRITE_AddToDrawList);
	SCRIPT_METHOD(NGFX_InitForNewGameLoop);
	// TODO unnamed functions?
}

void AGSNickenstienGFX::NGFX_GRAPHICS_Initialise(int GameNativeScreenWidth, int GameNativeScreenHeight) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_Enable(bool OnOff) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_SetTimeScalar(float TimeScalar) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeOut(int Time) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeIn(int Time) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeOut_2(int Time, int BlendMode, int R, int G, int B, int NumLayers, float BackSpeed, float FrontSpeed, Common::String TextureFilename) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeOut_2_SetBackgroundColour(int Back_R, int Back_G, int Back_B) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_FullScreenFadeIn_2(int Time) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_GRAPHICS_SetAnisotropicFilter(int TrueFalse) {
	// TODO rest of the owl
}

int AGSNickenstienGFX::NGFX_TEXTURE_Load(Common::String Filename) {
	// TODO rest of the owl
	return 0; // HACK stub
}

void AGSNickenstienGFX::NGFX_TEXTURE_Release(int TextureHandle) {
	// TODO rest of the owl
}

int AGSNickenstienGFX::NGFX_TEXTURE_GetWidth(int TextureHandle) {
	// TODO rest of the owl
	return 0; // HACK stub
}

int AGSNickenstienGFX::NGFX_TEXTURE_GetHeight(int TextureHandle) {
	// TODO rest of the owl
	return 0; // HACK stub
}

int AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Create(int MaxParticlesForThisEmitter) {
	// TODO rest of the owl
	return 0; // HACK stub
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Release(int ParticleEmitterHandle) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetType(int ParticleEmitterHandle, int EmitterType) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetLife(int ParticleEmitterHandle, float EmitterLife) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEmittionRate(int ParticleEmitterHandle, float EmittionRate) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetParticlesPerEmittion(int ParticleEmitterHandle, int ParticlesPerEmittion) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetPosition1(int ParticleEmitterHandle, float PosX, float PosY) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetPosition2(int ParticleEmitterHandle, float PosX, float PosY) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartVelocity(int ParticleEmitterHandle, float VelocityX, float VelocityY) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartVelocity_Deviation(int ParticleEmitterHandle, float VelocityDeviationX, float VelocityDeviationY) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndVelocity(int ParticleEmitterHandle, float VelocityX, float VelocityY) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndVelocity_Deviation(int ParticleEmitterHandle, float VelocityDeviationX, float VelocityDeviationY) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartWidth(int ParticleEmitterHandle, float Width) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartWidth_Deviation(int ParticleEmitterHandle, float WidthDeviation) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndWidth(int ParticleEmitterHandle, float Width) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndWidth_Deviation(int ParticleEmitterHandle, float WidthDeviation) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartHeight(int ParticleEmitterHandle, float Height) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartHeight_Deviation(int ParticleEmitterHandle, float HeightDeviation) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndHeight(int ParticleEmitterHandle, float Height) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndHeight_Deviation(int ParticleEmitterHandle, float HeightDeviation) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartAngle(int ParticleEmitterHandle, float Angle) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartAngle_Deviation(int ParticleEmitterHandle, float AngleDeviation) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetRotation(int ParticleEmitterHandle, float Rotation) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetRotation_Deviation(int ParticleEmitterHandle, float RotationDeviation) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartColour(int ParticleEmitterHandle, int R, int G, int B, int A) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetStartColour_Deviation(int ParticleEmitterHandle, int R, int G, int B, int A) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndColour(int ParticleEmitterHandle, int R, int G, int B, int A) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetEndColour_Deviation(int ParticleEmitterHandle, int R, int G, int B, int A) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetBlendMode(int ParticleEmitterHandle, int BlendMode) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetTexture(int ParticleEmitterHandle, int TextureHandle) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetForce(int ParticleEmitterHandle, float ForceX, float ForceY) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetParticleLife(int ParticleEmitterHandle, float Life) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetParticleLife_Deviation(int ParticleEmitterHandle, float LifeDeviation) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetNoTimeOut(int ParticleEmitterHandle, int TrueFalse) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetDrawOrderReversed(int ParticleEmitterHandle, int TrueFalse) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetProcessWhenOffScreen(int ParticleEmitterHandle, int TrueFalse) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetUseVelocityParticles(int ParticleEmitterHandle, int TrueFalse) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_SetChannelID(int ParticleEmitterHandle, int ChannelID) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Start(int ParticleEmitterHandle) {
	// TODO rest of the owl
}

int AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Clone(int SourceParticleEmitterHandle) {
	// TODO rest of the owl
	return 0; // HACK stub
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_ReleaseAll() {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_PARTICLE_EMITTER_Evolve(int ParticleEmitterHandle, float TimeToEvolveBy) {
	// TODO rest of the owl
}

bool AGSNickenstienGFX::NGFX_HasGameRoomChanged(int CurrentGameRoom) {
	// TODO rest of the owl
	return true; // HACK stub
}

bool AGSNickenstienGFX::NGFX_HasGameRoomChanged_SecondTest(int CurrentGameRoom) {
	// TODO rest of the owl
	return true; // HACK stub
}

void AGSNickenstienGFX::NGFX_SPRITE_ResetForNewRoom() {
	// TODO rest of the owl
}

int AGSNickenstienGFX::NGFX_SPRITE_Create() {
	// TODO rest of the owl
	return 0; // HACK stub
}

void AGSNickenstienGFX::NGFX_SPRITE_Release(int SpriteHandle) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetChannelID(int SpriteHandle, int ChannelID) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetPosition(int SpriteHandle, float x, float y) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetPivot(int SpriteHandle, float x, float y) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetTexture(int SpriteHandle, int TextureHandle) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetBlendMode(int SpriteHandle, int BlendMode) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetWidth(int SpriteHandle, float Width) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetHeight(int SpriteHandle, float Height) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetAngle(int SpriteHandle, float Angle) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetColour_1(int SpriteHandle, int R, int G, int B, int A) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetColour_2(int SpriteHandle, int R, int G, int B, int A) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetColour_3(int SpriteHandle, int R, int G, int B, int A) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetColour_4(int SpriteHandle, int R, int G, int B, int A) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetClipRectangle(int SpriteHandle, float x, float y, float x2, float y2) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetGourard(int SpriteHandle, int TrueFalse) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetFlipped_H(int SpriteHandle, int TrueFalse) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_SetFlipped_V(int SpriteHandle, int TrueFalse) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_SPRITE_AddToDrawList(int SpriteHandle) {
	// TODO rest of the owl
}

void AGSNickenstienGFX::NGFX_InitForNewGameLoop() {
	// TODO rest of the owl
}

} // namespace AGSNickenstienGFX
} // namespace Plugins
} // namespace AGS3
