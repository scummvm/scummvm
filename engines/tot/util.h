#ifndef TOT_UTIL_H
#define TOT_UTIL_H

#include "common/endian.h"
#include "common/events.h"
#include "common/scummsys.h"

#include "graphics/font.h"

namespace Tot {

typedef unsigned char boolean;

void _exit(int code);

// Delays a specified number of milliseconds.
void delay(uint16 ms);

// Starts the internal speaker with given frequency. The speaker continues until explicitly turned off by a call to NoSound.
void sound(uint16 frequency, int length);

int engine_start();

void setRGBPalette(int color, int r, int g, int b);

void outtextxy(int x, int y, char const *text, byte color, bool euro = false, Graphics::TextAlign align = Graphics::kTextAlignStart);

void outtextxy(int x, int y, Common::String text, byte color, bool euro= false, Graphics::TextAlign align = Graphics::kTextAlignStart);

void outtextxyBios(int x, int y, Common::String text, byte color);

void cleardevice(void);

unsigned int imagesize(int x, int y, int x2, int y2);

void line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, byte color);

void bar(int x1, int y1, int x2, int y2, byte color);

void putpixel(int x, int y, byte color);

void emptyLoop();

void emptyLoop2();

void waitForKey();

void changeGameSpeed(Common::Event e);

inline int keypressed() {
    warning("Stub keypressed!");
	return 0;
}

inline boolean odd(long i) { return i%2!= 0; }

unsigned Random(unsigned range);
int Random(int range);

} // End of namespace Tot

#endif
