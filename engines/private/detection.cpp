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

namespace Private {
static const PlainGameDescriptor privateGames[] = {
    { "private-eye", "Private Eye" },
    { 0, 0 }
};


static const ADGameDescription gameDescriptions[] = {
    {
        "private-eye",  // US release
        0,
        AD_ENTRY1s("PVTEYE.Z", "b682118cda6a42fa89833cae2b8824bd", 271895),
        Common::EN_USA,
        Common::kPlatformWindows,
        ADGF_NO_FLAGS,
        GUIO1(GUIO_NOMIDI)
    },

    {
        "private-eye",  // EU release
        0,
        AD_ENTRY1s("PVTEYE.Z", "", 0),
        Common::EN_GRB,
        Common::kPlatformWindows,
        ADGF_UNSUPPORTED,
        GUIO1(GUIO_NOMIDI)
    },

    {
        "private-eye", // Demo from archive.org
        "Demo",
        AD_ENTRY1s("PVTEYE.Z", "8ef908e212bb9c1e10f5e3c81f56682c", 263893),
        Common::EN_USA,
        Common::kPlatformWindows,
        ADGF_DEMO,
        GUIO1(GUIO_NOMIDI)
    },

    {
        "private-eye", // Demo from the US release
        "Demo",
        AD_ENTRY1s("PVTEYE.Z", "af383c813157810e89d8d6d595895ff7", 271214),
        Common::EN_USA,
        Common::kPlatformWindows,
        ADGF_DEMO,
        GUIO1(GUIO_NOMIDI)
    },


    {
        "private-eye", // Demo from the EU release
        "Demo",
        AD_ENTRY1s("PVTEYE.Z", "", 0),
        Common::EN_GRB,
        Common::kPlatformWindows,
        ADGF_DEMO | ADGF_UNSUPPORTED,
        GUIO1(GUIO_NOMIDI)
    },

    AD_TABLE_END_MARKER
};
} // End of namespace Private

static const char *const directoryGlobs[] = {
    "SUPPORT",
    0
};

class PrivateMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
    PrivateMetaEngineDetection() : AdvancedMetaEngineDetection(Private::gameDescriptions, sizeof(ADGameDescription), Private::privateGames) {
        _maxScanDepth = 2;
        _directoryGlobs = directoryGlobs;
    }

    const char *getEngineId() const override {
        return "private";
    }

    const char *getName() const override {
        return "Private Eye";
    }

    const char *getOriginalCopyright() const override {
        return "Copyright (C) Brooklyn Multimedia";
    }
};

REGISTER_PLUGIN_STATIC(PRIVATE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PrivateMetaEngineDetection);
