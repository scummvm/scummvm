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

#include "sci/engine/segment.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/transitions32.h"
#include "sci/sci.h"

namespace Sci {
static int dissolveSequences[2][20] = {
	/* SCI2.1early- */ { 3, 6, 12, 20, 48, 96, 184, 272, 576, 1280, 3232, 6912, 13568, 24576, 46080 },
	/* SCI2.1mid+ */ { 0, 0, 3, 6, 12, 20, 48, 96, 184, 272, 576, 1280, 3232, 6912, 13568, 24576, 46080, 73728, 132096, 466944 }
};
static int16 divisionsDefaults[2][16] = {
	/* SCI2.1early- */ { 1, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 40, 40, 101, 101 },
	/* SCI2.1mid+ */   { 1, 20, 20, 20, 20, 10, 10, 10, 10, 20, 20,  6, 10, 101, 101, 2 }
};

GfxTransitions32::GfxTransitions32(SegManager *segMan) :
	_segMan(segMan),
	_throttleState(0) {
	for (int i = 0; i < 236; i += 2) {
		_styleRanges[i] = 0;
		_styleRanges[i + 1] = -1;
	}
	for (int i = 236; i < ARRAYSIZE(_styleRanges); ++i) {
		_styleRanges[i] = 0;
	}

	if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
		_dissolveSequenceSeeds = dissolveSequences[0];
		_defaultDivisions = divisionsDefaults[0];
	} else {
		_dissolveSequenceSeeds = dissolveSequences[1];
		_defaultDivisions = divisionsDefaults[1];
	}
}

GfxTransitions32::~GfxTransitions32() {
	for (ShowStyleList::iterator it = _showStyles.begin(); it != _showStyles.end(); ++it) {
		deleteShowStyle(it);
	}
	_scrolls.clear();
}

void GfxTransitions32::throttle() {
	uint8 throttleTime;
	if (_throttleState == 2) {
		throttleTime = 34;
		_throttleState = 0;
	} else {
		throttleTime = 33;
		++_throttleState;
	}

	g_sci->getEngineState()->speedThrottler(throttleTime);
	g_sci->getEngineState()->_throttleTrigger = true;
}

#pragma mark -
#pragma mark Show styles

void GfxTransitions32::processShowStyles() {
	uint32 now = g_sci->getTickCount();

	bool continueProcessing;
	bool doFrameOut;
	do {
		continueProcessing = false;
		doFrameOut = false;
		ShowStyleList::iterator showStyle = _showStyles.begin();
		while (showStyle != _showStyles.end()) {
			bool finished = false;

			if (!showStyle->animate) {
				doFrameOut = true;
			}

			finished = processShowStyle(*showStyle, now);

			if (!finished) {
				continueProcessing = true;
			}

			if (finished && showStyle->processed) {
				showStyle = deleteShowStyle(showStyle);
			} else {
				showStyle = ++showStyle;
			}
		}

		if (g_engine->shouldQuit()) {
			return;
		}

		if (doFrameOut) {
			g_sci->_gfxFrameout->frameOut(true);
			throttle();
		}
	} while(continueProcessing && doFrameOut);
}

void GfxTransitions32::processEffects(PlaneShowStyle &showStyle) {
	switch(showStyle.type) {
	case kShowStyleHShutterOut:
		processHShutterOut(showStyle);
	break;
	case kShowStyleHShutterIn:
		processHShutterIn(showStyle);
	break;
	case kShowStyleVShutterOut:
		processVShutterOut(showStyle);
	break;
	case kShowStyleVShutterIn:
		processVShutterIn(showStyle);
	break;
	case kShowStyleWipeLeft:
		processWipeLeft(showStyle);
	break;
	case kShowStyleWipeRight:
		processWipeRight(showStyle);
	break;
	case kShowStyleWipeUp:
		processWipeUp(showStyle);
	break;
	case kShowStyleWipeDown:
		processWipeDown(showStyle);
	break;
	case kShowStyleIrisOut:
		processIrisOut(showStyle);
	break;
	case kShowStyleIrisIn:
		processIrisIn(showStyle);
	break;
	case kShowStyleDissolveNoMorph:
	case kShowStyleDissolve:
		processPixelDissolve(showStyle);
	break;
	case kShowStyleNone:
	case kShowStyleFadeOut:
	case kShowStyleFadeIn:
	case kShowStyleMorph:
	break;
	}
}

// TODO: 10-argument version is only in SCI3; argc checks are currently wrong for this version
// and need to be fixed in future
void GfxTransitions32::kernelSetShowStyle(const uint16 argc, const reg_t planeObj, const ShowStyleType type, const int16 seconds, const int16 back, const int16 priority, const int16 animate, const int16 frameOutNow, reg_t pFadeArray, int16 divisions, const int16 blackScreen) {

	bool hasDivisions = false;
	bool hasFadeArray = false;

	// KQ7 2.0b uses a mismatched version of the Styler script (SCI2.1early script
	// for SCI2.1mid engine), so the calls it makes to kSetShowStyle are wrong and
	// put `divisions` where `pFadeArray` is supposed to be
	if (getSciVersion() == SCI_VERSION_2_1_MIDDLE && g_sci->getGameId() == GID_KQ7) {
		hasDivisions = argc > 7;
		hasFadeArray = false;
		divisions = argc > 7 ? pFadeArray.toSint16() : -1;
		pFadeArray = NULL_REG;
	} else if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
		hasDivisions = argc > 7;
		hasFadeArray = false;
	} else if (getSciVersion() < SCI_VERSION_3) {
		hasDivisions = argc > 8;
		hasFadeArray = argc > 7;
	} else {
		hasDivisions = argc > 9;
		hasFadeArray = argc > 8;
	}

	bool isFadeUp;
	int16 color;
	if (back != -1) {
		isFadeUp = false;
		color = back;
	} else {
		isFadeUp = true;
		color = 0;
	}

	if ((getSciVersion() < SCI_VERSION_2_1_MIDDLE && g_sci->getGameId() != GID_KQ7 && type == 15) || type > 15) {
		error("Illegal show style %d for plane %04x:%04x", type, PRINT_REG(planeObj));
	}

	Plane *plane = g_sci->_gfxFrameout->getPlanes().findByObject(planeObj);
	if (plane == nullptr) {
		error("Plane %04x:%04x is not present in active planes list", PRINT_REG(planeObj));
	}

	bool createNewEntry = true;
	PlaneShowStyle *entry = findShowStyleForPlane(planeObj);
	if (entry != nullptr) {
		bool useExisting = true;

		if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
			useExisting = plane->_gameRect.width() == entry->width && plane->_gameRect.height() == entry->height;
		}

		if (useExisting) {
			useExisting = entry->divisions == (hasDivisions ? divisions : _defaultDivisions[type]);
		}

		if (useExisting) {
			createNewEntry = false;
			isFadeUp = true;
			entry->currentStep = 0;
		} else {
			isFadeUp = true;
			color = entry->color;
			deleteShowStyle(findIteratorForPlane(planeObj));
			entry = nullptr;
		}
	}

	if (type > 0) {
		if (createNewEntry) {
			entry = new PlaneShowStyle;
			// NOTE: SCI2.1 engine tests if allocation returned a null pointer
			// but then only avoids setting currentStep if this is so. Since
			// this is a nonsensical approach, we do not do that here
			entry->currentStep = 0;
			entry->processed = false;
			entry->divisions = hasDivisions ? divisions : _defaultDivisions[type];
			entry->plane = planeObj;
			entry->fadeColorRangesCount = 0;

			if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
				// for pixel dissolve
				entry->bitmap = NULL_REG;
				entry->bitmapScreenItem = nullptr;

				// for wipe
				entry->screenItems.clear();
				entry->width = plane->_gameRect.width();
				entry->height = plane->_gameRect.height();
			} else {
				entry->fadeColorRanges = nullptr;
				if (hasFadeArray) {
					// NOTE: SCI2.1mid engine does no check to verify that an array is
					// successfully retrieved, and SegMan will cause a fatal error
					// if we try to use a memory segment that is not an array
					SciArray<reg_t> *table = _segMan->lookupArray(pFadeArray);

					uint32 rangeCount = table->getSize();
					entry->fadeColorRangesCount = rangeCount;

					// NOTE: SCI engine code always allocates memory even if the range
					// table has no entries, but this does not really make sense, so
					// we avoid the allocation call in this case
					if (rangeCount > 0) {
						entry->fadeColorRanges = new uint16[rangeCount];
						for (size_t i = 0; i < rangeCount; ++i) {
							entry->fadeColorRanges[i] = table->getValue(i).toUint16();
						}
					}
				}
			}
		}

		// NOTE: The original engine had no nullptr check and would just crash
		// if it got to here
		if (entry == nullptr) {
			error("Cannot edit non-existing ShowStyle entry");
		}

		entry->fadeUp = isFadeUp;
		entry->color = color;
		entry->nextTick = g_sci->getTickCount();
		entry->type = type;
		entry->animate = animate;
		entry->delay = (seconds * 60 + entry->divisions - 1) / entry->divisions;

		if (entry->delay == 0) {
			error("ShowStyle has no duration");
		}

		if (frameOutNow) {
			// Creates a reference frame for the pixel dissolves to use
			g_sci->_gfxFrameout->frameOut(false);
		}

		if (createNewEntry) {
			if (getSciVersion() <= SCI_VERSION_2_1_EARLY) {
				switch (entry->type) {
				case kShowStyleIrisOut:
				case kShowStyleIrisIn:
					configure21EarlyIris(*entry, priority);
				break;
				case kShowStyleDissolve:
					configure21EarlyDissolve(*entry, priority, plane->_gameRect);
				break;
				default:
					// do nothing
				break;
				}
			}

			_showStyles.push_back(*entry);
			delete entry;
		}
	}
}

void GfxTransitions32::kernelSetPalStyleRange(const uint8 fromColor, const uint8 toColor) {
	if (toColor > fromColor) {
		return;
	}

	for (int i = fromColor; i <= toColor; ++i) {
		_styleRanges[i] = 0;
	}
}

PlaneShowStyle *GfxTransitions32::findShowStyleForPlane(const reg_t planeObj) {
	for (ShowStyleList::iterator it = _showStyles.begin(); it != _showStyles.end(); ++it) {
		if (it->plane == planeObj) {
			return &*it;
		}
	}

	return nullptr;
}

ShowStyleList::iterator GfxTransitions32::findIteratorForPlane(const reg_t planeObj) {
	ShowStyleList::iterator it;
	for (it = _showStyles.begin(); it != _showStyles.end(); ++it) {
		if (it->plane == planeObj) {
			break;
		}
	}

	return it;
}

ShowStyleList::iterator GfxTransitions32::deleteShowStyle(const ShowStyleList::iterator &showStyle) {
	switch (showStyle->type) {
	case kShowStyleDissolveNoMorph:
	case kShowStyleDissolve:
		if (getSciVersion() <= SCI_VERSION_2_1_EARLY) {
			_segMan->freeHunkEntry(showStyle->bitmap);
			g_sci->_gfxFrameout->deleteScreenItem(*showStyle->bitmapScreenItem);
		}
		break;
	case kShowStyleIrisOut:
	case kShowStyleIrisIn:
		if (getSciVersion() <= SCI_VERSION_2_1_EARLY) {
			for (uint i = 0; i < showStyle->screenItems.size(); ++i) {
				ScreenItem *screenItem = showStyle->screenItems[i];
				if (screenItem != nullptr) {
					g_sci->_gfxFrameout->deleteScreenItem(*screenItem);
				}
			}
		}
		break;
	case kShowStyleFadeIn:
	case kShowStyleFadeOut:
		if (getSciVersion() > SCI_VERSION_2_1_EARLY && showStyle->fadeColorRangesCount > 0) {
			delete[] showStyle->fadeColorRanges;
		}
		break;
	case kShowStyleNone:
	case kShowStyleMorph:
		// do nothing
		break;
	default:
		error("Unknown delete transition type %d", showStyle->type);
	}

	return _showStyles.erase(showStyle);
}

void GfxTransitions32::configure21EarlyIris(PlaneShowStyle &showStyle, const int16 priority) {
	showStyle.numEdges = 4;
	const int numScreenItems = showStyle.numEdges * showStyle.divisions;
	showStyle.screenItems.reserve(numScreenItems);

	CelInfo32 celInfo;
	celInfo.type = kCelTypeColor;
	celInfo.color = showStyle.color;

	const int width = showStyle.width;
	const int height = showStyle.height;
	const int divisions = showStyle.divisions;

	for (int i = 0; i < divisions; ++i) {
		Common::Rect rect;

		// Top
		rect.left = (width * i) / (2 * divisions);
		rect.top = (height * i) / (2 * divisions);
		rect.right = width - rect.left;
		rect.bottom = (height + 1) * (i + 1) / (2 * divisions);
		const int16 topTop = rect.top;
		const int16 topBottom = rect.bottom;

		showStyle.screenItems.push_back(new ScreenItem(showStyle.plane, celInfo, rect));
		showStyle.screenItems.back()->_priority = priority;
		showStyle.screenItems.back()->_fixedPriority = true;

		// Bottom
		rect.top = height - rect.bottom;
		rect.bottom = height - topTop;
		const int16 bottomTop = rect.top;

		showStyle.screenItems.push_back(new ScreenItem(showStyle.plane, celInfo, rect));
		showStyle.screenItems.back()->_priority = priority;
		showStyle.screenItems.back()->_fixedPriority = true;

		// Left
		rect.top = topBottom;
		rect.right = (width + 1) * (i + 1) / (2 * divisions);
		rect.bottom = bottomTop;
		const int16 leftLeft = rect.left;

		showStyle.screenItems.push_back(new ScreenItem(showStyle.plane, celInfo, rect));
		showStyle.screenItems.back()->_priority = priority;
		showStyle.screenItems.back()->_fixedPriority = true;

		// Right
		rect.left = width - rect.right;
		rect.right = width - leftLeft;

		showStyle.screenItems.push_back(new ScreenItem(showStyle.plane, celInfo, rect));
		showStyle.screenItems.back()->_priority = priority;
		showStyle.screenItems.back()->_fixedPriority = true;
	}

	if (showStyle.fadeUp) {
		for (int i = 0; i < numScreenItems; ++i) {
			g_sci->_gfxFrameout->addScreenItem(*showStyle.screenItems[i]);
		}
	}
}

void GfxTransitions32::configure21EarlyDissolve(PlaneShowStyle &showStyle, const int16 priority, const Common::Rect &gameRect) {

	BitmapResource bitmap(_segMan, showStyle.width, showStyle.height, kDefaultSkipColor, 0, 0, kLowResX, kLowResY, 0, false, false);

	showStyle.bitmap = bitmap.getObject();

	const Buffer &source = g_sci->_gfxFrameout->getCurrentBuffer();
	Buffer target(showStyle.width, showStyle.height, bitmap.getPixels());

	target.fillRect(Common::Rect(bitmap.getWidth(), bitmap.getHeight()), kDefaultSkipColor);
	target.copyRectToSurface(source, 0, 0, gameRect);

	CelInfo32 celInfo;
	celInfo.type = kCelTypeMem;
	celInfo.bitmap = bitmap.getObject();

	showStyle.bitmapScreenItem = new ScreenItem(showStyle.plane, celInfo, Common::Point(0, 0), ScaleInfo());
	showStyle.bitmapScreenItem->_priority = priority;
	showStyle.bitmapScreenItem->_fixedPriority = true;

	g_sci->_gfxFrameout->addScreenItem(*showStyle.bitmapScreenItem);
}

bool GfxTransitions32::processShowStyle(PlaneShowStyle &showStyle, uint32 now) {
	if (showStyle.nextTick >= now && showStyle.animate) {
		return false;
	}

	switch (showStyle.type) {
	default:
	case kShowStyleNone:
		return processNone(showStyle);
	case kShowStyleHShutterOut:
	case kShowStyleHShutterIn:
	case kShowStyleVShutterOut:
	case kShowStyleVShutterIn:
	case kShowStyleWipeLeft:
	case kShowStyleWipeRight:
	case kShowStyleWipeUp:
	case kShowStyleWipeDown:
	case kShowStyleDissolveNoMorph:
	case kShowStyleMorph:
		return processMorph(showStyle);
	case kShowStyleDissolve:
		if (getSciVersion() > SCI_VERSION_2_1_EARLY) {
			return processMorph(showStyle);
		} else {
			return processPixelDissolve(showStyle);
		}
	case kShowStyleIrisOut:
		if (getSciVersion() > SCI_VERSION_2_1_EARLY) {
			return processMorph(showStyle);
		} else {
			return processIrisOut(showStyle);
		}
	case kShowStyleIrisIn:
		if (getSciVersion() > SCI_VERSION_2_1_EARLY) {
			return processMorph(showStyle);
		} else {
			return processIrisIn(showStyle);
		}
	case kShowStyleFadeOut:
		return processFade(-1, showStyle);
	case kShowStyleFadeIn:
		return processFade(1, showStyle);
	}
}

bool GfxTransitions32::processNone(PlaneShowStyle &showStyle) {
	if (showStyle.fadeUp) {
		g_sci->_gfxPalette32->setFade(100, 0, 255);
	} else {
		g_sci->_gfxPalette32->setFade(0, 0, 255);
	}

	showStyle.processed = true;
	return true;
}

void GfxTransitions32::processHShutterOut(PlaneShowStyle &showStyle) {
	error("HShutterOut is not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
}

void GfxTransitions32::processHShutterIn(PlaneShowStyle &showStyle) {
	error("HShutterIn is not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
}

void GfxTransitions32::processVShutterOut(PlaneShowStyle &showStyle) {
	error("VShutterOut is not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
}

void GfxTransitions32::processVShutterIn(PlaneShowStyle &showStyle) {
	error("VShutterIn is not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
}

void GfxTransitions32::processWipeLeft(PlaneShowStyle &showStyle) {
	error("WipeLeft is not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
}

void GfxTransitions32::processWipeRight(PlaneShowStyle &showStyle) {
	error("WipeRight is not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
}

void GfxTransitions32::processWipeUp(PlaneShowStyle &showStyle) {
	error("WipeUp is not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
}

void GfxTransitions32::processWipeDown(PlaneShowStyle &showStyle) {
	error("WipeDown is not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
}

bool GfxTransitions32::processIrisOut(PlaneShowStyle &showStyle) {
	if (getSciVersion() > SCI_VERSION_2_1_EARLY) {
		error("IrisOut is not known to be used by any SCI2.1mid+ game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
	}

	return processWipe(-1, showStyle);
}

bool GfxTransitions32::processIrisIn(PlaneShowStyle &showStyle) {
	if (getSciVersion() > SCI_VERSION_2_1_EARLY) {
		error("IrisIn is not known to be used by any SCI2.1mid+ game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
	}

	return processWipe(1, showStyle);
}

void GfxTransitions32::processDissolveNoMorph(PlaneShowStyle &showStyle) {
	error("DissolveNoMorph is not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
}

inline int bitWidth(int number) {
	int width = 0;
	while (number != 0) {
		number >>= 1;
		width += 1;
	}
	return width;
}

bool GfxTransitions32::processPixelDissolve(PlaneShowStyle &showStyle) {
	if (getSciVersion() > SCI_VERSION_2_1_EARLY) {
		return processPixelDissolve21Mid(showStyle);
	} else {
		return processPixelDissolve21Early(showStyle);
	}
}

bool GfxTransitions32::processPixelDissolve21Early(PlaneShowStyle &showStyle) {
	bool unchanged = true;

	BitmapResource bitmap(showStyle.bitmap);
	Buffer buffer(showStyle.width, showStyle.height, bitmap.getPixels());

	uint32 numPixels = showStyle.width * showStyle.height;
	uint32 numPixelsPerDivision = (numPixels + showStyle.divisions) / showStyle.divisions;

	uint32 index;
	if (showStyle.currentStep == 0) {
		int i = 0;
		index = numPixels;
		if (index != 1) {
			for (;;) {
				index >>= 1;
				if (index == 1) {
					break;
				}
				i++;
			}
		}

		showStyle.dissolveMask = _dissolveSequenceSeeds[i];
		index = 53427;

		showStyle.firstPixel = index;
		showStyle.pixel = index;
	} else {
		index = showStyle.pixel;
		for (;;) {
			if (index & 1) {
				index >>= 1;
				index ^= showStyle.dissolveMask;
			} else {
				index >>= 1;
			}

			if (index < numPixels) {
				break;
			}
		}

		if (index == showStyle.firstPixel) {
			index = 0;
		}
	}

	if (showStyle.currentStep < showStyle.divisions) {
		for (uint32 i = 0; i < numPixelsPerDivision; ++i) {
			*(byte *)buffer.getBasePtr(index % showStyle.width, index / showStyle.width) = showStyle.color;

			for (;;) {
				if (index & 1) {
					index >>= 1;
					index ^= showStyle.dissolveMask;
				} else {
					index >>= 1;
				}

				if (index < numPixels) {
					break;
				}
			}

			if (index == showStyle.firstPixel) {
				buffer.fillRect(Common::Rect(0, 0, showStyle.width, showStyle.height), showStyle.color);
				break;
			}
		}

		showStyle.pixel = index;
		showStyle.nextTick += showStyle.delay;
		++showStyle.currentStep;
		unchanged = false;
		if (showStyle.bitmapScreenItem->_created == 0) {
			showStyle.bitmapScreenItem->_updated = g_sci->_gfxFrameout->getScreenCount();
		}
	}

	if ((showStyle.currentStep >= showStyle.divisions) && unchanged) {
		if (showStyle.fadeUp) {
			showStyle.processed = true;
		}

		return true;
	}

	return false;
}

bool GfxTransitions32::processPixelDissolve21Mid(PlaneShowStyle &showStyle) {
	// SQ6 room 530

	Plane* plane = g_sci->_gfxFrameout->getVisiblePlanes().findByObject(showStyle.plane);
	const Common::Rect &screenRect = plane->_screenRect;
	Common::Rect rect;

	const int planeWidth = screenRect.width();
	const int planeHeight = screenRect.height();
	const int divisions = showStyle.divisions;
	const int width = planeWidth / divisions + ((planeWidth % divisions) ? 1 : 0);
	const int height = planeHeight / divisions + ((planeHeight % divisions) ? 1 : 0);

	const uint32 mask = _dissolveSequenceSeeds[bitWidth(width * height - 1)];
	int seq = 1;

	uint iteration = 0;
	const uint numIterationsPerTick = (width * height + divisions) / divisions;

	do {
		int row = seq / width;
		int col = seq % width;

		if (row < height) {
			if (row == height && (planeHeight % divisions)) {
				if (col == width && (planeWidth % divisions)) {
					rect.left = col * divisions;
					rect.top = row * divisions;
					rect.right = col * divisions + (planeWidth % divisions);
					rect.bottom = row * divisions + (planeHeight % divisions);
					rect.clip(screenRect);
					g_sci->_gfxFrameout->showRect(rect);
				} else {
					rect.left = col * divisions;
					rect.top = row * divisions;
					rect.right = col * divisions * 2;
					rect.bottom = row * divisions + (planeHeight % divisions);
					rect.clip(screenRect);
					g_sci->_gfxFrameout->showRect(rect);
				}
			} else {
				if (col == width && (planeWidth % divisions)) {
					rect.left = col * divisions;
					rect.top = row * divisions;
					rect.right = col * divisions + (planeWidth % divisions) + 1;
					rect.bottom = row * divisions * 2 + 1;
					rect.clip(screenRect);
					g_sci->_gfxFrameout->showRect(rect);
				} else {
					rect.left = col * divisions;
					rect.top = row * divisions;
					rect.right = col * divisions * 2 + 1;
					rect.bottom = row * divisions * 2 + 1;
					rect.clip(screenRect);
					g_sci->_gfxFrameout->showRect(rect);
				}
			}
		}

		if (seq & 1) {
			seq = (seq >> 1) ^ mask;
		} else {
			seq >>= 1;
		}

		if (++iteration == numIterationsPerTick) {
			throttle();
			iteration = 0;
		}
	} while(seq != 1 && !g_engine->shouldQuit());

	rect.left = screenRect.left;
	rect.top = screenRect.top;
	rect.right = divisions + screenRect.left;
	rect.bottom = divisions + screenRect.bottom;
	rect.clip(screenRect);
	g_sci->_gfxFrameout->showRect(rect);
	throttle();

	g_sci->_gfxFrameout->showRect(screenRect);
	return true;
}

bool GfxTransitions32::processFade(const int8 direction, PlaneShowStyle &showStyle) {
	bool unchanged = true;
	if (showStyle.currentStep < showStyle.divisions) {
		int percent;
		if (direction <= 0) {
			percent = showStyle.divisions - showStyle.currentStep - 1;
		} else {
			percent = showStyle.currentStep;
		}

		percent *= 100;
		percent /= showStyle.divisions - 1;

		if (showStyle.fadeColorRangesCount > 0) {
			for (int i = 0, len = showStyle.fadeColorRangesCount; i < len; i += 2) {
				g_sci->_gfxPalette32->setFade(percent, showStyle.fadeColorRanges[i], showStyle.fadeColorRanges[i + 1]);
			}
		} else {
			g_sci->_gfxPalette32->setFade(percent, 0, 255);
		}

		++showStyle.currentStep;
		showStyle.nextTick += showStyle.delay;
		unchanged = false;
	}

	if (showStyle.currentStep >= showStyle.divisions && unchanged) {
		if (direction > 0) {
			showStyle.processed = true;
		}

		return true;
	}

	return false;
}

bool GfxTransitions32::processMorph(PlaneShowStyle &showStyle) {
	g_sci->_gfxFrameout->palMorphFrameOut(_styleRanges, &showStyle);
	showStyle.processed = true;
	return true;
}

bool GfxTransitions32::processWipe(const int8 direction, PlaneShowStyle &showStyle) {
	bool unchanged = true;
	if (showStyle.currentStep < showStyle.divisions) {
		int index;
		if (direction > 0) {
			index = showStyle.currentStep;
		} else {
			index = showStyle.divisions - showStyle.currentStep - 1;
		}

		index *= showStyle.numEdges;
		for (int i = 0; i < showStyle.numEdges; ++i) {
			ScreenItem *screenItem = showStyle.screenItems[index + i];
			if (showStyle.fadeUp) {
				g_sci->_gfxFrameout->deleteScreenItem(*screenItem);
				showStyle.screenItems[index + i] = nullptr;
			} else {
				g_sci->_gfxFrameout->addScreenItem(*screenItem);
			}
		}

		++showStyle.currentStep;
		showStyle.nextTick += showStyle.delay;
		unchanged = false;
	}

	if (showStyle.currentStep >= showStyle.divisions && unchanged) {
		if (showStyle.fadeUp) {
			showStyle.processed = true;
		}

		return true;
	}

	return false;
}

#pragma mark -
#pragma mark Scrolls

void GfxTransitions32::processScrolls() {
	for (ScrollList::iterator it = _scrolls.begin(); it != _scrolls.end(); ) {
		bool finished = processScroll(*it);
		if (finished) {
			it = _scrolls.erase(it);
		} else {
			++it;
		}
	}

	throttle();
}

void GfxTransitions32::kernelSetScroll(const reg_t planeId, const int16 deltaX, const int16 deltaY, const GuiResourceId pictureId, const bool animate, const bool mirrorX) {

	for (ScrollList::const_iterator it = _scrolls.begin(); it != _scrolls.end(); ++it) {
		if (it->plane == planeId) {
			error("Scroll already exists on plane %04x:%04x", PRINT_REG(planeId));
		}
	}

	if (!deltaX && !deltaY) {
		error("kSetScroll: Scroll has no movement");
	}

	if (deltaX && deltaY) {
		error("kSetScroll: Cannot scroll in two dimensions");
	}

	PlaneScroll *scroll = new PlaneScroll;
	scroll->plane = planeId;
	scroll->x = 0;
	scroll->y = 0;
	scroll->deltaX = deltaX;
	scroll->deltaY = deltaY;
	scroll->newPictureId = pictureId;
	scroll->animate = animate;
	scroll->startTick = g_sci->getTickCount();

	Plane *plane = g_sci->_gfxFrameout->getPlanes().findByObject(planeId);
	if (plane == nullptr) {
		error("kSetScroll: Plane %04x:%04x not found", PRINT_REG(planeId));
	}

	Plane *visiblePlane = g_sci->_gfxFrameout->getPlanes().findByObject(planeId);
	if (visiblePlane == nullptr) {
		error("kSetScroll: Visible plane %04x:%04x not found", PRINT_REG(planeId));
	}

	const Common::Rect &gameRect = visiblePlane->_gameRect;
	Common::Point picOrigin;

	if (deltaX) {
		picOrigin.y = 0;

		if (deltaX > 0) {
			scroll->x = picOrigin.x = -gameRect.width();
		} else {
			scroll->x = picOrigin.x = gameRect.width();
		}
	} else {
		picOrigin.x = 0;

		if (deltaY > 0) {
			scroll->y = picOrigin.y = -gameRect.height();
		} else {
			scroll->y = picOrigin.y = gameRect.height();
		}
	}

	scroll->oldPictureId = plane->addPic(pictureId, picOrigin, mirrorX);

	if (animate) {
		_scrolls.push_front(*scroll);
	} else {
		bool finished = false;
		while (!finished && !g_engine->shouldQuit()) {
			finished = processScroll(*scroll);
			g_sci->_gfxFrameout->frameOut(true);
			throttle();
		}
		delete scroll;
	}
}

bool GfxTransitions32::processScroll(PlaneScroll &scroll) {
	bool finished = false;
	uint32 now = g_sci->getTickCount();
	if (scroll.startTick >= now) {
		return false;
	}

	int deltaX = scroll.deltaX;
	int deltaY = scroll.deltaY;
	if (((scroll.x + deltaX) * scroll.y) <= 0) {
		deltaX = -scroll.x;
	}
	if (((scroll.y + deltaY) * scroll.y) <= 0) {
		deltaY = -scroll.y;
	}

	scroll.x += deltaX;
	scroll.y += deltaY;

	Plane *plane = g_sci->_gfxFrameout->getPlanes().findByObject(scroll.plane);

	if ((scroll.x == 0) && (scroll.y == 0)) {
		plane->deletePic(scroll.oldPictureId, scroll.newPictureId);
		finished = true;
	}

	plane->scrollScreenItems(deltaX, deltaY, true);

	return finished;
}

} // End of namespace Sci
