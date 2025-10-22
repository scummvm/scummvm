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
#include "common/translation.h"
#include "common/util.h"
#include "gamos/detection.h"
#include "gamos/detection_tables.h"

class GamosMetaEngineDetection : public AdvancedMetaEngineDetection<Gamos::GamosGameDescription> {

public:
	GamosMetaEngineDetection(): AdvancedMetaEngineDetection(
		    Gamos::gameDescriptions, Gamos::gamosGames) {
	}
	~GamosMetaEngineDetection() override {}

	const char *getName() const override {
		return "gamos";
	}

	const char *getEngineName() const override {
		return "Gamos";
	}

	const char *getOriginalCopyright() const override {
		return "Gamos (C)";
	}

};


REGISTER_PLUGIN_STATIC(GAMOS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, GamosMetaEngineDetection);
