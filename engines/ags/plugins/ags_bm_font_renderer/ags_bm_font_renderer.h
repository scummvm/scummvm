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

#ifndef AGS_PLUGINS_AGS_BM_FONT_RENDERER
#define AGS_PLUGINS_AGS_BM_FONT_RENDERER

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace AGSBMFontRenderer {

class AGSBMFontRenderer : public PluginBase {
	SCRIPT_HASH(AGSBMFontRenderer)

private:
	void SetBMFont(ScriptMethodParams &params);

public:
	AGSBMFontRenderer() : PluginBase() {}
	virtual ~AGSBMFontRenderer() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *engine) override;
};

} // namespace AGSBMFontRenderer
} // namespace Plugins
} // namespace AGS3

#endif
