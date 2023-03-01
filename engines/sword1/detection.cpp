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

#include "common/fs.h"
#include "common/gui_options.h"

#include "engines/advancedDetector.h"
#include "engines/obsolete.h"

#include "sword1/obsolete.h" // Obsolete ID table.

static const PlainGameDescriptor swordGames[] = {
	{"sword1", "Broken Sword: The Shadow of the Templars"},
	{nullptr, nullptr}
};

#include "sword1/detection_tables.h"

static const char *const directoryGlobs[] = {
	"smackshi",
	"video",
	nullptr
};

class SwordMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	SwordMetaEngineDetection() : AdvancedMetaEngineDetection(Sword1::gameDescriptions, sizeof(ADGameDescription), swordGames) {
		_guiOptions = GUIO2(GUIO_NOMIDI, GUIO_NOASPECT);
		_flags = kADFlagMatchFullPaths;
		_directoryGlobs = directoryGlobs;
	}

	PlainGameDescriptor findGame(const char *gameId) const override {
		return Engines::findGameID(gameId, _gameIds, obsoleteGameIDsTable);
	}

	const char *getName() const override {
		return "sword1";
	}

	const char *getEngineName() const override {
		return "Broken Sword: The Shadow of the Templars";
	}

	const char *getOriginalCopyright() const override {
		return "Broken Sword: The Shadow of the Templars (C) Revolution";
	}
};

REGISTER_PLUGIN_STATIC(SWORD1_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, SwordMetaEngineDetection);
