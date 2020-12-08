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

#include "lastexpress/lastexpress.h"
#include "engines/advancedDetector.h"

namespace LastExpress {

class LastExpressMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "lastexpress";
	}

protected:
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
};

Common::Error LastExpressMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	*engine = new LastExpressEngine(syst, (const ADGameDescription *)gd);
	return Common::kNoError;
}

bool LastExpressEngine::isDemo() const {
	return (bool)(_gameDescription->flags & ADGF_DEMO);
}

} // End of namespace LastExpress

#if PLUGIN_ENABLED_DYNAMIC(LASTEXPRESS)
	REGISTER_PLUGIN_DYNAMIC(LASTEXPRESS, PLUGIN_TYPE_ENGINE, LastExpress::LastExpressMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LASTEXPRESS, PLUGIN_TYPE_ENGINE, LastExpress::LastExpressMetaEngine);
#endif
