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

#ifndef AGS_PLUGINS_CORE_SYSTEM_H
#define AGS_PLUGINS_CORE_SYSTEM_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class System : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void GetAudioChannelCount(ScriptMethodParams &params);
	static void GetAudioChannels(ScriptMethodParams &params);
	static void GetCapsLock(ScriptMethodParams &params);
	static void GetColorDepth(ScriptMethodParams &params);
	static void GetGamma(ScriptMethodParams &params);
	static void SetGamma(ScriptMethodParams &params);
	static void GetHardwareAcceleration(ScriptMethodParams &params);
	static void GetNumLock(ScriptMethodParams &params);
	static void GetOS(ScriptMethodParams &params);
	static void GetRuntimeInfo(ScriptMethodParams &params);
	static void GetScreenHeight(ScriptMethodParams &params);
	static void GetScreenWidth(ScriptMethodParams &params);
	static void GetScrollLock(ScriptMethodParams &params);
	static void GetSupportsGammaControl(ScriptMethodParams &params);
	static void GetVersion(ScriptMethodParams &params);
	static void GetViewportHeight(ScriptMethodParams &params);
	static void GetViewportWidth(ScriptMethodParams &params);
	static void GetVolume(ScriptMethodParams &params);
	static void SetVolume(ScriptMethodParams &params);
	static void GetVsync(ScriptMethodParams &params);
	static void SetVsync(ScriptMethodParams &params);
	static void GetWindowed(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
