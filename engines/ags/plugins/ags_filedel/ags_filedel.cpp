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

#include "ags/plugins/ags_filedel/ags_filedel.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/savefile.h"
#include "common/system.h"

namespace AGS3 {
namespace Plugins {
namespace AGSFileDel {

const char *AGSFileDel::AGS_GetPluginName() {
	return "AGS FileDel";
}

void AGSFileDel::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(FileDelete, AGSFileDel::FileDelete);
}

void AGSFileDel::FileDelete(ScriptMethodParams &params) {
	PARAMS1(const char *, filename);

	if (!filename) {
		warning("AGSFileDel: empty filename!");
		params._result = 0;
	} else {
		Common::String fullPath(ConfMan.get("gameid") + "-" + filename);
		if (g_system->getSavefileManager()->removeSavefile(fullPath)) {
			debug(0, "AGSFileDel: Deleted file %s", fullPath.c_str());
			params._result = 1;
		} else {
			debug(0, "AGSFileDel: Couldn't delete file %s", fullPath.c_str());
			params._result = 0;
		}
	}
}

} // namespace AGSFileDel
} // namespace Plugins
} // namespace AGS3
