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

#ifndef SCI_GRAPHICS_CELOBJ32_H
#define SCI_GRAPHICS_CELOBJ32_H

#include "common/rational.h"
#include "common/rect.h"
#include "sci/resource/resource.h"
#include "sci/engine/vm_types.h"
#include "sci/util.h"

namespace Sci {
typedef Common::Rational Ratio;

// SCI32 has four different coordinate systems:
// 1. low resolution, 2. game/script resolution, 3. text/bitmap resolution,
// 4. screen resolution
//
// In CelObj, these values are used when there is no baked in resolution of
// cels.
//
// In ScreenItem, it is used when deciding which path to take to calculate
// dimensions.
enum {
	kLowResX = 320,
	kLowResY = 200
};

enum CelType {
	kCelTypeView  = 0,
	kCelTypePic   = 1,
	kCelTypeMem   = 2,
	kCelTypeColor = 3
};

enum CelCompressionType {
	kCelCompressionNone    = 0,
	kCelCompressionRLE     = 138,
	kCelCompressionInvalid = 1000
};

/**
 * A CelInfo32 object describes the basic properties of a cel object.
 */
struct CelInfo32 {
	/**
	 * The type of the cel object.
	 */
	CelType type;

	/**
	 * For cel objects that draw from resources, the ID of the resource to load.
	 */
	GuiResourceId resourceId;

	/**
	 * For CelObjView, the loop number to draw from the view resource.
	 */
	int16 loopNo;

	/**
	 * For CelObjView and CelObjPic, the cel number to draw from the view or pic
	 * resource.
	 */
	int16 celNo;

	/**
	 * For CelObjMem, a segment register pointing to a heap resource containing
	 * headered bitmap data.
	 */
	reg_t bitmap;

	/**
	 * For CelObjColor, the fill color.
	 */
	uint8 color;

	CelInfo32() :
		// In SSCI, color is left uninitialised
		type(kCelTypeMem),
		resourceId(0),
		loopNo(0),
		celNo(0),
		bitmap(NULL_REG) {}

	// This is the equivalence criteria used by CelObj::searchCache in at least
	// SSCI SQ6. Notably, it does not check the color field.
	inline bool operator==(const CelInfo32 &other) {
		return (
			type == other.type &&
			resourceId == other.resourceId &&
			loopNo == other.loopNo &&
			celNo == other.celNo &&
			bitmap == other.bitmap
		);
	}

	inline bool operator!=(const CelInfo32 &other) {
		return !(*this == other);
	}

	inline Common::String toString() const {
		switch (type) {
		case kCelTypeView:
			return Common::String::format("view %u, loop %d, cel %d", resourceId, loopNo, celNo);
		case kCelTypePic:
			return Common::String::format("pic %u, cel %d", resourceId, celNo);
		case kCelTypeColor:
			return Common::String::format("color %d", color);
		case kCelTypeMem:
			return Common::String::format("mem %04x:%04x", PRINT_REG(bitmap));
		default:
			assert(!"Should never happen");
		}
		// This code should not be reached but the compiler expects to see a legal
		// return from a non-void function.
		return Common::String("here be dragons");
	}
};

class CelObj;
struct CelCacheEntry {
	/**
	 * A monotonically increasing cache ID used to identify the least recently
	 * used item in the cache for replacement.
	 */
	int id;
	Common::ScopedPtr<CelObj> celObj;
	CelCacheEntry() : id(0) {}
};

typedef Common::Array<CelCacheEntry> CelCache;

#pragma mark -
#pragma mark CelScaler

enum {
	/**
	 * The maximum size of a row/column of scaled pixel data.
	 */
	kCelScalerTableSize = 4096
};

struct CelScalerTable {
	/**
	 * A lookup table of indexes that should be used to find the correct column
	 * to read from the source bitmap when drawing a scaled version of the
	 * source bitmap.
	 */
	int valuesX[kCelScalerTableSize];

	/**
	 * The ratio used to generate the x-values.
	 */
	Ratio scaleX;

	/**
	 * A lookup table of indexes that should be used to find the correct row to
	 * read from a source bitmap when drawing a scaled version of the source
	 * bitmap.
	 */
	int valuesY[kCelScalerTableSize];

	/**
	 * The ratio used to generate the y-values.
	 */
	Ratio scaleY;
};

class CelScaler {
	/**
	 * Cached scale tables.
	 */
	CelScalerTable _scaleTables[2];

	/**
	 * The index of the most recently used scale table.
	 */
	int _activeIndex;

	/**
	 * Activates a scale table for the given X and Y ratios. If there is no
	 * table that matches the given ratios, the least most recently used table
	 * will be replaced and activated.
	 */
	void activateScaleTables(const Ratio &scaleX, const Ratio &scaleY);

	/**
	 * Builds a pixel lookup table in `table` for the given ratio. The table
	 * will be filled up to the specified size, which should be large enough to
	 * draw across the entire target buffer.
	 */
	void buildLookupTable(int *table, const Ratio &ratio, const int size);

public:
	CelScaler() :
		_scaleTables(),
		_activeIndex(0) {
		CelScalerTable &table = _scaleTables[0];
		table.scaleX = Ratio();
		table.scaleY = Ratio();
		for (int i = 0; i < ARRAYSIZE(table.valuesX); ++i) {
			table.valuesX[i] = i;
			table.valuesY[i] = i;
		}
		for (int i = 1; i < ARRAYSIZE(_scaleTables); ++i) {
			_scaleTables[i] = _scaleTables[0];
		}
	}

	/**
	 * Retrieves scaler tables for the given X and Y ratios.
	 */
	const CelScalerTable &getScalerTable(const Ratio &scaleX, const Ratio &scaleY);
};

#pragma mark -
#pragma mark CelObj

class ScreenItem;
/**
 * A cel object is the lowest-level rendering primitive in the SCI engine and
 * draws itself directly to a target pixel buffer.
 */
class CelObj {
protected:
	/**
	 * When true, every second line of the cel will be rendered as a black line.
	 *
	 * @see ScreenItem::_drawBlackLines
	 * @note Using a static member because otherwise this would otherwise need
	 * to be copied down through several calls. (SSCI did similar, using a
	 * global variable.)
	 */
	static bool _drawBlackLines;

	/**
	 * When true, this cel will be horizontally mirrored when it is drawn. This
	 * is an internal flag that is set by draw methods based on the combination
	 * of the cel's `_mirrorX` property and the owner screen item's `_mirrorX`
	 * property.
	 */
	bool _drawMirrored;

public:
	static Common::ScopedPtr<CelScaler> _scaler;

	/**
	 * The basic identifying information for this cel. This information
	 * effectively acts as a composite key for a cel object, and any cel object
	 * can be recreated from this data alone.
	 */
	CelInfo32 _info;

	/**
	 * The offset to the cel header for this cel within the raw resource data.
	 */
	uint32 _celHeaderOffset;

	/**
	 * The offset to the embedded palette for this cel within the raw resource
	 * data.
	 */
	uint32 _hunkPaletteOffset;

	/**
	 * The natural dimensions of the cel.
	 */
	uint16 _width, _height;

	/**
	 * The origin of the cel, relative to the top-left corner, in cel
	 * coordinates.
	 */
	Common::Point _origin;

	/**
	 * The dimensions of the original coordinate system for the cel. Used to
	 * scale cels from their native size to the correct size on screen.
	 *
	 * @note This is set to scriptWidth/Height for CelObjColor. For other cel
	 * objects, the value comes from the raw resource data. For text bitmaps,
	 * this is the width/height of the coordinate system used to generate the
	 * text, which also defaults to scriptWidth/Height but seems to typically be
	 * changed to more closely match the native screen resolution.
	 */
	uint16 _xResolution, _yResolution;

	/**
	 * The skip (transparent) color for the cel. When compositing, any pixels
	 * matching this color will not be copied to the buffer.
	 */
	uint8 _skipColor;

	/**
	 * Whether or not this cel has any transparent regions. This is used for
	 * optimised drawing of non-transparent cels.
	 */
	bool _transparent;

	/**
	 * The compression type for the pixel data for this cel.
	 */
	CelCompressionType _compressionType;

	/**
	 * Whether or not this cel contains remap pixels.
	 */
	bool _remap;

	/**
	 * If true, the cel contains pre-mirrored picture data. This value comes
	 * directly from the resource data and is XORed with the `_mirrorX` property
	 * of the owner screen item when rendering.
	 */
	bool _mirrorX;

	/**
	 * If true, the source for this cel is a Mac pic or view whose pixels for
	 * entries 0 and 255 must be swapped when drawing since we use the PC palette.
	 */
	bool _isMacSource;

	/**
	 * Initialises static CelObj members.
	 */
	static void init();

	/**
	 * Frees static CelObj members.
	 */
	static void deinit();

	virtual ~CelObj() {};

	/**
	 * Draws the cel to the target buffer using the priority and positioning
	 * information from the given screen item. The mirroring of the cel will be
	 * unchanged from any previous call to draw.
	 */
	void draw(Buffer &target, const ScreenItem &screenItem, const Common::Rect &targetRect) const;

	/**
	 * Draws the cel to the target buffer using the priority and positioning
	 * information from the given screen item and the given mirror flag.
	 *
	 * @note In SSCI, this function was a virtual function, but CelObjView,
	 * CelObjPic, and CelObjMem all used the same function and the compiler
	 * deduplicated the copies; we deduplicate the source by putting the
	 * implementation on CelObj instead of copying it to 3/4 of the subclasses.
	 */
	virtual void draw(Buffer &target, const ScreenItem &screenItem, const Common::Rect &targetRect, const bool mirrorX);

	/**
	 * Draws the cel to the target buffer using the positioning and mirroring
	 * information from the provided arguments.
	 *
	 * @note In SSCI, this function was a virtual function, but CelObjView,
	 * CelObjPic, and CelObjMem all used the same function and the compiler
	 * deduplicated the copies; we deduplicate the source by putting the
	 * implementation on CelObj instead of copying it to 3/4 of the subclasses.
	 */
	virtual void draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const bool mirrorX);

	/**
	 * Draws the cel to the target buffer using the given position and scaling
	 * parameters. The mirroring of the cel will be unchanged from any previous
	 * call to draw.
	 */
	void drawTo(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const Ratio &scaleX, const Ratio &scaleY) const;

	/**
	 * Creates a copy of this cel on the free store and returns a pointer to the
	 * new object. The new cel will point to a shared copy of bitmap/resource
	 * data.
	 */
	virtual CelObj *duplicate() const = 0;

	/**
	 * Retrieves a pointer to the raw resource data for this cel. This method
	 * cannot be used with a CelObjColor.
	 */
	virtual const SciSpan<const byte> getResPointer() const = 0;

	/**
	 * Reads the pixel at the given coordinates. This method is valid only for
	 * CelObjView and CelObjPic.
	 */
	virtual uint8 readPixel(const uint16 x, const uint16 y, const bool mirrorX) const;

	/**
	 * Submits the palette from this cel to the palette manager for integration
	 * into the master screen palette.
	 */
	void submitPalette() const;

#pragma mark -
#pragma mark CelObj - Drawing
private:
	template<typename MAPPER, typename SCALER>
	void render(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;

	template<typename MAPPER, typename SCALER>
	void render(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const Ratio &scaleX, const Ratio &scaleY) const;

	void drawHzFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawNoFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawUncompNoFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawUncompHzFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void scaleDraw(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void scaleDrawUncomp(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;

	void drawHzFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawNoFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawUncompNoFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawUncompHzFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void scaleDrawMap(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void scaleDrawUncompMap(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	// SSCI includes versions of the above functions with priority parameters
	// which are not actually used in SCI32

	void drawHzFlipNoMD(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawNoFlipNoMD(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawUncompNoFlipNoMD(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawUncompNoFlipNoMDNoSkip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawUncompHzFlipNoMD(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void drawUncompHzFlipNoMDNoSkip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void scaleDrawNoMD(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	void scaleDrawUncompNoMD(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const;
	// SSCI includes versions of the above functions with priority parameters
	// which are not actually used in SCI32

#pragma mark -
#pragma mark CelObj - Caching
protected:
	/**
	 * A monotonically increasing cache ID used to identify the least recently
	 * used item in the cache for replacement.
	 */
	static int _nextCacheId;

	/**
	 * A cache of cel objects used to avoid reinitialisation overhead for cels
	 * with the same CelInfo32.
	 */
	static Common::ScopedPtr<CelCache> _cache;

	/**
	 * Searches the cel cache for a CelObj matching the provided CelInfo32. If
	 * not found, -1 is returned. `nextInsertIndex` will receive the index of
	 * the oldest item in the cache, which can be used to replace the oldest
	 * item with a newer item.
	 */
	int searchCache(const CelInfo32 &celInfo, int *nextInsertIndex) const;

	/**
	 * Puts a copy of this CelObj into the cache at the given cache index.
	 */
	void putCopyInCache(int index) const;
};

#pragma mark -
#pragma mark CelObjView

/**
 * A CelObjView is the drawing primitive for a View type resource. Each
 * CelObjView corresponds to a single cel within a single loop of a view.
 */
class CelObjView : public CelObj {
private:
	/**
	 * Analyses resources without baked-in remap flags to determine whether or
	 * not they should be remapped.
	 */
	bool analyzeUncompressedForRemap() const;

	/**
	 * Analyses compressed resources without baked-in remap flags to determine
	 * whether or not they should be remapped.
	 */
	bool analyzeForRemap() const;

public:
	CelObjView(const GuiResourceId viewId, const int16 loopNo, const int16 celNo);
	~CelObjView() override {};

	using CelObj::draw;

	static int16 getNumLoops(const GuiResourceId viewId);
	static int16 getNumCels(const GuiResourceId viewId, const int16 loopNo);

	/**
	 * Draws the cel to the target buffer using the positioning, mirroring, and
	 * scaling information from the provided arguments.
	 */
	void draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, bool mirrorX, const Ratio &scaleX, const Ratio &scaleY);

	CelObjView *duplicate() const override;
	const SciSpan<const byte> getResPointer() const override;

	Common::Point getLinkPosition(const int16 linkId) const;
};

#pragma mark -
#pragma mark CelObjPic

/**
 * A CelObjPic is the drawing primitive for a Picture type resource. Each
 * CelObjPic corresponds to a single cel within a picture.
 */
class CelObjPic : public CelObj {
private:
	/**
	 * Analyses uncompressed resources without baked-in skip flags to determine
	 * whether or not they can use fast blitting.
	 */
	bool analyzeUncompressedForSkip() const;

public:
	/**
	 * The number of cels in the original picture resource.
	 */
	uint8 _celCount;

	/**
	 * The position of this cel relative to the top-left corner of the picture.
	 */
	Common::Point _relativePosition;

	/**
	 * The z-buffer priority for this cel. Higher prorities are drawn on top of
	 * lower priorities.
	 */
	int16 _priority;

	CelObjPic(const GuiResourceId pictureId, const int16 celNo);
	~CelObjPic() override {};

	using CelObj::draw;
	void draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const bool mirrorX) override;

	CelObjPic *duplicate() const override;
	const SciSpan<const byte> getResPointer() const override;
};

#pragma mark -
#pragma mark CelObjMem

/**
 * A CelObjMem is the drawing primitive for arbitrary bitmaps generated in
 * memory. Generated bitmaps in SCI32 include text & vector drawings and
 * per-pixel screen transitions like dissolves.
 */
class CelObjMem : public CelObj {
public:
	CelObjMem(const reg_t bitmap);
	~CelObjMem() override {};

	CelObjMem *duplicate() const override;
	const SciSpan<const byte> getResPointer() const override;
};

#pragma mark -
#pragma mark CelObjColor

/**
 * A CelObjColor is the drawing primitive for fast,
 * low-memory, flat color fills.
 */
class CelObjColor : public CelObj {
public:
	CelObjColor(const uint8 color, const int16 width, const int16 height);
	~CelObjColor() override {};

	using CelObj::draw;
	/**
	 * Block fills the target buffer with the cel color.
	 */
	void draw(Buffer &target, const Common::Rect &targetRect) const;
	void draw(Buffer &target, const ScreenItem &screenItem, const Common::Rect &targetRect, const bool mirrorX) override;
	void draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const bool mirrorX) override;

	CelObjColor *duplicate() const override;
	const SciSpan<const byte> getResPointer() const override;
};
} // End of namespace Sci

#endif
