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

#include "petka/petka.h"

static const PlainGameDescriptor petkaGames[] = {
	{"petka_demo", "Red Comrades Demo"},
	{"petka1", "Red Comrades 1: Save the Galaxy"},
	{"petka2", "Red Comrades 2: For the Great Justice"},
	{0, 0}
};

#include "detection_tables.h"

class PetkaMetaEngine : public AdvancedMetaEngine {
public:
	PetkaMetaEngine() : AdvancedMetaEngine(Petka::gameDescriptions, sizeof(ADGameDescription), petkaGames) {
		_gameIds = petkaGames;
		_maxScanDepth = 2;
	}

	virtual const char *getName() const {
		return "Red Comrades";
	}

	virtual const char *getOriginalCopyright() const {
		return "Red Comrades (C) S.K.I.F";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool PetkaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc)
		*engine = new Petka::PetkaEngine(syst, desc);

	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(PETKA)
	REGISTER_PLUGIN_DYNAMIC(PETKA, PLUGIN_TYPE_ENGINE, PetkaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PETKA, PLUGIN_TYPE_ENGINE, PetkaMetaEngine);
#endif
