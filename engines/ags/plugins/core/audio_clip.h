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

#ifndef AGS_PLUGINS_CORE_AUDIO_CLIP_H
#define AGS_PLUGINS_CORE_AUDIO_CLIP_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class AudioClip : public ScriptContainer {
	BUILT_IN_HASH(AudioClip)
public:
	virtual ~AudioClip() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void Play(ScriptMethodParams &params);
	void PlayFrom(ScriptMethodParams &params);
	void PlayQueued(ScriptMethodParams &params);
	void Stop(ScriptMethodParams &params);
	void GetFileType(ScriptMethodParams &params);
	void GetIsAvailable(ScriptMethodParams &params);
	void GetType(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
