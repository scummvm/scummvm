/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/sprite.h"
#include "common/debug.h"

namespace Sherlock {

/*
struct SpriteFrame {
	byte *data;
	int width, height;
	uint16 flags;
	int xofs, yofs;
	byte rleMarker;
};
*/

Sprite::Sprite(Common::SeekableReadStream &stream) {
	load(stream);
}

Sprite::~Sprite() {
}

int Sprite::getFrameCount() {
	return _frames.size();
}

SpriteFrame *Sprite::getFrame(int index) {
	return _frames[index];
}

void Sprite::load(Common::SeekableReadStream &stream) {

    while (!stream.eos()) {
    
        debug("frameNum = %d\n", _frames.size());
    
        SpriteFrame *spriteFrame = new SpriteFrame();

		uint32 startOfs = stream.pos();

		debug("startOfs = %08X\n", startOfs);

        spriteFrame->frame = NULL;
        spriteFrame->width = stream.readUint16LE() + 1;
        spriteFrame->height = stream.readUint16LE() + 1;
        spriteFrame->flags = stream.readUint16LE();
        stream.readUint16LE();
        
        debug("width = %d; height = %d; flags = %04X\n", spriteFrame->width, spriteFrame->height, spriteFrame->flags);

        if (spriteFrame->flags & 0xFF) {
            spriteFrame->size = (spriteFrame->width * spriteFrame->height) / 2;
        } else if (spriteFrame->flags & 0x0100) {
            // this size includes the header size, which we subtract
            spriteFrame->size = stream.readUint16LE() - 11;
            spriteFrame->rleMarker = stream.readByte();
        } else {
            spriteFrame->size = spriteFrame->width * spriteFrame->height;
        }

        spriteFrame->data = new byte[spriteFrame->size];
        stream.read(spriteFrame->data, spriteFrame->size);
        
        decompressFrame(spriteFrame);

		/*
		debug("size = %d (%08X)\n", spriteFrame->size, spriteFrame->size);
		if (spriteFrame->frame) {
		    char fn[128];
		    sndebug(fn, 128, "%04d.spr", _frames.size());
		    FILE *x = fopen(fn, "wb");
		    fwrite(spriteFrame->frame->pixels, spriteFrame->frame->w * spriteFrame->frame->h, 1, x);
		    fclose(x);
		}
		*/

		_frames.push_back(spriteFrame);
		
    }
    
   // debug("Done: %08X\n", stream.pos()); fflush(stdout);

}

void Sprite::decompressFrame(SpriteFrame *frame) {

    frame->frame = new Graphics::Surface();
	frame->frame->create(frame->width, frame->height, Graphics::PixelFormat::createFormatCLUT8());

	if (frame->flags & 0xFF) {
	    debug("Sprite::decompressFrame() 4-bits/pixel\n");
	    debug("TODO\n");
	} else if (frame->flags & 0x0100) {
	    debug("Sprite::decompressFrame() RLE-compressed; rleMarker = %02X\n", frame->rleMarker);
	    const byte *src = frame->data;
	    byte *dst = (byte *)frame->frame->getPixels();
		for (uint16 h = 0; h < frame->height; h++) {
			int16 w = frame->width;
			while (w > 0) {
			    if (*src == frame->rleMarker) {
			        byte rleColor = src[1];
			        byte rleCount = src[2];
			        src += 3;
			        w -= rleCount;
			        while (rleCount--)
			            *dst++ = rleColor;
				} else {
				    *dst++ = *src++;
				    w--;
				}
			}
		}
	} else {
	    debug("Sprite::decompressFrame() Uncompressed\n");
	    memcpy(frame->data, frame->frame->getPixels(), frame->width * frame->height);
	}

}

} // End of namespace Sherlock
