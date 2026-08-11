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

#include "common/memstream.h"

namespace LastExpress {

void LastExpressEngine::doCredits() {
	if (!_doCredits) {
		TGAHeader creditsTextTga;
		TGAHeader mapTgas[35];
		byte *nextTga = nullptr;

		int tgaIndex = 0;
		int textHeight = 0;
		int32 scrollSpeed = 0;

		uint16 tgaPalette[256];
		uint16 mapPalette[256];

		memset(tgaPalette, 0, sizeof(tgaPalette));

		_doCredits = 1;
		bool oldCanDrawMouse = getGraphicsManager()->canDrawMouse();
		getGraphicsManager()->setMouseDrawable(false);
		getGraphicsManager()->burstMouseArea();
		getGraphicsManager()->stepBG(49);
		getMemoryManager()->freeFX();
		getOtisManager()->wipeAllGSysInfo();

		readTGAIntoMemory("credits.tga", &creditsTextTga);

		if (_doCredits)
			constructPalette(&creditsTextTga, tgaPalette);

		for (int i = 1; _doCredits && i < ARRAYSIZE(mapTgas); i++) {
			char currentTgaFilename[24];
			memset(currentTgaFilename, 0, sizeof(currentTgaFilename));

			Common::sprintf_s(currentTgaFilename, "map%05d.tga", i + 1);
			readTGAIntoMemory(currentTgaFilename, &mapTgas[i]);

			handleEvents();
			getSoundManager()->soundThread();
		}

		if (_doCredits) {
			tgaIndex = 0;
			textHeight = creditsTextTga.height - 355;
			nextTga = (creditsTextTga.rawDataPtr + 3 * creditsTextTga.colorMapLength + creditsTextTga.idLength + 18);
			Slot *cacheSlot = getSoundManager()->_soundCache;

			while (cacheSlot) {
				if (cacheSlot->hasTag(kSoundTagLink))
					break;

				cacheSlot = cacheSlot->getNext();
			}

			if (cacheSlot) {
				scrollSpeed = (textHeight << 16) / (2 * cacheSlot->getBlockCount()); // Scroll speed based on music length
			} else {
				_doCredits = 0;
			}
		}

		if (_doCredits) {
			_savedMouseEventHandle = getMessageManager()->getEventHandle(1);
			_savedTimerEventHandle = getMessageManager()->getEventHandle(3);

			getMessageManager()->setEventHandle(kEventChannelMouse, &LastExpressEngine::creditsMouseWrapper);
			getMessageManager()->setEventHandle(kEventChannelTimer, &LastExpressEngine::creditsTimerWrapper);

			do {
				getSoundManager()->soundThread();
			} while (getMessageManager()->process());

			waitForTimer(5);

			setEventTickInternal(false);
			
			_savedFrameInterval = getSoundFrameCounter();

			// Main loop
			while (_doCredits) {
				do {
					getSoundManager()->soundThread();
				} while (_doCredits && getMessageManager()->process());

				waitForTimer(5);

				if (!_savedFrameCounter) {
					// Handle the background map transition
					if (_doCredits == 1) {
						int mapChangeRate = (322 - creditsTextTga.height) / 34;
						int nextMapIndex = textHeight / mapChangeRate + 34;

						// Switch to a new iteration of the map
						if (nextMapIndex != tgaIndex && nextMapIndex <= 34) {
							TGAHeader *currentMap = &mapTgas[nextMapIndex];
							tgaIndex = nextMapIndex;

							constructPalette(currentMap, mapPalette);

							// 18 is the size of the original TARGA struct
							byte *mapDataPtr = &currentMap->rawDataPtr[18] + 3 * currentMap->colorMapLength + currentMap->idLength;

							// Draw the map to the screen
							if (getGraphicsManager()->acquireSurface()) {
								PixMap *screenPtr = getGraphicsManager()->_backBuffer + 20496;
								byte *surfacePtr = (byte *)getGraphicsManager()->_screenSurface.getPixels() + 40992;

								for (int row = 0; row < currentMap->height; row++) {
									int remainingWidth = currentMap->width;

									// Decompress the RLE image
									while (remainingWidth > 0) {
										byte controlByte = *mapDataPtr;
										byte runLength = (controlByte & 0x7F) + 1;
										remainingWidth -= runLength;

										if (remainingWidth < 0) {
											abortCredits();
											break;
										}

										mapDataPtr++;

										if ((controlByte & 0x80) == 0) { // Non-repeated sequence
											for (int i = 0; i < runLength; i++) {
												if (*mapDataPtr)
													*screenPtr = mapPalette[*mapDataPtr];
												screenPtr++;
												mapDataPtr++;
											}
										} else { // Repeated sequence
											if (*mapDataPtr) { // Non-transparent pixels
												uint16 paletteValue = mapPalette[*mapDataPtr];
												for (int i = 0; i < runLength; i++) {
													*screenPtr++ = paletteValue;
												}
											} else { // Transparent pixels (skip)
												screenPtr += runLength;
											}

											mapDataPtr++;
										}
									}

									// Copy to surface
									memcpy(surfacePtr, &screenPtr[-currentMap->width], 2 * currentMap->width);
									surfacePtr += 1280;
									screenPtr += 640 - currentMap->width;
								}

								getGraphicsManager()->unlockSurface();
							}

							_doCredits = 2; // Signal that a map change occurred
						}
					}

					// Calculate scroll position for credits text
					int scrollPosition = ((scrollSpeed * (getSoundFrameCounter() - _savedFrameInterval)) >> 16) - creditsTextTga.height + textHeight + 355;

					// Scroll the credits...
					if (scrollPosition >= 1) {
						if (scrollPosition > textHeight)
							scrollPosition = textHeight;

						byte *textTgaData = nextTga;

						// Draw credits text to screen...
						if (getGraphicsManager()->acquireSurface()) {
							PixMap *surfacePtr = (PixMap *)((byte *)getGraphicsManager()->_screenSurface.getPixels() + 79920);
							int rowCounter = 355;
							PixMap *screenPtr = getGraphicsManager()->_backBuffer + 39960;

							while (rowCounter > 0) {
								memcpy(surfacePtr, screenPtr, 640);
								int remainingWidth = 320;

								// RLE decompression of credits text
								while (remainingWidth > 0) {
									byte controlByte = *textTgaData;
									byte runLength = (controlByte & 0x7F) + 1;
									remainingWidth -= runLength;

									if (remainingWidth < 0) {
										abortCredits();
										break;
									}

									textTgaData++;

									if ((controlByte & 0x80) == 0) { // Non-repeated sequence
										for (int i = 0; i < runLength; i++) {
											byte colorIndex = *textTgaData;
											if (colorIndex) {
												// Apply brightness/fade at top and bottom of screen
												if (rowCounter <= 339) {
													if (rowCounter > 16)
														*surfacePtr = tgaPalette[colorIndex];
													else
														*surfacePtr = (getGraphicsManager()->_brightnessData[4 - ((rowCounter - 1) >> 2)] & tgaPalette[colorIndex]) >> (4 - ((rowCounter - 1) >> 2));
												} else {
													*surfacePtr = (getGraphicsManager()->_brightnessData[4 - ((355 - rowCounter) >> 2)] & tgaPalette[colorIndex]) >> (4 - ((355 - rowCounter) >> 2));
												}
											}
											++surfacePtr;
											++textTgaData;
										}
									} else { // Repeated sequence
										byte colorIndex = *textTgaData;
										if (colorIndex) {
											// Apply brightness/fade at top and bottom of screen
											uint16 paletteValue;
											if (rowCounter <= 339) {
												if (rowCounter > 16)
													paletteValue = tgaPalette[colorIndex];
												else
													paletteValue = (getGraphicsManager()->_brightnessData[4 - ((rowCounter - 1) >> 2)] & tgaPalette[colorIndex]) >> (4 - ((rowCounter - 1) >> 2));
											} else {
												paletteValue = (getGraphicsManager()->_brightnessData[4 - ((355 - rowCounter) >> 2)] & tgaPalette[colorIndex]) >> (4 - ((355 - rowCounter) >> 2));
											}

											for (int i = 0; i < runLength; i++) {
												*surfacePtr++ = paletteValue;
											}
										} else { // Transparent pixels (skip)
											surfacePtr += runLength;
										}
										++textTgaData;
									}
								}

								screenPtr += 640;
								surfacePtr += 320;
								--rowCounter;
							}

							getGraphicsManager()->unlockSurface();
						}

						// Blit stuff to screen
						if (_doCredits == 2) {
							getGraphicsManager()->burstBox(
								getGraphicsManager()->_renderBox1.x,
								getGraphicsManager()->_renderBox1.y,
								getGraphicsManager()->_renderBox1.width,
								getGraphicsManager()->_renderBox1.height
							);

							_doCredits = 1;
						} else {
							getGraphicsManager()->burstBox(280, 62, 320, 355);
						}

						textHeight -= scrollPosition;

						// Check if we have more text to display
						if (textHeight > 0) {
							for (int i = 0; i < scrollPosition && textHeight > 0; i++) {
								int remainingWidth = 320;

								while (remainingWidth > 0) {
									byte controlByte = *nextTga;
									byte runLength = (controlByte & 0x7F) + 1;
									remainingWidth -= runLength;

									if (remainingWidth < 0) {
										abortCredits();
										break;
									}

									nextTga++;

									if ((controlByte & 0x80) == 0) // Non-repeated sequence
										nextTga += runLength;
									else // Repeated sequence
										nextTga++;
								}
							}
						} else {
							abortCredits();
						}
					}
				}
			}

			// Clean-up and exit...
			getGraphicsManager()->setMouseDrawable(oldCanDrawMouse);

			getGraphicsManager()->burstMouseArea();
			Slot *cacheSlot = getSoundManager()->_soundCache;
			if (cacheSlot) {
				do {
					if (cacheSlot->getTag() == kSoundTagLink)
						break;

					cacheSlot = cacheSlot->getNext();
				} while (cacheSlot);

				if (cacheSlot && cacheSlot->getBlockCount())
					cacheSlot->setFade(0);
			}

			getLogicManager()->fadeToBlack();

			if (nextTga)
				getMemoryManager()->lockFX();

			getMessageManager()->setEventHandle(kEventChannelMouse, _savedMouseEventHandle);
			getMessageManager()->setEventHandle(kEventChannelTimer, _savedTimerEventHandle);

			_doCredits = 0;

			if (creditsTextTga.rawDataPtr) {
				free(creditsTextTga.rawDataPtr);
				creditsTextTga.rawDataPtr = nullptr;
			}

			for (int i = 0; i < 35; i++) {
				if (mapTgas[i].rawDataPtr) {
					free(mapTgas[i].rawDataPtr);
					mapTgas[i].rawDataPtr = nullptr;
				}
			}
		}
	}
}

void LastExpressEngine::abortCredits() {
	_doCredits = 0;
}

void LastExpressEngine::creditsMouse(Event *event) {
	_cursorX = event->x;
	_cursorY = event->y;

	mouseSetRightClicked(false);
	
	if ((event->flags & kMouseFlagLeftDown) != 0)
		_savedFrameCounter = getSoundFrameCounter();

	if (_savedFrameCounter && (event->flags & kMouseFlagLeftUp) != 0) {
		_savedFrameInterval += getSoundFrameCounter() - _savedFrameCounter;
		_savedFrameCounter = 0;
	}

	if ((event->flags & kMouseFlagRightDown) != 0)
		abortCredits();
}

void LastExpressEngine::creditsTimer(Event *event) {
	setEventTickInternal(false);
}

int32 LastExpressEngine::readTGAIntoMemory(const char *filename, TGAHeader *tgaHeader) {
	int32 fileSize = 0;

	HPF *archive = getArchiveManager()->openHPF(filename);
	if (!archive) {
		warning("Error opening file \"%s\".  It probably doesn\'t exist or is write protected.", filename);
		abortCredits();

		return fileSize;
	}

	if (_doCredits) {
		fileSize = archive->size * MEM_PAGE_SIZE;

		byte *tgaRawData = (byte *)malloc(fileSize);

		assert(tgaRawData);

		getArchiveManager()->readHPF(archive, tgaRawData, archive->size);

		Common::SeekableReadStream *tgaReadStream = new Common::MemoryReadStream(tgaRawData, fileSize, DisposeAfterUse::NO);

		tgaHeader->idLength = tgaReadStream->readByte();
		tgaHeader->colorMapType = tgaReadStream->readByte();
		tgaHeader->imageType = tgaReadStream->readByte();
		tgaHeader->colorMapFirstEntryIndex = tgaReadStream->readUint16LE();
		tgaHeader->colorMapLength = tgaReadStream->readUint16LE();
		tgaHeader->colorMapEntrySize = tgaReadStream->readByte();
		tgaHeader->xOrigin = tgaReadStream->readUint16LE();
		tgaHeader->yOrigin = tgaReadStream->readUint16LE();
		tgaHeader->width = tgaReadStream->readUint16LE();
		tgaHeader->height = tgaReadStream->readUint16LE();
		tgaHeader->bitsPerPixel = tgaReadStream->readByte();
		tgaHeader->imageDescriptor = tgaReadStream->readByte();
		tgaHeader->rawDataPtr = tgaRawData;

		delete tgaReadStream;

		getArchiveManager()->closeHPF(archive);

		if ((tgaHeader->imageDescriptor & 0x10) != 0)
			abortCredits();

		if (tgaHeader->bitsPerPixel != 8)
			abortCredits();

		if (!tgaHeader->colorMapType)
			abortCredits();

		if (tgaHeader->colorMapEntrySize != 24)
			abortCredits();

		if ((tgaHeader->colorMapFirstEntryIndex >= 256 || tgaHeader->colorMapLength > 256 - tgaHeader->colorMapFirstEntryIndex)) {
			abortCredits();
		}
	}

	return fileSize;
}

void LastExpressEngine::constructPalette(TGAHeader *tgaHeader, uint16 *palette) {
	uint16 *paletteEntry;

	byte *colorMapData = tgaHeader->rawDataPtr + 18 + tgaHeader->idLength;
	int currentIndex = tgaHeader->colorMapFirstEntryIndex;

	if (currentIndex + tgaHeader->colorMapLength > currentIndex) {
		paletteEntry = &palette[currentIndex];

		// Read BGR components (as the TGA format stores them)...
		do {
			// The component are reduced to 5 bits...
			byte blue = colorMapData[0] >> 3;
			byte green = colorMapData[1] >> 3;
			byte red = colorMapData[2] >> 3;
			
			colorMapData += 3;

			 // Construct 16-bit color value in RGB555 format
			*paletteEntry = (red << 10) | (green << 5) | blue;

			paletteEntry++;
			currentIndex++;
		} while (tgaHeader->colorMapLength + tgaHeader->colorMapFirstEntryIndex > currentIndex);
	}

	getGraphicsManager()->modifyPalette(palette, 256);
}

bool LastExpressEngine::demoEnding(bool wonGame) {
	bool exitFlag = false;
	int frameIndex = 0;
	bool savedMouseState = getGraphicsManager()->canDrawMouse();

	const char backgroundNames[35][9] = {
		"CROSSING", "ABBOT", "ANCATH", "MORNING", "GUNS", "DRINKUP", "SERBSRES",
		"ANALXR13", "TATIANA", "KRONAN", "CONCERT", "AUDIENCE", "COUPLE", "RUSSIANS",
		"SPIES", "1017DOG", "CARRIAGE", "TYLEREGG", "TRNM2", "MAHMUD", "CATHMIL",
		"FRANCY", "ONROOF", "COPS2", "MILOSVES", "KAHINGUN", "1041KISS", "EVERYONE",
		"BONDAGE", "KILL", "HIGHFITE", "1315GUNS", "BOOM2", "ISTANBUL", "LASTSHOT"};

	if (wonGame) {
		getMessageManager()->setEventHandle(kEventChannelMouse, &LastExpressEngine::emptyHandler);
	} else {
		getMessageManager()->setEventHandle(kEventChannelMouse, &LastExpressEngine::demoEndingMouseWrapper);
	}

	getMessageManager()->setEventHandle(kEventChannelTimer, &LastExpressEngine::demoEndingTimerWrapper);
	getGraphicsManager()->setMouseDrawable(false);

	mouseSetRightClicked(false);

	if (getGraphicsManager()->acquireSurface()) {
		getGraphicsManager()->clear(getGraphicsManager()->_screenSurface, 0, 0, 640, 480);
		getGraphicsManager()->unlockSurface();
	}

	getGraphicsManager()->burstAll();
	getSoundManager()->playSoundFile("MUSSELL.SND", kSoundTypeNIS | kVolumeFull, 0, 0);

	while (!exitFlag && frameIndex < 35) {
		Slot *soundSlot = getSoundManager()->_soundCache;
		int frameDuration = 180;

		if (soundSlot) {
			while (soundSlot && soundSlot->getTag() != kSoundTagNIS)
				soundSlot = soundSlot->getNext();

			if (soundSlot)
				frameDuration = 2 * soundSlot->getBlockCount() / (35 - frameIndex);
		}

		int targetTime = getSoundFrameCounter() + frameDuration;
		int bgResult = getArchiveManager()->loadBG(backgroundNames[frameIndex]);

		if (bgResult < 0) {
			exitFlag = true;
			break;
		}

		TBM *renderBox = (bgResult == 0) ? &getGraphicsManager()->_renderBox1 : &getGraphicsManager()->_renderBox2;
		getGraphicsManager()->stepDissolve(renderBox);

		while (getSoundFrameCounter() < targetTime && !exitFlag) {
			if (wonGame) {
				if (mouseHasRightClicked()) {
					exitFlag = true;
				}
			} else {
				if (getMenu()->getEggTimerDelta())
					exitFlag = true;
			}

			if (!exitFlag) {
				bool haveMoreMessages = getMessageManager()->process();
				getSoundManager()->soundThread();
				getSubtitleManager()->subThread();

				// Only wait and handle events if we've processed all messages, unlike the original which had a separate thread for input...
				if (!haveMoreMessages)
					waitForTimer(5);
			}
		}

		frameIndex++;
	}

	getGraphicsManager()->setMouseDrawable(savedMouseState);
	getMenu()->setEggTimerDelta(DEMO_TIMEOUT);

	return exitFlag;
}

void LastExpressEngine::demoEndingMouse(Event *event) {
	if (event->flags || ABS<int32>((int32)event->x - _cursorX) > 5 || ABS<int32>((int32)event->y - _cursorY) > 5)
		getMenu()->setEggTimerDelta(DEMO_TIMEOUT);
}

void LastExpressEngine::demoEndingTimer(Event *event) {
	setEventTickInternal(false);
}

} // End of namespace LastExpress
