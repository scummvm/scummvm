
#include "backends/platform/dingux/dingux.h"

#if defined(DINGUX)

bool OSystem_SDL_Dingux::hasFeature(Feature f) {
	return
	    (f == kFeatureAspectRatioCorrection) ||
	    (f == kFeatureCursorHasPalette);
}

void OSystem_SDL_Dingux::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureAspectRatioCorrection:
		setAspectRatioCorrection(enable);
		break;
	default:
		break;
	}
}

bool OSystem_SDL_Dingux::getFeatureState(Feature f) {
	assert(_transactionMode == kTransactionNone);

	switch (f) {
	case kFeatureAspectRatioCorrection:
		return _videoMode.aspectRatioCorrection;
	default:
		return false;
	}
}

#endif

