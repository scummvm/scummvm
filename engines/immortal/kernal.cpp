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

/* This file covers both Kernal.GS and Driver.GS.
 * This is because most of Driver.GS is hardware specific,
 * and what is not (the slightly abstracted aspects), is
 * directly connected to kernal, and might as well be
 * considered part of the same process.
 */

#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"

#include "immortal/immortal.h"
#include "immortal/disk.h"
#include "immortal/compression.h"

namespace Immortal {

/* 
 *
 * -----                -----
 * ----- Main Functions -----
 * -----                -----
 *
 */

Common::ErrorCode ImmortalEngine::main() {
    Common::Event event;
    g_system->getEventManager()->pollEvent(event);

    userIO();
    noNetwork();
    pollKeys();
    logic();
    pollKeys();
    if (logicFreeze() == 0) {
        drawUniv();
        pollKeys();
        fixColors();
        copyToScreen();
        pollKeys();
    }

    return Common::kNoError;
}

void ImmortalEngine::delay(int j) {             // Delay is measured in jiffies, which are 56.17ms
    g_system->delayMillis(j * 56);
}

void ImmortalEngine::delay4(int j) {            // Named in source quarterClock for some reason, 1/4 jiffies are 14.04ms
    g_system->delayMillis(j * 14);
}

void ImmortalEngine::delay8(int j) {            // 1/8 jiffies are 7.02ms
    g_system->delayMillis(j * 7);
}

/* 
 *
 * -----                          -----
 * ----- Screen Drawing Functions -----
 * -----                          -----
 *
 */

void ImmortalEngine::drawUniv() {
    /* The byte buffer for the screen (_screenBuff) has one byte for
     * every pixel, with the resolution of the game being 320x200.
     * For a bitmap like the window frame, all we need to do is
     * extract the pixel out of each nyble (half byte) of the data,
     * by looping over it one row at a time.
     */

    // Apply the window frame to the buffer
    _window.seek(0);
    byte pixel;
    int pos;
    for (int y = 0; y < _resV; y++) {
        for (int x = 0; x < _resH; x += 2) {
            pos = (y * _resH) + x;
            pixel = _window.readByte();
            _screenBuff[pos]     = (pixel & kMask8High) >> 4;
            _screenBuff[pos + 1] =  pixel & kMask8Low;
        }
    }

    /* copyRectToSurface will apply the screenbuffer to the ScummVM surface.
     * We want to do 320 bytes per scanline, at location (0,0), with a
     * size of 320x200.
     */
    _mainSurface->copyRectToSurface(_screenBuff, _resH, 0, 0, _resH, _resV);

}

void ImmortalEngine::copyToScreen() {
    if (_draw == 1) {
        g_system->copyRectToScreen((byte *)_mainSurface->getPixels(), _resH, 0, 0, _resH, _resV);
        g_system->updateScreen();
    }
}

void ImmortalEngine::clearScreen() {
    //fill the visible screen with black pixels by drawing a rectangle

    //rect(32, 20, 256, 128, 0)
    
    if ((_dontResetColors & kMaskLow) == 0) {
        useNormal();
    }
}

/*
 *
 * -----              -----
 * ----- File Loading -----
 * -----              -----
 *
 */

void ImmortalEngine::loadSprites() {
    // Load MoreSprites.spr


}

void ImmortalEngine::loadWindow() {
    // Initialize the window bitmap
    if (!_window.open("WINDOWS.BM")) {
        debug("oh nose :(");
    }
}

void ImmortalEngine::loadFont() {
    // Initialize the font sprite
    if (!_font.open("FONT.SPR")) {
        debug("oh nose :(");
    }
}

Common::SeekableReadStream *ImmortalEngine::loadIFF(Common::String fileName) {
    Common::File f;
    if (!f.open(fileName)) {
        debug("*surprised pikachu face*");
        return nullptr;
    }

    /* This isn't the most efficient way to do this (could just read a 32bit uint and compare),
     * but this makes it more obvious what the source was doing. We want to know if the 4 bytes
     * at file[8] are 'C' 'M' 'P' '0', so this grabs just the ascii bits of those 4 bytes,
     * allowing us to directly compare it with 'CMP0'.
     */
    char compSig[] = "CMP0";
        char sig[] = "0000";

    f.seek(8);

    for (int i = 0; i < 4; i++) {
        sig[i] = f.readByte() & kMaskASCII;
    }

    if (strcmp(sig, compSig) == 0) {
        debug("compressed");
        
        /* The size of the compressed data is stored in the header, but doesn't
         * account for the FORM part?? Also, **technically** this is a uint32LE,
         * but the engine itself actually /doesn't/ use it like that. It only
         * decrements the first word (although it compares against the second half,
         * as if it is expecting that to be zero? It's a little bizarre).
         */
        f.seek(6);
        int len = f.readUint16LE() - 4;

        // Compressed files have a 12 byte header before the data
        f.seek(12);
        return Compression::unCompress(&f, len);
    }

    byte *out = (byte *)malloc(f.size());
    f.read(out, f.size());
    return new Common::MemoryReadStream(out, f.size(), DisposeAfterUse::YES);

}


/*
 *
 * -----                   -----
 * ----- Palette Functions -----
 * -----                   -----
 *
 */

/* Palettes on the Apple IIGS:
 * In High-res mode you have 2 options: 320x200 @ 4bpp or 320x640 @ 2bpp.
 * The Immortal uses the former, giving us 16 colours to use
 * for any given pixel on the screen (ignoring per scanline palettes because
 * The Immortal does not use them). This 16 colour palette is made of 2 byte
 * words containing the RGB components in the form 0RGB.
 *
 * The equivalent palette for ScummVM is a byte stream of up to 256
 * colours composed of 3 bytes each, ending with a transparency byte.
 *
 * Because each colour in the game palette is only a single nyble (4 bits),
 * we also need to multiply the nyble up to the size of a byte (* 16, or << 4).
 */

void ImmortalEngine::loadPalette() {
    // The palettes are stored at a particular location in the disk, this just grabs them
    Common::File d;
    d.open("IMMORTAL.dsk");
    d.seek(kPaletteOffset);

    d.read(_palDefault, 32);
    d.read(_palWhite, 32);
    d.read(_palBlack, 32);
    d.read(_palDim, 32);

    d.close();
}

void ImmortalEngine::setColors(uint16 pal[]) {
    // The RGB palette is 3 bytes per entry, and each byte is a colour
    for (int i = 0; i < 16; i++) {

        // The palette gets masked so it can update only specific indexes and uses FFFF to do so. However the check is simply for a negative
        if (pal[i] < kMaskNeg) {

            // Green is already the correct size, being the second nyble (00G0)
            // Red is in the first nyble of the high byte, so it needs to move right by 4 bits (0R00 -> 00R0)
            // Blue is the first nyble of the first byte, so it needs to move left by 4 bits (000B -> 00B0)
            _palRGB[(i * 3)]     = ((pal[i] & kMaskRed) >> 4);
            _palRGB[(i * 3) + 1] = ((pal[i] & kMaskGreen));
            _palRGB[(i * 3) + 2] = (pal[i] & kMaskBlue) << 4;
        }
    }
    // Palette index to update first is 0, and there are 16 colours to update
    g_system->getPaletteManager()->setPalette(_palRGB, 0, 16);
    g_system->updateScreen();
}

void ImmortalEngine::fixColors() {
    // Pretty silly that this is done with two separate variables, could just index by one...
    if (_dim == true) {
        if (_usingNormal == true) {
            useDim();
        }
    } else {
        if (_usingNormal == false) {
            useNormal();
        }
    }
}

void ImmortalEngine::pump() {
    // Flashes the screen (except the frame thankfully) white, black, white, black, then clears the screen and goes back to normal
    useWhite();
    g_system->updateScreen();
    delay(2);
    useBlack();
    g_system->updateScreen();
    delay(2);
    useWhite();
    g_system->updateScreen();
    delay(2);
    useBlack();
    g_system->updateScreen();
    clearScreen();
    // Why does it do this instead of setting _dontResetColors for clearScreen() instead?
    useNormal();
}

void ImmortalEngine::fadePal(uint16 pal[], int count, uint16 target[]) {
    /* This will fade the palette used by everything inside the game screen
     * but will not touch the window frame palette. It essentially takes the
     * color value nyble, multiplies it by a multiplier, then takes the whole
     * number result and inserts it into the word at the palette index of the
     * temporary palette. This could I'm sure be done with regular multiplication
     * and division operators, but in case the bits that get dropped are otherwise
     * kept, this is a direct translation of the bit manipulation sequence.
     */
    int maskPal[16] = {0xFFFF, 0x0000, 0x0000, 0x0000,
                       0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                       0xFFFF, 0xFFFF, 0xFFFF, 0x0000,
                       0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    uint16 result;
    uint16 temp;

    for (int i = 15; i >= 0; i--) {
        result = maskPal[i];
        if (result == 0) {
            result = pal[i];
            if (result != 0xFFFF) {
                // Blue = 0RGB -> 000B -> 0BBB -> BB0B -> 000B
                result = (xba(mult16((result & kMaskFirst), count))) & kMaskFirst;

                // Green = 0RGB -> 00RG -> 000G -> 0GGG -> GG0G -> 000G -> 00G0 -> 00GB
                temp = mult16(((pal[i] >> 4) & kMaskFirst), count);
                temp = (xba(temp) & kMaskFirst) << 4;
                result = temp | result;

                // Red = 0RGB -> GB0R -> 000R -> 0RRR -> RR0R -> 000R -> 0R00 -> 0RGB
                temp = xba(pal[i]) & kMaskFirst;
                temp = xba(mult16(temp, count));
                temp = xba(temp & kMaskFirst);
                result = temp | result;
            }
        }
        target[i] = result;
    }
}

void ImmortalEngine::fade(uint16 pal[], int dir, int delay) {
    // This temp palette will have FFFF in it, which will be understood as masks by setColors()
    uint16 target[16];
    uint16 count;

    // Originally used a branch, but this is functionally identical and much nicer
    count = dir * 256;

    while ((count >= 0) && (count <= 256)) {
        fadePal(pal, count, target);
        delay8(delay);
        setColors(target);

        // Same as above, it was originally a branch, this does the same thing
        count += (dir == 0) ? 16 : -16;
    }
}

// These two can probably be removed and instead use an enum to declare fadeout/in
void ImmortalEngine::fadeOut(int j) {
    fade(_palDefault, 1, j);
}

void ImmortalEngine::fadeIn(int j) {
    fade(_palDefault, 0, j);
}

// These two can probably be removed since the extra call in C doesn't have the setup needed in ASM
void ImmortalEngine::useBlack() {
    setColors(_palBlack);
}
void ImmortalEngine::useWhite() {
    setColors(_palBlack);
}

void ImmortalEngine::useNormal() {
    setColors(_palDefault);
     _usingNormal = true;
}

void ImmortalEngine::useDim() {
    setColors(_palDim);
    _usingNormal = false;
}


/*
 *
 * -----                 -----
 * ----- Input Functions -----
 * -----                 -----
 *
 */

void ImmortalEngine::userIO() {}
void ImmortalEngine::pollKeys() {}
void ImmortalEngine::noNetwork() {}

void ImmortalEngine::loadSingles(Common::String songName) {
    debug("%s", songName.c_str());
}
void ImmortalEngine::clearSprites() {}
void ImmortalEngine::keyTraps() {}


} // namespace Immortal




















