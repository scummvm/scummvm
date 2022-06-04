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

/* Common structures, macros, and base class shared by both Indeo4 and
 * Indeo5 decoders, derived from ffmpeg.
 */

#include "image/codecs/indeo/indeo.h"
#include "image/codecs/indeo/indeo_dsp.h"
#include "image/codecs/indeo/mem.h"
#include "graphics/yuv_to_rgb.h"
#include "common/system.h"
#include "common/algorithm.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Image {
namespace Indeo {

/**
 * These are 2x8 predefined Huffman codebooks for coding macroblock/block
 * signals. They are specified using "huffman descriptors" in order to
 * avoid huge static tables. The decoding tables will be generated at
 * startup from these descriptors.
 */

/**
 * Static macroblock huffman tables
 */
static const IVIHuffDesc ivi_mb_huff_desc[8] = {
	{8,  {0, 4, 5, 4, 4, 4, 6, 6}},
	{12, {0, 2, 2, 3, 3, 3, 3, 5, 3, 2, 2, 2}},
	{12, {0, 2, 3, 4, 3, 3, 3, 3, 4, 3, 2, 2}},
	{12, {0, 3, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2}},
	{13, {0, 4, 4, 3, 3, 3, 3, 2, 3, 3, 2, 1, 1}},
	{9,  {0, 4, 4, 4, 4, 3, 3, 3, 2}},
	{10, {0, 4, 4, 4, 4, 3, 3, 2, 2, 2}},
	{12, {0, 4, 4, 4, 3, 3, 2, 3, 2, 2, 2, 2}}
};

/**
 * static block huffman tables
 */
static const IVIHuffDesc ivi_blk_huff_desc[8] = {
	{10, {1, 2, 3, 4, 4, 7, 5, 5, 4, 1} },
	{11, {2, 3, 4, 4, 4, 7, 5, 4, 3, 3, 2} },
	{12, {2, 4, 5, 5, 5, 5, 6, 4, 4, 3, 1, 1} },
	{13, {3, 3, 4, 4, 5, 6, 6, 4, 4, 3, 2, 1, 1} },
	{11, {3, 4, 4, 5, 5, 5, 6, 5, 4, 2, 2} },
	{13, {3, 4, 5, 5, 5, 5, 6, 4, 3, 3, 2, 1, 1} },
	{13, {3, 4, 5, 5, 5, 6, 5, 4, 3, 3, 2, 1, 1} },
	{9,  {3, 4, 4, 5, 5, 5, 6, 5, 5} }
};

/*------------------------------------------------------------------------*/

/**
 * calculate number of _tiles in a stride
 */
#define IVI_NUM_TILES(stride, tileSize) (((stride) + (tileSize) - 1) / (tileSize))

/*------------------------------------------------------------------------*/

int IVIHuffDesc::createHuffFromDesc(VLC *vlc, bool flag) const {
	uint16 codewords[256];
	uint8 bits[256];

	int pos = 0; // current position = 0

	for (int i = 0; i < _numRows; i++) {
		int codesPerRow = 1 << _xBits[i];
		int notLastRow  = (i != _numRows - 1);
		int prefix      = ((1 << i) - 1) << (_xBits[i] + notLastRow);

		for (int j = 0; j < codesPerRow; j++) {
			if (pos >= 256) // Some Indeo5 codebooks can have more than 256
				break;      // elements, but only 256 codes are allowed!

			bits[pos] = i + _xBits[i] + notLastRow;
			if (bits[pos] > IVI_VLC_BITS)
				return -1; // invalid descriptor

			codewords[pos] = invertBits((prefix | j), bits[pos]);
			if (!bits[pos])
				bits[pos] = 1;

			pos++;
		}//for j
	}//for i

	// number of codewords = pos
	return vlc->init_vlc(IVI_VLC_BITS, pos, bits, 1, 1, codewords, 2, 2,
					(flag ? INIT_VLC_USE_NEW_STATIC : 0) | INIT_VLC_LE);
}

/*------------------------------------------------------------------------*/

bool IVIHuffDesc::huffDescCompare(const IVIHuffDesc *desc2) const {
	return _numRows != desc2->_numRows || memcmp(_xBits, desc2->_xBits, _numRows);
}

void IVIHuffDesc::huffDescCopy(const IVIHuffDesc *src) {
	_numRows = src->_numRows;
	memcpy(_xBits, src->_xBits, src->_numRows);
}

/*------------------------------------------------------------------------*/

IVIHuffTab::IVIHuffTab() : _tab(nullptr) {
	_custDesc._numRows = 0;
	Common::fill(&_custDesc._xBits[0], &_custDesc._xBits[16], 0);
}

int IVIHuffTab::decodeHuffDesc(IVI45DecContext *ctx, int descCoded, int whichTab) {
	IVIHuffDesc newHuff;

	if (!descCoded) {
		// select default table
		_tab = (whichTab) ? &ctx->_iviBlkVlcTabs[7]
			: &ctx->_iviMbVlcTabs[7];
		return 0;
	}

	_tabSel = ctx->_gb->getBits<3>();
	if (_tabSel == 7) {
		// custom huffman table (explicitly encoded)
		newHuff._numRows = ctx->_gb->getBits<4>();
		if (!newHuff._numRows) {
			warning("Empty custom Huffman table!");
			return -1;
		}

		for (int i = 0; i < newHuff._numRows; i++)
			newHuff._xBits[i] = ctx->_gb->getBits<4>();

		// Have we got the same custom table? Rebuild if not.
		if (newHuff.huffDescCompare(&_custDesc) || !_custTab._table) {
			_custDesc.huffDescCopy(&newHuff);

			if (_custTab._table)
				_custTab.freeVlc();
			int result = _custDesc.createHuffFromDesc(&_custTab, false);
			if (result) {
				// reset faulty description
				_custDesc._numRows = 0;
				warning("Error while initializing custom vlc table!");
				return result;
			}
		}
		_tab = &_custTab;
	} else {
		// select one of predefined tables
		_tab = (whichTab) ? &ctx->_iviBlkVlcTabs[_tabSel]
			: &ctx->_iviMbVlcTabs[_tabSel];
	}

	return 0;
}

/*------------------------------------------------------------------------*/

IVIMbInfo::IVIMbInfo() : _xPos(0), _yPos(0), _bufOffs(0), _type(0), _cbp(0),
		_qDelta(0), _mvX(0), _mvY(0), _bMvX(0), _bMvY(0) {
}

/*------------------------------------------------------------------------*/

IVITile::IVITile() : _xPos(0), _yPos(0), _width(0), _height(0), _mbSize(0),
		_isEmpty(false), _dataSize(0), _numMBs(0), _mbs(nullptr), _refMbs(nullptr) {
}

/*------------------------------------------------------------------------*/

IVIBandDesc::IVIBandDesc() : _plane(0), _bandNum(0), _width(0), _height(0),
		_aHeight(0), _dataPtr(nullptr), _dataSize(0), _buf(nullptr),
		_refBuf(nullptr), _bRefBuf(nullptr), _pitch(0), _isEmpty(false),
		_mbSize(0), _blkSize(0), _isHalfpel(false), _inheritMv(false), _bufSize(0),
		_inheritQDelta(false), _qdeltaPresent(false), _quantMat(0), _globQuant(0),
		_scan(nullptr), _scanSize(0), _numCorr(0), _rvmapSel(0), _rvMap(nullptr),
		_numTiles(0), _tiles(nullptr), _invTransform(nullptr), _transformSize(0),
		_dcTransform(nullptr), _is2dTrans(0), _checksum(0), _checksumPresent(false),
		_intraBase(nullptr), _interBase(nullptr), _intraScale(nullptr),
		_interScale(nullptr) {
	Common::fill(&_bufs[0], &_bufs[4], (int16 *)nullptr);
	Common::fill(&_corr[0], &_corr[61 * 2], 0);
}

int IVIBandDesc::initTiles(IVITile *refTile, int p, int b, int tHeight, int tWidth) {
	IVITile *tile = _tiles;

	for (int y = 0; y < _height; y += tHeight) {
		for (int x = 0; x < _width; x += tWidth) {
			tile->_xPos = x;
			tile->_yPos = y;
			tile->_mbSize = _mbSize;
			tile->_width = MIN(_width - x, tWidth);
			tile->_height = MIN(_height - y, tHeight);
			tile->_dataSize = 0;
			tile->_isEmpty = false;

			// calculate number of macroblocks
			tile->_numMBs = IVI_MBs_PER_TILE(tile->_width, tile->_height,
				_mbSize);

			avFreeP(&tile->_mbs);
			tile->_mbs = (IVIMbInfo *)calloc(tile->_numMBs, sizeof(IVIMbInfo));
			if (!tile->_mbs)
				return -2;

			tile->_refMbs = 0;
			if (p || b) {
				if (tile->_numMBs != refTile->_numMBs) {
					warning("refTile mismatch");
					return -1;
				}
				tile->_refMbs = refTile->_mbs;
				refTile++;
			}
			tile++;
		}
	}

	return 0;
}

/*------------------------------------------------------------------------*/

IVIPicConfig::IVIPicConfig() : _picWidth(0), _picHeight(0), _chromaWidth(0),
		_chromaHeight(0), _tileWidth(0), _tileHeight(0), _lumaBands(0), _chromaBands(0) {
}

bool IVIPicConfig::ivi_pic_config_cmp(const IVIPicConfig &cfg2) {
	return _picWidth != cfg2._picWidth || _picHeight != cfg2._picHeight ||
		_chromaWidth != cfg2._chromaWidth || _chromaHeight != cfg2._chromaHeight ||
		_tileWidth != cfg2._tileWidth || _tileHeight != cfg2._tileHeight ||
		_lumaBands != cfg2._lumaBands || _chromaBands != cfg2._chromaBands;
}

/*------------------------------------------------------------------------*/

IVIPlaneDesc::IVIPlaneDesc() : _width(0), _height(0), _numBands(0), _bands(nullptr) {
}

int IVIPlaneDesc::initPlanes(IVIPlaneDesc *planes, const IVIPicConfig *cfg, bool isIndeo4) {
	uint32 b_width, b_height, align_fac, width_aligned, height_aligned, bufSize;
	IVIBandDesc *band;

	freeBuffers(planes);

	if (checkImageSize(cfg->_picWidth, cfg->_picHeight, 0) < 0 ||
		cfg->_lumaBands < 1 || cfg->_chromaBands < 1)
		return -1;

	// fill in the descriptor of the luminance _plane
	planes[0]._width = cfg->_picWidth;
	planes[0]._height = cfg->_picHeight;
	planes[0]._numBands = cfg->_lumaBands;

	// fill in the descriptors of the chrominance planes
	planes[1]._width = planes[2]._width = (cfg->_picWidth + 3) >> 2;
	planes[1]._height = planes[2]._height = (cfg->_picHeight + 3) >> 2;
	planes[1]._numBands = planes[2]._numBands = cfg->_chromaBands;

	for (int p = 0; p < 3; p++) {
		planes[p]._bands = (IVIBandDesc *)calloc(planes[p]._numBands, sizeof(IVIBandDesc));
		if (!planes[p]._bands)
			return -2;

		// select band dimensions: if there is only one band then it
		// has the full size, if there are several bands each of them
		// has only half size
		b_width = planes[p]._numBands == 1 ? planes[p]._width
			: (planes[p]._width + 1) >> 1;
		b_height = planes[p]._numBands == 1 ? planes[p]._height
			: (planes[p]._height + 1) >> 1;

		// luma   band buffers will be aligned on 16x16 (max macroblock size)
		// chroma band buffers will be aligned on   8x8 (max macroblock size)
		align_fac = p ? 8 : 16;
		width_aligned = FFALIGN(b_width, align_fac);
		height_aligned = FFALIGN(b_height, align_fac);
		bufSize = width_aligned * height_aligned * sizeof(int16);

		for (int b = 0; b < planes[p]._numBands; b++) {
			band = &planes[p]._bands[b]; // select appropriate _plane/band
			band->_plane = p;
			band->_bandNum = b;
			band->_width = b_width;
			band->_height = b_height;
			band->_pitch = width_aligned;
			band->_aHeight = height_aligned;
			band->_bufs[0] = (int16 *)calloc(bufSize, 1);
			band->_bufs[1] = (int16 *)calloc(bufSize, 1);
			band->_bufSize = bufSize / 2;
			if (!band->_bufs[0] || !band->_bufs[1])
				return -2;

			// allocate the 3rd band buffer for scalability mode
			if (cfg->_lumaBands > 1) {
				band->_bufs[2] = (int16 *)calloc(bufSize, 1);
				if (!band->_bufs[2])
					return -2;
			}
			if (isIndeo4) {
				band->_bufs[3] = (int16 *)calloc(bufSize, 1);
				if (!band->_bufs[3])
					return -2;
			}
			// reset custom vlc
			planes[p]._bands[0]._blkVlc._custDesc._numRows = 0;
		}
	}

	return 0;
}

int IVIPlaneDesc::initTiles(IVIPlaneDesc *planes, int tileWidth, int tileHeight) {
	int xTiles, yTiles, tWidth, tHeight, ret;
	IVIBandDesc *band;

	for (int p = 0; p < 3; p++) {
		tWidth = !p ? tileWidth : (tileWidth + 3) >> 2;
		tHeight = !p ? tileHeight : (tileHeight + 3) >> 2;

		if (!p && planes[0]._numBands == 4) {
			tWidth >>= 1;
			tHeight >>= 1;
		}
		if (tWidth <= 0 || tHeight <= 0)
			return -3;

		for (int b = 0; b < planes[p]._numBands; b++) {
			band = &planes[p]._bands[b];
			xTiles = IVI_NUM_TILES(band->_width, tWidth);
			yTiles = IVI_NUM_TILES(band->_height, tHeight);
			band->_numTiles = xTiles * yTiles;

			avFreeP(&band->_tiles);
			band->_tiles = (IVITile *)calloc(band->_numTiles, sizeof(IVITile));
			if (!band->_tiles)
				return -2;

			// use the first luma band as reference for motion vectors
			// and quant
			ret = band->initTiles(planes[0]._bands[0]._tiles,
				p, b, tHeight, tWidth);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}

void IVIPlaneDesc::freeBuffers(IVIPlaneDesc *planes) {
	for (int p = 0; p < 3; p++) {
		if (planes[p]._bands)
			for (int b = 0; b < planes[p]._numBands; b++) {
				avFreeP(&planes[p]._bands[b]._bufs[0]);
				avFreeP(&planes[p]._bands[b]._bufs[1]);
				avFreeP(&planes[p]._bands[b]._bufs[2]);
				avFreeP(&planes[p]._bands[b]._bufs[3]);

				if (planes[p]._bands[b]._blkVlc._custTab._table)
					planes[p]._bands[b]._blkVlc._custTab.freeVlc();
				for (int t = 0; t < planes[p]._bands[b]._numTiles; t++)
					avFreeP(&planes[p]._bands[b]._tiles[t]._mbs);
				avFreeP(&planes[p]._bands[b]._tiles);
			}
		avFreeP(&planes[p]._bands);
		planes[p]._numBands = 0;
	}
}

int IVIPlaneDesc::checkImageSize(unsigned int w, unsigned int h, int log_offset) {
	if (((w + 128) * (uint64)(h + 128)) < (MAX_INTEGER / 8))
		return 0;

	error("Picture size %ux%u is invalid", w, h);
}

/*------------------------------------------------------------------------*/

AVFrame::AVFrame() {
	Common::fill(&_data[0], &_data[AV_NUM_DATA_POINTERS], (uint8 *)nullptr);
	Common::fill(&_linesize[0], &_linesize[AV_NUM_DATA_POINTERS], 0);
}

int AVFrame::setDimensions(uint16 width, uint16 height) {
	_width = width;
	_height = height;
	_linesize[0] = _linesize[1] = _linesize[2] = width;

	return 0;
}

int AVFrame::getBuffer(int flags) {
	freeFrame();

	// Luminance channel
	_data[0] = (uint8 *)calloc(_width * _height, 1);

	// UV Chroma Channels
	_data[1] = (uint8 *)malloc(_width * _height);
	_data[2] = (uint8 *)malloc(_width * _height);
	Common::fill(_data[1], _data[1] + _width * _height, 0x80);
	Common::fill(_data[2], _data[2] + _width * _height, 0x80);

	return 0;
}

void AVFrame::freeFrame() {
	avFreeP(&_data[0]);
	avFreeP(&_data[1]);
	avFreeP(&_data[2]);
}

/*------------------------------------------------------------------------*/

IVI45DecContext::IVI45DecContext() : _gb(nullptr), _frameNum(0), _frameType(0),
		_prevFrameType(0), _dataSize(0), _isScalable(0), _frameData(0),
		_interScal(0), _frameSize(0), _picHdrSize(0), _frameFlags(0),
		_checksum(0), _bufSwitch(0), _dstBuf(0), _refBuf(0), _ref2Buf(0),
		_bRefBuf(0), _rvmapSel(0), _inImf(false), _inQ(false), _picGlobQuant(0),
		_unknown1(0), _gopHdrSize(0), _gopFlags(0), _lockWord(0), _hasBFrames(false),
		_hasTransp(false), _usesTiling(false), _usesHaar(false), _usesFullpel(false),
		_gopInvalid(false), _isIndeo4(false), _transKeyColor(0), _pFrame(nullptr),
		_gotPFrame(false) {
	Common::fill(&_bufInvalid[0], &_bufInvalid[4], 0);
	Common::copy(&_ff_ivi_rvmap_tabs[0], &_ff_ivi_rvmap_tabs[9], &_rvmapTabs[0]);

	for (int idx = 0; idx < (8192 * 16); ++idx)
		_tableData[idx][0] = _tableData[idx][1] = 0;

	for (int i = 0; i < 8; i++) {
		_iviMbVlcTabs[i]._table = _tableData + i * 2 * 8192;
		_iviMbVlcTabs[i]._tableAllocated = 8192;
		ivi_mb_huff_desc[i].createHuffFromDesc(&_iviMbVlcTabs[i], true);
		_iviBlkVlcTabs[i]._table = _tableData + (i * 2 + 1) * 8192;
		_iviBlkVlcTabs[i]._tableAllocated = 8192;
		ivi_blk_huff_desc[i].createHuffFromDesc(&_iviBlkVlcTabs[i], true);
	}
}

/*------------------------------------------------------------------------*/

IndeoDecoderBase::IndeoDecoderBase(uint16 width, uint16 height, uint bitsPerPixel) : Codec() {
	_pixelFormat = g_system->getScreenFormat();

	if (_pixelFormat.bytesPerPixel == 1) {
		switch (bitsPerPixel) {
		case 15:
			_pixelFormat = Graphics::PixelFormat(2, 5, 5, 5, 0, 0, 5, 10, 0);
			break;
		case 16:
			_pixelFormat = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
			break;
		case 24:
			_pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 0, 16, 8, 0, 0);
			break;
		case 32:
			_pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
			break;
		default:
			error("Invalid color depth");
			break;
		}
	}

	_surface.create(width, height, _pixelFormat);
	_surface.fillRect(Common::Rect(0, 0, width, height), (bitsPerPixel == 32) ? 0xff : 0);
	_ctx._bRefBuf = 3; // buffer 2 is used for scalability mode
}

IndeoDecoderBase::~IndeoDecoderBase() {
	_surface.free();
	IVIPlaneDesc::freeBuffers(_ctx._planes);
	if (_ctx._mbVlc._custTab._table)
		_ctx._mbVlc._custTab.freeVlc();
	if (_ctx._transVlc._custTab._table)
		_ctx._transVlc._custTab.freeVlc();

	delete _ctx._pFrame;
}

int IndeoDecoderBase::decodeIndeoFrame() {
	int result;
	AVFrame frameData;
	AVFrame *frame = &frameData;

	// Decode the header
	if (decodePictureHeader() < 0)
		return -1;

	if (_ctx._gopInvalid)
		return -1;

	if (_ctx._frameType == IVI4_FRAMETYPE_NULL_LAST) {
		// Returning the previous frame, so exit wth success
		return 0;
	}

	if (_ctx._gopFlags & IVI5_IS_PROTECTED) {
		warning("Password-protected clip");
		return -1;
	}

	if (!_ctx._planes[0]._bands) {
		warning("Color planes not initialized yet");
		return -1;
	}

	switchBuffers();

	//{ START_TIMER;

	if (isNonNullFrame()) {
		_ctx._bufInvalid[_ctx._dstBuf] = 1;
		for (int p = 0; p < 3; p++) {
			for (int b = 0; b < _ctx._planes[p]._numBands; b++) {
				result = decode_band(&_ctx._planes[p]._bands[b]);
				if (result < 0) {
					warning("Error while decoding band: %d, _plane: %d", b, p);
					return result;
				}
			}
		}
		_ctx._bufInvalid[_ctx._dstBuf] = 0;
	} else {
		if (_ctx._isScalable)
			return -1;

		for (int p = 0; p < 3; p++) {
			if (!_ctx._planes[p]._bands[0]._buf)
				return -1;
		}
	}
	if (_ctx._bufInvalid[_ctx._dstBuf])
		return -1;

	//STOP_TIMER("decode_planes"); }

	if (!isNonNullFrame())
		return 0;

	assert(_ctx._planes[0]._width <= _surface.w && _ctx._planes[0]._height <= _surface.h);
	result = frame->setDimensions(_ctx._planes[0]._width, _ctx._planes[0]._height);
	if (result < 0)
		return result;

	if ((result = frame->getBuffer(0)) < 0)
		return result;

	if (_ctx._isScalable) {
		if (_ctx._isIndeo4)
			recomposeHaar(&_ctx._planes[0], frame->_data[0], frame->_linesize[0]);
		else
			recompose53(&_ctx._planes[0], frame->_data[0], frame->_linesize[0]);
	} else {
		outputPlane(&_ctx._planes[0], frame->_data[0], frame->_linesize[0]);
	}

	outputPlane(&_ctx._planes[2], frame->_data[1], frame->_linesize[1]);
	outputPlane(&_ctx._planes[1], frame->_data[2], frame->_linesize[2]);

	// Merge the planes into the final surface
	YUVToRGBMan.convert410(&_surface, Graphics::YUVToRGBManager::kScaleITU,
		frame->_data[0], frame->_data[1], frame->_data[2], frame->_width, frame->_height,
		frame->_width, frame->_width);

	if (_ctx._hasTransp)
		decodeTransparency();

	// If the bidirectional mode is enabled, next I and the following P
	// frame will be sent together. Unfortunately the approach below seems
	// to be the only way to handle the B-frames mode.
	// That's exactly the same Intel decoders do.
	if (_ctx._isIndeo4 && _ctx._frameType == IVI4_FRAMETYPE_INTRA) {
		// TODO: It appears from the reference decoder that this should be
		// aligning GetBits to a 32-bit boundary before reading again?

		int left;

		// skip version string
		while (_ctx._gb->getBits<8>()) {
			if (_ctx._gb->getBitsLeft() < 8)
				return -1;
		}
		left = _ctx._gb->pos() & 0x18;
		_ctx._gb->skip(64 - left);
		if (_ctx._gb->getBitsLeft() > 18 &&
			_ctx._gb->peekBits<21>() == 0xBFFF8) { // syncheader + inter _type
			error("Indeo decoder: Mode not currently implemented in ScummVM");
		}
	}

	// Free the now un-needed frame data
	frame->freeFrame();

	return 0;
}

int IndeoDecoderBase::decode_band(IVIBandDesc *band) {
	band->_buf = band->_bufs[_ctx._dstBuf];
	if (!band->_buf) {
		warning("Band buffer points to no data!");
		return -1;
	}
	if (_ctx._isIndeo4 && _ctx._frameType == IVI4_FRAMETYPE_BIDIR) {
		band->_refBuf = band->_bufs[_ctx._bRefBuf];
		band->_bRefBuf = band->_bufs[_ctx._refBuf];
	} else {
		band->_refBuf = band->_bufs[_ctx._refBuf];
		band->_bRefBuf = 0;
	}
	band->_dataPtr = _ctx._frameData + (_ctx._gb->pos() >> 3);

	int result = decodeBandHeader(band);
	if (result) {
		warning("Error while decoding band header: %d",
			result);
		return result;
	}

	if (band->_isEmpty) {
		warning("Empty band encountered!");
		return -1;
	}

	band->_rvMap = &_ctx._rvmapTabs[band->_rvmapSel];

	// apply corrections to the selected rvmap table if present
	for (int i = 0; i < band->_numCorr; i++) {
		int idx1 = band->_corr[i * 2];
		int idx2 = band->_corr[i * 2 + 1];
		SWAP(band->_rvMap->_runtab[idx1], band->_rvMap->_runtab[idx2]);
		SWAP(band->_rvMap->_valtab[idx1], band->_rvMap->_valtab[idx2]);
		if (idx1 == band->_rvMap->_eobSym || idx2 == band->_rvMap->_eobSym)
			band->_rvMap->_eobSym ^= idx1 ^ idx2;
		if (idx1 == band->_rvMap->_escSym || idx2 == band->_rvMap->_escSym)
			band->_rvMap->_escSym ^= idx1 ^ idx2;
	}

	int pos = _ctx._gb->pos();

	for (int t = 0; t < band->_numTiles; t++) {
		IVITile *tile = &band->_tiles[t];

		if (tile->_mbSize != band->_mbSize) {
			warning("MB sizes mismatch: %d vs. %d",
				band->_mbSize, tile->_mbSize);
			return -1;
		}
		tile->_isEmpty = _ctx._gb->getBit();
		if (tile->_isEmpty) {
			result = processEmptyTile(band, tile,
				(_ctx._planes[0]._bands[0]._mbSize >> 3) - (band->_mbSize >> 3));
			if (result < 0)
				break;
			warning("Empty tile encountered!");
		} else {
			tile->_dataSize = decodeTileDataSize(_ctx._gb);
			if (!tile->_dataSize) {
				warning("Tile data size is zero!");
				result = -1;
				break;
			}

			result = decodeMbInfo(band, tile);
			if (result < 0)
				break;

			result = decodeBlocks(_ctx._gb, band, tile);
			if (result < 0) {
				warning("Corrupted tile data encountered!");
				break;
			}

			if ((((int)_ctx._gb->pos() - pos) >> 3) != tile->_dataSize) {
				warning("Tile _dataSize mismatch!");
				result = -1;
				break;
			}

			pos += tile->_dataSize << 3; // skip to next tile
		}
	}

	// restore the selected rvmap table by applying its corrections in
	// reverse order
	for (int i = band->_numCorr - 1; i >= 0; i--) {
		int idx1 = band->_corr[i * 2];
		int idx2 = band->_corr[i * 2 + 1];
		SWAP(band->_rvMap->_runtab[idx1], band->_rvMap->_runtab[idx2]);
		SWAP(band->_rvMap->_valtab[idx1], band->_rvMap->_valtab[idx2]);
		if (idx1 == band->_rvMap->_eobSym || idx2 == band->_rvMap->_eobSym)
			band->_rvMap->_eobSym ^= idx1 ^ idx2;
		if (idx1 == band->_rvMap->_escSym || idx2 == band->_rvMap->_escSym)
			band->_rvMap->_escSym ^= idx1 ^ idx2;
	}

	_ctx._gb->align();

	return result;
}

void IndeoDecoderBase::recomposeHaar(const IVIPlaneDesc *_plane,
		uint8 *dst, const int dstPitch) {

	// all bands should have the same _pitch
	int32 pitch = _plane->_bands[0]._pitch;

	// get pointers to the wavelet bands
	const short *b0Ptr = _plane->_bands[0]._buf;
	const short *b1Ptr = _plane->_bands[1]._buf;
	const short *b2Ptr = _plane->_bands[2]._buf;
	const short *b3Ptr = _plane->_bands[3]._buf;

	for (int y = 0; y < _plane->_height; y += 2) {
		for (int x = 0, indx = 0; x < _plane->_width; x += 2, indx++) {
			// load coefficients
			int b0 = b0Ptr[indx]; //should be: b0 = (_numBands > 0) ? b0Ptr[indx] : 0;
			int b1 = b1Ptr[indx]; //should be: b1 = (_numBands > 1) ? b1Ptr[indx] : 0;
			int b2 = b2Ptr[indx]; //should be: b2 = (_numBands > 2) ? b2Ptr[indx] : 0;
			int b3 = b3Ptr[indx]; //should be: b3 = (_numBands > 3) ? b3Ptr[indx] : 0;

							   // haar wavelet recomposition
			int p0 = (b0 + b1 + b2 + b3 + 2) >> 2;
			int p1 = (b0 + b1 - b2 - b3 + 2) >> 2;
			int p2 = (b0 - b1 + b2 - b3 + 2) >> 2;
			int p3 = (b0 - b1 - b2 + b3 + 2) >> 2;

			// bias, convert and output four pixels
			dst[x] = avClipUint8(p0 + 128);
			dst[x + 1] = avClipUint8(p1 + 128);
			dst[dstPitch + x] = avClipUint8(p2 + 128);
			dst[dstPitch + x + 1] = avClipUint8(p3 + 128);
		}// for x

		dst += dstPitch << 1;

		b0Ptr += pitch;
		b1Ptr += pitch;
		b2Ptr += pitch;
		b3Ptr += pitch;
	}// for y
}

void IndeoDecoderBase::recompose53(const IVIPlaneDesc *_plane,
		uint8 *dst, const int dstPitch) {
	int32 p0, p1, p2, p3, tmp0, tmp1, tmp2;
	int32 b0_1, b0_2, b1_1, b1_2, b1_3, b2_1, b2_2, b2_3, b2_4, b2_5, b2_6;
	int32 b3_1, b3_2, b3_3, b3_4, b3_5, b3_6, b3_7, b3_8, b3_9;
	const int numBands = 4;

	// all bands should have the same _pitch
	int32 pitch_ = _plane->_bands[0]._pitch;

	// pixels at the position "y-1" will be set to pixels at the "y" for the 1st iteration
	int32 back_pitch = 0;

	// get pointers to the wavelet bands
	const short *b0Ptr = _plane->_bands[0]._buf;
	const short *b1Ptr = _plane->_bands[1]._buf;
	const short *b2Ptr = _plane->_bands[2]._buf;
	const short *b3Ptr = _plane->_bands[3]._buf;

	for (int y = 0; y < _plane->_height; y += 2) {

		if (y + 2 >= _plane->_height)
			pitch_ = 0;
		// load storage variables with values
		if (numBands > 0) {
			b0_1 = b0Ptr[0];
			b0_2 = b0Ptr[pitch_];
		}

		if (numBands > 1) {
			b1_1 = b1Ptr[back_pitch];
			b1_2 = b1Ptr[0];
			b1_3 = b1_1 - b1_2 * 6 + b1Ptr[pitch_];
		}

		if (numBands > 2) {
			b2_2 = b2Ptr[0];		// b2[x,  y  ]
			b2_3 = b2_2;			// b2[x+1,y  ] = b2[x,y]
			b2_5 = b2Ptr[pitch_];	// b2[x  ,y+1]
			b2_6 = b2_5;			// b2[x+1,y+1] = b2[x,y+1]
		}

		if (numBands > 3) {
			b3_2 = b3Ptr[back_pitch];	// b3[x  ,y-1]
			b3_3 = b3_2;				// b3[x+1,y-1] = b3[x  ,y-1]
			b3_5 = b3Ptr[0];			// b3[x  ,y  ]
			b3_6 = b3_5;				// b3[x+1,y  ] = b3[x  ,y  ]
			b3_8 = b3_2 - b3_5 * 6 + b3Ptr[pitch_];
			b3_9 = b3_8;
		}

		for (int x = 0, indx = 0; x < _plane->_width; x += 2, indx++) {
			if (x + 2 >= _plane->_width) {
				b0Ptr--;
				b1Ptr--;
				b2Ptr--;
				b3Ptr--;
			}

			// some values calculated in the previous iterations can
			// be reused in the next ones, so do appropriate copying
			b2_1 = b2_2; // b2[x-1,y  ] = b2[x,  y  ]
			b2_2 = b2_3; // b2[x  ,y  ] = b2[x+1,y  ]
			b2_4 = b2_5; // b2[x-1,y+1] = b2[x  ,y+1]
			b2_5 = b2_6; // b2[x  ,y+1] = b2[x+1,y+1]
			b3_1 = b3_2; // b3[x-1,y-1] = b3[x  ,y-1]
			b3_2 = b3_3; // b3[x  ,y-1] = b3[x+1,y-1]
			b3_4 = b3_5; // b3[x-1,y  ] = b3[x  ,y  ]
			b3_5 = b3_6; // b3[x  ,y  ] = b3[x+1,y  ]
			b3_7 = b3_8; // vert_HPF(x-1)
			b3_8 = b3_9; // vert_HPF(x  )

			p0 = p1 = p2 = p3 = 0;

			// process the LL-band by applying LPF both vertically and horizontally
			if (numBands > 0) {
				tmp0 = b0_1;
				tmp2 = b0_2;
				b0_1 = b0Ptr[indx + 1];
				b0_2 = b0Ptr[pitch_ + indx + 1];
				tmp1 = tmp0 + b0_1;

				p0 = tmp0 << 4;
				p1 = tmp1 << 3;
				p2 = (tmp0 + tmp2) << 3;
				p3 = (tmp1 + tmp2 + b0_2) << 2;
			}

			// process the HL-band by applying HPF vertically and LPF horizontally
			if (numBands > 1) {
				tmp0 = b1_2;
				tmp1 = b1_1;
				b1_2 = b1Ptr[indx + 1];
				b1_1 = b1Ptr[back_pitch + indx + 1];

				tmp2 = tmp1 - tmp0 * 6 + b1_3;
				b1_3 = b1_1 - b1_2 * 6 + b1Ptr[pitch_ + indx + 1];

				p0 += (tmp0 + tmp1) << 3;
				p1 += (tmp0 + tmp1 + b1_1 + b1_2) << 2;
				p2 += tmp2 << 2;
				p3 += (tmp2 + b1_3) << 1;
			}

			// process the LH-band by applying LPF vertically and HPF horizontally
			if (numBands > 2) {
				b2_3 = b2Ptr[indx + 1];
				b2_6 = b2Ptr[pitch_ + indx + 1];

				tmp0 = b2_1 + b2_2;
				tmp1 = b2_1 - b2_2 * 6 + b2_3;

				p0 += tmp0 << 3;
				p1 += tmp1 << 2;
				p2 += (tmp0 + b2_4 + b2_5) << 2;
				p3 += (tmp1 + b2_4 - b2_5 * 6 + b2_6) << 1;
			}

			// process the HH-band by applying HPF both vertically and horizontally
			if (numBands > 3) {
				b3_6 = b3Ptr[indx + 1];            // b3[x+1,y  ]
				b3_3 = b3Ptr[back_pitch + indx + 1]; // b3[x+1,y-1]

				tmp0 = b3_1 + b3_4;
				tmp1 = b3_2 + b3_5;
				tmp2 = b3_3 + b3_6;

				b3_9 = b3_3 - b3_6 * 6 + b3Ptr[pitch_ + indx + 1];

				p0 += (tmp0 + tmp1) << 2;
				p1 += (tmp0 - tmp1 * 6 + tmp2) << 1;
				p2 += (b3_7 + b3_8) << 1;
				p3 += b3_7 - b3_8 * 6 + b3_9;
			}

			// output four pixels
			dst[x] = avClipUint8((p0 >> 6) + 128);
			dst[x + 1] = avClipUint8((p1 >> 6) + 128);
			dst[dstPitch + x] = avClipUint8((p2 >> 6) + 128);
			dst[dstPitch + x + 1] = avClipUint8((p3 >> 6) + 128);
		}// for x

		dst += dstPitch << 1;

		back_pitch = -pitch_;

		b0Ptr += pitch_ + 1;
		b1Ptr += pitch_ + 1;
		b2Ptr += pitch_ + 1;
		b3Ptr += pitch_ + 1;
	}
}

void IndeoDecoderBase::outputPlane(IVIPlaneDesc *_plane, uint8 *dst, int dstPitch) {
	const int16 *src = _plane->_bands[0]._buf;
	uint32 pitch = _plane->_bands[0]._pitch;

	if (!src)
		return;

	for (int y = 0; y < _plane->_height; y++) {
		for (int x = 0; x < _plane->_width; x++)
			dst[x] = avClipUint8(src[x] + 128);
		src += pitch;
		dst += dstPitch;
	}
}

int IndeoDecoderBase::processEmptyTile(IVIBandDesc *band,
			IVITile *tile, int32 mvScale) {
	if (tile->_numMBs != IVI_MBs_PER_TILE(tile->_width, tile->_height, band->_mbSize)) {
		warning("Allocated tile size %d mismatches "
			"parameters %d in processEmptyTile()",
			tile->_numMBs, IVI_MBs_PER_TILE(tile->_width, tile->_height, band->_mbSize));
		return -1;
	}

	int offs = tile->_yPos * band->_pitch + tile->_xPos;
	IVIMbInfo *mb = tile->_mbs;
	IVIMbInfo *refMb = tile->_refMbs;
	int rowOffset = band->_mbSize * band->_pitch;
	int needMc = 0; // reset the mc tracking flag

	for (int y = tile->_yPos; y < (tile->_yPos + tile->_height); y += band->_mbSize) {
		int mbOffset = offs;

		for (int x = tile->_xPos; x < (tile->_xPos + tile->_width); x += band->_mbSize) {
			mb->_xPos = x;
			mb->_yPos = y;
			mb->_bufOffs = mbOffset;

			mb->_type = 1; // set the macroblocks _type = INTER
			mb->_cbp = 0;  // all blocks are empty

			if (!band->_qdeltaPresent && !band->_plane && !band->_bandNum) {
				mb->_qDelta = band->_globQuant;
				mb->_mvX = 0;
				mb->_mvY = 0;
			}

			if (band->_inheritQDelta && refMb)
				mb->_qDelta = refMb->_qDelta;

			if (band->_inheritMv && refMb) {
				// motion vector inheritance
				if (mvScale) {
					mb->_mvX = scaleMV(refMb->_mvX, mvScale);
					mb->_mvY = scaleMV(refMb->_mvY, mvScale);
				} else {
					mb->_mvX = refMb->_mvX;
					mb->_mvY = refMb->_mvY;
				}
				needMc |= mb->_mvX || mb->_mvY; // tracking non-zero motion vectors

				int dmv_x, dmv_y, cx, cy;

				dmv_x = mb->_mvX >> band->_isHalfpel;
				dmv_y = mb->_mvY >> band->_isHalfpel;
				cx = mb->_mvX &  band->_isHalfpel;
				cy = mb->_mvY &  band->_isHalfpel;

				if (mb->_xPos + dmv_x < 0
					|| mb->_xPos + dmv_x + band->_mbSize + cx > band->_pitch
					|| mb->_yPos + dmv_y < 0
					|| mb->_yPos + dmv_y + band->_mbSize + cy > band->_aHeight) {
					warning("MV out of bounds");
					return -1;
				}
			}

			mb++;
			if (refMb)
				refMb++;
			mbOffset += band->_mbSize;
		} // for x
		offs += rowOffset;
	} // for y

	if (band->_inheritMv && needMc) { // apply motion compensation if there is at least one non-zero motion vector
		int numBlocks = (band->_mbSize != band->_blkSize) ? 4 : 1; // number of blocks per mb
		IviMCFunc mcNoDeltaFunc = (band->_blkSize == 8) ? IndeoDSP::ffIviMc8x8NoDelta
			: IndeoDSP::ffIviMc4x4NoDelta;

		int mbn;
		for (mbn = 0, mb = tile->_mbs; mbn < tile->_numMBs; mb++, mbn++) {
			int mvX = mb->_mvX;
			int mvY = mb->_mvY;
			int mcType;
			if (!band->_isHalfpel) {
				mcType = 0; // we have only fullpel vectors
			} else {
				mcType = ((mvY & 1) << 1) | (mvX & 1);
				mvX >>= 1;
				mvY >>= 1; // convert halfpel vectors into fullpel ones
			}

			for (int blk = 0; blk < numBlocks; blk++) {
				// adjust block position in the buffer according with its number
				offs = mb->_bufOffs + band->_blkSize * ((blk & 1) + !!(blk & 2) * band->_pitch);
				int ret = iviMc(band, mcNoDeltaFunc, nullptr, offs,
					mvX, mvY, 0, 0, mcType, -1);
				if (ret < 0)
					return ret;
			}
		}
	} else {
		// copy data from the reference tile into the current one
		const int16 *src = band->_refBuf + tile->_yPos * band->_pitch + tile->_xPos;
		int16 *dst = band->_buf + tile->_yPos * band->_pitch + tile->_xPos;
		for (int y = 0; y < tile->_height; y++) {
			memcpy(dst, src, tile->_width*sizeof(band->_buf[0]));
			src += band->_pitch;
			dst += band->_pitch;
		}
	}

	return 0;
}

int IndeoDecoderBase::decodeTileDataSize(GetBits *gb) {
	int len = 0;

	if (gb->getBit()) {
		len = gb->getBits<8>();
		if (len == 255)
			len = gb->getBits<24>();
	}

	// align the bitstream reader on the byte boundary
	gb->align();

	return len;
}

int IndeoDecoderBase::decodeBlocks(GetBits *_gb, IVIBandDesc *band, IVITile *tile) {
	int ret;
	int mcType = 0, mcType2 = -1;
	int mvX = 0, mvY = 0, mvX2 = 0, mvY2 = 0;

	// init intra prediction for the DC coefficient
	int32 prevDc  = 0;
	int blkSize   = band->_blkSize;
	// number of blocks per mb
	int numBlocks = (band->_mbSize != blkSize) ? 4 : 1;
	IviMCFunc mcWithDeltaFunc, mcNoDeltaFunc;
	IviMCAvgFunc mcAvgWithDeltaFunc, mcAvgNoDeltaFunc;

	if (blkSize == 8) {
		mcWithDeltaFunc     = IndeoDSP::ffIviMc8x8Delta;
		mcNoDeltaFunc       = IndeoDSP::ffIviMc8x8NoDelta;
		mcAvgWithDeltaFunc = IndeoDSP::ffIviMcAvg8x8Delta;
		mcAvgNoDeltaFunc   = IndeoDSP::ffIviMcAvg8x8NoDelta;
	} else {
		mcWithDeltaFunc     = IndeoDSP::ffIviMc4x4Delta;
		mcNoDeltaFunc       = IndeoDSP::ffIviMc4x4NoDelta;
		mcAvgWithDeltaFunc = IndeoDSP::ffIviMcAvg4x4Delta;
		mcAvgNoDeltaFunc   = IndeoDSP::ffIviMcAvg4x4NoDelta;
	}

	int mbn;
	IVIMbInfo *mb;

	for (mbn = 0, mb = tile->_mbs; mbn < tile->_numMBs; mb++, mbn++) {
		int isIntra    = !mb->_type;
		uint32 cbp     = mb->_cbp;
		uint32 bufOffs = mb->_bufOffs;

		uint32 quant = band->_globQuant + mb->_qDelta;
		if (_ctx._isIndeo4)
			quant = avClipUintp2(quant, 5);
		else
			quant = CLIP((int)quant, 0, 23);

		const uint8 *scaleTab = isIntra ? band->_intraScale : band->_interScale;
		if (scaleTab)
			quant = scaleTab[quant];

		if (!isIntra) {
			mvX  = mb->_mvX;
			mvY  = mb->_mvY;
			mvX2 = mb->_bMvX;
			mvY2 = mb->_bMvY;
			if (band->_isHalfpel) {
				mcType  = ((mvY  & 1) << 1) | (mvX  & 1);
				mcType2 = ((mvY2 & 1) << 1) | (mvX2 & 1);
				mvX  >>= 1;
				mvY  >>= 1;
				mvX2 >>= 1;
				mvY2 >>= 1; // convert halfpel vectors into fullpel ones
			}
			if (mb->_type == 2)
				mcType = -1;
			if (mb->_type != 2 && mb->_type != 3)
				mcType2 = -1;
			if (mb->_type) {
				int dmv_x, dmv_y, cx, cy;

				dmv_x = mb->_mvX >> band->_isHalfpel;
				dmv_y = mb->_mvY >> band->_isHalfpel;
				cx    = mb->_mvX &  band->_isHalfpel;
				cy    = mb->_mvY &  band->_isHalfpel;

				if (mb->_xPos + dmv_x < 0 ||
					mb->_xPos + dmv_x + band->_mbSize + cx > band->_pitch ||
					mb->_yPos + dmv_y < 0 ||
					mb->_yPos + dmv_y + band->_mbSize + cy > band->_aHeight) {
					return -1;
				}
			}
			if (mb->_type == 2 || mb->_type == 3) {
				int dmv_x, dmv_y, cx, cy;

				dmv_x = mb->_bMvX >> band->_isHalfpel;
				dmv_y = mb->_bMvY >> band->_isHalfpel;
				cx    = mb->_bMvX &  band->_isHalfpel;
				cy    = mb->_bMvY &  band->_isHalfpel;

				if (mb->_xPos + dmv_x < 0 ||
					mb->_xPos + dmv_x + band->_mbSize + cx > band->_pitch ||
					mb->_yPos + dmv_y < 0 ||
					mb->_yPos + dmv_y + band->_mbSize + cy > band->_aHeight) {
					return -1;
				}
			}
		}

		for (int blk = 0; blk < numBlocks; blk++) {
			// adjust block position in the buffer according to its number
			if (blk & 1) {
				bufOffs += blkSize;
			} else if (blk == 2) {
				bufOffs -= blkSize;
				bufOffs += blkSize * band->_pitch;
			}

			if (cbp & 1) { // block coded ?
				ret = decodeCodedBlocks(_gb, band, mcWithDeltaFunc,
											  mcAvgWithDeltaFunc,
											  mvX, mvY, mvX2, mvY2,
											  &prevDc, isIntra,
											  mcType, mcType2, quant,
											  bufOffs);
				if (ret < 0)
					return ret;
			} else {
				// block not coded
				// for intra blocks apply the dc slant transform
				// for inter - perform the motion compensation without delta
				if (isIntra) {
					ret = iviDcTransform(band, &prevDc, bufOffs, blkSize);
					if (ret < 0)
						return ret;
				} else {
					ret = iviMc(band, mcNoDeltaFunc, mcAvgNoDeltaFunc,
								 bufOffs, mvX, mvY, mvX2, mvY2,
								 mcType, mcType2);
					if (ret < 0)
						return ret;
				}
			}

			cbp >>= 1;
		}// for blk
	}// for mbn

	_gb->align();
	return 0;
}

int IndeoDecoderBase::scaleMV(int mv, int mvScale) {
	return (mv + (mv > 0) + (mvScale - 1)) >> mvScale;
}

int IndeoDecoderBase::iviMc(IVIBandDesc *band, IviMCFunc mc, IviMCAvgFunc mcAvg,
				  int offs, int mvX, int mvY, int mvX2, int mvY2,
				  int mcType, int mcType2) {
	int refOffs = offs + mvY * band->_pitch + mvX;
	int bufSize = band->_pitch * band->_aHeight;
	int minSize = band->_pitch * (band->_blkSize - 1) + band->_blkSize;
	int refSize = (mcType > 1) * band->_pitch + (mcType & 1);

	if (mcType != -1) {
		assert(offs >= 0 && refOffs >= 0 && band->_refBuf);
		assert(bufSize - minSize >= offs);
		assert(bufSize - minSize - refSize >= refOffs);
	}

	if (mcType2 == -1) {
		mc(band->_buf + offs, band->_refBuf + refOffs, band->_pitch, mcType);
	} else {
		int ref_offs2 = offs + mvY2 * band->_pitch + mvX2;
		int ref_size2 = (mcType2 > 1) * band->_pitch + (mcType2 & 1);
		if (offs < 0 || ref_offs2 < 0 || !band->_bRefBuf)
			return -1;
		if (bufSize - minSize - ref_size2 < ref_offs2)
			return -1;

		if (mcType == -1)
			mc(band->_buf + offs, band->_bRefBuf + ref_offs2,
			   band->_pitch, mcType2);
		else
			mcAvg(band->_buf + offs, band->_refBuf + refOffs,
				   band->_bRefBuf + ref_offs2, band->_pitch,
				   mcType, mcType2);
	}

	return 0;
}

int IndeoDecoderBase::decodeCodedBlocks(GetBits *gb, IVIBandDesc *band,
		IviMCFunc mc, IviMCAvgFunc mcAvg, int mvX, int mvY,
		int mvX2, int mvY2, int32 *prevDc, int isIntra,
		int mcType, int mcType2, uint32 quant, int offs) {
	const uint16 *baseTab = isIntra ? band->_intraBase : band->_interBase;
	RVMapDesc *rvmap = band->_rvMap;
	uint8 colFlags[8];
	int32 trvec[64];
	uint32 sym = 0, q;
	int lo, hi;
	int pos, run, val;
	int blkSize = band->_blkSize;
	int numCoeffs = blkSize * blkSize;
	int colMask = blkSize - 1;
	int scanPos = -1;
	int minSize = band->_pitch * (band->_transformSize - 1) +
		band->_transformSize;
	int bufSize = band->_pitch * band->_aHeight - offs;

	if (minSize > bufSize)
		return -1;

	if (!band->_scan) {
		warning("Scan pattern is not set.");
		return -1;
	}

	// zero transform vector
	memset(trvec, 0, numCoeffs * sizeof(trvec[0]));
	// zero column flags
	memset(colFlags, 0, sizeof(colFlags));
	while (scanPos <= numCoeffs) {
		sym = gb->getVLC2<1, IVI_VLC_BITS>(band->_blkVlc._tab->_table);
		if (sym == rvmap->_eobSym)
			break; // End of block

		// Escape - run/val explicitly coded using 3 vlc codes
		if (sym == rvmap->_escSym) {
			run = gb->getVLC2<1, IVI_VLC_BITS>(band->_blkVlc._tab->_table) + 1;
			lo = gb->getVLC2<1, IVI_VLC_BITS>(band->_blkVlc._tab->_table);
			hi = gb->getVLC2<1, IVI_VLC_BITS>(band->_blkVlc._tab->_table);
			// merge them and convert into signed val
			val = IVI_TOSIGNED((hi << 6) | lo);
		} else {
			if (sym >= 256U) {
				warning("Invalid sym encountered");
				return -1;
			}
			run = rvmap->_runtab[sym];
			val = rvmap->_valtab[sym];
		}

		// de-zigzag and dequantize
		scanPos += run;
		if (scanPos >= numCoeffs || scanPos < 0)
			break;
		pos = band->_scan[scanPos];

		if (!val)
			warning("Val = 0 encountered!");

		q = (baseTab[pos] * quant) >> 9;
		if (q > 1)
			val = val * q + FFSIGN(val) * (((q ^ 1) - 1) >> 1);
		trvec[pos] = val;
		// track columns containing non-zero coeffs
		colFlags[pos & colMask] |= !!val;
	}

	if (scanPos < 0 || (scanPos >= numCoeffs && sym != rvmap->_eobSym))
		return -1; // corrupt block data

	// undoing DC coeff prediction for intra-blocks
	if (isIntra && band->_is2dTrans) {
		*prevDc += trvec[0];
		trvec[0] = *prevDc;
		colFlags[0] |= !!*prevDc;
	}

	if (band->_transformSize > band->_blkSize) {
		warning("Too large transform");
		return -1;
	}

	// apply inverse transform
	band->_invTransform(trvec, band->_buf + offs,
		band->_pitch, colFlags);

	// apply motion compensation
	if (!isIntra)
		return iviMc(band, mc, mcAvg, offs, mvX, mvY, mvX2, mvY2,
			mcType, mcType2);

	return 0;
}

int IndeoDecoderBase::iviDcTransform(IVIBandDesc *band, int32 *prevDc,
		int bufOffs, int blkSize) {
	int bufSize = band->_pitch * band->_aHeight - bufOffs;
	int minSize = (blkSize - 1) * band->_pitch + blkSize;

	if (minSize > bufSize)
		return -1;

	band->_dcTransform(prevDc, band->_buf + bufOffs, band->_pitch, blkSize);
	return 0;
}

/*------------------------------------------------------------------------*/

const uint8 IndeoDecoderBase::_ffIviVerticalScan8x8[64] = {
	0,  8, 16, 24, 32, 40, 48, 56,
	1,  9, 17, 25, 33, 41, 49, 57,
	2, 10, 18, 26, 34, 42, 50, 58,
	3, 11, 19, 27, 35, 43, 51, 59,
	4, 12, 20, 28, 36, 44, 52, 60,
	5, 13, 21, 29, 37, 45, 53, 61,
	6, 14, 22, 30, 38, 46, 54, 62,
	7, 15, 23, 31, 39, 47, 55, 63
};

const uint8 IndeoDecoderBase::_ffIviHorizontalScan8x8[64] = {
	0,  1,  2,  3,  4,  5,  6,  7,
	8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61, 62, 63
};

const uint8 IndeoDecoderBase::_ffIviDirectScan4x4[16] = {
	0, 1, 4, 8, 5, 2, 3, 6, 9, 12, 13, 10, 7, 11, 14, 15
};

const RVMapDesc IVI45DecContext::_ff_ivi_rvmap_tabs[9] = {
{   // MapTab0
	5, // _eobSym
	2, // _escSym
	// run table
	{1,  1,  0,  1,  1,  0,  1,  1,  2,  2,  1,  1,  1,  1,  3,  3,
	 1,  1,  2,  2,  1,  1,  4,  4,  1,  1,  1,  1,  2,  2,  5,  5,
	 1,  1,  3,  3,  1,  1,  6,  6,  1,  2,  1,  2,  7,  7,  1,  1,
	 8,  8,  1,  1,  4,  2,  1,  4,  2,  1,  3,  3,  1,  1,  1,  9,
	 9,  1,  2,  1,  2,  1,  5,  5,  1,  1, 10, 10,  1,  1,  3,  3,
	 2,  2,  1,  1, 11, 11,  6,  4,  4,  1,  6,  1,  2,  1,  2, 12,
	 8,  1, 12,  7,  8,  7,  1, 16,  1, 16,  1,  3,  3, 13,  1, 13,
	 2,  2,  1, 15,  1,  5, 14, 15,  1,  5, 14,  1, 17,  8, 17,  8,
	 1,  4,  4,  2,  2,  1, 25, 25, 24, 24,  1,  3,  1,  3,  1,  8,
	 6,  7,  6,  1, 18,  8, 18,  1,  7, 23,  2,  2, 23,  1,  1, 21,
	22,  9,  9, 22, 19,  1, 21,  5, 19,  5,  1, 33, 20, 33, 20,  8,
	 4,  4,  1, 32,  2,  2,  8,  3, 32, 26,  3,  1,  7,  7, 26,  6,
	 1,  6,  1,  1, 16,  1, 10,  1, 10,  2, 16, 29, 28,  2, 29, 28,
	 1, 27,  5,  8,  5, 27,  1,  8,  3,  7,  3, 31, 41, 31,  1, 41,
	 6,  1,  6,  7,  4,  4,  1,  1,  2,  1,  2, 11, 34, 30, 11,  1,
	30, 15, 15, 34, 36, 40, 36, 40, 35, 35, 37, 37, 39, 39, 38, 38},

	// value table
	{ 1,  -1,   0,   2,  -2,   0,   3,  -3,   1,  -1,   4,  -4,   5,  -5,   1,  -1,
	  6,  -6,   2,  -2,   7,  -7,   1,  -1,   8,  -8,   9,  -9,   3,  -3,   1,  -1,
	 10, -10,   2,  -2,  11, -11,   1,  -1,  12,   4, -12,  -4,   1,  -1,  13, -13,
	  1,  -1,  14, -14,   2,   5,  15,  -2,  -5, -15,  -3,   3,  16, -16,  17,   1,
	 -1, -17,   6,  18,  -6, -18,   2,  -2,  19, -19,   1,  -1,  20, -20,   4,  -4,
	  7,  -7,  21, -21,   1,  -1,   2,   3,  -3,  22,  -2, -22,   8,  23,  -8,   1,
	  2, -23,  -1,   2,  -2,  -2,  24,   1, -24,  -1,  25,   5,  -5,   1, -25,  -1,
	  9,  -9,  26,   1, -26,   3,   1,  -1,  27,  -3,  -1, -27,   1,   3,  -1,  -3,
	 28,  -4,   4,  10, -10, -28,   1,  -1,   1,  -1,  29,   6, -29,  -6,  30,  -4,
	  3,   3,  -3, -30,   1,   4,  -1,  31,  -3,   1,  11, -11,  -1, -31,  32,  -1,
	 -1,   2,  -2,   1,   1, -32,   1,   4,  -1,  -4,  33,  -1,   1,   1,  -1,   5,
	  5,  -5, -33,  -1, -12,  12,  -5,  -7,   1,   1,   7,  34,   4,  -4,  -1,   4,
	-34,  -4,  35,  36,  -2, -35,  -2, -36,   2,  13,   2,  -1,   1, -13,   1,  -1,
	 37,   1,  -5,   6,   5,  -1,  38,  -6,  -8,   5,   8,  -1,   1,   1, -37,  -1,
	  5,  39,  -5,  -5,   6,  -6, -38, -39, -14,  40,  14,   2,   1,   1,  -2, -40,
	 -1,  -2,   2,  -1,  -1,  -1,   1,   1,   1,  -1,   1,  -1,   1,  -1,   1,  -1}
},{
	// MapTab1
	0,  // _eobSym
	38, // _escSym
	// run table
	{0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  8,  6,  8,  7,
	 7,  9,  9, 10, 10, 11, 11,  1, 12,  1, 12, 13, 13, 16, 14, 16,
	14, 15, 15, 17, 17, 18,  0, 18, 19, 20, 21, 19, 22, 21, 20, 22,
	25, 24,  2, 25, 24, 23, 23,  2, 26, 28, 26, 28, 29, 27, 29, 27,
	33, 33,  1, 32,  1,  3, 32, 30, 36,  3, 36, 30, 31, 31, 35, 34,
	37, 41, 34, 35, 37,  4, 41,  4, 49,  8,  8, 49, 40, 38,  5, 38,
	40, 39,  5, 39, 42, 43, 42,  7, 57,  6, 43, 44,  6, 50,  7, 44,
	57, 48, 50, 48, 45, 45, 46, 47, 51, 46, 47, 58,  1, 51, 58,  1,
	52, 59, 53,  9, 52, 55, 55, 59, 53, 56, 54, 56, 54,  9, 64, 64,
	60, 63, 60, 63, 61, 62, 61, 62,  2, 10,  2, 10, 11,  1, 11, 13,
	12,  1, 12, 13, 16, 16,  8,  8, 14,  3,  3, 15, 14, 15,  4,  4,
	 1, 17, 17,  5,  1,  7,  7,  5,  6,  1,  2,  2,  6, 22,  1, 25,
	21, 22,  8, 24,  1, 21, 25, 24,  8, 18, 18, 23,  9, 20, 23, 33,
	29, 33, 20,  1, 19,  1, 29, 36,  9, 36, 19, 41, 28, 57, 32,  3,
	28,  3,  1, 27, 49, 49,  1, 32, 26, 26,  2,  4,  4,  7, 57, 41,
	 2,  7, 10,  5, 37, 16, 10, 27,  8,  8, 13, 16, 37, 13,  1,  5},

	// value table
	{0,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   1,   1,  -1,  -1,   1,
	-1,   1,  -1,   1,  -1,   1,  -1,   2,   1,  -2,  -1,   1,  -1,   1,   1,  -1,
	-1,   1,  -1,   1,  -1,   1,   0,  -1,   1,   1,   1,  -1,   1,  -1,  -1,  -1,
	 1,   1,   2,  -1,  -1,   1,  -1,  -2,   1,   1,  -1,  -1,   1,   1,  -1,  -1,
	 1,  -1,   3,   1,  -3,   2,  -1,   1,   1,  -2,  -1,  -1,  -1,   1,   1,   1,
	 1,   1,  -1,  -1,  -1,   2,  -1,  -2,   1,   2,  -2,  -1,   1,   1,   2,  -1,
	-1,   1,  -2,  -1,   1,   1,  -1,   2,   1,   2,  -1,   1,  -2,  -1,  -2,  -1,
	-1,   1,   1,  -1,   1,  -1,   1,   1,   1,  -1,  -1,   1,   4,  -1,  -1,  -4,
	 1,   1,   1,   2,  -1,  -1,   1,  -1,  -1,   1,  -1,  -1,   1,  -2,   1,  -1,
	 1,   1,  -1,  -1,   1,   1,  -1,  -1,   3,   2,  -3,  -2,   2,   5,  -2,   2,
	 2,  -5,  -2,  -2,  -2,   2,  -3,   3,   2,   3,  -3,   2,  -2,  -2,   3,  -3,
	 6,   2,  -2,   3,  -6,   3,  -3,  -3,   3,   7,  -4,   4,  -3,   2,  -7,   2,
	 2,  -2,  -4,   2,   8,  -2,  -2,  -2,   4,   2,  -2,   2,   3,   2,  -2,  -2,
	 2,   2,  -2,  -8,  -2,   9,  -2,   2,  -3,  -2,   2,  -2,   2,   2,   2,   4,
	-2,  -4,  10,   2,   2,  -2,  -9,  -2,   2,  -2,   5,   4,  -4,   4,  -2,   2,
	-5,  -4,  -3,   4,   2,  -3,   3,  -2,  -5,   5,   3,   3,  -2,  -3, -10,  -4}
},{
	// MapTab2
	2,  // _eobSym
	11, // _escSym
	// run table
	{1,  1,  0,  2,  2,  1,  1,  3,  3,  4,  4,  0,  1,  1,  5,  5,
	 2,  2,  6,  6,  7,  7,  1,  8,  1,  8,  3,  3,  9,  9,  1,  2,
	 2,  1,  4, 10,  4, 10, 11, 11,  1,  5, 12, 12,  1,  5, 13, 13,
	 3,  3,  6,  6,  2,  2, 14, 14, 16, 16, 15,  7, 15,  8,  8,  7,
	 1,  1, 17, 17,  4,  4,  1,  1, 18, 18,  2,  2,  5,  5, 25,  3,
	 9,  3, 25,  9, 19, 24, 19, 24,  1, 21, 20,  1, 21, 22, 20, 22,
	23, 23,  8,  6, 33,  6,  8, 33,  7,  7, 26, 26,  1, 32,  1, 32,
	28,  4, 28, 10, 29, 27, 27, 10, 41,  4, 29,  2,  2, 41, 36, 31,
	49, 31, 34, 30, 34, 36, 30, 35,  1, 49, 11,  5, 35, 11,  1,  3,
	 3,  5, 37, 37,  8, 40,  8, 40, 12, 12, 42, 42,  1, 38, 16, 57,
	 1,  6, 16, 39, 38,  6,  7,  7, 13, 13, 39, 43,  2, 43, 57,  2,
	50,  9, 44,  9, 50,  4, 15, 48, 44,  4,  1, 15, 48, 14, 14,  1,
	45, 45,  8,  3,  5,  8, 51, 47,  3, 46, 46, 47,  5, 51,  1, 17,
	17, 58,  1, 58,  2, 52, 52,  2, 53,  7, 59,  6,  6, 56, 53, 55,
	 7, 55,  1, 54, 59, 56, 54, 10,  1, 10,  4, 60,  1, 60,  8,  4,
	 8, 64, 64, 61,  1, 63,  3, 63, 62, 61,  5, 11,  5,  3, 11, 62},

	// value table
	{ 1,  -1,   0,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   0,   3,  -3,   1,  -1,
	  2,  -2,   1,  -1,   1,  -1,   4,   1,  -4,  -1,   2,  -2,   1,  -1,   5,   3,
	 -3,  -5,   2,   1,  -2,  -1,   1,  -1,   6,   2,   1,  -1,  -6,  -2,   1,  -1,
	  3,  -3,   2,  -2,   4,  -4,   1,  -1,   1,  -1,   1,   2,  -1,   2,  -2,  -2,
	  7,  -7,   1,  -1,   3,  -3,   8,  -8,   1,  -1,   5,  -5,   3,  -3,   1,   4,
	  2,  -4,  -1,  -2,   1,   1,  -1,  -1,   9,   1,   1,  -9,  -1,   1,  -1,  -1,
	  1,  -1,   3,  -3,   1,   3,  -3,  -1,   3,  -3,   1,  -1,  10,   1, -10,  -1,
	  1,   4,  -1,   2,   1,  -1,   1,  -2,   1,  -4,  -1,   6,  -6,  -1,   1,   1,
	  1,  -1,   1,   1,  -1,  -1,  -1,   1,  11,  -1,  -2,   4,  -1,   2, -11,   5,
	 -5,  -4,  -1,   1,   4,   1,  -4,  -1,  -2,   2,   1,  -1,  12,   1,  -2,   1,
	-12,   4,   2,   1,  -1,  -4,   4,  -4,   2,  -2,  -1,   1,   7,  -1,  -1,  -7,
	 -1,  -3,   1,   3,   1,   5,   2,   1,  -1,  -5,  13,  -2,  -1,   2,  -2, -13,
	  1,  -1,   5,   6,   5,  -5,   1,   1,  -6,   1,  -1,  -1,  -5,  -1,  14,   2,
	 -2,   1, -14,  -1,   8,   1,  -1,  -8,   1,   5,   1,   5,  -5,   1,  -1,   1,
	 -5,  -1,  15,   1,  -1,  -1,  -1,   3, -15,  -3,   6,   1,  16,  -1,   6,  -6,
	 -6,   1,  -1,   1, -16,   1,   7,  -1,   1,  -1,  -6,  -3,   6,  -7,   3,  -1}
},{
	// MapTab3
	0,  // _eobSym
	35, // _escSym
	// run table
	{0,  1,  1,  2,  2,  3,  3,  4,  4,  1,  1,  5,  5,  6,  6,  7,
	 7,  8,  8,  9,  9,  2,  2, 10, 10,  1,  1, 11, 11, 12, 12,  3,
	 3, 13, 13,  0, 14, 14, 16, 15, 16, 15,  4,  4, 17,  1, 17,  1,
	 5,  5, 18, 18,  2,  2,  6,  6,  8, 19,  7,  8,  7, 19, 20, 20,
	21, 21, 22, 24, 22, 24, 23, 23,  1,  1, 25, 25,  3,  3, 26, 26,
	 9,  9, 27, 27, 28, 28, 33, 29,  4, 33, 29,  1,  4,  1, 32, 32,
	 2,  2, 31, 10, 30, 10, 30, 31, 34, 34,  5,  5, 36, 36, 35, 41,
	35, 11, 41, 11, 37,  1,  8,  8, 37,  6,  1,  6, 40,  7,  7, 40,
	12, 38, 12, 39, 39, 38, 49, 13, 49, 13,  3, 42,  3, 42, 16, 16,
	43, 43, 14, 14,  1,  1, 44, 15, 44, 15,  2,  2, 57, 48, 50, 48,
	57, 50,  4, 45, 45,  4, 46, 47, 47, 46,  1, 51,  1, 17, 17, 51,
	 8,  9,  9,  5, 58,  8, 58,  5, 52, 52, 55, 56, 53, 56, 55, 59,
	59, 53, 54,  1,  6, 54,  7,  7,  6,  1,  2,  3,  2,  3, 64, 60,
	60, 10, 10, 64, 61, 62, 61, 63,  1, 63, 62,  1, 18, 24, 18,  4,
	25,  4,  8, 21, 21,  1, 24, 22, 25, 22,  8, 11, 19, 11, 23,  1,
	20, 23, 19, 20,  5, 12,  5,  1, 16,  2, 12, 13,  2, 13,  1, 16},

	// value table
	{ 0,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   1,
	 -1,   1,  -1,   1,  -1,   2,  -2,   1,  -1,   3,  -3,   1,  -1,   1,  -1,   2,
	 -2,   1,  -1,   0,   1,  -1,   1,   1,  -1,  -1,   2,  -2,   1,   4,  -1,  -4,
	  2,  -2,   1,  -1,  -3,   3,   2,  -2,   2,   1,   2,  -2,  -2,  -1,   1,  -1,
	  1,  -1,   1,   1,  -1,  -1,   1,  -1,   5,  -5,   1,  -1,   3,  -3,   1,  -1,
	  2,  -2,   1,  -1,   1,  -1,   1,   1,   3,  -1,  -1,   6,  -3,  -6,  -1,   1,
	  4,  -4,   1,   2,   1,  -2,  -1,  -1,   1,  -1,   3,  -3,   1,  -1,   1,   1,
	 -1,   2,  -1,  -2,   1,   7,  -3,   3,  -1,   3,  -7,  -3,   1,  -3,   3,  -1,
	  2,   1,  -2,   1,  -1,  -1,   1,   2,  -1,  -2,  -4,  -1,   4,   1,   2,  -2,
	  1,  -1,  -2,   2,   8,  -8,  -1,   2,   1,  -2,  -5,   5,   1,  -1,  -1,   1,
	 -1,   1,   4,  -1,   1,  -4,  -1,  -1,   1,   1,   9,   1,  -9,   2,  -2,  -1,
	 -4,   3,  -3,  -4,  -1,   4,   1,   4,   1,  -1,   1,  -1,   1,   1,  -1,   1,
	 -1,  -1,  -1,  10,   4,   1,   4,  -4,  -4, -10,   6,   5,  -6,  -5,   1,  -1,
	  1,   3,  -3,  -1,   1,  -1,  -1,  -1,  11,   1,   1, -11,  -2,  -2,   2,   5,
	 -2,  -5,  -5,   2,  -2,  12,   2,  -2,   2,   2,   5,  -3,  -2,   3,  -2, -12,
	 -2,   2,   2,   2,  -5,   3,   5,  13,  -3,   7,  -3,  -3,  -7,   3, -13,   3}
},{
	// MapTab4
	0,  // _eobSym
	34, // _escSym
	// run table
	{0,  1,  1,  1,  2,  2,  1,  3,  3,  1,  1,  1,  4,  4,  1,  5,
	 2,  1,  5,  2,  1,  1,  6,  6,  1,  1,  1,  1,  1,  7,  3,  1,
	 2,  3,  0,  1,  2,  7,  1,  1,  1,  8,  1,  1,  8,  1,  1,  1,
	 9,  1,  9,  1,  2,  1,  1,  2,  1,  1, 10,  4,  1, 10,  1,  4,
	 1,  1,  1,  1,  1,  3,  1,  1,  1,  3,  2,  1,  5,  1,  1,  1,
	 2,  5,  1, 11,  1, 11,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 2,  1,  6,  1,  6,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1, 12,
	 3,  1, 12,  1,  1,  1,  2,  1,  1,  3,  1,  1,  1,  1,  1,  1,
	 4,  1,  1,  1,  2,  1,  1,  4,  1,  1,  1,  1,  1,  1,  2,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  2,  1,  1,  5,
	 1,  1,  1,  1,  1,  7,  1,  7,  1,  1,  2,  3,  1,  1,  1,  1,
	 5,  1,  1,  1,  1,  1,  1,  2, 13,  1,  1,  1,  1,  1,  1,  1,
	 1,  1,  1,  1,  1,  1,  1,  1, 13,  2,  1,  1,  4,  1,  1,  1,
	 3,  1,  6,  1,  1,  1, 14,  1,  1,  1,  1,  1, 14,  6,  1,  1,
	 1,  1, 15,  2,  4,  1,  2,  3, 15,  1,  1,  1,  8,  1,  1,  8,
	 1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1},

	// value table
	{ 0,   1,  -1,   2,   1,  -1,  -2,   1,  -1,   3,  -3,   4,   1,  -1,  -4,   1,
	  2,   5,  -1,  -2,  -5,   6,   1,  -1,  -6,   7,  -7,   8,  -8,   1,   2,   9,
	  3,  -2,   0,  -9,  -3,  -1,  10, -10,  11,   1, -11,  12,  -1, -12,  13, -13,
	  1,  14,  -1, -14,   4,  15, -15,  -4,  16, -16,   1,   2,  17,  -1, -17,  -2,
	 18, -18,  19, -19,  20,   3, -20,  21, -21,  -3,   5,  22,   2, -22, -23,  23,
	 -5,  -2,  24,   1, -24,  -1,  25, -25,  26, -26, -27,  27,  28,  29, -28, -29,
	  6,  30,   2, -31,  -2, -30,  31,  -6, -32,  32,  33, -33,  34, -35, -34,   1,
	  4, -36,  -1,  35,  37,  36,   7, -37,  38,  -4, -38,  39,  41,  40, -40, -39,
	  3,  42, -43, -41,  -7, -42,  43,  -3,  44, -44,  45, -45,  46,  47,   8, -47,
	-48, -46,  50, -50,  48,  49,  51, -49,  52, -52,   5, -51,  -8, -53,  53,   3,
	-56,  56,  55,  54, -54,   2,  60,  -2, -55,  58,   9,  -5,  59,  57, -57, -63,
	 -3, -58, -60, -61,  61, -59, -62,  -9,   1,  64,  62,  69, -64,  63,  65, -67,
	-68,  66, -65,  68, -66, -69,  67, -70,  -1,  10,  71, -71,   4,  73,  72,  70,
	  6, -76,  -3,  74, -78, -74,   1,  78,  80, -72, -75,  76,  -1,   3, -73,  79,
	 75,  77,   1,  11,  -4, -79, -10,  -6,  -1, -77, -83, -80,   2,  81, -84,  -2,
	 83, -81,  82, -82,  84, -87, -86,  85, -11, -85,  86, -89,  87, -88,  88,  89}
},{
	// MapTab5
	2,  // _eobSym
	33, // _escSym
	// run table
	{1,  1,  0,  2,  1,  2,  1,  3,  3,  1,  1,  4,  4,  2,  2,  1,
	 1,  5,  5,  6,  1,  6,  1,  7,  7,  3,  3,  2,  8,  2,  8,  1,
	 1,  0,  9,  9,  1,  1, 10,  4, 10,  4, 11, 11,  2,  1,  2,  1,
	12, 12,  3,  3,  1,  1, 13,  5,  5, 13, 14,  1,  1, 14,  2,  2,
	 6,  6, 15,  1,  1, 15, 16,  4,  7, 16,  4,  7,  1,  1,  3,  3,
	 8,  8,  2,  2,  1,  1, 17, 17,  1,  1, 18, 18,  5,  5,  2,  2,
	 1,  1,  9, 19,  9, 19, 20,  3,  3, 20,  1, 10, 21,  1, 10,  4,
	 4, 21, 22,  6,  6, 22,  1,  1, 23, 24,  2,  2, 23, 24, 11,  1,
	 1, 11,  7, 25,  7,  1,  1, 25,  8,  8,  3, 26,  3,  1, 12,  2,
	 2, 26,  1, 12,  5,  5, 27,  4,  1,  4,  1, 27, 28,  1, 28, 13,
	 1, 13,  2, 29,  2,  1, 32,  6,  1, 30, 14, 29, 14,  6,  3, 31,
	 3,  1, 30,  1, 32, 31, 33,  9, 33,  1,  1,  7,  9,  7,  2,  2,
	 1,  1,  4, 36, 34,  4,  5, 10, 10,  5, 34,  1,  1, 35,  8,  8,
	36,  3, 35,  1, 15,  3,  2,  1, 16, 15, 16,  2, 37,  1, 37,  1,
	 1,  1,  6,  6, 38,  1, 38, 11,  1, 39, 39, 40, 11,  2, 41,  4,
	40,  1,  2,  4,  1,  1,  1, 41,  3,  1,  3,  1,  5,  7,  5,  7},

	// value table
	{ 1,  -1,   0,   1,   2,  -1,  -2,   1,  -1,   3,  -3,   1,  -1,   2,  -2,   4,
	 -4,   1,  -1,   1,   5,  -1,  -5,   1,  -1,   2,  -2,   3,   1,  -3,  -1,   6,
	 -6,   0,   1,  -1,   7,  -7,   1,   2,  -1,  -2,   1,  -1,   4,   8,  -4,  -8,
	  1,  -1,   3,  -3,   9,  -9,   1,   2,  -2,  -1,   1,  10, -10,  -1,   5,  -5,
	  2,  -2,   1,  11, -11,  -1,   1,   3,   2,  -1,  -3,  -2,  12, -12,   4,  -4,
	  2,  -2,  -6,   6,  13, -13,   1,  -1,  14, -14,   1,  -1,   3,  -3,   7,  -7,
	 15, -15,   2,   1,  -2,  -1,   1,   5,  -5,  -1, -16,   2,   1,  16,  -2,   4,
	 -4,  -1,   1,   3,  -3,  -1,  17, -17,   1,   1,  -8,   8,  -1,  -1,   2,  18,
	-18,  -2,   3,   1,  -3,  19, -19,  -1,   3,  -3,   6,   1,  -6,  20,   2,   9,
	 -9,  -1, -20,  -2,   4,  -4,   1,  -5,  21,   5, -21,  -1,   1, -22,  -1,   2,
	 22,  -2,  10,   1, -10,  23,   1,   4, -23,   1,   2,  -1,  -2,  -4,  -7,   1,
	  7, -24,  -1,  24,  -1,  -1,   1,   3,  -1, -25,  25,   4,  -3,  -4,  11, -11,
	 26, -26,   6,   1,   1,  -6,  -5,  -3,   3,   5,  -1, -27,  27,   1,   4,  -4,
	 -1,  -8,  -1,  28,   2,   8, -12, -28,  -2,  -2,   2,  12,  -1,  29,   1, -29,
	 30, -30,   5,  -5,   1, -31,  -1,   3,  31,  -1,   1,   1,  -3, -13,   1,  -7,
	 -1, -32,  13,   7,  32,  33, -33,  -1,  -9, -34,   9,  34,  -6,   5,   6,  -5}
},{
	// MapTab6
	2,  // _eobSym
	13, // _escSym
	// run table
	{1,  1,  0,  1,  1,  2,  2,  1,  1,  3,  3,  1,  1,  0,  2,  2,
	 4,  1,  4,  1,  1,  1,  5,  5,  1,  1,  6,  6,  2,  2,  1,  1,
	 3,  3,  7,  7,  1,  1,  8,  8,  1,  1,  2,  2,  1,  9,  1,  9,
	 4,  4, 10,  1,  1, 10,  1,  1, 11, 11,  3,  3,  1,  2,  1,  2,
	 1,  1, 12, 12,  5,  5,  1,  1, 13,  1,  1, 13,  2,  2,  1,  1,
	 6,  6,  1,  1,  4, 14,  4, 14,  3,  1,  3,  1,  1,  1, 15,  7,
	15,  2,  2,  7,  1,  1,  1,  8,  1,  8, 16, 16,  1,  1,  1,  1,
	 2,  1,  1,  2,  1,  1,  3,  5,  5,  3,  4,  1,  1,  4,  1,  1,
	17, 17,  9,  1,  1,  9,  2,  2,  1,  1, 10, 10,  1,  6,  1,  1,
	 6, 18,  1,  1, 18,  1,  1,  1,  2,  2,  3,  1,  3,  1,  1,  1,
	 4,  1, 19,  1, 19,  7,  1,  1, 20,  1,  4, 20,  1,  7, 11,  2,
	 1, 11, 21,  2,  8,  5,  1,  8,  1,  5, 21,  1,  1,  1, 22,  1,
	 1, 22,  1,  1,  3,  3,  1, 23,  2, 12, 24,  1,  1,  2,  1,  1,
	12, 23,  1,  1, 24,  1,  1,  1,  4,  1,  1,  1,  2,  1,  6,  6,
	 4,  2,  1,  1,  1,  1,  1,  1,  1, 14, 13,  3,  1, 25,  9, 25,
	14,  1,  9,  3, 13,  1,  1,  1,  1,  1, 10,  1,  1,  2, 10,  2},

	// value table
	{-20,  -1,   0,   2,  -2,   1,  -1,   3,  -3,   1,  -1,   4,  -4,   0,   2,  -2,
	   1,   5,  -1,  -5,   6,  -6,   1,  -1,   7,  -7,   1,  -1,   3,  -3,   8,  -8,
	   2,  -2,   1,  -1,   9,  -9,   1,  -1,  10, -10,   4,  -4,  11,   1, -11,  -1,
	   2,  -2,   1,  12, -12,  -1,  13, -13,   1,  -1,   3,  -3,  14,   5, -14,  -5,
	 -15,  15,  -1,   1,   2,  -2,  16, -16,   1,  17, -17,  -1,   6,  -6,  18, -18,
	   2,  -2, -19,  19,  -3,   1,   3,  -1,   4,  20,  -4,   1, -21,  21,   1,   2,
	  -1,  -7,   7,  -2,  22, -22,  23,   2, -23,  -2,   1,  -1, -24,  24, -25,  25,
	  -8, -26,  26,   8, -27,  27,   5,   3,  -3,  -5,  -4,  28, -28,   4,  29, -29,
	   1,  -1,  -2, -30,  30,   2,   9,  -9, -31,  31,   2,  -2, -32,   3,  32, -33,
	  -3,   1,  33, -34,  -1,  34, -35,  35, -10,  10,  -6,  36,   6, -36,  37, -37,
	  -5,  38,   1, -38,  -1,   3,  39, -39,  -1,  40,   5,   1, -40,  -3,   2, -11,
	 -41,  -2,   1,  11,  -3,  -4,  41,   3,  42,   4,  -1, -43, -42,  43,   1, -44,
	  45,  -1,  44, -45,  -7,   7, -46,   1, -12,   2,   1, -47,  46,  12,  47,  48,
	  -2,  -1, -48,  49,  -1, -50, -49,  50,  -6, -51,  51,  52, -13,  53,  -4,   4,
	   6,  13, -53, -52, -54,  55,  54, -55, -56,  -2,   2,  -8,  56,   1,  -3,  -1,
	   2,  58,   3,   8,  -2,  57, -58, -60, -59, -57,  -3,  60,  59, -14,   3,  14}
},{
	// MapTab7
	2,  // _eobSym
	38, // _escSym
	// run table
	{1,  1,  0,  2,  2,  1,  1,  3,  3,  4,  4,  5,  5,  1,  1,  6,
	 6,  2,  2,  7,  7,  8,  8,  1,  1,  3,  3,  9,  9, 10, 10,  1,
	 1,  2,  2,  4,  4, 11,  0, 11, 12, 12, 13, 13,  1,  1,  5,  5,
	14, 14, 15, 16, 15, 16,  3,  3,  1,  6,  1,  6,  2,  2,  7,  7,
	 8,  8, 17, 17,  1,  1,  4,  4, 18, 18,  2,  2,  1, 19,  1, 20,
	19, 20, 21, 21,  3,  3, 22, 22,  5,  5, 24,  1,  1, 23,  9, 23,
	24,  9,  2,  2, 10,  1,  1, 10,  6,  6, 25,  4,  4, 25,  7,  7,
	26,  8,  1,  8,  3,  1, 26,  3, 11, 11, 27, 27,  2, 28,  1,  2,
	28,  1, 12, 12,  5,  5, 29, 13, 13, 29, 32,  1,  1, 33, 31, 30,
	32,  4, 30, 33,  4, 31,  3, 14,  1,  1,  3, 34, 34,  2,  2, 14,
	 6,  6, 35, 36, 35, 36,  1, 15,  1, 16, 16, 15,  7,  9,  7,  9,
	37,  8,  8, 37,  1,  1, 39,  2, 38, 39,  2, 40,  5, 38, 40,  5,
	 3,  3,  4,  4, 10, 10,  1,  1,  1,  1, 41,  2, 41,  2,  6,  6,
	 1,  1, 11, 42, 11, 43,  3, 42,  3, 17,  4, 43,  1, 17,  7,  1,
	 8, 44,  4,  7, 44,  5,  8,  2,  5,  1,  2, 48, 45,  1, 12, 45,
	12, 48, 13, 13,  1,  9,  9, 46,  1, 46, 47, 47, 49, 18, 18, 49},

	// value table
	{ 1,  -1,   0,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   1,  -1,   3,  -3,   1,
	 -1,  -2,   2,   1,  -1,   1,  -1,   4,  -4,  -2,   2,   1,  -1,   1,  -1,   5,
	 -5,  -3,   3,   2,  -2,   1,   0,  -1,   1,  -1,   1,  -1,   6,  -6,   2,  -2,
	  1,  -1,   1,   1,  -1,  -1,  -3,   3,   7,   2,  -7,  -2,  -4,   4,   2,  -2,
	  2,  -2,   1,  -1,   8,  -8,   3,  -3,   1,  -1,  -5,   5,   9,   1,  -9,   1,
	 -1,  -1,   1,  -1,  -4,   4,   1,  -1,   3,  -3,   1, -10,  10,   1,   2,  -1,
	 -1,  -2,   6,  -6,   2,  11, -11,  -2,   3,  -3,   1,  -4,   4,  -1,   3,  -3,
	  1,   3,  12,  -3,  -5, -12,  -1,   5,   2,  -2,   1,  -1,  -7,   1,  13,   7,
	 -1, -13,   2,  -2,   4,  -4,   1,   2,  -2,  -1,   1,  14, -14,   1,   1,   1,
	 -1,  -5,  -1,  -1,   5,  -1,  -6,   2, -15,  15,   6,   1,  -1,  -8,   8,  -2,
	 -4,   4,   1,   1,  -1,  -1,  16,   2, -16,  -2,   2,  -2,   4,   3,  -4,  -3,
	 -1,  -4,   4,   1, -17,  17,  -1,  -9,   1,   1,   9,   1,  -5,  -1,  -1,   5,
	 -7,   7,   6,  -6,   3,  -3,  18, -18,  19, -19,   1, -10,  -1,  10,  -5,   5,
	 20, -20,  -3,   1,   3,   1,   8,  -1,  -8,   2,   7,  -1, -21,  -2,   5,  21,
	  5,  -1,  -7,  -5,   1,  -6,  -5, -11,   6,  22,  11,   1,   1, -22,  -3,  -1,
	  3,  -1,   3,  -3, -23,   4,  -4,   1,  23,  -1,   1,  -1,   1,  -2,   2,  -1}
},{
	// MapTab8
	4,  // _eobSym
	11, // _escSym
	// run table
	{1,  1,  1,  1,  0,  2,  2,  1,  1,  3,  3,  0,  1,  1,  2,  2,
	 4,  4,  1,  1,  5,  5,  1,  1,  2,  2,  3,  3,  6,  6,  1,  1,
	 7,  7,  8,  1,  8,  2,  2,  1,  4,  4,  1,  3,  1,  3,  9,  9,
	 2,  2,  1,  5,  1,  5, 10, 10,  1,  1, 11, 11,  3,  6,  3,  4,
	 4,  6,  2,  2,  1, 12,  1, 12,  7, 13,  7, 13,  1,  1,  8,  8,
	 2,  2, 14, 14, 16, 15, 16,  5,  5,  1,  3, 15,  1,  3,  4,  4,
	 1,  1, 17, 17,  2,  2,  6,  6,  1, 18,  1, 18, 22, 21, 22, 21,
	25, 24, 25, 19,  9, 20,  9, 23, 19, 24, 20,  3, 23,  7,  3,  1,
	 1,  7, 28, 26, 29,  5, 28, 26,  5,  8, 29,  4,  8, 27,  2,  2,
	 4, 27,  1,  1, 10, 36, 10, 33, 33, 36, 30,  1, 32, 32,  1, 30,
	 6, 31, 31, 35,  3,  6, 11, 11,  3,  2, 35,  2, 34,  1, 34,  1,
	37, 37, 12,  7, 12,  5, 41,  5,  4,  7,  1,  8, 13,  4,  1, 41,
	13, 38,  8, 38,  9,  1, 40, 40,  9,  1, 39,  2,  2, 49, 39, 42,
	 3,  3, 14, 16, 49, 14, 16, 42, 43, 43,  6,  6, 15,  1,  1, 15,
	44, 44,  1,  1, 50, 48,  4,  5,  4,  7,  5,  2, 10, 10, 48,  7,
	50, 45,  2,  1, 45,  8,  8,  1, 46, 46,  3, 47, 47,  3,  1,  1},

	// value table
	{ 1,  -1,   2,  -2,   0,   1,  -1,   3,  -3,   1,  -1,   0,   4,  -4,   2,  -2,
	  1,  -1,   5,  -5,   1,  -1,   6,  -6,   3,  -3,   2,  -2,   1,  -1,   7,  -7,
	  1,  -1,   1,   8,  -1,   4,  -4,  -8,   2,  -2,   9,   3,  -9,  -3,   1,  -1,
	  5,  -5,  10,   2, -10,  -2,   1,  -1,  11, -11,   1,  -1,  -4,   2,   4,   3,
	 -3,  -2,   6,  -6,  12,   1, -12,  -1,   2,   1,  -2,  -1,  13, -13,   2,  -2,
	  7,  -7,   1,  -1,   1,   1,  -1,   3,  -3,  14,   5,  -1, -14,  -5,   4,  -4,
	 15, -15,   1,  -1,   8,  -8,  -3,   3,  16,   1, -16,  -1,   1,   1,  -1,  -1,
	  1,   1,  -1,   1,   2,   1,  -2,   1,  -1,  -1,  -1,   6,  -1,   3,  -6,  17,
	-17,  -3,   1,   1,   1,   4,  -1,  -1,  -4,   3,  -1,   5,  -3,  -1,  -9,   9,
	 -5,   1,  18, -18,   2,   1,  -2,   1,  -1,  -1,   1,  19,  -1,   1, -19,  -1,
	  4,   1,  -1,   1,   7,  -4,  -2,   2,  -7,  10,  -1, -10,   1,  20,  -1, -20,
	  1,  -1,   2,   4,  -2,   5,   1,  -5,   6,  -4,  21,   4,   2,  -6, -21,  -1,
	 -2,   1,  -4,  -1,  -3,  22,  -1,   1,   3, -22,  -1,  11, -11,   1,   1,   1,
	  8,  -8,   2,   2,  -1,  -2,  -2,  -1,   1,  -1,  -5,   5,   2,  23, -23,  -2,
	  1,  -1,  24, -24,  -1,  -1,   7,   6,  -7,   5,  -6,  12,  -3,   3,   1,  -5,
	  1,   1, -12,  25,  -1,  -5,   5, -25,  -1,   1,   9,   1,  -1,  -9,  26, -26}
}
};

} // End of namespace Indeo
} // End of namespace Image
