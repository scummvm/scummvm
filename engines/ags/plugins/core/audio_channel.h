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

#ifndef AGS_PLUGINS_CORE_AUDIO_CHANNEL_H
#define AGS_PLUGINS_CORE_AUDIO_CHANNEL_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class AudioChannel : public ScriptContainer {
public:
	static void AGS_EngineStartup(IAGSEngine *engine);

	static void Seek(ScriptMethodParams &params);
	static void SetRoomLocation(ScriptMethodParams &params);
	static void Stop(ScriptMethodParams &params);
	static void GetID(ScriptMethodParams &params);
	static void GetIsPlaying(ScriptMethodParams &params);
	static void GetLengthMs(ScriptMethodParams &params);
	static void GetPanning(ScriptMethodParams &params);
	static void SetPanning(ScriptMethodParams &params);
	static void GetPlayingClip(ScriptMethodParams &params);
	static void GetPosition(ScriptMethodParams &params);
	static void GetPositionMs(ScriptMethodParams &params);
	static void GetVolume(ScriptMethodParams &params);
	static void SetVolume(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
