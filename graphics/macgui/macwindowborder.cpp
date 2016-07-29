#include "macwindowborder.h"

namespace Graphics {

MacWindowBorder::MacWindowBorder() : _activeInitialized(false), _inactiveInitialized(false) {
	_activeBorder = nullptr;
	_inactiveBorder = nullptr;
	_hasOffsets = false;
}


MacWindowBorder::~MacWindowBorder() {
	if (_activeBorder)
		delete _activeBorder;
	if (_inactiveBorder)
		delete _inactiveBorder;
}

bool MacWindowBorder::hasBorder(bool active) {
	return active ? _activeInitialized : _inactiveInitialized;
}

void MacWindowBorder::addActiveBorder(TransparentSurface &source) {
	assert(!_activeBorder);
	_activeBorder = new NinePatchBitmap(&source, false);
	_activeInitialized = true;
}

void MacWindowBorder::addInactiveBorder(TransparentSurface &source) {
	assert(!_inactiveBorder);
	_inactiveBorder = new NinePatchBitmap(&source, false);
	_inactiveInitialized = true;
}

bool MacWindowBorder::hasOffsets() {
	return _hasOffsets;
}

void MacWindowBorder::setBorderOffsets(int left, int right, int top, int bottom) {
	_borderOffsets[0] = left;
	_borderOffsets[1] = right;
	_borderOffsets[2] = top;
	_borderOffsets[3] = bottom;
	_hasOffsets = true;
}

int MacWindowBorder::getBorderOffset(MacBorderOffset offset) {
	return _borderOffsets[offset];
}

void MacWindowBorder::blitBorderInto(ManagedSurface &destination, bool active) {

	TransparentSurface srf;
	NinePatchBitmap *src = active ? _activeBorder : _inactiveBorder;

	srf.create(destination.w, destination.h, src->getSource()->format);

	src->blit(srf, 0, 0, srf.w, srf.h);
	destination.transBlitFrom(srf, destination.format.ARGBToColor(0, 255, 255, 255));
}

} // End of namespace Graphics
