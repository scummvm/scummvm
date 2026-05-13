#define CXXTEST_HAVE_EH
#define CXXTEST_ABORT_TEST_ON_FAIL
#include <cxxtest/ErrorPrinter.h>

int main()
{
    std::cout << "Starting test runner" << std::endl;
    int status = CxxTest::ErrorPrinter().run();
    std::cout << "Stopping test runner" << std::endl;
    return status;
}

// The CxxTest "world"
<CxxTest world>
