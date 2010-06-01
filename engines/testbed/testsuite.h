#ifndef TESTSUITE_H
#define TESTSUITE_H

#include "common/system.h"

namespace Testbed {

typedef bool (*invokingFunction)();

/**
 * This represents the a feature to be tested
 *
 * @Note
 * featureName stores the name of this feature test, for display purposes
 * driver is pointer to the function that will invoke this feature test
 * enabled	decides whether or not this test is to be executed
 * passed	collects and stores result of this feature test.
 *
 */
struct Test {
	Test(Common::String name, invokingFunction f) : featureName(name), driver(f) {}
	Common::String featureName;
	invokingFunction driver;
	bool enabled;
	bool passed;
};

/**
 * The basic Testsuite class
 * All the other testsuites would inherit it and override its virtual methods
 */

class Testsuite {
public:
	Testsuite() {
		extern OSystem *g_system;
		_backend = g_system;
	}
	~Testsuite() {}
	inline int getNumTestsPassed() { return _numTestsPassed; };
	inline int getNumTestsFailed() { return _numTestsExecuted - _numTestsPassed; };
	inline void addTest(Common::String name, invokingFunction f) {
		Test featureTest(name, f);
		_testsToExecute.push_back(featureTest);
	}
	virtual int execute() = 0;
	virtual const char *getName() = 0;

private:
	OSystem		*_backend;
	int		    _numTestsPassed;
	int  		_numTestsExecuted;
	Common::Array<Test> _testsToExecute; 
}

}	// End of namespace testbed

#endif
