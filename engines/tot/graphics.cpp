/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/paletteman.h"

#include "tot/graphics.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

signed char datosfundido[256][256];
int ipal, jpal;

void rectangle(int x1, int y1, int x2, int y2, byte color) {
	g_engine->_screen->drawLine(x1, y1, x2, y1, color);
	g_engine->_screen->drawLine(x2, y1, x2, y2, color);
	g_engine->_screen->drawLine(x1, y2, x2, y2, color);
	g_engine->_screen->drawLine(x1, y1, x1, y2, color);
	g_engine->_screen->addDirtyRect(Common::Rect(x1, y1, x2, y2));
}

void rectangle(uint x1, uint y1, uint x2, uint y2, byte color) {
	g_engine->_screen->drawLine(x1, y1, x2, y1, color);
	g_engine->_screen->drawLine(x2, y1, x2, y2, color);
	g_engine->_screen->drawLine(x1, y2, x2, y2, color);
	g_engine->_screen->drawLine(x1, y1, x1, y2, color);
	g_engine->_screen->addDirtyRect(Common::Rect(x1, y1, x2, y2));
}

// Copies the rectangle delimited by getCoord** from backgroundScreen into image
void getVirtualImg(
	uint getcoordx1,        // xframe
	uint getcoordy1,        // yframe
	uint getcoordx2,        // xframe + pasoFrameW
	uint getcoordy2,        // yframe + pasoFrameH
	byte *backgroundScreen, // fondo
	byte *image             // fondsprite
) {
	uint16 w = getcoordx2 - getcoordx1;
	uint16 h = getcoordy2 - getcoordy1;

	WRITE_UINT16(image, w);
	WRITE_UINT16(image + 2, h);
	w++;
	h++;
	int posAbs = 4 + getcoordx1 + (getcoordy1 * 320);

	int sourcePtr = 0;
	byte *destPtr = 4 + image; // Start writing after width and height

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			*destPtr++ = backgroundScreen[posAbs + sourcePtr++];
		}
		sourcePtr += (320 - w); // Move to the beginning of the next row in PantFondo
	}
}
// puts an image into a buffer in the given position, asuming 320 width
void putVirtualImg(uint putcoordx, uint putcoordy, byte *backgroundScreen, byte *image) {
	uint16 w, h;

	w = READ_LE_UINT16(image);
	h = READ_LE_UINT16(image + 2);

	w++;
	h++;
	int posAbs = 4 + putcoordx + (putcoordy * 320);

	int sourcePtr = 0;
	byte *destPtr = 4 + image; // Start writing after width and height

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			// if(destPtr > 0)
			backgroundScreen[posAbs + sourcePtr++] = *destPtr++;
		}
		sourcePtr += (320 - w); // Move to the beginning of the next row in PantFondo
	}
}

void getImg(uint coordx1, uint coordy1, uint coordx2, uint coordy2, byte *image) {

	uint16 width = coordx2 - coordx1;
	uint16 height = coordy2 - coordy1;

	WRITE_LE_UINT16(image, width);
	WRITE_LE_UINT16(image + 2, height);

	width++;
	height++;

	for (int j = 0; j < width; j++) {
		for (int i = 0; i < height; i++) {
			int idx = 4 + i * width + j;
			*(image + idx) = *(byte *)g_engine->_screen->getBasePtr(coordx1 + j, coordy1 + i);
		}
	}
}

void putShape(uint coordx, uint coordy, byte *image) {
	putImg(coordx, coordy, image, true);
}

void drawFullScreen(byte *screen) {
	Common::copy(screen, screen + 64000, (byte *)g_engine->_screen->getPixels());
}

void copyFromScreen(byte *&screen) {
	byte *src = (byte *)g_engine->_screen->getPixels();
	Common::copy(src, src + 64000, screen);
}

void drawScreen(byte *screen, boolean offsetSize) {
	int offset = offsetSize ? 4:0;
	for (int i1 = 0; i1 < 320; i1++) {
		for (int j1 = 0; j1 < 140; j1++) {
			((byte *)g_engine->_screen->getPixels())[j1 * 320 + i1] = ((byte *)screen)[offset + j1 * 320 + i1];
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
}

void putImg(uint coordx, uint coordy, byte *image, boolean transparency) {
	uint16 w, h;

	w = READ_LE_UINT16(image);
	h = READ_LE_UINT16(image + 2);

	w++;
	h++;
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int index = 4 + (j * w + i);
			if (!transparency || image[index] != 0) {
				*(byte *)g_engine->_screen->getBasePtr(coordx + i, coordy + j) = image[index];
			}
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(coordx, coordy, coordx + w, coordy + h));
}

void clear() {
	g_engine->_screen->clear();
}

void loadPalette(Common::String paletteName) {

	Common::File fichero;

	if (!fichero.open(Common::Path(paletteName + ".PAL")))
		error("unable to load palette %s (310)", paletteName.c_str());
	fichero.read(pal, 768);
	fichero.close();

	g_engine->_graphics->fixPalette(pal, 768);
	g_engine->_graphics->setPalette(pal);
}

void effect(byte numeroefecto, boolean pasaranegro, byte *pantalla2) {

	int i1, i2, i3, j1, j2, j3;
	bool enabled = false;
	// numeroefecto = 1;

	byte * screenBuf = (byte *)g_engine->_screen->getPixels();
	// Disable effects for now
	if (!enabled) {
		debug("Effects disabled! requested effect is %d, with fadeToBlack =%d", numeroefecto, pasaranegro);
		if (!pasaranegro) {
			drawScreen(pantalla2);
		}
	} else {
		contadorpc2 = contadorpc;
		if (contadorpc > 145)
			error("effect(): contadorpc (274)");
		debug("Effect %d, pasaraanegro=%d", numeroefecto, pasaranegro);
		if (pasaranegro)

			switch (numeroefecto) {
			case 1: {
				for (i1 = 0; i1 <= 69; i1++) {
					line(0, (i1 * 2), 319, (i1 * 2), 0);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
					delay(5);
				}
				for (i1 = 70; i1 >= 1; i1--) {
					line(0, (i1 * 2 - 1), 319, (i1 * 2 - 1), 0);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
					delay(5);
				}
			} break;
			case 2: {
				for (i1 = 70; i1 >= 1; i1--) {
					line(0, (i1 * 2 - 1), 319, (i1 * 2 - 1), 0);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
					delay(5);
				}
				for (i1 = 0; i1 <= 69; i1++) {
					line(0, (i1 * 2), 319, (i1 * 2), 0);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
					delay(5);
				}
			} break;
			case 3: {
				for (i2 = 1; i2 <= 9; i2++) {
					i1 = 10;
					do {
						j1 = 10;
						do {
							bar((i1 - i2), (j1 - i2), (i1 + i2), (j1 + i2), 0);
							if(j1 % 40 == 0) {
								g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
								g_engine->_screen->update();
							}
							j1 += 20;
						} while (j1 != 150);
						i1 += 20;
					} while (i1 != 330);
				}
				bar(0, 0, 319, 139, 0);
				g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
				g_engine->_screen->update();
			} break;
			case 4: {
				for (i2 = 1; i2 <= 22; i2++) {
					for (i1 = 1; i1 <= 16; i1++)
						for (j1 = 1; j1 <= 7; j1++) {
							if ((i1 + j1) == (i2 + 1)) {
								bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
								g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
								g_engine->_screen->update();
								delay(8);
							}
						}
				}
			} break;
			case 5: {
				for (j1 = 0; j1 <= 139; j1++) {
					line(0, j1, 319, (139 - j1), 0);
					if(j1 % 5 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
				for (j1 = 318; j1 >= 1; j1--) {
					line(j1, 0, (319 - j1), 139, 0);
					if(j1 % 5 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
			} break;
			case 6: {
				for (i1 = 0; i1 < 70; i1++) {
					rectangle(i1, i1, (319 - i1), (139 - i1), 0);
					delay(5);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
				}
			} break;
			case 7: {
				for (i1 = 69; i1 >= 0; i1--) {
					rectangle(i1, i1, (319 - i1), (139 - i1), 0);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
				}
			} break;
			case 8: {

				for (i1 = 0; i1 <= 319; i1++) {
					line(0, 139, i1, 0, 0);
					if(i1 % 5 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
				for (i1 = 1; i1 <= 139; i1++) {
					line(0, 139, 319, i1, 0);
					if(i1 % 5 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
			} break;
			case 9: {

				for (i1 = 319; i1 >= 0; i1--) {
					line(319, 139, i1, 0, 0);
					if(i1 % 5 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
				for (i1 = 1; i1 <= 139; i1++) {
					line(319, 139, 0, i1, 0);
					if(i1 % 5 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
			} break;
			case 10: {
				for (j1 = 1; j1 <= 7; j1++) {
					if ((j1 % 2) == 0)
						for (i1 = 1; i1 <= 16; i1++) {
							bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
							g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
							g_engine->_screen->update();
							delay(8);
						}
					else
						for (i1 = 16; i1 >= 1; i1--) {
							bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
							g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
							g_engine->_screen->update();
							delay(8);
						}
				}
			} break;
			case 11: {
				// for (i2 = 0; i2 <= 3; i2++) {
				// 	j1 = 1 + i2;
				// 	for (i1 = (1 + i2); i1 <= (16 - i2); i1++) {
				// 		bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
				// 		delay(8);
				// 	}
				// 	for (j1 = (2 + i2); j1 <= (7 - i2); j1++) {
				// 		bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
				// 		delay(8);
				// 	}
				// 	for (i1 = (16 - i2); i1 >= (1 + i2); i1--) {
				// 		bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
				// 		delay(8);
				// 	}
				// 	for (j1 = (6 - i2); j1 >= (2 + i2); j1--) {
				// 		bar(((i1 - 1) * 20), ((j1 - 1) * 20), ((i1 - 1) * 20 + 19), ((j1 - 1) * 20 + 19), 0);
				// 		delay(8);
				// 	}
				// }
				drawScreen(pantalla2);
			} break;
			case 12: {
				for (i1 = 1; i1 <= 15000; i1++) {
					i2 = Random(318);
					j2 = Random(138);
					bar(i2, j2, (i2 + 2), (j2 + 2), 0);
					putpixel(Random(320), Random(139), 0);
					if(i1 % 100 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
				bar(0, 0, 319, 139, 0);
				g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
				g_engine->_screen->update();
			} break;
			case 13: {
				for (i1 = 319; i1 >= 0; i1--) {
					line(i1, 0, i1, 139, 0);
					if(i1 % 5 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
			} break;
			case 14: {
				for (i1 = 0; i1 <= 319; i1++) {
					line(i1, 0, i1, 139, 0);
					if(i1 % 5 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
			} break;
			case 15: {

				for (j1 = 0; j1 <= 70; j1++) {
					Common::copy(screenBuf + (j1 * 320), screenBuf + (j1 * 320) + 320, screenBuf + ((j1 + 1) *320));
					// move(ptr(0xa000, (j1 * 320)), ptr(0xa000, ((j1 + 1) * 320)), 320);
					line(0, j1, 319, j1, 0);
					Common::copy(screenBuf + ((139 - j1) * 320), screenBuf + ((139 - j1) * 320) + 320, screenBuf + ((138 - j1) * 320));
					// move(ptr(0xa000, ((139 - j1) * 320)), ptr(0xa000, ((138 - j1) * 320)), 320);
					line(0, (139 - j1), 319, (139 - j1), 0);
					delay(2);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
				}
				delay(5);
				for (j1 = 0; j1 <= 160; j1++) {
					line(0, 68, j1, 68, 0);
					line(319, 68, (319 - j1), 68, 0);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
				}
			} break;
			}
		else
			switch (numeroefecto) {
			case 1: {
				for (i1 = 0; i1 <= 69; i1++) {
					byte *src = pantalla2 + (i1 * 640) + 4;
					Common::copy(src, src + 320, screenBuf + (i1 * 640));
					// move(ptr(seg(pantalla2), (ofs(pantalla2) + (i1 * 640) + 4)), ptr(0xa000, (i1 * 640)), 320);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
					delay(5);
				}
				for (i1 = 70; i1 >= 1; i1--) {
					byte *src = pantalla2 + (i1 * 640) - 320 + 4;
					Common::copy(src, src + 320, screenBuf + (i1 * 640) - 320);
					// move(ptr(seg(pantalla2), (ofs(pantalla2) + ((i1 * 640) - 320) + 4)), ptr(0xa000, ((i1 * 640) - 320)), 320);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
					delay(5);

					// delay(5);
				}
			} break;
			case 2:
			case 5:
			case 8:
			case 9: {
				for (i1 = 70; i1 >= 1; i1--) {
					byte *src = pantalla2 + ((i1 * 640) - 320) + 4;
					Common::copy(src, src + 320, screenBuf + (i1 * 640) - 320);
					// move(ptr(seg(pantalla2), (ofs(pantalla2) + ((i1 * 640) - 320) + 4)), ptr(0xa000, ((i1 * 640) - 320)), 320);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
					delay(5);
				}
				for (i1 = 0; i1 <= 69; i1++) {
					byte *src = pantalla2 + (i1 * 640) + 4;
					Common::copy(src, src + 320, screenBuf + (i1 * 640));
					// move(ptr(seg(pantalla2), (ofs(pantalla2) + (i1 * 640) + 4)), ptr(0xa000, (i1 * 640)), 320);
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
					delay(5);
				}
			} break;
			case 3:
			case 7: {
				for (i2 = 1; i2 <= 9; i2++) {
					i1 = 10;
					do {
						j1 = 10;
						do {
							for (i3 = (j1 - i2); i3 <= (j1 + i2); i3++) {
								byte *src = pantalla2 + + 4 + (i3 * 320) +  + (i1 - i2);
								Common::copy(src, src + (i2 * 2), screenBuf + (i3 * 320) + (i1 - i2));
								// move(ptr(seg(pantalla2), (ofs(pantalla2) + 4 + (i3 * 320) + (i1 - i2))), ptr(0xa000, ((i3 * 320) + (i1 - i2))), (i2 * 2));
							}
							j1 += 20;
						} while (j1 != 150);
						if(i1 % 40 == 0) {
							g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
							g_engine->_screen->update();
						}
						i1 += 20;
					} while (i1 != 330);
				}
				drawScreen(pantalla2);
			} break;
			case 4: {
				for (i2 = 1; i2 <= 22; i2++) {
					for (i1 = 1; i1 <= 16; i1++)
						for (j1 = 1; j1 <= 7; j1++) {
							if ((i1 + j1) == (i2 + 1)) {
								for (i3 = ((j1 - 1) * 20); i3 <= ((j1 - 1) * 20 + 19); i3++) {
									byte *src = pantalla2 + + 4 + (i3 * 320) +  ((i1 - 1) * 20);
									Common::copy(src, src + 20, screenBuf + (i3 * 320) + ((i1 - 1) * 20));
									// move(ptr(seg(pantalla2), (ofs(pantalla2) + 4 + (i3 * 320) +  ((i1 - 1) * 20))),	 ptr(0xa000, ((i3 * 320) + ((i1 - 1) * 20))), 20);
									if(i3 % 4 == 0) {
										g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
										g_engine->_screen->update();
									}
								}
								delay(8);
							}
						}
				}
			} break;
			case 6: {
				for (i1 = 70; i1 >= 0; i1--) {
					for (j1 = i1; j1 <= (139 - i1); j1++) {
						byte *src = pantalla2 + 4 + (j1 * 320) + i1;
						Common::copy(src, src + 319 - (i1 * 2), screenBuf + (j1 * 320) + i1);
						// move(ptr(seg(pantalla2), (ofs(pantalla2) + 4 + (j1 * 320) + i1)),  ptr(0xa000, ((j1 * 320) + i1)), (319 - (i1 * 2)));
					}
					g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
					g_engine->_screen->update();
				}
			} break;
			case 10: {
				for (j1 = 1; j1 <= 7; j1++) {
					if ((j1 % 2) == 0)
						for (i1 = 1; i1 <= 16; i1++) {
							for (j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3++) {
								byte *src = pantalla2 + 4 + (j3 * 320) + ((i1 - 1) *20);

								Common::copy(src, src + 20, screenBuf + (j3 * 320) + ((i1 - 1) * 20));
								// move(ptr(seg(pantalla2), (ofs(pantalla2) + 4 + (j3 * 320) + ((i1 - 1) * 20))), ptr(0xa000, ((j3 * 320) + ((i1 - 1) * 20))), 20);
							}
							// if(i1 % 300 == 0) {
								g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
								g_engine->_screen->update();
							// }
							delay(8);
						}
					else
						for (i1 = 16; i1 >= 1; i1--) {
							for (j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3++) {
								byte *src = pantalla2 + 4 + (j3 * 320) + ((i1 - 1) *20);
								Common::copy(src, src + 20, screenBuf + (j3 * 320) + ((i1 - 1) * 20));
							}
							g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
							g_engine->_screen->update();
							delay(8);
						}
				}
			} break;
			case 11: {
				// for (i2 = 0; i2 <= 3; i2 ++) {
					// j1 = 1 + i2;
					// for (i1 = (1 + i2); i1 <= (16 - i2); i1 ++) {
					// 	for (j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3 ++){
					// 		byte *buf = pantalla2 + 4 + (j3 * 320) + ((i1 - 1) * 20);
					// 		Common::copy(buf, buf + 20, screenBuf + (j3 * 320) + ((i1 - 1) * 20));
					// 		g_engine->_screen->markAllDirty();
					// 		g_engine->_screen->update();
					// 	}
					// 	// delay(8);
					// }
					// for (j1 = (2 + i2); j1 <= (7 - i2); j1 ++) {
					// 	for (j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3 ++) {
					// 		byte *buf = pantalla2 + 4 + (j3 * 320) + ((i1 - 1) * 20);
					// 		Common::copy(buf, buf + 20, screenBuf + (j3 * 320) + ((i1 - 1) * 20));
					// 		g_engine->_screen->markAllDirty();
					// 		g_engine->_screen->update();
					// 		// move(ptr(seg(pantalla2), (ofs(pantalla2) +4 + (j3 * 320) + ((i1 - 1) * 20))), ptr(0xa000, ((j3 * 320) + ((i1 - 1) * 20))), 20);
					// 	}
					// 	// delay(8);
					// }
					// for (i1 = (16 - i2); i1 > (1 + i2); i1 --) {
					// 	for (j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3 ++) {
					// 		byte *buf = pantalla2 + 4 + (j3 * 320) + ((i1 - 1) * 20);
					// 		Common::copy(buf, buf + 20, screenBuf + (j3 * 320) + ((i1 - 1) * 20));
					// 		g_engine->_screen->markAllDirty();
					// 		g_engine->_screen->update();
					// 		// move(ptr(seg(pantalla2), (ofs(pantalla2) +4 + (j3 * 320) + ((i1 - 1) * 20))), ptr(0xa000, ((j3 * 320) + ((i1 - 1) * 20))), 20);
					// 	}
					// 	delay(8);
					// }
					// // i1 = 1;
					// for (j1 = (6 - i2); j1 >= (2 + i2); j1 --) {
					// 	for (j3 = ((j1 - 1) * 20); j3 <= ((j1 - 1) * 20 + 19); j3 ++) {
					// 		byte *buf = pantalla2 + 4 + (j3 * 320) + ((i1 - 1) * 20);
					// 		Common::copy(buf, buf + 20, screenBuf + (j3 * 320) + ((i1 - 1) * 20));
					// 		g_engine->_screen->markAllDirty();
					// 		g_engine->_screen->update();
					// 		// move(ptr(seg(pantalla2), (ofs(pantalla2) +4 + (j3 * 320) + ((i1 - 1) * 20))), ptr(0xa000, ((j3 * 320) + ((i1 - 1) * 20))), 20);
					// 	}
					// 	delay(8);
					// }
				// }
				drawScreen(pantalla2);

			} break;
			case 12: {
				for (i1 = 1; i1 <= 15000; i1++) {
					i2 = Random(318);
					j2 = Random(138);

					byte *src1 = pantalla2 + 4 + (j2 * 320) + i2;
					byte *src2 = pantalla2 + 4 + ((j2 + 1) * 320) + i2;
					Common::copy(src1, src1 + 2, screenBuf + j2 * 320 + i2);
					// move(ptr(seg(pantalla2), (ofs(pantalla2) + 4 + (j2 * 320) + i2)),  ptr(0xa000, ((j2 * 320) + i2)), 2);

					Common::copy(src2, src2 + 2, screenBuf + (j2 + 1) * 320 + i2);
					// move(ptr(seg(pantalla2), (ofs(pantalla2) + 4 + ((j2 + 1) * 320) + i2)), ptr(0xa000, (((j2 + 1) * 320) + i2)), 2);
					i2 = Random(320);
					j2 = Random(140);

					byte *src3 = pantalla2 + 4 + (j2 * 320) + i2;
					Common::copy(src3, src3 + 1, screenBuf + (j2 * 320) + i2);

					if(i1 % 300 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
					// move(ptr(seg(pantalla2), (ofs(pantalla2) + 4 + (j2 * 320) + i2)),  ptr(0xa000, ((j2 * 320) + i2)), 1);
				}
				drawScreen(pantalla2);
			} break;
			case 13: {
				debug("case 13!");

				for (i1 = 0; i1 <= 319; i1++) {
					for (j1 = 0; j1 <= 139; j1++) {
						int color = pantalla2[4 + (j1 * 320) + i1];
						*((byte *)g_engine->_screen->getBasePtr(i1, j1)) = color;


					}
					// if (g_engine->_chrono->shouldPaintEffect()) {
					if(i1 % 10 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
			} break;
			case 14: {
				for (i1 = 320; i1 > 0; i1--) {
					for (j1 = 0; j1 < 140; j1++) {
						int color = pantalla2[4 + (j1 * 320) + i1];
						*((byte *)g_engine->_screen->getBasePtr(i1, j1)) = color;
					}
					if(i1 % 10 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
			} break;
			case 15: {
				for (j1 = 160; j1 > 0; j1--) {
					line(j1, 69, (319 - j1), 69, 255);
					if(j1 % 4 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
				// delay(5);
				for (j1 = 70; j1 > 0; j1--) {
					byte *src1 = pantalla2 + 4 + j1 * 320;
					byte *src2 = pantalla2  + 4 + (139 - j1) * 320;
					Common::copy(src1, src1 + 320, screenBuf + j1 * 320);

					Common::copy(src2, src2 + 320, screenBuf + (139 - j1) * 320);
					if(j1 % 2 == 0) {
						g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
						g_engine->_screen->update();
					}
				}
			} break;
			}
	}
}

void changeRGBBlock(byte initialColor, uint numColors, void *rgb) {
	g_system->getPaletteManager()->setPalette((byte *)rgb, initialColor, numColors);
}

void changePalette(palette depaleta, palette apaleta) {
	palette palpaso;
	int auxpaso;

	copyPalette(depaleta, palpaso);
	for (jpal = 32; jpal >= 0; jpal--) {
		for (ipal = 0; ipal < 256; ipal++) {
			auxpaso = apaleta[3 * ipal + 0] - palpaso[3 * ipal + 0];
			if (auxpaso > 0)
				palpaso[3 * ipal + 0] = palpaso[3 * ipal + 0] + datosfundido[auxpaso][jpal];
			else
				palpaso[3 * ipal + 0] = palpaso[3 * ipal + 0] - datosfundido[-auxpaso][jpal];

			auxpaso = apaleta[3 * ipal + 1] - palpaso[3 * ipal + 1];
			if (auxpaso > 0)
				palpaso[3 * ipal + 1] = palpaso[3 * ipal + 1] + datosfundido[auxpaso][jpal];
			else
				palpaso[3 * ipal + 1] = palpaso[3 * ipal + 1] - datosfundido[-auxpaso][jpal];

			auxpaso = apaleta[3 * ipal + 2] - palpaso[3 * ipal + 2];
			if (auxpaso > 0)
				palpaso[3 * ipal + 2] = palpaso[3 * ipal + 2] + datosfundido[auxpaso][jpal];
			else
				palpaso[3 * ipal + 2] = palpaso[3 * ipal + 2] - datosfundido[-auxpaso][jpal];
		}
		changeRGBBlock(0, 256, palpaso);
		g_engine->_screen->markAllDirty();
		g_engine->_screen->update();
	}
}

void turnLightOn() {
	Common::File paletteFile;
	palette palpaso;

	if (!paletteFile.open("PALETAS.DAT")) {
		error("turnLightOn(): ioresult (311)");
	}
	paletteFile.seek(1536);
	paletteFile.read(palpaso, 768);
	paletteFile.close();
	for (int i = 0; i < 256; i++) {
		if (i >= 201) {
			palpaso[3 * i + 0] = pal[3 * i + 0];
			palpaso[3 * i + 1] = pal[3 * i + 1];
			palpaso[3 * i + 2] = pal[3 * i + 2];
		// }
		} else {
			palpaso[3 * i + 0] = palpaso[3 * i + 0] << 2;
			palpaso[3 * i + 1] = palpaso[3 * i + 1] << 2;
			palpaso[3 * i + 2] = palpaso[3 * i + 2] << 2;
		}
	}
	changePalette(pal, palpaso);
	copyPalette(palpaso, pal);
}

void totalFadeOut(byte rojo) {
	palette palpaso;

	for (ipal = 0; ipal <= 255; ipal++) {
		palpaso[3 * ipal + 0] = rojo;
		palpaso[3 * ipal + 1] = 0;
		palpaso[3 * ipal + 2] = 0;
	}
	changePalette(pal, palpaso);
	copyPalette(palpaso, pal);
}

void partialFadeOut(byte numcol) {
	palette palpaso;

	for (ipal = 0; ipal <= numcol; ipal++) {
		palpaso[3 * ipal + 0] = 0;
		palpaso[3 * ipal + 1] = 0;
		palpaso[3 * ipal + 2] = 0;
	}
	for (ipal = (numcol + 1); ipal <= 255; ipal++) {
		palpaso[3 * ipal + 0] = pal[3 * ipal + 0];
		palpaso[3 * ipal + 1] = pal[3 * ipal + 1];
		palpaso[3 * ipal + 2] = pal[3 * ipal + 2];
	}
	changePalette(pal, palpaso);
	copyPalette(palpaso, pal);
}

void partialFadeIn(byte numcol) {
	palette palnegro;

	for (ipal = 0; ipal <= numcol; ipal++) {
		palnegro[3 * ipal + 0] = 0;
		palnegro[3 * ipal + 1] = 0;
		palnegro[3 * ipal + 2] = 0;
	}
	for (ipal = (numcol + 1); ipal <= 255; ipal++) {
		palnegro[3 * ipal + 0] = pal[3 * ipal + 0];
		palnegro[3 * ipal + 1] = pal[3 * ipal + 1];
		palnegro[3 * ipal + 2] = pal[3 * ipal + 2];
	}
	changePalette(palnegro, pal);
}

void totalFadeIn(uint numeropaleta, Common::String nombrepaleta) {
	palette palpaso, palnegro;
	Common::File fichpaleta;
	if (numeropaleta > 0) {
		if (!fichpaleta.open("PALETAS.DAT"))
			error("totalFadeIn(): ioresult (311)");
		fichpaleta.seek(numeropaleta);
		fichpaleta.read(palpaso, 768);
		fichpaleta.close();
	} else {
		if (!fichpaleta.open(Common::Path(Common::String(nombrepaleta + ".PAL")))) {
			error("totalFadeIn(): ioresult (311)");
		}
		fichpaleta.read(palpaso, 768);
		fichpaleta.close();
	}
	for (ipal = 0; ipal <= 255; ipal++) {
		palnegro[3 * ipal + 0] = 0;
		palnegro[3 * ipal + 1] = 0;
		palnegro[3 * ipal + 2] = 0;
		// 6-bit color correction
		palpaso[3 * ipal + 0] = palpaso[3 * ipal + 0] << 2;
		palpaso[3 * ipal + 1] = palpaso[3 * ipal + 1] << 2;
		palpaso[3 * ipal + 2] = palpaso[3 * ipal + 2] << 2;
	}

	changePalette(palnegro, palpaso);
	copyPalette(palpaso, pal);
}

void redFadeIn(palette palpaso) {
	palette palnegro;

	for (ipal = 0; ipal < 256; ipal++) {
		palnegro[3 * ipal + 0] = 0;
		palnegro[3 * ipal + 1] = 0;
		palnegro[3 * ipal + 2] = 0;
	}
	changePalette(pal, palnegro);
	changePalette(palnegro, palpaso);
	copyPalette(palpaso, pal);
}

void updatePalette(byte indicepaleta) {
	int ip;

	switch (parte_del_juego) {
	case 1: {
		for (ipal = 0; ipal <= 5; ipal++) {
			pal[(ipal + 195) * 3 + 0] = movimientopal[(indicepaleta * 6 + ipal) * 3 + 0];
			pal[(ipal + 195) * 3 + 1] = movimientopal[(indicepaleta * 6 + ipal) * 3 + 1];
			pal[(ipal + 195) * 3 + 2] = movimientopal[(indicepaleta * 6 + ipal) * 3 + 2];
		}
		changeRGBBlock(195, 6, &pal[195 * 3 + 0]);
	} break;
	case 2: {
		switch (indicepaleta) {
		case 0:
			ip = 0;
			break;
		case 1:
			ip = 4;
			break;
		case 2:
			ip = 8;
			break;
		case 3:
			ip = 4;
			break;
		case 4:
			ip = 0;
			break;
		case 5:
			ip = -4;
			break;
		case 6:
			ip = -8;
			break;
		case 7:
			ip = -4;
			break;
		}
		byte *palette = g_engine->_graphics->getPalette();

		palette[131 * 3 + 0] = palette[131 * 3 + 0] - ip;
		palette[131 * 3 + 1] = palette[131 * 3 + 1] - ip;
		palette[131 * 3 + 2] = palette[131 * 3 + 2] - ip;

		palette[134 * 3 + 0] = palette[134 * 3 + 0] - ip;
		palette[134 * 3 + 1] = palette[134 * 3 + 1] - ip;
		palette[134 * 3 + 2] = palette[134 * 3 + 2] - ip;

		palette[143 * 3 + 0] = palette[143 * 3 + 0] - ip;
		palette[143 * 3 + 1] = palette[143 * 3 + 1] - ip;
		palette[143 * 3 + 2] = palette[143 * 3 + 2] - ip;

		palette[187 * 3 + 0] = palette[187 * 3 + 0] - ip;
		palette[187 * 3 + 1] = palette[187 * 3 + 1] - ip;
		palette[187 * 3 + 2] = palette[187 * 3 + 2] - ip;

		g_engine->_graphics->setPalette(palette);

	} break;
	}
}

void processingActive() {

	setRGBPalette(255, 63, 63, 63);
	outtextxy(121, 72, "PROCESANDO......", 0);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(120, 71, "PROCESANDO......", 0);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(119, 72, "PROCESANDO......", 0);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(120, 73, "PROCESANDO......", 0);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(120, 72, "PROCESANDO......", 255);
	g_engine->_screen->update();

	// enforce a delay for now so it's visible
	g_system->delayMillis(200);
}

void initGraph() {
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
			datosfundido[i][j] = i / (j + 1);
}

void handPantallaToBackground() {
	Common::copy(handpantalla + 4, handpantalla + sizepantalla, background + 4);
}

void copyPalette(palette from, palette to) {
	Common::copy(from, from + 768, to);
}

void loadAnimationIntoBuffer(Common::SeekableReadStream *stream, byte *&buf, int animSize) {
	buf = (byte *)malloc(animSize);
	stream->read(buf, animSize);
	Common::copy(buf, buf + animSize, pasoanimado);
}

} // End of namespace Tot
