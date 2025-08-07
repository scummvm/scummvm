#ifndef TOT_UTIL_H
#define TOT_UTIL_H

#include "common/endian.h"
#include "common/events.h"
#include "common/language.h"
#include "common/scummsys.h"

#include "graphics/font.h"
#include "engines/tot/texts.h"
#include "engines/tot/tot.h"

namespace Tot {

extern const int enforcedTextAnimDelay;

void showError(int code);

// Delays a specified number of milliseconds.
void delay(uint16 ms);

int engine_start();

void setRGBPalette(int color, int r, int g, int b);

void littText(int x, int y, Common::String text, byte color, Graphics::TextAlign align = Graphics::kTextAlignStart);
void littText(int x, int y, char const *text, byte color, Graphics::TextAlign align = Graphics::kTextAlignStart);

void euroText(int x, int y, Common::String text, byte color, Graphics::TextAlign align = Graphics::kTextAlignStart);
void euroText(int x, int y, char const *text, byte color, Graphics::TextAlign align = Graphics::kTextAlignStart);

void biosText(int x, int y, Common::String text, byte color);

unsigned int imagesize(int x, int y, int x2, int y2);

void rectangle(int x1, int y1, int x2, int y2, byte color);

void line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, byte color);

void bar(int x1, int y1, int x2, int y2, byte color);

void putpixel(int x, int y, byte color);

void emptyLoop();

void emptyLoop2();

void waitForKey();

void changeGameSpeed(Common::Event e);

inline bool odd(long i) { return i % 2 != 0; }

unsigned Random(unsigned range);
int Random(int range);

inline Common::String getObjectName(int idx) {
	return g_engine->_lang == Common::ES_ESP ? hardcodedObjects_ES[idx] : hardcodedObjects_EN[idx];
}

inline Common::String getActionLineText(int idx) {
	return g_engine->_lang == Common::ES_ESP ? actionLine_ES[idx] : actionLine_EN[idx];
}
inline Common::KeyCode hotKeyFor(HOTKEYS hotkey) {
	const Common::KeyCode *selectedHotkeys = (g_engine->_lang == Common::ES_ESP)? hotkeys[0]: hotkeys[1];
	return selectedHotkeys[hotkey];
};


} // End of namespace Tot

#endif
