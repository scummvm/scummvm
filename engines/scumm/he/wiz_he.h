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

struct WizImage {
	int resNum;
	int x1;
	int y1;
	int zorder;
	int state;
	int flags;
	int shadow;
	int zbuffer;
	int palette;
};

struct FontProperties {
	byte string[4096];
	byte fontName[4096];
	int fgColor;
	int bgColor;
	int style;
	int size;
	int xPos;
	int yPos;
};

struct EllipseProperties {
	int px;
	int py;
	int qx;
	int qy;
	int kx;
	int ky;
	int lod;
	int color;
};

struct WizParameters {
	int dwSize;
	byte filename[260];
	Common::Rect box;
	int actionFlags;
	int actionMode;
	int params[8];
	int compressionType;
	int fileType;
	int angle;
	int scale;
	int polygon;
	int polygon2;
	int resDefImgW;
	int resDefImgH;
	int sourceImage;
	int propertyValue;
	int propertyNumber;
	uint8 remapTable[256];
	uint8 remapList[256];
	int remapNum;
	int dstResNum;
	uint16 fillColor;
	FontProperties fontProperties;
	EllipseProperties ellipseProperties;
	Common::Rect renderCoords;
	int blendFlags;
	int spriteId;
	int spriteGroup;
	int conditionBits;
	WizImage img;

	void reset() {
		dwSize = 0;
		memset(filename, 0, sizeof(filename));
		box.top = box.left = box.bottom = box.right = 0;
		actionFlags = 0;
		actionMode = 0;
		memset(params, 0, sizeof(params));
		compressionType = 0;
		fileType = 0;
		angle = 0;
		scale = 0;
		polygon = 0;
		polygon2 = 0;
		resDefImgW = 0;
		resDefImgH = 0;
		sourceImage = 0;
		propertyValue = 0;
		propertyNumber = 0;
		memset(remapTable, 0, sizeof(remapTable));
		memset(remapList, 0, sizeof(remapList));
		remapNum = 0;
		dstResNum = 0;
		fillColor = 0;
		memset(&fontProperties, 0, sizeof(FontProperties));
		memset(&ellipseProperties, 0, sizeof(EllipseProperties));
		renderCoords.left = renderCoords.top = renderCoords.bottom = renderCoords.right = 0;
		blendFlags = 0;
		spriteId = 0;
		spriteGroup = 0;
		conditionBits = 0;
		memset(&img, 0, sizeof(WizImage));
	}
};

enum WizRenderingFlags {
	kWRFUsePalette = 0x00000001,
	kWRFRemap      = 0x00000002,
	kWRFPrint      = 0x00000004,
	kWRFBackground = 0x00000008,
	kWRFForeground = 0x00000010,
	kWRFAlloc      = 0x00000020,
	kWIFIsPolygon  = 0x00000040,
	kWIFZPlaneOn   = 0x00000080,
	kWIFZPlaneOff  = 0x00000100,
	kWIFUseShadow  = 0x00000200,
	kWIFFlipX      = 0x00000400,
	kWIFFlipY      = 0x00000800,
	hWRFRotate90   = 0x00001000
};

enum WizActions {
	kWAUnknown = 0,
	kWADraw = 1,
	kWACapture = 2,
	kWALoad = 3,
	kWASave = 4,
	kWAGlobalState = 5,
	kWAModify = 6,
	kWAPolyCapture = 7,
	kWANew = 8,
	kWARenderRectangle = 9,
	kWARenderLine = 10,
	kWARenderPixel = 11,
	kWARenderFloodFill = 12,
	kWAFontStart = 13,
	kWAFontEnd = 14,
	kWAFontCreate = 15,
	kWAFontRender = 16,
	kWARenderEllipse = 17,
};

enum WizActionFlags {
	kWAFSpot            = 0x00000001,
	kWAFPolygon         = 0x00000002,
	kWAFShadow          = 0x00000004,
	kWAFScaled          = 0x00000008,
	kWAFRotate          = 0x00000010,
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
	kWCFConditionBits = 0x01,
	kWCFSubState = 0x02,
	kWCFXDelta = 0x04,
	kWCFYDelta = 0x08,
	kWCFDrawFlags = 0x10,
	kWCFSubConditionBits = 0x20
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

	WizImage _images[NUM_IMAGES];
	uint16 _imagesNum;
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

	void createWizEmptyImage(int resNum, int x1, int y1, int width, int height);
	void fillWizRect(const WizParameters *params);
	void fillWizLine(const WizParameters *params);
	void fillWizPixel(const WizParameters *params);
	void fillWizFlood(const WizParameters *params);
	void remapWizImagePal(const WizParameters *params);

	void getWizImageDim(int resNum, int state, int32 &w, int32 &h);
	void getWizImageDim(uint8 *dataPtr, int state, int32 &w, int32 &h);
	int getWizImageStates(int resnum);
	int getWizImageStates(const uint8 *ptr);
	int isWizPixelNonTransparent(int resnum, int state, int x, int y, int flags);
	int isWizPixelNonTransparent(uint8 *data, int state, int x, int y, int flags);
	int isPixelNonTransparent(const uint8 *data, int x, int y, int w, int h, uint8 bitdepth);
	uint16 getWizPixelColor(int resnum, int state, int x, int y);
	int getWizImageData(int resNum, int state, int type);

	void flushWizBuffer();

	void getWizImageSpot(int resId, int state, int32 &x, int32 &y);
	void getWizImageSpot(uint8 *data, int state, int32 &x, int32 &y);
	void loadWizCursor(int resId, int palette);

	void captureWizImage(int resNum, const Common::Rect& r, bool frontBuffer, int compType);
	void captureImage(uint8 *src, int srcPitch, int srcw, int srch, int resNum, const Common::Rect& r, int compType);
	void captureWizPolygon(int resNum, int maskNum, int maskState, int id1, int id2, int compType);
	void displayWizComplexImage(const WizParameters *params);
	void displayWizImage(WizImage *pwi);
	void processWizImage(const WizParameters *params);

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
};

} // End of namespace Scumm

#endif
