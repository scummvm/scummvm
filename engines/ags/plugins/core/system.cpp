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

#include "ags/plugins/core/system.h"
#include "ags/engine/ac/system.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void System::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(System::get_AudioChannelCount, System::GetAudioChannelCount);
	SCRIPT_METHOD(System::geti_AudioChannels, System::GetAudioChannels);
	SCRIPT_METHOD(System::get_CapsLock, System::GetCapsLock);
	SCRIPT_METHOD(System::get_ColorDepth, System::GetColorDepth);
	SCRIPT_METHOD(System::get_Gamma, System::GetGamma);
	SCRIPT_METHOD(System::set_Gamma, System::SetGamma);
	SCRIPT_METHOD(System::get_HardwareAcceleration, System::GetHardwareAcceleration);
	SCRIPT_METHOD(System::get_NumLock, System::GetNumLock);
	SCRIPT_METHOD(System::get_OperatingSystem, System::GetOS);
	SCRIPT_METHOD(System::get_RuntimeInfo, System::GetRuntimeInfo);
	SCRIPT_METHOD(System::get_ScreenHeight, System::GetScreenHeight);
	SCRIPT_METHOD(System::get_ScreenWidth, System::GetScreenWidth);
	SCRIPT_METHOD(System::get_ScrollLock, System::GetScrollLock);
	SCRIPT_METHOD(System::get_SupportsGammaControl, System::GetSupportsGammaControl);
	SCRIPT_METHOD(System::get_Version, System::GetVersion);
	SCRIPT_METHOD(SystemInfo::get_Version, System::GetVersion);
	SCRIPT_METHOD(System::get_ViewportHeight, System::GetViewportHeight);
	SCRIPT_METHOD(System::get_ViewportWidth, System::GetViewportWidth);
	SCRIPT_METHOD(System::get_Volume, System::GetVolume);
	SCRIPT_METHOD(System::set_Volume, System::SetVolume);
	SCRIPT_METHOD(System::get_VSync, System::GetVsync);
	SCRIPT_METHOD(System::set_VSync, System::SetVsync);
	SCRIPT_METHOD(System::get_Windowed, System::GetWindowed);
}

void System::GetAudioChannelCount(ScriptMethodParams &params) {
	params._result = AGS3::System_GetAudioChannelCount();
}

void System::GetAudioChannels(ScriptMethodParams &params) {
	PARAMS1(int, index);
	params._result = AGS3::System_GetAudioChannels(index);
}

void System::GetCapsLock(ScriptMethodParams &params) {
	params._result = AGS3::System_GetCapsLock();
}

void System::GetColorDepth(ScriptMethodParams &params) {
	params._result = AGS3::System_GetColorDepth();
}

void System::GetGamma(ScriptMethodParams &params) {
	params._result = AGS3::System_GetGamma();
}

void System::SetGamma(ScriptMethodParams &params) {
	PARAMS1(int, newValue);
	AGS3::System_SetGamma(newValue);
}

void System::GetHardwareAcceleration(ScriptMethodParams &params) {
	params._result = AGS3::System_GetHardwareAcceleration();
}

void System::GetNumLock(ScriptMethodParams &params) {
	params._result = AGS3::System_GetNumLock();
}

void System::GetOS(ScriptMethodParams &params) {
	params._result = AGS3::System_GetOS();
}

void System::GetRuntimeInfo(ScriptMethodParams &params) {
	params._result = AGS3::System_GetRuntimeInfo();
}

void System::GetScreenHeight(ScriptMethodParams &params) {
	params._result = AGS3::System_GetScreenHeight();
}

void System::GetScreenWidth(ScriptMethodParams &params) {
	params._result = AGS3::System_GetScreenWidth();
}

void System::GetScrollLock(ScriptMethodParams &params) {
	params._result = AGS3::System_GetScrollLock();
}

void System::GetSupportsGammaControl(ScriptMethodParams &params) {
	params._result = AGS3::System_GetSupportsGammaControl();
}

void System::GetVersion(ScriptMethodParams &params) {
	params._result = AGS3::System_GetVersion();
}

void System::GetViewportHeight(ScriptMethodParams &params) {
	params._result = AGS3::System_GetViewportHeight();
}

void System::GetViewportWidth(ScriptMethodParams &params) {
	params._result = AGS3::System_GetViewportWidth();
}

void System::GetVolume(ScriptMethodParams &params) {
	params._result = AGS3::System_GetVolume();
}

void System::SetVolume(ScriptMethodParams &params) {
	PARAMS1(int, newvol);
	AGS3::System_SetVolume(newvol);
}

void System::GetVsync(ScriptMethodParams &params) {
	params._result = AGS3::System_GetVsync();
}

void System::SetVsync(ScriptMethodParams &params) {
	PARAMS1(int, newValue);
	AGS3::System_SetVsync(newValue);
}

void System::GetWindowed(ScriptMethodParams &params) {
	params._result = AGS3::System_GetWindowed();
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
