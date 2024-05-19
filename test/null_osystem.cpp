#define FORBIDDEN_SYMBOL_EXCEPTION_abort

#define USE_NULL_DRIVER 1
#define NULL_DRIVER_USE_FOR_TEST 1
#include "null_osystem.h"
#include "../backends/platform/null/null.cpp"

//#define DISPLAY_ERROR_MESSAGES

void Common::install_null_g_system() {
#ifdef DISPLAY_ERROR_MESSAGES
	const bool silenceLogs = false;
#else
	const bool silenceLogs = true;
#endif

	g_system = OSystem_NULL_create(silenceLogs);
}

void OSystem_NULL::quit() {
	abort();
}

bool BaseBackend::setScaler(const char *name, int factor) {
	return false;
}

void BaseBackend::displayMessageOnOSD(const Common::U32String &msg) {
}

void BaseBackend::initBackend() {
	OSystem::initBackend();
}

void BaseBackend::fillScreen(uint32 col) {
}

void BaseBackend::fillScreen(const Common::Rect &r, uint32 col) {
}

void EventsBaseBackend::initBackend() {
	BaseBackend::initBackend();
}
