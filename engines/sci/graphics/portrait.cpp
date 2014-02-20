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

#include "common/archive.h"
#include "common/file.h"	// for DumpFile
#include "common/system.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/portrait.h"
#include "sci/sound/audio.h"

namespace Sci {

Portrait::Portrait(ResourceManager *resMan, EventManager *event, GfxScreen *screen, GfxPalette *palette, AudioPlayer *audio, Common::String resourceName)
	: _resMan(resMan), _event(event), _screen(screen), _palette(palette), _audio(audio), _resourceName(resourceName) {
	init();
}

Portrait::~Portrait() {
	delete[] _lipSyncDataOffsetTable;
	delete[] _bitmaps;
	delete[] _fileData;
}

void Portrait::init() {
	// .BIN files are loaded from actors directory and from .\ directory
	// header:
	// 3 bytes "WIN"
	// 2 bytes main width (should be the same as first bitmap header width)
	// 2 bytes main height (should be the same as first bitmap header height)
	// 2 bytes animation count
	// 2 bytes unknown
	// 2 bytes lip sync ID count
	// 4 bytes paletteSize (base 1)
	//  -> 17 bytes
	// paletteSize bytes paletteData
	// 14 bytes bitmap header
	//  -> 4 bytes unknown
	//  -> 2 bytes height
	//  -> 2 bytes width
	//  -> 6 bytes unknown
	// height * width bitmap data
	// another animation count times bitmap header and data
	int32 fileSize = 0;
	Common::SeekableReadStream *file =
		SearchMan.createReadStreamForMember("actors/" + _resourceName + ".bin");
	if (!file) {
		file = SearchMan.createReadStreamForMember(_resourceName + ".bin");
		if (!file)
			error("portrait %s.bin not found", _resourceName.c_str());
	}
	fileSize = file->size();
	_fileData = new byte[fileSize];
	file->read(_fileData, fileSize);
	delete file;

	if (strncmp((char *)_fileData, "WIN", 3)) {
		error("portrait %s doesn't have valid header", _resourceName.c_str());
	}
	_width = READ_LE_UINT16(_fileData + 3);
	_height = READ_LE_UINT16(_fileData + 5);
	_bitmapCount = READ_LE_UINT16(_fileData + 7);
	_lipSyncIDCount = READ_LE_UINT16(_fileData + 11);

	_bitmaps = new PortraitBitmap[_bitmapCount];

	uint16 portraitPaletteSize = READ_LE_UINT16(_fileData + 13);
	byte *data = _fileData + 17;
	// Read palette
	memset(&_portraitPalette, 0, sizeof(Palette));
	uint16 palSize = 0, palNr = 0;
	while (palSize < portraitPaletteSize) {
		_portraitPalette.colors[palNr].b = *data++;
		_portraitPalette.colors[palNr].g = *data++;
		_portraitPalette.colors[palNr].r = *data++;
		_portraitPalette.colors[palNr].used = 1;
		_portraitPalette.intensity[palNr] = 100;
		palNr++; palSize += 3;
	}

	// Read all bitmaps
	PortraitBitmap *curBitmap = _bitmaps;
	uint16 bitmapNr;
	uint16 bytesPerLine;

	for (bitmapNr = 0; bitmapNr < _bitmapCount; bitmapNr++) {
		curBitmap->width = READ_LE_UINT16(data + 2);
		curBitmap->height = READ_LE_UINT16(data + 4);
		bytesPerLine = READ_LE_UINT16(data + 6);
		if (bytesPerLine < curBitmap->width)
			error("kPortrait: bytesPerLine larger than actual width");
		curBitmap->extraBytesPerLine = bytesPerLine - curBitmap->width;
		curBitmap->rawBitmap = data + 14;
		data += 14 + (curBitmap->height * bytesPerLine);
		curBitmap++;
	}

	// Offset table follows
	curBitmap = _bitmaps;
	int32 offsetTableSize = READ_LE_UINT32(data);
	assert((bitmapNr + 1) * 14 <= offsetTableSize);
	data += 4;
	byte *dataOffsetTable = data + 14; // we skip first bitmap offsets
	for (bitmapNr = 0; bitmapNr < _bitmapCount; bitmapNr++) {
		curBitmap->displaceX = READ_LE_UINT16(dataOffsetTable);
		curBitmap->displaceY = READ_LE_UINT16(dataOffsetTable + 2);
		dataOffsetTable += 14;
		curBitmap++;
	}
	data += offsetTableSize;

	// raw lip-sync ID table follows
	uint32 lipSyncIDTableSize;
	
	lipSyncIDTableSize = READ_LE_UINT32(data);
	data += 4;
	assert( lipSyncIDTableSize == (_lipSyncIDCount * 4) );
	_lipSyncIDTable = data;
	data += lipSyncIDTableSize;
	
	// raw lip-sync frame table follows
	uint32 lipSyncDataTableSize;
	uint32 lipSyncDataTableLastOffset;
	byte   lipSyncData;
	uint16 lipSyncDataNr;
	uint16 lipSyncCurOffset;
	
	lipSyncDataTableSize = READ_LE_UINT32(data);
	data += 4;
	assert( lipSyncDataTableSize == 0x220 ); // always this size, just a safety-check
	
	_lipSyncData = data;
	lipSyncDataTableLastOffset = lipSyncDataTableSize - 1;
	_lipSyncDataOffsetTable = new uint16[ _lipSyncIDCount ];
	
	lipSyncDataNr = 0;
	lipSyncCurOffset = 0;
	while ( (lipSyncCurOffset < lipSyncDataTableSize) && (lipSyncDataNr < _lipSyncIDCount) ) {
		// We are currently at the start of ID-frame data
		_lipSyncDataOffsetTable[lipSyncDataNr] = lipSyncCurOffset;
		
		// Look for end of ID-frame data
		lipSyncData = *data++; lipSyncCurOffset++;
		while ( (lipSyncData != 0xFF) && (lipSyncCurOffset < lipSyncDataTableLastOffset) ) {
			// Either terminator (0xFF) or frame-data (1 byte tick count and 1 byte bitmap ID)
			data++;
			lipSyncData = *data++;
			lipSyncCurOffset += 2;
		}
		lipSyncDataNr++;
	}
	_lipSyncDataOffsetTableEnd = data;
	// last 4 bytes seem to be garbage
}

// use this to print out kPortrait debug data
//#define DEBUG_PORTRAIT
// use this to use sync resources instead of rave resources (rave resources are better though)
//#define DEBUG_PORTRAIT_USE_SYNC_RESOURCES

void Portrait::doit(Common::Point position, uint16 resourceId, uint16 noun, uint16 verb, uint16 cond, uint16 seq) {
	_position = position;

	// Now init audio and sync resource
	uint32 audioNumber = ((noun & 0xff) << 24) | ((verb & 0xff) << 16) | ((cond & 0xff) << 8) | (seq & 0xff);
#ifndef DEBUG_PORTRAIT_USE_SYNC_RESOURCES
	ResourceId raveResourceId = ResourceId(kResourceTypeRave, resourceId, noun, verb, cond, seq);
	Resource *raveResource = _resMan->findResource(raveResourceId, true);
	uint raveOffset = 0;
#else
	ResourceId syncResourceId = ResourceId(kResourceTypeSync36, resourceId, noun, verb, cond, seq);
	Resource *syncResource = _resMan->findResource(syncResourceId, true);
	uint syncOffset = 0;
#endif
	
#ifdef DEBUG_PORTRAIT
	// prints out the current lip sync ASCII data
	char debugPrint[4000];
	if (raveResource->size < 4000) {
		memcpy(debugPrint, raveResource->data, raveResource->size);
		debugPrint[raveResource->size] = 0; // set terminating NUL
		debug("kPortrait (noun %d, verb %d, cond %d, seq %d)", noun, verb, cond, seq);
		debug("kPortrait: %s", debugPrint);
	}
#endif

	// TODO: maybe try to create the missing sync resources for low-res KQ6 out of the rave resources

#ifdef DEBUG_PORTRAIT_USE_SYNC_RESOURCES
		// Dump the sync resources to disk
		Common::DumpFile *outFile = new Common::DumpFile();
		Common::String outName = syncResourceId.toPatchNameBase36() + ".sync36";
		outFile->open(outName);
		syncResource->writeToStream(outFile);
		outFile->finalize();
		outFile->close();

		ResourceId raveResourceId = ResourceId(kResourceTypeRave, resourceId, noun, verb, cond, seq);
		Resource *raveResource = _resMan->findResource(raveResourceId, true);
		outName = raveResourceId.toPatchNameBase36() + ".rave";
		outFile->open(outName);
		raveResource->writeToStream(outFile);
		outFile->finalize();
		outFile->close();
		_resMan->unlockResource(raveResource);

		delete outFile;
#endif

	// Set the portrait palette
	_palette->set(&_portraitPalette, false, true);

	// Draw base bitmap
	drawBitmap(0);
	bitsShow();

	// Start playing audio...
	_audio->stopAudio();
	_audio->startAudio(resourceId, audioNumber);

#ifndef DEBUG_PORTRAIT_USE_SYNC_RESOURCES
	if (!raveResource) {
		warning("kPortrait: no rave resource %d %X", resourceId, audioNumber);
		return;
	}
	
	// Do animation depending on rave resource till audio is done playing
	int16 raveTicks;
	uint16 raveID;
	byte *raveLipSyncData;
	byte raveLipSyncTicks;
	byte raveLipSyncBitmapNr;
	int timerPosition = 0;
	int timerPositionWithin = 0;
	int curPosition;
	SciEvent curEvent;
	bool userAbort = false;

	while ((raveOffset < raveResource->size) && (!userAbort)) {
		// rave string starts with tick count, followed by lipSyncID, tick count and so on
		raveTicks = raveGetTicks(raveResource, &raveOffset);
		if (raveTicks < 0)
			break;
		
		// get lipSyncID
		raveID = raveGetID(raveResource, &raveOffset);
		if (raveID) {
			raveLipSyncData = raveGetLipSyncData(raveID);
		} else {
			raveLipSyncData = NULL;
		}
		
		timerPosition += raveTicks;

		// Wait till syncTime passed, then show specific animation bitmap
		if (timerPosition > 0) {
			do {
				g_sci->getEngineState()->wait(1);
				curEvent = _event->getSciEvent(SCI_EVENT_ANY);
				if (curEvent.type == SCI_EVENT_MOUSE_PRESS ||
					(curEvent.type == SCI_EVENT_KEYBOARD && curEvent.data == SCI_KEY_ESC) ||
					g_sci->getEngineState()->abortScriptProcessing == kAbortQuitGame)
					userAbort = true;
				curPosition = _audio->getAudioPosition();
			} while ((curPosition != -1) && (curPosition < timerPosition) && (!userAbort));
		}
		
		if (raveLipSyncData) {
			// lip sync data is
			//  Tick:Byte, Bitmap-Nr:BYTE
			//  Tick = 0xFF is the terminator for the data
			timerPositionWithin = timerPosition;
			raveLipSyncTicks = *raveLipSyncData++;
			while ( (raveLipSyncData < _lipSyncDataOffsetTableEnd) && (raveLipSyncTicks != 0xFF) ) {
				timerPositionWithin += raveLipSyncTicks;

				do {
					g_sci->getEngineState()->wait(1);
					curEvent = _event->getSciEvent(SCI_EVENT_ANY);
					if (curEvent.type == SCI_EVENT_MOUSE_PRESS ||
						(curEvent.type == SCI_EVENT_KEYBOARD && curEvent.data == SCI_KEY_ESC) ||
						g_sci->getEngineState()->abortScriptProcessing == kAbortQuitGame)
						userAbort = true;
					curPosition = _audio->getAudioPosition();
				} while ((curPosition != -1) && (curPosition < timerPositionWithin) && (!userAbort));

				raveLipSyncBitmapNr = *raveLipSyncData++;
				
				// bitmap nr within sync data is base 1, we need base 0
				raveLipSyncBitmapNr--;

				if (raveLipSyncBitmapNr < _bitmapCount) {
					drawBitmap(0);
					drawBitmap(raveLipSyncBitmapNr);
					bitsShow();
				} else {
					warning("kPortrait: rave lip sync data tried to draw non-existent bitmap %d", raveLipSyncBitmapNr);
				}
				
				raveLipSyncTicks = *raveLipSyncData++;
			}
		}
	}
#else
	if (!syncResource) {
		// Getting the book in the book shop calls kPortrait where no sync exists
		// TODO: find out what to do then
		warning("kPortrait: no sync resource %d %X", resourceId, audioNumber);
		return;
	}

	// Do animation depending on sync resource till audio is done playing
	uint16 syncCue;
	int timerPosition, curPosition;
	SciEvent curEvent;
	bool userAbort = false;

	while ((syncOffset < syncResource->size - 2) && (!userAbort)) {
		timerPosition = (int16)READ_LE_UINT16(syncResource->data + syncOffset);
		syncOffset += 2;
		if (syncOffset < syncResource->size - 2) {
			syncCue = READ_LE_UINT16(syncResource->data + syncOffset);
			syncOffset += 2;
		} else {
			syncCue = 0xFFFF;
		}

		// Wait till syncTime passed, then show specific animation bitmap
		do {
			g_sci->getEngineState()->wait(1);
			curEvent = _event->getSciEvent(SCI_EVENT_ANY);
			if (curEvent.type == SCI_EVENT_MOUSE_PRESS ||
				(curEvent.type == SCI_EVENT_KEYBOARD && curEvent.data == SCI_KEY_ESC) ||
				g_sci->getEngineState()->abortScriptProcessing == kAbortQuitGame)
				userAbort = true;
			curPosition = _audio->getAudioPosition();
		} while ((curPosition != -1) && (curPosition < timerPosition) && (!userAbort));

		if (syncCue != 0xFFFF) {
			// Display animation bitmap
			if (syncCue < _bitmapCount) {
				if (syncCue)
					drawBitmap(0); // Draw base bitmap first to get valid animation frame
				drawBitmap(syncCue);
				bitsShow();
			} else {
				warning("kPortrait: sync information tried to draw non-existent %d", syncCue);
			}
		}
	}
#endif
	
	// Reset the portrait bitmap to "closed mouth" state (rave.dll seems to do the same)
	drawBitmap(0);
	bitsShow();
	if (userAbort) {
		_audio->stopAudio();
	}

#ifndef DEBUG_PORTRAIT_USE_SYNC_RESOURCES
	_resMan->unlockResource(raveResource);
#else
	_resMan->unlockResource(syncResource);
#endif
}

// returns ASCII ticks from lip sync string as uint16
int16 Portrait::raveGetTicks(Resource *resource, uint *offset) {
	uint curOffset = *offset;
	byte *curData = resource->data + curOffset;
	byte curByte;
	uint16 curValue = 0;
	
	if (curOffset >= resource->size)
		return -1;
	
	while (curOffset < resource->size) {
		curByte = *curData++; curOffset++;
		if ( curByte == ' ' )
			break;
		if ( (curByte >= '0') && (curByte <= '9') ) {
			curValue = curValue * 10 + ( curByte - '0' );
		} else {
			// no number -> assume there is an ID at current offset
			return 0;
		}
	}
	*offset = curOffset;
	return curValue;
}

// returns ASCII ID from lip sync string as uint16
uint16 Portrait::raveGetID(Resource *resource, uint *offset) {
	uint curOffset = *offset;
	byte *curData = resource->data + curOffset;
	byte curByte = 0;
	uint16 curValue = 0;
	
	while (curOffset < resource->size) {
		curByte = *curData++; curOffset++;
		if ( curByte == ' ' )
			break;
		if (!curValue) {
			curValue = curByte << 8;
		} else {
			curValue |= curByte;
		}
	}
	
	*offset = curOffset;
	return curValue;
}

// Searches for a specific lip sync ID and returns pointer to lip sync data or NULL in case ID was not found
byte *Portrait::raveGetLipSyncData(uint16 raveID) {
	uint lipSyncIDNr = 0;
	byte *lipSyncIDPtr = _lipSyncIDTable;
	byte lipSyncIDByte1, lipSyncIDByte2;
	uint16 lipSyncID;
	
	lipSyncIDPtr++; // skip over first byte
	while (lipSyncIDNr < _lipSyncIDCount) {
		lipSyncIDByte1 = *lipSyncIDPtr++;
		lipSyncIDByte2 = *lipSyncIDPtr++;
		lipSyncID = ( lipSyncIDByte1 << 8 ) | lipSyncIDByte2;
		
		if ( lipSyncID == raveID ) {
			return _lipSyncData + _lipSyncDataOffsetTable[lipSyncIDNr];
		}
		
		lipSyncIDNr++;
		lipSyncIDPtr += 2; // ID is every 4 bytes
	}
	return NULL;
}

void Portrait::drawBitmap(uint16 bitmapNr) {
	byte *data = _bitmaps[bitmapNr].rawBitmap;
	uint16 bitmapHeight = _bitmaps[bitmapNr].height;
	uint16 bitmapWidth = _bitmaps[bitmapNr].width;
	Common::Point bitmapPosition = _position;

	bitmapPosition.x += _bitmaps[bitmapNr].displaceX;
	bitmapPosition.y += _bitmaps[bitmapNr].displaceY;

	for (int y = 0; y < bitmapHeight; y++) {
		for (int x = 0; x < bitmapWidth; x++) {
			_screen->putPixelOnDisplay(bitmapPosition.x + x, bitmapPosition.y + y, _portraitPalette.mapping[*data++]);
		}
		data += _bitmaps[bitmapNr].extraBytesPerLine;
	}
}

void Portrait::bitsShow() {
	Common::Rect bitmapRect = Common::Rect(_width, _height);
	bitmapRect.moveTo(_position.x, _position.y);
	_screen->copyDisplayRectToScreen(bitmapRect);
	g_system->updateScreen();
}

} // End of namespace Sci
