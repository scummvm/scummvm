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

// Game detection, general game parameters

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "saga/detection.h"

static const PlainGameDescriptor sagaGames[] = {
	{"ite", "Inherit the Earth: Quest for the Orb"},
	{"ihnm", "I Have No Mouth and I Must Scream"},
	{"dino", "Dinotopia"},
	{"fta2", "Faery Tale Adventure II: Halls of the Dead"},
	{0, 0}
};

#include "saga/detection_tables.h"

class SagaMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	SagaMetaEngineDetection() : AdvancedMetaEngineDetection(Saga::gameDescriptions, sizeof(Saga::SAGAGameDescription), sagaGames) {
	}

	const char *getEngineId() const override {
		return "saga";
	}

	const char *getName() const override {
		return "SAGA ["

#if defined(ENABLE_IHNM) && defined(ENABLE_SAGA2)
			"all games"
#else
			"ITE"

#if defined(ENABLE_IHNM)
			", IHNM"
#endif

#if defined(ENABLE_SAGA2)
			", SAGA2 games"
#endif

#endif
		"]";

;
	}

	const char *getOriginalCopyright() const override {
		return "Inherit the Earth (C) Wyrmkeep Entertainment";
	}
};

REGISTER_PLUGIN_STATIC(SAGA_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, SagaMetaEngineDetection);
