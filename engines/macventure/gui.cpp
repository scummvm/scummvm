#include "common/file.h"
#include "image/bmp.h"

#include "macventure/macventure.h"
#include "macventure/gui.h"

namespace MacVenture {

/* priority, name, action, shortcut, enabled*/
#define MV_MENU5(p, n, a, s, e) Graphics::MenuData{p, n, a, s, e}
#define MV_MENU4(p, n, a, s) Graphics::MenuData{p, n, a, s, false}
#define MV_MENUtop(n, a, s) Graphics::MenuData{-1, n, a, s, true}

static const Graphics::MenuData menuSubItems[] = {
	{ -1, "Hello World",	0, 0, false },
	{ 0, "How yo duin",	0, 0, false },
};

Gui::Gui(MacVentureEngine *engine) {
	_engine = engine;
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

	_menu = _wm.addMenu();

	loadMenus();

	_menu->calcDimensions();

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

void Gui::loadMenus() {
	Graphics::MenuData data;
	Common::Array<Graphics::MenuData>::const_iterator iter;
	_menu->addStaticMenus(_engine->getMenuData());	
}

} // End of namespace MacVenture
