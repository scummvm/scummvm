/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGSCONTROLLER_AGSCONTROLLER_ARCNOR_H
#define AGS_PLUGINS_AGSCONTROLLER_AGSCONTROLLER_ARCNOR_H

#include "ags/plugins/ags_controller/ags_controller.h"

namespace AGS3 {
namespace Plugins {
namespace AGSController {

class AGSControllerArcnor : public AGSController {
	SCRIPT_HASH_SUB(AGSControllerArcnor, AGSController)
private:
	void Controller_IsButtonUpOnce(ScriptMethodParams &params);
	void Controller_PressAnyButton(ScriptMethodParams &params);
	void Controller_GetPlayerIndex(ScriptMethodParams &params);
	void Controller_SetPlayerIndex(ScriptMethodParams &params);
	void RunVirtualKeyboard(ScriptMethodParams &params);

public:
	AGSControllerArcnor() : AGSController() {}
	virtual ~AGSControllerArcnor() {}

	void AGS_EngineStartup(IAGSEngine *engine) override;
};

} // namespace AGSController
} // namespace Plugins
} // namespace AGS3

#endif
