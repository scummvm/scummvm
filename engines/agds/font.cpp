#include "agds/font.h"
#include "graphics/transparent_surface.h"

namespace AGDS {

Font::Font(Graphics::TransparentSurface *surface, int gw, int gh): _surface(surface), _gw(gw), _gh(gh) {
}

void Font::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (chr >= 0x100)
		return;

	Common::Rect srcRect(_gw, _gh);
	srcRect.moveTo(_surface->w / 16 * (chr & 0x0f), _surface->h / 16 * (chr >> 4));
	_surface->blit(*dst, x, y, Graphics::FLIP_NONE, &srcRect);
}

}
