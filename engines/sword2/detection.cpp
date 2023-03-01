/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 */

#include "engines/advancedDetector.h"
#include "engines/obsolete.h"

#include "sword2/detection.h"
#include "sword2/obsolete.h" // Obsolete ID table.

static const PlainGameDescriptor sword2Games[] = {
	{"sword2", "Broken Sword II: The Smoking Mirror"},
	{nullptr, nullptr}
};

#include "sword2/detection_tables.h"

static const char *const directoryGlobs[] = {
	"clusters",
	"smacks",
	"video",
	"extras", // GOG.com
	nullptr
};

class Sword2MetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	Sword2MetaEngineDetection() : AdvancedMetaEngineDetection(Sword2::gameDescriptions, sizeof(ADGameDescription), sword2Games) {
		_guiOptions = GUIO3(GUIO_NOMIDI, GUIO_NOASPECT, GAMEOPTION_OBJECT_LABELS);
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	PlainGameDescriptor findGame(const char *gameId) const override {
		return Engines::findGameID(gameId, _gameIds, obsoleteGameIDsTable);
	}

	const char *getName() const override {
		return "sword2";
	}

	const char *getEngineName() const override {
		return "Broken Sword II: The Smoking Mirror";
	}

	const char *getOriginalCopyright() const override {
		return "Broken Sword II: The Smoking Mirror (C) Revolution";
	}
};

REGISTER_PLUGIN_STATIC(SWORD2_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Sword2MetaEngineDetection);
