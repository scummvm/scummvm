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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_SYSTEM_SCREEN_H
#define ASYLUM_SYSTEM_SCREEN_H

#include "common/array.h"
#include "common/queue.h"
#include "common/rect.h"

#include "graphics/surface.h"

#include "asylum/shared.h"

namespace Asylum {

#define PALETTE_SIZE (256 * 3)

class AsylumEngine;
class GraphicResource;
class ResourcePack;

struct GraphicFrame;

enum GraphicItemType {
	kGraphicItemNormal = 1,
	kGraphicItemMasked = 5
};

typedef struct GraphicQueueItem {
	int32 priority;

	GraphicItemType type;
	ResourceId resourceId;
	uint32 frameIndex;
	Common::Point source;
	ResourceId resourceIdDestination;
	Common::Point destination;
	DrawFlags flags;
	int32 transTableNum;

	GraphicQueueItem() {
		priority = 0;

		type = kGraphicItemNormal;
		resourceId = kResourceNone;
		frameIndex = 0;
		resourceIdDestination = kResourceNone;
		flags = kDrawFlagNone;
		transTableNum = 0;
	}
} GraphicQueueItem;

struct FadeParameters {
	ResourceId resourceId;
	int  ticksWait;
	int  delta;
	uint nextTick;
	int  step;
};

class Screen {
public:
	Screen(AsylumEngine *_vm);
	~Screen();

	// Drawing
	void draw(ResourceId resourceId);
	void draw(ResourceId resourceId, uint32 frameIndex, const Common::Point &source, DrawFlags flags = kDrawFlagNone, bool colorKey = true);
	void draw(ResourceId resourceId, uint32 frameIndex, const int16 (*srcPtr)[2], DrawFlags flags = kDrawFlagNone, bool colorKey = true);
	void draw(GraphicResource *resource, uint32 frameIndex, const Common::Point &source, DrawFlags flags = kDrawFlagNone, bool colorKey = true);
	void drawTransparent(ResourceId resourceId, uint32 frameIndex, const Common::Point &source, DrawFlags flags, uint32 transTableNum);
	void drawTransparent(GraphicResource *resource, uint32 frameIndex, const Common::Point &source, DrawFlags flags, uint32 transTableNum);
	void draw(ResourceId resourceId, uint32 frameIndex, const Common::Point &source, DrawFlags flags, ResourceId resourceId2, const Common::Point &destination, bool colorKey = true);
	void draw(const Graphics::Surface &surface, int x, int y);

	// Misc
	void clear();
	void clearDefaultColor() { memset(_mainPalette, 0, 3); setupPalette(NULL, 0, 0); }
	void drawWideScreenBars(int16 barSize) const;
	void fillRect(int16 x, int16 y, int16 x2, int16 y2, uint32 color);
	void copyBackBufferToScreen();
	void setFlag(int16 val) { _flag = (val < -1) ? -1 : val; }
	int16 getFlag() { return _flag; }

	// Palette
	void setPalette(ResourceId id);
	const byte *getPalette() { return _mainPalette; }
	void setMainPalette(const byte *data);
	void loadGrayPalette();
	void updatePalette();
	void updatePalette(int32 param);
	void setupPalette(byte *buffer, int start, int count);

	bool isFading() { return _isFading; }
	void queuePaletteFade(ResourceId resourceId, int32 ticksWait, int32 delta);
	void paletteFade(uint32 start, int32 ticksWait, int32 delta);
	void stopPaletteFade(char red, char green, char blue);
	void stopPaletteFadeAndSet(ResourceId id, int32 ticksWait, int32 delta);
	void processPaletteFadeQueue();

	// Gamma
	void setPaletteGamma(ResourceId id);
	void setGammaLevel(ResourceId id);

	// Transparency tables
	void setupTransTable(ResourceId resourceId);
	void setupTransTables(uint32 count, ...);
	void selectTransTable(uint32 index);

	// Graphic queue
	void addGraphicToQueue(ResourceId resourceId, uint32 frameIndex, const Common::Point &point, DrawFlags flags, int32 transTableNum, int32 priority);
	void addGraphicToQueue(ResourceId resourceId, uint32 frameIndex, const int16 (*pointPtr)[2], DrawFlags flags, int32 transTableNum, int32 priority);
	void addGraphicToQueueCrossfade(ResourceId resourceId, uint32 frameIndex, const Common::Point &source, int32 objectResourceId, const Common::Point &destination, uint32 transTableNum);
	void addGraphicToQueueMasked(ResourceId resourceId, uint32 frameIndex, const Common::Point &source, int32 objectResourceId, const Common::Point &destination, DrawFlags flags, int32 priority);
	void addGraphicToQueue(GraphicQueueItem const &item);
	void drawGraphicsInQueue();
	void clearGraphicsInQueue();
	void deleteGraphicFromQueue(ResourceId resourceId);
	bool isGraphicQueueEmpty() { return _queueItems.empty(); }

	// Used by Video
	void copyToBackBuffer(const byte *buffer, int32 pitch, int16 x, int16 y, uint16 width, uint16 height, bool mirrored = false);

	// Debug
	void drawLine(const Common::Point &source, const Common::Point &destination, uint32 color = 0xFF);
	void drawLine(const int16 (*srcPtr)[2], const int16 (*dstPtr)[2], uint32 color = 0xFF);
	void drawRect(const Common::Rect &rect, uint32 color = 0xFF);
	void copyToBackBufferClipped(Graphics::Surface *surface, int16 x, int16 y);

	// Used by Writings puzzle and Chinese renderer
	Graphics::Surface *getSurface() { return &_backBuffer; };

private:
	AsylumEngine *_vm;

	Graphics::Surface _backBuffer;
	Common::Rect _clipRect;
	Common::Array<GraphicQueueItem> _queueItems;

	int16 _flag;
	bool _useColorKey;

	// Transparency tables
	uint32 _transTableCount;
	byte *_transTable;
	byte *_transTableBuffer;
	void clearTransTables();

	// Palette
	byte _currentPalette[PALETTE_SIZE];
	byte _mainPalette[PALETTE_SIZE];
	byte _fromPalette[PALETTE_SIZE];
	byte _toPalette[PALETTE_SIZE];
	bool _isFading;
	bool _fadeStop;
	Common::Queue<FadeParameters> _fadeQueue;

	byte *getPaletteData(ResourceId id);
	void setPaletteGamma(byte *data, byte *target = NULL);

	void stopQueuedPaletteFade();
	void initQueuedPaletteFade(ResourceId id, int32 delta);
	void runQueuedPaletteFade(ResourceId id, int32 delta, int i);

	// Graphic queue
	static bool graphicQueueItemComparator(const GraphicQueueItem &item1, const GraphicQueueItem &item2);

	// Misc
	void clip(Common::Rect *source, Common::Rect *destination, int32 flags) const;

	void draw(GraphicResource *resource, uint32 frameIndex, const Common::Point &source, DrawFlags flags, ResourceId resourceId2, const Common::Point &destination, bool colorKey = true);

	// Screen blitting
	void blit(GraphicFrame *frame, Common::Rect *source, Common::Rect *destination, int32 flags);
	void blitMasked(GraphicFrame *frame, Common::Rect *source, byte *maskData, Common::Rect *sourceMask, Common::Rect *destMask, uint16 maskWidth, Common::Rect *destination, int32 flags);
	void blitTranstable        (byte *dstBuffer, byte *srcBuffer, int16 height, int16 width, uint16 srcPitch, uint16 dstPitch) const;
	void blitTranstableMirrored(byte *dstBuffer, byte *srcBuffer, int16 height, int16 width, uint16 srcPitch, uint16 dstPitch) const;
	void blitMirrored          (byte *dstBuffer, byte *srcBuffer, int16 height, int16 width, uint16 srcPitch, uint16 dstPitch) const;
	void blitMirroredColorKey  (byte *dstBuffer, byte *srcBuffer, int16 height, int16 width, uint16 srcPitch, uint16 dstPitch) const;
	void blitRaw               (byte *dstBuffer, byte *srcBuffer, int16 height, int16 width, uint16 srcPitch, uint16 dstPitch) const;
	void blitRawColorKey       (byte *dstBuffer, byte *srcBuffer, int16 height, int16 width, uint16 srcPitch, uint16 dstPitch) const;
	void blitCrossfade         (byte *dstBuffer, byte *srcBuffer, byte *objectBuffer, int16 height, int16 width, uint16 srcPitch, uint16 dstPitch, uint16 objectPitch) const;
	void bltMasked             (byte *srcBuffer, byte *maskBuffer, int16 height, int16 width, uint16 srcPitch, uint16 maskPitch, byte zoom, byte *dstBuffer, uint16 dstPitch) const;

	// DirectDraw-equivalent functions
	void blt(Common::Rect *dest, GraphicFrame *frame, Common::Rect *source, int32 flags);
	void bltFast(int16 dX, int16 dY, GraphicFrame *frame, Common::Rect *source);

	void copyToBackBufferWithTransparency(byte *buffer, int32 pitch, int16 x, int16 y, uint16 width, uint16 height, bool mirrored = false);

	// Debug
	void drawZoomedMask(byte *mask, uint16 height, uint16 width, uint16 maskPitch);
};

} // end of namespace Asylum

#endif // ASYLUM_SYSTEM_SCREEN_H
