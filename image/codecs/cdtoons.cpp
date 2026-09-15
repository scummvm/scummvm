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

#include "image/codecs/cdtoons.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/array.h"

namespace Image {

struct CDToonsDiff {
	byte *data;
	uint32 size;
	Common::Rect rect;
};

enum CDToonsBlockFlags : uint32 {
	/**
	 * Identifies palette data.
	 */
	kCDToonsBlockTypePalette = 0x01,
	/**
	 * Identifies RLE image data.
	 */
	kCDToonsBlockTypeImage = 0x02,
	kCDToonsBlockTypeMask = kCDToonsBlockTypePalette | kCDToonsBlockTypeImage,
	/**
	 * Marks a block header that has already been converted to host byte order.
	 */
	kCDToonsBlockFlagEndianConverted = 0x80,
};

enum CDToonsFrameFlags : uint32 {
	/**
	 * Declares the presence of the corresponding optional frame subchunk.
	 */
	kCDToonsFrameFlagHasMarkers = 0x02,
	/**
	 * Skips dirty-region clearing so transparent RLE runs retain existing pixels.
	 */
	kCDToonsFrameFlagPreservePreviousPixels = 0x08,
	kCDToonsFrameFlagHasXFrm = 0x10,
	kCDToonsFrameFlagHasBackgroundRects = 0x20,
	kCDToonsFrameFlagHasForegroundRects = 0x40,
	/**
	 * Marks a frame header that has already been converted to host byte order.
	 */
	kCDToonsFrameFlagEndianConverted = 0x80,
};

static const uint16 kCDToonsFrameFormat = 9;
static const byte kCDToonsActionSize = 10;

static const char *getBlockTypeName(uint16 typeFlags) {
	switch (typeFlags & kCDToonsBlockTypeMask) {
	case kCDToonsBlockTypePalette:
		return "palette";
	case kCDToonsBlockTypeImage:
		return "image";
	default:
		return "unknown";
	}
}

static Common::Rect readRect(Common::SeekableReadStream &stream) {
	Common::Rect rect;
	rect.top = stream.readUint16BE();
	rect.left = stream.readUint16BE();
	rect.bottom = stream.readUint16BE();
	rect.right = stream.readUint16BE();
	return rect;
}

static Common::Rect clipToSurface(const Common::Rect &rect, const Graphics::Surface &surface) {
	Common::Rect clipped = rect;
	clipped.clip(Common::Rect(0, 0, surface.w, surface.h));
	return clipped;
}

static void fillSurfaceRect(Graphics::Surface &surface, const Common::Rect &rect, byte color) {
	Common::Rect clipped = clipToSurface(rect, surface);
	if (clipped.isEmpty())
		return;

	for (int y = clipped.top; y < clipped.bottom; y++)
		memset(surface.getBasePtr(clipped.left, y), color, clipped.width());
}

static void copySurfaceRect(Graphics::Surface &dest, const Graphics::Surface &source, const Common::Rect &rect) {
	Common::Rect clipped = clipToSurface(rect, dest);
	clipped.clip(Common::Rect(0, 0, source.w, source.h));
	if (!clipped.isEmpty())
		dest.copyRectToSurface(source, clipped.left, clipped.top, clipped);
}

CDToonsDecoder::CDToonsDecoder(uint16 width, uint16 height) : _palette(256) {
	debugN(5, "CDToons: width %d, height %d\n", width, height);

	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_backingSurface = new Graphics::Surface();
	_backingSurface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_backingSurfaceValid = false;

	_currentPaletteId = 0;
	_dirtyPalette = false;
}

CDToonsDecoder::~CDToonsDecoder() {
	_surface->free();
	delete _surface;
	_backingSurface->free();
	delete _backingSurface;

	for (auto &block : _blocks)
		delete[] block._value.data;
}

Graphics::Surface *CDToonsDecoder::decodeFrame(Common::SeekableReadStream &stream) {
	// (0x00) The first word identifies the frame-header format.
	// There is only one CDToons frame format: 9.
	uint16 frameFormat = stream.readUint16BE();
	if (frameFormat != kCDToonsFrameFormat)
		error("CDToons: unsupported frame format %d", frameFormat);

	// (0x02) Sequential frame ID.
	uint16 frameId = stream.readUint16BE();
	// (0x04) Last frame through which newly supplied block records remain valid.
	uint16 blocksValidUntil = stream.readUint16BE();
	// This authoring hint identifies a primary action involved in the frame.
	// (0x06) It does not control rendering; only actions and block IDs do.
	byte primaryActionIndexHint = stream.readByte();
	// (0x07) Palette index used when clearing frame regions.
	byte backgroundColor = stream.readByte();
	debugN(5, "CDToons frame %d, size %d, format %d, blocks valid until %d, primary action %d, bkg color is %02x\n",
		frameId, (int)stream.size(), frameFormat, blocksValidUntil, primaryActionIndexHint, backgroundColor);

	// (0x08) Frame clip rectangle.
	Common::Rect clipRect = readRect(stream);
	debugN(9, "CDToons clipRect: (%d, %d) to (%d, %d)\n",
		clipRect.left, clipRect.top, clipRect.right, clipRect.bottom);

	// (0x10) Frame output dirty rectangle.
	Common::Rect dirtyRect = readRect(stream);
	debugN(9, "CDToons dirtyRect: (%d, %d) to (%d, %d)\n",
		dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);

	// (0x18) Optional-subchunk and compositing flags.
	uint32 flags = stream.readUint32BE();
	if (flags & kCDToonsFrameFlagEndianConverted)
		error("CDToons: frame was already endian-converted");
	debugN(5, "CDToons flags: %08x\n", flags);

	// (0x1C) Number of block records supplied by this frame.
	uint16 blockCount = stream.readUint16BE();
	// (0x1E) Offset from the frame start to the first block record.
	uint16 blockOffset = stream.readUint16BE();
	debugN(9, "CDToons: %d blocks at 0x%04x\n",
		blockCount, blockOffset);

	// (0x20) The max block ID used by block records and references.
	uint16 maxBlockId = stream.readUint16BE();
	debugN(5, "CDToons highest block ID: %d\n", maxBlockId);

	// (0x22) Number of action records in the frame header.
	byte actionCount = stream.readByte();
	// (0x23) The stream declares the size of each action record.
	byte actionSize = stream.readByte();
	if (actionSize != kCDToonsActionSize) // CDToons uses a fixed 10-byte layout.
		error("CDToons: unsupported action size %d", actionSize);

	// (0x24) Block ID of the palette selected for this frame.
	uint16 paletteId = stream.readUint16BE();
	if (paletteId && maxBlockId < paletteId)
		error("CDToons: palette block ID %d exceeded declared maximum %d", paletteId, maxBlockId);
	// (0x26) A nonzero value suppresses palette realization for 8-bit indexed-color output.
	uint16 paletteAlreadySet = stream.readUint16BE();
	// (0x28) This reserved word is copied with the cached action descriptor but never consumed.
	uint16 reserved40 = stream.readUint16BE();
	// (0x2A) This revision decides whether a repeated frame ID with paletteAlreadySet should be composited again.
	uint16 sameFrameRevision = stream.readUint16BE();
	debugN(5, "CDToons: action size %d, palette id %04x, palette already set %04x, reserved %04x, same-frame revision %04x\n",
		actionSize, paletteId, paletteAlreadySet, reserved40, sameFrameRevision);

	Common::Array<CDToonsAction> actions;

	// Actions reference cached image blocks by ID.
	// - The rectangle supplies the placement and validity fields.
	// - The encoded block supplies its dimensions.
	// The first entry starts at frame offset 0x2C; offsets below are entry-relative.
	for (uint i = 0; i < actionCount; i++) {
		CDToonsAction action;
		// (0x00) Cached image block ID.
		action.blockId = stream.readUint16BE();
		if (action.blockId && action.blockId != 0xffff && maxBlockId < action.blockId)
			error("CDToons: action block ID %d exceeded declared maximum %d", action.blockId, maxBlockId);
		// (0x02) Action placement and validity rectangle.
		action.rect = readRect(stream);
		debugN(9, "CDToons action: render block %d at (%d, %d) to (%d, %d)\n",
			action.blockId, action.rect.left, action.rect.top, action.rect.right, action.rect.bottom);
		actions.push_back(action);
	}

	if (stream.pos() > blockOffset)
		error("CDToons header ended at 0x%08x, but blocks should have started at 0x%08x",
			(int)stream.pos(), blockOffset);

	if (stream.pos() != blockOffset)
		error("CDToons had %d unknown bytes after header", blockOffset - (int)stream.pos());

	// A block record contains an ID, flags, declared size, frame lifetime, and payload.
	// - Payloads referenced by paletteId are palettes.
	// - Action payloads contain an image header followed by scanline RLE data.
	// Offsets below are relative to the block record selected by blockOffset.
	for (uint i = 0; i < blockCount; i++) {
		// (0x00) Block ID used by palette and action references.
		uint16 blockId = stream.readUint16BE();
		if (maxBlockId < blockId)
			error("CDToons: block ID %d exceeded declared maximum %d", blockId, maxBlockId);
		if (_blocks.contains(blockId))
			error("CDToons: new block %d was already seen", blockId);

		CDToonsBlock block;
		// (0x02) Payload type and storage flags.
		block.typeFlags = stream.readUint16BE();
		if (block.typeFlags & kCDToonsBlockFlagEndianConverted)
			error("CDToons: block was already endian-converted");
		// (0x04) Total block-record size, including this fixed header.
		block.size = stream.readUint32BE();
		if (block.size < 14)
			error("CDToons: block size was %d, too small", block.size);
		block.size -= 14;
		// (0x08) First frame for which the cached block is valid.
		block.startFrame = stream.readUint16BE();
		// (0x0A) Last frame for which the cached block is valid.
		block.endFrame = stream.readUint16BE();
		// (0x0C) Hints the block's primary action slot. Does not affect image decoding.
		byte blockPrimaryActionIndexHint = stream.readByte();
		// (0x0D) Reserved byte that does not affect image decoding.
		byte blockReserved13 = stream.readByte();
		block.data = new byte[block.size];
		// (0x0E) Palette or image payload.
		stream.read(block.data, block.size);

		debugN(9, "CDToons block id 0x%04x of size 0x%08x, type %s, flags %04x, from frame %d to %d, primary action %d, reserved %02x\n",
			blockId, block.size, getBlockTypeName(block.typeFlags), block.typeFlags,
			block.startFrame, block.endFrame, blockPrimaryActionIndexHint, blockReserved13);

		_blocks[blockId] = block;
	}

	bool hasXFrm = false;
	byte xFrmBegin = 0;
	byte xFrmCount = 0;
	Common::Rect xFrmRect;
	Common::Array<CDToonsDiff> diffs;
	Common::Array<Common::Rect> backgroundRects;
	Common::Array<Common::Rect> foregroundRects;

	while (true) {
		int32 nextPos = stream.pos();
		// Offsets are relative to the tagged block.
		// (0x00) Four-character block type.
		uint32 tag = stream.readUint32BE();
		// (0x04) Total tagged-block size, including the tag and this field.
		uint32 size = stream.readUint32BE();
		if (size < 8)
			error("CDToons: block '%s' size was %d, too small", tag2str(tag), size);
		nextPos += size;
		if (stream.size() < nextPos)
			error("CDToons: block '%s' ran past the frame", tag2str(tag));

		switch (tag) {
		case MKTAG('D','i','f','f'):
			{
			// [Diff] contains a count and aggregate clip rectangle, followed by variable-sized replacement rectangles.
			// Each entry stores its bounds, total entry size, dimensions, two reserved words, and scanline RLE.
			// Diff entries patch the output directly instead of using frame actions.
			debugN(5, "CDToons: Diff\n");
			if (nextPos - stream.pos() < 10)
				error("CDToons: Diff block was too small");
			// (0x08) Number of replacement-rectangle entries.
			uint16 count = stream.readUint16BE();

			// (0x0A) Aggregate clip rectangle for the replacement entries.
			Common::Rect diffClipRect = readRect(stream);
			debugN(9, "CDToons diffClipRect: (%d, %d) to (%d, %d)\n",
				diffClipRect.left, diffClipRect.top, diffClipRect.right, diffClipRect.bottom);

			debugN(5, "CDToons Diff: %d subentries\n", count);
			// The first entry starts at block offset 0x12; offsets below are entry-relative.
			for (uint i = 0; i < count; i++) {
				CDToonsDiff diff;
				int32 entryStart = stream.pos();
				if (nextPos - entryStart < 20)
					error("CDToons: Diff entry %d header ran past the block", i);

				// (0x00) Replacement rectangle.
				diff.rect = readRect(stream);
				// (0x08) Total size of this variable-sized entry.
				uint32 entrySize = stream.readUint32BE();
				if (entrySize < 20)
					error("CDToons: Diff entry size was %d, too small", entrySize);
				if (static_cast<uint32>(nextPos - entryStart) < entrySize)
					error("CDToons: Diff entry %d ran past the block", i);

				// (0x0C) Width that duplicates the rectangle.
				uint16 diffWidth = stream.readUint16BE();
				// (0x0E) Height that duplicates the rectangle.
				uint16 diffHeight = stream.readUint16BE();
				// The render size is derived from the rectangle, and all four words are otherwise skipped.
				// (0x10) First reserved word.
				uint16 reserved16 = stream.readUint16BE();
				// (0x12) Second reserved word.
				uint16 reserved18 = stream.readUint16BE();
				diff.size = entrySize - 20;

				if (diffWidth != diff.rect.width() || diffHeight != diff.rect.height())
					error("CDToons: Diff sizes didn't match");
				debugN(5, "CDToons Diff: size %d, frame from (%d, %d) to (%d, %d), reserved %04x, %04x\n",
					diff.size, diff.rect.left, diff.rect.top, diff.rect.right, diff.rect.bottom,
					reserved16, reserved18);

				diff.data = new byte[diff.size];
				// (0x14) Scanline RLE payload.
				stream.read(diff.data, diff.size);
				diffs.push_back(diff);
			}
			}
			break;
		case MKTAG('X','F','r','m'):
			{
			// [XFrm] stores a one-based action start, an action count, one retained background update/clip rectangle, and optional reserved trailing data.
			// The selected actions update the clean background;
			// later actions are foreground overlays after that background is restored to the output.
			debugN(5, "CDToons: XFrm\n");
			if (!(flags & kCDToonsFrameFlagHasXFrm))
				error("CDToons: XFrm block was not declared by frame flags");

			if (hasXFrm)
				error("CDToons: duplicate XFrm");
			if (nextPos - stream.pos() < 10)
				error("CDToons: XFrm block was too small");

			hasXFrm = true;
			// (0x08) One-based index of the first retained-background action.
			xFrmBegin = stream.readByte();
			// (0x09) Number of retained-background actions.
			xFrmCount = stream.readByte();
			debugN(9, "CDToons XFrm: run %d actions from %d\n", xFrmCount, xFrmBegin - 1);

			// (0x0A) The selected actions update a retained background layer inside this rectangle.
			xFrmRect = readRect(stream);
			debugN(9, "CDToons XFrm dirtyRect: (%d, %d) to (%d, %d)\n",
				xFrmRect.left, xFrmRect.top, xFrmRect.right, xFrmRect.bottom);

			// Bytes after the update rectangle are reserved extension data.
			// They are not a second rectangle.
			if (stream.pos() != nextPos) {
				debugN(9, "CDToons XFrm: skipping %d reserved bytes\n", nextPos - (int32)stream.pos());
				stream.seek(nextPos);
			}
			}
			break;
		case MKTAG('M','r','k','s'):
			{
			// [Mrks] contains cues attached to this point in the video timeline.
			// [PreF] and [PstF] cues notify the player immediately before or after the associated frame is rendered,
			// allowing an external application to synchronize its own actions with video playback.
			// This block does not have rendering commands and do not change the decoded image.

			// LOGO025.MOV from Logical Journey of the Zoombinis v1.11KR contains this block, but the game never uses the callback feature.
			// The actual data does not have any valid [PreF] or [PstF] tags, too.
			// Thus, ScummVM decoder does not implement a callback system for such timeline cues.
			debugN(5, "CDToons: Mrks\n");
			if (!(flags & kCDToonsFrameFlagHasMarkers))
				error("CDToons: Mrks block was not declared by frame flags");

			if (nextPos - stream.pos() < 2)
				error("CDToons: Mrks block was too small");

			// (0x08) Number of timeline-marker entries.
			uint16 markerCount = stream.readUint16BE();
			debugN(9, "CDToons Mrks: %d markers\n", markerCount);
			// The first entry starts at block offset 0x0A; offsets below are entry-relative.
			for (uint i = 0; i < markerCount; i++) {
				if (nextPos - stream.pos() < 8)
					error("CDToons: Mrks entry %d header ran past the block", i);

				// (0x00) Four-character marker type.
				uint32 markerTag = stream.readUint32BE();
				// (0x04) Total marker-entry size, including this header.
				uint32 markerSize = stream.readUint32BE();
				if (markerSize < 8)
					error("CDToons: Mrks entry %d size was %d, too small", i, markerSize);
				if (nextPos - stream.pos() < markerSize - 8)
					error("CDToons: Mrks entry %d ran past the block", i);

				// [PreF] and [PstF] select pre-render and post-render delivery.
				if (markerTag == MKTAG('P','r','e','F') || markerTag == MKTAG('P','s','t','F')) {
					if (markerSize < 12)
						error("CDToons: callback marker '%s' was too small", tag2str(markerTag));

					// (0x08) Size of the application callback payload.
					uint32 callbackDataSize = stream.readUint32BE();
					if (markerSize - 12 < callbackDataSize)
						error("CDToons: callback marker '%s' data ran past the entry", tag2str(markerTag));
					debugN(9, "CDToons marker '%s': entry size %d, callback data size %d\n",
						tag2str(markerTag), markerSize, callbackDataSize);
					stream.skip(markerSize - 12);
				} else {
					debugN(9, "CDToons marker tag %08x: entry size %d, ignored\n", markerTag, markerSize);
					stream.skip(markerSize - 8);
				}
			}
			}
			break;
		case MKTAG('S','c','a','l'):
			// [Scal] is an opaque extension payload.
			debugN(5, "CDToons: skipping opaque Scal block (%d payload bytes)\n", size - 8);
			stream.seek(nextPos);
			break;
		case MKTAG('W','r','M','p'):
			// [WrMp] is an opaque extension payload.
			debugN(5, "CDToons: skipping opaque WrMp block (%d payload bytes)\n", size - 8);
			stream.seek(nextPos);
			break;
		case MKTAG('F','r','t','R'):
			{
			// [FrtR] contains a count followed by output dirty rectangles.
			// They limit foreground/output clearing and the final transfer to the host surface.
			debugN(5, "CDToons: FrtR\n");
			if (!(flags & kCDToonsFrameFlagHasForegroundRects))
				error("CDToons: FrtR block was not declared by frame flags");
			if (nextPos - stream.pos() < 2)
				error("CDToons: FrtR block was too small");

			// (0x08) Number of output dirty rectangles.
			uint16 count = stream.readUint16BE();
			if ((nextPos - stream.pos()) / 8 < count)
				error("CDToons: FrtR rectangle list ran past the block");
			debugN(9, "CDToons FrtR: %d dirty rectangles\n", count);
			for (uint i = 0; i < count; i++) {
				// (0x0A + i * 0x08) Output dirty rectangle.
				Common::Rect dirtyRectFrtR = readRect(stream);
				foregroundRects.push_back(dirtyRectFrtR);
				debugN(9, "CDToons FrtR dirtyRect: (%d, %d) to (%d, %d)\n",
					dirtyRectFrtR.left, dirtyRectFrtR.top, dirtyRectFrtR.right, dirtyRectFrtR.bottom);
			}
			}
			break;
		case MKTAG('B','c','k','R'):
			{
			// [BckR] contains a count followed by retained-background dirty rectangles.
			// When clearing is enabled, these regions are reset to the frame background color before the XFrm action range updates them.
			debugN(5, "CDToons: BckR\n");
			if (!(flags & kCDToonsFrameFlagHasBackgroundRects))
				error("CDToons: BckR block was not declared by frame flags");
			if (nextPos - stream.pos() < 2)
				error("CDToons: BckR block was too small");

			// (0x08) Number of retained-background dirty rectangles.
			uint16 count = stream.readUint16BE();
			if ((nextPos - stream.pos()) / 8 < count)
				error("CDToons: BckR rectangle list ran past the block");
			debugN(9, "CDToons BckR: %d subentries\n", count);
			for (uint i = 0; i < count; i++) {
				// (0x0A + i * 0x08) Retained-background dirty rectangle.
				Common::Rect dirtyRectBckR = readRect(stream);
				backgroundRects.push_back(dirtyRectBckR);
				debugN(9, "CDToons BckR dirtyRect: (%d, %d) to (%d, %d)\n",
					dirtyRectBckR.left, dirtyRectBckR.top, dirtyRectBckR.right, dirtyRectBckR.bottom);
			}
			}
			break;
		default:
			warning("Unknown CDToons tag '%s'", tag2str(tag));
		}

		if (stream.pos() > nextPos)
			error("CDToons ran off the end of a block while reading it (at %d, next block at %d)",
				(int)stream.pos(), nextPos);
		if (stream.pos() != nextPos) {
			warning("CDToons had %d unknown bytes after block", nextPos - (int32)stream.pos());
			stream.seek(nextPos);
		}

		if (stream.pos() == stream.size())
			break;
	}

	for (uint i = 0; i < diffs.size(); i++) {
		Image::CDToonsDiff &diff = diffs[i];
		renderBlock(*_surface, diff.data, diff.size, diff.rect.left, diff.rect.top, diff.rect.width(), diff.rect.height());
		delete[] diff.data;
	}
	if (!diffs.empty())
		return _surface;

	int xFrmStart = static_cast<int8>(xFrmBegin) - 1;
	if (xFrmStart < 0)
		xFrmStart = 0;
	uint backgroundBegin = MIN<uint>(xFrmStart, actions.size());
	uint backgroundEnd = MIN<uint>(backgroundBegin + xFrmCount, actions.size());

	if (hasXFrm && xFrmCount && !xFrmRect.isEmpty()) {
		if (!_backingSurfaceValid) {
			fillSurfaceRect(*_backingSurface, Common::Rect(_backingSurface->w, _backingSurface->h), backgroundColor);
			_backingSurfaceValid = true;
		}

		// Repeated RLE runs of color zero are transparent and leave destination pixels unchanged.
		// Clear dirty background regions unless this frame asks to retain the previous pixels through those transparent runs.
		if (!(flags & kCDToonsFrameFlagPreservePreviousPixels)) {
			if (!backgroundRects.empty()) {
				for (const Common::Rect &rect : backgroundRects)
					fillSurfaceRect(*_backingSurface, rect, backgroundColor);
			} else {
				fillSurfaceRect(*_backingSurface, xFrmRect, backgroundColor);
			}
		}
		renderActions(actions, backgroundBegin, backgroundEnd, *_backingSurface, &xFrmRect);
	} else if (frameId == 1 && !actions.empty() && actions[0].blockId && actions[0].blockId != 0xffff) {
		if (!_backingSurfaceValid) {
			fillSurfaceRect(*_backingSurface, Common::Rect(_backingSurface->w, _backingSurface->h), backgroundColor);
			_backingSurfaceValid = true;
		}
		renderActions(actions, 0, 1, *_backingSurface, nullptr);
	}

	if (_backingSurfaceValid) {
		copySurfaceRect(*_surface, *_backingSurface, dirtyRect);
	} else if (!(flags & kCDToonsFrameFlagPreservePreviousPixels)) {
		if (!foregroundRects.empty()) {
			for (const Common::Rect &rect : foregroundRects)
				fillSurfaceRect(*_surface, rect, backgroundColor);
		} else {
			fillSurfaceRect(*_surface, dirtyRect, backgroundColor);
		}
	}

	uint foregroundBegin = hasXFrm ? backgroundEnd : MIN<uint>(1, actions.size());
	renderActions(actions, foregroundBegin, actions.size(), *_surface, nullptr);

	if (paletteId && _currentPaletteId != paletteId) {
		if (!_blocks.contains(paletteId))
			error("CDToons: no block for palette %04x", paletteId);
		if (_blocks[paletteId].size != 2 * 3 * 256)
			error("CDToons: palette %04x is wrong size (%d)", paletteId, _blocks[paletteId].size);

		_currentPaletteId = paletteId;
		if (!paletteAlreadySet)
			setPalette(_blocks[paletteId].data);
	}

	return _surface;
}

void CDToonsDecoder::renderActions(const Common::Array<CDToonsAction> &actions, uint begin, uint end, Graphics::Surface &surface, const Common::Rect *clipRect) {
	end = MIN<uint>(end, actions.size());
	for (uint i = begin; i < end; i++) {
		const CDToonsAction &action = actions[i];
		if (!_blocks.contains(action.blockId) || !action.rect.right)
			continue;

		CDToonsBlock &block = _blocks[action.blockId];
		if (block.size < 14)
			error("CDToons: image block %d was too small", action.blockId);
		// Offsets are relative to the image payload at block-record offset 0x0E.
		// (0x02) Encoded image width.
		uint16 width = READ_BE_UINT16(block.data + 2);
		// (0x00) Encoded image height.
		uint16 height = READ_BE_UINT16(block.data);

		// (0x0E) Scanline RLE begins after the fixed image header.
		renderBlock(surface, block.data + 14, block.size - 14,
			action.rect.left, action.rect.top, width, height, clipRect);
	}
}

void CDToonsDecoder::renderBlock(Graphics::Surface &surface, byte *data, uint dataSize, int destX, int destY, uint width, uint height, const Common::Rect *clipRect) {
	byte *currData = data;
	byte *dataEnd = data + dataSize;

	debugN(9, "CDToons renderBlock at (%d, %d), width %d, height %d\n",
		destX, destY, width, height);

	Common::Rect drawRect(destX, destY, destX + static_cast<int>(width), destY + static_cast<int>(height));
	drawRect.clip(Common::Rect(0, 0, surface.w, surface.h));
	if (clipRect)
		drawRect.clip(*clipRect);
	if (drawRect.isEmpty())
		return;

	uint leftToSkipInitial = drawRect.left - destX;
	uint drawWidth = drawRect.width();

	for (uint y = 0; y < height; y++) {
		if (currData + 2 > dataEnd)
			error("CDToons renderBlock overran whole data by %d bytes", (uint32)(currData - dataEnd));

		// Offsets are relative to this scanline entry.
		// (0x00) Byte length of all RLE runs in this scanline.
		uint16 lineSize = READ_BE_UINT16(currData);
		currData += 2;
		// (0x02) RLE run entries begin at scanline offset.
		byte *nextLine = currData + lineSize;

		if (nextLine > dataEnd)
			error("CDToons renderBlock was going to overrun data by %d bytes (line size %d)",
				(uint32)(nextLine - dataEnd), (uint32)(nextLine - currData));

		int outputY = destY + static_cast<int>(y);
		if (outputY < drawRect.top || drawRect.bottom <= outputY) {
			currData = nextLine;
			continue;
		}

		byte *pixels = (byte *)surface.getBasePtr(drawRect.left, outputY);

		uint leftToSkip = leftToSkipInitial;
		uint x = 0;
		bool done = false;
		while (x < drawWidth && !done) {
			if (nextLine <= currData)
				error("CDToons renderBlock reached the end of a line before its pixels");
			// Offsets are relative to this variable-sized run entry.
			// (0x00) Run mode and encoded length.
			uint runSize = *currData;
			currData++;
			bool raw = !(runSize & 0x80);
			runSize = (runSize & 0x7f) + 1;

			if (leftToSkip) {
				if (runSize <= leftToSkip) {
					leftToSkip -= runSize;
					if (raw)
						currData += runSize;
					else
						currData++;
					continue;
				} else {
					runSize -= leftToSkip;
					if (raw)
						currData += leftToSkip;
					leftToSkip = 0;
				}
			}

			if (drawWidth <= x + runSize) {
				runSize = drawWidth - x;
				done = true;
			}

			if (raw) {
				// (0x01) Literal pixel bytes begin immediately after the run header.
				memcpy(pixels + x, currData, runSize);
				currData += runSize;
				x += runSize;
			} else {
				// (0x01) Repeated palette index.
				byte color = *currData;
				currData++;
				if (color) {
					memset(pixels + x, color, runSize);
				}
				x += runSize;
			}

			if (currData > nextLine) {
				warning("CDToons renderBlock overran line by %d bytes", (uint32)(currData - nextLine));
				return;
			}
		}

		currData = nextLine;
	}
}

void CDToonsDecoder::setPalette(byte *data) {
	_dirtyPalette = true;

	// A lovely QuickTime palette
	for (uint i = 0; i < 256; i++) {
		_palette.set(i, *data, *(data + 2), *(data + 4));
		data += 6;
	}

	_palette.set(0, 0, 0, 0);
}

} // End of namespace Image
