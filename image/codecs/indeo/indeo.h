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
#include "graphics/surface.h"
#include "image/codecs/codec.h"

/* Common structures, macros, and base class shared by both Indeo4 and
 * Indeo5 decoders, derived from ffmpeg. We don't currently support Indeo5
 * decoding, but just in case we eventually need it, this is kept as a separate
 * file like it is in ffmpeg.
 *
 * Original copyright note: * Intel Indeo 4 (IV41, IV42, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#ifndef IMAGE_CODECS_INDEO_INDEO_H
#define IMAGE_CODECS_INDEO_INDEO_H

#include "image/codecs/indeo/get_bits.h"
#include "image/codecs/indeo/vlc.h"

namespace Image {
namespace Indeo {

/**
 *  Indeo 4 frame types.
 */
enum {
	IVI4_FRAMETYPE_INTRA       = 0,
	IVI4_FRAMETYPE_INTRA1      = 1,  ///< intra frame with slightly different bitstream coding
	IVI4_FRAMETYPE_INTER       = 2,  ///< non-droppable P-frame
	IVI4_FRAMETYPE_BIDIR       = 3,  ///< bidirectional frame
	IVI4_FRAMETYPE_INTER_NOREF = 4,  ///< droppable P-frame
	IVI4_FRAMETYPE_NULL_FIRST  = 5,  ///< empty frame with no data
	IVI4_FRAMETYPE_NULL_LAST   = 6   ///< empty frame with no data
};

enum {
	IVI_MB_HUFF   = 0,      /// Huffman table is used for coding macroblocks
	IVI_BLK_HUFF  = 1       /// Huffman table is used for coding blocks
};

/**
 *  Declare inverse transform function types
 */
typedef void (InvTransformPtr)(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags);
typedef void (DCTransformPtr)(const int32 *in, int16 *out, uint32 pitch, int blkSize);

typedef void (*IviMCFunc)(int16 *buf, const int16 *refBuf, uint32 pitch, int mcType);
typedef void (*IviMCAvgFunc)(int16 *buf, const int16 *refBuf1, const int16 *refBuf2,
	uint32 pitch, int mcType, int mcType2);

///< max number of bits of the ivi's huffman codes
#define IVI_VLC_BITS 13
#define IVI5_IS_PROTECTED 0x20

/**
 * convert unsigned values into signed ones (the sign is in the LSB)
 */
#define IVI_TOSIGNED(val) (-(((val) >> 1) ^ -((val) & 1)))

/**
 * calculate number of macroblocks in a tile
 */
#define IVI_MBs_PER_TILE(tileWidth, tileHeight, mbSize) \
	((((tileWidth) + (mbSize) - 1) / (mbSize)) * (((tileHeight) + (mbSize) - 1) / (mbSize)))

/**
 *  huffman codebook descriptor
 */
struct IVIHuffDesc {
	int32		_numRows;
	uint8		_xBits[16];

	/**
	 *  Generate a huffman codebook from the given descriptor
	 *  and convert it into the FFmpeg VLC table.
	 *
	 *  @param[out]  vlc	Where to place the generated VLC table
	 *  @param[in]   flag	Flag: true - for static or false for dynamic tables
	 *  @returns	result code: 0 - OK, -1 = error (invalid codebook descriptor)
	 */
	int createHuffFromDesc(VLC *vlc, bool flag) const;

	/**
	 *  Compare two huffman codebook descriptors.
	 *
	 *  @param[in]  desc2	Ptr to the 2nd descriptor to compare
	 *  @returns	comparison result: 0 - equal, 1 - not equal
	 */
	bool huffDescCompare(const IVIHuffDesc *desc2) const;

	/**
	 *  Copy huffman codebook descriptors.
	 *
	 *  @param[in]   src	ptr to the source descriptor
	 */
	void huffDescCopy(const IVIHuffDesc *src);
};

struct IVI45DecContext;

/**
 *  Macroblock/block huffman table descriptor
 */
struct IVIHuffTab {
public:
	int32	_tabSel;	/// index of one of the predefined tables
						/// or "7" for custom one
	VLC *	_tab;		/// pointer to the table associated with tab_sel

	/// the following are used only when tab_sel == 7
	IVIHuffDesc _custDesc;	/// custom Huffman codebook descriptor
	VLC         _custTab;	/// vlc table for custom codebook

	/**
	 * Constructor
	 */
	IVIHuffTab();

	/**
	 *  Decode a huffman codebook descriptor from the bitstream
	 *  and select specified huffman table.
	 *
	 *  @param[in]		ctx			Decoder context
	 *  @param[in]      descCoded	Flag signalling if table descriptor was coded
	 *  @param[in]      whichTab	Codebook purpose (IVI_MB_HUFF or IVI_BLK_HUFF)
	 *  @returns	Zero on success, negative value otherwise
	 */
	int decodeHuffDesc(IVI45DecContext *ctx, int descCoded, int whichTab);
};

/**
 *  run-value (RLE) table descriptor
 */
struct RVMapDesc {
	uint8     _eobSym; ///< end of block symbol
	uint8     _escSym; ///< escape symbol
	uint8     _runtab[256];
	int8      _valtab[256];
};

/**
 *  information for Indeo macroblock (16x16, 8x8 or 4x4)
 */
struct IVIMbInfo {
	int16	_xPos;
	int16	_yPos;
	uint32	_bufOffs;	///< address in the output buffer for this mb
	uint8	_type;		///< macroblock type: 0 - INTRA, 1 - INTER
	uint8	_cbp;		///< coded block pattern
	int8	_qDelta;	///< quant delta
	int8	_mvX;		///< motion vector (x component)
	int8	_mvY;		///< motion vector (y component)
	int8	_bMvX;		///< second motion vector (x component)
	int8	_bMvY;		///< second motion vector (y component)

	IVIMbInfo();
};

/**
 *  information for Indeo tile
 */
struct IVITile {
	int			_xPos;
	int			_yPos;
	int			_width;
	int			_height;
	int			_mbSize;
	bool		_isEmpty;
	int			_dataSize;	///< size of the data in bytes
	int			_numMBs;	///< number of macroblocks in this tile
	IVIMbInfo *	_mbs;		///< array of macroblock descriptors
	IVIMbInfo *	_refMbs;	///< ptr to the macroblock descriptors of the reference tile

	IVITile();
};

/**
 *  information for Indeo wavelet band
 */
struct IVIBandDesc {
	int				_plane;			///< plane number this band belongs to
	int				_bandNum;		///< band number
	int				_width;
	int				_height;
	int				_aHeight;		///< aligned band height
	const uint8 *	_dataPtr;		///< ptr to the first byte of the band data
	int				_dataSize;		///< size of the band data
	int16 *			_buf;			///< pointer to the output buffer for this band
	int16 *			_refBuf;		///< pointer to the reference frame buffer (for motion compensation)
	int16 *			_bRefBuf;		///< pointer to the second reference frame buffer (for motion compensation)
	int16 *			_bufs[4];		///< array of pointers to the band buffers
	int				_pitch;			///< _pitch associated with the buffers above
	bool			_isEmpty;
	int				_mbSize;		///< macroblock size
	int				_blkSize;		///< block size
	uint8			_isHalfpel;		///< precision of the motion compensation: 0 - fullpel, 1 - halfpel
	bool			_inheritMv;		///< tells if motion vector is inherited from reference macroblock
	bool			_inheritQDelta;	///< tells if quantiser delta is inherited from reference macroblock
	bool			_qdeltaPresent;	///< tells if Qdelta signal is present in the bitstream (Indeo5 only)
	int				_quantMat;		///< dequant matrix index
	int				_globQuant;		///< quant base for this band
	const uint8 *	_scan;			///< ptr to the scan pattern
	int				_scanSize;		///< size of the scantable

	IVIHuffTab		_blkVlc;		///< vlc table for decoding block data

	int				_numCorr;		///< number of correction entries
	uint8			_corr[61 * 2];	///< rvmap correction pairs
	int				_rvmapSel;		///< rvmap table selector
	RVMapDesc *		_rvMap;			///< ptr to the RLE table for this band
	int				_numTiles;		///< number of tiles in this band
	IVITile *		_tiles;			///< array of tile descriptors
	InvTransformPtr *_invTransform;
	int				_transformSize;
	DCTransformPtr *_dcTransform;
	bool			_is2dTrans;
	int32			_checksum;		///< for debug purposes
	int				_checksumPresent;
	int				_bufSize;		///< band buffer size in bytes
	const uint16 *	_intraBase;		///< quantization matrix for intra blocks
	const uint16 *	_interBase;		///< quantization matrix for inter blocks
	const uint8 *	_intraScale;	///< quantization coefficient for intra blocks
	const uint8 *	_interScale;	///< quantization coefficient for inter blocks

	IVIBandDesc();

	int initTiles(IVITile *refTile, int p, int b, int tHeight, int tWidth);
};

struct IVIPicConfig {
	uint16		_picWidth;
	uint16		_picHeight;
	uint16		_chromaWidth;
	uint16		_chromaHeight;
	uint16		_tileWidth;
	uint16		_tileHeight;
	uint8		_lumaBands;
	uint8		_chromaBands;

	IVIPicConfig();

	/**
	 * Compare some properties of two pictures
	 */
	bool ivi_pic_config_cmp(const IVIPicConfig &cfg2);
};

/**
 *  color plane (luma or chroma) information
 */
struct IVIPlaneDesc {
	uint16			_width;
	uint16			_height;
	uint8			_numBands;	///< number of bands this plane subdivided into
	IVIBandDesc *	_bands;		///< array of band descriptors

	IVIPlaneDesc();

	static int initPlanes(IVIPlaneDesc *planes, const IVIPicConfig *cfg, bool isIndeo4);

	static int initTiles(IVIPlaneDesc *planes, int tileWidth, int tileHeight);

	/**
	 *  Free planes, bands and macroblocks buffers.
	 *
	 *  @param[in]  planes  pointer to the array of the plane descriptors
	 */
	static void freeBuffers(IVIPlaneDesc *planes);

	/**
	 * Check if the given dimension of an image is valid, meaning that all
	 * bytes of the image can be addressed with a signed int.
	 *
	 * @param w the width of the picture
	 * @param h the height of the picture
	 * @param log_offset the offset to sum to the log level for logging with log_ctx
	 * @returns >= 0 if valid, a negative error code otherwise
	 */
	static int checkImageSize(unsigned int w, unsigned int h, int logOffset);
};

struct AVFrame {
	/**
	 * Dimensions
	 */
	int _width, _height;

#define AV_NUM_DATA_POINTERS 3
	/**
	 * pointer to the picture/channel planes.
	 * This might be different from the first allocated byte
	 *
	 * Some decoders access areas outside 0,0 - width,height, please
	 * see avcodec_align_dimensions2(). Some filters and swscale can read
	 * up to 16 bytes beyond the planes, if these filters are to be used,
	 * then 16 extra bytes must be allocated.
	 *
	 * NOTE: Except for hwaccel formats, pointers not needed by the format
	 * MUST be set to NULL.
	 */
	uint8 *_data[AV_NUM_DATA_POINTERS];

	/**
	 * For video, size in bytes of each picture line.
	 * For audio, size in bytes of each plane.
	 *
	 * For audio, only linesize[0] may be set. For planar audio, each channel
	 * plane must be the same size.
	 *
	 * For video the linesizes should be multiples of the CPUs alignment
	 * preference, this is 16 or 32 for modern desktop CPUs.
	 * Some code requires such alignment other code can be slower without
	 * correct alignment, for yet other it makes no difference.
	 *
	 * @note The linesize may be larger than the size of usable data -- there
	 * may be extra padding present for performance reasons.
	 */
	int _linesize[AV_NUM_DATA_POINTERS];

	/**
	 * Constructor
	 */
	AVFrame();

	/**
	 * Destructor
	 */
	~AVFrame() { freeFrame(); }

	/**
	 * Sets the frame dimensions
	 */
	int setDimensions(uint16 width, uint16 height);

	/**
	 * Get a buffer for a frame
	 */
	int getBuffer(int flags);

	/**
	 * Frees any data loaded for the frame
	 */
	void freeFrame();
};

struct IVI45DecContext {
	friend struct IVIHuffTab;
private:
	VLC_TYPE _tableData[8192 * 16][2];
	VLC _iviMbVlcTabs[8];			///< static macroblock Huffman tables
	VLC _iviBlkVlcTabs[8];			///< static block Huffman tables
public:
	GetBits *		_gb;
	RVMapDesc		_rvmapTabs[9];	///< local corrected copy of the static rvmap tables

	uint32			_frameNum;
	int				_frameType;
	int				_prevFrameType;	///< frame type of the previous frame
	uint32			_dataSize;		///< size of the frame data in bytes from picture header
	int				_isScalable;
	const uint8 *	_frameData;		///< input frame data pointer
	int				_interScal;		///< signals a sequence of scalable inter frames
	uint32			_frameSize;		///< frame size in bytes
	uint32			_picHdrSize;	///< picture header size in bytes
	uint8			_frameFlags;
	uint16			_checksum;		///< frame _checksum

	IVIPicConfig	_picConf;
	IVIPlaneDesc	_planes[3];		///< color planes

	int				_bufSwitch;		///< used to switch between three buffers
	int				_dstBuf;		///< buffer index for the currently decoded frame
	int				_refBuf;		///< inter frame reference buffer index
	int				_ref2Buf;		///< temporal storage for switching buffers
	int				_bRefBuf;		///< second reference frame buffer index

	IVIHuffTab		_mbVlc;			///< current macroblock table descriptor
	IVIHuffTab		_blkVlc;		///< current block table descriptor
	IVIHuffTab		_transVlc;		///< current transparency table descriptor

	uint8			_rvmapSel;
	bool			_inImf;
	bool			_inQ;			///< flag for explicitly stored quantiser delta
	uint8			_picGlobQuant;
	uint8			_unknown1;

	uint16			_gopHdrSize;
	uint8			_gopFlags;
	uint32			_lockWord;

	bool			_hasBFrames;
	bool			_hasTransp;		///< transparency mode enabled
	bool			_usesTiling;
	bool			_usesHaar;
	bool			_usesFullpel;

	bool			_gopInvalid;
	int				_bufInvalid[4];

	bool			_isIndeo4;
	uint32			_transKeyColor;

	AVFrame *		_pFrame;
	bool			_gotPFrame;

	IVI45DecContext();
private:
	/**
	 *  Initial Run-value (RLE) tables.
	 */
	static const RVMapDesc _ff_ivi_rvmap_tabs[9];
};

class IndeoDecoderBase : public Codec {
private:
	/**
	 *  Decode an Indeo 4 or 5 band.
	 *
	 *  @param[in,out]  band   ptr to the band descriptor
	 *  @returns        result code: 0 = OK, -1 = error
	 */
	int decode_band(IVIBandDesc *band);

	/**
	 *  Haar wavelet recomposition filter for Indeo 4
	 *
	 *  @param[in]  plane		Pointer to the descriptor of the plane being processed
	 *  @param[out] dst			pointer to the destination buffer
	 *  @param[in]  dstPitch	Pitch of the destination buffer
	 */
	void recomposeHaar(const IVIPlaneDesc *plane, uint8 *dst, const int dstPitch);

	/**
	 *  5/3 wavelet recomposition filter for Indeo5
	 *
	 *  @param[in]   plane        Pointer to the descriptor of the plane being processed
	 *  @param[out]  dst          Pointer to the destination buffer
	 *  @param[in]   dstPitch     Pitch of the destination buffer
	 */
	void recompose53(const IVIPlaneDesc *plane, uint8 *dst, const int dstPitch);

	/*
	 *  Convert and output the current plane.
	 *  This conversion is done by adding back the bias value of 128
	 *  (subtracted in the encoder) and clipping the result.
	 *
	 *  @param[in]   plane		Pointer to the descriptor of the plane being processed
	 *  @param[out]  dst		Pointer to the buffer receiving converted pixels
	 *  @param[in]   dstPitch	Pitch for moving to the next y line
	 */
	void outputPlane(IVIPlaneDesc *plane, uint8 *dst, int dstPitch);

	/**
	 *  Handle empty tiles by performing data copying and motion
	 *  compensation respectively.
	 *
	 *  @param[in]  band		Pointer to the band descriptor
	 *  @param[in]  tile		Pointer to the tile descriptor
	 *  @param[in]  mvScale		Scaling factor for motion vectors
	 */
	int processEmptyTile(IVIBandDesc *band, IVITile *tile, int32 mvScale);

	/*
	 *  Decode size of the tile data.
	 *  The size is stored as a variable-length field having the following format:
	 *  if (tile_data_size < 255) than this field is only one byte long
	 *  if (tile_data_size >= 255) than this field four is byte long: 0xFF X1 X2 X3
	 *  where X1-X3 is size of the tile data
	 *
	 *  @param[in,out]  gb  the GetBit context
	 *  @returns	Size of the tile data in bytes
	 */
	int decodeTileDataSize(GetBits *gb);

	/*
	 *  Decode block data:
	 *  extract huffman-coded transform coefficients from the bitstream,
	 *  dequantize them, apply inverse transform and motion compensation
	 *  in order to reconstruct the picture.
	 *
	 *  @param[in,out]  gb		The GetBit context
	 *  @param[in]      band	Pointer to the band descriptor
	 *  @param[in]      tile	Pointer to the tile descriptor
	 *  @returns	Result code: 0 - OK, -1 = error (corrupted blocks data)
	 */
	int decodeBlocks(GetBits *gb, IVIBandDesc *band, IVITile *tile);

	int iviMc(IVIBandDesc *band, IviMCFunc mc, IviMCAvgFunc mcAvg,
		int offs, int mvX, int mvY, int mvX2, int mvY2, int mcType, int mcType2);

	int decodeCodedBlocks(GetBits *gb, IVIBandDesc *band,
		IviMCFunc mc, IviMCAvgFunc mcAvg, int mvX, int mvY,
		int mvX2, int mvY2, int32 *prevDc, int isIntra,
		int mcType, int mcType2, uint32 quant, int offs);

	int iviDcTransform(IVIBandDesc *band, int32 *prevDc, int bufOffs,
		int blkSize);
protected:
	IVI45DecContext _ctx;
	Graphics::PixelFormat _pixelFormat;
	Graphics::Surface _surface;

	/**
	 *  Scan patterns shared between indeo4 and indeo5
	 */
	static const uint8 _ffIviVerticalScan8x8[64];
	static const uint8 _ffIviHorizontalScan8x8[64];
	static const uint8 _ffIviDirectScan4x4[16];
protected:
	/**
	 * Returns the pixel format for the decoder's surface
	 */
	virtual Graphics::PixelFormat getPixelFormat() const { return _pixelFormat; }

	/**
	 * Decode the Indeo picture header.
	 * @returns		0 = Ok, negative number = error
	 */
	virtual int decodePictureHeader() = 0;

	/**
	 *  Rearrange decoding and reference buffers.
	 */
	virtual void switchBuffers() = 0;

	virtual bool isNonNullFrame() const = 0;

	/**
	 *  Decode Indeo band header.
	 *
	 *  @param[in,out] band		Pointer to the band descriptor
	 *  @returns	Result code: 0 = OK, negative number = error
	 */
	virtual int decodeBandHeader(IVIBandDesc *band) = 0;

	/**
	*  Decode information (block type, _cbp, quant delta, motion vector)
	*  for all macroblocks in the current tile.
	*
	*  @param[in,out] band		Pointer to the band descriptor
	*  @param[in,out] tile		Pointer to the tile descriptor
	*  @returns		Result code: 0 = OK, negative number = error
	*/
	virtual int decodeMbInfo(IVIBandDesc *band, IVITile *tile) = 0;

	/**
	 * Decodes optional transparency data within Indeo frames
	 */
	virtual int decodeTransparency() { return -1; }

	/**
	 * Decodes the Indeo frame from the bit reader already
	 * loaded into the context
	 */
	int decodeIndeoFrame();

	/**
	 * scale motion vector
	 */
	int scaleMV(int mv, int mvScale);
public:
	IndeoDecoderBase(uint16 width, uint16 height, uint bitsPerPixel);
	virtual ~IndeoDecoderBase();
};

} // End of namespace Indeo
} // End of namespace Image

#endif
