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
#include "sci/engine/state.h"
#include "sci/engine/vm.h"
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_view.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_transitions.h"
#include "sci/gui/gui_animate.h"

namespace Sci {

SciGuiAnimate::SciGuiAnimate(EngineState *state, SciGuiGfx *gfx, SciGuiScreen *screen, SciGuiPalette *palette)
	: _s(state), _gfx(gfx), _screen(screen), _palette(palette) {
	init();
}

SciGuiAnimate::~SciGuiAnimate() {
}

void SciGuiAnimate::init() {
	_listData = NULL;
	_listSize = 0;
}

void SciGuiAnimate::disposeLastCast() {
	// FIXME
	//if (!_lastCast->first.isNull())
		//_lastCast->DeleteList();
}

bool SciGuiAnimate::invoke(List *list, int argc, reg_t *argv) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 signal;

	// LSL1VGA does use global variable 84, and it does point to the fastCast object. But
	// it should not abort. Hooray for another game-specific workaround...
	reg_t fastCastObj = (_s->_gameName != "lsl1sci") ? _s->_segMan->findObjectByName("fastCast") : NULL_REG;

	while (curNode) {
		curObject = curNode->value;
		// Check if the game has a fastCast object set
		//  if we don't abort kAnimate processing, at least in kq5 there will be animation cels drawn into speech boxes.
		reg_t global84 = _s->script_000->_localsBlock->_locals[84];

		if (!fastCastObj.isNull() && global84 == fastCastObj)
			return false;

		signal = GET_SEL32V(segMan, curObject, signal);
		if (!(signal & kSignalFrozen)) {
			// Call .doit method of that object
			invoke_selector(_s, curObject, _s->_kernel->_selectorCache.doit, kContinueOnInvalidSelector, argv, argc, __FILE__, __LINE__, 0);
			// Lookup node again, since the nodetable it was in may have been reallocated
			curNode = _s->_segMan->lookupNode(curAddress);
		}
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
	return true;
}

bool sortHelper(const GuiAnimateEntry* entry1, const GuiAnimateEntry* entry2) {
	return (entry1->y == entry2->y) ? (entry1->z < entry2->z) : (entry1->y < entry2->y);
}

void SciGuiAnimate::makeSortedList(List *list) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	int16 listNr, listCount = 0;

	// Count the list entries
	while (curNode) {
		listCount++;
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}

	_list.clear();

	// No entries -> exit immediately
	if (listCount == 0)
		return;

	// Adjust list size, if needed
	if ((_listData == NULL) || (_listSize < listCount)) {
		if (_listData)
			free(_listData);
		_listData = (GuiAnimateEntry *)malloc(listCount * sizeof(GuiAnimateEntry));
		if (!_listData)
			error("Could not allocate memory for _animateList");
		_listSize = listCount;
	}

	// Fill the list
	curAddress = list->first;
	curNode = _s->_segMan->lookupNode(curAddress);
	listEntry = _listData;
	for (listNr = 0; listNr < listCount; listNr++) {
		curObject = curNode->value;
		listEntry->object = curObject;

		// Get data from current object
		listEntry->viewId = GET_SEL32V(segMan, curObject, view);
		listEntry->loopNo = GET_SEL32V(segMan, curObject, loop);
		listEntry->celNo = GET_SEL32V(segMan, curObject, cel);
		listEntry->paletteNo = GET_SEL32V(segMan, curObject, palette);
		listEntry->x = GET_SEL32V(segMan, curObject, x);
		listEntry->y = GET_SEL32V(segMan, curObject, y);
		listEntry->z = GET_SEL32V(segMan, curObject, z);
		listEntry->priority = GET_SEL32V(segMan, curObject, priority);
		listEntry->signal = GET_SEL32V(segMan, curObject, signal);
		// listEntry->celRect is filled in AnimateFill()
		listEntry->showBitsFlag = false;

		_list.push_back(listEntry);

		listEntry++;
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}

	// Now sort the list according y and z (descending)
	GuiAnimateList::iterator listBegin = _list.begin();
	GuiAnimateList::iterator listEnd = _list.end();

	Common::sort(_list.begin(), _list.end(), sortHelper);
}

void SciGuiAnimate::fill(byte &old_picNotValid) {
	SegManager *segMan = _s->_segMan;
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	uint16 signal;
	SciGuiView *view = NULL;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listEnd = _list.end();

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;

		// Get the corresponding view
		view = new SciGuiView(_s->resMan, _screen, _palette, listEntry->viewId);
		
		// adjust loop and cel, if any of those is invalid
		if (listEntry->loopNo >= view->getLoopCount()) {
			listEntry->loopNo = 0;
			PUT_SEL32V(segMan, curObject, loop, listEntry->loopNo);
		}
		if (listEntry->celNo >= view->getCelCount(listEntry->loopNo)) {
			listEntry->celNo = 0;
			PUT_SEL32V(segMan, curObject, cel, listEntry->celNo);
		}

		// Create rect according to coordinates and given cel
		view->getCelRect(listEntry->loopNo, listEntry->celNo, listEntry->x, listEntry->y, listEntry->z, &listEntry->celRect);
		PUT_SEL32V(segMan, curObject, nsLeft, listEntry->celRect.left);
		PUT_SEL32V(segMan, curObject, nsTop, listEntry->celRect.top);
		PUT_SEL32V(segMan, curObject, nsRight, listEntry->celRect.right);
		PUT_SEL32V(segMan, curObject, nsBottom, listEntry->celRect.bottom);

		signal = listEntry->signal;

		// Calculate current priority according to y-coordinate
		if (!(signal & kSignalFixedPriority)) {
			listEntry->priority = _gfx->CoordinateToPriority(listEntry->y);
			PUT_SEL32V(segMan, curObject, priority, listEntry->priority);
		}
		
		if (signal & kSignalNoUpdate) {
			if (signal & (kSignalForceUpdate | kSignalViewUpdated)
				|| (signal & kSignalHidden && !(signal & kSignalRemoveView))
				|| (!(signal & kSignalHidden) && signal & kSignalRemoveView)
				|| (signal & kSignalAlwaysUpdate))
				old_picNotValid++;
			signal &= 0xFFFF ^ kSignalStopUpdate;
		} else {
			if (signal & kSignalStopUpdate || signal & kSignalAlwaysUpdate)
				old_picNotValid++;
			signal &= 0xFFFF ^ kSignalForceUpdate;
		}
		listEntry->signal = signal;

		listIterator++;

		delete view;
	}
}

void SciGuiAnimate::update() {
	SegManager *segMan = _s->_segMan;
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	uint16 signal;
	reg_t bitsHandle;
	Common::Rect rect;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listBegin = _list.begin();
	GuiAnimateList::iterator listEnd = _list.end();

	// Remove all no-update cels, if requested
	listIterator = _list.reverse_begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalNoUpdate) {
			if (!(signal & kSignalRemoveView)) {
				bitsHandle = GET_SEL32(segMan, curObject, underBits);
				if (_screen->_picNotValid != 1) {
					_gfx->BitsRestore(bitsHandle);
					listEntry->showBitsFlag = true;
				} else	{
					_gfx->BitsFree(bitsHandle);
				}
				PUT_SEL32V(segMan, curObject, underBits, 0);
			}
			signal &= 0xFFFF ^ kSignalForceUpdate;
			signal &= signal & kSignalViewUpdated ? 0xFFFF ^ (kSignalViewUpdated | kSignalNoUpdate) : 0xFFFF;
		} else if (signal & kSignalStopUpdate) {
			signal =  (signal & (0xFFFF ^ kSignalStopUpdate)) | kSignalNoUpdate;
		}
		listEntry->signal = signal;
		listIterator--;
	}

	// Draw always-update cels
	listIterator = listBegin;
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalAlwaysUpdate) {
			// draw corresponding cel
			_gfx->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
			listEntry->showBitsFlag = true;

			signal &= 0xFFFF ^ (kSignalStopUpdate | kSignalViewUpdated | kSignalNoUpdate | kSignalForceUpdate);
			if ((signal & kSignalIgnoreActor) == 0) {
				rect = listEntry->celRect;
				rect.top = CLIP<int16>(_gfx->PriorityToCoordinate(listEntry->priority) - 1, rect.top, rect.bottom - 1);  
				_gfx->FillRect(rect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
			listEntry->signal = signal;
		}
		listIterator++;
	}

	// Saving background for all NoUpdate-cels
	listIterator = listBegin;
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalNoUpdate) {
			if (signal & kSignalHidden) {
				signal |= kSignalRemoveView;
			} else {
				signal &= 0xFFFF ^ kSignalRemoveView;
				if (signal & kSignalIgnoreActor)
					bitsHandle = _gfx->BitsSave(listEntry->celRect, SCI_SCREEN_MASK_VISUAL|SCI_SCREEN_MASK_PRIORITY);
				else
					bitsHandle = _gfx->BitsSave(listEntry->celRect, SCI_SCREEN_MASK_ALL);
				PUT_SEL32(segMan, curObject, underBits, bitsHandle);
			}
			listEntry->signal = signal;
		}
		listIterator++;
	}

	// Draw NoUpdate cels
	listIterator = listBegin;
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & kSignalNoUpdate && !(signal & kSignalHidden)) {
			// draw corresponding cel
			_gfx->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
			listEntry->showBitsFlag = true;

			if ((signal & kSignalIgnoreActor) == 0) {
				rect = listEntry->celRect;
				rect.top = CLIP<int16>(_gfx->PriorityToCoordinate(listEntry->priority) - 1, rect.top, rect.bottom - 1);  
				_gfx->FillRect(rect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
		}
		listIterator++;
	}
}

void SciGuiAnimate::drawCels() {
	SegManager *segMan = _s->_segMan;
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	uint16 signal;
	reg_t bitsHandle;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listEnd = _list.end();

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (!(signal & (kSignalNoUpdate | kSignalHidden | kSignalAlwaysUpdate))) {
			// Save background
			bitsHandle = _gfx->BitsSave(listEntry->celRect, SCI_SCREEN_MASK_ALL);
			PUT_SEL32(segMan, curObject, underBits, bitsHandle);

			// draw corresponding cel
			_gfx->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
			listEntry->showBitsFlag = true;

			if (signal & kSignalRemoveView) {
				signal &= 0xFFFF ^ kSignalRemoveView;
			}
			listEntry->signal = signal;
			
//			HEAPHANDLE hNewCast = heapNewPtr(sizeof(sciCast), kDataCast);
//			sciCast *pNewCast = (sciCast *)heap2Ptr(hNewCast);
//			pNewCast->view = view;
//			pNewCast->loop = loop;
//			pNewCast->cel = cel;
//			pNewCast->z = z;
//			pNewCast->pal = pal;
//			pNewCast->hSaved = 0;
//			pNewCast->rect = *rect;
//			_lastCast->AddToEnd(hNewCast);
		}
		listIterator++;
	}
}

void SciGuiAnimate::updateScreen(byte oldPicNotValid) {
	SegManager *segMan = _s->_segMan;
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	uint16 signal;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listEnd = _list.end();
	Common::Rect lsRect;
	Common::Rect workerRect;

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (listEntry->showBitsFlag || !(signal & (kSignalRemoveView | kSignalNoUpdate) ||
										(!(signal & kSignalRemoveView) && (signal & kSignalNoUpdate) && oldPicNotValid))) {
			lsRect.left = GET_SEL32V(segMan, curObject, lsLeft);
			lsRect.top = GET_SEL32V(segMan, curObject, lsTop);
			lsRect.right = GET_SEL32V(segMan, curObject, lsRight);
			lsRect.bottom = GET_SEL32V(segMan, curObject, lsBottom);

			workerRect = lsRect;
			workerRect.clip(listEntry->celRect);

			if (!workerRect.isEmpty()) {
				workerRect = lsRect;
				workerRect.extend(listEntry->celRect);
			} else {
				_gfx->BitsShow(lsRect);
				workerRect = listEntry->celRect;
			}
			PUT_SEL32V(segMan, curObject, lsLeft, workerRect.left);
			PUT_SEL32V(segMan, curObject, lsTop, workerRect.top);
			PUT_SEL32V(segMan, curObject, lsRight, workerRect.right);
			PUT_SEL32V(segMan, curObject, lsBottom, workerRect.bottom);
			_gfx->BitsShow(workerRect);

			if (signal & kSignalHidden) {
				listEntry->signal |= kSignalRemoveView;
			}
		}

		listIterator++;
	}
	// use this for debug purposes
	// _screen->copyToScreen();
}

void SciGuiAnimate::restoreAndDelete(int argc, reg_t *argv) {
	SegManager *segMan = _s->_segMan;
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	uint16 signal;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listEnd = _list.end();


	// This has to be done in a separate loop. At least in sq1 some .dispose modifies FIXEDLOOP flag in signal for
	//  another object. In that case we would overwrite the new signal with our version of the old signal
	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		// Finally update signal
		PUT_SEL32V(segMan, curObject, signal, signal);
		listIterator++;
	}

	listIterator = _list.reverse_begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		// We read out signal here again, this is not by accident but to ensure that we got an up-to-date signal
		signal = GET_SEL32V(segMan, curObject, signal);

		if ((signal & (kSignalNoUpdate | kSignalRemoveView)) == 0) {
			_gfx->BitsRestore(GET_SEL32(segMan, curObject, underBits));
			PUT_SEL32V(segMan, curObject, underBits, 0);
		}

		if (signal & kSignalDisposeMe) {
			// Call .delete_ method of that object
			invoke_selector(_s, curObject, _s->_kernel->_selectorCache.delete_, kContinueOnInvalidSelector, argv, argc, __FILE__, __LINE__, 0);
		}
		listIterator--;
	}
}

void SciGuiAnimate::reAnimate(Common::Rect rect) {
	// TODO: implement ReAnimate
	_gfx->BitsShow(rect);
}

void SciGuiAnimate::addToPicDrawCels(List *list) {
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	SciGuiView *view = NULL;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listEnd = _list.end();

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;

		if (listEntry->priority == -1)
			listEntry->priority = _gfx->CoordinateToPriority(listEntry->y);

		// Get the corresponding view
		view = new SciGuiView(_s->resMan, _screen, _palette, listEntry->viewId);

		// Create rect according to coordinates and given cel
		view->getCelRect(listEntry->loopNo, listEntry->celNo, listEntry->x, listEntry->y, listEntry->z, &listEntry->celRect);

		// draw corresponding cel
		_gfx->drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
		if ((listEntry->signal & kSignalIgnoreActor) == 0) {
			listEntry->celRect.top = CLIP<int16>(_gfx->PriorityToCoordinate(listEntry->priority) - 1, listEntry->celRect.top, listEntry->celRect.bottom - 1);
			_gfx->FillRect(listEntry->celRect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
		}

		listIterator++;

		delete view;
	}
}

void SciGuiAnimate::addToPicDrawView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control) {
	SciGuiView *view = NULL;
	Common::Rect celRect;

	view = new SciGuiView(_s->resMan, _screen, _palette, viewId);

	// Create rect according to coordinates and given cel
	view->getCelRect(loopNo, celNo, leftPos, topPos, priority, &celRect);
	_gfx->drawCel(view, loopNo, celNo, celRect, priority, 0);

	delete view;
}

} // End of namespace Sci
