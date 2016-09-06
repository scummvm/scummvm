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

#include "common/scummsys.h"

/* Intel Indeo 4 decompressor, derived from ffmpeg.
 *
 * Original copyright note: * Intel Indeo 3 (IV41, IV42, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#include "common/system.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/yuv_to_rgb.h"
#include "image/codecs/indeo4.h"

namespace Image {

#define IVI4_PIC_SIZE_ESC   7

Indeo4Decoder::Indeo4Decoder(uint16 width, uint16 height) {
	_pixelFormat = g_system->getScreenFormat();
	_surface = new Graphics::ManagedSurface();
	_surface->create(width, height, _pixelFormat);
	_ctx.gb = nullptr;
	_ctx.pic_conf.pic_width = _ctx.pic_conf.pic_height = 0;
	_ctx.is_indeo4 = true;
	_ctx.show_indeo4_info = false;
	_ctx.b_ref_buf = 3; // buffer 2 is used for scalability mode
}

Indeo4Decoder::~Indeo4Decoder() {
	delete _surface;
}

bool Indeo4Decoder::isIndeo4(Common::SeekableReadStream &stream) {
	// Less than 16 bytes? This can't be right
	if (stream.size() < 16)
		return false;

	// Read in the start of the data
	byte buffer[16];
	stream.read(buffer, 16);
	stream.seek(-16, SEEK_CUR);

	// Validate the first 18-bit word has the correct identifier
	Indeo::GetBits gb(buffer, 16 * 8);
	bool isIndeo4 = gb.getBits(18) == 0x3FFF8;

	return isIndeo4;
}

const Graphics::Surface *Indeo4Decoder::decodeFrame(Common::SeekableReadStream &stream) {
	// Not Indeo 4? Fail
	if (!isIndeo4(stream))
		return nullptr;

	// Set up the GetBits instance for reading the stream
	_ctx.gb = new GetBits(stream);

	// Decode the header
	int err = decodePictureHeader();

	delete _ctx.gb;
	_ctx.gb = nullptr;

	// TODO
	err = -1;
	return (err < 0) ? nullptr : &_surface->rawSurface();
}

int Indeo4Decoder::decodePictureHeader() {
	int             pic_size_indx, i, p;
	IVIPicConfig    pic_conf;

	if (_ctx.gb->getBits(18) != 0x3FFF8) {
		warning("Invalid picture start code!");
		return -1;
	}

	_ctx.prev_frame_type = _ctx.frame_type;
	_ctx.frame_type = _ctx.gb->getBits(3);
	if (_ctx.frame_type == 7) {
		warning("Invalid frame type: %d", _ctx.frame_type);
		return -1;
	}

	if (_ctx.frame_type == IVI4_FRAMETYPE_BIDIR)
		_ctx.has_b_frames = 1;

	_ctx.has_transp = _ctx.gb->getBits1();

	// unknown bit: Mac decoder ignores this bit, XANIM returns error
	if (_ctx.gb->getBits1()) {
		warning("Sync bit is set!");
		return -1;
	}

	_ctx.data_size = _ctx.gb->getBits1() ? _ctx.gb->getBits(24) : 0;

	// null frames don't contain anything else so we just return
	if (_ctx.frame_type >= IVI4_FRAMETYPE_NULL_FIRST) {
		warning("Null frame encountered!");
		return 0;
	}

	// Check key lock status. If enabled - ignore lock word.
	// Usually we have to prompt the user for the password, but
	// we don't do that because Indeo 4 videos can be decoded anyway
	if (_ctx.gb->getBits1()) {
		_ctx.gb->skipBitsLong(32);
		warning("Password-protected clip!");
	}

	pic_size_indx = _ctx.gb->getBits(3);
	if (pic_size_indx == IVI4_PIC_SIZE_ESC) {
		pic_conf.pic_height = _ctx.gb->getBits(16);
		pic_conf.pic_width = _ctx.gb->getBits(16);
	} else {
		pic_conf.pic_height = _ivi4_common_pic_sizes[pic_size_indx * 2 + 1];
		pic_conf.pic_width = _ivi4_common_pic_sizes[pic_size_indx * 2];
	}

	// Decode tile dimensions.
	_ctx.uses_tiling = _ctx.gb->getBits1();
	if (_ctx.uses_tiling) {
		pic_conf.tile_height = scaleTileSize(pic_conf.pic_height, _ctx.gb->getBits(4));
		pic_conf.tile_width = scaleTileSize(pic_conf.pic_width, _ctx.gb->getBits(4));
	} else {
		pic_conf.tile_height = pic_conf.pic_height;
		pic_conf.tile_width = pic_conf.pic_width;
	}

	// Decode chroma subsampling. We support only 4:4 aka YVU9.
	if (_ctx.gb->getBits(2)) {
		warning("Only YVU9 picture format is supported!");
		return -1;
	}
	pic_conf.chroma_height = (pic_conf.pic_height + 3) >> 2;
	pic_conf.chroma_width = (pic_conf.pic_width + 3) >> 2;

	// decode subdivision of the planes
	pic_conf.luma_bands = decodePlaneSubdivision();
	pic_conf.chroma_bands = 0;
	if (pic_conf.luma_bands)
		pic_conf.chroma_bands = decodePlaneSubdivision();
	_ctx.is_scalable = pic_conf.luma_bands != 1 || pic_conf.chroma_bands != 1;
	if (_ctx.is_scalable && (pic_conf.luma_bands != 4 || pic_conf.chroma_bands != 1)) {
		warning("Scalability: unsupported subdivision! Luma bands: %d, chroma bands: %d",
			pic_conf.luma_bands, pic_conf.chroma_bands);
		return -1;
	}

	// check if picture layout was changed and reallocate buffers
	if (pic_conf.ivi_pic_config_cmp(_ctx.pic_conf)) {
		if (IVIPlaneDesc::ff_ivi_init_planes(_ctx.planes, &pic_conf, 1)) {
			warning("Couldn't reallocate color planes!");
			_ctx.pic_conf.luma_bands = 0;
			return -2;
		}

		_ctx.pic_conf = pic_conf;

		// set default macroblock/block dimensions
		for (p = 0; p <= 2; p++) {
			for (i = 0; i < (!p ? pic_conf.luma_bands : pic_conf.chroma_bands); i++) {
				_ctx.planes[p].bands[i].mb_size = !p ? (!_ctx.is_scalable ? 16 : 8) : 4;
				_ctx.planes[p].bands[i].blk_size = !p ? 8 : 4;
			}
		}

		if (IVIPlaneDesc::ff_ivi_init_tiles(_ctx.planes, _ctx.pic_conf.tile_width,
			_ctx.pic_conf.tile_height)) {
			warning("Couldn't reallocate internal structures!");
			return -2;
		}
	}

	_ctx.frame_num = _ctx.gb->getBits1() ? _ctx.gb->getBits(20) : 0;

	// skip decTimeEst field if present
	if (_ctx.gb->getBits1())
		_ctx.gb->skipBits(8);

	// decode macroblock and block huffman codebooks
	if (_ctx.mb_vlc.ff_ivi_dec_huff_desc(_ctx.gb, _ctx.gb->getBits1(), IVI_MB_HUFF) ||
		_ctx.blk_vlc.ff_ivi_dec_huff_desc(_ctx.gb, _ctx.gb->getBits1(), IVI_BLK_HUFF))
		return -1;

	_ctx.rvmap_sel = _ctx.gb->getBits1() ? _ctx.gb->getBits(3) : 8;

	_ctx.in_imf = _ctx.gb->getBits1();
	_ctx.in_q = _ctx.gb->getBits1();

	_ctx.pic_glob_quant = _ctx.gb->getBits(5);

	// TODO: ignore this parameter if unused
	_ctx.unknown1 = _ctx.gb->getBits1() ? _ctx.gb->getBits(3) : 0;

	_ctx.checksum = _ctx.gb->getBits1() ? _ctx.gb->getBits(16) : 0;

	// skip picture header extension if any
	while (_ctx.gb->getBits1()) {
		warning("Pic hdr extension encountered!");
		_ctx.gb->skipBits(8);
	}

	if (_ctx.gb->getBits1()) {
		warning("Bad blocks bits encountered!");
	}

	_ctx.gb->alignGetBits();

	return 0;
}

int Indeo4Decoder::scaleTileSize(int def_size, int size_factor) {
	return size_factor == 15 ? def_size : (size_factor + 1) << 5;
}

int Indeo4Decoder::decodePlaneSubdivision() {
	int i;

	switch (_ctx.gb->getBits(2)) {
	case 3:
		return 1;
	case 2:
		for (i = 0; i < 4; i++)
			if (_ctx.gb->getBits(2) != 3)
				return 0;
		return 4;
	default:
		return 0;
	}
}

/*------------------------------------------------------------------------*/

const uint Indeo4Decoder::_ivi4_common_pic_sizes[14] = {
	640, 480, 320, 240, 160, 120, 704, 480, 352, 240, 352, 288, 176, 144
};

} // End of namespace Image
