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

#include "ags/plugins/ags_trans/ags_trans.h"

namespace AGS3 {
namespace Plugins {
namespace AGSTrans {

const char *AGSTrans::AGS_GetPluginName() {
	return "AGS Trans";
}

void AGSTrans::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(GenerateArray, AGSTrans::GenerateArray);
	SCRIPT_METHOD(LoadArray, AGSTrans::LoadArray);
	SCRIPT_METHOD(CreateTransparentOverlay, AGSTrans::CreateTransparentOverlay);
	SCRIPT_METHOD(CreateColorisedOverlay, AGSTrans::CreateColorisedOverlay);
	SCRIPT_METHOD(RemoveTransOverlay, AGSTrans::RemoveTransOverlay);
	SCRIPT_METHOD(ChangeOverlayAlpha, AGSTrans::ChangeOverlayAlpha);
	SCRIPT_METHOD(ChangeOverlayPosition, AGSTrans::ChangeOverlayPosition);
}

void AGSTrans::GenerateArray(ScriptMethodParams &params) {
	// TODO
	warning("STUB: AGSTrans::GenerateArray");
}

void AGSTrans::LoadArray(ScriptMethodParams &params) {
	// TODO
	warning("STUB: AGSTrans::LoadArray");
}

void AGSTrans::CreateTransparentOverlay(ScriptMethodParams &params) {
	// TODO
	warning("STUB: AGSTrans::CreateTransparentOverlay");
}

void AGSTrans::CreateColorisedOverlay(ScriptMethodParams &params) {
	// TODO
	warning("STUB: AGSTrans::CreateColorisedOverlay");
}

void AGSTrans::RemoveTransOverlay(ScriptMethodParams &params) {
	// TODO
	warning("STUB: AGSTrans::RemoveTransOverlay");
}

void AGSTrans::ChangeOverlayAlpha(ScriptMethodParams &params) {
	// TODO
	warning("STUB: AGSTrans::ChangeOverlayAlpha");
}

void AGSTrans::ChangeOverlayPosition(ScriptMethodParams &params) {
	// TODO
	warning("STUB: AGSTrans::ChangeOverlayPosition");
}

} // namespace AGSTrans
} // namespace Plugins
} // namespace AGS3
