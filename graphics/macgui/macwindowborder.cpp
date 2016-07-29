#include "macwindowborder.h"

namespace Graphics {

MacWindowBorder::MacWindowBorder() {
	_activeBorder = nullptr;
	_inactiveBorder = nullptr;
}


MacWindowBorder::~MacWindowBorder() {
	if (_activeBorder)
		delete _activeBorder;
	if (_inactiveBorder)
		delete _inactiveBorder;
}

bool MacWindowBorder::empty() { return !(_activeBorder || _inactiveBorder); }

void MacWindowBorder::addActiveBorder(TransparentSurface *source) {
	// Assumes NinePatchBitmap invariants hold
	_activeBorder = new NinePatchBitmap(source, false);	
}

void MacWindowBorder::addInactiveBorder(TransparentSurface *source) {
	_inactiveBorder = new NinePatchBitmap(source, false);
}

void MacWindowBorder::blitBorderInto(ManagedSurface &destination, bool active) {

	TransparentSurface srf;
	NinePatchBitmap *src = active ? _activeBorder : _inactiveBorder;

	srf.create(destination.w, destination.h, src->getSource()->format);

	src->blit(srf, 0, 0, srf.w, srf.h);
	destination.transBlitFrom(srf, destination.format.ARGBToColor(0, 255, 255, 255));
}

} // End of namespace Graphics
