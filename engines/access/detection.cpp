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
#include "access/detection.h"

static const PlainGameDescriptor AccessGames[] = {
	{"amazon", "Amazon: Guardians of Eden"},
	{"martian", "Martian Memorandum"},
	{0, 0}
};

#include "access/detection_tables.h"

class AccessMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	AccessMetaEngineDetection() : AdvancedMetaEngineDetection(Access::gameDescriptions, sizeof(Access::AccessGameDescription), AccessGames) {
		_maxScanDepth = 3;
	}

	const char *getEngineId() const override {
		return "access";
	}

	const char *getName() const override {
		return "Access";
	}

	const char *getOriginalCopyright() const override {
		return "Access Engine (C) 1989-1994 Access Software";
	}
};


REGISTER_PLUGIN_STATIC(ACCESS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AccessMetaEngineDetection);
