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

			// Lookup node again, since the nodetable it was in may have been reallocated
			curNode = _s->_segMan->lookupNode(curAddress);
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

	// Now sort the list according y and z (descending)
	Common::sort(_list.begin(), _list.end(), sortHelper);
}

void GfxAnimate::fill(byte &old_picNotValid) {
	reg_t curObject;
	uint16 signal;
	GfxView *view = NULL;
	AnimateList::iterator listEntry;
	const AnimateList::iterator end = _list.end();

	for (listEntry = _list.begin(); listEntry != end; ++listEntry) {
		curObject = listEntry->object;

		// Get the corresponding view
		view = _cache->getView(listEntry->viewId);

		// adjust loop and cel, if any of those is invalid
		if (listEntry->loopNo >= view->getLoopCount()) {
			listEntry->loopNo = 0;
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(loop), listEntry->loopNo);
		}
		if (listEntry->celNo >= view->getCelCount(listEntry->loopNo)) {
			listEntry->celNo = 0;
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(cel), listEntry->celNo);
		}

		// Process global scaling, if needed
		if (listEntry->scaleSignal & kScaleSignalDoScaling) {
			if (listEntry->scaleSignal & kScaleSignalGlobalScaling) {
				// Global scaling uses global var 2 and some other stuff to calculate scaleX/scaleY
				int16 maxScale = readSelectorValue(_s->_segMan, curObject, SELECTOR(maxScale));
				int16 celHeight = view->getHeight(listEntry->loopNo, listEntry->celNo);
				int16 maxCelHeight = (maxScale * celHeight) >> 7;
				reg_t globalVar2 = _s->variables[VAR_GLOBAL][2]; // current room object
				int16 vanishingY = readSelectorValue(_s->_segMan, globalVar2, SELECTOR(vanishingY));

				int16 fixedPortY = _ports->getPort()->rect.bottom - vanishingY;
				int16 fixedEntryY = listEntry->y - vanishingY;
				if (!fixedEntryY)
					fixedEntryY = 1;

				if ((celHeight == 0) || (fixedPortY == 0))
					error("global scaling panic");

				listEntry->scaleY = ( maxCelHeight * fixedEntryY ) / fixedPortY;
				listEntry->scaleY = (listEntry->scaleY * 128) / celHeight;

				listEntry->scaleX = listEntry->scaleY;

				// and set objects scale selectors
				writeSelectorValue(_s->_segMan, curObject, SELECTOR(scaleX), listEntry->scaleX);
				writeSelectorValue(_s->_segMan, curObject, SELECTOR(scaleY), listEntry->scaleY);
			}
		}

		// Create rect according to coordinates and given cel
		if (listEntry->scaleSignal & kScaleSignalDoScaling) {
			view->getCelScaledRect(listEntry->loopNo, listEntry->celNo, listEntry->x, listEntry->y, listEntry->z, listEntry->scaleX, listEntry->scaleY, listEntry->celRect);
		} else {
			view->getCelRect(listEntry->loopNo, listEntry->celNo, listEntry->x, listEntry->y, listEntry->z, listEntry->celRect);
		}
		writeSelectorValue(_s->_segMan, curObject, SELECTOR(nsLeft), listEntry->celRect.left);
		writeSelectorValue(_s->_segMan, curObject, SELECTOR(nsTop), listEntry->celRect.top);
		writeSelectorValue(_s->_segMan, curObject, SELECTOR(nsRight), listEntry->celRect.right);
		writeSelectorValue(_s->_segMan, curObject, SELECTOR(nsBottom), listEntry->celRect.bottom);

		signal = listEntry->signal;

		// Calculate current priority according to y-coordinate
		if (!(signal & kSignalFixedPriority)) {
			listEntry->priority = _ports->kernelCoordinateToPriority(listEntry->y);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(priority), listEntry->priority);
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
		listEntry->signal = signal;
	}
}

void GfxAnimate::update() {
	reg_t curObject;
	uint16 signal;
	reg_t bitsHandle;
	Common::Rect rect;
	AnimateList::iterator listEntry;
	const AnimateList::iterator end = _list.end();

	// Remove all no-update cels, if requested
	for (listEntry = _list.reverse_begin(); listEntry != end; --listEntry) {
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalNoUpdate) {
			if (!(signal & kSignalRemoveView)) {
				bitsHandle = readSelector(_s->_segMan, curObject, SELECTOR(underBits));
				if (_screen->_picNotValid != 1) {
					_paint16->bitsRestore(bitsHandle);
					listEntry->showBitsFlag = true;
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
		listEntry->signal = signal;
	}

	// Draw always-update cels
	for (listEntry = _list.begin(); listEntry != end; ++listEntry) {
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalAlwaysUpdate) {
			// draw corresponding cel
			_paint16->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo, listEntry->scaleX, listEntry->scaleY);
			listEntry->showBitsFlag = true;

			signal &= ~(kSignalStopUpdate | kSignalViewUpdated | kSignalNoUpdate | kSignalForceUpdate);
			if ((signal & kSignalIgnoreActor) == 0) {
				rect = listEntry->celRect;
				rect.top = CLIP<int16>(_ports->kernelPriorityToCoordinate(listEntry->priority) - 1, rect.top, rect.bottom - 1);
				_paint16->fillRect(rect, GFX_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
			listEntry->signal = signal;
		}
	}

	// Saving background for all NoUpdate-cels
	for (listEntry = _list.begin(); listEntry != end; ++listEntry) {
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalNoUpdate) {
			if (signal & kSignalHidden) {
				signal |= kSignalRemoveView;
			} else {
				signal &= ~kSignalRemoveView;
				if (signal & kSignalIgnoreActor)
					bitsHandle = _paint16->bitsSave(listEntry->celRect, GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY);
				else
					bitsHandle = _paint16->bitsSave(listEntry->celRect, GFX_SCREEN_MASK_ALL);
				writeSelector(_s->_segMan, curObject, SELECTOR(underBits), bitsHandle);
			}
			listEntry->signal = signal;
		}
	}

	// Draw NoUpdate cels
	for (listEntry = _list.begin(); listEntry != end; ++listEntry) {
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalNoUpdate && !(signal & kSignalHidden)) {
			// draw corresponding cel
			_paint16->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo, listEntry->scaleX, listEntry->scaleY);
			listEntry->showBitsFlag = true;

			if (!(signal & kSignalIgnoreActor)) {
				rect = listEntry->celRect;
				rect.top = CLIP<int16>(_ports->kernelPriorityToCoordinate(listEntry->priority) - 1, rect.top, rect.bottom - 1);
				_paint16->fillRect(rect, GFX_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
		}
	}
}

void GfxAnimate::drawCels() {
	reg_t curObject;
	uint16 signal;
	reg_t bitsHandle;
	AnimateList::iterator listEntry;
	const AnimateList::iterator end = _list.end();
	_lastCastData.clear();

	for (listEntry = _list.begin(); listEntry != end; ++listEntry) {
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (!(signal & (kSignalNoUpdate | kSignalHidden | kSignalAlwaysUpdate))) {
			// Save background
			bitsHandle = _paint16->bitsSave(listEntry->celRect, GFX_SCREEN_MASK_ALL);
			writeSelector(_s->_segMan, curObject, SELECTOR(underBits), bitsHandle);

			// draw corresponding cel
			_paint16->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo, listEntry->scaleX, listEntry->scaleY);
			listEntry->showBitsFlag = true;

			if (signal & kSignalRemoveView) {
				signal &= ~kSignalRemoveView;
			}
			listEntry->signal = signal;

			// Remember that entry in lastCast
			_lastCastData.push_back(*listEntry);
		}
	}
}

void GfxAnimate::updateScreen(byte oldPicNotValid) {
	reg_t curObject;
	uint16 signal;
	AnimateList::iterator listEntry;
	const AnimateList::iterator end = _list.end();
	Common::Rect lsRect;
	Common::Rect workerRect;

	for (listEntry = _list.begin(); listEntry != end; ++listEntry) {
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (listEntry->showBitsFlag || !(signal & (kSignalRemoveView | kSignalNoUpdate) ||
										(!(signal & kSignalRemoveView) && (signal & kSignalNoUpdate) && oldPicNotValid))) {
			lsRect.left = readSelectorValue(_s->_segMan, curObject, SELECTOR(lsLeft));
			lsRect.top = readSelectorValue(_s->_segMan, curObject, SELECTOR(lsTop));
			lsRect.right = readSelectorValue(_s->_segMan, curObject, SELECTOR(lsRight));
			lsRect.bottom = readSelectorValue(_s->_segMan, curObject, SELECTOR(lsBottom));

			workerRect = lsRect;
			workerRect.clip(listEntry->celRect);

			if (!workerRect.isEmpty()) {
				workerRect = lsRect;
				workerRect.extend(listEntry->celRect);
			} else {
				_paint16->bitsShow(lsRect);
				workerRect = listEntry->celRect;
			}
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(lsLeft), listEntry->celRect.left);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(lsTop), listEntry->celRect.top);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(lsRight), listEntry->celRect.right);
			writeSelectorValue(_s->_segMan, curObject, SELECTOR(lsBottom), listEntry->celRect.bottom);
			// may get used for debugging
			//_paint16->frameRect(workerRect);
			_paint16->bitsShow(workerRect);

			if (signal & kSignalHidden) {
				listEntry->signal |= kSignalRemoveView;
			}
		}
	}
	// use this for debug purposes
	// _screen->copyToScreen();
}

void GfxAnimate::restoreAndDelete(int argc, reg_t *argv) {
	reg_t curObject;
	uint16 signal;
	AnimateList::iterator listEntry;
	const AnimateList::iterator end = _list.end();


	// This has to be done in a separate loop. At least in sq1 some .dispose
	// modifies FIXEDLOOP flag in signal for another object. In that case we
	// would overwrite the new signal with our version of the old signal.
	for (listEntry = _list.begin(); listEntry != end; ++listEntry) {
		curObject = listEntry->object;
		signal = listEntry->signal;

		// Finally update signal
		writeSelectorValue(_s->_segMan, curObject, SELECTOR(signal), signal);
	}

	for (listEntry = _list.reverse_begin(); listEntry != end; --listEntry) {
		curObject = listEntry->object;
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
	AnimateList::iterator listEntry;
	const AnimateList::iterator end = _list.end();

	for (listEntry = _list.begin(); listEntry != end; ++listEntry) {
		curObject = listEntry->object;

		if (listEntry->priority == -1)
			listEntry->priority = _ports->kernelCoordinateToPriority(listEntry->y);

		// Get the corresponding view
		view = _cache->getView(listEntry->viewId);

		// Create rect according to coordinates and given cel
		view->getCelRect(listEntry->loopNo, listEntry->celNo, listEntry->x, listEntry->y, listEntry->z, listEntry->celRect);

		// draw corresponding cel
		_paint16->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
		if ((listEntry->signal & kSignalIgnoreActor) == 0) {
			listEntry->celRect.top = CLIP<int16>(_ports->kernelPriorityToCoordinate(listEntry->priority) - 1, listEntry->celRect.top, listEntry->celRect.bottom - 1);
			_paint16->fillRect(listEntry->celRect, GFX_SCREEN_MASK_CONTROL, 0, 0, 15);
		}
	}
}

void GfxAnimate::addToPicDrawView(GuiResourceId viewId, int16 loopNo, int16 celNo, int16 leftPos, int16 topPos, int16 priority, int16 control) {
	GfxView *view = _cache->getView(viewId);
	Common::Rect celRect;

	// Create rect according to coordinates and given cel
	view->getCelRect(loopNo, celNo, leftPos, topPos, priority, celRect);
	_paint16->drawCel(view, loopNo, celNo, celRect, priority, 0);
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

	// We set SCI1.1 priority band information here
	_ports->priorityBandsRecall();
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
		// beginUpdate()/endUpdate() were introduced SCI1
		//  calling those for SCI0 will work most of the time but breaks minor stuff like percentage bar of qfg1ega
		//   at the character skill screen
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

	if (_lastCastData.size() > 1)
		_s->_throttleTrigger = true;

	_ports->setPort(oldPort);
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

void GfxAnimate::kernelAddToPicView(GuiResourceId viewId, int16 loopNo, int16 celNo, int16 leftPos, int16 topPos, int16 priority, int16 control) {
	_ports->setPort((Port *)_ports->_picWind);
	addToPicDrawView(viewId, loopNo, celNo, leftPos, topPos, priority, control);
	addToPicSetPicNotValid();
}

} // End of namespace Sci
