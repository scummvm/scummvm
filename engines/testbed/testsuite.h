#ifndef TESTSUITE_H
#define TESTSUITE_H

#include "common/system.h"
#include "common/str.h"
#include "common/array.h"

namespace Testbed {

typedef bool (*invokingFunction)();

/**
 * Make g_system available to test invoker functions
 */
extern OSystem *g_system;

/**
 * This represents a feature to be tested
 */

struct Test {
	Test(Common::String name, invokingFunction f) : featureName(name),
													driver(f),
													enabled(true),
													passed(false) {}

	Common::String featureName;		///< Name of feature to be tested
	invokingFunction driver;	    ///< Pointer to the function that will invoke this feature test
	bool enabled;				    ///< Decides whether or not this test is to be executed
	bool passed;					///< Collects and stores result of this feature test
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
		_numTestsPassed = 0;
		_numTestsExecuted = 0;
	}
	~Testsuite() {}
	inline int getNumTests() { return _testsToExecute.size(); }
	inline int getNumTestsPassed() { return _numTestsPassed; }
	inline int getNumTestsFailed() { return _numTestsExecuted - _numTestsPassed; }

	/**
	 * Adds a test to the list of tests to be executed
	 *
	 * @param	name the string description of the test, for display purposes
	 * @param	f pointer to the function that invokes this test
	 */
	inline void addTest(Common::String name, invokingFunction f) {
		Test featureTest(name, f);
		_testsToExecute.push_back(featureTest);
	}
	
	/**
	 * The driver function for the testsuite
	 * All code should go in here.
	 */
	virtual int execute() = 0;
	virtual const char *getName() = 0;

private:
	OSystem		*_backend;					///< Pointer to OSystem backend
	int		    _numTestsPassed;			///< Number of tests passed
	int  		_numTestsExecuted;			///< Number of tests executed
	Common::Array<Test> _testsToExecute;	///< List of tests to be executed
}

}	// End of namespace testbed

#endif
