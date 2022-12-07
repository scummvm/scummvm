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

#include "engines/advancedDetector.h"

static const PlainGameDescriptor agdsGames[] = {
    {"nibiru", "NiBiRu: Age of Secrets"},
    {"black-mirror", "Black Mirror"},
    {0, 0}};

#include "agds/detection_tables.h"

class AGDSMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	AGDSMetaEngineDetection() : AdvancedMetaEngineDetection(AGDS::gameDescriptions, sizeof(ADGameDescription), agdsGames) {
		_maxScanDepth = 3;
	}

	const char *getName() const override {
		return "agds";
	}

	const char *getEngineName() const override {
		return "AGDS Engine";
	}

	const char *getOriginalCopyright() const override {
		return "AGDS (C) Future Games";
	}
};

REGISTER_PLUGIN_STATIC(AGDS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AGDSMetaEngineDetection);
