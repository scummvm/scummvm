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

#include "lastexpress/lastexpress.h"
#include "lastexpress/game/nis.h"
#include "lastexpress/data/archive.h"

namespace LastExpress {

NISManager::NISManager(LastExpressEngine *engine) {
	_engine = engine;

	_background1 = new NisSprite();
	_background2 = new NisSprite();
	_waneSprite = new NisSprite();
	_waxSprite = new NisSprite();
}

NISManager::~NISManager() {
	SAFE_DELETE(_background1);
	SAFE_DELETE(_background2);
	SAFE_DELETE(_waneSprite);
	SAFE_DELETE(_waxSprite);
	SAFE_DELETE_ARR(_events);
}

void NISManager::clearBounds() {
	_nisRect.left = 640;
	_nisRect.top = 480;
	_nisRect.right = 0;
	_nisRect.bottom = 0;
}

void NISManager::addBounds(Extent extent) {
	if (_nisRect.left > (int16)extent.left)
		_nisRect.left = (int16)extent.left;

	if (_nisRect.right < (int16)extent.right)
		_nisRect.right = (int16)extent.right;

	if (_nisRect.top > (int16)extent.top)
		_nisRect.top = (int16)extent.top;

	if (_nisRect.bottom < (int16)extent.bottom)
		_nisRect.bottom = (int16)extent.bottom;
}

void NISManager::convertNSPR16(byte *spriteData, NisSprite *outSprite) {
	uint32 compDataOffset     = READ_LE_UINT32(spriteData);
	uint32 eraseMaskOffset    = READ_LE_UINT32(spriteData + 4);
	uint32 colorPaletteOffset = READ_LE_UINT32(spriteData + 8);

	outSprite->rect.left   = READ_LE_INT32(spriteData + 12);
	outSprite->rect.top    = READ_LE_INT32(spriteData + 16);
	outSprite->rect.right  = READ_LE_INT32(spriteData + 20);
	outSprite->rect.bottom = READ_LE_INT32(spriteData + 24);
	outSprite->rect.width  = READ_LE_INT32(spriteData + 28);
	outSprite->rect.height = READ_LE_INT32(spriteData + 32);

	outSprite->colorPalette = (uint16 *)&spriteData[colorPaletteOffset];

	WRITE_LE_UINT16(&outSprite->colorPalette[0], 0);
	WRITE_LE_UINT16(&outSprite->colorPalette[1], 0);

	for (int i = 0; i < 128; ++i)
		outSprite->colorPalette[i] = READ_LE_UINT16(&outSprite->colorPalette[i]);

	_engine->getGraphicsManager()->modifyPalette((uint16 *)outSprite->colorPalette, 128);

	for (int i = 0; i < 128; i++)
		outSprite->gammaPalette[i] = READ_UINT16(spriteData + 2 * i + 36);

	outSprite->compBits = spriteData[2 * 128 + 36];

	outSprite->compData = &spriteData[compDataOffset];

	if (eraseMaskOffset)
		outSprite->eraseMask = &spriteData[eraseMaskOffset];
}

void NISManager::getStream(byte *data, int32 size) {
	if (size > _remainingStreamBytes) {
		Slot *slot;

		for (slot = _engine->getSoundManager()->_soundCache; slot; slot = slot->getNext()) {
			if (!slot->hasTag(kSoundTagNIS))
				break;
		}

		if (slot && _currentNISSound)
			_currentNISSound->addStatusFlag(kSoundFlagPauseRequested);

		for (int i = 0; i < 20; ++i) {
			if ((_flags & kNisFlagDataChunksAvailable) != 0)
				loadChunk(32);
		}

		getStream(data, size);

		if (_currentNISSound)
			_currentNISSound->removeStatusFlag(kSoundFlagPauseRequested | kSoundFlagPaused);
	} else if (size + _streamCurrentPosition <= _streamBufferSize) {
		memcpy(data, (byte *)_backgroundSurface + _streamCurrentPosition, size);

		_streamCurrentPosition += size;
		_remainingStreamBytes -= size;

		if (_streamCurrentPosition >= _streamBufferSize)
			_streamCurrentPosition = 0;
	} else {
		int32 prevSize = _streamBufferSize - _streamCurrentPosition;
		getStream(data, _streamBufferSize - _streamCurrentPosition);
		getStream(&data[prevSize], size - prevSize);
	}
}

void NISManager::loadSnd(int32 size) {
	byte *currentBufferPtr = _currentNISSound->getCurrentBufferPtr();
	byte *dataEnd = _currentNISSound->getDataEnd();
	int32 availableSize = dataEnd - currentBufferPtr;

	if (size < dataEnd - currentBufferPtr) {
		getStream(_currentNISSound->getCurrentBufferPtr(), size);
		_currentNISSound->advanceCurrentBufferPtrBy(size);
		_currentNISSound->advanceLoadedBytesBy(size);
	} else {
		getStream(_currentNISSound->getCurrentBufferPtr(), dataEnd - currentBufferPtr);
		_currentNISSound->advanceCurrentBufferPtrBy(availableSize);
		_currentNISSound->setCurrentBufferPtr(_currentNISSound->getDataStart());
		loadSnd(size - availableSize);
	}

	if (_currentNISSound->getCurrentDataPtr() < _currentNISSound->getDataStart() + 88064)
		memcpy(_currentNISSound->getDataEnd(), _currentNISSound->getDataStart(), NIS_SOUND_CHUNK_SIZE);
}

int NISManager::loadChunk(int32 size) {
	int32 sizeToLoad = _totalStreamPages - ((_remainingStreamBytes + 2047) / MEM_PAGE_SIZE);

	if (!_archive || (_archive->status & 2) == 0)
		return 0;

	if (sizeToLoad > _totalStreamPages)
		sizeToLoad = 0;

	if (sizeToLoad > size)
		sizeToLoad = size;

	if (!sizeToLoad)
		return 0;

	uint16 sizeArch = _archive->size;
	uint16 currentPos = _archive->currentPos;

	if (sizeArch <= currentPos)
		return 0;

	if (sizeToLoad > 16)
		sizeToLoad = 16;

	if (currentPos + sizeToLoad > sizeArch) {
		_flags ^= kNisFlagDataChunksAvailable;
		sizeToLoad = sizeArch - currentPos;
	}

	if (sizeToLoad + _currentStreamPage >= _totalStreamPages)
		sizeToLoad = _totalStreamPages - _currentStreamPage;

	_engine->getArchiveManager()->readHPF(_archive, ((byte *)_backgroundSurface + (_currentStreamPage * MEM_PAGE_SIZE)), sizeToLoad);

	_currentStreamPage += sizeToLoad;
	_remainingStreamBytes += sizeToLoad * MEM_PAGE_SIZE;

	if (_currentStreamPage >= _totalStreamPages)
		_currentStreamPage -= _totalStreamPages;

	return sizeToLoad;
}

bool NISManager::initNIS(const char *filename, int32 flags) {
	int32 chunkSizeRead = 0;
	int32 eventSize = 0;
	Slot *slot;

	_currentNISSound = nullptr;
	_backgroundType = 0;
	_selectBackgroundType = 0;
	_backgroundFlag = false;
	_cumulativeEventSize = 0;
	_flags = flags | kNisFlagDataChunksAvailable;
	_decompressToBackBuffer = true;
	_firstNISBackgroundDraw = true;

	clearBounds();

	_currentStreamPage = 0;
	_streamCurrentPosition = 0;
	_remainingStreamBytes = 0;
	_streamBufferSize = 1530 * MEM_PAGE_SIZE;
	_originalBackgroundSurface = _engine->getGraphicsManager()->_frontBuffer;
	_totalBackgroundPages = 1530;
	_totalStreamPages = 1530;
	_backgroundSurface = _engine->getGraphicsManager()->_frontBuffer;

	_archive = _engine->getArchiveManager()->openHPF(filename);
	if (!_archive) {
		warning("NIS %s not ready", filename);
		return false;
	}

	_engine->getMemoryManager()->lockSeqMem((_totalBackgroundPages - 300) * MEM_PAGE_SIZE);
	getStream((byte *)&_eventsCount, 4);

	_eventsCount = READ_LE_INT32(&_eventsCount);

	_eventsByteStream = (byte *)(_backgroundSurface + 2);

	_background1Offset = READ_LE_INT32((int32 *)_backgroundSurface + 2);
	_background1Offset += 16;
	_background1Offset &= 0xFFFFFFF0;
	_streamBufferSize -= _background1Offset;
	_background1ByteStream = (byte *)((byte *)_backgroundSurface + _streamBufferSize);

	_waneSpriteOffset = READ_LE_INT32((int32 *)_backgroundSurface + 4);
	_waneSpriteOffset += 16;
	_waneSpriteOffset &= 0xFFFFFFF0;
	_streamBufferSize -= _waneSpriteOffset;

	_waneSpriteByteStream = (byte *)((byte *)_backgroundSurface + _streamBufferSize);
	_streamBufferSize -= 8 * _eventsCount;
	_eventsByteStream = (byte *)((byte *)_backgroundSurface + _streamBufferSize);
	_totalStreamPages = (_streamBufferSize / MEM_PAGE_SIZE);
	_streamBufferSize = (_streamBufferSize / MEM_PAGE_SIZE) * MEM_PAGE_SIZE;

	chunkSizeRead = loadChunk(32);

	getStream(_eventsByteStream, 8 * _eventsCount);

	SAFE_DELETE_ARR(_events);
	_events = new NisEvents[_eventsCount];

	for (int i = 0; i < _eventsCount; i++) {
		_events[i].eventType = READ_LE_INT16(_eventsByteStream + i * 8);
		_events[i].eventTime = READ_LE_INT16(_eventsByteStream + i * 8 + 2);
		_events[i].eventSize = READ_LE_INT32(_eventsByteStream + i * 8 + 4);
	}

	while ((_flags & kNisFlagDataChunksAvailable) != 0) {
		if (!_events[4].eventSize)
			break;

		chunkSizeRead = loadChunk(32) * MEM_PAGE_SIZE;

		if (!chunkSizeRead)
			break;

		eventSize = _events[4].eventSize;
		_events[4].eventSize = chunkSizeRead >= eventSize ? 0 : eventSize - chunkSizeRead;
	}

	if ((flags & kNisFlagSoundFade) != 0)
		_engine->getSoundManager()->NISFadeOut();

	getNISSlot();

	for (slot = _engine->getSoundManager()->_soundCache; slot; slot = slot->getNext()) {
		if (slot->hasTag(kSoundTagNIS))
			break;
	}

	_currentNISSound = slot;

	if (!slot) {
		if ((flags & kNisFlagSoundFade) != 0)
			_engine->getSoundManager()->NISFadeIn();

		endNIS();
		return false;
	}

	Common::String noExtName = filename;
	noExtName.replace('.', '\0');
	_currentNISSound->setSub(noExtName.c_str());

	Common::String lnkName = Common::String(noExtName.c_str()) + ".LNK";

	HPF *lnkResource = _engine->getArchiveManager()->openHPF(lnkName.c_str());
	if (lnkResource) {
		_engine->getArchiveManager()->closeHPF(lnkResource);
		_chainedSoundSlot = new Slot(_engine->getSoundManager(), lnkName.c_str(), kSoundTypeLink | kVolumeFull, 105);
	} else {
		_chainedSoundSlot = nullptr;
	}

	if (_chainedSoundSlot) {
		_currentNISSound->setChainedSound(_chainedSoundSlot);
		_currentNISSound->addStatusFlag(kSoundFlagHasLinkAfter);

		if (!scumm_stricmp(lnkName.c_str(), "1917.LNK")) {
			_chainedSoundSlot->setAssociatedCharacter(kCharacterKronos);
			_chainedSoundSlot->removeStatusFlag(kSoundTypeLink | kSoundFlagFixedVolume);
			_chainedSoundSlot->addStatusFlag(kSoundTypeConcert);
			_chainedSoundSlot->assignDirectTag(kSoundTagConcert);
			_chainedSoundSlot->assignDirectVolume(16);
		}
	}

	return true;
}

void NISManager::endNIS() {
	if (_archive) {
		_engine->getArchiveManager()->closeHPF(_archive);
		_archive = nullptr;
	}
}

void NISManager::abortNIS() {
	if ((_flags & kNisFlagPlaying) != 0)
		_flags |= kNisFlagAbortRequested;
}

void NISManager::nisMouse(Event *event) {
	if ((event->flags & kMouseFlagRightDown) != 0)
		abortNIS();
}

void NISManager::nisTimer(Event *event) {
	_engine->setEventTickInternal(false);
}

void NISManager::drawSprite(NisSprite *sprite) {
	Extent rect;

	rect.left = sprite->rect.left;
	rect.right = sprite->rect.right;
	rect.top = sprite->rect.top;
	rect.bottom = sprite->rect.bottom;
	rect.width = sprite->rect.width;
	rect.height = sprite->rect.height;

	addBounds(rect);

	// Use a temp sprite and just fill out the bare minimum needed for decompression...
	Sprite tempSprite;
	tempSprite.compData = sprite->compData;
	tempSprite.colorPalette = sprite->colorPalette;
	tempSprite.rect = rect;

	switch (sprite->compBits) {
	case 3:
		if (_decompressToBackBuffer) {
			_engine->getGraphicsManager()->bitBltSprite8(&tempSprite, _engine->getGraphicsManager()->_backBuffer);
		} else if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltSprite8(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	case 4:
		if (_decompressToBackBuffer) {
			_engine->getGraphicsManager()->bitBltSprite16(&tempSprite, _engine->getGraphicsManager()->_backBuffer);
		} else if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltSprite16(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	case 5:
		if (_decompressToBackBuffer) {
			_engine->getGraphicsManager()->bitBltSprite32(&tempSprite, _engine->getGraphicsManager()->_backBuffer);
		} else if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltSprite32(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	case 7:
		if (_decompressToBackBuffer) {
			_engine->getGraphicsManager()->bitBltSprite128(&tempSprite, _engine->getGraphicsManager()->_backBuffer);
		} else if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltSprite128(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	case 255:
		if (_decompressToBackBuffer) {
			_engine->getGraphicsManager()->bitBltSprite255(&tempSprite, _engine->getGraphicsManager()->_backBuffer);
		} else if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltSprite255(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	default:
		return;
	}
}

void NISManager::waxSprite(NisSprite *sprite) {
	Extent rect;

	rect.left = sprite->rect.left;
	rect.right = sprite->rect.right;
	rect.top = sprite->rect.top;
	rect.bottom = sprite->rect.bottom;
	rect.width = sprite->rect.width;
	rect.height = sprite->rect.height;

	addBounds(rect);

	// Use a temp sprite and just fill out the bare minimum needed for decompression...
	Sprite tempSprite;
	tempSprite.compData = sprite->compData;
	tempSprite.colorPalette = sprite->colorPalette;
	tempSprite.rect = rect;

	switch (sprite->compBits) {
	case 3:
		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltWax8(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	case 4:
		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltWax16(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	case 5:
		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltWax32(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	case 7:
		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltWax128(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	default:
		return;
	}
}

void NISManager::waneSprite(NisSprite *sprite) {
	Extent rect;

	rect.left = sprite->rect.left;
	rect.right = sprite->rect.right;
	rect.top = sprite->rect.top;
	rect.bottom = sprite->rect.bottom;
	rect.width = sprite->rect.width;
	rect.height = sprite->rect.height;

	addBounds(rect);

	// Use a temp sprite and just fill out the bare minimum needed for decompression...
	Sprite tempSprite;
	tempSprite.compData = sprite->compData;
	tempSprite.colorPalette = sprite->colorPalette;
	tempSprite.rect = rect;

	switch (sprite->compBits) {
	case 3:
		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltWane8(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	case 4:
		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltWane16(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	case 5:
		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltWane32(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	case 7:
		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->bitBltWane128(&tempSprite, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
			_engine->getGraphicsManager()->unlockSurface();
		}

		break;
	default:
		return;
	}
}

void NISManager::showNISStatus(int32 status) {
	if (status != -1)
		_nisStatus = status;
}

bool NISManager::doNIS(const char *name, int32 flags) {
	if (_engine->isGoldEdition())
		return true;

	Slot *slot;

	if (_engine->mouseHasRightClicked())
		return false;

	if (!initNIS(name, flags))
		return false;

	for (slot = _engine->getSoundManager()->_soundCache; slot; slot = slot->getNext()) {
		if (slot->hasTag(kSoundTagNIS))
			break;
	}

	if (slot) {
		_currentNISSound = slot;
		slot->addStatusFlag(kSoundFlagKeepAfterFinish);
	}

	for (int i = 0; i < 30; i++) {
		loadChunk(32);
	}

	_savedMouseEventHandle = _engine->getMessageManager()->getEventHandle(1);
	_savedTimerEventHandle = _engine->getMessageManager()->getEventHandle(3);

	_engine->getMessageManager()->setEventHandle(kEventChannelMouse, &LastExpressEngine::nisMouseWrapper);
	_engine->getMessageManager()->setEventHandle(kEventChannelTimer, &LastExpressEngine::nisTimerWrapper);

	_engine->getSoundManager()->setSoundDriverTicks(0);

	_numOfOverlays = 0;

	for (slot = _engine->getSoundManager()->_soundCache; slot; slot = slot->getNext()) {
		if (slot->hasTag(kSoundTagNIS))
			break;
	}

	_currentNISSound = slot;
	if (slot) {
		_flags |= (kNisFlagPlaying | kNisFlagHasSound);
		slot->addStatusFlag(kSoundFlagKeepAfterFinish);
	} else {
		_flags = 0;
	}

	_engine->getSoundManager()->setSoundDriverTicks(0);

	if (_engine->mouseHasRightClicked()) {
		abortNIS();
		_flags &= ~kNisFlagPlaying;
	}

	for (int i = 0; i < _eventsCount; i++) {
		if ((_flags & kNisFlagPlaying) == 0)
			break;

		if (_events[i].eventTime && _events[i].eventTime + 60 < _engine->getSoundManager()->getSoundDriverTicks()) {
			_engine->getSoundManager()->addSoundDriverFlags(kSoundDriverNISHasRequestedDelay);
			_currentNISSound->addStatusFlag(kSoundFlagPauseRequested);
			loadChunk(32);
			loadChunk(32);
		}

		bool useDriverTicks = false;

		if ((_currentNISSound->getStatusFlags() & (kSoundFlagCloseRequested | kSoundFlagClosed)) != 0 || _currentNISSound->getTime() <= 1) {
			useDriverTicks = true;
		} else {
			_engine->getSoundManager()->setSoundDriverTicks(_currentNISSound->getTime());
		}

		if ((_flags & kNisFlagPlaying) != 0) {
			do {
				if (useDriverTicks) {
					if (_events[i].eventTime <= _engine->getSoundManager()->getSoundDriverTicks())
						break;
				} else {
					if (_events[i].eventTime <= _currentNISSound->getTime())
						break;
				}

				if ((_flags & kNisFlagDataChunksAvailable) != 0)
					loadChunk(32);

				showNISStatus(-1);

				if ((_currentNISSound->getStatusFlags() & kSoundFlagPaused) != 0) {
					_currentNISSound->removeStatusFlag(kSoundFlagPauseRequested | kSoundFlagPaused);
					_engine->getSoundManager()->removeSoundDriverFlags(kSoundDriverNISHasRequestedDelay);
				}

				_engine->getSoundManager()->soundThread();
				_engine->getSubtitleManager()->subThread();
				if (!_engine->getMessageManager()->process()) {
					// Only wait and handle events if we've processed all messages, unlike the original which had a separate thread for input...
					_engine->waitForTimer(5);
				}

				for (slot = _engine->getSoundManager()->_soundCache; slot; slot = slot->getNext()) {
					if (slot->hasTag(kSoundTagNIS))
						break;
				}

				_currentNISSound = slot;

				if (!_backgroundFlag && (_flags & kNisFlagAbortRequested) != 0)
					_flags &= ~kNisFlagPlaying;

			} while ((_flags & kNisFlagPlaying) != 0);

			if ((_flags & kNisFlagPlaying) != 0) {
				if (_events[i].eventTime) {
					showNISStatus(_engine->getSoundManager()->getSoundDriverTicks() - _events[i].eventTime);
				} else {
					showNISStatus(-1);
				}

				processNIS(&_events[i]);

				while ((_flags & kNisFlagPlaying) != 0) {
					if (!_engine->getMessageManager()->process())
						break;

					_engine->handleEvents();
					_engine->getSubtitleManager()->subThread();
					_engine->getSoundManager()->soundThread();

					if (!_backgroundFlag && (_flags & kNisFlagAbortRequested) != 0)
						_flags &= ~kNisFlagPlaying;
				}
			}
		}
	}

	if (_currentNISSound && !_currentNISSound->getTime())
		_currentNISSound->addStatusFlag(kSoundFlagCloseRequested);

	_engine->getMessageManager()->setEventHandle(kEventChannelMouse, _savedMouseEventHandle);
	_engine->getMessageManager()->setEventHandle(kEventChannelTimer, _savedTimerEventHandle);

	if (_currentNISSound && (_flags & kNisFlagAbortRequested) != 0)
		_currentNISSound->addStatusFlag(kSoundFlagCloseRequested);

	if (_currentNISSound) {
		if (_currentNISSound->getSubtitle()) {
			_currentNISSound->getSubtitle()->kill();
			_engine->getSubtitleManager()->subThread();
		}

		_currentNISSound->removeStatusFlag(kSoundFlagKeepAfterFinish);
	}

	if (_chainedSoundSlot && (_chainedSoundSlot->getStatusFlags() & (kSoundFlagPlayRequested | kSoundFlagPlaying)) == 0) {
		do {
			_engine->getSoundManager()->soundThread();
			_engine->handleEvents();
		} while ((_chainedSoundSlot->getStatusFlags() & (kSoundFlagPlayRequested | kSoundFlagPlaying)) == 0);
	}

	endNIS();
	_engine->getMemoryManager()->freeSeqMem();
	_flags = 0;

	if ((flags & kNisFlagSoundFade) != 0)
		_engine->getSoundManager()->NISFadeIn();

	return true;
}

void NISManager::processNIS(NisEvents *event) {
	int32 x = _engine->getGraphicsManager()->_renderBox1.x;
	int32 width = _engine->getGraphicsManager()->_renderBox1.width;

	debug(2, "NISManager::processNIS(): event at time %d type %d size %d", event->eventTime, event->eventType, event->eventSize);

	switch (event->eventType) {
	case kNISEventBackground1: // 10
		getStream(_background1ByteStream, event->eventSize);
		convertNSPR16(_background1ByteStream, _background1);

		if (_backgroundType == 1)
			_backgroundType = 0;

		return;
	case kNISEventSelectBackground1: // 11
		_selectBackgroundType = 1;
		return;
	case kNISEventBackground2: // 12
		_background2ByteStream = (_background1ByteStream + ((_background1Offset - event->eventSize) & 0xFFFFFFF0));

		getStream(_background2ByteStream, event->eventSize);
		convertNSPR16(_background2ByteStream, _background2);

		if (_backgroundType == 2)
			_backgroundType = 0;

		return;
	case kNISEventSelectBackground2: // 13
		_selectBackgroundType = 2;
		return;
	case kNISEventOverlay: // 20
		if (!_decompressToBackBuffer) {
			addBounds(_spriteExtent);
		}

		_waxWaneToggleFlag = !_waxWaneToggleFlag;

		if (_waxWaneToggleFlag) {
			_waxSpriteByteStream = (_waneSpriteByteStream + ((_waneSpriteOffset - event->eventSize) & 0xFFFFFFF0));
			getStream(_waxSpriteByteStream, event->eventSize);
			convertNSPR16(_waxSpriteByteStream, _waxSprite);
		} else {
			getStream(_waneSpriteByteStream, event->eventSize);
			convertNSPR16(_waneSpriteByteStream, _waneSprite);
		}

		_numOfOverlays++;

		return;
	case kNISEventUpdate: // 21
		_backgroundFlag = 0;

		if (_decompressToBackBuffer)
			_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_backBuffer, 0, 0, 640, 480);

		if (_selectBackgroundType != _backgroundType) {
			drawBK(_selectBackgroundType);
		} else if (_engine->getGraphicsManager()->acquireSurface()) {
			if (_backgroundType == 1) {
				_engine->getGraphicsManager()->copy(
					_engine->getGraphicsManager()->_backBuffer,
					(PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(),
					_background1->rect.left,
					_background1->rect.top,
					_background1->rect.right - _background1->rect.left + 1,
					_background1->rect.bottom - _background1->rect.top + 1
				);
			} else if (_backgroundType == 2) {
				_engine->getGraphicsManager()->copy(
					_engine->getGraphicsManager()->_backBuffer,
					(PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(),
					_background2->rect.left,
					_background2->rect.top,
					_background2->rect.right - _background2->rect.left + 1,
					_background2->rect.bottom - _background2->rect.top + 1
				);
			}

			_engine->getGraphicsManager()->unlockSurface();
		}

		if (!_decompressToBackBuffer) {
			addBounds(_spriteExtent);
		}

		if (_waxWaneToggleFlag) {
			if (_waxSprite->rect.left != 640)
				drawSprite(_waxSprite);

			_spriteExtent.left   = _waxSprite->rect.left;
			_spriteExtent.right  = _waxSprite->rect.right;
			_spriteExtent.top    = _waxSprite->rect.top;
			_spriteExtent.bottom = _waxSprite->rect.bottom;
			_spriteExtent.width  = _waxSprite->rect.width;
			_spriteExtent.height = _waxSprite->rect.height;
		} else {
			if (_waneSprite->rect.left != 640)
				drawSprite(_waneSprite);

			_spriteExtent.left   = _waneSprite->rect.left;
			_spriteExtent.right  = _waneSprite->rect.right;
			_spriteExtent.top    = _waneSprite->rect.top;
			_spriteExtent.bottom = _waneSprite->rect.bottom;
			_spriteExtent.width  = _waneSprite->rect.width;
			_spriteExtent.height = _waneSprite->rect.height;
		}

		if (_firstNISBackgroundDraw) {
			_engine->getGraphicsManager()->burstBox(80, 0, 480, 480);
		} else {
			_engine->getGraphicsManager()->burstBox(
				_nisRect.left,
				_nisRect.top,
				_nisRect.right - _nisRect.left + 1,
				_nisRect.bottom - _nisRect.top + 1
			);
		}

		_engine->getSoundManager()->soundThread();
		_firstNISBackgroundDraw = 0;

		if (_decompressToBackBuffer) {
			for (int i = 0; i < 3; i++) {
				_engine->getSoundManager()->soundThread();
				_engine->getGraphicsManager()->dissolve((2 * x) + (2 * (i & 1)), width, 480, _engine->getGraphicsManager()->_backBuffer);
				_engine->getGraphicsManager()->burstBox(x, 0, width, 480);
				_engine->getSoundManager()->soundThread();
				_engine->handleEvents();
			}

			if (_engine->getGraphicsManager()->acquireSurface()) {
				_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_backBuffer, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(), x, 0, width, 480);
				_engine->getGraphicsManager()->unlockSurface();
			}

			_engine->getGraphicsManager()->burstBox(x, 0, width, 480);
			_engine->getSoundManager()->soundThread();

			if (_backgroundType == 1) {
				drawSprite(_background1);
			} else {
				drawSprite(_background2);
			}
			
			_decompressToBackBuffer = 0;
			_currentNISSound->play();
			_engine->getSoundManager()->setSoundDriverTicks(0);
		}

		clearBounds();

		return;
	case kNISEventUpdateTransition: // 22
		_backgroundFlag = 1;

		if (_selectBackgroundType != _backgroundType) {
			drawBK(_selectBackgroundType);
		} else {
			if (_engine->getGraphicsManager()->acquireSurface()) {
				if (_backgroundType == 1) {
					_engine->getGraphicsManager()->copy(
						_engine->getGraphicsManager()->_backBuffer,
						(PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(),
						_background1->rect.left,
						_background1->rect.top,
						_background1->rect.right - _background1->rect.left + 1,
						_background1->rect.bottom - _background1->rect.top + 1
					);
				} else if (_backgroundType == 2) {
					_engine->getGraphicsManager()->copy(
						_engine->getGraphicsManager()->_backBuffer,
						(PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(),
						_background2->rect.left,
						_background2->rect.top,
						_background2->rect.right - _background2->rect.left + 1,
						_background2->rect.bottom - _background2->rect.top + 1
					);
				}

				_engine->getGraphicsManager()->unlockSurface();
			}
		}

		waxSprite(_waxSprite);
		waneSprite(_waneSprite);

		_engine->getGraphicsManager()->burstBox(
			_nisRect.left,
			_nisRect.top,
			_nisRect.right - _nisRect.left + 1,
			_nisRect.bottom - _nisRect.top + 1
		);

		_engine->getSoundManager()->soundThread();
		clearBounds();

		return;
	case kNISEventSound1: // 30
	case kNISEventSound2: // 31
		_currentNISSound->setFade(event->eventSize);
		return;
	case kNISEventAudioData: // 32
		_cumulativeEventSize += event->eventSize;

		while (_currentNISSound->getSize() + NIS_SOUND_CHUNK_SIZE * _currentNISSound->getTime() + 745 <= (event->eventSize + _currentNISSound->getNumLoadedBytes()))
			_engine->getSoundManager()->soundThread();

		loadSnd(event->eventSize);

		if ((_flags & kNisFlagSoundInitialized) == 0) {
			_flags |= kNisFlagSoundInitialized;
			_currentNISSound->setBlockCount(READ_LE_UINT16((uint16 *)_currentNISSound->getDataStart() + 2) - 1);
			_currentNISSound->setSize(0x16000);
		}

		return;
	default:
		return;
	}
}

void NISManager::drawBK(int type) {
	Extent rect;

	rect.left = 80;
	rect.right = 559;
	rect.bottom = 479;
	rect.top = 0;
	rect.width = 0;
	rect.height = 0;

	if (!_decompressToBackBuffer && _engine->getGraphicsManager()->acquireSurface()) {
		_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_screenSurface, 80, 0, 480, 480);
		_engine->getGraphicsManager()->unlockSurface();
	}

	if (type == 1) {
		drawSprite(_background1);
	} else {
		drawSprite(_background2);
	}

	_firstNISBackgroundDraw = true;

	if (!_decompressToBackBuffer && _engine->getGraphicsManager()->acquireSurface()) {
		_engine->getGraphicsManager()->copy((PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(), _engine->getGraphicsManager()->_backBuffer, 0, 0, 640, 480);
		_engine->getGraphicsManager()->unlockSurface();
	}

	addBounds(rect);
	_backgroundType = type;
}

void NISManager::getNISSlot() {
	// This slot will automatically be included in the queue
	Slot *slot = new Slot(_engine->getSoundManager(), kSoundTypeNIS | kSoundFlagCyclicBuffer | kVolumeFull, 90);

	slot->setCurrentBufferPtr(slot->getSoundBuffer());
	slot->setDataStart(slot->getSoundBuffer());
	slot->setDataEnd(slot->getSoundBuffer() + (44 * MEM_PAGE_SIZE));
	slot->setCurrentDataPtr(slot->getDataStart() + 6);
	slot->setSize(44 * MEM_PAGE_SIZE);
}

Slot *NISManager::getChainedSound() {
	return _chainedSoundSlot;
}

void NISManager::setChainedSound(Slot *slot) {
	_chainedSoundSlot = slot;
}

int32 NISManager::getNISFlag() {
	return _flags;
}

} // End of namespace LastExpress
