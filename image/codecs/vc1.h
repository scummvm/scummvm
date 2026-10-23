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

/*
 * WMV3 and VC-1 software video decoder.
 *
 * This decoder is a ScummVM port of the FFmpeg WMV3/VC-1 decoder path
 * used by libavcodec revision c23123630e6a7e645c199599b8ade3fe7e9ab3db:
 * https://git.ffmpeg.org/gitweb/ffmpeg.git/commit/c23123630e6a7e645c199599b8ade3fe7e9ab3db
 * The original code was Copyright
 * (c) 2005 Anonymous, Alex Beregszaszi and Michael Niedermayer,
 * (c) 2006-2007 Konstantin Shishkov,
 * (c) 2011 Mashiat Sarker Shakkhar and distributed under the GNU LGPL
 * version 2.1 or later. The migrated path includes sequence and picture
 * headers, bitplanes, VLC tables, prediction, motion compensation, inverse
 * transforms, overlap and loop filtering, IntraX8, error concealment,
 * reference-frame ordering, and YUV420 output.
 */

#ifndef IMAGE_CODECS_VC1_H
#define IMAGE_CODECS_VC1_H

#include "common/bitstream.h"
#include "common/scummsys.h"
#include "image/codecs/codec.h"

namespace Image {

struct VC1Ratio {
	int num;
	int den;
	constexpr VC1Ratio(int numerator = 0, int denominator = 1) : num(numerator), den(denominator) {}
};

struct VC1Header {
	int bitrtq_postproc = 0;
	int broadcast = 0;
	int broken_link = 0;
	int chromaformat = 0;
	int closed_entry = 0;
	int color_prim = 0;
	int dquant = 0;
	int extended_dmv = 0;
	int extended_mv = 0;
	int fastuvmc = 0;
	int finterpflag = 0;
	int frmrtq_postproc = 0;
	int height = 0;
	int hrd_num_leaky_buckets = 0;
	int hrd_param_flag = 0;
	int interlace = 0;
	int level = 0;
	int loop_filter = 0;
	int matrix_coef = 0;
	int max_b_frames = 0;
	int max_coded_height = 0;
	int max_coded_width = 0;
	int multires = 0;
	int overlap = 0;
	int panscanflag = 0;
	int postprocflag = 0;
	int profile = 0;
	int psf = 0;
	int quantizer_mode = 0;
	int range_mapuv = 0;
	int range_mapuv_flag = 0;
	int range_mapy = 0;
	int range_mapy_flag = 0;
	int rangered = 0;
	int refdist_flag = 0;
	int res_fasttx = 0;
	int res_rtm_flag = 0;
	int res_sprite = 0;
	int res_transtab = 0;
	int res_x8 = 0;
	int res_y411 = 0;
	int resync_marker = 0;
	int tfcntrflag = 0;
	int transfer_char = 0;
	int vstransform = 0;
	int width = 0;
	const uint8 *zz_8x4 = nullptr;
	const uint8 *zz_4x8 = nullptr;
	VC1Ratio sample_aspect_ratio;
	VC1Ratio framerate;
};

int readVC1Sequence(VC1Header *v, Common::BitStream8MSB *gb);
int readVC1Entry(VC1Header *v, Common::BitStream8MSB *gb);

class VC1Bitplane {
public:
	VC1Bitplane();
	int decode(uint8 *data, int *rawFlag, int width, int height, int stride, Common::BitStream8MSB *bits) const;

private:
	uint16 _imode[16];
	uint16 _norm2[8];
	uint16 _norm6[8192];
};

namespace VC1 {
class Decoder;
struct Picture;
} // namespace VC1
/** Packet-based WMV3/VC-1 decoder with delayed-frame output. */
class WMV3Decoder : public Codec {
public:
	WMV3Decoder(int width, int height, bool advanced = false);
	~WMV3Decoder() override;
	/** Supply container initialization data before decoding packets. */
	bool setExtraData(Common::SeekableReadStream &stream);
	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	/** Return delayed output at end of stream, or nullptr when exhausted. */
	const Graphics::Surface *drainFrame();
	Graphics::PixelFormat getPixelFormat() const override { return _format; }
	bool setOutputPixelFormat(const Graphics::PixelFormat &format) override;
	bool hasError() const;

private:
	VC1::Decoder *_decoder;
	Graphics::Surface _surface;
	Graphics::PixelFormat _format;
	const Graphics::Surface *convert(const VC1::Picture *picture);
};

} // namespace Image

#endif // IMAGE_CODECS_VC1_H
