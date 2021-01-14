#include "base/plugins.h"
#include "engines/advancedDetector.h"

namespace Private {
static const PlainGameDescriptor privateGames[] = {
    { "private-eye", "Private Eye" },
    { 0, 0 }
};


static const ADGameDescription gameDescriptions[] = {
    {
        "private-eye",  // Full game
        0,
        AD_ENTRY1s("ASSETS.Z", "3a7532349cda8126e96dd5e49884af3a", 40232),
        Common::EN_ANY,
        Common::kPlatformWindows,
        ADGF_NO_FLAGS,
        GUIO1(GUIO_NOMIDI)
    },
    {
        "private-eye", // Demo from archive.org
        "Demo",
        AD_ENTRY1s("ASSETS.Z", "854e141bb67535359620a1833fcc1566", 5955),
        Common::EN_ANY, 
        Common::kPlatformWindows,
        ADGF_DEMO,
        GUIO1(GUIO_NOMIDI)
    },

    {
        "private-eye", // Demo from the full game CDROM
        "Demo",
        AD_ENTRY1s("ASSETS.Z", "045766e39f44d6ee3bf92f0d4521587c", 5961),
        Common::EN_ANY, 
        Common::kPlatformWindows,
        ADGF_DEMO,
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
