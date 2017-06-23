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

#include "bladerunner/bladerunner.h"
#include "bladerunner/detection_tables.h"

namespace BladeRunner {

static const PlainGameDescriptor bladeRunnerGames[] = {
	{"bladerunner", "Blade Runner"},
	{0, 0}
};

} // End of namespace BladeRunner

class BladeRunnerMetaEngine : public AdvancedMetaEngine {
public:
	BladeRunnerMetaEngine() : AdvancedMetaEngine(BladeRunner::gameDescriptions, sizeof(BladeRunner::gameDescriptions[0]), BladeRunner::bladeRunnerGames) {
	}

	virtual const char *getName() const {
		return "Blade Runner Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Blade Runner (C) Westwood Studios.";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool BladeRunnerMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const
{
	*engine = new BladeRunner::BladeRunnerEngine(syst);

	return true;
}

#if PLUGIN_ENABLED_DYNAMIC(BLADERUNNER)
	REGISTER_PLUGIN_DYNAMIC(BLADERUNNER, PLUGIN_TYPE_ENGINE, BladeRunnerMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BLADERUNNER, PLUGIN_TYPE_ENGINE, BladeRunnerMetaEngine);
#endif
