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

#include "composer/detection.h"

static const PlainGameDescriptor composerGames[] = {
	{"babayaga", "Magic Tales: Baba Yaga and the Magic Geese"},
	{"darby", "Darby the Dragon"},
	{"gregory", "Gregory and the Hot Air Balloon"},
	{"imoking", "Magic Tales: Imo and the King"},
	{"liam", "Magic Tales: Liam Finds a Story"},
	{"littlesamurai", "Magic Tales: The Little Samurai"},
	{"magictales", "Magic Tales"},
	{"princess", "Magic Tales: The Princess and the Crab"},
	{"sleepingcub", "Magic Tales: Sleeping Cub's Test of Courage"},
	{0, 0}
};

#include "composer/detection_tables.h"

using namespace Composer;

// we match from data too, to stop detection from a non-top-level directory
static const char *directoryGlobs[] = {
	"data",
	"liam",
	"programs",
	"princess",
	"sleepcub",
	0
};

class ComposerMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	ComposerMetaEngineDetection() : AdvancedMetaEngineDetection(Composer::gameDescriptions, sizeof(Composer::ComposerGameDescription), composerGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "composer";
	}

	const char *getName() const override {
		return "Magic Composer";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) 1995-1999 Animation Magic";
	}
};

REGISTER_PLUGIN_STATIC(COMPOSER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ComposerMetaEngineDetection);
