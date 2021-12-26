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

#include "common/str-array.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "voyeur/detection.h"
#include "voyeur/voyeur.h"

static const PlainGameDescriptor voyeurGames[] = {
	{"voyeur", "Voyeur"},
	{0, 0}
};

static const DebugChannelDef debugFlagList[] = {
	{Voyeur::kDebugScripts, "scripts", "Game scripts"},
	DEBUG_CHANNEL_END
};

#include "voyeur/detection_tables.h"

class VoyeurMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	VoyeurMetaEngineDetection() : AdvancedMetaEngineDetection(Voyeur::gameDescriptions, sizeof(Voyeur::VoyeurGameDescription), voyeurGames) {
		_maxScanDepth = 3;
	}

	const char *getEngineId() const override {
		return "voyeur";
	}

	const char *getName() const override {
		return "Voyeur";
	}

	const char *getOriginalCopyright() const override {
		return "Voyeur (C) Philips P.O.V. Entertainment Group";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(VOYEUR_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, VoyeurMetaEngineDetection);
