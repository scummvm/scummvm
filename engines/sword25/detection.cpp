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
#include "common/translation.h"
#include "engines/advancedDetector.h"

#include "sword25/detection.h"
#include "sword25/detection_tables.h"

static const PlainGameDescriptor sword25Game[] = {
	{"sword25", "Broken Sword 2.5"},
	{0, 0}
};

static const char *directoryGlobs[] = {
	"system", // Used by extracted dats
	0
};

static const ExtraGuiOption sword25ExtraGuiOption = {
	_s("Use English speech"),
	_s("Use English speech instead of German for every language other than German"),
	"english_speech",
	false
};

class Sword25MetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	Sword25MetaEngineDetection() : AdvancedMetaEngineDetection(Sword25::gameDescriptions, sizeof(ADGameDescription), sword25Game) {
		_guiOptions = GUIO1(GUIO_NOMIDI);
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "sword25";
	}

	const char *getName() const override {
		return "Broken Sword 2.5";
	}

	const char *getOriginalCopyright() const override {
		return "Broken Sword 2.5 (C) Malte Thiesen, Daniel Queteschiner and Michael Elsdorfer";
	}

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
};

const ExtraGuiOptions Sword25MetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	options.push_back(sword25ExtraGuiOption);
	return options;
}

REGISTER_PLUGIN_STATIC(SWORD25_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Sword25MetaEngineDetection);
