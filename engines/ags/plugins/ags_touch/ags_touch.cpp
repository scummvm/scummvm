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

#include "common/system.h"
#include "ags/plugins/ags_touch/ags_touch.h"

namespace AGS3 {
namespace Plugins {
namespace AGSTouch {

const char *AGSTouch::AGS_GetPluginName() {
	return "Touch device control";
}

void AGSTouch::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(TouchShowKeyboard, AGSTouch::TouchShowKeyboard);
	SCRIPT_METHOD(TouchHideKeyboard, AGSTouch::TouchHideKeyboard);
	SCRIPT_METHOD(TouchIsKeyboardVisible, AGSTouch::TouchIsKeyboardVisible);
}

void AGSTouch::TouchShowKeyboard(ScriptMethodParams &params) {
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
}

void AGSTouch::TouchHideKeyboard(ScriptMethodParams &params) {
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGSTouch::TouchIsKeyboardVisible(ScriptMethodParams &params) {
	params._result = g_system->getFeatureState(OSystem::kFeatureVirtualKeyboard);
}

} // namespace AGSTouch
} // namespace Plugins
} // namespace AGS3
