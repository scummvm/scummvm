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
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"

#include "toltecs/toltecs.h"
#include "toltecs/detection.h"

static const PlainGameDescriptor toltecsGames[] = {
	{"toltecs", "3 Skulls of the Toltecs"},
	{0, 0}
};


#include "toltecs/detection_tables.h"

class ToltecsMetaEngineDetection : public AdvancedMetaEngineDetection<Toltecs::ToltecsGameDescription> {
public:
	ToltecsMetaEngineDetection() : AdvancedMetaEngineDetection(Toltecs::gameDescriptions, toltecsGames) {
		_flags = kADFlagMatchFullPaths;
	}

	const char *getName() const override {
		return "toltecs";
	}

	const char *getEngineName() const override {
		return "3 Skulls of the Toltecs";
	}

	const char *getOriginalCopyright() const override {
		return "3 Skulls of the Toltecs (C) Revistronic 1996";
	}
};

REGISTER_PLUGIN_STATIC(TOLTECS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ToltecsMetaEngineDetection);
