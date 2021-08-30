#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {

bool WetEngine::clickedSecondaryShoot(Common::Point mousePos) {
	return clickedPrimaryShoot(mousePos);
}

void WetEngine::drawShoot(Common::Point mousePos) {
	uint32 c = _pixelFormat.RGBToColor(252, 252, 0);
	_compositeSurface->drawLine(0, _screenH, mousePos.x, mousePos.y, c);
	_compositeSurface->drawLine(0, _screenH, mousePos.x-1, mousePos.y, c);
	_compositeSurface->drawLine(0, _screenH, mousePos.x-2, mousePos.y, c);

	_compositeSurface->drawLine(_screenW, _screenH, mousePos.x, mousePos.y, c);
	_compositeSurface->drawLine(_screenW, _screenH, mousePos.x-1, mousePos.y, c);
	_compositeSurface->drawLine(_screenW, _screenH, mousePos.x-2, mousePos.y, c);
	playSound(_soundPath + _shootSound, 1);
}

void WetEngine::drawPlayer(Common::String player, MVideo &background) {
	// TARGET ACQUIRED frame
	uint32 c = _pixelFormat.RGBToColor(32, 208, 32);
	_compositeSurface->drawLine(113, 1, 119, 1, c);
	_compositeSurface->drawLine(200, 1, 206, 1, c);

	_compositeSurface->drawLine(113, 1, 113, 9, c);
	_compositeSurface->drawLine(206, 1, 206, 9, c);

	_compositeSurface->drawLine(113, 9, 119, 9, c);
	_compositeSurface->drawLine(200, 9, 206, 9, c);

	c = _pixelFormat.RGBToColor(255, 0, 0);
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	int i = detectTarget(mousePos);
	if (i > 0)
		_font->drawString(_compositeSurface, "TARGET  ACQUIRED", 120, 1, 80, c);

	//_transparentColor = _pixelFormat.RGBToColor(0, 0, 0);
	Graphics::Surface *image = decodeFrame(player, background.decoder->getCurFrame() % 3);
	drawImage(*image, 0, 200 - image->h + 1, true);
	_transparentColor = _pixelFormat.RGBToColor(0, 0, 0);
	image->free();
	delete image;
}

void WetEngine::drawHealth() {
	uint32 c = _pixelFormat.RGBToColor(252, 252, 0);
	int p = (100 * _health) / _maxHealth;
	_font->drawString(_compositeSurface, Common::String::format("ENERGY   %d%%", p), 70, 160, 65, c);
	_font->drawString(_compositeSurface, Common::String::format("SCORE    %04d", 0), 180, 160, 71, c);
}

} // End of namespace Hypno