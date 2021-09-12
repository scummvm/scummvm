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

#include "chamber/chamber.h"
#include "engines/advancedDetector.h"

class ChamberMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "chamber";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

Common::Error ChamberMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Chamber::ChamberEngine(syst);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(CHAMBER)
REGISTER_PLUGIN_DYNAMIC(CHAMBER, PLUGIN_TYPE_ENGINE, ChamberMetaEngine);
#else
REGISTER_PLUGIN_STATIC(CHAMBER, PLUGIN_TYPE_ENGINE, ChamberMetaEngine);
#endif
