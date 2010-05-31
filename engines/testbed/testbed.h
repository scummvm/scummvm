#ifndef TESTBED_H
#define TESTBED_H
 
#include "engines/engine.h"
 
namespace Testbed {
 
class TestbedEngine : public Engine {
public:
	TestbedEngine(OSystem *syst);
	~TestbedEngine();
 
	virtual Common::Error run();
};

} // End of namespace Testbed
 
#endif
