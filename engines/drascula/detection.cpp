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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/file.h"

#include "drascula/drascula.h"


namespace Drascula {

struct DrasculaGameDescription {
	ADGameDescription desc;
};

uint32 DrasculaEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language DrasculaEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

void DrasculaEngine::loadArchives() {
	const ADGameFileDescription *ag;

	if (getFeatures() & GF_PACKED) {
		for (ag = _gameDescription->desc.filesDescriptions; ag->fileName; ag++)
			_arj.registerArchive(ag->fileName);
	}

	_arj.enableFallback(true);
}

}

static const PlainGameDescriptor drasculaGames[] = {
	{"drascula", "Drascula: The Vampire Strikes Back"},
	{0, 0}
};


namespace Drascula {

using Common::GUIO_NONE;

static const DrasculaGameDescription gameDescriptions[] = {

	{
		// Drascula English version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "09b2735953edcd43af115c65ae00b10e", 1595),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
	},

	{
		// Drascula English version (original packed files)
		{
			"drascula",
			0,
			AD_ENTRY1s("packet.001", "c6a8697396e213a18472542d5f547cb4", 32847563),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_KEEPMATCH | GF_PACKED,
			GUIO_NONE
		},
	},

	{
		// Drascula German version (original packed files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.003", 0, "e8f4dc6091037329bab4ddb1cba35807", 719728},
				{NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			GF_PACKED,
			GUIO_NONE
		},
	},

	{
		// Drascula French version (original packed files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.002", 0, "4401123400f22f212b89f15fb4b43013", 721122},
				{NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			GF_PACKED,
			GUIO_NONE
		},
	},

	{
		// Drascula Spanish version (original packed version)
		{
			"drascula",
			0,
			AD_ENTRY1s("packet.001", "3c971aba65a037d29d0b479cad6f5943", 31702652),
			Common::ES_ESP,
			Common::kPlatformPC,
			GF_PACKED,
			GUIO_NONE
		},
	},

	{
		// Drascula Spanish version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "0746ed1a5cc8d9728f790c29813f4b43", 23059),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
	},

	{
		// Drascula German version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "72e46089033d56bad1c179ac36e2a9d2", 610),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
	},

	{
		// Drascula French version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "eeeee96b82169003630e08992248296c", 608),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
	},

	{
		// Drascula Italian version (original packed version)
		{
			"drascula",
			0,
			AD_ENTRY1s("packet.001", "0253e924af223f5fe52537023385159b", 32564209),
			Common::IT_ITA,
			Common::kPlatformPC,
			GF_PACKED,
			GUIO_NONE
		},
	},
	{
		// Drascula Italian version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "02b49a18328d0bf2efe6ba658c9c7a1d", 2098),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
	},

	{
		// Drascula Spanish version (ScummVM repacked files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.004", 0, "a289d3cf80d50f25ec569b653248437e", 17205838},
				{NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			GF_PACKED,
			GUIO_NONE
		},
	},

	{
		// Drascula Italian version (ScummVM repacked files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.005", 0, "58caac54b891f5d7f335e710e45e5d29", 16209623},
				{NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			GF_PACKED,
			GUIO_NONE
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Drascula

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Drascula::gameDescriptions,
	// Size of that superset structure
	sizeof(Drascula::DrasculaGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	drasculaGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"drascula",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NOMIDI
};

class DrasculaMetaEngine : public AdvancedMetaEngine {
public:
	DrasculaMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Drascula Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Drascula Engine (C) 2000 Alcachofa Soft, 1996 (C) Digital Dreams Multimedia, 1994 (C) Emilio de Paz";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool DrasculaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Drascula::DrasculaGameDescription *gd = (const Drascula::DrasculaGameDescription *)desc;
	if (gd) {
		*engine = new Drascula::DrasculaEngine(syst, gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(DRASCULA)
	REGISTER_PLUGIN_DYNAMIC(DRASCULA, PLUGIN_TYPE_ENGINE, DrasculaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DRASCULA, PLUGIN_TYPE_ENGINE, DrasculaMetaEngine);
#endif
