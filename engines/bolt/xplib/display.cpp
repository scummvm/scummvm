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

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

namespace Bolt {

bool XpLib::initDisplay() {
	g_virtualWidth = (_bolt->g_extendedViewport) ? EXTENDED_SCREEN_WIDTH : SCREEN_WIDTH;
	g_virtualHeight = (_bolt->g_extendedViewport) ? EXTENDED_SCREEN_HEIGHT : SCREEN_HEIGHT;
	g_currentDisplayPage = stFront;

	// Create front and back display surfaces
	if (!createSurface(&g_surfaces[stFront]))
		return false;

	if (!createSurface(&g_surfaces[stBack]))
		return false;

	fillDisplay(0, stFront);
	fillDisplay(0, stBack);

	g_prevRenderFlags = 0;
	g_renderFlags = 0;
	g_frameRateFPS = 0;
	g_overlayCount = 0;
	g_prevDirtyCount = 0;
	g_prevDirtyValid = 0;

	g_cursorBackgroundSave.pixelData = g_cursorBackgroundSaveBuffer;
	g_cursorBackgroundSave.width = 16;
	g_cursorBackgroundSave.height = 16;

	return true;
}

void XpLib::shutdownDisplay() {
	freeSurface(&g_surfaces[stFront]);
	freeSurface(&g_surfaces[stBack]);
	stopCycle();
}

bool XpLib::createSurface(XPSurface *surf) {
	if (surf->mainPic.width == g_surfaceWidth && surf->mainPic.height == g_surfaceHeight)
		return true;

	freeSurface(surf);

	surf->mainPic.pixelData = (byte *)allocMem((uint32)g_surfaceWidth * (uint32)g_surfaceHeight);
	if (!surf->mainPic.pixelData)
		return false;

	surf->mainPic.palette = (byte *)allocMem(127 * 3);
	if (!surf->mainPic.palette)
		return false;

	surf->mainPic.width = g_surfaceWidth;
	surf->mainPic.height = g_surfaceHeight;

	for (int16 i = 0; i < 127 * 3; i += 3) {
		surf->mainPic.palette[i] = 0;
		surf->mainPic.palette[i + 1] = 0;
		surf->mainPic.palette[i + 2] = 0;
	}

	surf->mainPic.paletteStart = 1;
	surf->mainPic.paletteCount = 127;
	surf->mainPic.flags = 0;
	surf->overlayPic.pixelData = nullptr;
	surf->dirtyPalStart = 0;
	surf->dirtyPalEnd = 0;

	return true;
}

void XpLib::freeSurface(XPSurface *surf) {
	if (surf->mainPic.palette) {
		freeMem(surf->mainPic.palette);
		surf->mainPic.palette = nullptr;
	}

	if (surf->mainPic.pixelData) {
		freeMem(surf->mainPic.pixelData);
		surf->mainPic.pixelData = nullptr;
	}
}

bool XpLib::setDisplaySpec(int *outMode, DisplaySpecs *spec) {
	*outMode = spec->id;
	g_surfaceWidth = spec->width;
	g_surfaceHeight = spec->height;

	if (!createSurface(&g_surfaces[stFront]))
		return false;

	if (!createSurface(&g_surfaces[stBack]))
		return false;

	return true;
}

void XpLib::setCoordSpec(int16 x, int16 y, int16 width, int16 height) {
	if (width == g_surfaceWidth && height == g_surfaceHeight) {
		g_viewportOffsetX = x;
		g_viewportOffsetY = y;
	}
}

void XpLib::virtualToScreen(int16 *x, int16 *y) {
	*x -= g_viewportOffsetX;
	*y -= g_viewportOffsetY;
}

void XpLib::screenToVirtual(int16 *x, int16 *y) {
	*x += g_viewportOffsetX;
	*y += g_viewportOffsetY;
}

void XpLib::displayPic(XPPicDesc *pic, int16 x, int16 y, int16 page) {
	XPSurface *surf = &g_surfaces[page];

	virtualToScreen(&x, &y);

	// --- Pixel data ---
	if (pic->pixelData) {
		bool isFullScreen = (pic->width == g_surfaceWidth &&  pic->height == g_surfaceHeight);

		if (isFullScreen)
			g_renderFlags |= RF_FULL_REDRAW; // Mark full-screen overwrite

		// Full-screen transparent non-RLE overlayPic on surface 0: store as overlay source...
		if (isFullScreen && page == stFront && (pic->flags & 1) && !(pic->flags & 2)) {
			surf->overlayPic.pixelData = pic->pixelData;
			surf->overlayPic.width = pic->width;
			surf->overlayPic.height = pic->height;
			surf->overlayPic.palette = pic->palette;
			surf->overlayPic.paletteStart = pic->paletteStart;
			surf->overlayPic.paletteCount = pic->paletteCount;
			surf->overlayPic.flags = pic->flags;

			g_renderFlags |= RF_OVERLAY_ACTIVE;
		} else {
			// Clear previous overlay if needed...
			if ((g_renderFlags & RF_OVERLAY_ACTIVE) && page == stFront) {
				if (!isFullScreen)
					clipAndBlit(&surf->overlayPic, &surf->mainPic, 0, 0, nullptr);

				g_renderFlags &= ~RF_OVERLAY_ACTIVE;
			}

			// Clip and blit to surface...
			Common::Rect clipResult;
			if (clipAndBlit(pic, &surf->mainPic, x, y, &clipResult))
				addDirtyRect(&clipResult);
		}

		// Mark page as dirty...
		g_renderFlags |= (page != stFront) ? RF_BACK_DIRTY : RF_FRONT_DIRTY;
	}

	// --- Palette data ---
	if (pic->palette) {
		int16 palStart = pic->paletteStart;
		int16 palEnd = palStart + pic->paletteCount - 1;
		int16 adjStart = palStart;
		int16 adjCount = pic->paletteCount;
		byte *palData = pic->palette;

		// Skip index 0...
		if (adjStart == 0) {
			adjStart = 1;
			palData += 3;
		}

		// Clamp to page range (indices 1-127)...
		if (adjStart > 127)
			adjStart = 127;

		if (palEnd > 127)
			palEnd = 127;

		adjCount = palEnd - adjStart + 1;

		if (adjCount > 0) {
			// Copy palette to surface's palette buffer...
			int16 destOffset = (adjStart - surf->mainPic.paletteStart) * 3;
			memcpy(surf->mainPic.palette + destOffset, palData, adjCount * 3);
		}

		int16 dirtyFlag = (page != stFront) ? RF_BACK_PAL_DIRTY : RF_FRONT_PAL_DIRTY;

		if (g_renderFlags & dirtyFlag) {
			// Merge dirty range...
			if (surf->dirtyPalStart > adjStart)
				surf->dirtyPalStart = adjStart;

			if (surf->dirtyPalEnd < palEnd)
				surf->dirtyPalEnd = palEnd;
		} else {
			// First palette update, set range...
			surf->dirtyPalStart = adjStart;
			surf->dirtyPalEnd = palEnd;
			g_renderFlags |= dirtyFlag;
		}
	}
}

void XpLib::dispatchBlit(int16 mode, byte *src, uint16 srcStride, byte *dst, uint16 dstStride, uint16 width, uint16 height) {
	switch (mode) {
	case 0:
		blit(src, srcStride, dst, dstStride, width, height);
		break;
	case 1:
		maskBlit(src, srcStride, dst, dstStride, width, height);
		break;
	case 2:
		rleBlit(src, srcStride, dst, dstStride, width, height);
		break;
	case 3:
		rleMaskBlit(src, srcStride, dst, dstStride, width, height);
		break;
	}
}

bool XpLib::clipAndBlit(XPPicDesc *src, XPPicDesc *dest, int16 x, int16 y, Common::Rect *outClip) {
	int16 blitMode = 0;

	if (src->flags & 1)
		blitMode |= 2; // Transparent

	if (src->flags & 2)
		blitMode |= 1; // RLE

	Common::Rect srcRect(x, y, x + src->width, y + src->height);
	Common::Rect destRect(0, 0, dest->width, dest->height);

	Common::Rect clipped;
	clipped = srcRect.findIntersectingRect(destRect);
	if (clipped.isEmpty())
		return false;

	// For transparent blits, check if content actually changed...
	if (src->flags & 1) {
		if (!clipped.equals(srcRect))
			return false;
	}

	if (outClip) {
		outClip->left = clipped.left;
		outClip->top = clipped.top;
		outClip->right = clipped.right;
		outClip->bottom = clipped.bottom;
	}

	int32 srcOff = (clipped.top - srcRect.top) * src->width + (clipped.left - srcRect.left);
	int32 destOff = clipped.top * dest->width + clipped.left;
	int32 clipW = clipped.right - clipped.left;
	int32 clipH = clipped.bottom - clipped.top;

	dispatchBlit(blitMode,
		src->pixelData + srcOff, src->width,
		dest->pixelData + destOff, dest->width,
		clipW, clipH
	);

	return true;
}

void XpLib::addDirtyRect(Common::Rect *rect) {
	if (g_renderFlags & RF_FULL_REDRAW)
		return; // Full-screen overwrite, no need to track...

	if (g_overlayCount >= 30) {
		g_renderFlags |= RF_FULL_REDRAW; // Too many rects, mark full dirty...
		return;
	}

	g_dirtyRects[g_overlayCount].left = rect->left;
	g_dirtyRects[g_overlayCount].right = rect->right;
	g_dirtyRects[g_overlayCount].top = rect->top;
	g_dirtyRects[g_overlayCount].bottom = rect->bottom;

	g_overlayCount++;
}

void XpLib::setFrameRate(int16 fps) {
	g_frameRateFPS = fps;
	g_nextFrameTime = _bolt->_system->getMillis();
}

void XpLib::updateDisplay() {
	if (g_cursorHidden == 0)
		g_renderFlags |= RF_CURSOR_VISIBLE;
	else
		g_renderFlags &= ~RF_CURSOR_VISIBLE;

	// This will call delayMillis (but must not call pollEvent, otherwise events get desynced)...
	waitForFrameRate();

	handlePaletteTransitions();

	// If back surface pixels changed, mark all with cursor bit and reset row flags...
	if (g_renderFlags & RF_BACK_DIRTY) {
		markCursorPixels(g_surfaces[stBack].mainPic.pixelData, (uint32)g_surfaces[stBack].mainPic.width * (uint32)g_surfaces[stBack].mainPic.height);
		if (_bolt->g_extendedViewport) {
			memset(g_rowDirtyFlags, 0, EXTENDED_SCREEN_HEIGHT);
		} else {
			memset(g_rowDirtyFlags, 0, SCREEN_HEIGHT);
		}
	}

	// Render to screen!
	if (g_renderFlags & RF_OVERLAY_ACTIVE) {
		overlayComposite(); // RLE overlay path
	} else {
		compositeToScreen(); // Standard dirty-rect path
	}

	// Save current state, clear per-frame flags...
	g_prevRenderFlags = g_renderFlags;
	g_renderFlags &= (RF_OVERLAY_ACTIVE | RF_DOUBLE_BUFFER); // Keep only overlay + double-buffer bits...

	_bolt->_system->copyRectToScreen(g_vgaFramebuffer, g_virtualWidth, 0, 0, g_virtualWidth, g_virtualHeight);
	_bolt->_system->updateScreen();
}

void XpLib::waitForFrameRate() {
	if (g_frameRateFPS == 0)
		return;

	while (_bolt->_system->getMillis() < g_nextFrameTime) {
		_bolt->_system->delayMillis(16);
	}

	g_nextFrameTime = (uint32)(_bolt->_system->getMillis() + 1000 / (uint32)g_frameRateFPS);
}

void XpLib::handlePaletteTransitions() {
	// Only act if palette dirty flags are set alongside pixel dirty flags...
	if (!((g_renderFlags & RF_FRONT_PAL_DIRTY) && (g_renderFlags & RF_FRONT_DIRTY)) &&
		!((g_renderFlags & RF_BACK_PAL_DIRTY)  && (g_renderFlags & RF_BACK_DIRTY)))
		return;

	// Build transition code: bit 0 = entering double-buffer, bit 1 = was double-buffer
	int16 transition = 0;
	if (g_renderFlags & RF_DOUBLE_BUFFER)
		transition |= 1;
	if (g_prevRenderFlags & RF_DOUBLE_BUFFER)
		transition |= 2;

	switch (transition) {
	case 0:
		// Single -> Single
		applyCursorPalette();
		break;

	case 1: {
		// Single -> Double
		if (!(g_renderFlags & RF_BACK_PAL_DIRTY))
			break;

		// Check if back surface palette actually differs from VGA palette...
		XPSurface *back = &g_surfaces[stBack];
		int16 count = back->dirtyPalEnd - back->dirtyPalStart + 1;
		int16 surfOffset = (back->dirtyPalStart - back->mainPic.paletteStart) * 3;

		if (memcmp(&g_paletteBuffer[back->dirtyPalStart * 3],
				   back->mainPic.palette + surfOffset,
				   count * 3) != 0)
			break;

		applyCursorPalette();
		break;
	}
	
	case 2: {
		// Double -> Single
		if (!(g_renderFlags & RF_FRONT_PAL_DIRTY))
			break;

		// Check if front surface palette differs from cursor palette page...
		XPSurface *front = &g_surfaces[stFront];
		int16 count = front->dirtyPalEnd - front->dirtyPalStart + 1;
		int16 surfOffset = (front->dirtyPalStart - front->mainPic.paletteStart) * 3;

		if (memcmp(&g_paletteBuffer[(front->dirtyPalStart + 128) * 3],
				   front->mainPic.palette + surfOffset,
				   count * 3) != 0)
			break;

		prepareBackSurface();
		break;
	}

	case 3:
		// Double -> Double: no special handling
		break;
	}
}

void XpLib::flushPalette() {
	// Front surface palette -> VGA indices 1-127
	if (g_renderFlags & RF_FRONT_PAL_DIRTY) {
		XPSurface *front = &g_surfaces[stFront];
		int16 start = front->dirtyPalStart;
		int16 count = front->dirtyPalEnd - start + 1;
		int16 offset = (start - front->mainPic.paletteStart) * 3;
		setPalette(count, start, front->mainPic.palette + offset);
	}

	// Back surface palette -> VGA indices 129-255 (start + 128)
	if (g_renderFlags & RF_BACK_PAL_DIRTY) {
		XPSurface *back = &g_surfaces[stBack];
		int16 start = back->dirtyPalStart;
		int16 count = back->dirtyPalEnd - start + 1;
		int16 offset = (start - back->mainPic.paletteStart) * 3;
		setPalette(count, start + 128, back->mainPic.palette + offset);
	}
}

void XpLib::overlayComposite() {
	bool pixelsDirty = (g_renderFlags & (RF_FRONT_DIRTY | RF_BACK_DIRTY)) != 0;
	bool cursorNow = (g_renderFlags & RF_CURSOR_VISIBLE) != 0;
	bool cursorPrev = (g_prevRenderFlags & RF_CURSOR_VISIBLE) != 0;

	// Nothing changed at all, just flush the palette...
	if (!pixelsDirty && !cursorNow && !cursorPrev) {
		flushPalette();
		return;
	}

	int16 cursorX = 0, cursorY = 0;

	// Decode RLE overlay onto front surface...
	if (pixelsDirty) {
		if (g_renderFlags & RF_DOUBLE_BUFFER) {
			// Double-buffer: RLE composite (transparent pixels from back surface)
			rleCompositeBlit(
				g_surfaces[stFront].overlayPic.pixelData,
				g_surfaces[stBack].mainPic.pixelData,
				g_surfaces[stFront].mainPic.pixelData,
				g_virtualWidth, g_virtualHeight,
				g_rowDirtyFlags);
		} else {
			// Single-buffer: straight RLE decode to front
			rleBlit(
				g_surfaces[stFront].overlayPic.pixelData,
				g_surfaces[stFront].overlayPic.width,
				g_surfaces[stFront].mainPic.pixelData,
				g_surfaces[stFront].mainPic.width,
				g_virtualWidth, g_virtualHeight);
		}
	}

	// Draw cursor onto front surface...
	if (cursorNow) {
		cursorX = g_lastCursorX - g_cursorHotspotX;
		cursorY = g_lastCursorY - g_cursorHotspotY;

		// Save background under cursor...
		clipAndBlit(&g_surfaces[stFront].mainPic, &g_cursorBackgroundSave,
					  -cursorX, -cursorY, nullptr);

		// Draw cursor sprite...
		clipAndBlit(&g_cursorSprite, &g_surfaces[stFront].mainPic,
					  cursorX, cursorY, &g_overlayCursorRect);
	}

	flushPalette();

	// Transfer front surface to main framebuffer...
	if (pixelsDirty) {
		if (g_renderFlags & RF_DOUBLE_BUFFER) {
			// Tracked blit: only copy rows that changed...
			dirtyBlit(
				g_surfaces[stFront].mainPic.pixelData,
				g_vgaFramebuffer,
				g_virtualWidth, g_virtualHeight,
				g_rowDirtyFlags);
		} else {
			memcpy(g_vgaFramebuffer, g_surfaces[stFront].mainPic.pixelData,
				   (uint32)g_virtualWidth * (uint32)g_virtualHeight);
		}
	} else {
		if (cursorNow)
			compositeDirtyRects(&g_overlayCursorRect, 1);

		if (cursorPrev)
			compositeDirtyRects(&g_prevOverlayCursorRect, 1);
	}

	// Save cursor rect and restore front surface
	if (cursorNow) {
		g_prevOverlayCursorRect = g_overlayCursorRect;

		clipAndBlit(&g_cursorBackgroundSave, &g_surfaces[stFront].mainPic, cursorX, cursorY, nullptr);
	}
}

void XpLib::compositeToScreen() {
	int16 cursorX = 0, cursorY = 0;

	// Draw cursor onto front surface...
	if (g_renderFlags & RF_CURSOR_VISIBLE) {
		cursorX = g_lastCursorX - g_cursorHotspotX;
		cursorY = g_lastCursorY - g_cursorHotspotY;

		// Save background under cursor position...
		clipAndBlit(&g_cursorBackgroundSave, &g_surfaces[stFront].mainPic, -cursorX, -cursorY, nullptr);

		// Draw cursor sprite at position...
		clipAndBlit(&g_cursorSprite, &g_surfaces[stFront].mainPic, cursorX, cursorY, &g_cursorRect);
	}

	flushPalette();

	if (!(g_renderFlags & RF_DOUBLE_BUFFER)) {
		// SINGLE-BUFFER MODE

		if (g_renderFlags & RF_FRONT_DIRTY) {
			if (g_renderFlags & RF_FULL_REDRAW) {
				// Full-screen dirty, blit entire front surface to framebuffer...
				blit(g_surfaces[stFront].mainPic.pixelData, g_surfaces[stFront].mainPic.width,
					   g_vgaFramebuffer, g_virtualWidth,
					   g_virtualWidth, g_virtualHeight);
			} else {
				// Partial dirty, blit changed rects only...
				if (g_prevDirtyValid > 0) {
					mergeDirtyRects();
					blitDirtyRects(g_prevDirtyRects, g_prevDirtyCount);
				}

				blitDirtyRects(g_dirtyRects, g_overlayCount);

				if (g_renderFlags & RF_CURSOR_VISIBLE)
					blitDirtyRects(&g_cursorRect, 1);

				if (g_prevRenderFlags & RF_CURSOR_VISIBLE)
					blitDirtyRects(&g_prevCursorRect, 1);
			}
		} else {
			// No pixel changes - just cursor
			if (g_renderFlags & RF_CURSOR_VISIBLE)
				blitDirtyRects(&g_cursorRect, 1);

			if (g_prevRenderFlags & RF_CURSOR_VISIBLE)
				blitDirtyRects(&g_prevCursorRect, 1);
		}

	} else {
		// DOUBLE-BUFFER MODE

		if (g_renderFlags & (RF_FRONT_DIRTY | RF_BACK_DIRTY)) {
			if ((g_renderFlags & RF_FULL_REDRAW) ||
				g_prevDirtyValid > 0 || g_overlayCount > 0) {
				// Full composite...
				compositeBlit(g_surfaces[stFront].mainPic.pixelData, g_surfaces[stBack].mainPic.pixelData,
							  g_vgaFramebuffer, g_virtualWidth, g_virtualWidth, g_virtualHeight);
				g_overlayCount = 0;
				g_prevDirtyCount = 0;
			} else {
				// Partial composite...
				if (g_prevDirtyValid > 0) {
					mergeDirtyRects();
					compositeDirtyRects(g_prevDirtyRects, g_prevDirtyCount);
				}

				compositeDirtyRects(g_dirtyRects, g_overlayCount);

				if (g_renderFlags & RF_CURSOR_VISIBLE)
					compositeDirtyRects(&g_cursorRect, 1);

				if (g_prevRenderFlags & RF_CURSOR_VISIBLE)
					compositeDirtyRects(&g_prevCursorRect, 1);
			}
		} else {
			if (g_renderFlags & RF_CURSOR_VISIBLE)
				compositeDirtyRects(&g_cursorRect, 1);

			if (g_prevRenderFlags & RF_CURSOR_VISIBLE)
				compositeDirtyRects(&g_prevCursorRect, 1);
		}
	}

	// Save current dirty rects for next frame...
	if (g_prevDirtyValid > 0) {
		g_prevDirtyValid = 0;
		if (g_overlayCount > 0) {
			for (int i = 0; i < g_overlayCount; i++) {
				g_prevDirtyRects[i] = g_dirtyRects[i];
			}
		}

		g_prevDirtyCount = g_overlayCount;
	}

	g_overlayCount = 0;

	// Save cursor state for next frame...
	if (g_renderFlags & RF_CURSOR_VISIBLE) {
		g_prevCursorRect = g_cursorRect;

		// Restore front surface under cursor...
		clipAndBlit(&g_cursorBackgroundSave, &g_surfaces[stFront].mainPic, cursorX, cursorY, nullptr);
	}
}

void XpLib::mergeDirtyRects() {
	for (int16 i = 0; i < g_prevDirtyCount; i++) {
		Common::Rect *prev = &g_prevDirtyRects[i];

		for (int16 j = 0; j < g_overlayCount; j++) {
			Common::Rect *curr = &g_dirtyRects[j];

			if (curr->right < 0)
				continue; // A negative value indicates "already consumed"...

			if (!prev->intersects(*curr))
				continue;

			prev->extend(*curr);
			curr->right *= -1; // Negate to mark consumed...
			break;
		}
	}
}

void XpLib::blitDirtyRects(Common::Rect *rects, int16 count) {
	for (int16 i = 0; i < count; i++) {
		Common::Rect *rect = &rects[i];

		if (rect->right < 0) {
			rect->right = -rect->right;
		} else {
			uint32 offset = rect->top * g_virtualWidth + rect->left;
			int32 w = rect->right - rect->left;
			int32 h = rect->bottom - rect->top;

			blit(g_surfaces[stFront].mainPic.pixelData + offset, g_surfaces[stFront].mainPic.width,
				 g_vgaFramebuffer + offset, g_virtualWidth,
				 w, h);
		}
	}
}

void XpLib::compositeDirtyRects(Common::Rect *rects, int16 count) {
	for (int16 i = 0; i < count; i++) {
		Common::Rect *rect = &rects[i];

		if (rect->right < 0) {
			rect->right = -rect->right;
		} else {
			uint32 offset = rect->top * g_virtualWidth + rect->left;
			int32 w = rect->right - rect->left;
			int32 h = rect->bottom - rect->top;

			compositeBlit(
				g_surfaces[stFront].mainPic.pixelData + offset,
				g_surfaces[stBack].mainPic.pixelData + offset,
				g_vgaFramebuffer + offset,
				g_virtualWidth, w, h);
		}
	}
}

void XpLib::applyCursorPalette() {
	setPalette(127, 129, &g_paletteBuffer[3]);
	markCursorPixels(g_vgaFramebuffer, (uint32)g_virtualWidth * (uint32)g_virtualHeight);
}

void XpLib::prepareBackSurface() {
	XPPicDesc *src;
	if (g_renderFlags & RF_OVERLAY_ACTIVE) {
		src = &g_surfaces[stFront].overlayPic;
	} else {
		src = &g_surfaces[stFront].mainPic;
	}

	clipAndBlit(src, &g_surfaces[stBack].mainPic, 0, 0, nullptr);

	markCursorPixels(g_surfaces[stBack].mainPic.pixelData, (uint32)g_surfaces[stBack].mainPic.width * (uint32)g_surfaces[stBack].mainPic.height);

	memcpy(g_vgaFramebuffer, g_surfaces[stBack].mainPic.pixelData, (uint32)g_surfaces[stBack].mainPic.width * (uint32)g_surfaces[stBack].mainPic.height);
}

void XpLib::setTransparency(bool toggle) {
	if (toggle) {
		g_renderFlags |= RF_DOUBLE_BUFFER; // Enables double-buffer compositing
	} else {
		g_renderFlags &= ~RF_DOUBLE_BUFFER;
	}
}

void XpLib::fillDisplay(byte color, int16 page) {
	XPSurface *surf = &g_surfaces[page];
	uint32 size = (uint32)surf->mainPic.width * (uint32)surf->mainPic.height;

	// Back surface pixels have bit 7 set for cursor palette mapping...
	byte fillVal = (page == stBack) ? (color + 0x80) : color;

	memset(surf->mainPic.pixelData, fillVal, size);

	// Mark page as pixel-dirty...
	g_renderFlags |= (page != stFront) ? RF_BACK_DIRTY : RF_FRONT_DIRTY;

	// Clear overlay flag, signal full redraw needed...
	g_renderFlags &= ~RF_OVERLAY_ACTIVE;
	g_prevDirtyValid = 1;
}

} // End of namespace Bolt
