#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {

void SpiderEngine::drawPlayer(Common::String player, MVideo &background) {
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	uint32 idx;
	if (mousePos.y / (_screenH / 2) == 0)
		idx = mousePos.x / (_screenW / 3);
	else
		idx = 5 - mousePos.x / (_screenW / 3);
	Graphics::Surface *image = decodeFrame(player, idx);
	drawImage(*image, 60, 129, true);
	image->free();
	delete image;
}

void SpiderEngine::drawHealth(const Graphics::Font &font) {
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
}

} // End of namespace Hypno