#include "private/private.h"
#include "engines/advancedDetector.h"

class PrivateMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "private";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

Common::Error PrivateMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Private::PrivateEngine(syst);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(PRIVATE)
REGISTER_PLUGIN_DYNAMIC(PRIVATE, PLUGIN_TYPE_ENGINE, PrivateMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PRIVATE, PLUGIN_TYPE_ENGINE, PrivateMetaEngine);
#endif
