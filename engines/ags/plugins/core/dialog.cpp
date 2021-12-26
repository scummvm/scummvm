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

#include "ags/plugins/core/dialog.h"
#include "ags/engine/ac/dialog.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Dialog::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Dialog::get_ID, Dialog::GetID);
	SCRIPT_METHOD(Dialog::get_OptionCount, Dialog::GetOptionCount);
	SCRIPT_METHOD(Dialog::get_ShowTextParser, Dialog::GetShowTextParser);
	SCRIPT_METHOD(Dialog::DisplayOptions^1, Dialog::DisplayOptions);
	SCRIPT_METHOD(Dialog::GetOptionState^1, Dialog::GetOptionState);
	SCRIPT_METHOD(Dialog::GetOptionText^1, Dialog::GetOptionText);
	SCRIPT_METHOD(Dialog::HasOptionBeenChosen^1, Dialog::HasOptionBeenChosen);
	SCRIPT_METHOD(Dialog::SetOptionState^2, Dialog::SetOptionState);
	SCRIPT_METHOD(Dialog::Start^0, Dialog::Start);
}

void Dialog::GetID(ScriptMethodParams &params) {
	PARAMS1(ScriptDialog *, sd);
	params._result = AGS3::Dialog_GetID(sd);
}

void Dialog::GetOptionCount(ScriptMethodParams &params) {
	PARAMS1(ScriptDialog *, sd);
	params._result = AGS3::Dialog_GetOptionCount(sd);
}

void Dialog::GetShowTextParser(ScriptMethodParams &params) {
	PARAMS1(ScriptDialog *, sd);
	params._result = AGS3::Dialog_GetShowTextParser(sd);
}

void Dialog::DisplayOptions(ScriptMethodParams &params) {
	PARAMS2(ScriptDialog *, sd, int, sayChosenOption);
	params._result = AGS3::Dialog_DisplayOptions(sd, sayChosenOption);
}

void Dialog::GetOptionState(ScriptMethodParams &params) {
	PARAMS2(ScriptDialog *, sd, int, option);
	params._result = AGS3::Dialog_GetOptionState(sd, option);
}

void Dialog::GetOptionText(ScriptMethodParams &params) {
	PARAMS2(ScriptDialog *, sd, int, option);
	params._result = AGS3::Dialog_GetOptionText(sd, option);
}

void Dialog::HasOptionBeenChosen(ScriptMethodParams &params) {
	PARAMS2(ScriptDialog *, sd, int, option);
	params._result = AGS3::Dialog_HasOptionBeenChosen(sd, option);
}

void Dialog::SetOptionState(ScriptMethodParams &params) {
	PARAMS3(ScriptDialog *, sd, int, option, int, newState);
	AGS3::Dialog_SetOptionState(sd, option, newState);
}

void Dialog::Start(ScriptMethodParams &params) {
	PARAMS1(ScriptDialog *, sd);
	AGS3::Dialog_Start(sd);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
