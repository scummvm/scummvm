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

#include "mtropolis/detection.h"

static const PlainGameDescriptor mTropolisGames[] = {
	{"obsidian", "Obsidian"},
	{"mti", "Muppet Treasure Island"},
	{"albert1", "Uncle Albert's Magical Album"},
	{"albert2", "Uncle Albert's Fabulous Voyage"},
	{"albert3", "Uncle Albert's Mysterious Island"},
	{"spqr", "SPQR: The Empire's Darkest Hour"},
	{"sttgs", "Star Trek: The Game Show"},
	{nullptr, nullptr}
};

#include "mtropolis/detection_tables.h"

static const char *directoryGlobs[] = {
	"Obsidian",
	"RESOURCE",
	"Saved Games",
	"MTPLAY32",
	"Albert",
	"DATA",
	"GAME",
	nullptr
};

class MTropolisMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	MTropolisMetaEngineDetection() : AdvancedMetaEngineDetection(MTropolis::gameDescriptions, sizeof(MTropolis::MTropolisGameDescription), mTropolisGames) {
		_guiOptions = GUIO3(GAMEOPTION_DYNAMIC_MIDI, GAMEOPTION_LAUNCH_DEBUG, GAMEOPTION_ENABLE_SHORT_TRANSITIONS);
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
		_flags = kADFlagCanPlayUnknownVariants;
	}

	const char *getName() const override {
		return "mtropolis";
	}

	const char *getEngineName() const override {
		return "mTropolis";
	}

	const char *getOriginalCopyright() const override {
		return "mTropolis (C) mFactory/Quark";
	}
};

REGISTER_PLUGIN_STATIC(MTROPOLIS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MTropolisMetaEngineDetection);
