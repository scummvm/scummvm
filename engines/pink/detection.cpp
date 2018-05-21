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

#include "gui/EventRecorder.h"

#include "pink/pink.h"

static const PlainGameDescriptor pinkGames[] = {
        {"peril", "The Pink Panther: Passport to Peril"},
        {"pokus", "The Pink Panther: Hokus Pokus Pink"},
        {0, 0}
};

#include "detection_tables.h"

class PinkMetaEngine : public AdvancedMetaEngine {
public:
    PinkMetaEngine() : AdvancedMetaEngine(Pink::gameDescriptions, sizeof(ADGameDescription), pinkGames) {
        _gameIds = pinkGames;
    }

    virtual const char *getName() const {
        return "Pink Panther Engine";
    }

    virtual const char *getOriginalCopyright() const {
        return "Pink Panther Engine (C) Wanderlust Interactive";
    }

    //virtual bool hasFeature(MetaEngineFeature f) const;
    //virtual int getMaximumSaveSlot() const { return 0; }
    //virtual SaveStateList listSaves(const char *target) const;
    //virtual void removeSaveState(const char *target, int slot) const;
    //virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
    virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool PinkMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
    if (desc) {
        *engine = new Pink::PinkEngine(syst, desc);
    }

    return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(PINK)
REGISTER_PLUGIN_DYNAMIC(PINK, PLUGIN_TYPE_ENGINE, PinkMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PINK, PLUGIN_TYPE_ENGINE, PinkMetaEngine);
#endif
