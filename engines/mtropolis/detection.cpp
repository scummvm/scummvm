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
#include "engines/advancedDetector.h"

#include "common/config-manager.h"
#include "common/translation.h"

#include "mtropolis/detection.h"

static const PlainGameDescriptor mTropolisGames[] = {
	{"obsidian", "Obsidian"},
	{nullptr, nullptr}
};

#include "mtropolis/detection_tables.h"

static const char *directoryGlobs[] = {
	"Obsidian",
	"RESOURCE",
	"Saved Games",
	nullptr
};

class MTropolisMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	MTropolisMetaEngineDetection() : AdvancedMetaEngineDetection(MTropolis::gameDescriptions, sizeof(MTropolis::MTropolisGameDescription), mTropolisGames) {
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "mtropolis";
	}

	const char *getName() const override {
		return "mTropolis";
	}

	const char *getOriginalCopyright() const override {
		return "mTropolis (C) mFactory/Quark";
	}

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
};

const ExtraGuiOptions MTropolisMetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;

	Common::String gameid = ConfMan.get("gameid", target);

	if (gameid == "obsidian") {
		static const ExtraGuiOption widescreenOption = {
			_s("16:9 Widescreen Mod"),
			_s("Removes letterboxing and moves some display elements, improving coverage on widescreen displays"),
			"mtropolis_mod_obsidian_widescreen",
			false,
			0,
			0};

		options.push_back(widescreenOption);
	}

	static const ExtraGuiOption dynamicMIDIOption = {
		_s("Improved Music Mixing"),
		_s("Enables dynamic MIDI mixer, improving quality, but behaving less like mTropolis Player."),
		"mtropolis_mod_dynamic_midi",
		true,
		0,
		0};

	static const ExtraGuiOption launchDebugOption = {
		_s("Start with debugger"),
		_s("Starts with the debugger dashboard active"),
		"mtropolis_debug_at_start",
		false,
		0,
		0
	};
	static const ExtraGuiOption launchBreakOption = {
		_s("Start debugging immediately"),
		_s("Halts progress and stops at the debugger immediately"),
		"mtropolis_pause_at_start",
		false,
		0,
		0
	};

	options.push_back(dynamicMIDIOption);
	options.push_back(launchDebugOption);
	options.push_back(launchBreakOption);

	return options;
}

REGISTER_PLUGIN_STATIC(MTROPOLIS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MTropolisMetaEngineDetection);
