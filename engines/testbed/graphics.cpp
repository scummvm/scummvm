#include "testbed/graphics.h"
#include "testbed/gfxtests.h"

namespace Testbed {

GFXTestSuite::GFXTestSuite() {
	//addTest("FullScreenMode", &testFullScreenMode);
	addTest("AspectRatio", &testAspectRatio);
}

GFXTestSuite::~GFXTestSuite() {
	for (Common::Array<Test*>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
		delete (*i);
	}
}

const char *GFXTestSuite::getName() {
	return "GFX";
}

int GFXTestSuite::execute() {
	//TODO: Implement the method
	for (Common::Array<Test*>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
		printf("Executing Test:%s\n", ((*i)->featureName).c_str());
		printf("Result:%d\n",(*i)->driver());
	}

	return 1;
}


}
