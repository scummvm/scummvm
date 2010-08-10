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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/transitions.h"
#include "sci/graphics/animate.h"

namespace Sci {

GfxAnimate::GfxAnimate(EngineState *state, GfxCache *cache, GfxPorts *ports, GfxPaint16 *paint16, GfxScreen *screen, GfxPalette *palette, GfxCursor *cursor, GfxTransitions *transitions)
	: _s(state), _cache(cache), _ports(ports), _paint16(paint16), _screen(screen), _palette(palette), _cursor(cursor), _transitions(transitions) {
	init();
}

GfxAnimate::~GfxAnimate() {
}

void GfxAnimate::init() {
	_lastCastData.clear();

	_ignoreFastCast = false;
	// fastCast object is not found in any SCI games prior SCI1
	if (getSciVersion() <= SCI_VERSION_01)
		_ignoreFastCast = true;
	// Also if fastCast object exists at gamestartup, we can assume that the interpreter doesnt do kAnimate aborts
	//  (found in Larry 1)
	if (getSciVersion() > SCI_VERSION_0_EARLY) {
		if (!_s->_segMan->findObjectByName("fastCast").isNull())
			_ignoreFastCast = true;
	}
}

void GfxAnimate::disposeLastCast() {
	_lastCastData.clear();
}

bool GfxAnimate::invoke(List *list, int argc, reg_t *argv) {
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 signal;

	while (curNode) {
		curObject = curNode->value;

		if (!_ignoreFastCast) {
			// Check if the game has a fastCast object set
			//  if we don't abort kAnimate processing, at least in kq5 there will be animation cels drawn into speech boxes.
			if (!_s->variables[VAR_GLOBAL][84].isNull()) {
				if (!strcmp(_s->_segMan->getObjectName(_s->variables[VAR_GLOBAL][84]), "fastCast"))
					return false;
			}
		}

		signal = readSelectorValue(_s->_segMan, curObject, SELECTOR(signal));
		if (!(signal & kSignalFrozen)) {
			// Call .doit method of that object
			invokeSelector(_s, curObject, SELECTOR(doit), argc, argv, 0);

			// If a game is being loaded, stop processing
			if (_s->abortScriptProcessing != kAbortNone || g_engine->shouldQuit())
				return true; // Stop processing

			// Lookup node again, since the nodetable it was in may have been reallocated.
			// The node might have been deallocated at this point (e.g. LSL2, room 42),
			// in which case the node reference will be null and the loop will stop below.
			// If the node is deleted from kDeleteKey, it won't have a successor node, thus
			// list processing will stop here (which is what SSCI does).
			curNode = _s->_segMan->lookupNode(curAddress, false);
		}

		if (curNode) {
			curAddress = curNode->succ;
			curNode = _s->_segMan->lookupNode(curAddress);
		}
	}
	return true;
}

bool sortHelper(const AnimateEntry &entry1, const AnimateEntry &entry2) {
	if (entry1.y == entry2.y) {
		// if both y and z are the same, use the order we were given originally
		//  this is needed for special cases like iceman room 35
		if (entry1.z == entry2.z)
			return entry1.givenOrderNo < entry2.givenOrderNo;
		else
			return entry1.z < entry2.z;
	}
	return entry1.y < entry2.y;
}

void GfxAnimate::makeSortedList(List *list) {
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	int16 listNr;

	// Clear lists
	_list.clear();
	_lastCastData.clear();

	// Fill the list
	for (listNr = 0; curNode != 0; listNr++) {
		AnimateEntry listEntry;
		const reg_t curObject = curNode->value;
		listEntry.object = curObject;

		// Get data from current object
		listEntry.givenOrderNo = listNr;
		listEntry.viewId = readSelectorValue(_s->_segMan, curObject, SELECTOR(view));
		listEntry.loopNo = readSelectorValue(_s->_segMan, curObject, SELECTOR(loop));
		listEntry.celNo = readSelectorValue(_s->_segMan, curObject, SELECTOR(cel));
		listEntry.paletteNo = readSelectorValue(_s->_segMan, curObject, SELECTOR(palette));
		listEntry.x = readSelectorValue(_s->_segMan, curObject, SELECTOR(x));
		listEntry.y = readSelectorValue(_s->_segMan, curObject, SELECTOR(y));
		listEntry.z = readSelectorValue(_s->_segMan, curObject, SELECTOR(z));
		listEntry.priority = readSelectorValue(_s->_segMan, curObject, SELECTOR(priority));
		listEntry.signal = readSelectorValue(_s->_segMan, curObject, SELECTOR(signal));
		if (getSciVersion() >= SCI_VERSION_1_1) {
			// Cel scaling
			listEntry.scaleSignal = readSelectorValue(_s->_segMan, curObject, SELECTOR(scaleSignal));
			if (listEntry.scaleSignal & kScaleSignalDoScaling) {
				listEntry.scaleX = readSelectorValue(_s->_segMan, curObject, SELECTOR(scaleX));
				listEntry.scaleY = readSelectorValue(_s->_segMan, curObject, SELECTOR(scaleY));
			} else {
				listEntry.scaleX = 128;
				listEntry.scaleY = 128;
			}
		} else {
			listEntry.scaleSignal = 0;
			listEntry.scaleX = 128;
			listEntry.scaleY = 128;
		}
		// listEntry.celRect is filled in AnimateFill()
		listEntry.showBitsFlag = false;

		_list.push_back(listEntry);

		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}

	// Possible TODO: As noted in the comment in sortHelper we actually
	// require a stable sorting algorithm here. Since Common::sort is not stable
	// at the time of writing this comment, we work around that in our ordering
	// comparator. If that changes in the future or we want to use some
	// stable sorting algorithm here, we should change that.
	// In that case we should test such changes intensively. A good place to test stable sort
	// is iceman, cupboard within the submarine. If sort isn't stable, the cupboard will be
	// half-open, half-closed. Of course that's just one of many special cases.

	// Now sort the list according y and z (descending)
	Common::sort(_list.begin(), _list.end(), sortHelper);
}

void GfxAnimate::fill(byte &old_picNotValid) {
	reg_t curObject;
	uint16 signal;
	GfxView *view = NULL;
	AnimateList::iterator it;
	const AnimateList::iterator end = _list.end();

	for (it = _list.begin(); it != end; ++it) {
		curObject = it->object;
		signal = it->signal;

		// Get the corresponding view
		view = _cache->getView(it->viewId);

		// adjust loop and cel, if any of those is invalid
		if (it->loopNo >= view->getLoopCount()) {
			it->loopNo = 0;
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(loop), it->loopNo);
		}
		if (it->celNo >= view->getCelCount(it->loopNo)) {
			it->celNo = 0;
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(cel), it->celNo);
		}

		// Process global scaling, if needed
		if (it->scaleSignal & kScaleSignalDoScaling) {
			if (it->scaleSignal & kScaleSignalGlobalScaling) {
				// Global scaling uses global var 2 and some other stuff to calculate scaleX/scaleY
				int16 maxScale = readSelectorValue(_s->_segMan, curObject, SELECTOR(maxScale));
				int16 celHeight = view->getHeight(it->loopNo, it->celNo);
				int16 maxCelHeight = (maxScale * celHeight) >> 7;
				reg_t globalVar2 = _s->variables[VAR_GLOBAL][2]; // current room object
				int16 vanishingY = readSelectorValue(_s->_segMan, globalVar2, SELECTOR(vanishingY));

				int16 fixedPortY = _ports->getPort()->rect.bottom - vanishingY;
				int16 fixedEntryY = it->y - vanishingY;
				if (!fixedEntryY)
					fixedEntryY = 1;

				if ((celHeight == 0) || (fixedPortY == 0))
					error("global scaling panic");

				it->scaleY = ( maxCelHeight * fixedEntryY ) / fixedPortY;
				it->scaleY = (it->scaleY * 128) / celHeight;

				it->scaleX = it->scaleY;

				// and set objects scale selectors
				writeSelectorValue(_s->_segMan, curObject, SELECTOR(scaleX), it->scaleX);
				writeSelectorValue(_s->_segMan, curObject, SELECTOR(scaleY), it->scaleY);
			}
		}

		if (!view->isScaleable()) {
			// Laura Bow 2 (especially floppy) depends on this, some views are not supposed to be scaleable
			//  this "feature" was removed in later versions of SCI1.1
			it->scaleSignal = 0;
			it->scaleY = it->scaleX = 128;
		}

		bool setNsRect = true;

		// Create rect according to coordinates and given cel
		if (it->scaleSignal & kScaleSignalDoScaling) {
			view->getCelScaledRect(it->loopNo, it->celNo, it->x, it->y, it->z, it->scaleX, it->scaleY, it->celRect);
			// when being scaled, only set nsRect, if object will get drawn
			if ((signal & kSignalHidden) && !(signal & kSignalAlwaysUpdate))
				setNsRect = false;
		} else {
			view->getCelRect(it->loopNo, it->celNo, it->x, it->y, it->z, it->celRect);
		}
		if (setNsRect) {
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(nsLeft), it->celRect.left);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(nsTop), it->celRect.top);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(nsRight), it->celRect.right);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(nsBottom), it->celRect.bottom);
		}

		// Calculate current priority according to y-coordinate
		if (!(signal & kSignalFixedPriority)) {
			it->priority = _ports->kernelCoordinateToPriority(it->y);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(priority), it->priority);
		}

		if (signal & kSignalNoUpdate) {
			if (signal & (kSignalForceUpdate | kSignalViewUpdated)
				|| (signal & kSignalHidden && !(signal & kSignalRemoveView))
				|| (!(signal & kSignalHidden) && signal & kSignalRemoveView)
				|| (signal & kSignalAlwaysUpdate))
				old_picNotValid++;
			signal &= ~kSignalStopUpdate;
		} else {
			if (signal & kSignalStopUpdate || signal & kSignalAlwaysUpdate)
				old_picNotValid++;
			signal &= ~kSignalForceUpdate;
		}
		it->signal = signal;
	}
}

void GfxAnimate::update() {
	reg_t curObject;
	uint16 signal;
	reg_t bitsHandle;
	Common::Rect rect;
	AnimateList::iterator it;
	const AnimateList::iterator end = _list.end();

	// Remove all no-update cels, if requested
	for (it = _list.reverse_begin(); it != end; --it) {
		curObject = it->object;
		signal = it->signal;

		if (signal & kSignalNoUpdate) {
			if (!(signal & kSignalRemoveView)) {
				bitsHandle = readSelector(_s->_segMan, curObject, SELECTOR(underBits));
				if (_screen->_picNotValid != 1) {
					_paint16->bitsRestore(bitsHandle);
					it->showBitsFlag = true;
				} else	{
					_paint16->bitsFree(bitsHandle);
				}
				writeSelectorValue(_s->_segMan, curObject, SELECTOR(underBits), 0);
			}
			signal &= ~kSignalForceUpdate;
			if (signal & kSignalViewUpdated)
				signal &= ~(kSignalViewUpdated | kSignalNoUpdate);
		} else if (signal & kSignalStopUpdate) {
			signal &= ~kSignalStopUpdate;
			signal |= kSignalNoUpdate;
		}
		it->signal = signal;
	}

	// Draw always-update cels
	for (it = _list.begin(); it != end; ++it) {
		curObject = it->object;
		signal = it->signal;

		if (signal & kSignalAlwaysUpdate) {
			// draw corresponding cel
			_paint16->drawCel(it->viewId, it->loopNo, it->celNo, it->celRect, it->priority, it->paletteNo, it->scaleX, it->scaleY);
			it->showBitsFlag = true;

			signal &= ~(kSignalStopUpdate | kSignalViewUpdated | kSignalNoUpdate | kSignalForceUpdate);
			if ((signal & kSignalIgnoreActor) == 0) {
				rect = it->celRect;
				rect.top = CLIP<int16>(_ports->kernelPriorityToCoordinate(it->priority) - 1, rect.top, rect.bottom - 1);
				_paint16->fillRect(rect, GFX_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
			it->signal = signal;
		}
	}

	// Saving background for all NoUpdate-cels
	for (it = _list.begin(); it != end; ++it) {
		curObject = it->object;
		signal = it->signal;

		if (signal & kSignalNoUpdate) {
			if (signal & kSignalHidden) {
				signal |= kSignalRemoveView;
			} else {
				signal &= ~kSignalRemoveView;
				if (signal & kSignalIgnoreActor)
					bitsHandle = _paint16->bitsSave(it->celRect, GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY);
				else
					bitsHandle = _paint16->bitsSave(it->celRect, GFX_SCREEN_MASK_ALL);
				writeSelector(_s->_segMan, curObject, SELECTOR(underBits), bitsHandle);
			}
			it->signal = signal;
		}
	}

	// Draw NoUpdate cels
	for (it = _list.begin(); it != end; ++it) {
		curObject = it->object;
		signal = it->signal;

		if (signal & kSignalNoUpdate && !(signal & kSignalHidden)) {
			// draw corresponding cel
			_paint16->drawCel(it->viewId, it->loopNo, it->celNo, it->celRect, it->priority, it->paletteNo, it->scaleX, it->scaleY);
			it->showBitsFlag = true;

			if (!(signal & kSignalIgnoreActor)) {
				rect = it->celRect;
				rect.top = CLIP<int16>(_ports->kernelPriorityToCoordinate(it->priority) - 1, rect.top, rect.bottom - 1);
				_paint16->fillRect(rect, GFX_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
		}
	}
}

void GfxAnimate::drawCels() {
	reg_t curObject;
	uint16 signal;
	reg_t bitsHandle;
	AnimateList::iterator it;
	const AnimateList::iterator end = _list.end();
	_lastCastData.clear();

	for (it = _list.begin(); it != end; ++it) {
		curObject = it->object;
		signal = it->signal;

		if (!(signal & (kSignalNoUpdate | kSignalHidden | kSignalAlwaysUpdate))) {
			// Save background
			bitsHandle = _paint16->bitsSave(it->celRect, GFX_SCREEN_MASK_ALL);
			writeSelector(_s->_segMan, curObject, SELECTOR(underBits), bitsHandle);

			// draw corresponding cel
			_paint16->drawCel(it->viewId, it->loopNo, it->celNo, it->celRect, it->priority, it->paletteNo, it->scaleX, it->scaleY);
			it->showBitsFlag = true;

			if (signal & kSignalRemoveView) {
				signal &= ~kSignalRemoveView;
			}
			it->signal = signal;

			// Remember that entry in lastCast
			_lastCastData.push_back(*it);
		}
	}
}

void GfxAnimate::updateScreen(byte oldPicNotValid) {
	reg_t curObject;
	uint16 signal;
	AnimateList::iterator it;
	const AnimateList::iterator end = _list.end();
	Common::Rect lsRect;
	Common::Rect workerRect;

	for (it = _list.begin(); it != end; ++it) {
		curObject = it->object;
		signal = it->signal;

		if (it->showBitsFlag || !(signal & (kSignalRemoveView | kSignalNoUpdate) ||
										(!(signal & kSignalRemoveView) && (signal & kSignalNoUpdate) && oldPicNotValid))) {
			lsRect.left = readSelectorValue(_s->_segMan, curObject, SELECTOR(lsLeft));
			lsRect.top = readSelectorValue(_s->_segMan, curObject, SELECTOR(lsTop));
			lsRect.right = readSelectorValue(_s->_segMan, curObject, SELECTOR(lsRight));
			lsRect.bottom = readSelectorValue(_s->_segMan, curObject, SELECTOR(lsBottom));

			workerRect = lsRect;
			workerRect.clip(it->celRect);

			if (!workerRect.isEmpty()) {
				workerRect = lsRect;
				workerRect.extend(it->celRect);
			} else {
				_paint16->bitsShow(lsRect);
				workerRect = it->celRect;
			}
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(lsLeft), it->celRect.left);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(lsTop), it->celRect.top);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(lsRight), it->celRect.right);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(lsBottom), it->celRect.bottom);
			// may get used for debugging
			//_paint16->frameRect(workerRect);
			_paint16->bitsShow(workerRect);

			if (signal & kSignalHidden) {
				it->signal |= kSignalRemoveView;
			}
		}
	}
	// use this for debug purposes
	// _screen->copyToScreen();
}

void GfxAnimate::restoreAndDelete(int argc, reg_t *argv) {
	reg_t curObject;
	uint16 signal;
	AnimateList::iterator it;
	const AnimateList::iterator end = _list.end();


	// This has to be done in a separate loop. At least in sq1 some .dispose
	// modifies FIXEDLOOP flag in signal for another object. In that case we
	// would overwrite the new signal with our version of the old signal.
	for (it = _list.begin(); it != end; ++it) {
		curObject = it->object;
		signal = it->signal;

		// Finally update signal
		writeSelectorValue(_s->_segMan, curObject, SELECTOR(signal), signal);
	}

	for (it = _list.reverse_begin(); it != end; --it) {
		curObject = it->object;
		// We read out signal here again, this is not by accident but to ensure
		// that we got an up-to-date signal
		signal = readSelectorValue(_s->_segMan, curObject, SELECTOR(signal));

		if ((signal & (kSignalNoUpdate | kSignalRemoveView)) == 0) {
			_paint16->bitsRestore(readSelector(_s->_segMan, curObject, SELECTOR(underBits)));
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(underBits), 0);
		}

		if (signal & kSignalDisposeMe) {
			// Call .delete_ method of that object
			invokeSelector(_s, curObject, SELECTOR(delete_), argc, argv, 0);
		}
	}
}

void GfxAnimate::reAnimate(Common::Rect rect) {
	if (!_lastCastData.empty()) {
		AnimateArray::iterator it;
		AnimateArray::iterator end = _lastCastData.end();
		for (it = _lastCastData.begin(); it != end; ++it) {
			it->castHandle = _paint16->bitsSave(it->celRect, GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY);
			_paint16->drawCel(it->viewId, it->loopNo, it->celNo, it->celRect, it->priority, it->paletteNo, it->scaleX, it->scaleY);
		}
		_paint16->bitsShow(rect);
		// restoring
		while (it != _lastCastData.begin()) {		// FIXME: HACK, this iterator use is not very safe
			it--;
			_paint16->bitsRestore(it->castHandle);
		}
	} else {
		_paint16->bitsShow(rect);
	}
}

void GfxAnimate::addToPicDrawCels() {
	reg_t curObject;
	GfxView *view = NULL;
	AnimateList::iterator it;
	const AnimateList::iterator end = _list.end();

	for (it = _list.begin(); it != end; ++it) {
		curObject = it->object;

		if (it->priority == -1)
			it->priority = _ports->kernelCoordinateToPriority(it->y);

		// Get the corresponding view
		view = _cache->getView(it->viewId);

		// Create rect according to coordinates and given cel
		view->getCelRect(it->loopNo, it->celNo, it->x, it->y, it->z, it->celRect);

		// draw corresponding cel
		_paint16->drawCel(it->viewId, it->loopNo, it->celNo, it->celRect, it->priority, it->paletteNo);
		if ((it->signal & kSignalIgnoreActor) == 0) {
			it->celRect.top = CLIP<int16>(_ports->kernelPriorityToCoordinate(it->priority) - 1, it->celRect.top, it->celRect.bottom - 1);
			_paint16->fillRect(it->celRect, GFX_SCREEN_MASK_CONTROL, 0, 0, 15);
		}
	}
}

void GfxAnimate::addToPicDrawView(GuiResourceId viewId, int16 loopNo, int16 celNo, int16 x, int16 y, int16 priority, int16 control) {
	GfxView *view = _cache->getView(viewId);
	Common::Rect celRect;

	if (priority == -1)
		priority = _ports->kernelCoordinateToPriority(y);

	// Create rect according to coordinates and given cel
	view->getCelRect(loopNo, celNo, x, y, 0, celRect);
	_paint16->drawCel(view, loopNo, celNo, celRect, priority, 0);

	if (control != -1) {
		celRect.top = CLIP<int16>(_ports->kernelPriorityToCoordinate(priority) - 1, celRect.top, celRect.bottom - 1);
		_paint16->fillRect(celRect, GFX_SCREEN_MASK_CONTROL, 0, 0, control);
	}
}


void GfxAnimate::animateShowPic() {
	Port *picPort = _ports->_picWind;
	Common::Rect picRect = picPort->rect;
	bool previousCursorState = _cursor->isVisible();

	if (previousCursorState)
		_cursor->kernelHide();
	// Adjust picRect to become relative to screen
	picRect.translate(picPort->left, picPort->top);
	_transitions->doit(picRect);
	if (previousCursorState)
		_cursor->kernelShow();
}

void GfxAnimate::kernelAnimate(reg_t listReference, bool cycle, int argc, reg_t *argv) {
	byte old_picNotValid = _screen->_picNotValid;

	if (getSciVersion() >= SCI_VERSION_1_1)
		_palette->palVaryUpdate();

	if (listReference.isNull()) {
		disposeLastCast();
		if (_screen->_picNotValid)
			animateShowPic();
		return;
	}

	List *list = _s->_segMan->lookupList(listReference);
	if (!list)
		error("kAnimate called with non-list as parameter");

	if (cycle) {
		if (!invoke(list, argc, argv))
			return;

		// Look up the list again, as it may have been modified
		list = _s->_segMan->lookupList(listReference);
	}

	Port *oldPort = _ports->setPort((Port *)_ports->_picWind);
	disposeLastCast();

	makeSortedList(list);
	fill(old_picNotValid);

	if (old_picNotValid) {
		// beginUpdate()/endUpdate() were introduced SCI1.
		// Calling those for SCI0 will work most of the time but breaks minor
		// stuff like percentage bar of qfg1ega at the character skill screen.
		if (getSciVersion() >= SCI_VERSION_1_EGA)
			_ports->beginUpdate(_ports->_picWind);
		update();
		if (getSciVersion() >= SCI_VERSION_1_EGA)
			_ports->endUpdate(_ports->_picWind);
	}

	drawCels();

	if (_screen->_picNotValid)
		animateShowPic();

	updateScreen(old_picNotValid);
	restoreAndDelete(argc, argv);

	// We update the screen here as well, some scenes like EQ1 credits run w/o calling kGetEvent thus we wouldn't update
	//  screen at all
	g_sci->getEventManager()->updateScreen();

	_ports->setPort(oldPort);


	// Now trigger speed throttler
	switch (_lastCastData.size()) {
	case 0:
		// No entries drawn -> no speed throttler triggering
		break;
	case 1: {
		
		// One entry drawn -> check if that entry was a speed benchmark view, if not enable speed throttler
		AnimateEntry *onlyCast = &_lastCastData[0];
		if ((onlyCast->viewId == 0) && (onlyCast->loopNo == 13) && (onlyCast->celNo == 0)) {
			// this one is used by jones talkie
			if ((onlyCast->celRect.height() == 8) && (onlyCast->celRect.width() == 8))
				return;
		}
		// first loop and first cel used?
		if ((onlyCast->loopNo == 0) && (onlyCast->celNo == 0)) {
			// and that cel has a known speed benchmark resolution
			int16 onlyHeight = onlyCast->celRect.height();
			int16 onlyWidth = onlyCast->celRect.width();
			if (((onlyWidth == 12) && (onlyHeight == 35)) || // regular benchmark view ("fred", "Speedy", "ego")
				((onlyWidth == 29) && (onlyHeight == 45)) || // King's Quest 5 french "fred"
				((onlyWidth == 1) && (onlyHeight == 1))) { // Laura Bow 2 Talkie
				// check further that there is only one cel in that view
				GfxView *onlyView = _cache->getView(onlyCast->viewId);
				if ((onlyView->getLoopCount() == 1) && (onlyView->getCelCount(0)))
					return;
			}
		}
		_s->_throttleTrigger = true;
		break;
	}
	default:
		// More than 1 entry drawn -> time for speed throttling
		_s->_throttleTrigger = true;
		break;
	}
}

void GfxAnimate::addToPicSetPicNotValid() {
	if (getSciVersion() <= SCI_VERSION_1_EARLY)
		_screen->_picNotValid = 1;
	else
		_screen->_picNotValid = 2;
}

void GfxAnimate::kernelAddToPicList(reg_t listReference, int argc, reg_t *argv) {
	List *list;

	_ports->setPort((Port *)_ports->_picWind);

	list = _s->_segMan->lookupList(listReference);
	if (!list)
		error("kAddToPic called with non-list as parameter");

	makeSortedList(list);
	addToPicDrawCels();

	addToPicSetPicNotValid();
}

void GfxAnimate::kernelAddToPicView(GuiResourceId viewId, int16 loopNo, int16 celNo, int16 x, int16 y, int16 priority, int16 control) {
	_ports->setPort((Port *)_ports->_picWind);
	addToPicDrawView(viewId, loopNo, celNo, x, y, priority, control);
	addToPicSetPicNotValid();
}

} // End of namespace Sci
