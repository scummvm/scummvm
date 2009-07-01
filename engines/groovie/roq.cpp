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
 * $URL$
 * $Id$
 *
 */

// ROQ video player based on this specification by Dr. Tim Ferguson:
// http://www.csse.monash.edu.au/~timf/videocodec/idroq.txt

#include "groovie/groovie.h"
#include "groovie/roq.h"

#include "graphics/jpeg.h"

#ifdef ENABLE_RGB_COLOR
// Required for the YUV to RGB conversion
#include "graphics/dither.h"
#endif
#include "sound/mixer.h"

namespace Groovie {

ROQPlayer::ROQPlayer(GroovieEngine *vm) :
#ifdef DITHER
	_dither(NULL),
#endif
	VideoPlayer(vm), _codingTypeCount(0) {

	// Create the work surfaces
	_currBuf = new Graphics::Surface();
	_prevBuf = new Graphics::Surface();

	if (_vm->_mode8bit) {
		byte pal[256 * 4];
#ifdef DITHER
		byte pal3[256 * 3];
		// Initialize to a black palette
		for (int i = 0; i < 256 * 3; i++) {
			pal3[i] = 0;
		}

		// Build a basic color palette
		for (int r = 0; r < 4; r++) {
			for (int g = 0; g < 4; g++) {
				for (int b = 0; b < 4; b++) {
					byte col = (r << 4) | (g << 2) | (b << 0);
					pal3[3 * col + 0] = r << 6;
					pal3[3 * col + 1] = g << 6;
					pal3[3 * col + 2] = b << 6;
				}
			}
		}

		// Initialize the dithering algorithm
		_paletteLookup = new Graphics::PaletteLUT(8, Graphics::PaletteLUT::kPaletteYUV);
		_paletteLookup->setPalette(pal3, Graphics::PaletteLUT::kPaletteRGB, 8);
		for (int i = 0; (i < 64) && !_vm->shouldQuit(); i++) {
			debug("Groovie::ROQ: Building palette table: %02d/63", i);
			_paletteLookup->buildNext();
		}

		// Prepare the palette to show
		for (int i = 0; i < 256; i++) {
			pal[(i * 4) + 0] = pal3[(i * 3) + 0];
			pal[(i * 4) + 1] = pal3[(i * 3) + 1];
			pal[(i * 4) + 2] = pal3[(i * 3) + 2];
		}
#else // !DITHER
		// Set a grayscale palette
		for (int i = 0; i < 256; i++) {
			pal[(i * 4) + 0] = i;
			pal[(i * 4) + 1] = i;
			pal[(i * 4) + 2] = i;
		}
#endif // DITHER

		_syst->setPalette(pal, 0, 256);
	}
}

ROQPlayer::~ROQPlayer() {
	// Free the buffers
	_currBuf->free();
	delete _currBuf;
	_prevBuf->free();
	delete _prevBuf;
	_showBuf.free();

#ifdef DITHER
	// Free the dithering algorithm
	delete _dither;
	delete _paletteLookup;
#endif
}

uint16 ROQPlayer::loadInternal() {
	// Begin reading the file
	debugC(1, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Loading video");

	// Read the file header
	ROQBlockHeader blockHeader;
	if (!readBlockHeader(blockHeader)) {
		return 0;
	}

	// Verify the file signature
	if (blockHeader.type != 0x1084) {
		return 0;
	}

	// Clear the dirty flag
	_dirty = true;

	// Reset the codebooks
	_num2blocks = 0;
	_num4blocks = 0;

	if ((blockHeader.size == 0) && (blockHeader.param == 0)) {
		// Set the offset scaling to 2
		_offScale = 2;

		// Hardcoded FPS
		return 30;
	} else if (blockHeader.size == (uint32)-1) {
		// Set the offset scaling to 1
		_offScale = 1;

		// In this case the block parameter is the framerate
		return blockHeader.param;
	} else {
		warning("Groovie::ROQ: Invalid header with size=%d and param=%d", blockHeader.size, blockHeader.param);
		return 0;
	}
}

void ROQPlayer::buildShowBuf() {
#ifdef DITHER
	// Start a new frame dithering
	_dither->newFrame();
#endif

	for (int line = 0; line < _showBuf.h; line++) {
		byte *out = (byte *)_showBuf.getBasePtr(0, line);
		byte *in = (byte *)_prevBuf->getBasePtr(0, line / _scaleY);
		for (int x = 0; x < _showBuf.w; x++) {
			if (_vm->_mode8bit) {
#ifdef DITHER
				*out = _dither->dither(*in, *(in + 1), *(in + 2), x);
#else
				// Just use the luminancy component
				*out = *in;
#endif // DITHER
#ifdef ENABLE_RGB_COLOR
			} else {
				// Do the format conversion (YUV -> RGB -> Screen format)
				byte r, g, b;
				Graphics::PaletteLUT::YUV2RGB(*in, *(in + 1), *(in + 2), r, g, b);
				// FIXME: this is fixed to 16bit
				*(uint16 *)out = (uint16)_vm->_pixelFormat.RGBToColor(r, g, b);
#endif // ENABLE_RGB_COLOR
			}

			// Skip to the next pixel
			out += _vm->_pixelFormat.bytesPerPixel;
			if (!(x % _scaleX))
				in += _prevBuf->bytesPerPixel;
		}
#ifdef DITHER
		_dither->nextLine();
#endif
	}
}

bool ROQPlayer::playFrameInternal() {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Playing frame");

	// Process the needed blocks until the next video frame
	bool endframe = false;
	while (!endframe && !_file->eos()) {
		endframe = processBlock();
	}

	if (_dirty) {
		// Build the show buffer from the previous (back) buffer
		buildShowBuf();
	}

	// Wait until the current frame can be shown
	waitFrame();

	if (_dirty) {
		// Update the screen
		_syst->copyRectToScreen((byte *)_showBuf.getBasePtr(0, 0), _showBuf.pitch, 0, (_syst->getHeight() - _showBuf.h) / 2, _showBuf.w, _showBuf.h);
		_syst->updateScreen();

		// Clear the dirty flag
		_dirty = false;
	}

	// Return whether the video has ended
	return _file->eos();
}

bool ROQPlayer::readBlockHeader(ROQBlockHeader &blockHeader) {
	if (_file->eos()) {
		return false;
	} else {
		blockHeader.type = _file->readUint16LE();
		blockHeader.size = _file->readUint32LE();
		blockHeader.param = _file->readUint16LE();

		debugC(10, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Block type = 0x%02X", blockHeader.type);
		debugC(10, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Block size = 0x%08X", blockHeader.size);
		debugC(10, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Block param = 0x%04X", blockHeader.param);

		return true;
	}
}

bool ROQPlayer::processBlock() {
	// Read the header of the block
	ROQBlockHeader blockHeader;
	if (!readBlockHeader(blockHeader)) {
		return true;
	}

	// Calculate where the block should end
	int32 endpos = _file->pos() + blockHeader.size;

	// Detect the end of the video
	if (_file->eos()) {
		return false;
	}

	bool ok = true;
	bool endframe = false;
	switch (blockHeader.type) {
	case 0x1001: // Video info
		ok = processBlockInfo(blockHeader);
		break;
	case 0x1002: // Quad codebook definition
		ok = processBlockQuadCodebook(blockHeader);
		break;
	case 0x1011: { // Quad vector quantised video frame
		ok = processBlockQuadVector(blockHeader);
		_dirty = true;
		endframe = true;

		// Swap buffers
		Graphics::Surface *tmp = _prevBuf;
		_prevBuf = _currBuf;
		_currBuf = tmp;
		break;
	}
	case 0x1012: // Still image (JPEG)
		ok = processBlockStill(blockHeader);
		break;
	case 0x1013: // Hang
		assert(blockHeader.size == 0 && blockHeader.param == 0);
		break;
	case 0x1020: // Mono sound samples
		ok = processBlockSoundMono(blockHeader);
		break;
	case 0x1021: // Stereo sound samples
		ok = processBlockSoundStereo(blockHeader);
		break;
	case 0x1030: // Audio container
		endpos = _file->pos();
		ok = processBlockAudioContainer(blockHeader);
		break;
	default:
		warning("Groovie::ROQ: Unknown block type: 0x%04X", blockHeader.type);
		ok = false;
		_file->skip(blockHeader.size);
	}

	if (endpos != _file->pos())
		warning("Groovie::ROQ: BLOCK %04x Should have ended at %d, and has ended at %d", blockHeader.type, endpos, _file->pos());

	// End the frame when the graphics have been modified or when there's an error
	return endframe || !ok;
}

bool ROQPlayer::processBlockInfo(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing info block");

	// Verify the block header
	if (blockHeader.type != 0x1001 || blockHeader.size != 8 || (blockHeader.param != 0 && blockHeader.param != 1)) {
		warning("Groovie::ROQ: BlockInfo size=%d param=%d", blockHeader.size, blockHeader.param);
		return false;
	}

	// Save the alpha channel size
	_alpha = blockHeader.param;

	// Read the information
	uint16 width = _file->readUint16LE();
	uint16 height = _file->readUint16LE();
	uint16 unk1 = _file->readUint16LE();
	uint16 unk2 = _file->readUint16LE();
	if (unk1 != 8 || unk2 != 4) {
		warning("Groovie::ROQ: unk1 = %d, unk2 = %d", unk1, unk2);
		return false;
	}

	// If the size of the image has changed, resize the buffers
	if ((width != _currBuf->w) || (height != _currBuf->h)) {
		// Calculate the maximum scale that fits the screen
		_scaleX = MIN(_syst->getWidth() / width, 2);
		_scaleY = MIN(_syst->getHeight() / height, 2);

		// Free the previous surfaces
		_currBuf->free();
		_prevBuf->free();
		_showBuf.free();

		// Allocate new buffers
		_currBuf->create(width, height, 3);
		_prevBuf->create(width, height, 3);
		_showBuf.create(width * _scaleX, height * _scaleY, _vm->_pixelFormat.bytesPerPixel);

		// Clear the buffers with black YUV values
		byte *ptr1 = (byte *)_currBuf->getBasePtr(0, 0);
		byte *ptr2 = (byte *)_prevBuf->getBasePtr(0, 0);
		for (int i = 0; i < width * height; i++) {
			*ptr1++ = 0;
			*ptr1++ = 128;
			*ptr1++ = 128;
			*ptr2++ = 0;
			*ptr2++ = 128;
			*ptr2++ = 128;
		}

#ifdef DITHER
		// Reset the dithering algorithm with the new width
		delete _dither;
		_dither = new Graphics::SierraLight(width * _scaleX, _paletteLookup);
#endif
	}

	return true;
}

bool ROQPlayer::processBlockQuadCodebook(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing quad codebook block");

	// Get the number of 2x2 pixel blocks to read
	int newNum2blocks = blockHeader.param >> 8;
	if (newNum2blocks == 0) {
		newNum2blocks = 256;
	}
	if (newNum2blocks > _num2blocks)
		_num2blocks = newNum2blocks;

	// Get the number of 4x4 pixel blocks
	_num4blocks = blockHeader.param & 0xFF;
	if ((_num4blocks == 0) && (blockHeader.size > (uint32)_num2blocks * (6 + _alpha * 4))) {
		_num4blocks = 256;
	}

	// Read the 2x2 codebook
	for (int i = 0; i < newNum2blocks; i++) {
		// Read the 4 Y components and their alpha channel
		for (int j = 0; j < 4; j++) {
			_codebook2[i * 10 + j * 2] = _file->readByte();
			_codebook2[i * 10 + j * 2 + 1] = _alpha ? _file->readByte() : 255;
		}

		// Read the subsampled Cb and Cr
		_file->read(&_codebook2[i * 10 + 8], 2);
	}

	// Read the 4x4 codebook
	_file->read(_codebook4, _num4blocks * 4);

	return true;
}

bool ROQPlayer::processBlockQuadVector(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing quad vector block");

	// Get the mean motion vectors
	int8 Mx = blockHeader.param >> 8;
	int8 My = blockHeader.param & 0xFF;

	// Calculate where the block should end
	int32 endpos =_file->pos() + blockHeader.size;

	// Reset the coding types
	_codingTypeCount = 0;

	// Traverse the image in 16x16 macroblocks
	for (int macroY = 0; macroY < _currBuf->h; macroY += 16) {
		for (int macroX = 0; macroX < _currBuf->w; macroX += 16) {
			// Traverse the macroblock in 8x8 blocks
			for (int blockY = 0; blockY < 16; blockY += 8) {
				for (int blockX = 0; blockX < 16; blockX += 8) {
					processBlockQuadVectorBlock(macroX + blockX, macroY + blockY, Mx, My);
				}
			}
		}
	}

	// HACK: Skip the remaining bytes
	int32 skipBytes = endpos -_file->pos();
	if (skipBytes > 0) {
		_file->skip(skipBytes);
		if (skipBytes != 2) {
			warning("Groovie::ROQ: Skipped %d bytes", skipBytes);
		}
	}
	return true;
}

void ROQPlayer::processBlockQuadVectorBlock(int baseX, int baseY, int8 Mx, int8 My) {
	uint16 codingType = getCodingType();
	switch (codingType) {
	case 0: // MOT: Skip block
		break;
	case 1: { // FCC: Copy an existing block
		byte argument = _file->readByte();
		int16 DDx = 8 - (argument >> 4);
		int16 DDy = 8 - (argument & 0x0F);
		copy(8, baseX, baseY, DDx - Mx, DDy - My);
		break;
	}
	case 2: // SLD: Quad vector quantisation
		// Upsample the 4x4 pixel block
		paint8(_file->readByte(), baseX, baseY);
		break;
	case 3: // CCC:
		// Traverse the block in 4x4 sub-blocks
		for (int subBlockY = 0; subBlockY < 8; subBlockY += 4) {
			for (int subBlockX = 0; subBlockX < 8; subBlockX += 4) {
				processBlockQuadVectorBlockSub(baseX + subBlockX, baseY + subBlockY, Mx, My);
			}
		}
		break;
	}
}

void ROQPlayer::processBlockQuadVectorBlockSub(int baseX, int baseY, int8 Mx, int8 My) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing quad vector sub block");

	uint16 codingType = getCodingType();
	switch (codingType) {
	case 0: // MOT: Skip block
		break;
	case 1: { // FCC: Copy an existing block
		byte argument = _file->readByte();
		int16 DDx = 8 - (argument >> 4);
		int16 DDy = 8 - (argument & 0x0F);
		copy(4, baseX, baseY, DDx - Mx, DDy - My);
		break;
	}
	case 2: // SLD: Quad vector quantisation
		paint4(_file->readByte(), baseX, baseY);
		break;
	case 3:
		paint2(_file->readByte(), baseX    , baseY);
		paint2(_file->readByte(), baseX + 2, baseY);
		paint2(_file->readByte(), baseX    , baseY + 2);
		paint2(_file->readByte(), baseX + 2, baseY + 2);
		break;
	}
}

bool ROQPlayer::processBlockStill(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing still (JPEG) block");

	warning("Groovie::ROQ: JPEG frame (unfinshed)");

	Graphics::JPEG *jpg = new Graphics::JPEG();
	jpg->read(_file);
	byte *y = (byte *)jpg->getComponent(1)->getBasePtr(0, 0);
	byte *u = (byte *)jpg->getComponent(2)->getBasePtr(0, 0);
	byte *v = (byte *)jpg->getComponent(3)->getBasePtr(0, 0);

	byte *ptr = (byte *)_prevBuf->getBasePtr(0, 0);
	for (int i = 0; i < _prevBuf->w * _prevBuf->h; i++) {
		*ptr++ = *y++;
		*ptr++ = *u++;
		*ptr++ = *v++;
	}

	return true;
}

bool ROQPlayer::processBlockSoundMono(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing mono sound block");

	// Verify the block header
	if (blockHeader.type != 0x1020) {
		return false;
	}

	// Initialize the audio stream if needed
	if (!_audioStream) {
		byte flags = Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_AUTOFREE;
#ifdef SCUMM_LITTLE_ENDIAN
		flags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
#endif
		_audioStream = Audio::makeAppendableAudioStream(22050, flags);
		Audio::SoundHandle sound_handle;
		g_system->getMixer()->playInputStream(Audio::Mixer::kPlainSoundType, &sound_handle, _audioStream);
	}

	// Create the audio buffer
	int16 *buffer = new int16[blockHeader.size];

	// Initialize the prediction with the block parameter
	int16 prediction = blockHeader.param ^ 0x8000;

	// Process the data
	for (uint16 i = 0; i < blockHeader.size; i++) {
		int16 data = _file->readByte();
		if (data < 0x80) {
			prediction += data * data;
		} else {
			data -= 0x80;
			prediction -= data * data;
		}
		buffer[i] = prediction;
	}

	// Queue the read buffer
	_audioStream->queueBuffer((byte *)buffer, blockHeader.size * 2);

	return true;
}

bool ROQPlayer::processBlockSoundStereo(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing stereo sound block");

	// Verify the block header
	if (blockHeader.type != 0x1021) {
		return false;
	}

	// Initialize the audio stream if needed
	if (!_audioStream) {
		byte flags = Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_STEREO;
#ifdef SCUMM_LITTLE_ENDIAN
		flags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
#endif
		_audioStream = Audio::makeAppendableAudioStream(22050, flags);
		Audio::SoundHandle sound_handle;
		g_system->getMixer()->playInputStream(Audio::Mixer::kPlainSoundType, &sound_handle, _audioStream);
	}

	// Create the audio buffer
	int16 *buffer = new int16[blockHeader.size];

	// Initialize the prediction with the block parameter
	int16 predictionLeft = (blockHeader.param & 0xFF00) ^ 0x8000;
	int16 predictionRight = (blockHeader.param << 8) ^ 0x8000;
	bool left = true;

	// Process the data
	for (uint16 i = 0; i < blockHeader.size; i++) {
		int16 data = _file->readByte();
		if (left) {
			if (data < 0x80) {
				predictionLeft += data * data;
			} else {
				data -= 0x80;
				predictionLeft -= data * data;
			}
			buffer[i] = predictionLeft;
		} else {
			if (data < 0x80) {
				predictionRight += data * data;
			} else {
				data -= 0x80;
				predictionRight -= data * data;
			}
			buffer[i] = predictionRight;
		}
		left = !left;
	}

	// Queue the read buffer
	_audioStream->queueBuffer((byte *)buffer, blockHeader.size * 2);

	return true;
}

bool ROQPlayer::processBlockAudioContainer(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing audio container block: 0x%04X", blockHeader.param);
	return true;
}

byte ROQPlayer::getCodingType() {
	_codingType <<= 2;
	if (!_codingTypeCount) {
		_codingType = _file->readUint16LE();
		_codingTypeCount = 8;
	}

	_codingTypeCount--;
	return (_codingType >> 14);
}

void ROQPlayer::paint2(byte i, int destx, int desty) {
	if (i > _num2blocks) {
		error("Groovie::ROQ: Invalid 2x2 block %d (%d available)", i, _num2blocks);
	}

	byte *block = &_codebook2[i * 10];
	byte u = block[8];
	byte v = block[9];

	byte *ptr = (byte *)_currBuf->getBasePtr(destx, desty);
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			// Basic alpha test
			// TODO: Blending
			if (*(block + 1) > 128) {
				*ptr = *block;
				*(ptr + 1) = u;
				*(ptr + 2) = v;
			}
			ptr += 3;
			block += 2;
		}
		ptr += _currBuf->pitch - 6;
	}
}

void ROQPlayer::paint4(byte i, int destx, int desty) {
	if (i > _num4blocks) {
		error("Groovie::ROQ: Invalid 4x4 block %d (%d available)", i, _num4blocks);
	}

	byte *block4 = &_codebook4[i * 4];
	for (int origy = 0; origy < 4; origy += 2) {
		for (int origx = 0; origx < 4; origx += 2) {
			paint2(*block4, destx + origx, desty + origy);
			block4++;
		}
	}
}

void ROQPlayer::paint8(byte i, int destx, int desty) {
	if (i > _num4blocks) {
		error("Groovie::ROQ: Invalid 4x4 block %d (%d available)", i, _num4blocks);
	}

	byte *block4 = &_codebook4[i * 4];
	for (int y4 = 0; y4 < 2; y4++) {
		for (int x4 = 0; x4 < 2; x4++) {
			byte *block2 = &_codebook2[(*block4) * 10];
			byte u = block2[8];
			byte v = block2[9];
			block4++;
			for (int y2 = 0; y2 < 2; y2++) {
				for (int x2 = 0; x2 < 2; x2++) {
					for (int repy = 0; repy < 2; repy++) {
						for (int repx = 0; repx < 2; repx++) {
							// Basic alpha test
							// TODO: Blending
							if (*(block2 + 1) > 128) {
								byte *ptr = (byte *)_currBuf->getBasePtr(destx + x4*4 + x2*2 + repx, desty + y4*4 + y2*2 + repy);
								*ptr = *block2;
								*(ptr + 1) = u;
								*(ptr + 2) = v;
							}
						}
					}
					block2 += 2;
				}
			}
		}
	}
}

void ROQPlayer::copy(byte size, int destx, int desty, int offx, int offy) {
	offx *= _offScale / _scaleX;
	offy *= _offScale / _scaleY;

	// Get the beginning of the first line
	byte *dst = (byte *)_currBuf->getBasePtr(destx, desty);
	byte *src = (byte *)_prevBuf->getBasePtr(destx + offx, desty + offy);

	for (int i = 0; i < size; i++) {
		// Copy the current line
		memcpy(dst, src, size * _currBuf->bytesPerPixel);

		// Move to the beginning of the next line
		dst += _currBuf->pitch;
		src += _currBuf->pitch;
	}
}

} // End of Groovie namespace
