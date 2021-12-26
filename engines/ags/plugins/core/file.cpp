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

#include "ags/plugins/core/file.h"
#include "ags/engine/ac/file.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void File::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(File::Delete^1, File::Delete);
	SCRIPT_METHOD(File::Exists^1, File::Exists);
	SCRIPT_METHOD(File::Open^2, File::OpenFile);
	SCRIPT_METHOD(File::Close^0, File::Close);
	SCRIPT_METHOD(File::ReadInt^0, File::ReadInt);
	SCRIPT_METHOD(File::ReadRawChar^0, File::ReadRawChar);
	SCRIPT_METHOD(File::ReadRawInt^0, File::ReadRawInt);
	SCRIPT_METHOD(File::ReadRawLine^1, File::ReadRawLine);
	SCRIPT_METHOD(File::ReadRawLineBack^0, File::ReadRawLineBack);
	SCRIPT_METHOD(File::ReadString^1, File::ReadString);
	SCRIPT_METHOD(File::ReadStringBack^0, File::ReadStringBack);
	SCRIPT_METHOD(File::WriteInt^1, File::WriteInt);
	SCRIPT_METHOD(File::WriteRawChar^1, File::WriteRawChar);
	SCRIPT_METHOD(File::WriteRawLine^1, File::WriteRawLine);
	SCRIPT_METHOD(File::WriteString^1, File::WriteString);
	SCRIPT_METHOD(File::get_EOF, File::GetEOF);
	SCRIPT_METHOD(File::get_Error, File::GetError);
}

void File::Delete(ScriptMethodParams &params) {
	PARAMS1(const char *, fnmm);
	params._result = AGS3::File_Delete(fnmm);
}

void File::Exists(ScriptMethodParams &params) {
	PARAMS1(const char *, fnmm);
	params._result = AGS3::File_Exists(fnmm);
}

void File::OpenFile(ScriptMethodParams &params) {
	PARAMS2(const char *, fnmm, int, mode);
	params._result = AGS3::sc_OpenFile(fnmm, mode);
}

void File::Close(ScriptMethodParams &params) {
	PARAMS1(sc_File *, fil);
	AGS3::File_Close(fil);
}

void File::ReadInt(ScriptMethodParams &params) {
	PARAMS1(sc_File *, fil);
	params._result = AGS3::File_ReadInt(fil);
}

void File::ReadRawChar(ScriptMethodParams &params) {
	PARAMS1(sc_File *, fil);
	params._result = AGS3::File_ReadRawChar(fil);
}

void File::ReadRawInt(ScriptMethodParams &params) {
	PARAMS1(sc_File *, fil);
	params._result = AGS3::File_ReadRawInt(fil);
}

void File::ReadRawLine(ScriptMethodParams &params) {
	PARAMS2(sc_File *, fil, char *, buffer);
	AGS3::File_ReadRawLine(fil, buffer);
}

void File::ReadRawLineBack(ScriptMethodParams &params) {
	PARAMS1(sc_File *, fil);
	params._result = AGS3::File_ReadRawLineBack(fil);
}

void File::ReadString(ScriptMethodParams &params) {
	PARAMS1(sc_File *, fil);
	params._result = AGS3::File_ReadInt(fil);
}

void File::ReadStringBack(ScriptMethodParams &params) {
	PARAMS1(sc_File *, fil);
	params._result = AGS3::File_ReadStringBack(fil);
}

void File::WriteInt(ScriptMethodParams &params) {
	PARAMS2(sc_File *, fil, int, towrite);
	AGS3::File_WriteInt(fil, towrite);
}

void File::WriteRawChar(ScriptMethodParams &params) {
	PARAMS2(sc_File *, fil, int, towrite);
	AGS3::File_WriteRawChar(fil, towrite);
}

void File::WriteRawLine(ScriptMethodParams &params) {
	PARAMS2(sc_File *, fil, const char *, toWrite);
	AGS3::File_WriteRawLine(fil, toWrite);
}

void File::WriteString(ScriptMethodParams &params) {
	PARAMS2(sc_File *, fil, const char *, toWrite);
	AGS3::File_WriteString(fil, toWrite);
}

void File::GetEOF(ScriptMethodParams &params) {
	PARAMS1(sc_File *, fil);
	params._result = AGS3::File_GetEOF(fil);
}

void File::GetError(ScriptMethodParams &params) {
	PARAMS1(sc_File *, fil);
	params._result = AGS3::File_GetError(fil);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
