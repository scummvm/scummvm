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

struct WizPolygon {
	Common::Point vert[5];
	Common::Rect bound;
	int id;
	int numVerts;
	bool flag;

	void reset() {
		for (int i = 0; i < ARRAYSIZE(vert); i++) {
			vert[i].x = vert[i].y = 0;
		}
		bound.top = bound.left = bound.bottom = bound.right = 0;
		id = 0;
		numVerts = 0;
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
	int color;
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
	int actionMode;
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
	uint16 colorValue;
	WizFontProperties fontProperties;
	WizEllipseProperties ellipseProperties;
	Common::Rect renderCoords;
	WizExtendedRenderInfo extendedRenderInfo;

	void reset() {
		dwSize = 0;
		memset(filename, 0, sizeof(filename));
		box.top = box.left = box.bottom = box.right = 0;
		actionFlags = 0;
		actionMode = 0;
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

struct WizSimpleBitmap {
	int32 *bufferPtr;
	int bitmapWidth;
	int bitmapHeight;
};


enum WizRenderingFlags {
	// Standard rendering flags
	kWRFUsePalette = 0x00000001,
	kWRFRemap      = 0x00000002,
	kWRFPrint      = 0x00000004,
	kWRFBackground = 0x00000008,
	kWRFForeground = 0x00000010,
	kWRFAlloc      = 0x00000020,
	kWRFIsPolygon  = 0x00000040,
	kWRFZPlaneOn   = 0x00000080,
	kWRFZPlaneOff  = 0x00000100,
	kWRFUseShadow  = 0x00000200,
	kWRFFlipX      = 0x00000400,
	kWRFFlipY      = 0x00000800,
	kWRFRotate90   = 0x00001000,

	// Special rendering flags
	kWRFSpecialRenderBitMask         = 0xfff00000,
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
	kWAFSourceImg       = 0x00080000,
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
	kWCTNone32Bpp                 = 0x00000003,
	kWCTComposite                 = 0x00000004,
	kWCTTRLE16Bpp                 = 0x00000005,
	kWCTTRLE32Bpp                 = 0x00000006,
	kWCTMRLEWithLineSizePrefix    = 0x00000007,
	kWCTMRLEWithoutLineSizePrefix = 0x00000008,
	kWCTDataBlockDependent        = 0x00000009,
	kWCTNone16BppBigEndian        = 0x0000000A,
	kWCTNone32BppBigEndian        = 0x0000000B,
	kWCTTRLE16BppBigEndian        = 0x0000000C,
	kWCTTRLE32BppBigEndian        = 0x0000000D
};

enum CreateWizFlags {
	kCWFPalette    = 0x00000001,
	kCWFSpot       = 0x00000002,
	kCWFRemapTable = 0x00000008,
	kCWFDefault    = ((kCWFPalette) | (kCWFSpot) | (kCWFRemapTable))
};

enum WizSpcConditionTypes : uint {
	kWSPCCTBits = 0xc0000000,
	kWSPCCTOr   = 0x00000000,
	kWSPCCTAnd  = 0x40000000,
	kWSPCCTNot  = 0x80000000
};

enum WizMoonSystemBits {
	kWMSBRopMask = 0xff,
	kWMSBRopParamMask = 0xff00,
	kWMSBReservedBits = (kWMSBRopMask | kWMSBRopParamMask),
	kWMSBRopParamRShift = 8
};

enum WizEclipseConstants {
	kWECFixedSize = 16,
	kWECPiOver2 = 102944, // Fixed point PI/2
	kWECHalf = 32768      // Fixed point 1/2
};

enum {
	kWizXMap = 0,
	kWizRMap,
	kWizCopy
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

	WizBufferElement _wizBuffer[NUM_IMAGES];
	uint16 _wizBufferIndex;
	WizPolygon _polygons[NUM_POLYGONS];

	Wiz(ScummEngine_v71he *vm);

	void clearWizBuffer();
	Common::Rect _rectOverride;
	bool _cursorImage;
	bool _rectOverrideEnabled;

	void polygonClear();
	void polygonLoad(const uint8 *polData);
	void polygonStore(int id, bool flag, int vert1x, int vert1y, int vert2x, int vert2y, int vert3x, int vert3y, int vert4x, int vert4y);
	void polygonCalcBoundBox(Common::Point *vert, int numVerts, Common::Rect & bound);
	void polygonErase(int fromId, int toId);
	int polygonHit(int id, int x, int y);
	bool polygonDefined(int id);
	bool polygonContains(const WizPolygon &pol, int x, int y);
	void polygonRotatePoints(Common::Point *pts, int num, int alpha);
	void polygonTransform(int resNum, int state, int po_x, int po_y, int angle, int zoom, Common::Point *vert);

	void dwCreateRawWiz(int imageNum, int w, int h, int flags, int bitsPerPixel, int optionalSpotX, int optionalSpotY);
	bool dwSetSimpleBitmapStructFromImage(int imageNum, int imageState, WizSimpleBitmap *destBM);

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
	int getWizImageStates(int resnum);
	int getWizImageStates(const uint8 *ptr);
	void *getWizStateHeaderPrim(int resNum, int state);
	void *getWizStateDataPrim(int resNum, int state);

	int isWizPixelNonTransparent(int resnum, int state, int x, int y, int flags);
	int isWizPixelNonTransparent(uint8 *data, int state, int x, int y, int flags);
	int isPixelNonTransparent(const uint8 *data, int x, int y, int w, int h, uint8 bitdepth);
	uint16 getWizPixelColor(int resnum, int state, int x, int y);
	int getWizImageData(int resNum, int state, int type);
	bool isUncompressedFormatTypeID(int id);

	void flushWizBuffer();

	void getWizImageSpot(int resId, int state, int32 &x, int32 &y);
	void getWizImageSpot(uint8 *data, int state, int32 &x, int32 &y);
	void loadWizCursor(int resId, int palette);

	void captureImage(uint8 *src, int srcPitch, int srcw, int srch, int resNum, const Common::Rect& r, int compType);
	void takeAWiz(int resNum, const Common::Rect &r, bool backBuffer, int compType);

	void simpleDrawAWiz(int image, int state, int x, int y, int flags);
	void bufferAWiz(int image, int state, int x, int y, int z, int flags, int optionalShadowImage, int optionalZBufferImage, int whichPalette);

	uint8 *drawWizImage(int resNum, int state, int maskNum, int maskState, int x1, int y1, int zorder, int shadow, int zbuffer, const Common::Rect *clipBox, int flags, int dstResNum, const uint8 *palPtr, uint32 conditionBits);
	void drawWizImageEx(uint8 *dst, uint8 *src, uint8 *mask, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, int state, const Common::Rect *rect, int flags, const uint8 *palPtr, int transColor, uint8 bitDepth, const uint8 *xmapPtr, uint32 conditionBits);
	void drawWizPolygon(int resNum, int state, int id, int flags, int shadow, int dstResNum, int palette);
	void drawWizComplexPolygon(int resNum, int state, int po_x, int po_y, int shadow, int angle, int zoom, const Common::Rect *r, int flags, int dstResNum, int palette);
	void drawWizPolygonTransform(int resNum, int state, Common::Point *wp, int flags, int shadow, int dstResNum, int palette);
	void drawWizPolygonImage(uint8 *dst, const uint8 *src, const uint8 *mask, int dstpitch, int dstType, int dstw, int dsth, int wizW, int wizH, Common::Rect &bound, Common::Point *wp, uint8 bitDepth);

#ifdef USE_RGB_COLOR
	static void copyMaskWizImage(uint8 *dst, const uint8 *src, const uint8 *mask, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr);

	void copyCompositeWizImage(uint8 *dst, uint8 *wizPtr, uint8 *wizd, uint8 *maskPtr, int dstPitch, int dstType,
		int dstw, int dsth, int srcx, int srcy, int srcw, int srch, int state, const Common::Rect *clipBox,
		int flags, const uint8 *palPtr, int transColor, uint8 bitDepth, const uint8 *xmapPtr, uint32 conditionBits);
	void copy555WizImage(uint8 *dst, uint8 *wizd, int dstPitch, int dstType,
			int dstw, int dsth, int srcx, int srcy, const Common::Rect *clipBox, uint32 conditionBits);
#endif

	static void copyAuxImage(uint8 *dst1, uint8 *dst2, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, uint8 bitdepth);
	static void copyWizImageWithMask(uint8 *dst, const uint8 *src, int dstPitch, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int maskT, int maskP);
	static void copyWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitdepth);
	static void copyRawWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr, int transColor, uint8 bitdepth);
#ifdef USE_RGB_COLOR
	static void copy16BitWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *xmapPtr);
	static void copyRaw16BitWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, int transColor);
	template<int type> static void decompress16BitWizImage(uint8 *dst, int dstPitch, int dstType, const uint8 *src, const Common::Rect &srcRect, int flags, const uint8 *xmapPtr = NULL);
#endif
	template<int type> static void decompressWizImage(uint8 *dst, int dstPitch, int dstType, const uint8 *src, const Common::Rect &srcRect, int flags, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitdepth);
	template<int type> static void decompressRawWizImage(uint8 *dst, int dstPitch, int dstType, const uint8 *src, int srcPitch, int w, int h, int transColor, const uint8 *palPtr, uint8 bitdepth);

#ifdef USE_RGB_COLOR
	template<int type> static void write16BitColor(uint8 *dst, const uint8 *src, int dstType, const uint8 *xmapPtr);
#endif
	template<int type> static void write8BitColor(uint8 *dst, const uint8 *src, int dstType, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitDepth);
	static void writeColor(uint8 *dstPtr, int dstType, uint16 color);

	uint16 getWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 bitDepth, uint16 color);
	uint16 getRawWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 bitDepth, uint16 color);
	void computeWizHistogram(uint32 *histogram, const uint8 *data, const Common::Rect& rCapt);
	void computeRawWizHistogram(uint32 *histogram, const uint8 *data, int srcPitch, const Common::Rect& rCapt);

private:
	ScummEngine_v71he *_vm;

	/* Wiz Drawing Primitives
	 *
	 * These primitives are slightly different and less precise
	 * than the ones available in our Graphics subsystem.
	 * But they are more accurate in the context of SCUMM HE graphics.
	 * So leave them be and resist the urge to replace them with our own
	 * primitives, please :-P
	 */

	int  pgReadPixel(const WizSimpleBitmap *srcBM, int x, int y, int defaultValue);
	void pgWritePixel(WizSimpleBitmap *srcBM, int x, int y, int32 value);
	void pgClippedWritePixel(WizSimpleBitmap *srcBM, int x, int y, const Common::Rect *clipRectPtr, int32 value);
	void pgClippedLineDraw(WizSimpleBitmap *destBM, int asx, int asy, int aex, int aey, const Common::Rect *clipRectPtr, int32 value);
	void pgClippedThickLineDraw(WizSimpleBitmap *destBM, int asx, int asy, int aex, int aey, const Common::Rect *clipRectPtr, int iLineThickness, int32 value);
	void pgDrawClippedEllipse(WizSimpleBitmap *pDestBitmap, int iPX, int iPY, int iQX, int iQY, int iKX, int iKY, int iLOD, const Common::Rect *pClipRectPtr, int iThickness, int32 aColor);
	void pgDrawSolidRect(WizSimpleBitmap *destBM, const Common::Rect *rectPtr, int32 color);

	bool findRectOverlap(Common::Rect *destRectPtr, const Common::Rect *sourceRectPtr);
	bool isPointInRect(Common::Rect *r, Common::Point *pt);
	void makeSizedRectAt(Common::Rect *rectPtr, int x, int y, int width, int height);
	void makeSizedRect(Common::Rect *rectPtr, int width, int height);
};

} // End of namespace Scumm

#endif
