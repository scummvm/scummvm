#include <algorithm>

#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {


static const int frame_idx[9] = {0, 1, 2, 7, 8, 3, 6, 5, 4};

void SpiderEngine::drawShoot(Common::Point target) {
	uint32 c = _pixelFormat.RGBToColor(255, 255, 255);
	_compositeSurface->drawLine(80, 155, target.x, target.y + 1, c);
	_compositeSurface->drawLine(80, 155, target.x, target.y, c);
	_compositeSurface->drawLine(80, 155, target.x, target.y - 1, c);
	playSound(_soundPath + _shootSound, 1);
}

void SpiderEngine::drawPlayer(Common::String player, MVideo &background) {
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	uint32 idx = frame_idx[std::min(2, mousePos.x / (_screenW / 3)) + 3 * std::min(2, mousePos.y / (_screenH / 3))];
	Graphics::Surface *image = decodeFrame(player, idx);
	drawImage(*image, 60, 129, true);
	image->free();
	delete image;
}

void SpiderEngine::drawHealth() {
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

	_font->drawString(_compositeSurface, "ENERGY", 248, 180, 38, c);
}

} // End of namespace Hypno