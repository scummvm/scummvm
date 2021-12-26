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

#ifndef AGS_PLUGINS_CORE_AUDIO_CHANNEL_H
#define AGS_PLUGINS_CORE_AUDIO_CHANNEL_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class AudioChannel : public ScriptContainer {
	BUILT_IN_HASH(AudioChannel)
public:
	virtual ~AudioChannel() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void Seek(ScriptMethodParams &params);
	void SetRoomLocation(ScriptMethodParams &params);
	void Stop(ScriptMethodParams &params);
	void GetID(ScriptMethodParams &params);
	void GetIsPlaying(ScriptMethodParams &params);
	void GetLengthMs(ScriptMethodParams &params);
	void GetPanning(ScriptMethodParams &params);
	void SetPanning(ScriptMethodParams &params);
	void GetPlayingClip(ScriptMethodParams &params);
	void GetPosition(ScriptMethodParams &params);
	void GetPositionMs(ScriptMethodParams &params);
	void GetVolume(ScriptMethodParams &params);
	void SetVolume(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
