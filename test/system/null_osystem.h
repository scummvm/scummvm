#ifndef TEST_NULL_OSYSTEM
#define TEST_NULL_OSYSTEM 1
namespace Common {
#if defined(POSIX) || defined(WIN32)
void install_null_g_system();
#define NULL_OSYSTEM_IS_AVAILABLE 1
#else
#define NULL_OSYSTEM_IS_AVAILABLE 0
#endif
}
#endif
