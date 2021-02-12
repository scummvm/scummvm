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
#include "ags/plugins/dll.h"

namespace AGS3 {
namespace Plugins {
namespace AGSNickenstienGFX {

class AGSNickenstienGFX : public DLL {
private:
	static IAGSEngine *_engine;
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);

	static void NGFX_GRAPHICS_Initialise(int GameNativeScreenWidth, int GameNativeScreenHeight);
	static void NGFX_GRAPHICS_Enable(bool OnOff);
	static void NGFX_GRAPHICS_SetTimeScalar(float TimeScalar);
	static void NGFX_GRAPHICS_FullScreenFadeOut(int Time);
	static void NGFX_GRAPHICS_FullScreenFadeIn(int Time);
	static void NGFX_GRAPHICS_FullScreenFadeOut_2(int Time, int BlendMode, int R, int G, int B, int NumLayers, float BackSpeed, float FrontSpeed, Common::String TextureFilename);
	static void NGFX_GRAPHICS_FullScreenFadeOut_2_SetBackgroundColour(int Back_R, int Back_G, int Back_B);
	static void NGFX_GRAPHICS_FullScreenFadeIn_2(int Time);
	static void NGFX_GRAPHICS_SetAnisotropicFilter(int TrueFalse);
	static int NGFX_TEXTURE_Load(Common::String Filename);
	static void NGFX_TEXTURE_Release(int TextureHandle);
	static int NGFX_TEXTURE_GetWidth(int TextureHandle);
	static int NGFX_TEXTURE_GetHeight(int TextureHandle);
	static int NGFX_PARTICLE_EMITTER_Create(int MaxParticlesForThisEmitter);
	static void NGFX_PARTICLE_EMITTER_Release(int ParticleEmitterHandle);
	static void NGFX_PARTICLE_EMITTER_SetType(int ParticleEmitterHandle, int EmitterType);
	static void NGFX_PARTICLE_EMITTER_SetLife(int ParticleEmitterHandle, float EmitterLife);
	static void NGFX_PARTICLE_EMITTER_SetEmittionRate(int ParticleEmitterHandle, float EmittionRate);
	static void NGFX_PARTICLE_EMITTER_SetParticlesPerEmittion(int ParticleEmitterHandle, int ParticlesPerEmittion);
	static void NGFX_PARTICLE_EMITTER_SetPosition1(int ParticleEmitterHandle, float PosX, float PosY);
	static void NGFX_PARTICLE_EMITTER_SetPosition2(int ParticleEmitterHandle, float PosX, float PosY);
	static void NGFX_PARTICLE_EMITTER_SetStartVelocity(int ParticleEmitterHandle, float VelocityX, float VelocityY);
	static void NGFX_PARTICLE_EMITTER_SetStartVelocity_Deviation(int ParticleEmitterHandle, float VelocityDeviationX, float VelocityDeviationY);
	static void NGFX_PARTICLE_EMITTER_SetEndVelocity(int ParticleEmitterHandle, float VelocityX, float VelocityY);
	static void NGFX_PARTICLE_EMITTER_SetEndVelocity_Deviation(int ParticleEmitterHandle, float VelocityDeviationX, float VelocityDeviationY);
	static void NGFX_PARTICLE_EMITTER_SetStartWidth(int ParticleEmitterHandle, float Width);
	static void NGFX_PARTICLE_EMITTER_SetStartWidth_Deviation(int ParticleEmitterHandle, float WidthDeviation);
	static void NGFX_PARTICLE_EMITTER_SetEndWidth(int ParticleEmitterHandle, float Width);
	static void NGFX_PARTICLE_EMITTER_SetEndWidth_Deviation(int ParticleEmitterHandle, float WidthDeviation);
	static void NGFX_PARTICLE_EMITTER_SetStartHeight(int ParticleEmitterHandle, float Height);
	static void NGFX_PARTICLE_EMITTER_SetStartHeight_Deviation(int ParticleEmitterHandle, float HeightDeviation);
	static void NGFX_PARTICLE_EMITTER_SetEndHeight(int ParticleEmitterHandle, float Height);
	static void NGFX_PARTICLE_EMITTER_SetEndHeight_Deviation(int ParticleEmitterHandle, float HeightDeviation);
	static void NGFX_PARTICLE_EMITTER_SetStartAngle(int ParticleEmitterHandle, float Angle);
	static void NGFX_PARTICLE_EMITTER_SetStartAngle_Deviation(int ParticleEmitterHandle, float AngleDeviation);
	static void NGFX_PARTICLE_EMITTER_SetRotation(int ParticleEmitterHandle, float Rotation);
	static void NGFX_PARTICLE_EMITTER_SetRotation_Deviation(int ParticleEmitterHandle, float RotationDeviation);
	static void NGFX_PARTICLE_EMITTER_SetStartColour(int ParticleEmitterHandle, int R, int G, int B, int A);
	static void NGFX_PARTICLE_EMITTER_SetStartColour_Deviation(int ParticleEmitterHandle, int R, int G, int B, int A);
	static void NGFX_PARTICLE_EMITTER_SetEndColour(int ParticleEmitterHandle, int R, int G, int B, int A);
	static void NGFX_PARTICLE_EMITTER_SetEndColour_Deviation(int ParticleEmitterHandle, int R, int G, int B, int A);
	static void NGFX_PARTICLE_EMITTER_SetBlendMode(int ParticleEmitterHandle, int BlendMode);
	static void NGFX_PARTICLE_EMITTER_SetTexture(int ParticleEmitterHandle, int TextureHandle);
	static void NGFX_PARTICLE_EMITTER_SetForce(int ParticleEmitterHandle, float ForceX, float ForceY);
	static void NGFX_PARTICLE_EMITTER_SetParticleLife(int ParticleEmitterHandle, float Life);
	static void NGFX_PARTICLE_EMITTER_SetParticleLife_Deviation(int ParticleEmitterHandle, float LifeDeviation);
	static void NGFX_PARTICLE_EMITTER_SetNoTimeOut(int ParticleEmitterHandle, int TrueFalse);
	static void NGFX_PARTICLE_EMITTER_SetDrawOrderReversed(int ParticleEmitterHandle, int TrueFalse);
	static void NGFX_PARTICLE_EMITTER_SetProcessWhenOffScreen(int ParticleEmitterHandle, int TrueFalse);
	static void NGFX_PARTICLE_EMITTER_SetUseVelocityParticles(int ParticleEmitterHandle, int TrueFalse);
	static void NGFX_PARTICLE_EMITTER_SetChannelID(int ParticleEmitterHandle, int ChannelID);
	static void NGFX_PARTICLE_EMITTER_Start(int ParticleEmitterHandle);
	static int NGFX_PARTICLE_EMITTER_Clone(int SourceParticleEmitterHandle);
	static void NGFX_PARTICLE_EMITTER_ReleaseAll();
	static void NGFX_PARTICLE_EMITTER_Evolve(int ParticleEmitterHandle, float TimeToEvolveBy);
	static bool NGFX_HasGameRoomChanged(int CurrentGameRoom);
	static bool NGFX_HasGameRoomChanged_SecondTest(int CurrentGameRoom);
	static void NGFX_SPRITE_ResetForNewRoom();
	static int NGFX_SPRITE_Create();
	static void NGFX_SPRITE_Release(int SpriteHandle);
	static void NGFX_SPRITE_SetChannelID(int SpriteHandle, int ChannelID);
	static void NGFX_SPRITE_SetPosition(int SpriteHandle, float x, float y);
	static void NGFX_SPRITE_SetPivot(int SpriteHandle, float x, float y);
	static void NGFX_SPRITE_SetTexture(int SpriteHandle, int TextureHandle);
	static void NGFX_SPRITE_SetBlendMode(int SpriteHandle, int BlendMode);
	static void NGFX_SPRITE_SetWidth(int SpriteHandle, float Width);
	static void NGFX_SPRITE_SetHeight(int SpriteHandle, float Height);
	static void NGFX_SPRITE_SetAngle(int SpriteHandle, float Angle);
	static void NGFX_SPRITE_SetColour_1(int SpriteHandle, int R, int G, int B, int A);
	static void NGFX_SPRITE_SetColour_2(int SpriteHandle, int R, int G, int B, int A);
	static void NGFX_SPRITE_SetColour_3(int SpriteHandle, int R, int G, int B, int A);
	static void NGFX_SPRITE_SetColour_4(int SpriteHandle, int R, int G, int B, int A);
	static void NGFX_SPRITE_SetClipRectangle(int SpriteHandle, float x, float y, float x2, float y2);
	static void NGFX_SPRITE_SetGourard(int SpriteHandle, int TrueFalse);
	static void NGFX_SPRITE_SetFlipped_H(int SpriteHandle, int TrueFalse);
	static void NGFX_SPRITE_SetFlipped_V(int SpriteHandle, int TrueFalse);
	static void NGFX_SPRITE_AddToDrawList(int SpriteHandle);
	static void NGFX_InitForNewGameLoop();

public:
	AGSNickenstienGFX();
};

} // namespace AGSNickenstienGFX
} // namespace Plugins
} // namespace AGS3

#endif
