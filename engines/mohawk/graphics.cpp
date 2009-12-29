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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "mohawk/file.h"
#include "mohawk/graphics.h"
#include "mohawk/myst.h"
#include "mohawk/riven.h"
#include "mohawk/riven_cursors.h"

#include "graphics/cursorman.h"
#include "graphics/primitives.h"
#include "gui/message.h"

namespace Mohawk {

Graphics::Surface *ImageData::getSurface() {
	Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_surface->w, _surface->h, pixelFormat.bytesPerPixel);
	
	if (_surface->bytesPerPixel == 1) {
		assert(_palette);

		for (uint16 i = 0; i < _surface->h; i++) {
			for (uint16 j = 0; j < _surface->w; j++) {
				byte palIndex = *((byte *)_surface->pixels + i * _surface->pitch + j);
				byte r = _palette[palIndex * 4];
				byte g = _palette[palIndex * 4 + 1];
				byte b = _palette[palIndex * 4 + 2];
				if (pixelFormat.bytesPerPixel == 2)
					*((uint16 *)surface->getBasePtr(j, i)) = pixelFormat.RGBToColor(r, g, b);
				else
					*((uint32 *)surface->getBasePtr(j, i)) = pixelFormat.RGBToColor(r, g, b);
			}
		}
	} else
		memcpy(surface->pixels, _surface->pixels, _surface->w * _surface->h * _surface->bytesPerPixel);
	
	return surface;
}

MystGraphics::MystGraphics(MohawkEngine_Myst* vm) : _vm(vm) {
	_bmpDecoder = new MystBitmap();
	
	// The original version of Myst could run in 8bpp color too.
	// However, it dithered videos to 8bpp and they looked considerably
	// worse (than they already did :P). So we're not even going to
	// support 8bpp mode in Myst (Myst ME required >8bpp anyway).
	initGraphics(544, 333, true, NULL); // What an odd screen size!
	
	_pixelFormat = _vm->_system->getScreenFormat();
	
	if (_pixelFormat.bytesPerPixel == 1)
		error("Myst requires greater than 256 colors to run");
	
	if (_vm->getFeatures() & GF_ME) {		
		_jpegDecoder = new MystJPEG();
		_pictDecoder = new MystPICT(_jpegDecoder);
	} else {
		_jpegDecoder = NULL;
		_pictDecoder = NULL;
	}
	
	_pictureFile.entries = NULL;
}

MystGraphics::~MystGraphics() {
	delete _bmpDecoder;
	delete _jpegDecoder;
	delete _pictDecoder;
}

static const char* picFileNames[] = {
	"CHpics",
	"",
	"DUpics",
	"INpics",
	"MEpics",
	"MYpics",
	"SEpics",
	"STpics",
	""
};

void MystGraphics::loadExternalPictureFile(uint16 stack) {
	if (_vm->getPlatform() != Common::kPlatformMacintosh)
		return;
		
	if (_pictureFile.picFile.isOpen())
		_pictureFile.picFile.close();
	delete[] _pictureFile.entries;
		
	if (!scumm_stricmp(picFileNames[stack], ""))
		return;
	
	if (!_pictureFile.picFile.open(picFileNames[stack]))
		error ("Could not open external picture file \'%s\'", picFileNames[stack]);
		
	_pictureFile.pictureCount = _pictureFile.picFile.readUint32BE();
	_pictureFile.entries = new PictureFile::PictureEntry[_pictureFile.pictureCount];
	
	for (uint32 i = 0; i < _pictureFile.pictureCount; i++) {
		_pictureFile.entries[i].offset = _pictureFile.picFile.readUint32BE();
		_pictureFile.entries[i].size = _pictureFile.picFile.readUint32BE();
		_pictureFile.entries[i].id = _pictureFile.picFile.readUint16BE();
		_pictureFile.entries[i].type = _pictureFile.picFile.readUint16BE();
		_pictureFile.entries[i].width = _pictureFile.picFile.readUint16BE();
		_pictureFile.entries[i].height = _pictureFile.picFile.readUint16BE();
	}
}

void MystGraphics::copyImageSectionToScreen(uint16 image, Common::Rect src, Common::Rect dest) {
	// Clip the destination rect to the screen
	if (dest.right > _vm->_system->getWidth() || dest.bottom > _vm->_system->getHeight())
		dest.debugPrint(4, "Clipping destination rect to the screen:");

	dest.right = CLIP<int>(dest.right, 0, _vm->_system->getWidth());
	dest.bottom = CLIP<int>(dest.bottom, 0, _vm->_system->getHeight());
	
	Graphics::Surface *surface = NULL;


	// Myst ME uses JPEG/PICT images instead of compressed Windows Bitmaps for room images,
	// though there are a few weird ones that use that format. For further nonsense with images,
	// the Macintosh version stores images in external "picture files." We check them before
	// going to check for a PICT resource.
	if (_vm->getFeatures() & GF_ME && _vm->getPlatform() == Common::kPlatformMacintosh && _pictureFile.picFile.isOpen()) {
		for (uint32 i = 0; i < _pictureFile.pictureCount; i++)
			if (_pictureFile.entries[i].id == image) {
				if (_pictureFile.entries[i].type == 0)
					surface = _jpegDecoder->decodeImage(new Common::SeekableSubReadStream(&_pictureFile.picFile, _pictureFile.entries[i].offset, _pictureFile.entries[i].offset + _pictureFile.entries[i].size));
				else if (_pictureFile.entries[i].type == 1)
					surface = _pictDecoder->decodeImage(new Common::SeekableSubReadStream(&_pictureFile.picFile, _pictureFile.entries[i].offset, _pictureFile.entries[i].offset + _pictureFile.entries[i].size));
				else
					error ("Unknown Picture File type %d", _pictureFile.entries[i].type);
				break;
			}
	}

	// We're not using the external Mac files, so it's time to delve into the main Mohawk
	// archives. However, we still don't know if it's a PICT or WDIB resource. If it's Myst
	// ME it's most likely a PICT, and if it's original it's definitely a WDIB. However,
	// Myst ME throws us another curve ball in that PICT resources can contain WDIB's instead
	// of PICT's.
	if (!surface) {
		bool isPict = false;
		Common::SeekableReadStream *dataStream = NULL;

		if (_vm->getFeatures() & GF_ME && _vm->hasResource(ID_PICT, image)) {
			// The PICT resource exists. However, it could still contain a MystBitmap
			// instead of a PICT image...
			dataStream = _vm->getRawData(ID_PICT, image);

			// Here we detect whether it's really a PICT or a WDIB. Since a MystBitmap
			// would be compressed, there's no way to detect for the BM without a hack.
			// So, we search for the PICT version opcode for detection.
			dataStream->seek(512 + 10); // 512 byte pict header
			isPict = (dataStream->readUint32BE() == 0x001102FF);
			dataStream->seek(0);
		} else // No PICT, so the WDIB must exist. Let's go grab it.
			dataStream = _vm->getRawData(ID_WDIB, image);

		if (isPict)
			surface = _pictDecoder->decodeImage(dataStream);
		else {
			ImageData *imageData = _bmpDecoder->decodeImage(dataStream);
			surface = imageData->getSurface();
			delete imageData;
		}
	}
	
	debug(3, "Image Blit:");
	debug(3, "src.x: %d", src.left); 
	debug(3, "src.y: %d", src.top);
	debug(3, "dest.x: %d", dest.left);
	debug(3, "dest.y: %d", dest.top);
	debug(3, "width: %d", src.width());
	debug(3, "height: %d", src.height());
	
	if (surface) {
		uint16 width = MIN<int>(surface->w, dest.width());
		uint16 height = MIN<int>(surface->h, dest.height());
		_vm->_system->copyRectToScreen((byte *)surface->getBasePtr(src.left, src.top), surface->pitch, dest.left, dest.top, width, height);
		surface->free();
		delete surface;
	}
	
	// FIXME: Remove this and update only at certain points
	_vm->_system->updateScreen();
}

void MystGraphics::copyImageToScreen(uint16 image, Common::Rect dest) {
	copyImageSectionToScreen(image, Common::Rect(0, 0, 544, 333), dest);
}

void MystGraphics::showCursor(void) {
	CursorMan.showMouse(true);
	_vm->_needsUpdate = true;
}

void MystGraphics::hideCursor(void) {
	CursorMan.showMouse(false);
	_vm->_needsUpdate = true;
}

void MystGraphics::changeCursor(uint16 cursor) {
	// Both Myst and Myst ME use the "MystBitmap" format for cursor images.
	ImageData *data = _bmpDecoder->decodeImage(_vm->getRawData(ID_WDIB, cursor));
	Common::SeekableReadStream *clrcStream = _vm->getRawData(ID_CLRC, cursor);
	uint16 hotspotX = clrcStream->readUint16LE();
	uint16 hotspotY = clrcStream->readUint16LE();
	delete clrcStream;

	// Myst ME stores some cursors as 24bpp images instead of 8bpp
	if (data->_surface->bytesPerPixel == 1) {
		CursorMan.replaceCursor((byte *)data->_surface->pixels, data->_surface->w, data->_surface->h, hotspotX, hotspotY, 0);
		CursorMan.replaceCursorPalette(data->_palette, 0, 256);
	} else
		CursorMan.replaceCursor((byte *)data->_surface->pixels, data->_surface->w, data->_surface->h, hotspotX, hotspotY, 0xFFFFFFFF, 1, &_pixelFormat);

	_vm->_needsUpdate = true;
}

void MystGraphics::drawRect(Common::Rect rect, bool active) {
	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	if (rect.left < 0 || rect.top < 0 || rect.right > 544 || rect.bottom > 333 || !rect.isValidRect() || rect.width() == 0 || rect.height() == 0)
		return;

	Graphics::Surface *screen = _vm->_system->lockScreen();

	if (active)
		screen->frameRect(rect, _pixelFormat.RGBToColor(0, 255, 0));
	else
		screen->frameRect(rect, _pixelFormat.RGBToColor(255, 0, 0));
	
	_vm->_system->unlockScreen();
}

RivenGraphics::RivenGraphics(MohawkEngine_Riven* vm) : _vm(vm) {
	_bitmapDecoder = new MohawkBitmap();

	// Give me the best you've got!
	initGraphics(608, 436, true, NULL);
	_pixelFormat = _vm->_system->getScreenFormat();
	
	if (_pixelFormat.bytesPerPixel == 1)
		error("Riven requires greater than 256 colors to run");
	
	// The actual game graphics only take up the first 392 rows. The inventory
	// occupies the rest of the screen and we don't use the buffer to hold that.
	_mainScreen = new Graphics::Surface();
	_mainScreen->create(608, 392, _pixelFormat.bytesPerPixel);
	
	_updatesEnabled = true;
	_scheduledTransition = -1;	// no transition
	_dirtyScreen = false;
	_inventoryDrawn = false;
}

RivenGraphics::~RivenGraphics() {
	_mainScreen->free();
	delete _mainScreen;
	delete _bitmapDecoder;
}

void RivenGraphics::copyImageToScreen(uint16 image, uint32 left, uint32 top, uint32 right, uint32 bottom) {
	// First, decode the image and get the high color surface
	ImageData *imageData = _bitmapDecoder->decodeImage(_vm->getRawData(ID_TBMP, image));
	Graphics::Surface *surface = imageData->getSurface();
	delete imageData;
	
	// Clip the width to fit on the screen. Fixes some images.
	if (left + surface->w > 608)
		surface->w = 608 - left;
		
	for (uint16 i = 0; i < surface->h; i++)
		memcpy(_mainScreen->getBasePtr(left, i + top), surface->getBasePtr(0, i), surface->w * surface->bytesPerPixel);
		
	surface->free();
	delete surface;
	
	_dirtyScreen = true;
}

void RivenGraphics::drawPLST(uint16 x) {
	Common::SeekableReadStream* plst = _vm->getRawData(ID_PLST, _vm->getCurCard());
	uint16 index, id, left, top, right, bottom;
	uint16 recordCount = plst->readUint16BE();
	
	for (uint16 i = 0; i < recordCount; i++) {
		index = plst->readUint16BE();
		id = plst->readUint16BE();
		left = plst->readUint16BE();
		top = plst->readUint16BE();
		right = plst->readUint16BE();
		bottom = plst->readUint16BE();

		// We are also checking here to make sure we haven't drawn the image yet on screen.
		// This fixes problems with drawing PLST 1 twice and some other images twice. PLST
		// 1 is sometimes not called by the scripts, so some cards don't appear if we don't
		// draw PLST 1 each time. This "hack" is here to catch any PLST attempting to draw
		// twice. There should never be a problem with doing it this way.
		if (index == x && !(Common::find(_activatedPLSTs.begin(), _activatedPLSTs.end(), x) != _activatedPLSTs.end())) {
			debug (0, "Drawing image %d", id);
			copyImageToScreen(id, left, top, right, bottom);
			_activatedPLSTs.push_back(x);
			break;
		}
	}

	delete plst;
}

void RivenGraphics::updateScreen() {
	if (_updatesEnabled) {
		_vm->runUpdateScreenScript();
		
		if (_dirtyScreen) {
			_activatedPLSTs.clear();
			
			// Copy to screen if there's no transition. Otherwise transition. ;)
			if (_scheduledTransition < 0)
				_vm->_system->copyRectToScreen((byte *)_mainScreen->pixels, _mainScreen->pitch, 0, 0, _mainScreen->w, _mainScreen->h);
			else
				runScheduledTransition();
			
			// Finally, update the screen.
			_vm->_system->updateScreen();
			_dirtyScreen = false;
		}
	}
}

void RivenGraphics::scheduleWaterEffect(uint16 sfxeID) {
	Common::SeekableReadStream *sfxeStream = _vm->getRawData(ID_SFXE, sfxeID);

	if (sfxeStream->readUint16BE() != 'SL')
		error ("Unknown sfxe tag");
	
	// Read in header info
	SFXERecord sfxeRecord;
	sfxeRecord.frameCount = sfxeStream->readUint16BE();
	uint32 offsetTablePosition = sfxeStream->readUint32BE();
	sfxeRecord.rect.left = sfxeStream->readUint16BE();
	sfxeRecord.rect.top = sfxeStream->readUint16BE();
	sfxeRecord.rect.right = sfxeStream->readUint16BE();
	sfxeRecord.rect.bottom = sfxeStream->readUint16BE();
	sfxeRecord.speed = sfxeStream->readUint16BE();
	// Skip the rest of the fields...
	
	// Read in offsets
	sfxeStream->seek(offsetTablePosition);
	uint32 *frameOffsets = new uint32[sfxeRecord.frameCount];
	for (uint16 i = 0; i < sfxeRecord.frameCount; i++)
		frameOffsets[i] = sfxeStream->readUint32BE();
	sfxeStream->seek(frameOffsets[0]);
	
	// Read in the scripts
	for (uint16 i = 0; i < sfxeRecord.frameCount; i++) 
		sfxeRecord.frameScripts.push_back(sfxeStream->readStream((i == sfxeRecord.frameCount - 1) ? sfxeStream->size() - frameOffsets[i] : frameOffsets[i + 1] - frameOffsets[i]));
	
	// Set it to the first frame
	sfxeRecord.curFrame = 0;
	sfxeRecord.lastFrameTime = 0;
	
	delete[] frameOffsets;
	delete sfxeStream;
	_waterEffects.push_back(sfxeRecord);
}

void RivenGraphics::clearWaterEffects() {
	_waterEffects.clear();
}

bool RivenGraphics::runScheduledWaterEffects() {
	// Don't run the effect if it's disabled
	if (*_vm->matchVarToString("waterenabled") == 0)
		return false;

	Graphics::Surface *screen = NULL;
		
	for (uint16 i = 0; i < _waterEffects.size(); i++) {
		if (_vm->_system->getMillis() > _waterEffects[i].lastFrameTime + 1000 / _waterEffects[i].speed) {
			// Lock the screen!
			if (!screen)
				screen = _vm->_system->lockScreen();
				
			// Make sure the script is at the starting point
			Common::SeekableReadStream *script = _waterEffects[i].frameScripts[_waterEffects[i].curFrame];
			if (script->pos() != 0)
				script->seek(0);
				
			// Run script
			uint16 curRow = 0;
			for (uint16 op = script->readUint16BE(); op != 4; op = script->readUint16BE()) {
				if (op == 1) {        // Increment Row
					curRow++;
				} else if (op == 3) { // Copy Pixels
					uint16 dstLeft = script->readUint16BE();
					uint16 srcLeft = script->readUint16BE();
					uint16 srcTop = script->readUint16BE();
					uint16 rowWidth = script->readUint16BE();
					memcpy ((byte *)screen->getBasePtr(dstLeft, curRow + _waterEffects[i].rect.top), (byte *)_mainScreen->getBasePtr(srcLeft, srcTop), rowWidth * _pixelFormat.bytesPerPixel);
				} else if (op != 4) { // End of Script
					error ("Unknown SFXE opcode %d", op);
				}
			}
			
			// Increment frame
			_waterEffects[i].curFrame++;
			if (_waterEffects[i].curFrame == _waterEffects[i].frameCount)
				_waterEffects[i].curFrame = 0;
				
			// Set the new time
			_waterEffects[i].lastFrameTime = _vm->_system->getMillis();
		}
	}
	
	// Unlock the screen if it has been locked and return true to update the screen
	if (screen) {
		_vm->_system->unlockScreen();
		return true;
	}
	
	return false;
}

void RivenGraphics::scheduleTransition(uint16 id, Common::Rect rect) {
	_scheduledTransition = id;
	_transitionRect = rect;
}

void RivenGraphics::runScheduledTransition() {
	if (_scheduledTransition < 0) // No transition is scheduled
		return;
		
	// TODO: There's a lot to be done here...
	
	// Note: Transitions 0-11 are actual transitions, but none are used in-game.
	// There's no point in implementing them if they're not used. These extra
	// transitions were found by hacking scripts.

	switch (_scheduledTransition) {
		case 12: // Pan Left
			warning ("STUB: Pan left");
			break;
		case 13: // Pan Right
			warning ("STUB: Pan right");
			break;
		case 14: // Pan Up
			warning ("STUB: Pan up");
			break;
		case 15: // Pan Down
			warning ("STUB: Pan down");
			break;
		case 16: // Dissolve
		case 17: // Dissolve (tspit CARD 155)
			warning ("STUB: Dissolve");
			break;
		default:
			if (_scheduledTransition < 12)
				error ("Found unused transition %d", _scheduledTransition);
			else
				error ("Found unknown transition %d", _scheduledTransition);
	}
	
	// For now, just copy the image to screen without doing any transition.
	_vm->_system->copyRectToScreen((byte *)_mainScreen->pixels, _mainScreen->pitch, 0, 0, _mainScreen->w, _mainScreen->h);
	_vm->_system->updateScreen();

	_scheduledTransition = -1; // Clear scheduled transition
}

// TODO: Marble Cursors/Palettes
void RivenGraphics::changeCursor(uint16 num) {
	// All of Riven's cursors are hardcoded. See riven_cursors.h for these definitions.

	switch (num) {
		case 1002:
			// Zip Mode
			CursorMan.replaceCursor(zipModeCursor, 16, 16, 8, 8, 0);
			CursorMan.replaceCursorPalette(zipModeCursorPalette, 1, ARRAYSIZE(zipModeCursorPalette) / 4);
			break;
		case 2003:
			// Hand Over Object
			CursorMan.replaceCursor(objectHandCursor, 16, 16, 8, 8, 0);
			CursorMan.replaceCursorPalette(handCursorPalette, 1, ARRAYSIZE(handCursorPalette) / 4);
			break;
		case 2004:
			// Grabbing/Using Object
			CursorMan.replaceCursor(grabbingHandCursor, 13, 13, 6, 6, 0);
			CursorMan.replaceCursorPalette(handCursorPalette, 1, ARRAYSIZE(handCursorPalette) / 4);
			break;
		case 3000:
			// Standard Hand
			CursorMan.replaceCursor(standardHandCursor, 15, 16, 6, 0, 0);
			CursorMan.replaceCursorPalette(handCursorPalette, 1, ARRAYSIZE(handCursorPalette) / 4);
			break;
		case 3001:
			// Pointing Left
			CursorMan.replaceCursor(pointingLeftCursor, 15, 13, 0, 3, 0);
			CursorMan.replaceCursorPalette(handCursorPalette, 1, ARRAYSIZE(handCursorPalette) / 4);
			break;
		case 3002:
			// Pointing Right
			CursorMan.replaceCursor(pointingRightCursor, 15, 13, 14, 3, 0);
			CursorMan.replaceCursorPalette(handCursorPalette, 1, ARRAYSIZE(handCursorPalette) / 4);
			break;
		case 3003:
			// Pointing Down (Palm Up)
			CursorMan.replaceCursor(pointingDownCursorPalmUp, 13, 16, 3, 15, 0);
			CursorMan.replaceCursorPalette(handCursorPalette, 1, ARRAYSIZE(handCursorPalette) / 4);
			break;
		case 3004:
			// Pointing Up (Palm Up)
			CursorMan.replaceCursor(pointingUpCursorPalmUp, 13, 16, 3, 0, 0);
			CursorMan.replaceCursorPalette(handCursorPalette, 1, ARRAYSIZE(handCursorPalette) / 4);
			break;
		case 3005:
			// Pointing Left (Curved)
			CursorMan.replaceCursor(pointingLeftCursorBent, 15, 13, 0, 5, 0);
			CursorMan.replaceCursorPalette(handCursorPalette, 1, ARRAYSIZE(handCursorPalette) / 4);
			break;
		case 3006:
			// Pointing Right (Curved)
			CursorMan.replaceCursor(pointingRightCursorBent, 15, 13, 14, 5, 0);
			CursorMan.replaceCursorPalette(handCursorPalette, 1, ARRAYSIZE(handCursorPalette) / 4);
			break;
		case 3007:
			// Pointing Down (Palm Down)
			CursorMan.replaceCursor(pointingDownCursorPalmDown, 15, 16, 7, 15, 0);
			CursorMan.replaceCursorPalette(handCursorPalette, 1, ARRAYSIZE(handCursorPalette) / 4);
			break;
		case 4001:
			// Red Marble
			break;
		case 4002:
			// Orange Marble
			break;
		case 4003:
			// Yellow Marble
			break;
		case 4004:
			// Green Marble
			break;
		case 4005:
			// Blue Marble
			break;
		case 4006:
			// Purple Marble
			break;
		case 5000:
			// Pellet
			CursorMan.replaceCursor(pelletCursor, 8, 8, 4, 4, 0);
			CursorMan.replaceCursorPalette(pelletCursorPalette, 1, ARRAYSIZE(pelletCursorPalette) / 4);
			break;
		case 9000:
			// Hide Cursor
			CursorMan.showMouse(false);
			break;
		default:
			error ("Cursor %d does not exist!", num);
	}
	
	if (num != 9000) // Show Cursor
		CursorMan.showMouse(true);
	
	// Should help in cases where we need to hide the cursor immediately.
	_vm->_system->updateScreen();
}
	
void RivenGraphics::showInventory() {
	// Don't redraw the inventory
	if (_inventoryDrawn)
		return;
	
	// Clear the inventory area
	clearInventoryArea();
	
	// The demo doesn't have the inventory system and we don't want
	// to show the inventory on setup screens or in other journals.
	if (_vm->getFeatures() & GF_DEMO || _vm->getCurStack() == aspit)
		return;
		
	// There are three books and three vars. However, there's only
	// a possible two combinations. Either you have only Atrus'
	// journal or you have all three books.
	// bool hasAtrusBook = *_vm->matchVarToString("aatrusbook") != 0;
	bool hasCathBook = *_vm->matchVarToString("acathbook") != 0;
	// bool hasTrapBook = *_vm->matchVarToString("atrapbook") != 0;
	
	if (!hasCathBook) {
		drawInventoryImage(101, atrusJournalRectSolo);
	} else {
		drawInventoryImage(101, atrusJournalRect);
		drawInventoryImage(102, cathJournalRect);
		drawInventoryImage(100, trapBookRect);
	}
	
	_vm->_system->updateScreen();
	_inventoryDrawn = true;
}
	
void RivenGraphics::hideInventory() {
	// Don't hide the inventory twice
	if (!_inventoryDrawn)
		return;
		
	// Clear the area
	clearInventoryArea();
	
	_inventoryDrawn = false;
}

void RivenGraphics::clearInventoryArea() {
	// Clear the inventory area
	static const Common::Rect inventoryRect = Common::Rect(0, 392, 608, 436);

	// Lock the screen
	Graphics::Surface *screen = _vm->_system->lockScreen();

	// Fill the inventory area with black
	screen->fillRect(inventoryRect, _pixelFormat.RGBToColor(0, 0, 0));
		
	_vm->_system->unlockScreen();
}

void RivenGraphics::drawInventoryImage(uint16 id, Common::Rect rect) {
	ImageData *imageData = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, id));
	Graphics::Surface *surface = imageData->getSurface();
	delete imageData;

	_vm->_system->copyRectToScreen((byte *)surface->pixels, surface->pitch, rect.left, rect.top, surface->w, surface->h);

	surface->free();
	delete surface;
}

void RivenGraphics::drawRect(Common::Rect rect, bool active) {
	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	Graphics::Surface *screen = _vm->_system->lockScreen();
	
	if (active)
		screen->frameRect(rect, _pixelFormat.RGBToColor(0, 255, 0));
	else
		screen->frameRect(rect, _pixelFormat.RGBToColor(255, 0, 0));
	
	_vm->_system->unlockScreen();
}

LBGraphics::LBGraphics(MohawkEngine_LivingBooks *vm) : _vm(vm) {
	_bmpDecoder = (_vm->getGameType() == GType_OLDLIVINGBOOKS) ? new OldMohawkBitmap() : new MohawkBitmap();
	_palette = new byte[256 * 4];
	memset(_palette, 0, 256 * 4);
}

LBGraphics::~LBGraphics() {
	delete _bmpDecoder;
	delete[] _palette;
}

void LBGraphics::copyImageToScreen(uint16 image, uint16 left, uint16 right) {
	if (_vm->getGameType() == GType_OLDLIVINGBOOKS) {
		// Drawing images in the old format isn't supported (yet)
		ImageData *imageData = _bmpDecoder->decodeImage(_vm->wrapStreamEndian(ID_BMAP, image));
		delete imageData;
	} else {
		ImageData *imageData = _bmpDecoder->decodeImage(_vm->getRawData(ID_TBMP, image));
		imageData->_palette = _palette;
		Graphics::Surface *surface = imageData->getSurface();
		imageData->_palette = NULL; // Unset the palette so it doesn't get deleted
		delete imageData;
	
		uint16 width = MIN<int>(surface->w, 640);
		uint16 height = MIN<int>(surface->h, 480);
		_vm->_system->copyRectToScreen((byte *)surface->pixels, surface->pitch, left, right, width, height);
		surface->free();
		delete surface;
		
		// FIXME: Remove this and update only at certain points
		_vm->_system->updateScreen();
	}
}

void LBGraphics::setPalette(uint16 id) {
	// Old Living Books gamnes use the old CTBL-style palette format while newer
	// games use the better tPAL format which can store partial palettes.

	if (_vm->getGameType() == GType_OLDLIVINGBOOKS) {
		Common::SeekableSubReadStreamEndian *ctblStream = _vm->wrapStreamEndian(ID_CTBL, id);
		uint16 colorCount = ctblStream->readUint16();
	
		for (uint16 i = 0; i < colorCount; i++) {
			_palette[i * 4] = ctblStream->readByte();
			_palette[i * 4 + 1] = ctblStream->readByte();
			_palette[i * 4 + 2] = ctblStream->readByte();
			_palette[i * 4 + 3] = ctblStream->readByte();
		}
		
		delete ctblStream;
	} else {
		Common::SeekableReadStream *tpalStream = _vm->getRawData(ID_TPAL, id);
		uint16 colorStart = tpalStream->readUint16BE();
		uint16 colorCount = tpalStream->readUint16BE();

		for (uint16 i = colorStart; i < colorStart + colorCount; i++) {
			_palette[i * 4] = tpalStream->readByte();
			_palette[i * 4 + 1] = tpalStream->readByte();
			_palette[i * 4 + 2] = tpalStream->readByte();
			_palette[i * 4 + 3] = tpalStream->readByte();
		}
	
		delete tpalStream;
	}
}

} // End of namespace Mohawk
