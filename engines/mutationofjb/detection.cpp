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

#include "mutationofjb/mutationofjb.h"

#include "common/config-manager.h"

#include "engines/advancedDetector.h"

static const PlainGameDescriptor mutationofjbGames[] = {
	{"mutationofjb", "Mutation of J.B."},
	{nullptr, nullptr}
};

static const ADGameDescription mutationofjbDescriptions[] = {
	{
		"mutationofjb",
		"",
		{
			{"jb.ex_", 0, "934164b09c72fa7167811f448ee0a426", 150048},
			{"startup.dat", 0, nullptr, -1},
			{"startupb.dat", 0, nullptr, -1},
			{"global.atn", 0, nullptr, -1},
			{"piggy.apk", 0, nullptr, -1},
			{"foogl.apk", 0, nullptr, -1},
			{"tosay.ger", 0, nullptr, -1},
			{"response.ger", 0, nullptr, -1},
			{"font1.aft", 0, nullptr, -1},
			{"sysfnt.aft", 0, nullptr, -1},
			{nullptr, 0, nullptr, 0}
		},
		Common::SK_SVK,
		Common::kPlatformDOS,
		ADGF_CD,
		GUIO0()
	},
	{
		"mutationofjb",
		"",
		{
			{"jb.ex_", 0, "8833f22f1763d05eeb909e8626cdec7b", 150800},
			{"startup.dat", 0, nullptr, -1},
			{"startupb.dat", 0, nullptr, -1},
			{"global.atn", 0, nullptr, -1},
			{"piggy.apk", 0, nullptr, -1},
			{"foogl.apk", 0, nullptr, -1},
			{"tosay.ger", 0, nullptr, -1},
			{"response.ger", 0, nullptr, -1},
			{"font1.aft", 0, nullptr, -1},
			{"sysfnt.aft", 0, nullptr, -1},
			{nullptr, 0, nullptr, 0}
		},
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_CD,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

static const char *const mutationofjbDirectoryGlobs[] = {
	"data",
	nullptr
};

class MutationOfJBMetaEngine : public AdvancedMetaEngine {
public:
	MutationOfJBMetaEngine() : AdvancedMetaEngine(mutationofjbDescriptions, sizeof(ADGameDescription), mutationofjbGames) {
		_maxScanDepth = 2;
		_directoryGlobs = mutationofjbDirectoryGlobs;
	}

	virtual const char *getName() const {
		return "Mutation of J.B.";
	}

	virtual const char *getOriginalCopyright() const {
		return "Mutation of J.B. (C) 1996 RIKI Computer Games";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		if (desc) {
			*engine = new MutationOfJB::MutationOfJBEngine(syst);
		}
		return desc != nullptr;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(MUTATIONOFJB)
	REGISTER_PLUGIN_DYNAMIC(MUTATIONOFJB, PLUGIN_TYPE_ENGINE, MutationOfJBMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MUTATIONOFJB, PLUGIN_TYPE_ENGINE, MutationOfJBMetaEngine);
#endif
