#include "hypno/hypno.h"
#include "hypno/grammar.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {

void WetEngine::drawPlayer(Common::String player, uint32 idx) {
	_transparentColor = _pixelFormat.RGBToColor(4, 4, 0);
	Graphics::Surface *image = decodeFrame(player, idx);
	drawImage(*image, 0, 130, true);
	_transparentColor = _pixelFormat.RGBToColor(0, 0, 0);
	image->free();
	delete image;
}

void WetEngine::drawHealth(const Graphics::Font &font) {
	/*
	Common::Rect r;
	uint32 c;
	int d = (22 * (_maxHealth - _health) / _maxHealth);

	r = Common::Rect(256, 152 + d, 272, 174);
	if (d >= 11)
		c = _pixelFormat.RGBToColor(255, 0, 0);
	else 
		c = _pixelFormat.RGBToColor(32, 208, 32);

	_compositeSurface->fillRect(r, c);

	r = Common::Rect(256, 152, 272, 174);
	c = _pixelFormat.RGBToColor(0, 0, 255);
	_compositeSurface->frameRect(r, c);

	font.drawString(_compositeSurface, "ENERGY", 248, 180, 38, c);
	*/
}

} // End of namespace Hypno