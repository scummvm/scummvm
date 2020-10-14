/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"
#include "common/language.h"
#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "twine/detection.h"

static const PlainGameDescriptor twineGames[] = {
	{ "twine", "Little Big Adventure" },
	{ nullptr,  nullptr }
};

static const ADGameDescription twineGameDescriptions[] = {
	{
		"twine",
		"GOG 1.0",
		AD_ENTRY1s("text.hqr", "ae7343552f8fbd17a1fc6cea2197a912", 248654),
		Common::EN_ANY,
		Common::kPlatformDOS,
		0,
		GUIO1(GUIO_NONE)
	},
	AD_TABLE_END_MARKER
};

class TwinEMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	TwinEMetaEngineDetection() : AdvancedMetaEngineDetection(twineGameDescriptions, sizeof(ADGameDescription), twineGames) {
	}

	const char *getEngineId() const override {
		return "twine";
	}

	const char *getName() const override {
		return "Little Big Adventure";
	}

	const char *getOriginalCopyright() const override {
		return "Little Big Adventure (C) Adeline Software International";
	}
};

REGISTER_PLUGIN_STATIC(TWINE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TwinEMetaEngineDetection);
