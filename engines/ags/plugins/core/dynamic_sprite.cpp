/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

#include "ags/plugins/core/dynamic_sprite.h"
#include "ags/engine/ac/dynamic_sprite.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void DynamicSprite::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(DynamicSprite::ChangeCanvasSize^4, DynamicSprite::ChangeCanvasSize);
	SCRIPT_METHOD(DynamicSprite::CopyTransparencyMask^1, DynamicSprite::CopyTransparencyMask);
	SCRIPT_METHOD(DynamicSprite::Crop^4, DynamicSprite::Crop);
	SCRIPT_METHOD(DynamicSprite::Delete, DynamicSprite::Delete);
	SCRIPT_METHOD(DynamicSprite::Flip^1, DynamicSprite::Flip);
	SCRIPT_METHOD(DynamicSprite::GetDrawingSurface^0, DynamicSprite::GetDrawingSurface);
	SCRIPT_METHOD(DynamicSprite::Resize^2, DynamicSprite::Resize);
	SCRIPT_METHOD(DynamicSprite::Rotate^3, DynamicSprite::Rotate);
	SCRIPT_METHOD(DynamicSprite::SaveToFile^1, DynamicSprite::SaveToFile);
	SCRIPT_METHOD(DynamicSprite::Tint^5, DynamicSprite::Tint);
	SCRIPT_METHOD(DynamicSprite::get_ColorDepth, DynamicSprite::GetColorDepth);
	SCRIPT_METHOD(DynamicSprite::get_Graphic, DynamicSprite::GetGraphic);
	SCRIPT_METHOD(DynamicSprite::get_Height, DynamicSprite::GetHeight);
	SCRIPT_METHOD(DynamicSprite::get_Width, DynamicSprite::GetWidth);
	SCRIPT_METHOD(DynamicSprite::Create^3, DynamicSprite::Create);
	SCRIPT_METHOD(DynamicSprite::CreateFromBackground, DynamicSprite::CreateFromBackground);
	SCRIPT_METHOD(DynamicSprite::CreateFromDrawingSurface^5, DynamicSprite::CreateFromDrawingSurface);
	SCRIPT_METHOD(DynamicSprite::CreateFromExistingSprite^1, DynamicSprite::CreateFromExistingSprite_Old);
	SCRIPT_METHOD(DynamicSprite::CreateFromExistingSprite^2, DynamicSprite::CreateFromExistingSprite);
	SCRIPT_METHOD(DynamicSprite::CreateFromFile, DynamicSprite::CreateFromFile);
	SCRIPT_METHOD(DynamicSprite::CreateFromSaveGame, DynamicSprite::CreateFromSaveGame);
	SCRIPT_METHOD(DynamicSprite::CreateFromScreenShot, DynamicSprite::CreateFromScreenShot);
}

void DynamicSprite::ChangeCanvasSize(ScriptMethodParams &params) {
	PARAMS5(ScriptDynamicSprite *, sds, int, width, int, height, int, x, int, y);
	AGS3::DynamicSprite_ChangeCanvasSize(sds, width, height, x, y);
}

void DynamicSprite::CopyTransparencyMask(ScriptMethodParams &params) {
	PARAMS2(ScriptDynamicSprite *, sds, int, sourceSprite);
	AGS3::DynamicSprite_CopyTransparencyMask(sds, sourceSprite);
}

void DynamicSprite::Crop(ScriptMethodParams &params) {
	PARAMS5(ScriptDynamicSprite *, sds, int, x1, int, y1, int, width, int, height);
	AGS3::DynamicSprite_Crop(sds, x1, y1, width, height);
}

void DynamicSprite::Delete(ScriptMethodParams &params) {
	PARAMS1(ScriptDynamicSprite *, sds);
	AGS3::DynamicSprite_Delete(sds);
}

void DynamicSprite::Flip(ScriptMethodParams &params) {
	PARAMS2(ScriptDynamicSprite *, sds, int, direction);
	AGS3::DynamicSprite_Flip(sds, direction);
}

void DynamicSprite::GetDrawingSurface(ScriptMethodParams &params) {
	PARAMS1(ScriptDynamicSprite *, dss);
	params._result = AGS3::DynamicSprite_GetDrawingSurface(dss);
}

void DynamicSprite::Resize(ScriptMethodParams &params) {
	PARAMS3(ScriptDynamicSprite *, sds, int, width, int, height);
	AGS3::DynamicSprite_Resize(sds, width, height);
}

void DynamicSprite::Rotate(ScriptMethodParams &params) {
	PARAMS4(ScriptDynamicSprite *, sds, int, angle, int, width, int, height);
	AGS3::DynamicSprite_Rotate(sds, angle, width, height);
}

void DynamicSprite::SaveToFile(ScriptMethodParams &params) {
	PARAMS2(ScriptDynamicSprite *, sds, const char *, namm);
	params._result = AGS3::DynamicSprite_SaveToFile(sds, namm);
}

void DynamicSprite::Tint(ScriptMethodParams &params) {
	PARAMS6(ScriptDynamicSprite *, sds, int, red, int, green, int, blue, int, saturation, int, luminance);
	AGS3::DynamicSprite_Tint(sds, red, green, blue, saturation, luminance);
}

void DynamicSprite::GetColorDepth(ScriptMethodParams &params) {
	PARAMS1(ScriptDynamicSprite *, sds);
	params._result = AGS3::DynamicSprite_GetColorDepth(sds);
}

void DynamicSprite::GetGraphic(ScriptMethodParams &params) {
	PARAMS1(ScriptDynamicSprite *, sds);
	params._result = AGS3::DynamicSprite_GetGraphic(sds);
}

void DynamicSprite::GetHeight(ScriptMethodParams &params) {
	PARAMS1(ScriptDynamicSprite *, sds);
	params._result = AGS3::DynamicSprite_GetHeight(sds);
}

void DynamicSprite::GetWidth(ScriptMethodParams &params) {
	PARAMS1(ScriptDynamicSprite *, sds);
	params._result = AGS3::DynamicSprite_GetWidth(sds);
}

void DynamicSprite::Create(ScriptMethodParams &params) {
	PARAMS3(int, width, int, height, int, alphaChannel);
	params._result = AGS3::DynamicSprite_Create(width, height, alphaChannel);
}

void DynamicSprite::CreateFromBackground(ScriptMethodParams &params) {
	PARAMS5(int, frame, int, x1, int, y1, int, width, int, height);
	params._result = AGS3::DynamicSprite_CreateFromBackground(frame, x1, y1, width, height);
}

void DynamicSprite::CreateFromDrawingSurface(ScriptMethodParams &params) {
	PARAMS5(ScriptDrawingSurface *, sds, int, x, int, y, int, width, int, height);
	params._result = AGS3::DynamicSprite_CreateFromDrawingSurface(sds, x, y, width, height);
}

void DynamicSprite::CreateFromExistingSprite_Old(ScriptMethodParams &params) {
	PARAMS1(int, slot);
	params._result = AGS3::DynamicSprite_CreateFromExistingSprite_Old(slot);
}

void DynamicSprite::CreateFromExistingSprite(ScriptMethodParams &params) {
	PARAMS2(int, slot, int, preserveAlphaChannel);
	params._result = AGS3::DynamicSprite_CreateFromExistingSprite(slot, preserveAlphaChannel);
}

void DynamicSprite::CreateFromFile(ScriptMethodParams &params) {
	PARAMS1(const char *, filename);
	params._result = AGS3::DynamicSprite_CreateFromFile(filename);
}

void DynamicSprite::CreateFromSaveGame(ScriptMethodParams &params) {
	PARAMS3(int, sgslot, int, width, int, height);
	params._result = AGS3::DynamicSprite_CreateFromSaveGame(sgslot, width, height);
}

void DynamicSprite::CreateFromScreenShot(ScriptMethodParams &params) {
	PARAMS2(int, width, int, height);
	params._result = AGS3::DynamicSprite_CreateFromScreenShot(width, height);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
