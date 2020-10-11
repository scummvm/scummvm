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

#include "common/scummsys.h"

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "common/translation.h"
#include "common/str-array.h"

#include "zvision/detection.h"
#include "zvision/detection_tables.h"

class ZVisionMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	ZVisionMetaEngineDetection() : AdvancedMetaEngineDetection(ZVision::gameDescriptions, sizeof(ZVision::ZVisionGameDescription), ZVision::zVisionGames, ZVision::optionsList) {
		_maxScanDepth = 2;
		_directoryGlobs = ZVision::directoryGlobs;
	}

	const char *getEngineId() const override {
		return "zvision";
	}

	const char *getName() const override {
		return "Z-Vision";
	}

	const char *getOriginalCopyright() const override {
		return "Z-Vision (C) 1996 Activision";
	}
};

REGISTER_PLUGIN_STATIC(ZVISION_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ZVisionMetaEngineDetection);
