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

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/str-array.h"
#include "common/util.h"
#include "hpl1/detection.h"
#include "hpl1/detection_tables.h"
#include "hpl1/debug.h"

const DebugChannelDef Hpl1MetaEngineDetection::debugFlagList[] = {
	{ Hpl1::kDebugGraphics, "Graphics", "Graphics debug channel" },
	{ Hpl1::kDebugResourceLoading, "ResourceLoading", "Resource loading debug channel" },
	{ Hpl1::kDebugFilePath, "FilePath", "File path debug channel" },
	{ Hpl1::kDebugOpenGL, "OpenGL", "OpenGL debug channel"},
	{ Hpl1::kDebugRenderer, "Renderer", "Rederer debug channel"},
	{ Hpl1::kDebugAudio, "Audio", "Audio debug channel"},
	{ Hpl1::kDebugSaves, "Saves", "Channel for debugging game saving and loading"},
	{ Hpl1::kDebugTextures, "Textures", "Texture debug channel"},
	{ Hpl1::kDebugScripts, "Scripts", "Scripts debug channel"},
	DEBUG_CHANNEL_END
};

Hpl1MetaEngineDetection::Hpl1MetaEngineDetection() : AdvancedMetaEngineDetection(Hpl1::GAME_DESCRIPTIONS,
	sizeof(ADGameDescription), Hpl1::GAME_NAMES) {
}

REGISTER_PLUGIN_STATIC(HPL1_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Hpl1MetaEngineDetection);
