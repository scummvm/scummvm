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

#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "audio/mididrv.h"

#include "snatcher/detection.h"
#include "snatcher/detection_tables.h"

class SnatcherMetaEngineDetection : public AdvancedMetaEngineDetection<SnatcherGameDescription> {
public:
	SnatcherMetaEngineDetection() : AdvancedMetaEngineDetection(adGameDescs, gameList) {
		_md5Bytes = 1024 * 1024;
		_maxScanDepth = 2;
		_directoryGlobs = 0;
	}

	const char *getName() const override {
		return "snatcher";
	}

	const char *getEngineName() const override {
		return "snatcher";
	}

	const char *getOriginalCopyright() const override {
		return "Snatcher (C) Konami";
	}
};

REGISTER_PLUGIN_STATIC(SNATCHER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, SnatcherMetaEngineDetection);
