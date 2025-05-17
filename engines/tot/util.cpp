
#include "common/textconsole.h"
#include "graphics/paletteman.h"

#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

void _exit(int code) {
	warning("STUB: exit!");
}

void delay(uint16 ms) {
	g_engine->_chrono->delay(ms);
}

void sound(uint16 frequency, int ms) {
	g_engine->_sound->beep(frequency, ms);
}

/*
Allows you to modify palette entries for the IBM
8514 and the VGA drivers.
*/
void setRGBPalette(int color, int r, int g, int b) {
	// Take current palette
	byte palbuf[768];
	g_system->getPaletteManager()->grabPalette(palbuf, 0, 256);

	palbuf[color * 3 + 0] = r << 2;
	palbuf[color * 3 + 1] = g << 2;
	palbuf[color * 3 + 2] = b << 2;

	g_system->getPaletteManager()->setPalette(palbuf, 0, 256);
}

void outtextxy(int x, int y, Common::String text, byte color, bool euro, Graphics::TextAlign align) {
	outtextxy(x, y, text.c_str(), color, euro, align);
}

void outtextxy(int x, int y, char const *text, byte color, bool euro, Graphics::TextAlign align) {
	bool yAligned = (align == Graphics::kTextAlignCenter) ? true : false;
	x = (align == Graphics::kTextAlignCenter) ? 0 : x;
	// TODO: Investigate why this is needed
	y = y + 2;
	if (euro) {
		g_engine->_graphics->euroText(text, x, y, color, align, yAligned);
	} else {
		g_engine->_graphics->littText(text, x, y, color, align, yAligned);
	}
}


void outtextxyBios(int x, int y, Common::String text, byte color) {
	g_engine->_graphics->biosText(text, x, y, color);
}

void cleardevice(void) {
	g_engine->_screen->clear();
	g_engine->_screen->update();
}

unsigned int imagesize(int x, int y, int x2, int y2) {
	int w = x2 - x + 1;
	int h = y2 - y + 1;
	return 4 + (w * h);
}

void bar(int x1, int y1, int x2, int y2, byte color) {
	x2 = x2 + 1;
	y2 = y2 + 1;
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++) {
			*((byte *)g_engine->_screen->getBasePtr(i, j)) = color;
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(x1, y1, x2, y2));
}

void putpixel(int x, int y, byte color) {
	g_engine->_screen->setPixel(x, y, color);
}

void line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, byte color) {
	g_engine->_screen->drawLine(x1, y1, x2, y2, color);
}

void changeGameSpeed(Common::Event e) {
	if (e.type == Common::EVENT_KEYUP) {
		if (e.kbd.hasFlags(Common::KBD_CTRL)) {
			if (e.kbd.keycode == Common::KEYCODE_f) {
				if (g_engine->_chrono->_speedMultiplier == 1)
					g_engine->_chrono->_speedMultiplier = 3;
				else
					g_engine->_chrono->_speedMultiplier = 1;
			}
		}
	}
}

void emptyLoop() {
	do {
		g_engine->_chrono->updateChrono();
		g_engine->_screen->update();
	} while (!tocapintar && !g_engine->shouldQuit());
}

void emptyLoop2() {
	do {
		g_engine->_chrono->updateChrono();
		g_engine->_screen->update();
	} while (!tocapintar2);
}

void waitForKey() {
	bool teclapulsada = false;
	Common::Event e;
	debug("Waiting for key!");
	while (!teclapulsada && !g_engine->shouldQuit())
	{
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN) {
				teclapulsada = true;
			}
		}

		g_engine->_screen->update();
		g_system->delayMillis(10);
	}

}

unsigned int Random(unsigned int range) { return g_engine->getRandomNumber(range); }
int Random(int range) { return g_engine->getRandomNumber(range - 1); }

} // End of namespace Tot
