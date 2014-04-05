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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// libjpeg uses forbidden symbols in its header. Thus, we need to allow them
// here.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "image/jpeg.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"

#ifdef USE_JPEG
// The original release of libjpeg v6b did not contain any extern "C" in case
// its header files are included in a C++ environment. To avoid any linking
// issues we need to add it on our own.
extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}
#endif

namespace Image {

JPEGDecoder::JPEGDecoder() : _surface(), _colorSpace(kColorSpaceRGBA) {
}

JPEGDecoder::~JPEGDecoder() {
	destroy();
}

const Graphics::Surface *JPEGDecoder::getSurface() const {
	return &_surface;
}

void JPEGDecoder::destroy() {
	_surface.free();
}

const Graphics::Surface *JPEGDecoder::decodeFrame(Common::SeekableReadStream &stream) {
	if (!loadStream(stream))
		return 0;

	return getSurface();
}

Graphics::PixelFormat JPEGDecoder::getPixelFormat() const {
	return _surface.format;
}

#ifdef USE_JPEG
namespace {

#define JPEG_BUFFER_SIZE 4096

struct StreamSource : public jpeg_source_mgr {
	Common::SeekableReadStream *stream;
	bool startOfFile;
	JOCTET buffer[JPEG_BUFFER_SIZE];
};

void initSource(j_decompress_ptr cinfo) {
	StreamSource *source = (StreamSource *)cinfo->src;
	source->startOfFile = true;
}

boolean fillInputBuffer(j_decompress_ptr cinfo) {
	StreamSource *source = (StreamSource *)cinfo->src;

	uint32 bufferSize = source->stream->read((byte *)source->buffer, sizeof(source->buffer));
	if (bufferSize == 0) {
		if (source->startOfFile) {
			// An empty file is a fatal error
			ERREXIT(cinfo, JERR_INPUT_EMPTY);
		} else {
			// Otherwise we insert an EOF marker
			WARNMS(cinfo, JWRN_JPEG_EOF);
			source->buffer[0] = (JOCTET)0xFF;
			source->buffer[1] = (JOCTET)JPEG_EOI;
			bufferSize = 2;
		}
	}

	source->next_input_byte = source->buffer;
	source->bytes_in_buffer = bufferSize;
	source->startOfFile = false;

	return TRUE;
}

void skipInputData(j_decompress_ptr cinfo, long numBytes) {
	StreamSource *source = (StreamSource *)cinfo->src;

	if (numBytes > 0) {
		if (numBytes > (long)source->bytes_in_buffer) {
			// In case we need to skip more bytes than there are in the buffer
			// we will skip the remaining data and fill the buffer again
			numBytes -= (long)source->bytes_in_buffer;

			// Skip the remaining bytes
			source->stream->skip(numBytes);

			// Fill up the buffer again
			(*source->fill_input_buffer)(cinfo);
		} else {
			source->next_input_byte += (size_t)numBytes;
			source->bytes_in_buffer -= (size_t)numBytes;
		}

	}
}

void termSource(j_decompress_ptr cinfo) {
}

void jpeg_scummvm_src(j_decompress_ptr cinfo, Common::SeekableReadStream *stream) {
	StreamSource *source;

	// Initialize the source in case it has not been done yet.
	if (cinfo->src == NULL) {
		cinfo->src = (jpeg_source_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(StreamSource));
	}

	source = (StreamSource *)cinfo->src;
	source->init_source       = &initSource;
	source->fill_input_buffer = &fillInputBuffer;
	source->skip_input_data   = &skipInputData;
	source->resync_to_restart = &jpeg_resync_to_restart;
	source->term_source       = &termSource;
	source->bytes_in_buffer   = 0;
	source->next_input_byte   = NULL;

	source->stream = stream;
}

void errorExit(j_common_ptr cinfo) {
	char buffer[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message)(cinfo, buffer);
	// This function is not allowed to return to the caller, thus we simply
	// error out with our error handling here.
	error("%s", buffer);
}

void outputMessage(j_common_ptr cinfo) {
	char buffer[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message)(cinfo, buffer);
	// Is using debug here a good idea? Or do we want to ignore all libjpeg
	// messages?
	debug(3, "libjpeg: %s", buffer);
}

} // End of anonymous namespace
#endif

bool JPEGDecoder::loadStream(Common::SeekableReadStream &stream) {
#ifdef USE_JPEG
	// Reset member variables from previous decodings
	destroy();

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;

	// Initialize error handling callbacks
	cinfo.err = jpeg_std_error(&jerr);
	cinfo.err->error_exit = &errorExit;
	cinfo.err->output_message = &outputMessage;

	// Initialize the decompression structure
	jpeg_create_decompress(&cinfo);

	// Initialize our buffer handling
	jpeg_scummvm_src(&cinfo, &stream);

	// Read the file header
	jpeg_read_header(&cinfo, TRUE);

	// We can request YUV output because Groovie requires it
	switch (_colorSpace) {
	case kColorSpaceRGBA:
		cinfo.out_color_space = JCS_RGB;
		break;

	case kColorSpaceYUV:
		cinfo.out_color_space = JCS_YCbCr;
		break;
	}

	// Actually start decompressing the image
	jpeg_start_decompress(&cinfo);

	// Allocate buffers for the output data
	switch (_colorSpace) {
	case kColorSpaceRGBA:
		// We use RGBA8888 in this scenario
		_surface.create(cinfo.output_width, cinfo.output_height, Graphics::PixelFormat(4, 8, 8, 8, 0, 24, 16, 8, 0));
		break;

	case kColorSpaceYUV:
		// We use YUV with 3 bytes per pixel otherwise.
		// This is pretty ugly since our PixelFormat cannot express YUV...
		_surface.create(cinfo.output_width, cinfo.output_height, Graphics::PixelFormat(3, 0, 0, 0, 0, 0, 0, 0, 0));
		break;
	}

	// Allocate buffer for one scanline
	assert(cinfo.output_components == 3);
	JDIMENSION pitch = cinfo.output_width * cinfo.output_components;
	assert(_surface.pitch >= pitch);
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, pitch, 1);

	// Go through the image data scanline by scanline
	while (cinfo.output_scanline < cinfo.output_height) {
		byte *dst = (byte *)_surface.getBasePtr(0, cinfo.output_scanline);

		jpeg_read_scanlines(&cinfo, buffer, 1);

		const byte *src = buffer[0];
		switch (_colorSpace) {
		case kColorSpaceRGBA: {
			for (int remaining = cinfo.output_width; remaining > 0; --remaining) {
				byte r = *src++;
				byte g = *src++;
				byte b = *src++;
				// We need to insert a alpha value of 255 (opaque) here.
#ifdef SCUMM_BIG_ENDIAN
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				*dst++ = 0xFF;
#else
				*dst++ = 0xFF;
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
#endif
			}
			} break;

		case kColorSpaceYUV:
			memcpy(dst, src, pitch);
			break;
		}
	}

	// We are done with decompressing, thus free all the data
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return true;
#else
	return false;
#endif
}

} // End of Graphics namespace
