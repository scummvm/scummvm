/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 */

#include "engines/advancedDetector.h"

#include "engines/myst3/myst3.h"

namespace Myst3 {

struct Myst3GameDescription {
	ADGameDescription desc;
	int flags;
};

static const PlainGameDescriptor myst3Games[] = {
	{"myst3", "Myst III Exile"},
	{0, 0}
};

static const char *directoryGlobs[] = {
	"M3Data",
	"MYST3BIN",
	0
};

static const Myst3GameDescription gameDescriptions[] = {
	{
		// Myst 3 All PC / Mac Versions
		{
			"myst3",
			0,
			AD_ENTRY1s("RSRC.m3r", "a2c8ed69800f60bf5667e5c76a88e481", 1223862),
			Common::UNK_LANG,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		0,
	},

	{
		// Myst 3 PS2 (NTSC U/C)
		{
			"myst3",
			0,
			AD_ENTRY1s("RSRC.m3r", "c60d37bfd3bb8b0bee143018447bb460", 346618151),
			Common::UNK_LANG,
			Common::kPlatformPS2,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		0
	},

	{ AD_TABLE_END_MARKER, 0 }
};

class Myst3MetaEngine : public AdvancedMetaEngine {
public:
	Myst3MetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(Myst3GameDescription), myst3Games) {
		_singleid = "myst3";
		_guioptions = GUIO_NOMIDI;
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const {
		return "Myst III Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Myst III Game (C) Presto Studios";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool Myst3MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Myst3GameDescription *gd = (const Myst3GameDescription *)desc;
	if (gd) {
		*engine = new Myst3Engine(syst, gd->flags);
	}
	return gd != 0;
}

} // End of namespace Myst3

#if PLUGIN_ENABLED_DYNAMIC(MYST3)
	REGISTER_PLUGIN_DYNAMIC(MYST3, PLUGIN_TYPE_ENGINE, Myst3::Myst3MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MYST3, PLUGIN_TYPE_ENGINE, Myst3::Myst3MetaEngine);
#endif
