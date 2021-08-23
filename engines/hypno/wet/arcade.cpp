#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {

void WetEngine::drawPlayer(Common::String player, MVideo &background) {
	_transparentColor = _pixelFormat.RGBToColor(0, 0, 0);
	Graphics::Surface *image = decodeFrame(player, background.decoder->getCurFrame() % 3);
	drawImage(*image, 0, 200 - image->h + 1, true);
	_transparentColor = _pixelFormat.RGBToColor(0, 0, 0);
	image->free();
	delete image;
}

void WetEngine::drawHealth(const Graphics::Font &font) {
	//Common::Rect r;
	uint32 c;
	int p = (100 * _health) / _maxHealth;
	c = _pixelFormat.RGBToColor(252, 252, 0);
	font.drawString(_compositeSurface, Common::String::format("ENERGY   %d%%", p), 70, 160, 65, c);
	font.drawString(_compositeSurface, Common::String::format("SCORE    %04d", 0), 180, 160, 71, c);
}

} // End of namespace Hypno