#include "engines/advancedDetector.h"
#include "freescape/freescape.h"

class FreescapeMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "freescape";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

Common::Error FreescapeMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Freescape::FreescapeEngine(syst);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(FREESCAPE)
REGISTER_PLUGIN_DYNAMIC(FREESCAPE, PLUGIN_TYPE_ENGINE, FreescapeMetaEngine);
#else
REGISTER_PLUGIN_STATIC(FREESCAPE, PLUGIN_TYPE_ENGINE, FreescapeMetaEngine);
#endif
