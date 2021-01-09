#include "base/plugins.h"
#include "engines/advancedDetector.h"

namespace Private {
static const PlainGameDescriptor privateGames[] = {
    { "private-eye", "Private Eye" },
    { 0, 0 }
};


static const ADGameDescription gameDescriptions[] = {
    {
        "private-eye",
        0,
        AD_ENTRY1s("GAME.DAT", "7a3eb7d9dedf40680ac3d088524f976e", 190735),
        Common::EN_ANY,
        Common::kPlatformWindows,
        ADGF_NO_FLAGS,
        GUIO1(GUIO_NOMIDI)
    },
    {
        "private-eye",
        0,
        AD_ENTRY1s("GAME.TXT", "28ee7295db2cf6d9358ad737e3a4c4c0", 19286),
        Common::EN_ANY,
        Common::kPlatformWindows,
        ADGF_DEMO,
        GUIO1(GUIO_NOMIDI)
    },

    AD_TABLE_END_MARKER
};
} // End of namespace Private

static const char *const directoryGlobs[] = {
    "assets", // english CD
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
