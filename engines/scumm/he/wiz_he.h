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

#if !defined(SCUMM_HE_WIZ_HE_H) && defined(ENABLE_HE)
#define SCUMM_HE_WIZ_HE_H

//#define WIZ_DEBUG_BUFFERS

#include "common/rect.h"

namespace Scumm {

#define WIZBLOCK_WIZH_DATA_SIZE (4 * 3)
#define WIZBLOCK_RGBS_DATA_SIZE (768)
#define WIZBLOCK_SPOT_DATA_SIZE (4 * 2)
#define WIZBLOCK_RMAP_DATA_SIZE (256 + 4)

#define WIZBLOCK_WIZH_SIZE    (8 + (WIZBLOCK_WIZH_DATA_SIZE))
#define WIZBLOCK_RGBS_SIZE    (8 + (WIZBLOCK_RGBS_DATA_SIZE))
#define WIZBLOCK_SPOT_SIZE    (8 + (WIZBLOCK_SPOT_DATA_SIZE))
#define WIZBLOCK_RMAP_SIZE    (8 + (WIZBLOCK_RMAP_DATA_SIZE))

#define DW_LOAD_SUCCESS          0
#define DW_LOAD_NOT_TYPE        -1
#define DW_LOAD_READ_FAILURE    -2
#define DW_LOAD_OPEN_FAILURE    -3

#define DW_SAVE_SUCCESS            0
#define DW_SAVE_NOT_TYPE          -1
#define DW_SAVE_WRITE_FAILURE     -2
#define DW_SAVE_CREATE_FAILURE    -3

#define DW_SAVE_WIZ_FORMAT    0
#define DW_SAVE_PCX_FORMAT    1
#define DW_SAVE_RAW_FORMAT    2

#if defined SCUMM_LITTLE_ENDIAN
#define NATIVE_WIZ_COMP_NONE_16BPP kWCTNone16Bpp
#define NATIVE_WIZ_COMP_TRLE_16BPP kWCTTRLE16Bpp

#elif defined SCUMM_BIG_ENDIAN
#define NATIVE_WIZ_COMP_NONE_16BPP kWCTNone16BppBigEndian
#define NATIVE_WIZ_COMP_TRLE_16BPP kWCTTRLE16BppBigEndian
#endif

#define LITTLE_ENDIAN_WIZ(wizComp)    \
	    (wizComp) == kWCTNone16Bpp || \
		(wizComp) == kWCTTRLE16Bpp

#define NATIVE_WIZ_TYPE(wizComp)                   \
	(wizComp) == NATIVE_WIZ_COMP_NONE_16BPP || \
		(wizComp) == NATIVE_WIZ_COMP_TRLE_16BPP    \

#define WIZ_16BPP(wizComp)                       \
		(wizComp) == kWCTNone16Bpp ||            \
		(wizComp) == kWCTTRLE16Bpp ||            \
		(wizComp) == kWCTNone16BppBigEndian ||   \
		(wizComp) == kWCTTRLE16BppBigEndian

#define WIZ_MAGIC_REMAP_NUMBER  0x76543210

#define WIZRAWPIXEL_R_MASK    (_uses16BitColor ? 0x7C00 : 0xFF)
#define WIZRAWPIXEL_G_MASK    (_uses16BitColor ? 0x03E0 : 0xFF)
#define WIZRAWPIXEL_B_MASK    (_uses16BitColor ? 0x001F : 0xFF)

#define WIZRAWPIXEL_R_SHIFT   (_uses16BitColor ? 10 : 0)
#define WIZRAWPIXEL_G_SHIFT   (_uses16BitColor ? 5  : 0)
#define WIZRAWPIXEL_B_SHIFT   (0)

#define WIZRAWPIXEL_MASK   (_uses16BitColor ? 0xFFFF : 0xFF)

#define WIZRAWPIXEL_LO_R_BIT   (1 << WIZRAWPIXEL_R_SHIFT)
#define WIZRAWPIXEL_LO_G_BIT   (1 << WIZRAWPIXEL_G_SHIFT)
#define WIZRAWPIXEL_LO_B_BIT   (1 << WIZRAWPIXEL_B_SHIFT)
#define WIZRAWPIXEL_LO_BITS    ((WIZRAWPIXEL_LO_R_BIT) | (WIZRAWPIXEL_LO_G_BIT) | (WIZRAWPIXEL_LO_B_BIT))
#define WIZRAWPIXEL_HI_BITS    ~WIZRAWPIXEL_LO_BITS

#define WIZ_COLOR16_COMPONENT_COUNT   (1 << 5)
#define WIZ_QUANTIZED_ALPHA_COUNT     16
#define WIZ_QUANTIZED_ALPHA_DIV       ((256) / (WIZ_QUANTIZED_ALPHA_COUNT))

#define WIZRAWPIXEL_50_50_PREMIX_COLOR(__rawColor__)    (((__rawColor__) & WIZRAWPIXEL_HI_BITS) >> 1)
#define WIZRAWPIXEL_50_50_MIX(__colorA__, __colorB__)   ((__colorA__) + (__colorB__))

#define WIZRAWPIXEL_ADDITIVE_MIX(__colorA__, __colorB__)                                                         \
	(MIN<int>(WIZRAWPIXEL_R_MASK, (((__colorA__) & WIZRAWPIXEL_R_MASK) + ((__colorB__) & WIZRAWPIXEL_R_MASK))) | \
	 MIN<int>(WIZRAWPIXEL_G_MASK, (((__colorA__) & WIZRAWPIXEL_G_MASK) + ((__colorB__) & WIZRAWPIXEL_G_MASK))) | \
	 MIN<int>(WIZRAWPIXEL_B_MASK, (((__colorA__) & WIZRAWPIXEL_B_MASK) + ((__colorB__) & WIZRAWPIXEL_B_MASK))))

#define WIZRAWPIXEL_SUBTRACTIVE_MIX(__colorA__, __colorB__)                                                        \
	(MAX<int>(WIZRAWPIXEL_LO_R_BIT, (((__colorA__) & WIZRAWPIXEL_R_MASK) - ((__colorB__) & WIZRAWPIXEL_R_MASK))) | \
	 MAX<int>(WIZRAWPIXEL_LO_G_BIT, (((__colorA__) & WIZRAWPIXEL_G_MASK) - ((__colorB__) & WIZRAWPIXEL_G_MASK))) | \
	 MAX<int>(WIZRAWPIXEL_LO_B_BIT, (((__colorA__) & WIZRAWPIXEL_B_MASK) - ((__colorB__) & WIZRAWPIXEL_B_MASK))))


typedef uint16 WizRawPixel;
typedef uint8  WizRawPixel8;
typedef uint16 WizRawPixel16;

struct WizPolygon {
	Common::Point points[5];
	Common::Rect boundingRect;
	int id;
	int numPoints;
	bool flag;

	void reset() {
		for (int i = 0; i < ARRAYSIZE(points); i++) {
			points[i].x = points[i].y = 0;
		}
		boundingRect.top = boundingRect.left = boundingRect.bottom = boundingRect.right = 0;
		id = 0;
		numPoints = 0;
		flag = 0;
	}
};

struct WizBufferElement {
	int image;
	int x;
	int y;
	int z;
	int state;
	int flags;
	int shadow;
	int zbuffer;
	int palette;
};

struct WizFontProperties {
	byte string[4096];
	byte fontName[4096];
	int fgColor;
	int bgColor;
	int style;
	int size;
	int xPos;
	int yPos;
};

struct WizEllipseProperties {
	int px;
	int py;
	int qx;
	int qy;
	int kx;
	int ky;
	int lod;
	WizRawPixel color;
};

struct WizExtendedRenderInfo {
	int32 blendFlags;
	int32 sprite;
	int32 group;
	int32 conditionBits;
};

struct WizImageCommand {
	int dwSize;
	byte filename[260];
	Common::Rect box;
	int actionFlags;
	int actionType;
	int params[8];
	int flags;
	int xPos;
	int yPos;
	int zPos;
	int compressionType;
	int fileType;
	int angle;
	int state;
	int scale;
	int shadow;
	int polygon;
	int polygon2;
	int image;
	int width;
	int height;
	int palette;
	int sourceImage;
	int propertyValue;
	int propertyNumber;
	uint8 remapTable[256];
	uint8 remapList[256];
	int remapCount;
	int destImageNumber;
	int zbufferImage;
	WizRawPixel colorValue;
	WizFontProperties fontProperties;
	WizEllipseProperties ellipseProperties;
	Common::Rect renderCoords;
	WizExtendedRenderInfo extendedRenderInfo;

	void reset() {
		dwSize = 0;
		memset(filename, 0, sizeof(filename));
		box.top = box.left = box.bottom = box.right = 0;
		actionFlags = 0;
		actionType = 0;
		memset(params, 0, sizeof(params));
		image = 0;
		xPos = 0;
		yPos = 0;
		zPos = 0;
		state = 0;
		flags = 0;
		shadow = 0;
		zbufferImage = 0;
		palette = 0;
		compressionType = 0;
		fileType = 0;
		angle = 0;
		scale = 0;
		polygon = 0;
		polygon2 = 0;
		width = 0;
		height = 0;
		sourceImage = 0;
		propertyValue = 0;
		propertyNumber = 0;
		memset(remapTable, 0, sizeof(remapTable));
		memset(remapList, 0, sizeof(remapList));
		remapCount = 0;
		destImageNumber = 0;
		colorValue = 0;
		memset(&fontProperties, 0, sizeof(WizFontProperties));
		memset(&ellipseProperties, 0, sizeof(WizEllipseProperties));
		renderCoords.left = renderCoords.top = renderCoords.bottom = renderCoords.right = 0;
		memset(&extendedRenderInfo, 0, sizeof(WizExtendedRenderInfo));
	}
};

struct FloodFillCommand {
	Common::Rect box;
	int32 x;
	int32 y;
	int32 flags;
	int32 color;

	void reset() {
		box.top = box.left = box.bottom = box.right = 0;
		x = 0;
		y = 0;
		flags = 0;
		color = 0;
	}
};

#ifdef WIZ_DEBUG_BUFFERS
#define WizPxShrdBufferD(a, b) WizPxShrdBuffer(a, b, __FUNCTION__, __LINE__)
struct DbgEntry {
	DbgEntry(const void *a, const char *lpr, int ln) : addr(a), msg(Common::String::format("buffer allocated in: %s(), line %d", lpr, ln)) {}
	bool operator==(const void *ptr) const { return addr == ptr; }
	const void *addr;
	Common::String msg;
};
#else
#define WizPxShrdBufferD(a, b) WizPxShrdBuffer(a, b)
#endif

class WizPxShrdBuffer {
public:
#ifdef WIZ_DEBUG_BUFFERS
	static void dbgLeakRpt() {
		if (!_allocLocs)
			return;
		for (Common::Array<DbgEntry>::iterator i = _allocLocs->begin(); i != _allocLocs->end(); ++i)
			debug("Leaked: %s", i->msg.c_str());
		_allocLocs->clear();
		delete _allocLocs;
		_allocLocs = nullptr;
	}
	static Common::Array<DbgEntry> *_allocLocs;
#endif
	WizPxShrdBuffer() : _buff(nullptr), _lifes(nullptr), _xstart(0) {}
#ifdef WIZ_DEBUG_BUFFERS
	WizPxShrdBuffer(void *buff, bool hasOwnerShip) : WizPxShrdBuffer(buff, hasOwnerShip, 0, 0) {}
	WizPxShrdBuffer(void *buff, bool hasOwnerShip, const char *func, int line)
#else
	WizPxShrdBuffer(void *buff, bool hasOwnerShip)
#endif
		: _buff(reinterpret_cast<WizRawPixel *>(buff)), _lifes(nullptr), _xstart(0) {
		if (hasOwnerShip) {
#ifdef WIZ_DEBUG_BUFFERS
			if (!_allocLocs)
				_allocLocs = new Common::Array<DbgEntry>();
			_allocLocs->push_back(DbgEntry(buff, func, line));
#endif
			*(_lifes = new int) = 1;
		}
	}
	WizPxShrdBuffer(const WizPxShrdBuffer &other) : _buff(other._buff), _lifes(other._lifes), _xstart(other._xstart) {
		if (_lifes)
			++*_lifes;
	}
	WizPxShrdBuffer(WizPxShrdBuffer &&other) noexcept : _buff(other._buff), _lifes(other._lifes), _xstart(other._xstart) {
		other._lifes = nullptr;
	}
	~WizPxShrdBuffer() {
		dcrlifes();
	}
	WizRawPixel *operator()() const { return (WizRawPixel*)((byte*)_buff + _xstart); }
	WizPxShrdBuffer &operator=(const WizPxShrdBuffer &other) {
		if (this != &other) {
			dcrlifes();
			if ((_lifes = other._lifes) != nullptr) ++*_lifes;
			_buff = other._buff;
			_xstart = other._xstart;
		}
		return *this;
	}
	WizPxShrdBuffer &&operator=(WizPxShrdBuffer &&other) {
		dcrlifes();
		_lifes = other._lifes;
		_buff = other._buff;
		_xstart = other._xstart;
		other._lifes = nullptr;
		return Common::move(*this);
	}
	WizPxShrdBuffer &operator+(int bytes) {
		_xstart += bytes;
		return *this;
	}
	WizPxShrdBuffer &operator+=(int bytes) { return operator+(bytes); }
	bool operator==(WizRawPixel *ptr) const { return _buff == ptr; }
private:
	void dcrlifes() {
		if (_lifes && !--*_lifes) {
			free(_buff);
#ifdef WIZ_DEBUG_BUFFERS
			Common::Array<DbgEntry>::iterator i = Common::find(_allocLocs->begin(), _allocLocs->end(), _buff);
			if (i != _allocLocs->end())
				_allocLocs->erase(i);
#endif
		}
		if (_lifes && !*_lifes)
			delete _lifes;
	}
	WizRawPixel *_buff;
	int *_lifes;
	int _xstart;
};

struct WizSimpleBitmap {
	WizPxShrdBuffer bufferPtr;
	int32 bitmapWidth;
	int32 bitmapHeight;

	WizSimpleBitmap() : bufferPtr(), bitmapWidth(0), bitmapHeight(0) {

	}
};

struct WizMultiTypeBitmap {
	byte *data;
	int32 width;
	int32 height;
	int32 stride;
	int32 format;
	int32 bpp;
};

struct WizRawBitmap {
	int32 width;
	int32 height;
	int32 dataSize;
	WizRawPixel16 *data;
};

struct WizImage {
	int dataSize;
	byte *data;
};

struct WizFloodSegment {
	int y, xl, xr, dy;
};

struct WizFloodState {
	WizFloodSegment *stack;
	WizFloodSegment *top;
	WizFloodSegment *sp;
	Common::Rect updateRect;
	Common::Rect clipping;
	WizSimpleBitmap *bitmapPtr;
	int writeOverColor;
	int colorToWrite;
	int boundryColor;
	int numStackElements;
};

struct WizCompressedImage {
	const byte *data;
	int width;
	int height;
};

struct WizMoonbaseCompressedImage {
	int type, size, width, height;
	WizRawPixel16 transparentColor;
	byte *data;
};

struct MoonbaseDistortionInfo {
	byte *srcData;
	int baseX;
	int baseY;
	int srcPitch;
	Common::Rect *clipRect;
};

// Our Common::Point has int16 coordinates.
// This is not enough for polygon warping...
struct WarpWizPoint {
	int32 x;
	int32 y;

	WarpWizPoint(Common::Point pt) {
		x = pt.x;
		y = pt.y;
	}

	WarpWizPoint() {
		x = y = 0;
	}
};

struct WarpWizOneSpan {
	int dstLeft;
	int dstRight;
	WarpWizPoint srcLeft;
	WarpWizPoint srcRight;
};

struct WarpWizOneDrawSpan {
	int dstOffset;
	int xSrcStep;
	int ySrcStep;
	int xSrcOffset;
	int ySrcOffset;
	int dstWidth;
};

struct WarpWizOneSpanTable {
	WarpWizPoint dstMinPt, dstMaxPt;
	WarpWizPoint srcMinPt, srcMaxPt;
	WarpWizOneDrawSpan *drawSpans;
	WarpWizOneSpan *spans;
	int drawSpanCount;
	int spanCount;
};

enum WizRenderingFlags : uint {
	// Standard rendering flags
	kWRFUsePalette = 0x00000001,
	kWRFRemap      = 0x00000002,
	kWRFPrint      = 0x00000004,
	kWRFBackground = 0x00000008,
	kWRFForeground = 0x00000010,
	kWRFAlloc      = 0x00000020,
	kWRFPolygon    = 0x00000040,
	kWRFZPlaneOn   = 0x00000080,
	kWRFZPlaneOff  = 0x00000100,
	kWRFUseShadow  = 0x00000200,
	kWRFHFlip      = 0x00000400,
	kWRFVFlip      = 0x00000800,
	kWRFRotate90   = 0x00001000,

	// Special rendering flags
	kWRFSpecialRenderBitMask         = 0xFFF00000,
	kWRFAdditiveBlend                = 0x00100000,
	kWRFSubtractiveBlend             = 0x00200000,
	kWRF5050Blend                    = 0x00400000,
	kWRFAreaSampleDuringWarp         = 0x00800000,
	kWRFUseSourceImageAsAlphaChannel = 0x01000000,
	kWRFBooleanAlpha                 = 0x02000000,
	kWRFInverseAlpha                 = 0x04000000,
	kWRFUseImageAsAlphaChannel       = 0x08000000,
	kWRFUseBlendPrimitives           = 0x10000000,
	kWRFFutureExpansionBit1          = 0x20000000,
	kWRFFutureExpansionBit2          = 0x40000000,
	kWRFFutureExpansionBit3          = 0x80000000,
};

enum WizActions {
	kWAUnknown         = 0,
	kWADraw            = 1,
	kWACapture         = 2,
	kWALoad            = 3,
	kWASave            = 4,
	kWAGlobalState     = 5,
	kWAModify          = 6,
	kWAPolyCapture     = 7,
	kWANew             = 8,
	kWARenderRectangle = 9,
	kWARenderLine      = 10,
	kWARenderPixel     = 11,
	kWARenderFloodFill = 12,
	kWAFontStart       = 13,
	kWAFontEnd         = 14,
	kWAFontCreate      = 15,
	kWAFontRender      = 16,
	kWARenderEllipse   = 17,
};

enum WizActionFlags {
	kWAFSpot            = 0x00000001,
	kWAFPolygon         = 0x00000002,
	kWAFShadow          = 0x00000004,
	kWAFScale           = 0x00000008,
	kWAFAngle           = 0x00000010,
	kWAFFlags           = 0x00000020,
	kWAFRemapList       = 0x00000040,
	kWAFFileType        = 0x00000080,
	kWAFCompressionType = 0x00000100,
	kWAFRect            = 0x00000200,
	kWAFState           = 0x00000400,
	kWAFFilename        = 0x00000800,
	kWAFPolygon2        = 0x00001000,
	kWAFWidth           = 0x00002000,
	kWAFHeight          = 0x00004000,
	kWAFPalette         = 0x00008000,
	kWAFDestImage       = 0x00010000,
	kWAFColor           = 0x00020000,
	kWAFRenderCoords    = 0x00040000,
	kWAFSourceImage     = 0x00080000,
	kWAFProperty        = 0x00100000,
	kWAFZBufferImage    = 0x00200000
};

enum WizCompositeFlags {
	kWCFConditionBits    = 0x00000001,
	kWCFSubState         = 0x00000002,
	kWCFXDelta           = 0x00000004,
	kWCFYDelta           = 0x00000008,
	kWCFDrawFlags        = 0x00000010,
	kWCFSubConditionBits = 0x00000020
};

enum WizCompressionTypes {
	kWCTNone                      = 0x00000000,
	kWCTTRLE                      = 0x00000001,
	kWCTNone16Bpp                 = 0x00000002,
	// kWCTNone32Bpp              = 0x00000003,
	kWCTComposite                 = 0x00000004,
	kWCTTRLE16Bpp                 = 0x00000005,
	// kWCTTRLE32Bpp              = 0x00000006,
	kWCTMRLEWithLineSizePrefix    = 0x00000007,
	kWCTMRLEWithoutLineSizePrefix = 0x00000008,
	kWCTDataBlockDependent        = 0x00000009,
	kWCTNone16BppBigEndian        = 0x0000000A,
	// kWCTNone32BppBigEndian     = 0x0000000B,
	kWCTTRLE16BppBigEndian        = 0x0000000C,
	// kWCTTRLE32BppBigEndian     = 0x0000000D
};

enum CreateWizFlags {
	kCWFPalette    = 0x00000001,
	kCWFSpot       = 0x00000002,
	kCWFRemapTable = 0x00000008,
	kCWFDefault    = ((kCWFPalette) | (kCWFSpot) | (kCWFRemapTable))
};

enum WizImgProps {
	kWIPCompressionType     = 0x10000000,
	kWIPPaletteBlockPresent = 0x10000001,
	kWIPRemapBlockPresent   = 0x10000002,
	kWIPOpaqueBlockPresent  = 0x10000003,
	kWIPXMAPBlockPresent    = 0x10000004
};

enum WizSpcConditionTypes : uint {
	kWSPCCTBits = 0xC0000000,
	kWSPCCTOr   = 0x00000000,
	kWSPCCTAnd  = 0x40000000,
	kWSPCCTNot  = 0x80000000
};

enum WizMoonSystemBits {
	kWMSBRopMask        = 0x000000FF,
	kWMSBRopParamMask   = 0x0000FF00,
	kWMSBReservedBits   = (kWMSBRopMask | kWMSBRopParamMask),
	kWMSBRopParamRShift = 8
};

enum WizEllipseConstants {
	kWECFixedSize = 16,
	kWECPiOver2 = 102944, // Fixed point PI/2
	kWECHalf = 32768      // Fixed point 1/2
};

enum WizZPlaneOps {
	kWZOIgnore = 0,
	kWZOClear  = 1,
	kWZOSet    = 2
};

enum DstSurface {
	kDstScreen   = 0,
	kDstMemory   = 1,
	kDstResource = 2,
	kDstCursor   = 3
};

class ScummEngine_v71he;

class Wiz {
public:
	enum {
		NUM_POLYGONS = 200,
		NUM_IMAGES   = 255
	};

	WizBufferElement _wizBuffer[NUM_IMAGES] = {};
	uint16 _wizBufferIndex = 0;
	WizPolygon _polygons[NUM_POLYGONS] = {};

	// For collision
	WizRawPixel _compareBufferA[640] = {};
	WizRawPixel _compareBufferB[640] = {};

	Wiz(ScummEngine_v71he *vm);
	~Wiz() {
#ifdef WIZ_DEBUG_BUFFERS
		WizPxShrdBuffer::dbgLeakRpt();
#endif
	}

	void clearWizBuffer();
	Common::Rect _wizClipRect;
	bool _useWizClipRect = false;
	bool _uses16BitColor = false;
	int _wizActiveShadow = 0;

	void deleteLocalPolygons();
	void polygonLoad(const uint8 *polData);
	void set4Polygon(int id, bool flag, int vert1x, int vert1y, int vert2x, int vert2y, int vert3x, int vert3y, int vert4x, int vert4y);
	void polyBuildBoundingRect(Common::Point *vert, int numVerts, Common::Rect & bound);
	void deletePolygon(int fromId, int toId);
	int  testForObjectPolygon(int id, int x, int y);
	int  findPolygon(int x, int y);
	bool doesObjectHavePolygon(int id);
	bool polyIsPointInsidePoly(Common::Point *listOfPoints, int numverts, Common::Point *checkPoint);
	void polyRotatePoints(Common::Point *pts, int num, int alpha);
	void polyMovePolygonPoints(Common::Point *listOfPoints, int numverts, int deltaX, int deltaY);
	bool polyIsRectangle(const Common::Point *points, int numverts);

	void dwCreateRawWiz(int imageNum, int w, int h, int flags, int bitsPerPixel, int optionalSpotX, int optionalSpotY);
	bool dwGetMultiTypeBitmapFromImageState(int imageNum, int imageState, WizMultiTypeBitmap *multiBM);
	bool dwSetSimpleBitmapStructFromImage(int imageNum, int imageState, WizSimpleBitmap *destBM);
	int  dwTryToLoadWiz(Common::SeekableReadStream *inFile, const WizImageCommand *params);
	void dwAltSourceDrawWiz(int maskImage, int maskState, int x, int y, int sourceImage, int sourceState, int32 flags, int paletteNumber, const Common::Rect *optionalClipRect, const WizSimpleBitmap *destBitmapPtr);
	void dwHandleComplexImageDraw(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable);
	bool dwIsMaskCompatibleCompressionType(int compressionType);
	bool dwIsUncompressedFormatTypeID(int id);
	int	 dwGetImageGeneralProperty(int image, int state, int property);

	void processWizImageCmd(const WizImageCommand *params);
	void processWizImageCaptureCmd(const WizImageCommand *params);
	void processWizImagePolyCaptureCmd(const WizImageCommand *params);
	void processWizImageDrawCmd(const WizImageCommand *params);
	void processWizImageRenderRectCmd(const WizImageCommand *params);
	void processWizImageRenderLineCmd(const WizImageCommand *params);
	void processWizImageRenderPixelCmd(const WizImageCommand *params);
	void processWizImageRenderFloodFillCmd(const WizImageCommand *params);
	void processWizImageModifyCmd(const WizImageCommand *params);
	void processWizImageRenderEllipseCmd(const WizImageCommand *params);
	void processWizImageFontStartCmd(const WizImageCommand *params);
	void processWizImageFontEndCmd(const WizImageCommand *params);
	void processWizImageFontCreateCmd(const WizImageCommand *params);
	void processWizImageFontRenderCmd(const WizImageCommand *params);
	void processNewWizImageCmd(const WizImageCommand *params);
	void processWizImageLoadCmd(const WizImageCommand *params);
	void processWizImageSaveCmd(const WizImageCommand *params);

	void getWizImageDim(int resNum, int state, int32 &w, int32 &h);
	void getWizImageDim(uint8 *dataPtr, int state, int32 &w, int32 &h);
	int getWizStateCount(int resnum);
	int getWizImageStates(const uint8 *ptr);
	byte *getWizStateHeaderPrim(int resNum, int state);
	byte *getWizStateDataPrim(int resNum, int state);
	byte *getWizStatePaletteDataPrim(int resNum, int state);
	byte *getWizStateRemapDataPrim(int resNum, int state);
	int getWizCompressionType(int resNum, int state);
	bool doesRawWizStateHaveTransparency(int globNum, int state);
	bool doesStateContainBlock(int globNum, int state, uint32 blockID);
	const byte *getColorMixBlockPtrForWiz(int image);
	void setWizCompressionType(int image, int state, int newType);

	bool isUncompressedFormatTypeID(int id);

	void handleRotate0SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable);
	void handleRotate90SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable);
	void handleRotate180SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable);
	void handleRotate270SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable);

	void flushAWizBuffer();

	void getWizSpot(int resId, int state, int32 &x, int32 &y);
	void getWizSpot(int resId, int32 &x, int32 &y); // HE80
	void getWizImageSpot(uint8 *data, int state, int32 &x, int32 &y);
	void loadWizCursor(int resId, int palette, bool useColor);

	void takeAWiz(int globnum, int x1, int y1, int x2, int y2, bool back, bool compress);
	void simpleDrawAWiz(int image, int state, int x, int y, int flags);
	void bufferAWiz(int image, int state, int x, int y, int z, int flags, int optionalShadowImage, int optionalZBufferImage, int whichPalette);
	WizPxShrdBuffer drawAWiz(int image, int state, int x, int y, int z, int flags, int optionalShadowImage, int optionalZBufferImage,
				   Common::Rect *optionalClipRect, int whichPalette, WizSimpleBitmap *optionalBitmapOverride);
	WizPxShrdBuffer drawAWizEx(int image, int state, int x, int y, int z, int flags, int optionalShadowImage, int optionalZBufferImage, Common::Rect *optionalClipRect,
					 int whichPalette, WizSimpleBitmap *optionalBitmapOverride, const WizImageCommand *optionalICmdPtr);
	WizPxShrdBuffer drawAWizPrim(int globNum, int state, int x, int y, int z, int shadowImage, int zbufferImage, const Common::Rect *optionalClipRect, int flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable);
	WizPxShrdBuffer drawAWizPrimEx(int globNum, int state, int x, int y, int z, int shadowImage, int zbufferImage, const Common::Rect *optionalClipRect, int flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable, const WizImageCommand *optionalICmdPtr);
	void buildAWiz(const WizPxShrdBuffer &bufPtr, int bufWidth, int bufHeight, const byte *palettePtr, const Common::Rect *rectPtr, int compressionType, int globNum, int transparentColor);

	int	pixelHitTestWiz(int image, int state, int x, int y, int32 flags);
	int pixelHitTestWizPrim(int image, int state, int x, int y, int32 flags);
	int	hitTestWiz(int image, int state, int x, int y, int32 flags);
	int hitTestWizPrim(int globNum, int state, int x, int y, int32 flags);
	void remapImagePrim(int image, int state, int tableCount, const uint8 *remapList, const uint8 *remapTable);
	int createHistogramArrayForImage(int image, int state, const Common::Rect *optionalClipRect);
	void ensureNativeFormatImageForState(int image, int state);

	bool compareDoPixelStreamsOverlap(const WizRawPixel *a, const WizRawPixel *b, int width, WizRawPixel transparentColor);
	bool collisionCompareImageLines(
		const byte *imageAData, int aType, int aw, int ah, int32 wizAFlags, int ax, int ay,
		const byte *imageBData, int bType, int bw, int bh, int32 wizBFlags, int bx, int by,
		int compareWidth, WizRawPixel transparentColor);

	/*
	 * Moonbase Commander custom Wiz operations
	 *
	 * These are defined in moonbase/moonbase_layered_wiz.cpp
	 * Beware: some of these functions are using both the custom WIZ
	 * Rect functions and our own. This is *by design*, and emulates
	 * what the original code does (using both the WIZ functions and
	 * the Windows Rect primitives). Do not attempt to change this!
	 * 
	 */

	bool drawMoonbaseLayeredWiz(
		byte *dstBitmapData, int dstWidth, int dstHeight, int dstPitch,
		int dstFormat, int dstBpp, byte *wizImageData,
		int x, const int y, int state, int clipX1, int clipY1, int clipX2, int clipY2,
		uint32 flags, uint32 conditionBits, byte *ptr8BppToXBppClut, byte *altSourceBuffer);

	void handleCompositeDrawMoonbaseImage(
		WizRawBitmap *bitmapPtr, WizImage *wizPtr, byte *compositeInfoBlockPtr,
		int x, int y, Common::Rect *srcRect, Common::Rect *clipRect,
		int32 flags, uint32 conditionBits, int32 outerSizeX, int32 outerSizeY,
		WizRawPixel16 *ptr8BppToXBppClut, byte *altSourceBuffer);

	void drawMoonbaseImageEx(
		WizRawBitmap *bitmapPtr, WizImage *wizPtr, int x, int y, int state,
		Common::Rect *clipRectPtr, int32 flags, Common::Rect *optionalSrcRect,
		uint32 conditionBits, WizRawPixel16 *ptr8BppToXBppClut, byte *altSourceBuffer);

	bool getRawMoonbaseBitmapInfoForState(WizRawBitmap *bitmapPtr, WizImage *wizPtr, int state);
	void rawMoonbaseBitmapBlit(WizRawBitmap *dstBitmap, Common::Rect *dstRectPtr, WizRawBitmap *srcBitmap, Common::Rect *srcRectPtr);

	void trleFLIPDecompressMoonbaseImage(
		WizRawBitmap *bitmapPtr, WizMoonbaseCompressedImage *imagePtr, int destX, int destY,
		Common::Rect *sourceCoords, Common::Rect *clipRectPtr, int32 flags);
	void trleFLIPDecompMoonbaseImageHull(
		WizRawPixel16 *bufferPtr, int bufferWidth, Common::Rect *destRect, byte *compData, Common::Rect *sourceRect, byte *extraPtr,
		void (*functionPtr)(Wiz *wiz, WizRawPixel *destPtr, byte *dataStream, int skipAmount, int decompAmount, byte *extraPtr));

	bool moonbaseLayeredWizHitTest(int32 *outValue, int32 *optionalOutActualValue, byte *globPtr, int state, int x, int y, int32 flags, uint32 conditionBits);

	void dispatchBlitRGB555(
		byte *bufferData, int bufferWidth, int bufferHeight, int bufferPitch,
		Common::Rect *optionalClippingRect, byte *compressedDataStream,
		int x, int y, int nROP, int nROPParam, byte *altSourceBuffer);

	void blitT14CodecImage(
		byte *dst, int dstw, int dsth, int dstPitch, const Common::Rect *clipBox,
		byte *wizd, int srcx, int srcy, int rawROP, int paramROP);

	void blitDistortion(
		byte *bufferData, int bufferWidth, int bufferHeight, int bufferPitch,
		Common::Rect *optionalClippingRect, byte *compressedDataStream,
		int x, int y, byte *altSourceBuffer);

	void blitUncompressedDistortionBitmap(
		byte *dstBitmapData, int dstWidth, int dstHeight, int dstPitch, int dstFormat, int dstBpp,
		byte *srcBitmapData, int srcWidth, int srcHeight, int srcPitch, int srcFormat, int srcBpp,
		byte *distortionBitmapData, int distortionWidth, int distortionHeight, int distortionPitch, int distortionFormat, int distortionBpp,
		int dstX, int dstY, int srcX, int srcY, int lReach, int rReach, int tReach, int bReach,
		int srcClipX1, int srcClipY1, int srcClipX2, int srcClipY2,
		int dstClipX1, int dstClipY1, int dstClipX2, int dstClipY2);

	void distortionBlitCore(
		Graphics::Surface *dstBitmap, int x, int y, Graphics::Surface *srcBitmap,
		Common::Rect *optionalSrcRectPtr, Common::Rect *optionalclipRectPtr, int transferOp, MoonbaseDistortionInfo *mdi);


private:
	ScummEngine_v71he *_vm;


public:
	/* Drawing Primitives
	 *
	 * These primitives are slightly different and less precise
	 * than the ones available in our Graphics subsystem.
	 * But they are more accurate in the context of SCUMM HE graphics.
	 * So leave them be and resist the urge to replace them with our own
	 * primitives, please :-P
	 */

	// Primitives
	int  pgReadPixel(const WizSimpleBitmap *srcBM, int x, int y, int defaultValue);
	void pgWritePixel(WizSimpleBitmap *srcBM, int x, int y, WizRawPixel value);
	void pgClippedWritePixel(WizSimpleBitmap *srcBM, int x, int y, const Common::Rect *clipRectPtr, WizRawPixel value);
	void pgClippedLineDraw(WizSimpleBitmap *destBM, int asx, int asy, int aex, int aey, const Common::Rect *clipRectPtr, WizRawPixel value);
	void pgClippedThickLineDraw(WizSimpleBitmap *destBM, int asx, int asy, int aex, int aey, const Common::Rect *clipRectPtr, int iLineThickness, WizRawPixel value);
	void pgDrawClippedEllipse(WizSimpleBitmap *pDestBitmap, int iPX, int iPY, int iQX, int iQY, int iKX, int iKY, int iLOD, const Common::Rect *pClipRectPtr, int iThickness, WizRawPixel aColor);
	void pgDrawSolidRect(WizSimpleBitmap *destBM, const Common::Rect *rectPtr, WizRawPixel color);
	void pgFloodFillCmd(int x, int y, int color, const Common::Rect *optionalClipRect);

	void pgHistogramBitmapSubRect(int *tablePtr, const WizSimpleBitmap *bitmapPtr, const Common::Rect *sourceRect);
	void pgSimpleBitmapFromDrawBuffer(WizSimpleBitmap *bitmapPtr, bool background);
	bool pgGetMultiTypeBitmapFromDrawBuffer(WizMultiTypeBitmap *multiBM, bool background);
	void pgDrawRawDataFormatImage(WizRawPixel *bufferPtr, const WizRawPixel *rawData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, int32 wizFlags, const byte *extraTable, int transparentColor);
	void pgSimpleBlit(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect);
	void pgSimpleBlitRemapColors(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, const byte *remapColorTable);
	void pgSimpleBlitTransparentRemapColors(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, WizRawPixel transparentColor, const byte *remapColorTable);
	void pgSimpleBlitMixColors(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, const byte *mixColorTable);
	void pgSimpleBlitTransparentMixColors(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, WizRawPixel transparentColor, const byte *mixColorTable);
	void pgTransparentSimpleBlit(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, WizRawPixel transparentColor);

	void pgDrawWarpDrawLetter(WizRawPixel *bitmapBuffer, int bitmapWidth, int bitmapHeight, const byte *charData, int x1, int y1, int width, int height, byte *colorLookupTable);
	void pgDraw8BppFormatImage(WizRawPixel *bufferPtr, const byte *rawData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, int32 wizFlags, const byte *extraTable, int transparentColor, const WizRawPixel *conversionTable);
	void pgDraw8BppSimpleBlit(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, const WizRawPixel *conversionTable);
	void pgDraw8BppTransparentSimpleBlit(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, int transparentColor, const WizRawPixel *conversionTable);
	void pgDrawImageWith16BitZBuffer(WizSimpleBitmap *psbDst, const WizSimpleBitmap *psbZBuffer, const byte *imgData, int x, int y, int z, int width, int height, Common::Rect *prcClip);
	void pgForwardRemapPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, const byte *lookupTable);
	void pgBackwardsRemapPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, const byte *lookupTable);
	void pgTransparentForwardRemapPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, WizRawPixel transparentColor, const byte *lookupTable);
	void pgTransparentBackwardsRemapPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, WizRawPixel transparentColor, const byte *lookupTable);
	void pgForwardMixColorsPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, const byte *lookupTable);
	void pgBackwardsMixColorsPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, const byte *lookupTable);
	void pgTransparentForwardMixColorsPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, WizRawPixel transparentColor, const byte *lookupTable);
	void pgTransparentBackwardsMixColorsPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, WizRawPixel transparentColor, const byte *lookupTable);
	void pgBlit90DegreeRotate(WizSimpleBitmap *dstBitmap, int x, int y, const WizSimpleBitmap *srcBitmap, const Common::Rect *optionalSrcRect, const Common::Rect *optionalClipRect, bool hFlip, bool vFlip);
	void pgBlit90DegreeRotateTransparent(WizSimpleBitmap *dstBitmap, int x, int y, const WizSimpleBitmap *srcBitmap, const Common::Rect *optionalSrcRect, const Common::Rect *optionalClipRect, bool hFlip, bool vFlip, WizRawPixel transparentColor);
	void pgBlit90DegreeRotateCore(WizSimpleBitmap *dstBitmap, int x, int y, const WizSimpleBitmap *srcBitmap, const Common::Rect *optionalSrcRect, const Common::Rect *optionalClipRect, bool hFlip, bool vFlip, const void *userParam, const void *userParam2,
								  void (*srcTransferFP)(Wiz *wiz, WizRawPixel *dstPtr, int dstStep, const WizRawPixel *srcPtr, int count, const void *userParam, const void *userParam2));

	// Rectangles
	bool findRectOverlap(Common::Rect *destRectPtr, const Common::Rect *sourceRectPtr);
	bool isPointInRect(Common::Rect *r, Common::Point *pt);
	bool isRectValid(Common::Rect r);
	void makeSizedRectAt(Common::Rect *rectPtr, int x, int y, int width, int height);
	void makeSizedRect(Common::Rect *rectPtr, int width, int height);
	void combineRects(Common::Rect *destRect, Common::Rect *ra, Common::Rect *rb);
	void clipRectCoords(Common::Rect *sourceRectPtr, Common::Rect *destRectPtr, Common::Rect *clipRectPtr);
	int getRectWidth(Common::Rect *rectPtr);
	int getRectHeight(Common::Rect *rectPtr);
	void moveRect(Common::Rect *rectPtr, int dx, int dy);
	void horzFlipAlignWithRect(Common::Rect *rectToAlign, const Common::Rect *baseRect);
	void vertFlipAlignWithRect(Common::Rect *rectToAlign, const Common::Rect *baseRect);
	void swapRectX(Common::Rect *rectPtr);
	void swapRectY(Common::Rect *rectPtr);

	// Flood fill
	void floodInitFloodState(WizFloodState *statePtr, WizSimpleBitmap *bitmapPtr, int colorToWrite, const Common::Rect *optionalClippingRect);
	WizFloodState *floodCreateFloodState(int numStackElements);
	void floodDestroyFloodState(WizFloodState *state);
	bool floodBoundryColorFloodCheckPixel(int x, int y, WizFloodState *state);
	void floodFloodFillPrim(int x, int y, WizFloodState *statePtr, bool (*checkPixelFnPtr)(Wiz *w, int x, int y, WizFloodState *state));
	void floodPerformOpOnRect(WizFloodState *statePtr, Common::Rect *rectPtr);
	bool floodSimpleFill(WizSimpleBitmap *bitmapPtr, int x, int y, int colorToWrite, const Common::Rect *optionalClipRect, Common::Rect *updateRectPtr);

	// Utils
	int getRawPixel(int color);
	void memset8BppConversion(void *dstPtr, int value, size_t count, const WizRawPixel *conversionTable);
	void memcpy8BppConversion(void *dstPtr, const void *srcPtr, size_t count, const WizRawPixel *conversionTable);
	void rawPixelMemset(void *dstPtr, int value, size_t count);
	WizRawPixel convert8BppToRawPixel(WizRawPixel value, const WizRawPixel *conversionTable);
	void rawPixelExtractComponents(WizRawPixel aPixel, int &r, int &g, int &b);
	void rawPixelPackComponents(WizRawPixel &aPixel, int r, int g, int b);

	/*
	 * Compression Primitives
	 */

	// MRLE
	void mrleFLIPAltSourceDecompressImage(
		WizRawPixel *destBufferPtr, const byte *compData, int destBufferWidth, int destBufferHeight,
		const void *altBufferPtr, int altWidth, int altHeight, int altBitsPerPixel,
		int x, int y, int width, int height, Common::Rect *clipRectPtr,
		int32 wizFlags, const WizRawPixel *conversionTable);
	void mrleFLIPAltSourceDecompressPrim(
		WizRawPixel *destBufferPtr, int destBufferWidth, int destBufferHeight,
		const void *altBufferPtr, int altBitsPerPixel,
		const WizCompressedImage *imagePtr, int destX, int destY,
		const Common::Rect *sourceCoords, const Common::Rect *clipRectPtr,
		int32 flags, const WizRawPixel *conversionTable,
		void (*forewordFunctionPtr)(Wiz *wiz,
			WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
			int skipAmount, int decompAmount, const WizRawPixel *conversionTable),
		void (*backwardFunctionPtr)(Wiz *wiz,
			WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
			int skipAmount, int decompAmount, const WizRawPixel *conversionTable));

	// Auxiliary compression routines
	void auxWRLEUncompressPixelStream(WizRawPixel *destStream, const byte *singleColorTable, const byte *streamData, int streamSize, const WizRawPixel *conversionTable);
	void auxWRLEUncompressAndCopyFromStreamOffset(WizRawPixel *destStream, const byte *singleColorTable, const byte *streamData, int streamSize, byte copyFromColor, int streamOffset, const WizRawPixel *conversionTable);

	void auxDecompSRLEStream(WizRawPixel *destStream, const WizRawPixel *backgroundStream, const byte *singleColorTable, const byte *streamData, int streamSize, const WizRawPixel *conversionTable);

	void auxDecompDRLEStream(WizRawPixel *destPtr, const byte *dataStream, WizRawPixel *backgroundPtr, int skipAmount, int decompAmount, const WizRawPixel *conversionTable);
	void auxDecompDRLEImage(WizRawPixel *foregroundBufferPtr, WizRawPixel *backgroundBufferPtr, const byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, const WizRawPixel *conversionTable);
	void auxDecompDRLEPrim(WizRawPixel *foregroundBufferPtr, WizRawPixel *backgroundBufferPtr, int bufferWidth, Common::Rect *destRect, const byte *compData, Common::Rect *sourceRect, const WizRawPixel *conversionTable);

	void auxDecompTRLEStream(WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable);
	void auxDecompTRLEImage(WizRawPixel *bufferPtr, const byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, const WizRawPixel *conversionTable);
	void auxDecompTRLEPrim(WizRawPixel *bufferPtr, int bufferWidth, Common::Rect *destRect, const byte *compData, Common::Rect *sourceRect, const WizRawPixel *conversionTable);
	void auxDrawZplaneFromTRLEImage(byte *zplanePtr, const byte *compData, int zplanePixelWidth, int zplanePixelHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, int transOp, int solidOp);
	void auxDrawZplaneFromTRLEPrim(byte *zplanePtr, int zplanePixelWidth, Common::Rect *destRect, const byte *compData, Common::Rect *sourceRect, int transOp, int solidOp);

	void auxDecompRemappedTRLEStream(WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, byte *remapTable, const WizRawPixel *conversionTable);
	void auxDecompRemappedTRLEImage(WizRawPixel *bufferPtr, const byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, byte *remapTable, const WizRawPixel *conversionTable);
	void auxDecompRemappedTRLEPrim(WizRawPixel *bufferPtr, int bufferWidth, Common::Rect *destRect, const byte *compData, Common::Rect *sourceRect, byte *remapTable, const WizRawPixel *conversionTable);

	int auxHitTestTRLEXPos(const byte *dataStream, int skipAmount);
	int auxHitTestTRLEImageRelPos(const byte *compData, int x, int y, int width, int height);
	int auxPixelHitTestTRLEXPos(byte *dataStream, int skipAmount, int transparentValue);
	int auxPixelHitTestTRLEImageRelPos(byte *compData, int x, int y, int width, int height, int transparentValue);

	void auxDecompMixColorsTRLEImage(WizRawPixel *bufferPtr, byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, const byte *colorMixTable, const WizRawPixel *conversionTable);
	void auxDecompMixColorsTRLEPrim(WizRawPixel *bufferPtr, int bufferWidth, Common::Rect *destRect, const byte *compData, Common::Rect *sourceRect, const byte *coloMixTable, const WizRawPixel *conversionTable);
	void auxColorMixDecompressLine(
		WizRawPixel *destPtr, const byte *dataStream, int skipAmount,
		int decompAmount, const byte *colorMixTable,
		const WizRawPixel *conversionTable);

	void auxRemappedMemcpy(
		WizRawPixel *dstPtr, const byte *srcPtr, int count, byte *remapTable,
		const WizRawPixel *conversionTable);

	void auxZplaneFromTRLEStream(
		byte *destPtr, const byte *dataStream, int skipAmount, int decompAmount,
		int mask, int transOp, int solidOp);

	void auxHistogramTRLELine(int *tablePtr, const byte *dataStream, int skipAmount, int decompAmount);
	void auxHistogramTRLEPrim(int *histogramTablePtr, byte *compData, Common::Rect *sourceRect);

	// TRLE
	int _trlePutSize = 0;
	byte _trleBuf[(128 * 2) * sizeof(WizRawPixel)] = {};

	byte *trlePutDump(byte *dest, int nn);
	byte *trlePutRun(byte *dest, int nn, int cc, int tcolor);
	int trleRLECompression(byte *pdest, const WizRawPixel *psource, int rowsize, WizRawPixel tcolor);
	int trleCompressImageArea(byte *destBuffer, const WizRawPixel *sourceBuffer, int sourceBufferWidth, int x1, int y1, int x2, int y2, WizRawPixel transparentColor);

	// TRLE FLIP
	bool _initializeAlphaTable = true;
	float _alphaTable[256] = {};
	int _precomputed16bppTable[WIZ_QUANTIZED_ALPHA_COUNT][WIZ_COLOR16_COMPONENT_COUNT][WIZ_COLOR16_COMPONENT_COUNT] = { {}, {}, {} };

	void trleFLIPDecompressImage(
		WizRawPixel *bufferPtr, const byte *compData, int bufferWidth, int bufferHeight,
		int x, int y, int width, int height, Common::Rect *clipRectPtr,
		int32 wizFlags, const byte *extraTable, const WizRawPixel *conversionTable,
		const WizImageCommand *optionalICmdPtr);

	void trleFLIPDecompressPrim(
		WizSimpleBitmap *bitmapPtr, const WizCompressedImage *imagePtr, int destX, int destY,
		const Common::Rect *sourceCoords, const Common::Rect *clipRectPtr, const byte *extraPtr,
		int32 flags, const WizRawPixel *conversionTable,
		void (*forewordFunctionPtr)(Wiz *wiz,
			WizRawPixel *destPtr, const byte *dataStream, int skipAmount,
			int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable),
		void (*backwardFunctionPtr)(Wiz *wiz,
			WizRawPixel *destPtr, const byte *dataStream, int skipAmount,
			int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable));

	void trleFLIPDecompImageHull(
		WizRawPixel *bufferPtr, int bufferWidth, const Common::Rect *destRect,
		const byte *compData, const Common::Rect *sourceRect, const byte *extraPtr,
		const WizRawPixel *conversionTable,
		void (*functionPtr)(Wiz *wiz,
			WizRawPixel *destPtr, const byte *dataStream, int skipAmount,
			int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable));

	void trleFLIPAltSourceDecompressImage(
		WizRawPixel *destBufferPtr, const byte *compData, int destBufferWidth, int destBufferHeight,
		const void *altBufferPtr, int altWidth, int altHeight, int altBitsPerPixel,
		int x, int y, int width, int height, Common::Rect *clipRectPtr,
		int32 wizFlags, const WizRawPixel *conversionTable,
		WizImageCommand *optionalICmdPtr);

	void trleFLIPAltSourceDecompressPrim(
		WizRawPixel *destBufferPtr, int destBufferWidth, int destBufferHeight,
		const void *altBufferPtr, int altBitsPerPixel,
		const WizCompressedImage *imagePtr, int destX, int destY,
		const Common::Rect *sourceCoords, const Common::Rect *clipRectPtr,
		int32 flags, const WizRawPixel *conversionTable,
		void (*forewordFunctionPtr)(Wiz *wiz,
									WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
									int skipAmount, int decompAmount, const WizRawPixel *conversionTable),
		void (*backwardFunctionPtr)(Wiz *wiz,
									WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
									int skipAmount, int decompAmount, const WizRawPixel *conversionTable));

	void trleFLIPAltSourceDecompImageHull(
		WizRawPixel *bufferPtr, int bufferWidth, const Common::Rect *destRect,
		const byte *altSourceBuffer, int altBytesPerLine,
		int altBytesPerPixel, const Common::Rect *altRect,
		const byte *compData, const Common::Rect *sourceRect,
		const WizRawPixel *conversionTable,
		void (*functionPtr)(Wiz *wiz,
							WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
							int skipAmount, int decompAmount, const WizRawPixel *conversionTable));

	bool trleFLIPAltSourceSpecialCaseDispatch(
		WizRawPixel *destBufferPtr, const byte *compData, int destBufferWidth, int destBufferHeight,
		const void *altBufferPtr, int altWidth, int altHeight, int altBitsPerPixel,
		int x, int y, int width, int height, Common::Rect *clipRectPtr,
		int32 wizFlags, const WizRawPixel *conversionTable,
		WizImageCommand *optionalICmdPtr);

	void trleFLIPRotate90DecompressImage(
		WizRawPixel *bufferPtr, const byte *compData, int bufferWidth, int bufferHeight,
		int x, int y, int width, int height, const Common::Rect *clipRectPtr,
		int32 wizFlags, const void *extraTable, const WizRawPixel *conversionTable,
		WizImageCommand *optionalICmdPtr);

	void trleFLIP90DegreeRotateCore(
		WizSimpleBitmap *dstBitmap, int x, int y, const WizCompressedImage *imagePtr, const Common::Rect *optionalSrcRect,
		const Common::Rect *optionalClipRect, bool hFlip, bool vFlip, const void *userParam, const WizRawPixel *conversionTable,
		void (*functionPtr)(Wiz *wiz,
			WizRawPixel *destPtr, const byte *dataStream, int skipAmount,
			int decompAmount, const void *userParam, int destStepValue,
			const WizRawPixel *conversionTable));

	void trleFLIPCheckAlphaSetup();
	WizRawPixel trleFLIPAlphaMixPrim(WizRawPixel b, WizRawPixel a, int alpha);
	void trleFLIPFiftyFiftyMixPixelMemset(WizRawPixel *dstPtr, WizRawPixel mixColor, int size);
	void trleFLIPFiftyFiftyMixForwardPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable);
	void trleFLIPFiftyFiftyMixBackwardsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable);
	void trleFLIPAdditivePixelMemset(WizRawPixel *dstPtr, WizRawPixel mixColor, int size);
	void trleFLIPAdditiveForwardPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable);
	void trleFLIPAdditiveBackwardsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable);
	void trleFLIPSubtractivePixelMemset(WizRawPixel *dstPtr, WizRawPixel mixColor, int size);
	void trleFLIPSubtractiveForwardPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable);
	void trleFLIPSubtractiveBackwardsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable);
	void trleFLIPRemapDestPixels(WizRawPixel *dstPtr, int size, const byte *lookupTable);
	void trleFLIPForwardPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable);
	void trleFLIPBackwardsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable);
	void trleFLIPForwardLookupPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const byte *lookupTable, const WizRawPixel *conversionTable);
	void trleFLIPBackwardsLookupPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const byte *lookupTable, const WizRawPixel *conversionTable);
	void trleFLIPForwardMixColorsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const byte *lookupTable);
	void trleFLIPBackwardsMixColorsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const byte *lookupTable);


	/*
	 * Image Warping on Polygons Primitives
	 */

	bool warpDrawWiz(int image, int state, int polygon, int32 flags, int transparentColor, WizSimpleBitmap *optionalDestBitmap, const WizRawPixel *optionalColorConversionTable, int shadowImage);
	bool warpDrawWizTo4Points(int image, int state, const WarpWizPoint *dstPoints, int32 flags, int transparentColor, const Common::Rect *optionalClipRect, WizSimpleBitmap *optionalDestBitmap, const WizRawPixel *optionalColorConversionTable, const byte *colorMixTable);
	WarpWizOneSpanTable *warpCreateSpanTable(int spanCount);
	void warpDestroySpanTable(WarpWizOneSpanTable *spanTable);
	WarpWizOneSpanTable *warpBuildSpanTable(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *dstPts, const WarpWizPoint *srcPts, int npoints, const Common::Rect *clipRectPtr);
	void warpProcessDrawSpansA(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count);
	void warpProcessDrawSpansTransparent(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor);
	void warpProcessDrawSpansTransparentFiltered(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor, const byte *pXmapColorTable, bool bIsHintColor, WizRawPixel hintColor);
	void warpProcessDrawSpansMixColors(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor, const byte *tablePtr);
	void warpFillSpanWithLine(WarpWizOneSpanTable *st, const WarpWizPoint *dstA, const WarpWizPoint *dstB, const WarpWizPoint *srcA, const WarpWizPoint *srcB);
	void warpProcessDrawSpansSampled(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count);
	void warpProcessDrawSpansTransparentSampled(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap,const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor);
	bool warpNPt2NPtWarpCORE(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const Common::Rect *optionalClipRect, int32 wizFlags);
	bool warpNPt2NPtNonClippedWarp(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor);
	bool warpNPt2NPtClippedWarp(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const Common::Rect *optionalClipRect);
	bool warpNPt2NPtClippedWarpMixColors(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const Common::Rect *optionalClipRect, const byte *colorMixTable);
	bool warpNPt2NPtNonClippedWarpFiltered(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const byte *pXmapColorTable, bool bIsHintColor, WizRawPixel hintColor);
	void warpFindMinMaxpoints(WarpWizPoint *minPtr, WarpWizPoint *maxPtr, const WarpWizPoint *points, int npoints);
};

} // End of namespace Scumm

#endif
