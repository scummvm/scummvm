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

	return true;
}

GFXTestSuite::GFXTestSuite() {
	addTest("FullScreenMode", &testFullScreenMode);
}

GFXTestSuite::~GFXTestSuite() {
	printf("Cleanup\n");
}

const char *GFXTestSuite::getName() {
	return "GFX";
}

int GFXTestSuite::execute() {
	//TODO: Implement the method
	for (Common::Array<Test*>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
		printf("Executing Test:%s\n", ((*i)->featureName).c_str());
		printf("Result:%d",(*i)->driver());
	}

	return 1;
}

}
