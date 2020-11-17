/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/icb/icb.h"

#include "common/savefile.h"
#include "common/system.h"

namespace ICB {

class IcbMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "icb";
	}
	virtual bool hasFeature(MetaEngineFeature f) const override { return false; }

	virtual Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

Common::Error IcbMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new IcbEngine(syst, desc);
	return Common::kNoError;
}

} // End of namespace ICB

#if PLUGIN_ENABLED_DYNAMIC(ICB)
	REGISTER_PLUGIN_DYNAMIC(ICB, PLUGIN_TYPE_ENGINE, ICB::IcbMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ICB, PLUGIN_TYPE_ENGINE, ICB::IcbMetaEngine);
#endif
