
#include "macventure/macventure.h"

#include "common/file.h"
#include "image/bmp.h"

namespace MacVenture {

Gui::Gui() {
	initGUI();
}

Gui::~Gui() {

}

void Gui::draw() {
	_wm.draw();
}

void Gui::initGUI() {
	_screen.create(kScreenWidth, kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	_wm.setScreen(&_screen);
	Graphics::MacWindow *w = _wm.addWindow(false, true, true);
	w->setDimensions(Common::Rect(100, 100));
	w->setActive(false);

	loadBorder(w, "border_inac.bmp", false);
}

void Gui::loadBorder(Graphics::MacWindow * target, Common::String filename, bool active) {
	Common::File borderfile;

	if (!borderfile.open(filename)) {
		debug(1, "Cannot open border file");
		return;
	}

	Image::BitmapDecoder bmpDecoder;
	Common::SeekableReadStream *stream = borderfile.readStream(borderfile.size());
	Graphics::Surface source;
	Graphics::TransparentSurface *surface = new Graphics::TransparentSurface();

	if (stream) {
		debug(4, "Loading %s border from %s", (active ? "active" : "inactive"), filename);
		bmpDecoder.loadStream(*stream);
		source = *(bmpDecoder.getSurface());

		source.convertToInPlace(surface->getSupportedPixelFormat(), bmpDecoder.getPalette());
		surface->create(source.w, source.h, source.format);
		surface->copyFrom(source);
		surface->applyColorKey(255, 0, 255, false);

		target->setBorder(*surface, active);

		borderfile.close();

		delete stream;
	}
}

} // End of namespace MacVenture
