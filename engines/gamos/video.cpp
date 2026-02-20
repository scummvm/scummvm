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

#include "gamos/gamos.h"
#include "image/bmp.h"
#include "video/avi_decoder.h"

namespace Gamos {

void GamosEngine::surfacePaletteRemap(Graphics::Surface *dst, const byte *tgtPalette, const Graphics::Surface *src, const byte *srcPalette, int srcColorCount) {
    byte remap[256];
    Graphics::PaletteLookup ptgt(tgtPalette, 256);

    const byte *psrc = srcPalette;
    for (int i = 0; i < srcColorCount; i++) {
        remap[i] = ptgt.findBestColor(psrc[0], psrc[1], psrc[2]);
        psrc += 3;
    }

    for (int y = 0; y < dst->h; y++) {
        byte *bdst = (byte *)dst->getBasePtr(0, y);
        const byte *bsrc = (const byte *)src->getBasePtr(0, y);
        for (int x = 0; x < dst->w; x++) {
            *bdst = remap[*bsrc];
            bdst++;
            bsrc++;
        }
    }
}

void GamosEngine::playVideo(const Common::String &video, const Common::Point &pos, const Common::Point &size) {
    if (shouldQuit())
        return;

    Common::Path bmpPath(_string1 + '/' + video + ".bmp", '/');
    Common::Path aviPath(_string1 + '/' + video + ".avi", '/');


    const Graphics::Surface *bkg = nullptr;
    const byte *bkgPalette = nullptr;
    int bkgPaletteCount = 0;

    ::Image::BitmapDecoder bmp;

    Graphics::Surface screenCopy;
    Common::Array<byte> screenPalette;

    bool isAllocated = false;


    if (!SearchMan.hasFile(aviPath))
        return;

    bool loadbkg = false;

    if (SearchMan.hasFile(bmpPath)) {
        Common::File f;
        if (f.open(bmpPath)) {
            bmp.loadStream(f);
            f.close();

            bkg = bmp.getSurface();
            bkgPalette = bmp.getPalette().data();
            bkgPaletteCount = bmp.getPalette().size();
            loadbkg = true;
        }
    }

    if (!loadbkg) {
        screenCopy.copyFrom( *_screen->surfacePtr() );
        bkg = &screenCopy;

        screenPalette.resize(3 * 256);
        _screen->getPalette( screenPalette.data() );

        bkgPalette = screenPalette.data();
        bkgPaletteCount = 256;

        isAllocated = true;
    }

    Common::File *avifile = new Common::File();
    if (!avifile->open(aviPath)) {
        delete avifile;
        return;
    }

    Video::AVIDecoder avi;
    avi.loadStream(avifile);
    avi.start();

    Common::Point sz = size;
    if (sz.x <= 0 || sz.y <= 0) {
        sz.x = avi.getWidth();
        sz.y = avi.getHeight();
    }

    bool dither = false;
    if (avi.getPixelFormat().bytesPerPixel != 1) {
        if (loadbkg) {
            usePalette(bkgPalette, bkgPaletteCount, 0, true);
            _screen->copyFrom(*bkg);
            _screen->markAllDirty();
        }

        avi.setDitheringPalette(_screen->getPalette().data());
        dither = true;
    }

    Common::Event e;
    while (!avi.endOfVideo()) {
        if (eventsSkip(true))
            break;

        if (avi.needsUpdate()) {
            const Graphics::Surface *frm = avi.decodeNextFrame();
            if (!dither && avi.hasDirtyPalette()) {
                _screen->setPalette(avi.getPalette());
                surfacePaletteRemap(_screen->surfacePtr(), avi.getPalette(), bkg, bkgPalette, bkgPaletteCount);
                _screen->markAllDirty();
            }
            if (frm) {
                _screen->blitFrom(*frm, Common::Rect(sz.x, sz.y), pos);
                _screen->addDirtyRect(Common::Rect(pos, sz.x, sz.y));
                _screen->update();
            }
        } else {
            _system->updateScreen();
        }

        _system->delayMillis(1);
    }

    avi.stop();

    setPaletteCurrentGS();
    if (isAllocated) {
        screenCopy.free();
        screenPalette.clear();
    }
}

};
