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

#include "mortevielle/detection.h"

static const PlainGameDescriptor MortevielleGame[] = {
	{"mortevielle", "Mortville Manor"},
	{0, 0}
};

#include "mortevielle/detection_tables.h"

class MortevielleMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	MortevielleMetaEngineDetection() : AdvancedMetaEngineDetection(Mortevielle::MortevielleGameDescriptions, sizeof(Mortevielle::MortevielleGameDescription),
		MortevielleGame) {
		_md5Bytes = 512;
		// Use kADFlagUseExtraAsHint to distinguish between original and improved versions
		// (i.e. use or not of the game data file).
		_flags = kADFlagUseExtraAsHint;
	}

	const char *getEngineId() const override {
		return "mortevielle";
	}

	const char *getName() const override {
		return "Mortville Manor";
	}

	const char *getOriginalCopyright() const override {
		return "Mortville Manor (C) 1987-89 Lankhor";
	}
};


REGISTER_PLUGIN_STATIC(MORTEVIELLE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MortevielleMetaEngineDetection);
