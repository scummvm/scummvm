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

#include "engines/advancedDetector.h"

#include "base/plugins.h"
#include "playground3d/playground3d.h"

static const PlainGameDescriptor playground3d_setting[] = {
	{ "playground3d", "Playground 3d: the testing and playground environment for 3d renderers" },
	{ nullptr, nullptr }
};

static const ADGameDescription playground3dDescriptions[] = {
	{
		"playground3d",
		"",
		AD_ENTRY1("PLAYGROUND3D", nullptr),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOLAUNCHLOAD)
	},
	AD_TABLE_END_MARKER
};

class Playground3dMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	Playground3dMetaEngineDetection() : AdvancedMetaEngineDetection(playground3dDescriptions, sizeof(ADGameDescription), playground3d_setting) {
		_md5Bytes = 512;
	}

	const char *getEngineId() const override {
		return "playground3d";
	}

	const char *getName() const override {
		return "Playground 3d: the testing and playground environment for 3d renderers";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) ScummVM";
	}
};

REGISTER_PLUGIN_STATIC(PLAYGROUND3D_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Playground3dMetaEngineDetection);
