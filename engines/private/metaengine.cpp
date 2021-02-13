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

#include "private/private.h"
#include "engines/advancedDetector.h"

class PrivateMetaEngine : public AdvancedMetaEngine {
public:
    const char *getName() const override {
        return "private";
    }

    Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

Common::Error PrivateMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
    *engine = new Private::PrivateEngine(syst, gd);
    return Common::kNoError;
}

namespace Private {

bool PrivateEngine::isDemo() const {
    return (bool)(_gameDescription->flags & ADGF_DEMO);
}

} // End of namespace Private

#if PLUGIN_ENABLED_DYNAMIC(PRIVATE)
REGISTER_PLUGIN_DYNAMIC(PRIVATE, PLUGIN_TYPE_ENGINE, PrivateMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PRIVATE, PLUGIN_TYPE_ENGINE, PrivateMetaEngine);
#endif
