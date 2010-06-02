#include "testbed/graphics.h"

namespace Testbed {

bool testFullScreenMode() {

	printf("Testing fullscreen mode\n");
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureFullscreenMode);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);

	printf("Testing Feature Presence.. \n");
	if (isFeaturePresent) {
		//Toggle
		printf("Supported\n");

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);
		
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, isFeatureEnabled);
		g_system->endGFXTransaction();
	}

}

GFXTestSuite::GFXTestSuite() {
	addTest("FullScreenMode", &testFullScreenMode);
}

int execute() {
	//TODO: Implement the method	
}

}
