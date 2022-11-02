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

#include "petka/petka.h"

static const PlainGameDescriptor petkaGames[] = {
	{"petka_demo", "Red Comrades Demo"},
	{"petka1", "Red Comrades 1: Save the Galaxy"},
	{"petka2", "Red Comrades 2: For the Great Justice"},
	{nullptr, nullptr}
};

static const DebugChannelDef debugFlagList[] = {
	{Petka::kPetkaDebugGeneral, "general", "General issues"},
	{Petka::kPetkaDebugResources, "resources", "Resources"},
	{Petka::kPetkaDebugMessagingSystem, "message_system", "Engine message system"},
	{Petka::kPetkaDebugDialogs, "dialogs", "Dialogs"},
	DEBUG_CHANNEL_END
};

#include "petka/detection_tables.h"

class PetkaMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	PetkaMetaEngineDetection() : AdvancedMetaEngineDetection(Petka::gameDescriptions, sizeof(ADGameDescription), petkaGames) {
		_gameIds = petkaGames;
		_maxScanDepth = 2;
	}

	const char *getName() const override {
		return "petka";
	}

	const char *getEngineName() const override {
		return "Red Comrades";
	}

	const char *getOriginalCopyright() const override {
		return "Red Comrades (C) S.K.I.F.";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(PETKA_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PetkaMetaEngineDetection);
