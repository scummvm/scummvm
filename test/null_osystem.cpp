#define USE_NULL_DRIVER 1
#define NULL_DRIVER_USE_FOR_TEST 1
#include "null_osystem.h"
#include "../backends/platform/null/null.cpp"

void Common::install_null_g_system() {
	g_system = OSystem_NULL_create();
}

bool BaseBackend::setScaler(const char *name, int factor) {
	return false;
}

void BaseBackend::displayMessageOnOSD(const Common::U32String &msg) {
}

void BaseBackend::initBackend() {
	OSystem::initBackend();
}

bool BaseBackend::hasFeature(OSystem::Feature f) {
	return false;
}

void BaseBackend::fillScreen(uint32 col) {
}

void BaseBackend::fillScreen(const Common::Rect &r, uint32 col) {
}

void EventsBaseBackend::initBackend() {
	BaseBackend::initBackend();
}
