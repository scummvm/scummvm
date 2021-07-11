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

#include "ags/plugins/ags_waves/ags_waves.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {

IAGSEngine *AGSWaves::_engine;

AGSWaves::AGSWaves() : PluginBase() {
	_engine = nullptr;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSWaves::AGS_GetPluginName() {
	return "AGS Waves";
}

void AGSWaves::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

	SCRIPT_METHOD(DrawScreenEffect);
	SCRIPT_METHOD(SFX_Play);
	SCRIPT_METHOD(SFX_SetVolume); 
	SCRIPT_METHOD(SFX_GetVolume);
	SCRIPT_METHOD(Music_Play);
	SCRIPT_METHOD(Music_GetVolume);
	SCRIPT_METHOD(Music_SetVolume);
	SCRIPT_METHOD(SFX_Stop);
	SCRIPT_METHOD(SFX_SetPosition);
	SCRIPT_METHOD(SFX_SetGlobalVolume);
	SCRIPT_METHOD(Load_SFX);
	SCRIPT_METHOD(Audio_Apply_Filter);
	SCRIPT_METHOD(Audio_Remove_Filter);
	SCRIPT_METHOD(SFX_AllowOverlap);
	SCRIPT_METHOD(SFX_Filter);
	SCRIPT_METHOD(DrawBlur); 
	SCRIPT_METHOD(DrawTunnel);
	SCRIPT_METHOD(DrawCylinder);
	SCRIPT_METHOD(DrawForceField);
	SCRIPT_METHOD(Grayscale);
	SCRIPT_METHOD(ReadWalkBehindIntoSprite);
	SCRIPT_METHOD(AdjustSpriteFont);
	SCRIPT_METHOD(SpriteGradient);
	SCRIPT_METHOD(Outline);
	SCRIPT_METHOD(OutlineOnly);
	SCRIPT_METHOD(SaveVariable);
	SCRIPT_METHOD(ReadVariable);
	SCRIPT_METHOD(GameDoOnceOnly);
	SCRIPT_METHOD(SetGDState);
	SCRIPT_METHOD(GetGDState);
	SCRIPT_METHOD(ResetAllGD);
	SCRIPT_METHOD(SpriteSkew);
	SCRIPT_METHOD(FireUpdate);
	SCRIPT_METHOD(WindUpdate);
	SCRIPT_METHOD(SetWindValues);
	SCRIPT_METHOD(ReturnWidth);
	SCRIPT_METHOD(ReturnHeight);
	SCRIPT_METHOD(ReturnNewHeight);
	SCRIPT_METHOD(ReturnNewWidth);
	SCRIPT_METHOD(Warper);
	SCRIPT_METHOD(SetWarper);
	SCRIPT_METHOD(RainUpdate);
	SCRIPT_METHOD(BlendTwoSprites);
	SCRIPT_METHOD(Blend);
	SCRIPT_METHOD(Dissolve);
	SCRIPT_METHOD(ReverseTransparency);
	SCRIPT_METHOD(NoiseCreator);
	SCRIPT_METHOD(TintProper);
	SCRIPT_METHOD(GetWalkbehindBaserine);
	SCRIPT_METHOD(SetWalkbehindBaserine);
}

void AGSWaves::DrawScreenEffect(ScriptMethodParams &params) {
	//PARAMS4(int, sprite, int, sprite_prev, int, ide, int, n);
}

void AGSWaves::SFX_Play(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, repeat);
}

void AGSWaves::SFX_SetVolume(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, volume);
}

void AGSWaves::SFX_GetVolume(ScriptMethodParams &params) {
	//PARAMS1(int, SFX);
	params._result = 0;
}

void AGSWaves::Music_Play(ScriptMethodParams &params) {
	//PARAMS6(int, MFX, int, repeat, int, fadeinMS, int, fadeoutMS, int, Position, bool, fixclick);
}

void AGSWaves::Music_SetVolume(ScriptMethodParams &params) {
	//PARAMS1(int, volume);
}

void AGSWaves::Music_GetVolume(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSWaves::SFX_Stop(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, fademsOUT);
}

void AGSWaves::SFX_SetPosition(ScriptMethodParams &params) {
	//PARAMS4(int, SFX, int, x, int, y, int, intensity);
}

void AGSWaves::SFX_SetGlobalVolume(ScriptMethodParams &params) {
	//PARAMS1(int, volume);
}

void AGSWaves::Load_SFX(ScriptMethodParams &params) {
	//PARAMS1(int, SFX);
}

void AGSWaves::Audio_Apply_Filter(ScriptMethodParams &params) {
	//PARAMS1(int, Frequency);
}

void AGSWaves::Audio_Remove_Filter(ScriptMethodParams &params) {
}

void AGSWaves::SFX_AllowOverlap(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, allow);
}

void AGSWaves::SFX_Filter(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, enable);
}

void AGSWaves::DrawBlur(ScriptMethodParams &params) {
	//PARAMS2(int, spriteD, int, radius);
}

void AGSWaves::DrawTunnel(ScriptMethodParams &params) {
	//PARAMS3(int, spriteD, float, scale, float, speed);
}

void AGSWaves::DrawCylinder(ScriptMethodParams &params) {
	//PARAMS2(int, spriteD, int, ogsprite);
}

void AGSWaves::DrawForceField(ScriptMethodParams &params) {
	//PARAMS4(int, spriteD, int, scale, float, speed, int, id);
}

void AGSWaves::Grayscale(ScriptMethodParams &params) {
	//PARAMS1(int, sprite);
}

void AGSWaves::ReadWalkBehindIntoSprite(ScriptMethodParams &params) {
	//PARAMS3(int, sprite, int, bgsprite, int, walkbehindBaseline);
}

void AGSWaves::AdjustSpriteFont(ScriptMethodParams &params) {
	//PARAMS5(int, sprite, int, rate, int, outlineRed, int, outlineGreen, int, outlineBlue);
}

void AGSWaves::SpriteGradient(ScriptMethodParams &params) {
	//PARAMS3(int, sprite, int, rate, int, toy);
}

void AGSWaves::Outline(ScriptMethodParams &params) {
	//PARAMS5(int, sprite, int, red, int, ged, int, bed, int, aed);
}

void AGSWaves::OutlineOnly(ScriptMethodParams &params) {
	//PARAMS7(int, sprite, int, refsprite, int, red, int, ged, int, bed, int, aed, int, trans);
}

void AGSWaves::SaveVariable(ScriptMethodParams &params) {
	//PARAMS2(const char *, value, int, id);
}

void AGSWaves::ReadVariable(ScriptMethodParams &params) {
	//PARAMS1(int, id);
	params._result = (const char *)nullptr;
}

void AGSWaves::GameDoOnceOnly(ScriptMethodParams &params) {
	//PARAMS1(const char *, value);
}

void AGSWaves::SetGDState(ScriptMethodParams &params) {
	//PARAMS2(const char *, value, bool, setvalue);
}

void AGSWaves::GetGDState(ScriptMethodParams &params) {
	//PARAMS1(const char *, value);
	params._result = false;
}

void AGSWaves::ResetAllGD(ScriptMethodParams &params) {
}

void AGSWaves::SpriteSkew(ScriptMethodParams &params) {
	//PARAMS5(int, sprite, float, xskewmin, float, yskewmin, float, xskewmax, float, yskewmax);
}

void AGSWaves::FireUpdate(ScriptMethodParams &params) {
	//PARAMS2(int, getDynamicSprite, bool, Fire2Visible));
}

void AGSWaves::WindUpdate(ScriptMethodParams &params) {
	//PARAMS4(int, ForceX, int, ForceY, int, Transparency, int, sprite);
}

void AGSWaves::SetWindValues(ScriptMethodParams &params) {
	//PARAMS4(int, w, int, h, int, pr, int, prev);
}

void AGSWaves::ReturnWidth(ScriptMethodParams &params) {
	//PARAMS8(int, x1, int, y1, int, x2, int, y2, int, x3, int, y3, int, x4, int, y4);
	params._result = 0;
}

void AGSWaves::ReturnHeight(ScriptMethodParams &params) {
	//PARAMS8(int, x1, int, y1, int, x2, int, y2, int, x3, int, y3, int, x4, int, y4);
	params._result = 0;
}

void AGSWaves::ReturnNewHeight(ScriptMethodParams &params) {
}

void AGSWaves::ReturnNewWidth(ScriptMethodParams &params) {
}

void AGSWaves::Warper(ScriptMethodParams &params) {
	//PARAMS5(int, swarp, int, sadjust, int, x1, int, y1, int, x2);
}

void AGSWaves::SetWarper(ScriptMethodParams &params) {
	//PARAMS5(int, y2x, int, x3x, int, y3x, int, x4x, int, y4x);
}

void AGSWaves::RainUpdate(ScriptMethodParams &params) {
	//PARAMS7(int, rdensity, int, FX, int, FY, int, RW, int, RH, int, graphic, float, perc);
}

void AGSWaves::BlendTwoSprites(ScriptMethodParams &params) {
	//PARAMS2(int, graphic, int, refgraphic);
}

void AGSWaves::Blend(ScriptMethodParams &params) {
	//PARAMS4(int, graphic, int, refgraphic, bool, screen, int, perc);
}

void AGSWaves::Dissolve(ScriptMethodParams &params) {
	//PARAMS3(int, graphic, int, noisegraphic, int, disvalue);
}

void AGSWaves::ReverseTransparency(ScriptMethodParams &params) {
	//PARAMS1(int, graphic);
}

void AGSWaves::NoiseCreator(ScriptMethodParams &params) {
	//PARAMS2(int, graphic, int, setA);
}

void AGSWaves::TintProper(ScriptMethodParams &params) {
	//PARAMS7(int, sprite, int, lightx, int, lighty, int, radi, int, rex, int, grx, int, blx);
}

void AGSWaves::GetWalkbehindBaserine(ScriptMethodParams &params) {
	//PARAMS1(int, id);
	params._result = 0;
}

void AGSWaves::SetWalkbehindBaserine(ScriptMethodParams &params) {
	//PARAMS2(int, id, int, base);
}

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3
