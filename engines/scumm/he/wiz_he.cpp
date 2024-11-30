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

#ifdef ENABLE_HE

#include "common/archive.h"
#include "common/ptr.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/primitives.h"
#include "scumm/he/logic_he.h"
#include "scumm/he/intern_he.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/util.h"
#include "scumm/he/wiz_he.h"
#include "scumm/he/moonbase/moonbase.h"

namespace Scumm {

#ifdef WIZ_DEBUG_BUFFERS
Common::Array<DbgEntry> *WizPxShrdBuffer::_allocLocs = 0;
#endif

Wiz::Wiz(ScummEngine_v71he *vm) : _vm(vm) {
	_wizBufferIndex = 0;
	memset(&_wizBuffer, 0, sizeof(_wizBuffer));
	memset(&_polygons, 0, sizeof(_polygons));
	_useWizClipRect = false;
	_uses16BitColor = (_vm->_game.features & GF_16BIT_COLOR);
}

void Wiz::clearWizBuffer() {
	_wizBufferIndex = 0;
}

void Wiz::processWizImageCaptureCmd(const WizImageCommand *params) {
	bool compressIt = (params->compressionType == kWCTTRLE);
	bool background = (params->flags & kWRFBackground) != 0;

	takeAWiz(params->image, params->box.left, params->box.top, params->box.right, params->box.bottom, background, compressIt);

	_vm->_res->setModified(rtImage, params->image);
}

void Wiz::takeAWiz(int globnum, int x1, int y1, int x2, int y2, bool back, bool compress) {
	int bufferWidth, bufferHeight;
	Common::Rect rect, clipRect;
	WizPxShrdBuffer srcPtr;

	VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
	bufferWidth = pvs->w;
	bufferHeight = pvs->h;

	if (back) {
		srcPtr = WizPxShrdBuffer(pvs->getBackPixels(0, 0), false);
	} else {
		srcPtr = WizPxShrdBuffer(pvs->getPixels(0, 0), false);
	}

	rect.left = x1;
	rect.top = y1;
	rect.right = x2;
	rect.bottom = y2;

	clipRect.left = 0;
	clipRect.top = 0;
	clipRect.right = bufferWidth - 1;
	clipRect.bottom = bufferHeight - 1;

	if (!findRectOverlap(&rect, &clipRect)) {
		error("Capture rect invalid (%-4d,%4d,%-4d,%4d)", x1, y1, x2, y2);
	}

	uint8 *palPtr = nullptr;
	if (_vm->_game.heversion >= 99) {
		palPtr = _vm->_hePalettes + _vm->_hePaletteSlot;
	} else {
		palPtr = _vm->_currentPalette;
	}

	buildAWiz(
		srcPtr, bufferWidth, bufferHeight,
		palPtr, &rect,
		(compress) ? kWCTTRLE : kWCTNone,
		globnum, _vm->_game.heversion <= 90 ? 0x05 : _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
}

void Wiz::simpleDrawAWiz(int image, int state, int x, int y, int flags) {
	if (!_vm->_fullRedraw) {
		drawAWiz(image, state, x, y, 0, flags, 0, 0, nullptr, 0, nullptr);
	} else {
		bufferAWiz(image, state, x, y, 0, flags, 0, 0, 0);
	}
}

void Wiz::bufferAWiz(int image, int state, int x, int y, int z, int flags, int optionalShadowImage, int optionalZBufferImage, int whichPalette) {
	assert(_wizBufferIndex < ARRAYSIZE(_wizBuffer));
	WizBufferElement *wi = &_wizBuffer[_wizBufferIndex];
	wi->image = image;
	wi->x = x;
	wi->y = y;
	wi->z = z;
	wi->state = state;
	wi->flags = flags;
	wi->shadow = optionalShadowImage;
	wi->zbuffer = optionalZBufferImage;
	wi->palette = whichPalette;
	++_wizBufferIndex;
}

WizPxShrdBuffer Wiz::drawAWiz(int image, int state, int x, int y, int z, int flags, int optionalShadowImage, int optionalZBufferImage, Common::Rect *optionalClipRect, int whichPalette, WizSimpleBitmap *optionalBitmapOverride) {
	return drawAWizEx(image, state, x, y, z, flags,
		optionalShadowImage, optionalZBufferImage, optionalClipRect,
		whichPalette, optionalBitmapOverride, nullptr);
}

WizPxShrdBuffer Wiz::drawAWizEx(int image, int state, int x, int y, int z, int flags, int optionalShadowImage, int optionalZBufferImage, Common::Rect *optionalClipRect, int whichPalette, WizSimpleBitmap *optionalBitmapOverride, const WizImageCommand *optionalICmdPtr) {
	const WizRawPixel *colorConversionTable;
	Common::Rect *clipRectPtr;

	if (whichPalette) {
		colorConversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(whichPalette);
	} else {
		colorConversionTable = nullptr;
	}

	// Get the shadow...
	if (!optionalShadowImage) {
		if (_wizActiveShadow && (flags & kWRFUseShadow)) {
			optionalShadowImage = _wizActiveShadow;
		}
	}

	if (!(flags & kWRFPolygon)) {
		// Get the clipping rect if any...
		if (!optionalClipRect) {
			if (_useWizClipRect && !(flags & (kWRFPrint | kWRFAlloc))) {
				clipRectPtr = &_wizClipRect;
			} else {
				clipRectPtr = nullptr;
			}
		} else {
			clipRectPtr = optionalClipRect;
		}

		// Call the primitive renderer.
		return drawAWizPrimEx(image, state, x, y, z,
			optionalShadowImage, optionalZBufferImage, clipRectPtr,
			flags, optionalBitmapOverride, colorConversionTable, optionalICmdPtr);
	} else {
		warpDrawWiz(
			image, state, x, flags, _vm->_game.heversion <= 90 ? 0x05 : _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
			optionalBitmapOverride, colorConversionTable, optionalShadowImage);

		return WizPxShrdBuffer();
	}
}

WizPxShrdBuffer Wiz::drawAWizPrim(int globNum, int state, int x, int y, int z, int shadowImage, int zbufferImage, const Common::Rect *optionalClipRect, int flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	return drawAWizPrimEx(globNum, state, x, y, z,
		shadowImage, zbufferImage, optionalClipRect, flags,
		optionalBitmapOverride, optionalColorConversionTable, 0);
}

WizPxShrdBuffer Wiz::drawAWizPrimEx(int globNum, int state, int x, int y, int z, int shadowImage, int zbufferImage, const Common::Rect *optionalClipRect, int flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable, const WizImageCommand *optionalICmdPtr) {
	int destWidth, destHeight, srcWidth, srcHeight, srcComp, remapId;
	byte *srcData, *srcPtr, *stateHeader, *remapPtr;
	const byte *shadowPtr;
	Common::Rect destRect, clipRect;
	bool markUpdates;
	WizPxShrdBuffer destPtr;

	markUpdates = true;
	remapPtr = nullptr;

	if (_vm->_game.heversion > 98) {
		// Set the optional remap table up to the default if one isn't specified
		if (!optionalColorConversionTable) {
			optionalColorConversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(1);
		}
	}

	if (shadowImage) {
		shadowPtr = getColorMixBlockPtrForWiz(shadowImage);

		if (shadowPtr) {
			shadowPtr += _vm->_resourceHeaderSize;
		}
	} else {
		shadowPtr = nullptr;
	}

	if (_vm->_game.heversion < 99 && _uses16BitColor) {
		if (shadowPtr) {
			shadowPtr = nullptr;
		}
	}

	if (_vm->_game.heversion > 99 || _vm->_isHE995) {
		// Z-Buffer image... (only format supported: uncompressed 16-bit)
		if (zbufferImage) {
			byte *pzbHeader = getWizStateHeaderPrim(zbufferImage, 0);

			assert(pzbHeader);

			int zbComp = READ_LE_UINT32(pzbHeader + _vm->_resourceHeaderSize);

			if (zbComp != kWCTNone16Bpp) {
				error("Wiz::drawAWizPrimEx(): 16-bit uncompressed z-buffers are the only currently supported format");
			}
		}
	}

	// Get the header for this state...
	stateHeader = getWizStateHeaderPrim(globNum, state);
	if (!stateHeader)
		error("Wiz::drawAWizPrimEx(): %d state %d missing header", globNum, state);

	srcComp = READ_LE_UINT32(stateHeader + _vm->_resourceHeaderSize + 0);
	srcWidth = READ_LE_UINT32(stateHeader + _vm->_resourceHeaderSize + 4);
	srcHeight = READ_LE_UINT32(stateHeader + _vm->_resourceHeaderSize + 8);

	if ((!isUncompressedFormatTypeID(srcComp)) && (srcComp != kWCTTRLE)) {
		error("Wiz::drawAWizPrimEx(): %d has invalid compression type %d", globNum, srcComp);
	}

	// Get the data block for this state...
	srcData = getWizStateDataPrim(globNum, state);
	if (!srcData)
		error("Wiz::drawAWizPrimEx(): %d state %d missing data block", globNum, state);

	// Copy the palette from this state...
	if (flags & kWRFUsePalette) {
		srcPtr = getWizStatePaletteDataPrim(globNum, state);
		if (!srcPtr)
			error("Wiz::drawAWizPrimEx(): %d state %d missing palette block", globNum, state);

		_vm->setPaletteFromPtr(srcPtr + _vm->_resourceHeaderSize, 256);
	}

	// Remap this wiz state...
	if (flags & kWRFRemap) {
		remapPtr = getWizStateRemapDataPrim(globNum, state);
		if (!remapPtr)
			error("Wiz::drawAWizPrimEx(): %d state %d is missing a remap palette block", globNum, state);

		remapId = READ_BE_UINT32(remapPtr + _vm->_resourceHeaderSize);

		if (_vm->_game.heversion <= 80 || remapId != WIZ_MAGIC_REMAP_NUMBER) {
			if (remapId != _vm->_paletteChangedCounter) {
				WRITE_LE_UINT32(remapPtr + _vm->_resourceHeaderSize, _vm->_paletteChangedCounter);

				srcPtr = getWizStatePaletteDataPrim(globNum, state);
				if (!srcPtr)
					error("Wiz::drawAWizPrimEx(): %d state %d missing palette block", globNum, state);

				const byte *palPtr = nullptr;
				if (_vm->_game.heversion >= 99) {
					palPtr = _vm->_hePalettes + _vm->_hePaletteSlot;
				} else {
					palPtr = _vm->_currentPalette;
				}

				_vm->buildRemapTable(remapPtr + _vm->_resourceHeaderSize + 4, palPtr, srcPtr + _vm->_resourceHeaderSize);
			}
		}
	}

	// Get the dest pointer...
	if (flags & (kWRFPrint | kWRFAlloc)) {
		destWidth = srcWidth;
		destHeight = srcHeight;
		if (_uses16BitColor) {
			destPtr = WizPxShrdBufferD(malloc(destWidth * destHeight * sizeof(WizRawPixel16)), true);
		} else {
			destPtr = WizPxShrdBufferD(malloc(destWidth * destHeight * sizeof(WizRawPixel8)), true);
		}

		if (!destPtr()) {
			warning("Wiz::drawAWizPrimEx(): Not enough memory for image operation (print / other)");
			return WizPxShrdBuffer();
		} else if (flags & kWRFAlloc) {
			memset8BppConversion(
				destPtr(),
				_vm->_game.heversion < 95 ? 0x05 : _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
				destWidth * destHeight,
				optionalColorConversionTable);
		}

	} else {
		if (optionalBitmapOverride) {
			destWidth = optionalBitmapOverride->bitmapWidth;
			destHeight = optionalBitmapOverride->bitmapHeight;
			destPtr = optionalBitmapOverride->bufferPtr;

			markUpdates = false;
		} else {
			VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
			destWidth = pvs->w;
			destHeight = pvs->h;

			if (flags & kWRFForeground) {
				destPtr = WizPxShrdBuffer(pvs->getPixels(0, pvs->topline), false);
			} else {
				destPtr = WizPxShrdBuffer(pvs->getBackPixels(0, pvs->topline), false);
			}
		}
	}

	// Make the clipping rect equal to the limits of the draw buffer...
	clipRect.left = 0;
	clipRect.top = 0;
	clipRect.right = destWidth - 1;
	clipRect.bottom = destHeight - 1;

	if (optionalClipRect && (!(flags & (kWRFPrint | kWRFAlloc)))) {
		if (!findRectOverlap(&clipRect, optionalClipRect)) {
			return WizPxShrdBuffer();
		}
	}

	// Decompress the image...
	if ((_vm->_game.heversion > 99 || _vm->_isHE995) && zbufferImage) {
		WizSimpleBitmap sbZBuffer;
		sbZBuffer.bitmapHeight = 0;
		sbZBuffer.bitmapWidth = 0;
		sbZBuffer.bufferPtr = WizPxShrdBuffer();

		dwSetSimpleBitmapStructFromImage(zbufferImage, 0, &sbZBuffer);

		// Validate destination for z-buffer...
		if ((destWidth != sbZBuffer.bitmapWidth) ||
			(destHeight != sbZBuffer.bitmapHeight)) {
			error("Wiz::drawAWizPrimEx(): destination size must match z-buffer size d:%dx%d  z:%dx%d",
					   destWidth, destHeight, sbZBuffer.bitmapWidth, sbZBuffer.bitmapHeight);
		}

		WizSimpleBitmap sbDst;

		sbDst.bufferPtr = destPtr;
		sbDst.bitmapWidth = destWidth;
		sbDst.bitmapHeight = destHeight;

		if (srcComp != kWCTTRLE) {
			pgDrawImageWith16BitZBuffer(&sbDst, &sbZBuffer, srcData + _vm->_resourceHeaderSize, x, y, z, srcWidth, srcHeight, &clipRect);
		}
	} else if (srcComp == kWCTTRLE) {
		if (flags & kWRFZPlaneOn) {
			if (_vm->_game.heversion > 95 && _vm->_gdi->_numZBuffer <= 1) {
				error("Wiz::drawAWizPrimEx(): No zplane %d (limit 0 to %d)", 1, (_vm->_gdi->_numZBuffer - 1));
			}

			auxDrawZplaneFromTRLEImage(_vm->getMaskBuffer(0, 0, 1), srcData + _vm->_resourceHeaderSize, destWidth, destHeight, x, y, srcWidth, srcHeight, &clipRect, kWZOIgnore, kWZOSet);
		} else if (flags & kWRFZPlaneOff) {
			if (_vm->_game.heversion > 95 && _vm->_gdi->_numZBuffer <= 1) {
				error("Wiz::drawAWizPrimEx(): No zplane %d (limit 0 to %d)", 1, (_vm->_gdi->_numZBuffer - 1));
			}

			auxDrawZplaneFromTRLEImage(_vm->getMaskBuffer(0, 0, 1), srcData + _vm->_resourceHeaderSize, destWidth, destHeight, x, y, srcWidth, srcHeight, &clipRect, kWZOIgnore, kWZOClear);
		} else if (_vm->_game.heversion <= 98 && !(flags & (kWRFHFlip | kWRFVFlip))) {
			if (flags & kWRFRemap) {
				auxDecompRemappedTRLEImage(
					destPtr(), srcData + _vm->_resourceHeaderSize, destWidth, destHeight,
					x, y, srcWidth, srcHeight, &clipRect, remapPtr + _vm->_resourceHeaderSize + 4,
					optionalColorConversionTable);
			} else if (!shadowPtr) {
				auxDecompTRLEImage(
					destPtr(), srcData + _vm->_resourceHeaderSize, destWidth, destHeight,
					x, y, srcWidth, srcHeight, &clipRect,
					optionalColorConversionTable);
			} else {
				auxDecompMixColorsTRLEImage(
					destPtr(), srcData + _vm->_resourceHeaderSize, destWidth, destHeight,
					x, y, srcWidth, srcHeight, &clipRect, shadowPtr,
					optionalColorConversionTable);
			}
		} else {
			const byte *dataPtr = nullptr;

			if (shadowPtr)
				dataPtr = shadowPtr;

			if (flags & kWRFRemap)
				dataPtr = remapPtr + _vm->_resourceHeaderSize + 4;

			trleFLIPDecompressImage(
				destPtr(), srcData + _vm->_resourceHeaderSize, destWidth, destHeight,
				x, y, srcWidth, srcHeight, &clipRect, flags, dataPtr,
				optionalColorConversionTable,
				optionalICmdPtr);
		}

	} else {
		int transColorOverride;
		const byte *dataPtr = nullptr;

		if (shadowPtr)
			dataPtr = shadowPtr;

		if (flags & kWRFRemap)
			dataPtr = remapPtr + _vm->_resourceHeaderSize + 4;

		if (_vm->_game.heversion >= 95 && doesRawWizStateHaveTransparency(globNum, state)) {
			transColorOverride = _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR);
		} else {
			transColorOverride = -1;
		}

		if (_uses16BitColor && srcComp != kWCTNone16Bpp && srcComp != kWCTNone16BppBigEndian) {
				if (srcComp == kWCTNone) {
					pgDraw8BppFormatImage(
						destPtr(), (byte *)(srcData + _vm->_resourceHeaderSize), destWidth, destHeight,
						x, y, srcWidth, srcHeight, &clipRect, flags, dataPtr, transColorOverride,
						optionalColorConversionTable);
				} else {
					error("Wiz::drawAWizPrimEx(): Raw data type mismatch for mode %d vs %d", srcComp, kWCTNone16Bpp);
				}
		} else {
			if (_vm->_game.heversion > 99 || _vm->_isHE995) {
				if (optionalColorConversionTable &&
					((WizRawPixel *)_vm->getHEPaletteSlot(1) != optionalColorConversionTable)) {
					flags |= kWRFRemap;
					dataPtr = (const byte *)optionalColorConversionTable;
				}
			} else {
				if (!_uses16BitColor && srcComp != kWCTNone) {
					error("Wiz::drawAWizPrimEx(): Raw data type mismatch for mode %d vs %d", srcComp, kWCTNone);
				}
			}

			// Use the native transfer function...
			pgDrawRawDataFormatImage(
				destPtr(), (WizRawPixel *)(srcData + _vm->_resourceHeaderSize), destWidth, destHeight,
				x, y, srcWidth, srcHeight, &clipRect, flags, dataPtr, transColorOverride);
		}
	}

	// Verify if it's a printing operation...
	if (flags & kWRFPrint) {
		warning("Wiz::drawAWizPrimEx(): Printing not yet supported");

		if (_vm->_game.heversion <= 99 || (flags & kWRFAlloc) == 0)
			destPtr = WizPxShrdBuffer();

	} else {
		if (!(flags & kWRFAlloc) && markUpdates) {
			destRect.left = x;
			destRect.top = y;
			destRect.right = x + srcWidth - 1;
			destRect.bottom = y + srcHeight - 1;

			if (findRectOverlap(&destRect, &clipRect)) {
				// If neither foreground or background, copy to both
				if ((flags & kWRFBackground) || ((flags & (kWRFBackground | kWRFForeground)) == 0)) {
					_vm->backgroundToForegroundBlit(destRect);
				} else {
					++destRect.bottom;
					_vm->markRectAsDirty(kMainVirtScreen, destRect);
				}
			}
		}
	}

	return destPtr;
}

void Wiz::buildAWiz(const WizPxShrdBuffer &bufPtr, int bufWidth, int bufHeight, const byte *palettePtr, const Common::Rect *rectPtr, int compressionType, int globNum, int transparentColor) {
	int dataSize, globSize, dataOffset, counter, height, width;
	Common::Rect compRect;
	byte *ptr;

	compRect.left = 0;
	compRect.top = 0;
	compRect.right = bufWidth - 1;
	compRect.bottom = bufHeight - 1;

	dataSize = 0;

	if (rectPtr) {
		if (!findRectOverlap(&compRect, rectPtr)) {
			error("Wiz::buildAWiz(): Build wiz incorrect size (%d,%d,%d,%d)", rectPtr->left, rectPtr->top, rectPtr->right, rectPtr->bottom);
		}
	}

	// Force the compression type if in hi-color mode...
	if (_uses16BitColor) {
		compressionType = kWCTNone16Bpp;
	}

	// Estimate the size of the wiz...
	globSize = (_vm->_resourceHeaderSize * 3) + 12; // AWIZ, WIZH + data (12), WIZD

	if (palettePtr) {
		globSize += (_vm->_resourceHeaderSize * 2) + 768 + 256 + 4; // RGBS + 768, RMAP + 256 + 4
	}

	if (compressionType == kWCTTRLE) {
		dataSize = trleCompressImageArea(
			nullptr, bufPtr(), bufWidth, compRect.left, compRect.top, compRect.right, compRect.bottom,
			(WizRawPixel)transparentColor);
	} else if (isUncompressedFormatTypeID(compressionType)) {
		dataSize = ((getRectWidth(&compRect) * getRectHeight(&compRect)) * sizeof(WizRawPixel));
	} else {
		error("Wiz::buildAWiz(): Unknown compression type %d", compressionType);
	}

	if (_vm->_game.heversion > 90) {
		// Make sure that the resource is even sized...
		if (dataSize & 1) {
			dataSize++;
		}
	}

	// Finalize the resource size...
	globSize += dataSize;

	// Finally build the wiz...
	ptr = (byte *)_vm->_res->createResource(rtImage, globNum, globSize);

	dataOffset = 0;

	// AWIZ block
	WRITE_BE_UINT32(ptr + 0, MKTAG('A', 'W', 'I', 'Z'));
	WRITE_BE_UINT32(ptr + 4, globSize); dataOffset += _vm->_resourceHeaderSize;

	// WIZH
	WRITE_BE_UINT32(ptr + dataOffset + 0, MKTAG('W', 'I', 'Z', 'H'));
	WRITE_BE_UINT32(ptr + dataOffset + 4, (12 + _vm->_resourceHeaderSize)); dataOffset += _vm->_resourceHeaderSize;
	WRITE_LE_UINT32(ptr + dataOffset, compressionType); dataOffset += 4; // COMPRESSION-TYPE
	WRITE_LE_UINT32(ptr + dataOffset, getRectWidth(&compRect)); dataOffset += 4; // WIDTH
	WRITE_LE_UINT32(ptr + dataOffset, getRectHeight(&compRect)); dataOffset += 4; // HEIGHT

	if (palettePtr) {
		// RGBS
		WRITE_BE_UINT32(ptr + dataOffset + 0, MKTAG('R', 'G', 'B', 'S'));
		WRITE_BE_UINT32(ptr + dataOffset + 4, (768 + _vm->_resourceHeaderSize)); dataOffset += _vm->_resourceHeaderSize;
		memcpy(ptr + dataOffset, palettePtr, 768); dataOffset += 768;

		// RMAP
		WRITE_BE_UINT32(ptr + dataOffset + 0, MKTAG('R', 'M', 'A', 'P'));
		WRITE_BE_UINT32(ptr + dataOffset + 4, (256 + 4 + _vm->_resourceHeaderSize)); dataOffset += _vm->_resourceHeaderSize;
		WRITE_LE_UINT32(ptr + dataOffset, 0); dataOffset += 4; // Remapped flag

		for (counter = 0; counter < 256; counter++) {
			*(ptr + dataOffset) = counter;
			dataOffset++;
		}
	}

	// WIZD
	WRITE_BE_UINT32(ptr + dataOffset + 0, MKTAG('W', 'I', 'Z', 'D'));
	WRITE_BE_UINT32(ptr + dataOffset + 4, (dataSize + _vm->_resourceHeaderSize)); dataOffset += _vm->_resourceHeaderSize;

	if (compressionType == kWCTTRLE) {
		if (!_uses16BitColor) {
			trleCompressImageArea(
				ptr + dataOffset, bufPtr(), bufWidth,
				compRect.left, compRect.top, compRect.right, compRect.bottom,
				(byte)transparentColor);
		} else {
			error("Wiz::buildAWiz(): Incorrect type %d for current pixel mode 16 bit", compressionType);
		}
	} else {
		WizSimpleBitmap srcBitmap, dstBitmap;
		Common::Rect dstRect;

		// Src setup
		srcBitmap.bufferPtr = bufPtr;
		srcBitmap.bitmapWidth = bufWidth;
		srcBitmap.bitmapHeight = bufHeight;

		// Dst setup
		width = getRectWidth(&compRect);
		height = getRectHeight(&compRect);

		dstBitmap.bufferPtr = WizPxShrdBuffer(ptr + dataOffset, false);
		dstBitmap.bitmapWidth = width;
		dstBitmap.bitmapHeight = height;

		dstRect.left = 0;
		dstRect.top = 0;
		dstRect.right = width - 1;
		dstRect.bottom = height - 1;

		// Call the blit primitive...
		pgSimpleBlit(&dstBitmap, &dstRect, &srcBitmap, &compRect);
	}

	dataOffset += dataSize;
	if (globSize != dataOffset) {
		error("Wiz::buildAWiz(): WIZ size mismatch!");
	}
}

int Wiz::pixelHitTestWiz(int image, int state, int x, int y, int32 flags) {
	return pixelHitTestWizPrim(image, state, x, y, flags);
}

int Wiz::pixelHitTestWizPrim(int globNum, int state, int x, int y, int32 flags) {
	int outValue = 0;

	if (((ScummEngine_v90he *)_vm)->_logicHE && ((ScummEngine_v90he *)_vm)->_logicHE->overrideImagePixelHitTest(&outValue, globNum, state, x, y, flags)) {
		return outValue;
	}

	int srcComp, srcWidth, srcHeight;
	byte *srcData;
	byte *headerPtr;

	// Get the header (width, height + compression)...
	headerPtr = getWizStateHeaderPrim(globNum, state);

	srcComp = READ_LE_UINT32(headerPtr + _vm->_resourceHeaderSize + 0);
	srcWidth = READ_LE_UINT32(headerPtr + _vm->_resourceHeaderSize + 4);
	srcHeight = READ_LE_UINT32(headerPtr + _vm->_resourceHeaderSize + 8);

	// If the compression type is TRLE...
	if (srcComp == kWCTTRLE) {
		srcData = getWizStateDataPrim(globNum, state);

		// Execute the color lookup...
		int pixel = auxPixelHitTestTRLEImageRelPos(
			srcData + _vm->_resourceHeaderSize, x, y, srcWidth, srcHeight,
			_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));

		if (!(_vm->_game.heversion > 99 || _vm->_isHE995)) {
			return pixel;
		}

		int compType = getWizCompressionType(globNum, state);

		if (LITTLE_ENDIAN_WIZ(compType)) {
			return pixel;
		} else {
			if (WIZ_16BPP(compType)) {
				return SWAP_BYTES_16(pixel);
			} else {
				return pixel;
			}
		}

	} else if (isUncompressedFormatTypeID(srcComp)) {
		WizSimpleBitmap srcBitmap;

		srcData = getWizStateDataPrim(globNum, state);

		srcBitmap.bufferPtr = WizPxShrdBuffer(srcData + _vm->_resourceHeaderSize, false);
		srcBitmap.bitmapWidth = srcWidth;
		srcBitmap.bitmapHeight = srcHeight;

		int pixel = pgReadPixel(&srcBitmap, x, y, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));

		if (!(_vm->_game.heversion > 99 || _vm->_isHE995)) {
			return pixel;
		}

		int compType = getWizCompressionType(globNum, state);

		if (LITTLE_ENDIAN_WIZ(compType)) {
			return pixel;
		} else {
			if (WIZ_16BPP(compType)) {
				return SWAP_BYTES_16(pixel);
			} else {
				return pixel;
			}
		}
	} else {
		return _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR);
	}
}

int Wiz::hitTestWiz(int image, int state, int x, int y, int32 flags) {
	return hitTestWizPrim(image, state, x, y, flags);
}

int Wiz::hitTestWizPrim(int globNum, int state, int x, int y, int32 flags) {
	int srcComp, srcWidth, srcHeight;
	const byte *srcData;
	const byte *headerPtr;
	byte *dataTmp = nullptr;

	int outValue = 0;
	
	if (((ScummEngine_v90he *)_vm)->_logicHE && ((ScummEngine_v90he *)_vm)->_logicHE->overrideImageHitTest(&outValue, globNum, state, x, y, flags)) {
		return outValue;
	}

	if (_vm->_game.heversion == 80) {
		dataTmp = _vm->getResourceAddress(rtImage, globNum);
		assert(dataTmp);
		headerPtr = _vm->findResourceData(MKTAG('W', 'I', 'Z', 'H'), dataTmp);
		assert(headerPtr);
	} else {
		headerPtr = getWizStateHeaderPrim(globNum, state);
	}

	// Get the header (width, height + compression)...
	srcComp = READ_LE_UINT32(headerPtr + _vm->_resourceHeaderSize + 0);
	srcWidth = READ_LE_UINT32(headerPtr + _vm->_resourceHeaderSize + 4);
	srcHeight = READ_LE_UINT32(headerPtr + _vm->_resourceHeaderSize + 8);

	if (_vm->_game.heversion == 80) {
		if (srcComp != kWCTTRLE)
			error("Wiz::hitTestWizPrim(): resource %d has invalid compression type %d", globNum, srcComp);

		srcData = _vm->findResourceData(MKTAG('W', 'I', 'Z', 'D'), dataTmp);

		return auxHitTestTRLEImageRelPos(srcData + _vm->_resourceHeaderSize, x, y, srcWidth, srcHeight);
	}

	if (_vm->_game.heversion > 98) {
		// Flip the test coords if needed and do simple point rejection...
		if (flags & kWRFHFlip) {
			x = ((srcWidth - 1) - x);

			if (x < 0) {
				return 0;
			}

		} else if ((x >= srcWidth) || (x < 0)) {
			return 0;
		}

		if (flags & kWRFVFlip) {
			y = ((srcHeight - 1) - y);

			if (y < 0) {
				return 0;
			}

		} else if ((y >= srcHeight) || (y < 0)) {
			return 0;
		}
	}

	// If the compression type is TRLE...
	if (srcComp == kWCTTRLE) {
		srcData = getWizStateDataPrim(globNum, state);

		return auxHitTestTRLEImageRelPos(srcData + _vm->_resourceHeaderSize, x, y, srcWidth, srcHeight);
	} else if (_vm->_game.heversion > 98 && isUncompressedFormatTypeID(srcComp)) {
		WizSimpleBitmap srcBitmap;

		srcData = getWizStateDataPrim(globNum, state) + _vm->_resourceHeaderSize;

		srcBitmap.bufferPtr = WizPxShrdBuffer(const_cast<byte *>(srcData), false);
		srcBitmap.bitmapWidth = srcWidth;
		srcBitmap.bitmapHeight = srcHeight;

		return (_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR) !=
				pgReadPixel(&srcBitmap, x, y, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR))) ? 1 : 0;
	} else {
		return 0;
	}
}

void Wiz::processWizImagePolyCaptureCmd(const WizImageCommand *params) {
	int polygon1, polygon2, compressionType, srcImage = 0, shadow = 0, state = 0;
	bool isHintColor = false;
	int hintColor = 0;

	// Get all the options...
	if (params->actionFlags & kWAFPolygon) {
		polygon1 = params->polygon;
	} else {
		error("Wiz::processWizImagePolyCaptureCmd(): Image capture poly: no polygon 1 specified.");
	}

	if (params->actionFlags & kWAFPolygon2) {
		polygon2 = params->polygon2;
	} else {
		polygon2 = polygon1;
	}

	if (params->actionFlags & kWAFCompressionType) {
		compressionType = params->compressionType;
	} else {
		compressionType = kWCTNone;
	}

	if (params->actionFlags & kWAFShadow) {
		shadow = params->shadow;
	}

	if (params->actionFlags & kWAFDestImage) {
		error("Wiz::processWizImagePolyCaptureCmd(): destination 'image' not supported, use 'source image'");
	}

	if (params->actionFlags & kWAFSourceImage) {
		srcImage = params->sourceImage;
	}

	if (params->actionFlags & kWAFState) {
		state = params->state;
	}

	if (params->actionFlags & kWAFProperty) {
		if (params->propertyNumber == 1) { // Color hint property
			if (!shadow) {
				debug(7, "Wiz::processWizImagePolyCaptureCmd(): color hint does nothing for an unfiltered scale.");
			}

			isHintColor = true;
			hintColor = params->propertyValue;
		}
	}

	// Validate the parameters...
	bool poly1Found = false;
	bool poly2Found = false;

	for (int polyIndex = 0; polyIndex < ARRAYSIZE(_polygons); ++polyIndex) {
		if (polygon1 == _polygons[polyIndex].id) {
			poly1Found = true;
			polygon1 = polyIndex;
			if (_polygons[polyIndex].numPoints != 5)
				error("Wiz::processWizImagePolyCaptureCmd(): Invalid point count");
		}

		if (polygon2 == _polygons[polyIndex].id) {
			poly2Found = true;
			polygon2 = polyIndex;
			if (_polygons[polyIndex].numPoints != 5)
				error("Wiz::processWizImagePolyCaptureCmd(): Invalid point count");
		}

		if (poly1Found && poly2Found) {
			break;
		}
	}

	if (!poly1Found) {
		error("Wiz::processWizImagePolyCaptureCmd(): Polygon %d not defined", polygon1);
	}

	if (!poly2Found) {
		error("Wiz::processWizImagePolyCaptureCmd(): Polygon %d not defined", polygon2);
	}

	if (_polygons[polygon1].numPoints != _polygons[polygon2].numPoints) {
		error("Wiz::processWizImagePolyCaptureCmd(): Polygons MUST have same number of points.");
	}

	// Create the buffers to hold the source and destination image bitmaps...
	WizSimpleBitmap srcBitmap, destBitmap;
	srcBitmap.bufferPtr = WizPxShrdBuffer();
	destBitmap.bufferPtr = WizPxShrdBuffer();

	// Build a bounding rect for the polys and set the appropriate sizes in the bitmaps...
	Common::Rect destPolyRect;
	Common::Rect srcPolyRect;

	polyBuildBoundingRect(_polygons[polygon2].points, _polygons[polygon2].numPoints, destPolyRect);
	destBitmap.bitmapWidth = getRectWidth(&destPolyRect);
	destBitmap.bitmapHeight = getRectHeight(&destPolyRect);
	destBitmap.bufferPtr = WizPxShrdBufferD(malloc(destBitmap.bitmapWidth * destBitmap.bitmapHeight * sizeof(WizRawPixel)), true);

	if (!destBitmap.bufferPtr()) {
		error("Wiz::processWizImagePolyCaptureCmd(): Could not allocate destination buffer");
	}

	// Fill with transparent color...
	rawPixelMemset(destBitmap.bufferPtr(),
					_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
					destBitmap.bitmapWidth * destBitmap.bitmapHeight);

	// Get the bound rect for the poly...
	polyBuildBoundingRect(_polygons[polygon1].points, _polygons[polygon1].numPoints, srcPolyRect);

	// We need to save the current points so that they can be offset to the correct position in
	// the source buffer in the case of a screen capture (since the whole screen may not be captured,
	// and the polygon may be offset into it)...
	int pointCt;
	Common::Point srcPoints[5];
	for (pointCt = 0; pointCt < 5; ++pointCt) {
		srcPoints[pointCt].x = _polygons[polygon1].points[pointCt].x;
		srcPoints[pointCt].y = _polygons[polygon1].points[pointCt].y;
	}

	// Check for one to one rectangle, which will set up for an image copy later...
	bool oneToOneRect = false;

	// See if they are both rectangles; the '4' notes the maximum amount of vertexes we can have...
	if (polyIsRectangle(_polygons[polygon1].points, 4) && polyIsRectangle(_polygons[polygon2].points, 4)) {
		// Check if the points are all the same, and bail if so...
		for (pointCt = 0; pointCt < 4; ++pointCt) {
			if ((_polygons[polygon1].points[pointCt].x != _polygons[polygon2].points[pointCt].x) ||
				(_polygons[polygon1].points[pointCt].y != _polygons[polygon2].points[pointCt].y)) {
				break;
			}
		}

		if (pointCt == 4) {
			oneToOneRect = true;
		}
	}

	// If there is a source image, get it, otherwise capture from the screen...
	if (srcImage) {
		// get the wiz size
		Common::Rect clipRect;
		int32 w, h;
		getWizImageDim(srcImage, state, w, h);

		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = w;
		clipRect.bottom = h;

		// Make sure the capture area isn't outside or bigger than the source image...
		Common::Rect testRect;
		combineRects(&testRect, &srcPolyRect, &clipRect);

		if ((getRectWidth(&testRect) * getRectHeight(&testRect)) >
			(getRectWidth(&clipRect) * getRectHeight(&clipRect))) {
			error("Wiz::processWizImagePolyCaptureCmd(): Specified polygon captures points outside bounds of source image");
		}

		// Clip poly to image and verify it's within the image...
		if (!findRectOverlap(&srcPolyRect, &clipRect)) {
			error("Wiz::processWizImagePolyCaptureCmd(): Specified polygon doesn't intersect source image.");
		}

		srcBitmap.bitmapWidth = getRectWidth(&srcPolyRect);
		srcBitmap.bitmapHeight = getRectHeight(&srcPolyRect);

		if ((srcBitmap.bitmapWidth == 0) || (srcBitmap.bitmapHeight == 0)) {
			error("Wiz::processWizImagePolyCaptureCmd(): Poly or source image invalid");
		}

		// Create the bitmap...
		srcBitmap.bufferPtr = WizPxShrdBufferD(malloc(srcBitmap.bitmapWidth * srcBitmap.bitmapHeight * sizeof(WizRawPixel)), true);
		if (!srcBitmap.bufferPtr()) {
			error("Wiz::processWizImagePolyCaptureCmd(): Could not allocate source buffer");
		}

		// Set it all to transparent...
		rawPixelMemset(srcBitmap.bufferPtr(),
						_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
						srcBitmap.bitmapWidth * srcBitmap.bitmapHeight);

		drawAWiz(srcImage, state, 0, 0, 0, 0, 0, 0, &srcPolyRect, 0, &srcBitmap);
	} else {
		// If we're here, we are handling a screen capture...
		VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
		int windowWidth =  pvs->w;
		int windowHeight = pvs->h;

		// Intersect the bound rect and the VirtScreen rect...
		Common::Rect clipRect;
		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = windowWidth - 1;
		clipRect.bottom = windowHeight - 1;

		if (!findRectOverlap(&srcPolyRect, &clipRect)) {
			error("Wiz::processWizImagePolyCaptureCmd(): Specified polygon doesn't intersect screen.");
		}

		srcBitmap.bitmapWidth = getRectWidth(&srcPolyRect);
		srcBitmap.bitmapHeight = getRectHeight(&srcPolyRect);

		if ((srcBitmap.bitmapWidth == 0) || (srcBitmap.bitmapHeight == 0)) {
			error("Wiz::processWizImagePolyCaptureCmd(): Specified screen rectangle invalid.");
		}

		// Create the bitmap...
		srcBitmap.bufferPtr = WizPxShrdBufferD(malloc(srcBitmap.bitmapWidth * srcBitmap.bitmapHeight * sizeof(WizRawPixel)), true);
		if (!srcBitmap.bufferPtr()) {
			error("Wiz::processWizImagePolyCaptureCmd(): Could not allocate source buffer");
		}

		// Set it all to transparent...
		rawPixelMemset(srcBitmap.bufferPtr(),
						_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
						srcBitmap.bitmapWidth * srcBitmap.bitmapHeight);

		// Fill it with screen data...
		WizRawPixel *screenPtr = (WizRawPixel *)pvs->getPixels(srcPolyRect.left, srcPolyRect.top);
		WizPxShrdBuffer destPtr = srcBitmap.bufferPtr;

		int screenRowLen = 640;
		int destRowLen = srcBitmap.bitmapWidth;

		if (_uses16BitColor) {
			WizRawPixel16 *screen16 = (WizRawPixel16 *)screenPtr;
			WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr();

			for (int i = 0; i < srcBitmap.bitmapHeight; ++i) {
				memcpy(dest16, screen16, destRowLen);
				screen16 += screenRowLen;
				dest16 += destRowLen;
			}
		} else {
			WizRawPixel8 *screen8 = (WizRawPixel8 *)screenPtr;
			WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr();

			for (int i = 0; i < srcBitmap.bitmapHeight; ++i) {
				memcpy(dest8, screen8, destRowLen);
				screen8 += screenRowLen;
				dest8 += destRowLen;
			}
		}

		// Translate the polygon so it is in the correct place in the buffer...
		int dx = 0, dy = 0;
		dx = 0 - srcPolyRect.left;
		dy = 0 - srcPolyRect.top;
		polyMovePolygonPoints(srcPoints, _polygons[polygon1].numPoints, dx, dy);
	}

	// If there is an xmap shadow, perform a filtered warp...
	if (shadow) {
		// Het the color map, bypass the header information...
		const byte *xmapColorTable = getColorMixBlockPtrForWiz(shadow);

		if (!xmapColorTable) {
			error("Wiz::processWizImagePolyCaptureCmd(): Shadow specified but not present in image.");
		}

		xmapColorTable += _vm->_resourceHeaderSize;

		WarpWizPoint polypoints[5];
		for (int i = 0; i < 5; i++) {
			WarpWizPoint tmp(_polygons[polygon2].points[i]);
			polypoints[i] = tmp;
		}

		WarpWizPoint srcWarpPoints[5];
		for (int i = 0; i < 5; i++) {
			WarpWizPoint tmp(srcPoints[i]);
			srcWarpPoints[i] = tmp;
		}

		warpNPt2NPtNonClippedWarpFiltered(
			&destBitmap, polypoints, &srcBitmap, srcWarpPoints,
			_polygons[polygon1].numPoints, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
			xmapColorTable, isHintColor, (WizRawPixel)hintColor);

	} else if (oneToOneRect) { // If a one to one copy is performed, just copy this bitmap...
		memcpy(destBitmap.bufferPtr(), srcBitmap.bufferPtr(), destBitmap.bitmapHeight * destBitmap.bitmapWidth);
	} else { // Otherwise fallback to regular warping...
		WarpWizPoint polypoints[5];
		for (int i = 0; i < 5; i++) {
			WarpWizPoint tmp(_polygons[polygon2].points[i]);
			polypoints[i] = tmp;
		}

		WarpWizPoint srcWarpPoints[5];
		for (int i = 0; i < 5; i++) {
			WarpWizPoint tmp(srcPoints[i]);
			srcWarpPoints[i] = tmp;
		}

		warpNPt2NPtNonClippedWarp(
			&destBitmap, polypoints, &srcBitmap, srcWarpPoints,
			_polygons[polygon1].numPoints, _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
	}

	// Now build a Wiz with the destination bitmap and throw the bitmaps away...
	srcBitmap.bufferPtr = WizPxShrdBuffer();

	uint8 *palPtr = nullptr;
	if (_vm->_game.heversion >= 99) {
		palPtr = _vm->_hePalettes + _vm->_hePaletteSlot;
	} else {
		palPtr = _vm->_currentPalette;
	}

	buildAWiz(destBitmap.bufferPtr,
			  destBitmap.bitmapWidth,
			  destBitmap.bitmapHeight,
			  palPtr,
			  &destPolyRect,
			  compressionType,
			  params->image,
			  _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));

	destBitmap.bufferPtr = WizPxShrdBuffer();

	_vm->_res->setModified(rtImage, params->image);
}

void Wiz::flushAWizBuffer() {
	if (_wizBufferIndex == 0)
		return;

	for (int i = 0; i < _wizBufferIndex; i++) {
		drawAWiz(
			_wizBuffer[i].image, _wizBuffer[i].state,
			_wizBuffer[i].x, _wizBuffer[i].y, _wizBuffer[i].z,
			_wizBuffer[i].flags,
			_wizBuffer[i].shadow,
			_wizBuffer[i].zbuffer,
			0,
			_wizBuffer[i].palette,
			0);
	}

	_wizBufferIndex = 0;
}

void Wiz::loadWizCursor(int resId, int palette, bool useColor) {
	int32 x, y;

	getWizSpot(resId, 0, x, y);

	if (x < 0) {
		x = 0;
	} else if (x > 32) {
		x = 32;
	}

	if (y < 0) {
		y = 0;
	} else if (y > 32) {
		y = 32;
	}

	WizRawPixel *colorConversionTable = nullptr;

	if (palette != 0)
		colorConversionTable = (WizRawPixel *) _vm->getHEPaletteSlot(palette);

	WizPxShrdBuffer cursorBuffer = drawAWizPrim(resId, 0, 0, 0, 0, 0, 0, nullptr, kWRFAlloc, nullptr, colorConversionTable);
	byte *cursor = (byte*)cursorBuffer();

	int32 cw, ch;
	getWizImageDim(resId, 0, cw, ch);

	// Hello! This is a hack :-D
	// In the original code, when useColor is false, the engine
	// instructs the OS to set the cursor to black and white only.
	// We obtain the same effect doing it like this...
	if (!useColor) {
		if (_vm->_bytesPerPixel == 1) {
			for (int i = 0; i < ch * cw; i++) {
				if (cursor[i] != 0 && cursor[i] != 5)
					cursor[i] = 15;
			}
		} else {
			for (int i = 0; i < ch * cw; i++) {
				if (((uint16 *)cursor)[i] != 0 && ((uint16 *)cursor)[i] != 5)
					((uint16 *)cursor)[i] = 0x7FFF;
			}
		}
	}

	_vm->setCursorHotspot(x, y);
	_vm->setCursorFromBuffer(cursor, cw, ch, cw * _vm->_bytesPerPixel);

	// Since we set up cursor palette for default cursor, disable it now...
	CursorMan.disableCursorPalette(true);
}

#define ADD_REQUIRED_IMAGE(whatImageIsRequired) {                                                                                    \
	if (ARRAYSIZE(requiredImages) <= (requiredImageCount + 1)) {                                                                     \
		error("Wiz::processWizImageDrawCmd(): Too many required images for image operation (limit %d).", ARRAYSIZE(requiredImages)); \
	}                                                                                                                                \
	requiredImages[requiredImageCount] = whatImageIsRequired;                                                                        \
	++requiredImageCount;                                                                                                            \
}

void Wiz::processWizImageDrawCmd(const WizImageCommand *params) {
	int shadowImage, state, angle, scale, paletteNumber, sourceImage;
	int zbufferImage = 0;
	const WizRawPixel *colorConversionTablePtr;
	Common::Rect *optionalRect;
	WizSimpleBitmap *destBitmap;
	WizSimpleBitmap fakeBitmap;
	Common::Rect clipRect;
	int32 flags;
	Common::Point pt;

	int requiredImages[5] = {0, 0, 0, 0, 0};
	int requiredImageCount = 0;

	ADD_REQUIRED_IMAGE(params->image);

	if (params->actionFlags & kWAFSourceImage) {
		sourceImage = params->sourceImage;
		ADD_REQUIRED_IMAGE(sourceImage);
	} else {
		sourceImage = 0;
	}

	if (params->actionFlags & kWAFPalette) {
		paletteNumber = params->palette;
	} else {
		paletteNumber = 0;
	}

	if (params->actionFlags & kWAFScale) {
		scale = params->scale;
	} else {
		scale = 256;
	}

	if (params->actionFlags & kWAFAngle) {
		angle = params->angle;
	} else {
		angle = 0;
	}

	if (params->actionFlags & kWAFState) {
		state = params->state;
	} else {
		state = 0;
	}

	if (params->actionFlags & kWAFFlags) {
		flags = params->flags;
	} else {
		flags = 0;
	}

	if (params->actionFlags & kWAFSpot) {
		pt.x = params->xPos;
		pt.y = params->yPos;
	} else {
		pt.x = 0;
		pt.y = 0;
	}

	if (params->actionFlags & kWAFShadow) {
		shadowImage = params->shadow;
		ADD_REQUIRED_IMAGE(shadowImage);
	} else {
		shadowImage = 0;
	}

	if (_vm->_game.heversion > 99 || _vm->_isHE995) {
		if (params->actionFlags & kWAFZBufferImage) {
			zbufferImage = params->zbufferImage;
			ADD_REQUIRED_IMAGE(zbufferImage);
		}
	}

	if (params->actionFlags & kWAFRect) {
		clipRect.left = params->box.left;
		clipRect.top = params->box.top;
		clipRect.right = params->box.right;
		clipRect.bottom = params->box.bottom;
		optionalRect = &clipRect;
	} else {
		optionalRect = nullptr;
	}

	if (params->actionFlags & kWAFDestImage) {
		ADD_REQUIRED_IMAGE(params->destImageNumber);
	}

	if (requiredImageCount > 0) {
		// Mark the all the image resources as stuck...
		for (int i = 0; i < requiredImageCount; i++) {
			_vm->_res->lock(rtImage, requiredImages[i]);
		}

		// Make sure that the resources are in RAM by requesting their address...
		for (int i = 0; i < requiredImageCount; i++) {
			_vm->ensureResourceLoaded(rtImage, requiredImages[i]);
		}

		// Mark the all the image resources as NOT stuck...
		for (int i = 0; i < requiredImageCount; i++) {
			_vm->_res->unlock(rtImage, requiredImages[i]);
		}

		// Validate that all of the images are now on the heap!
		for (int i = 0; i < requiredImageCount; i++) {
			if (!_vm->getResourceAddress(rtImage, requiredImages[i])) {
				error("Wiz::processWizImageDrawCmd(): Image %d missing for image operation", requiredImages[i]);
			}
		}

		// Check if the images are in their native format and swap them if needed...
		if (_vm->_game.heversion > 99) {
			for (int i = 0; i < requiredImageCount; i++) {
				ensureNativeFormatImageForState(requiredImages[i], state);
			}
		}
	}

	if (params->actionFlags & kWAFDestImage) {
		// Get the rendering surface for this image...
		if (!dwSetSimpleBitmapStructFromImage(params->destImageNumber, 0, &fakeBitmap)) {
			error("Wiz::processWizImageDrawCmd(): Image %d is invalid for rendering into", params->destImageNumber);
		}

		destBitmap = &fakeBitmap;
	} else {
		destBitmap = nullptr;
	}

	if ((_vm->_game.heversion > 99 || _vm->_isHE995) && (params->actionFlags & kWAFRemapList)) {
		processWizImageModifyCmd(params);
		flags |= kWRFRemap;
	}

	// Dispatch the command...
	if (!_vm->_fullRedraw || destBitmap != 0) {
		if (sourceImage != 0) {
			dwAltSourceDrawWiz(
				params->image, state, pt.x, pt.y,
				sourceImage, 0, flags, paletteNumber,
				optionalRect, destBitmap);
		} else {
			if (!(params->actionFlags & (kWAFScale | kWAFAngle))) {
				drawAWizEx(
					params->image, state,
					pt.x, pt.y, params->zPos,
					flags,
					shadowImage, zbufferImage,
					optionalRect,
					paletteNumber,
					destBitmap,
					params);
			} else {
				if (paletteNumber) {
					colorConversionTablePtr = (WizRawPixel *)_vm->getHEPaletteSlot(paletteNumber);
				} else {
					colorConversionTablePtr = nullptr;
				}

				dwHandleComplexImageDraw(
					params->image, state, pt.x, pt.y, shadowImage, angle, scale,
					optionalRect, flags, destBitmap, colorConversionTablePtr);
			}
		}
	} else {
		if (sourceImage != 0 || (params->actionFlags & (kWAFScale | kWAFAngle)))
			error("Can't do this command in the enter script");

		bufferAWiz(params->image, state, pt.x, pt.y, params->zPos, flags, shadowImage, zbufferImage, paletteNumber);
	}
}

#undef ADD_REQUIRED_IMAGE

void Wiz::dwCreateRawWiz(int imageNum, int w, int h, int flags, int bitsPerPixel, int optionalSpotX, int optionalSpotY) {
	int compressionType, wizdSize;

	int globSize = _vm->_resourceHeaderSize; // AWIZ header size
	globSize += WIZBLOCK_WIZH_SIZE;

	if (flags & kCWFPalette) {
		globSize += WIZBLOCK_RGBS_SIZE;
	}

	if (flags & kCWFSpot) {
		globSize += WIZBLOCK_SPOT_SIZE;
	}

	if (flags & kCWFRemapTable) {
		globSize += WIZBLOCK_RMAP_SIZE;
	}

	globSize += _vm->_resourceHeaderSize; // WIZD header size
	wizdSize = (w * h * (bitsPerPixel / 8));
	globSize += wizdSize;

	uint8 *writePtr = _vm->_res->createResource(rtImage, imageNum, globSize);

	if (!writePtr) {
		_vm->VAR(_vm->VAR_OPERATION_FAILURE) = -1;
		return;
	} else {
		_vm->VAR(_vm->VAR_OPERATION_FAILURE) = 0;
	}

	switch (bitsPerPixel) {
	case 8:
		compressionType = kWCTNone;
		break;
	case 16:
		compressionType = kWCTNone16Bpp;
		break;
	default:
		error("Unsupported image bits size %d", bitsPerPixel);
		break;
	}

	WRITE_BE_UINT32(writePtr, MKTAG('A', 'W', 'I', 'Z')); writePtr += 4;
	WRITE_BE_UINT32(writePtr, globSize); writePtr += 4;
	WRITE_BE_UINT32(writePtr, MKTAG('W', 'I', 'Z', 'H')); writePtr += 4;
	WRITE_BE_UINT32(writePtr, WIZBLOCK_WIZH_SIZE); writePtr += 4;
	WRITE_LE_UINT32(writePtr, compressionType); writePtr += 4;
	WRITE_LE_UINT32(writePtr, w); writePtr += 4;
	WRITE_LE_UINT32(writePtr, h); writePtr += 4;

	if (flags & kCWFPalette) {
		const uint8 *palPtr;
		if (_vm->_game.heversion >= 99) {
			palPtr = _vm->_hePalettes + _vm->_hePaletteSlot;
		} else {
			palPtr = _vm->_currentPalette;
		}

		WRITE_BE_UINT32(writePtr, MKTAG('R', 'G', 'B', 'S')); writePtr += 4;
		WRITE_BE_UINT32(writePtr, WIZBLOCK_RGBS_SIZE); writePtr += 4;
		memcpy(writePtr, palPtr, WIZBLOCK_RGBS_DATA_SIZE);

		writePtr += WIZBLOCK_RGBS_DATA_SIZE;
	}

	if (flags & kCWFSpot) {
		WRITE_BE_UINT32(writePtr, MKTAG('S', 'P', 'O', 'T')); writePtr += 4;
		WRITE_BE_UINT32(writePtr, WIZBLOCK_SPOT_SIZE); writePtr += 4;
		WRITE_LE_UINT32(writePtr + 0, optionalSpotX);
		WRITE_LE_UINT32(writePtr + 4, optionalSpotY);

		writePtr += WIZBLOCK_SPOT_DATA_SIZE;
	}

	if (flags & kCWFRemapTable) {
		WRITE_BE_UINT32(writePtr, MKTAG('R', 'M', 'A', 'P')); writePtr += 4;
		WRITE_BE_UINT32(writePtr, WIZBLOCK_RMAP_SIZE); writePtr += 4;
		WRITE_LE_UINT32(writePtr, 0); writePtr += 4;

		for (int i = 0; i < 256; ++i) {
			*writePtr++ = i;
		}
	}

	WRITE_BE_UINT32(writePtr, MKTAG('W','I', 'Z', 'D')); writePtr += 4;
	WRITE_BE_UINT32(writePtr, 8 + wizdSize); writePtr += 4;
}

bool Wiz::dwGetMultiTypeBitmapFromImageState(int imageNum, int imageState, WizMultiTypeBitmap *multiBM) {
	int compType, imageWidth, imageHeight;
	byte *wizHeader;
	byte *dataPtr;

	// Get the image header...
	wizHeader = getWizStateHeaderPrim(imageNum, imageState);

	if (!wizHeader) {
		memset(multiBM, 0, sizeof(WizMultiTypeBitmap));
		return false;
	}

	// Double check the image header compression type...
	compType = READ_LE_UINT32(wizHeader + _vm->_resourceHeaderSize);

	if (!isUncompressedFormatTypeID(compType)) {
		memset(multiBM, 0, sizeof(WizMultiTypeBitmap));
		return false;
	}

	imageWidth = READ_LE_UINT32(wizHeader + _vm->_resourceHeaderSize + 4);
	imageHeight = READ_LE_UINT32(wizHeader + _vm->_resourceHeaderSize + 8);

	dataPtr = getWizStateDataPrim(imageNum, imageState);

	if (!dataPtr) {
		memset(multiBM, 0, sizeof(WizMultiTypeBitmap));
		return false;
	}

	// Hook up the image info to the simple bitmap info...
	multiBM->data = (dataPtr + _vm->_resourceHeaderSize);
	multiBM->width = imageWidth;
	multiBM->height = imageHeight;

	switch (compType) {
	case kWCTNone:
		multiBM->bpp = 8;
		multiBM->format = 8;
		break;
	case kWCTNone16Bpp:
	case kWCTNone16BppBigEndian:
		multiBM->bpp = 16;
		multiBM->format = 555;
		break;
	default:
		memset(multiBM, 0, sizeof(WizMultiTypeBitmap));
		return false;
		break;
	}

	multiBM->stride = (multiBM->width * multiBM->bpp) / 8;

	return true;
}

bool Wiz::dwSetSimpleBitmapStructFromImage(int imageNum, int imageState, WizSimpleBitmap *destBM) {
	int compType, imageWidth, imageHeight;
	byte *wizHeader;
	byte *dataPtr;

	// Get the image header...
	wizHeader = (byte *)getWizStateHeaderPrim(imageNum, imageState);

	if (!wizHeader) {
		return false;
	}

	// Double check the image header compression type...
	compType = READ_LE_UINT32(wizHeader + _vm->_resourceHeaderSize);

	if (!isUncompressedFormatTypeID(compType)) {
		return false;
	}

	imageWidth = READ_LE_UINT32(wizHeader + _vm->_resourceHeaderSize + 4);
	imageHeight = READ_LE_UINT32(wizHeader + _vm->_resourceHeaderSize + 8);

	// Fetch the data...
	dataPtr = (byte *)getWizStateDataPrim(imageNum, imageState);

	if (!dataPtr) {
		return false;
	}

	// Hook up the image info to the simple bitmap info...
	destBM->bufferPtr = WizPxShrdBuffer(dataPtr + _vm->_resourceHeaderSize, false);
	destBM->bitmapWidth = imageWidth;
	destBM->bitmapHeight = imageHeight;

	return true;
}

int Wiz::dwTryToLoadWiz(Common::SeekableReadStream *inFile, const WizImageCommand *params) {
	uint32 blockSize;
	uint32 blockId;
	byte *ptr;

	inFile->seek(0, SEEK_SET);
	blockId = inFile->readUint32BE();

	if ((blockId != MKTAG('A', 'W', 'I', 'Z')) && (blockId != MKTAG('M', 'U', 'L', 'T'))) {
		return DW_LOAD_NOT_TYPE;
	}

	blockSize = inFile->readUint32BE();
	inFile->seek(-8, SEEK_CUR);

	ptr = _vm->_res->createResource(rtImage, params->image, blockSize);

	if (inFile->read(ptr, blockSize) != blockSize) {
		_vm->_res->nukeResource(rtImage, params->image);
		return DW_LOAD_READ_FAILURE;
	}

	_vm->_res->setModified(rtImage, params->image);
	return DW_LOAD_SUCCESS;
}

void Wiz::dwAltSourceDrawWiz(int maskImage, int maskState, int x, int y, int sourceImage, int sourceState, int32 flags, int paletteNumber, const Common::Rect *optionalClipRect, const WizSimpleBitmap *destBitmapPtr) {
	int srcBitsPerPixel, sourceCompressionType, maskCompressionType;
	int32 srcBitmapWidth, srcBitmapHeight, maskWidth, maskHeight;
	Common::Rect clipRect, destRect;
	WizSimpleBitmap drawBufferBitmap;
	WizRawPixel *conversionTable;
	byte *sourceBufferPtr;
	byte *maskDataPtr;
	bool markUpdates;

	// Get the conversion table if any...
	if (paletteNumber) {
		conversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(paletteNumber);
	} else {
		if (_uses16BitColor) {
			conversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(1); // Generic conversion table...
		} else {
			conversionTable = nullptr;
		}
	}

	// Get the destination bitmap...
	if (!destBitmapPtr) {
		markUpdates = true;

		if (flags & kWRFForeground) {
			pgSimpleBitmapFromDrawBuffer(&drawBufferBitmap, false);
		} else {
			pgSimpleBitmapFromDrawBuffer(&drawBufferBitmap, true);
		}

		destBitmapPtr = &drawBufferBitmap;
	} else {
		markUpdates = false;
	}

	// Check for overlap with the optional clip rectangle, if any...
	clipRect.left = 0;
	clipRect.top = 0;
	clipRect.right = destBitmapPtr->bitmapWidth - 1;
	clipRect.bottom = destBitmapPtr->bitmapHeight - 1;

	if (optionalClipRect) {
		if (!findRectOverlap(&clipRect, optionalClipRect)) {
			return; // We're done, there is no update region.
		}
	}

	// Get the source (alt) bitmap pointer, and general info...
	sourceCompressionType = getWizCompressionType(sourceImage, sourceState);

	if (!dwIsUncompressedFormatTypeID(sourceCompressionType)) {
		error("Wiz::dwAltSourceDrawWiz(): Source image %d must be uncompressed", sourceImage);
	}

	switch (sourceCompressionType) {

	default:
	case kWCTNone:
		srcBitsPerPixel = 8; // Default
		break;

	case kWCTNone16Bpp:
	case kWCTNone16BppBigEndian:
		srcBitsPerPixel = 16;
		break;
	}

	// Get the source wiz data pointer...
	sourceBufferPtr = (byte *)getWizStateDataPrim(sourceImage, sourceState);

	if (!sourceBufferPtr) {
		error("Wiz::dwAltSourceDrawWiz(): Image %d missing data block", sourceImage);
	}

	sourceBufferPtr += _vm->_resourceHeaderSize;
	getWizImageDim(sourceImage, sourceState, srcBitmapWidth, srcBitmapHeight);

	if ((destBitmapPtr->bitmapWidth != srcBitmapWidth) ||
		(destBitmapPtr->bitmapHeight != srcBitmapHeight)) {

		error(
			"Wiz::dwAltSourceDrawWiz(): Source image %d and dest image size mismatch (%d,%d) need (%d,%d)",
			sourceImage, srcBitmapWidth, srcBitmapHeight,
			destBitmapPtr->bitmapWidth, destBitmapPtr->bitmapHeight);
	}

	// Finally get the compressed data pointer...
	maskCompressionType = getWizCompressionType(maskImage, maskState);

	if (!dwIsMaskCompatibleCompressionType(maskCompressionType)) {
		error("Wiz::dwAltSourceDrawWiz(): Mask image %d must be a maskable compression type", maskImage);
	}

	maskDataPtr = (byte *)getWizStateDataPrim(maskImage, maskState);

	if (!maskDataPtr) {
		error("Wiz::dwAltSourceDrawWiz(): Image %d missing data block", maskImage);
	}

	maskDataPtr += _vm->_resourceHeaderSize;
	getWizImageDim(maskImage, maskState, maskWidth, maskHeight);

	// Make sure that we have an overlap before we call the decompressor...
	destRect.left = x;
	destRect.top = y;
	destRect.right = x + maskWidth - 1;
	destRect.bottom = y + maskHeight - 1;

	if (!findRectOverlap(&destRect, &clipRect)) {
		// We're done, there is no update region...
		return;
	}

	// Finally call the primitive...
	if (maskCompressionType == kWCTTRLE) {
		trleFLIPAltSourceDecompressImage(
			destBitmapPtr->bufferPtr(), maskDataPtr,
			destBitmapPtr->bitmapWidth, destBitmapPtr->bitmapHeight,
			sourceBufferPtr, srcBitmapWidth, srcBitmapHeight, srcBitsPerPixel,
			x, y, maskWidth, maskHeight, &clipRect, flags, conversionTable,
			nullptr);

	} else if (maskCompressionType == kWCTMRLEWithLineSizePrefix) {
		mrleFLIPAltSourceDecompressImage(
			destBitmapPtr->bufferPtr(), maskDataPtr,
			destBitmapPtr->bitmapWidth, destBitmapPtr->bitmapHeight,
			sourceBufferPtr, srcBitmapWidth, srcBitmapHeight, srcBitsPerPixel,
			x, y, maskWidth, maskHeight, &clipRect, flags, conversionTable);
	}

	// What type of update is necessary?
	if (!(flags & kWRFAlloc) && markUpdates) {
		// If neither foreground or background, copy to both...
		if ((flags & (kWRFBackground | kWRFForeground)) == 0) {
			_vm->backgroundToForegroundBlit(destRect);
		} else {
			++destRect.bottom;
			_vm->markRectAsDirty(kMainVirtScreen, destRect);
		}
	}
}

void Wiz::dwHandleComplexImageDraw(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	int32 w, h;
	int correctedAngle;
	Common::Point listOfPoints[4];
	const byte *shadowPtr;

	// Set the optional remap table up to the default if one isn't specified...
	if (!optionalColorConversionTable) {
		if (_uses16BitColor) {
			optionalColorConversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(1);
		}
	}

	// Setup the initial quad (0,0) relative...
	getWizImageDim(image, state, w, h);

	listOfPoints[0].x = -w / 2;
	listOfPoints[0].y = -h / 2;

	listOfPoints[1].x = listOfPoints[0].x + w - 1;
	listOfPoints[1].y = listOfPoints[0].y;

	listOfPoints[2].x = listOfPoints[1].x;
	listOfPoints[2].y = listOfPoints[0].y + h - 1;

	listOfPoints[3].x = listOfPoints[0].x;
	listOfPoints[3].y = listOfPoints[2].y;

	// Hflip?
	if (flags & kWRFHFlip) {
		SWAP<int16>(listOfPoints[0].x, listOfPoints[1].x);
		SWAP<int16>(listOfPoints[2].x, listOfPoints[3].x);
	}

	// VFlip?
	if (flags & kWRFVFlip) {
		SWAP<int16>(listOfPoints[0].y, listOfPoints[1].y);
		SWAP<int16>(listOfPoints[2].y, listOfPoints[3].y);
	}

	// Scale the points?
	if (scale != 256) {
		for (int i = 0; i < 4; i++) {
			listOfPoints[i].x = (scale * listOfPoints[i].x) / 256;
			listOfPoints[i].y = (scale * listOfPoints[i].y) / 256;
		}
	}

	// Rotate the points?
	if (angle) {
		polyRotatePoints(listOfPoints, 4, angle);
	}

	// Offset the points...
	polyMovePolygonPoints(listOfPoints, 4, x, y);

	// Special case rotate 0,90,180,270 degree special cases...
	if (scale == 256) {
		Common::Rect boundingRect;

		correctedAngle = abs(angle % 360);

		if (angle < 0) {
			correctedAngle = (360 - correctedAngle);
		}

		// Get the upper left point so that our blit matches 
		// in position the normal warp drawing function...
		polyBuildBoundingRect(listOfPoints, 4, boundingRect);
		x = boundingRect.left;
		y = boundingRect.top;

		// Special case renderers don't use shadows...
		if (!shadow || (_vm->_game.heversion <= 99 && !_vm->_isHE995)) {
			switch (correctedAngle) {
			case 0:
				handleRotate0SpecialCase(
					image, state, x, y, shadow, correctedAngle, scale, clipRect, flags,
					optionalBitmapOverride, optionalColorConversionTable);
				return;

			case 90:
				handleRotate90SpecialCase(
					image, state, x, y, shadow, correctedAngle, scale, clipRect, flags,
					optionalBitmapOverride, optionalColorConversionTable);
				return;

			case 180:
				handleRotate180SpecialCase(
					image, state, x, y, shadow, correctedAngle, scale, clipRect, flags,
					optionalBitmapOverride, optionalColorConversionTable);
				return;

			case 270:
				handleRotate270SpecialCase(
					image, state, x, y, shadow, correctedAngle, scale, clipRect, flags,
					optionalBitmapOverride, optionalColorConversionTable);
				return;
			}
		}
	}

	// If there is a shadow get it's address...
	if (shadow) {
		shadowPtr = getColorMixBlockPtrForWiz(shadow);

		// Let's replicate whichever bug they had in the interpreter...
		if (_vm->_game.heversion > 99 || _vm->_isHE995)
			shadowPtr += _vm->_resourceHeaderSize;
	} else {
		shadowPtr = nullptr;
	}

	WarpWizPoint listOfWarpPoints[4];
	for (int i = 0; i < 4; i++) {
		WarpWizPoint tmp(listOfPoints[i]);
		listOfWarpPoints[i] = tmp;
	}

	// Finally call the renderer...
	warpDrawWizTo4Points(
		image, state, listOfWarpPoints, flags,
		_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR),
		clipRect, optionalBitmapOverride, optionalColorConversionTable,
		shadowPtr);
}

bool Wiz::dwIsMaskCompatibleCompressionType(int compressionType) {
	return (kWCTTRLE == compressionType) || (kWCTMRLEWithLineSizePrefix == compressionType);
}

bool Wiz::dwIsUncompressedFormatTypeID(int id) {
	return ((kWCTNone == id)      ||
		    (kWCTNone16Bpp == id) ||
		    (kWCTNone16BppBigEndian == id));
}

int Wiz::dwGetImageGeneralProperty(int image, int state, int property) {
	if (_vm->_isHE995)
		return 0;

	switch (property) {
	case kWIPCompressionType:
		return getWizCompressionType(image, state);
		break;

	case kWIPPaletteBlockPresent:
		return doesStateContainBlock(image, state, MKTAG('R', 'G', 'B', 'S')) ? 1 : 0;
		break;

	case kWIPRemapBlockPresent:
		return doesStateContainBlock(image, state, MKTAG('R', 'M', 'A', 'P')) ? 1 : 0;
		break;

	case kWIPOpaqueBlockPresent:
		return doesRawWizStateHaveTransparency(image, state) ? 1 : 0;
		break;

	case kWIPXMAPBlockPresent:
		return doesStateContainBlock(image, state, MKTAG('X', 'M', 'A', 'P')) ? 1 : 0;
		break;

	default:
		debug("Wiz::dwGetImageGeneralProperty(): image %d state %d property %d (unknown property id).", image, state, property);
		break;
	}

	return 0;
}

void Wiz::handleRotate0SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	drawAWizPrim(image, state, x, y, 0, shadow, 0, clipRect, flags, optionalBitmapOverride, optionalColorConversionTable);
}

void Wiz::handleRotate90SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	WizSimpleBitmap srcBitmap, dstBitmap;
	Common::Rect updateRect;
	int compressionType;

	// Make the update rect and check it against the clip rect if one...
	int32 w, h;
	getWizImageDim(image, state, w, h);
	makeSizedRectAt(&updateRect, x, y, h, w); // We are swapping height and width on purpose!

	if (clipRect) {
		if (!findRectOverlap(&updateRect, clipRect)) {
			return;
		}
	}

	// Check to see if this is yet another special case :-)
	compressionType = getWizCompressionType(image, state);

	if (compressionType == kWCTTRLE) {
		int dest_w, dest_h, src_w, src_h;
		const byte *compressedDataPtr;
		WizPxShrdBuffer dest_p;

		// Get the size of the compressed image...
		src_w = w;
		src_h = h;

		// Get the compressed data pointer...
		compressedDataPtr = (byte *)getWizStateDataPrim(image, state);
		compressedDataPtr += _vm->_resourceHeaderSize;

		// Get the write data...
		if (optionalBitmapOverride) {
			dest_p = optionalBitmapOverride->bufferPtr;
			dest_w = optionalBitmapOverride->bitmapWidth;
			dest_h = optionalBitmapOverride->bitmapHeight;
		} else {
			VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];

			dest_w = pvs->w;
			dest_h = pvs->h;

			if (flags & kWRFForeground) {
				dest_p = WizPxShrdBuffer(pvs->getPixels(0, 0), false);
			} else {
				dest_p = WizPxShrdBuffer(pvs->getBackPixels(0, 0), false);
			}
		}

		trleFLIPRotate90DecompressImage(
			dest_p(), compressedDataPtr, dest_w, dest_h, x, y, src_w, src_h,
			clipRect, flags, nullptr, optionalColorConversionTable,
			nullptr);

		// Update the screen? (If not writing to another bitmap...)
		if (!optionalBitmapOverride) {
			if (!(flags & kWRFForeground)) {
				_vm->backgroundToForegroundBlit(updateRect);
			} else {
				++updateRect.bottom;
				_vm->markRectAsDirty(kMainVirtScreen, updateRect);
			}
		}

		return;
	}

	// Get the image from the basic drawing function...
	srcBitmap.bufferPtr = drawAWizPrim(
		image, state, 0, 0, 0, 0, 0, 0, kWRFAlloc,
		0, optionalColorConversionTable);

	srcBitmap.bitmapWidth = w;
	srcBitmap.bitmapHeight = h;

	// Get the bitmap to render into...
	if (optionalBitmapOverride) {
		dstBitmap = *optionalBitmapOverride;
	} else {
		pgSimpleBitmapFromDrawBuffer(&dstBitmap, (kWRFForeground != (kWRFForeground & flags)));
	}

	// Call the 90 blit function...
	if (_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR) == -1) {
		pgBlit90DegreeRotate(
			&dstBitmap, x, y, &srcBitmap, nullptr, clipRect,
			(flags & kWRFHFlip), (flags & kWRFVFlip));
	} else {
		pgBlit90DegreeRotateTransparent(
			&dstBitmap, x, y, &srcBitmap, nullptr, clipRect,
			(flags & kWRFHFlip), (flags & kWRFVFlip),
			(WizRawPixel)_vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));
	}

	// Free up the temporary pointer...
	srcBitmap.bufferPtr = WizPxShrdBuffer();

	// Update the screen? (If not writing to another bitmap...)
	if (!optionalBitmapOverride) {
		if (!(flags & kWRFForeground)) {
			_vm->backgroundToForegroundBlit(updateRect);
		} else {
			++updateRect.bottom;
			_vm->markRectAsDirty(kMainVirtScreen, updateRect);
		}
	}
}

void Wiz::handleRotate180SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	flags ^= (kWRFVFlip | kWRFHFlip);

	drawAWizPrim(image, state, x, y, 0, shadow, 0, clipRect, flags, optionalBitmapOverride, optionalColorConversionTable);
}

void Wiz::handleRotate270SpecialCase(int image, int state, int x, int y, int shadow, int angle, int scale, const Common::Rect *clipRect, int32 flags, WizSimpleBitmap *optionalBitmapOverride, const WizRawPixel *optionalColorConversionTable) {
	flags ^= (kWRFVFlip | kWRFHFlip);

	handleRotate90SpecialCase(
		image, state, x, y, shadow, angle, scale,
		clipRect, flags, optionalBitmapOverride,
		optionalColorConversionTable);
}

void Wiz::processWizImageRenderRectCmd(const WizImageCommand *params) {
	Common::Rect renderRect, clipRect, workClipRect;
	int whichState, whichImage;
	int32 w, h;
	WizSimpleBitmap renderBitmap;
	WizRawPixel whatColor;

	// What state is going to rendered into?
	if (params->actionFlags & kWAFState) {
		whichState = params->state;
	} else {
		whichState = 0;
	}

	whichImage = params->image;

	// Make the clipping rect for this image / state...
	getWizImageDim(whichImage, whichState, w, h);
	makeSizedRectAt(&clipRect, 0, 0, w, h);

	if (params->actionFlags & kWAFRect) {
		workClipRect.left = params->box.left;
		workClipRect.top = params->box.top;
		workClipRect.right = params->box.right;
		workClipRect.bottom = params->box.bottom;

		// Bail out if there isn't overlap between the clipping rects...
		if (!findRectOverlap(&clipRect, &workClipRect)) {
			return;
		}
	}

	// Get the rendering coords or assume the entire...
	if (params->actionFlags & kWAFRenderCoords) {
		renderRect = params->renderCoords;
	} else {
		renderRect = clipRect;
	}

	// What is the rendering color?
	if (params->actionFlags & kWAFColor) {
		whatColor = params->colorValue;
	} else {
		whatColor = _vm->VAR(_vm->VAR_COLOR_BLACK);
	}

	// Get the simple bitmap...
	if (!dwSetSimpleBitmapStructFromImage(whichImage, whichState, &renderBitmap)) {
		error("Wiz::processWizImageRenderRectCmd(): Image %d state %d invalid for rendering", whichImage, whichState);
	}

	// If we're here we must be able to render into the image (clipped)...
	if (findRectOverlap(&renderRect, &clipRect)) {
		pgDrawSolidRect(&renderBitmap, &renderRect, whatColor);
		_vm->_res->setModified(rtImage, params->image);
	}
}

void Wiz::processWizImageRenderLineCmd(const WizImageCommand *params) {
	Common::Rect clipRect, workClipRect;
	int whichState, whichImage;
	int32 w, h;
	WizSimpleBitmap renderBitmap;
	WizRawPixel whatColor;
	int propertyNumber = 0, propertyValue = 0;

	if (!(params->actionFlags & kWAFRenderCoords)) {
		return;
	}

	if (params->actionFlags & kWAFState) {
		whichState = params->state;
	} else {
		whichState = 0;
	}

	if (_vm->_game.heversion > 99 || _vm->_isHE995) {
		if (params->actionFlags & kWAFProperty) {
			propertyNumber = params->propertyNumber;
			propertyValue = params->propertyValue;
		}
	}

	whichImage = params->image;

	getWizImageDim(whichImage, whichState, w, h);
	makeSizedRectAt(&clipRect, 0, 0, w, h);

	if (params->actionFlags & kWAFRect) {
		workClipRect.left = params->box.left;
		workClipRect.top = params->box.top;
		workClipRect.right = params->box.right;
		workClipRect.bottom = params->box.bottom;

		// Bail out if there isn't overlap between the clipping rects...
		if (!findRectOverlap(&clipRect, &workClipRect)) {
			return;
		}
	}

	// What is the rendering color?
	if (params->actionFlags & kWAFColor) {
		whatColor = params->colorValue;
	} else {
		whatColor = _vm->VAR(_vm->VAR_COLOR_BLACK);
	}

	// Get the simple bitmap...
	if (!dwSetSimpleBitmapStructFromImage(whichImage, whichState, &renderBitmap)) {
		error("Wiz::processWizImageRenderLineCmd(): Image %d state %d invalid for rendering", whichImage, whichState);
	}

	// If we're here we must be able to render into the image (clipped)...
	if (_vm->_game.heversion > 99 || _vm->_isHE995) {
		switch (propertyNumber) {
		case 0:
			pgClippedLineDraw(
				&renderBitmap,
				params->renderCoords.left, params->renderCoords.top,
				params->renderCoords.right, params->renderCoords.bottom,
				&clipRect, whatColor);

			break;
		case 1:
			pgClippedThickLineDraw(
				&renderBitmap,
				params->renderCoords.left, params->renderCoords.top,
				params->renderCoords.right, params->renderCoords.bottom,
				&clipRect,
				propertyValue,
				whatColor);

			break;
		}

		_vm->_res->setModified(rtImage, params->image);
	} else {
		pgClippedLineDraw(
			&renderBitmap,
			params->renderCoords.left, params->renderCoords.top,
			params->renderCoords.right, params->renderCoords.bottom,
			&clipRect, whatColor);
	}
}

void Wiz::processWizImageRenderPixelCmd(const WizImageCommand *params) {
	Common::Rect clipRect, workClipRect;
	int whichState, whichImage;
	int32 w, h;
	WizSimpleBitmap renderBitmap;
	WizRawPixel whatColor;
	Common::Point pt;

	if (params->actionFlags & kWAFRenderCoords) {
		pt.x = params->renderCoords.left;
		pt.y = params->renderCoords.top;
	} else {
		return;
	}

	if (params->actionFlags & kWAFState) {
		whichState = params->state;
	} else {
		whichState = 0;
	}

	whichImage = params->image;

	getWizImageDim(whichImage, whichState, w, h);
	makeSizedRectAt(&clipRect, 0, 0, w, h);

	if (params->actionFlags & kWAFRect) {
		workClipRect.left = params->box.left;
		workClipRect.top = params->box.top;
		workClipRect.right = params->box.right;
		workClipRect.bottom = params->box.bottom;

		// Bail out if there isn't overlap between the clipping rects...
		if (!findRectOverlap(&clipRect, &workClipRect)) {
			return;
		}
	}

	if (params->actionFlags & kWAFColor) {
		whatColor = params->colorValue;
	} else {
		whatColor = _vm->VAR(_vm->VAR_COLOR_BLACK);
	}

	if (!dwSetSimpleBitmapStructFromImage(whichImage, whichState, &renderBitmap)) {
		error("Wiz::processWizImageRenderPixelCmd(): Image %d state %d invalid for rendering.", whichImage, whichState);
	}

	if (isPointInRect(&clipRect,&pt)) {
		pgWritePixel(&renderBitmap, pt.x, pt.y, whatColor);
		_vm->_res->setModified(rtImage, params->image);
	}
}

void Wiz::remapImagePrim(int image, int state, int tableCount, const uint8 *remapList, const uint8 *remapTable) {
	int index;
	byte *tablePtr;
	byte *basePtr;

	// Find the table...
	basePtr = getWizStateRemapDataPrim(image, state);
	assert(basePtr);

	// Set the modified bit...
	_vm->_res->setModified(rtImage, image);
	WRITE_BE_UINT32(basePtr + _vm->_resourceHeaderSize, WIZ_MAGIC_REMAP_NUMBER);
	tablePtr = basePtr + _vm->_resourceHeaderSize + 4;

	for (int i = 0; i < tableCount; i++) {
		index = *remapList++;
		tablePtr[index] = remapTable[index];
	}
}

int Wiz::createHistogramArrayForImage(int image, int state, const Common::Rect *optionalClipRect) {
	int src_c, src_w, src_h, globNum;
	Common::Rect realClippedRect;
	int histogramTable[256];
	byte *src_d;
	byte *pp;

	globNum = image;

	_vm->writeVar(0, 0);
	((ScummEngine_v72he *)_vm)->defineArray(0, _vm->kDwordArray, 0, 0, 0, 255);

	if (_vm->readVar(0) != 0) {
		// Get the header (width, height + compression)...
		pp = getWizStateHeaderPrim(globNum, state);

		src_c = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 0);
		src_w = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 4);
		src_h = READ_LE_UINT32(pp + _vm->_resourceHeaderSize + 8);

		// Clip the passed in coords to the real dimensions of the image...
		makeSizedRect(&realClippedRect, src_w, src_h);

		if (optionalClipRect) {
			if (!findRectOverlap(&realClippedRect, optionalClipRect)) {
				return _vm->readVar(0);
			}
		}

		// Get the data pointer...
		src_d = getWizStateDataPrim(globNum, state);

		// Start with a clean array...
		memset(histogramTable, 0, sizeof(histogramTable));

		// Handle the different compression types...
		if (src_c == kWCTTRLE) {
			// Get the histogram...
			auxHistogramTRLEPrim(histogramTable, src_d + _vm->_resourceHeaderSize, &realClippedRect);
		} else if (src_c == kWCTNone) {
			WizSimpleBitmap srcBitmap;

			srcBitmap.bufferPtr = WizPxShrdBuffer(src_d + _vm->_resourceHeaderSize, false);
			srcBitmap.bitmapWidth = src_w;
			srcBitmap.bitmapHeight = src_h;

			pgHistogramBitmapSubRect(histogramTable, &srcBitmap, &realClippedRect);
		} else {
			warning("Wiz::createHistogramArrayForImage(): Unable to return histogram for type %d", src_c);
		}

		// Fill in the scumm array with the values...
		if (_vm->readVar(0) != 0) {
			for (int i = 0; i < 256; i++) {
				((ScummEngine_v72he *)_vm)->writeArray(0, 0, i, histogramTable[i]);
			}
		}
	}

	return _vm->readVar(0);
}

// This function is currently deactivated until it can be assessed
// that it doesn't do more damage than good. Currently it does more
// damage (e.g. the images get re-byteswapped on every frame)...
void Wiz::ensureNativeFormatImageForState(int image, int state) {
#if 0
	// If AWIZ block is an XMAP, we don't want to do anything with it...
	if (dwGetImageGeneralProperty(image, state, kWIPXMAPBlockPresent)) {
		return;
	}

	int compType = getWizCompressionType(image, state);
	bool wiz16bpp = WIZ_16BPP(compType);
	bool native = NATIVE_WIZ_TYPE(compType);

	if (wiz16bpp && !native) {
		uint16 *ptr = (uint16 *)getWizStateDataPrim(image, state);

		int32 w, h;
		getWizImageDim(image, state, w, h);
		int32 pixelCount = w * h; // Number of pixels to twiddle

		if (pixelCount <= 0) {
			error("Width or height 0 for image %d state %d", image, state);
		}

		uint16 *thisPixel = ptr + 2; // Skip the "WIZD" header

		for (int i = 0; i < pixelCount; i++) {
			thisPixel[i] = SWAP_BYTES_16(thisPixel[i]);
		}

		int newCompType = compType;
		switch (compType) {
		// These were in the original but they appear to be dead code.
		// Not removing these just yet...
		// 
		// case kWCTNone16Bpp:
		// 	newCompType += kWCTNone16BppBigEndian - kWCTNone16Bpp;
		// 	break;
		// case kWCTTRLE16Bpp:
		// 	newCompType += kWCTTRLE16BppBigEndian - kWCTTRLE16Bpp;
		// 	break;
		case kWCTNone16BppBigEndian:
			newCompType -= kWCTNone16BppBigEndian - kWCTNone16Bpp;
			break;
		case kWCTTRLE16BppBigEndian:
			newCompType -= kWCTTRLE16BppBigEndian - kWCTTRLE16Bpp;
			break;
		}

		// Reset the compression type...
		setWizCompressionType(image, state, newCompType);
	}
#endif
}

void Wiz::processWizImageModifyCmd(const WizImageCommand *params) {
	int state;

	if (params->actionFlags & kWAFState) {
		state = params->state;
	} else {
		state = 0;
	}

	if (params->actionFlags & kWAFRemapList) {
		remapImagePrim(params->image, state, params->remapCount, params->remapList, params->remapTable);
	}
}

void Wiz::processWizImageRenderEllipseCmd(const WizImageCommand *params) {
	int whichState = 0, propertyValue = 0;
	int32 width = 0, height = 0;

	if (params->actionFlags & kWAFProperty) {
		propertyValue = params->propertyValue;
	}

	// What state is going to rendered into?
	if (params->actionFlags & kWAFState) {
		whichState = params->state;
	}

	int whichImage = params->image;
	
	// Make the clipping rect for this image / state...
	getWizImageDim(whichImage, whichState, width, height);

	Common::Rect clipRect;
	makeSizedRectAt(&clipRect, 0, 0, width, height);

	// Get the simple bitmap...
	WizSimpleBitmap renderBitmap;

	if (!dwSetSimpleBitmapStructFromImage(whichImage, whichState, &renderBitmap)) {
		error("Wiz::processWizImageRenderEllipseCmd(): Image %d state %d invalid for rendering.", whichImage, whichState);
	}

	pgDrawClippedEllipse(&renderBitmap,
						 params->ellipseProperties.px, params->ellipseProperties.py,
						 params->ellipseProperties.qx, params->ellipseProperties.qy,
						 params->ellipseProperties.kx, params->ellipseProperties.ky,
						 params->ellipseProperties.lod,
						 &clipRect,
						 propertyValue,
						 params->ellipseProperties.color);

	_vm->_res->setModified(rtImage, params->image);
}

void Wiz::processWizImageFontStartCmd(const WizImageCommand *params) {
	// Used for text in FreddisFunShop/PuttsFunShop/SamsFunShop
	// TODO: Start Font
}

void Wiz::processWizImageFontEndCmd(const WizImageCommand *params) {
	// Used for text in FreddisFunShop/PuttsFunShop/SamsFunShop
	// TODO: End Font
}

void Wiz::processWizImageFontCreateCmd(const WizImageCommand *params) {
	// Used for text in FreddisFunShop/PuttsFunShop/SamsFunShop
	// TODO: Create Font
}

void Wiz::processWizImageFontRenderCmd(const WizImageCommand *params) {
	// TODO: Render Font String
	error("Wiz::processWizImageFontRenderCmd(): Render Font String");
}

void Wiz::processWizImageRenderFloodFillCmd(const WizImageCommand *params) {
	Common::Rect renderRect, clipRect, workClipRect;
	int whichState, whichImage;
	int32 w, h;
	WizSimpleBitmap renderBitmap;
	WizRawPixel whatColor;
	Common::Point pt;

	// Get the rendering coords or bail if none...
	if (params->actionFlags & kWAFRenderCoords) {
		pt.x = params->renderCoords.left;
		pt.y = params->renderCoords.top;
	} else {
		return;
	}

	// What state is going to rendered into?
	if (params->actionFlags & kWAFState) {
		whichState = params->state;
	} else {
		whichState = 0;
	}

	whichImage = params->image;

	// Make the clipping rect for this image / state...
	getWizImageDim(whichImage, whichState, w, h);
	makeSizedRectAt(&clipRect, 0, 0, w, h);

	if (params->actionFlags & kWAFRect) {
		workClipRect.left = params->box.left;
		workClipRect.top = params->box.top;
		workClipRect.right = params->box.right;
		workClipRect.bottom = params->box.bottom;

		// Bail out if there isn't overlap between the clipping rects...
		if (!findRectOverlap(&clipRect, &workClipRect)) {
			return;
		}
	}

	// What is the rendering color?
	if (params->actionFlags & kWAFColor) {
		whatColor = params->colorValue;
	} else {
		whatColor = _vm->VAR(_vm->VAR_COLOR_BLACK);
	}

	// Get the simple bitmap...
	if (!dwSetSimpleBitmapStructFromImage(whichImage, whichState, &renderBitmap)) {
		error("Image %d state %d invalid for rendering.", whichImage, whichState);
	}

	// If we're here we must be able to render into the image (clipped)...
	if (isPointInRect(&clipRect, &pt)) {
		floodSimpleFill(&renderBitmap, pt.x, pt.y, whatColor, &clipRect, &renderRect);
		_vm->_res->setModified(rtImage, params->image);
	}
}

void Wiz::processNewWizImageCmd(const WizImageCommand *params) {
	int width, height;
	int propertyNumber = 0, propertyValue = 0;
	int hotspotX, hotspotY;

	if (params->actionFlags & kWAFWidth) {
		width = params->width;
	} else {
		width = 640;
	}

	if (params->actionFlags & kWAFHeight) {
		height = params->height;
	} else {
		height = 480;
	}

	if (params->actionFlags & kWAFSpot) {
		hotspotX = params->xPos;
		hotspotY = params->yPos;
	} else {
		hotspotX = 0;
		hotspotY = 0;
	}


	int pixelDepth = (_vm->_game.features & GF_16BIT_COLOR) ? 16 : 8;
	if (_vm->_game.heversion > 99 || _vm->_isHE995) {
		// Determine pixel depth...
		if (params->actionFlags & kWAFProperty) {
			propertyNumber = params->propertyNumber;
			propertyValue = params->propertyValue;
		}

		if (propertyNumber == 1) { // Color hint property
			pixelDepth = propertyValue;

			if ((pixelDepth != 16) && (pixelDepth != 8)) {
				error("Wiz::processNewWizImageCmd(): The only pixel depths supported for a new image are 16 and 8. You picked %d.", pixelDepth);
			}
		}
	}

	dwCreateRawWiz(params->image, width, height, kCWFDefault, pixelDepth, hotspotX, hotspotY);
	_vm->_res->setModified(rtImage, params->image);
}

void Wiz::processWizImageLoadCmd(const WizImageCommand *params) {
	Common::SeekableReadStream *inFile;
	int result;

	if (params->actionFlags & kWAFFilename) {
		inFile = _vm->openFileForReading(params->filename);

		if (!inFile) {
			_vm->VAR(_vm->VAR_GAME_LOADED) = DW_LOAD_OPEN_FAILURE;
			_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_LOAD_OPEN_FAILURE;

			debug(0, "Wiz::processWizImageLoadCmd(): Unable to open for read '%s'", params->filename);
			return;
		}

		result = dwTryToLoadWiz(inFile, params);
		_vm->VAR(_vm->VAR_GAME_LOADED) = result;
		_vm->VAR(_vm->VAR_OPERATION_FAILURE) = result;

		if (result == DW_LOAD_SUCCESS) {
			debug(7, "Wiz::processWizImageLoadCmd(): Correctly loaded file '%s'", params->filename);
		} else if (result == DW_LOAD_READ_FAILURE) {
			debug(0, "Wiz::processWizImageLoadCmd(): Got DW_LOAD_READ_FAILURE for file '%s'", params->filename);
		}

		delete inFile;
	}
}

void Wiz::processWizImageSaveCmd(const WizImageCommand *params) {
	if (params->actionFlags & kWAFFilename) {
		if (_vm->_game.heversion <= 99 && !_vm->_isHE995) {
			// Validate the type
			if (params->fileType != DW_SAVE_WIZ_FORMAT) {
				_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_NOT_TYPE;
				return;
			}

			// Open the file
			Common::WriteStream *f = _vm->openSaveFileForWriting(params->filename);

			if (!f) {
				debug(0, "Unable to open for write '%s'", params->filename);
				_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_CREATE_FAILURE;
				return;
			}

			// Get the data size and save out the glob.
			byte *p = _vm->getResourceAddress(rtImage, params->image);
			uint32 dataSize = READ_BE_UINT32(p + 4);

			if (f->write(p, dataSize) == dataSize) {
				_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_SUCCESS;
			} else {
				_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_WRITE_FAILURE;
			}

			f->finalize();
			delete f;
		} else {
			switch (params->fileType) {
			case DW_SAVE_RAW_FORMAT:
				// Ignore on purpose...
				_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_NOT_TYPE;
				break;
			case DW_SAVE_PCX_FORMAT:
				// TODO Write image to file
				break;
			case DW_SAVE_WIZ_FORMAT: {
				Common::WriteStream *f = _vm->openSaveFileForWriting(params->filename);

				if (!f) {
					debug(0, "Unable to open for write '%s'", params->filename);
					_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_CREATE_FAILURE;
				} else {
					byte *p = _vm->getResourceAddress(rtImage, params->image);
					uint32 size = READ_BE_UINT32(p + 4);

					if (f->write(p, size) != size) {
						error("i/o error when writing '%s'", params->filename);
						_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_WRITE_FAILURE;
					} else {
						_vm->VAR(_vm->VAR_OPERATION_FAILURE) = DW_SAVE_SUCCESS;
					}

					f->finalize();
					delete f;
				}

				break;
			}
			default:
				error("processWizImageCmd: actionType kWASave unhandled fileType %d", params->fileType);
			}
		}
	}
}

void Wiz::processWizImageCmd(const WizImageCommand *params) {
	if (((ScummEngine_v90he *)_vm)->_logicHE && ((ScummEngine_v90he *)_vm)->_logicHE->userCodeProcessWizImageCmd(params)) {
		return;
	}

	switch (params->actionType) {
	case kWAUnknown:
		// Do nothing...
		break;
	case kWADraw:
		processWizImageDrawCmd(params);
		break;
	case kWACapture:
		processWizImageCaptureCmd(params);
		break;
	case kWALoad:
		processWizImageLoadCmd(params);
		break;
	case kWASave:
		processWizImageSaveCmd(params);
		break;
	case kWAGlobalState:
		// Do nothing...
		break;
	case kWAModify:
		processWizImageModifyCmd(params);
		break;
	// HE 99+
	case kWAPolyCapture:
		processWizImagePolyCaptureCmd(params);
		break;
	case kWANew:
		processNewWizImageCmd(params);
		break;
	case kWARenderRectangle:
		processWizImageRenderRectCmd(params);
		break;
	case kWARenderLine:
		processWizImageRenderLineCmd(params);
		break;
	case kWARenderPixel:
		processWizImageRenderPixelCmd(params);
		break;
	case kWARenderFloodFill:
		processWizImageRenderFloodFillCmd(params);
		break;
	case kWAFontStart:
		processWizImageFontStartCmd(params);
		break;
	case kWAFontEnd:
		processWizImageFontEndCmd(params);
		break;
	case kWAFontCreate:
		processWizImageFontCreateCmd(params);
		break;
	case kWAFontRender:
		processWizImageFontRenderCmd(params);
		break;
	case kWARenderEllipse:
		processWizImageRenderEllipseCmd(params);
		break;
	default:
		error("Wiz::processWizImageCmd(): Unhandled processWizImageCmd mode %d", params->actionType);
	}
}

bool Wiz::isUncompressedFormatTypeID(int id) {
	return ((kWCTNone == id) || (kWCTNone16Bpp == id) || (kWCTNone16BppBigEndian == id));
}

void Wiz::getWizImageDim(int resNum, int state, int32 &w, int32 &h) {
	uint8 *dataPtr = _vm->getResourceAddress(rtImage, resNum);
	assert(dataPtr);
	getWizImageDim(dataPtr, state, w, h);
}

void Wiz::getWizImageDim(uint8 *dataPtr, int state, int32 &w, int32 &h) {
	uint8 *wizh = _vm->findWrappedBlock(MKTAG('W','I','Z','H'), dataPtr, state, 0);
	assert(wizh);
	w = READ_LE_UINT32(wizh + 0x4);
	h = READ_LE_UINT32(wizh + 0x8);
}

void Wiz::getWizSpot(int resId, int state, int32 &x, int32 &y) {
	uint8 *dataPtr = _vm->getResourceAddress(rtImage, resId);
	assert(dataPtr);
	getWizImageSpot(dataPtr, state, x, y);
}

void Wiz::getWizSpot(int resId, int32 &x, int32 &y) {
	uint8 *dataPtr = _vm->getResourceAddress(rtImage, resId);
	assert(dataPtr);
	const uint8 *spotPtr = _vm->findResourceData(MKTAG('S', 'P', 'O', 'T'), dataPtr);

	if (!spotPtr) {
		x = y = 0;
	} else {
		x = READ_LE_UINT32(spotPtr + 0x0);
		y = READ_LE_UINT32(spotPtr + 0x4);
	}
}

void Wiz::getWizImageSpot(uint8 *dataPtr, int state, int32 &x, int32 &y) {
	uint8 *spotPtr = _vm->findWrappedBlock(MKTAG('S','P','O','T'), dataPtr, state, 0);
	if (spotPtr) {
		x = READ_LE_UINT32(spotPtr + 0);
		y = READ_LE_UINT32(spotPtr + 4);
	} else {
		x = 0;
		y = 0;
	}
}

int Wiz::getWizStateCount(int resNum) {
	const uint8 *dataPtr = _vm->getResourceAddress(rtImage, resNum);
	assert(dataPtr);

	return getWizImageStates(dataPtr);
}

int Wiz::getWizImageStates(const uint8 *dataPtr) {
	if (READ_BE_UINT32(dataPtr) == MKTAG('M','U','L','T')) {
		const byte *offs, *wrap;

		wrap = _vm->findResource(MKTAG('W','R','A','P'), dataPtr);
		if (wrap == nullptr)
			return 1;

		offs = _vm->findResourceData(MKTAG('O','F','F','S'), wrap);
		if (offs == nullptr)
			return 1;

		return _vm->getResourceDataSize(offs) / 4;
	} else {
		return 1;
	}
}

byte *Wiz::getWizStateHeaderPrim(int resNum, int state) {
	byte *data = _vm->getResourceAddress(rtImage, resNum);
	assert(data);
	return _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), data, state, false) - _vm->_resourceHeaderSize;
}

byte *Wiz::getWizStateDataPrim(int resNum, int state) {
	byte *data = _vm->getResourceAddress(rtImage, resNum);
	assert(data);
	return _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'D'), data, state, false) - _vm->_resourceHeaderSize;
}

byte *Wiz::getWizStatePaletteDataPrim(int globNum, int state) {
	byte *data = _vm->getResourceAddress(rtImage, globNum);
	assert(data);
	return _vm->findWrappedBlock(MKTAG('R', 'G', 'B', 'S'), data, state, false) - _vm->_resourceHeaderSize;
}

byte *Wiz::getWizStateRemapDataPrim(int globNum, int state) {
	byte *data = _vm->getResourceAddress(rtImage, globNum);
	assert(data);
	return _vm->findWrappedBlock(MKTAG('R', 'M', 'A', 'P'), data, state, false) - _vm->_resourceHeaderSize;
}

const byte *Wiz::getColorMixBlockPtrForWiz(int image) {
	byte *data = _vm->getResourceAddress(rtImage, image);
	assert(data);
	return _vm->findResourceData(MKTAG('X', 'M', 'A', 'P'), data) - _vm->_resourceHeaderSize;
}

void Wiz::setWizCompressionType(int image, int state, int newType) {
	byte *data = getWizStateHeaderPrim(image, state);
	assert(data);

	WRITE_LE_UINT32(data + _vm->_resourceHeaderSize, newType);
}

int Wiz::getWizCompressionType(int image, int state) {
	byte *data = (byte *)getWizStateHeaderPrim(image, state);
	assert(data);

	return READ_LE_UINT32(data + _vm->_resourceHeaderSize);
}

bool Wiz::doesRawWizStateHaveTransparency(int globNum, int state) {
	byte *data = _vm->getResourceAddress(rtImage, globNum);
	assert(data);

	// Yes, this has to return "true" if the block is not available :-)
	return _vm->findWrappedBlock(MKTAG('T', 'R', 'N', 'S'), data, state, false) == nullptr;
}

bool Wiz::doesStateContainBlock(int globNum, int state, uint32 blockID) {
	byte *data = _vm->getResourceAddress(rtImage, globNum);
	assert(data);

	return _vm->findWrappedBlock(blockID, data, state, false) != nullptr;
}

bool Wiz::collisionCompareImageLines(
	const byte *imageAData, int aType, int aw, int ah, int32 wizAFlags, int ax, int ay,
	const byte *imageBData, int bType, int bw, int bh, int32 wizBFlags, int bx, int by,
	int compareWidth, WizRawPixel transparentColor) {

	// Get line A's data...
	rawPixelMemset(_compareBufferA, transparentColor, compareWidth);

	if (aType == kWCTTRLE) {
		trleFLIPDecompressImage(
			_compareBufferA, imageAData, compareWidth, 1,
			-ax, -ay, aw, ah, nullptr, wizAFlags, nullptr,
			(WizRawPixel *)_vm->getHEPaletteSlot(1),
			nullptr);
	} else {
		pgDrawRawDataFormatImage(
			_compareBufferA, (const WizRawPixel *)imageAData, compareWidth, 1,
			-ax, -ay, aw, ah, nullptr, wizAFlags, nullptr,
			transparentColor);
	}

	// Get line B's data...
	rawPixelMemset(_compareBufferB, transparentColor, compareWidth);

	if (bType == kWCTTRLE) {
		trleFLIPDecompressImage(
			_compareBufferB, imageBData, compareWidth, 1,
			-bx, -by, bw, bh, nullptr, wizBFlags, nullptr,
			(WizRawPixel *)_vm->getHEPaletteSlot(1),
			nullptr);
	} else {
		pgDrawRawDataFormatImage(
			_compareBufferB, (const WizRawPixel *)imageBData, compareWidth, 1,
			-bx, -by, bw, bh, nullptr, wizBFlags, nullptr,
			transparentColor);
	}

	// Finally compare the lines...
	if (compareDoPixelStreamsOverlap(_compareBufferA, _compareBufferB, compareWidth, transparentColor)) {
		return true;
	}

	return false;
}

} // End of namespace Scumm

#endif // ENABLE_HE
