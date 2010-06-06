#include "testbed/gfxtests.h"
#include "testbed/testsuite.h"

#include "graphics/fontman.h"
#include "graphics/surface.h"

namespace Testbed {

bool testFullScreenMode() {

	Testsuite::displayMessage("Testing fullscreen mode. \n \
	If the feature is supported by the backend, you should expect to see a toggle between fullscreen and normal modes");

	Common::Point pt(0,100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing fullscreen mode", pt);
	g_system->delayMillis(1000);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureFullscreenMode);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);

	if (isFeaturePresent) {
		//Toggle

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);
		
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, isFeatureEnabled);
		g_system->endGFXTransaction();
	}
	else {
		Testsuite::displayMessage("feature not supported");
	}

	Testsuite::clearScreen(rect);
	return true;
}

bool testAspectRatio() {
	return true;
}

}
