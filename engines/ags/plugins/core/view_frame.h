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

#ifndef AGS_PLUGINS_CORE_VIEW_FRAME_H
#define AGS_PLUGINS_CORE_VIEW_FRAME_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class ViewFrame : public ScriptContainer {
	BUILT_IN_HASH(ViewFrame)
public:
	virtual ~ViewFrame() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void GetFlipped(ScriptMethodParams &params);
	void GetFrame(ScriptMethodParams &params);
	void GetGraphic(ScriptMethodParams &params);
	void SetGraphic(ScriptMethodParams &params);
	void GetLinkedAudio(ScriptMethodParams &params);
	void SetLinkedAudio(ScriptMethodParams &params);
	void GetLoop(ScriptMethodParams &params);
	void GetSound(ScriptMethodParams &params);
	void SetSound(ScriptMethodParams &params);
	void GetSpeed(ScriptMethodParams &params);
	void GetView(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
