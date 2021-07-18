/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include "ags/plugins/core/system.h"
#include "ags/engine/ac/system.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void System::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(System::get_AudioChannelCount, GetAudioChannelCount);
	SCRIPT_METHOD_EXT(System::geti_AudioChannels, GetAudioChannels);
	SCRIPT_METHOD_EXT(System::get_CapsLock, GetCapsLock);
	SCRIPT_METHOD_EXT(System::get_ColorDepth, GetColorDepth);
	SCRIPT_METHOD_EXT(System::get_Gamma, GetGamma);
	SCRIPT_METHOD_EXT(System::set_Gamma, SetGamma);
	SCRIPT_METHOD_EXT(System::get_HardwareAcceleration, GetHardwareAcceleration);
	SCRIPT_METHOD_EXT(System::get_NumLock, GetNumLock);
	SCRIPT_METHOD_EXT(System::get_OperatingSystem, GetOS);
	SCRIPT_METHOD_EXT(System::get_RuntimeInfo, GetRuntimeInfo);
	SCRIPT_METHOD_EXT(System::get_ScreenHeight, GetScreenHeight);
	SCRIPT_METHOD_EXT(System::get_ScreenWidth, GetScreenWidth);
	SCRIPT_METHOD_EXT(System::get_ScrollLock, GetScrollLock);
	SCRIPT_METHOD_EXT(System::get_SupportsGammaControl, GetSupportsGammaControl);
	SCRIPT_METHOD_EXT(System::get_Version, GetVersion);
	SCRIPT_METHOD_EXT(SystemInfo::get_Version, GetVersion);
	SCRIPT_METHOD_EXT(System::get_ViewportHeight, GetViewportHeight);
	SCRIPT_METHOD_EXT(System::get_ViewportWidth, GetViewportWidth);
	SCRIPT_METHOD_EXT(System::get_Volume, GetVolume);
	SCRIPT_METHOD_EXT(System::set_Volume, SetVolume);
	SCRIPT_METHOD_EXT(System::get_VSync, GetVsync);
	SCRIPT_METHOD_EXT(System::set_VSync, SetVsync);
	SCRIPT_METHOD_EXT(System::get_Windowed, GetWindowed);
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
