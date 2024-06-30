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

#include "base/plugins.h"
#include "engines/advancedDetector.h"
//#include "common/translation.h"

static const PlainGameDescriptor dgdsGames[] = {
	{"dgds", "Dynamix DGDS game"},
	{"rise", "Rise of the Dragon"},
	{"china", "Heart of China"},
	{"beamish", "The Adventures of Willy Beamish"},

	{0, 0}
};

#include "dgds/detection_tables.h"

class DgdsMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	DgdsMetaEngineDetection() :
	AdvancedMetaEngineDetection(Dgds::gameDescriptions, dgdsGames) {
		_guiOptions = GUIO1(GUIO_NONE);
	}

	const char* getEngineName() const override {
		return "dgds";
	}

	const char *getName() const override {
		return "DGDS";
	}

	const char *getOriginalCopyright() const override {
		return "Dynamix Game Development System (C) Dynamix";
	}

};

REGISTER_PLUGIN_STATIC(DGDS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, DgdsMetaEngineDetection);
