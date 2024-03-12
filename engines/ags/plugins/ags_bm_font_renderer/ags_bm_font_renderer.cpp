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

#include "ags/plugins/ags_bm_font_renderer/ags_bm_font_renderer.h"
#include "common/debug.h"

namespace AGS3 {
namespace Plugins {
namespace AGSBMFontRenderer {

const char *AGSBMFontRenderer::AGS_GetPluginName() {
	return "AGS BMFontRenderer Plugin stub (agsbmfontrenderer.dll)";
}

void AGSBMFontRenderer::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(SetBMFont, AGSBMFontRenderer::SetBMFont);
}

void AGSBMFontRenderer::SetBMFont(ScriptMethodParams &params) {
	//	PARAMS2(const char *, file, int, fontNumber);

	debug("AGSBMFontRenderer: SetBMFont is not implemented!");
}

} // namespace AGSBMFontRenderer
} // namespace Plugins
} // namespace AGS3
