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

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/fs.h"

#include "base/plugins.h"

#include "testbed/testbed.h"

static const PlainGameDescriptor testbed_setting[] = {
	{ "testbed", "Testbed: The backend testing framework" },
	{ 0, 0 }
};

static const ADGameDescription testbedDescriptions[] = {
	{
		"testbed",
		"",
		AD_ENTRY1(NULL, 0),	// No data files required
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},
	AD_TABLE_END_MARKER
};

static const ADParams detectionParams = {
	(const byte *)testbedDescriptions,
	sizeof(ADGameDescription),
	512,
	testbed_setting,
	0,
	"testbed",
	0,
	ADGF_NO_FLAGS,
	Common::GUIO_NONE
};

class TestbedMetaEngine : public AdvancedMetaEngine {
public:
	TestbedMetaEngine() : AdvancedMetaEngine(detectionParams) {
	}

	virtual const char *getName() const {
		return "TestBed: The backend testing framework";
	}

	virtual const char *getOriginalCopyright() const {
		return "Copyright (C) ScummVM";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		// Instantiate Engine even if the game data is not found.
		*engine = new Testbed::TestbedEngine(syst);
		return true;
	}

};

#if PLUGIN_ENABLED_DYNAMIC(TESTBED)
	REGISTER_PLUGIN_DYNAMIC(TESTBED, PLUGIN_TYPE_ENGINE, TestbedMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TESTBED, PLUGIN_TYPE_ENGINE, TestbedMetaEngine);
#endif
