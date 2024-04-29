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

#include "bagel/boflib/app.h"
#include "bagel/boflib/gfx/text.h"
#include "bagel/boflib/gui/list_box.h"

#include "bagel/boflib/std_keys.h"

namespace Bagel {

#define TEXT_ITEM_HEIGHT 24

#define COLOR_USE_DEFAULT 0xFFFFFFFF


CBofListBox::CBofListBox() {
	_cTextColor = CTEXT_COLOR;
	_cHighColor = CTEXT_COLOR;
	_nTextSize = 10;
	_nTextWeight = TEXT_NORMAL;
	_nNumItems = 0;
	_n1stVisible = 0;
	_nPageSize = 0;
	_pWork = nullptr;
	_nItemHeight = TEXT_ITEM_HEIGHT;
	_nState = LISTBOX_NORMAL;

	// Initialized the selected item
	clearSelection();
}


CBofListBox::~CBofListBox() {
	// Kill the temporary work area
	if (_pWork != nullptr) {
		delete _pWork;
		_pWork  = nullptr;
	}

	deleteAll(false);
}

void CBofListBox::clearSelection() {
	_nSelectedItem = -1;
}

void CBofListBox::insertBefore(int nIndex, const CBofString &cString, bool bRepaint) {
	Assert(IsValidObject(this));

	ListBoxItem lbi;
	lbi._pTextStr          = new CBofString(cString);
	lbi._nTextLineColor    = COLOR_USE_DEFAULT;

	_cTextItems.insertBefore(nIndex, lbi);

	// One more item
	_nNumItems++;

	clearSelection();

	if (bRepaint && isCreated() && isVisible()) {
		repaintAll();
	}
}


void CBofListBox::insertAfter(int nIndex, const CBofString &cString, bool bRepaint) {
	Assert(IsValidObject(this));

	ListBoxItem lbi;
	lbi._pTextStr          = new CBofString(cString);
	lbi._nTextLineColor    = COLOR_USE_DEFAULT;

	_cTextItems.insertAfter(nIndex, lbi);

	// One more item
	_nNumItems++;

	if (bRepaint && isCreated() && isVisible()) {
		repaintAll();
	}
}


void CBofListBox::addToHead(const CBofString &cString, bool bRepaint) {
	Assert(IsValidObject(this));

	ListBoxItem lbi;
	lbi._pTextStr          = new CBofString(cString);
	lbi._nTextLineColor    = COLOR_USE_DEFAULT;

	_cTextItems.addToHead(lbi);

	// One more item
	_nNumItems++;

	clearSelection();

	if (bRepaint && isCreated() && isVisible()) {
		repaintAll();
	}
}


void CBofListBox::addToTail(const CBofString &cString, bool bRepaint) {
	Assert(IsValidObject(this));

	ListBoxItem lbi;
	lbi._pTextStr          = new CBofString(cString);
	lbi._nTextLineColor    = COLOR_USE_DEFAULT;

	_cTextItems.addToTail(lbi);

	// One more item
	_nNumItems++;

	clearSelection();

	if (bRepaint && isCreated() && isVisible()) {
		repaintAll();
	}
}


ErrorCode CBofListBox::delItem(int nIndex, bool bRepaint) {
	Assert(IsValidObject(this));
	Assert(nIndex >= 0 && nIndex < _nNumItems);

	_cTextItems.Remove(nIndex);

	// One less item
	_nNumItems--;

	if (_n1stVisible >= _nNumItems) {
		_n1stVisible = _nNumItems - 1;
		if (_n1stVisible < 0)
			_n1stVisible = 0;
	}

	clearSelection();


	if (bRepaint && isCreated() && isVisible()) {
		repaintAll();
	}

	return _errCode;
}

ErrorCode CBofListBox::deleteAll(bool bRepaint) {
	Assert(IsValidObject(this));

	// Switch item to be pointer to cbofstring instead of the item itself
	int nCount = _cTextItems.GetCount();

	for (int i = 0; i < nCount; i++) {
		ListBoxItem lbi = _cTextItems.GetNodeItem(i);
		delete lbi._pTextStr;
	}

	_cTextItems.RemoveAll();
	_nNumItems = 0;
	_n1stVisible = 0;

	clearSelection();

	if (bRepaint && isCreated() && isVisible()) {
		repaintAll();
	}

	return _errCode;
}


void CBofListBox::onLButtonDown(uint32 /*nFlags*/, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	int nIndex = (pPoint->y / _nItemHeight) + _n1stVisible;
	if (nIndex < _nNumItems) {

		_nSelectedItem = nIndex; // Set the selected item
		_nState = LISTBOX_SELECT;

		if (_parent != nullptr) {
			_parent->setPrevMouseDown(*pPoint);
			_parent->onBofListBox(this, nIndex);
		}
	}
}


void CBofListBox::onLButtonDblClk(uint32 /*nFlags*/, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	int nIndex = (pPoint->y / _nItemHeight) + _n1stVisible;

	if (nIndex < _nNumItems) {
		_nSelectedItem = nIndex; // Set the selected item
		_nState = LISTBOX_USENOW;

		if (_parent != nullptr) {
			_parent->setPrevMouseDown(*pPoint);
			_parent->onBofListBox(this, nIndex);
		}
	}

	setFocus();
}


void CBofListBox::onKeyHit(uint32 lKey, uint32 lRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {
	case BKEY_HOME:
		scrollTo(0);
		break;

	case BKEY_END:
		scrollTo(_nNumItems);
		break;

	case BKEY_UP:
		lineUp();
		break;

	case BKEY_DOWN:
		lineDown();
		break;

	case BKEY_PAGEUP:
		pageUp();
		break;

	case BKEY_PAGEDOWN:
		pageDown();
		break;

	default:
		// Call the previous windows onkeyhit
		CBofWindow *pParent = getParent();
		if (pParent && pParent != this) {
			pParent->onKeyHit(lKey, lRepCount);
		}
		break;
	}
}


ErrorCode CBofListBox::scrollUp(const int nLines) {
	Assert(IsValidObject(this));

	// If all the items fit on a single page, make this operation a no-op.
	if (_nNumItems <= _nPageSize) {
		return scrollTo(_n1stVisible);
	}

	int nNewLine = _n1stVisible - nLines;

	if (nNewLine < 0) {
		nNewLine = 0;

	} else if (nNewLine > (_nNumItems - _nPageSize)) {
		// If the line requested to be the top of the page
		// would cause fewer than _nPageSize lines to be displayed,
		// snap nNewLine to be equal to the top of the last full page.
		nNewLine = (_nNumItems - _nPageSize);
	}

	return scrollTo(nNewLine);
}

ErrorCode CBofListBox::scrollTo(const int nLine) {
	Assert(IsValidObject(this));
	Assert(nLine >= 0 && nLine <= _nNumItems);

	// Only update the screen if the list actually moved
	if (_n1stVisible != nLine) {
		_n1stVisible = nLine;

		if (nLine >= _nNumItems) {
			Assert(_nNumItems > 0);
			_n1stVisible--;
		}

		// Show the text box
		repaintAll();
	}

	return _errCode;
}


void CBofListBox::onPaint(CBofRect * /*pRect*/) {
	Assert(IsValidObject(this));

	_nPageSize = height() / _nItemHeight;

	if (_pBackdrop == nullptr) {
		saveBackground();
	}

	repaintAll();
}


void CBofListBox::killBackground() {
	Assert(IsValidObject(this));

	if (_pBackdrop != nullptr) {
		delete _pBackdrop;
		_pBackdrop = nullptr;
	}
}


ErrorCode CBofListBox::saveBackground() {
	Assert(IsValidObject(this));

	killBackground();

	if ((_pBackdrop = new CBofBitmap(width(), height(), CBofApp::GetApp()->getPalette())) != nullptr) {
		if ((_parent != nullptr) && (_parent->getBackdrop() != nullptr)) {
			CBofRect cRect = _pBackdrop->getRect();

			_parent->getBackdrop()->paint(_pBackdrop, &cRect, &_cWindowRect);

		} else {
			_pBackdrop->captureScreen(this, &_cRect);
		}

	} else {
		ReportError(ERR_MEMORY, "Unable to allocate a %d x %d CBofBitmap", width(), height());
	}

	return _errCode;
}

ErrorCode CBofListBox::createWorkArea() {
	Assert(IsValidObject(this));

	if (_pBackdrop == nullptr) {
		saveBackground();
	}

	if (_pWork == nullptr) {
		Assert(_pBackdrop != nullptr);

		if ((_pWork = new CBofBitmap(width(), height(), _pBackdrop->getPalette())) == nullptr) {
			ReportError(ERR_MEMORY, "Unable to allocate a %d x %d CBofBitmap", width(), height());
		}
	}

	return _errCode;
}


ErrorCode CBofListBox::repaintAll() {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		Assert(isCreated());

		int nCurFont = getFont();
		setFont(_nTextFont);

		createWorkArea();

		if (_pWork != nullptr) {
			_pWork->lock();

			int nIndexedColor = _pWork->getPalette()->getNearestIndex(_cTextColor);

			// prepare the background
			//
			Assert(_pBackdrop != nullptr);
			_pBackdrop->paint(_pWork);

			for (int i = 0; i < _nPageSize; i++) {
				CBofRect cRect;
				cRect.SetRect(0, i * _nItemHeight, width() - 1, (i + 1) * _nItemHeight - 1);

				if (i + _n1stVisible < _nNumItems) {
					// If this item is currently selected and we have a high color
					if ((i + _n1stVisible == _nSelectedItem) && (_cHighColor != _cTextColor)) {
						// display text highlighted
						PaintText(_pWork,
						          &cRect,
						          *(_cTextItems.GetNodeItem(i + _n1stVisible)._pTextStr),
						          _nTextSize,
						          _nTextWeight,
						          _cHighColor,
						          JUSTIFY_LEFT,
						          FORMAT_TOP_LEFT | FORMAT_SINGLE_LINE,
						          getFont());
					} else {
						// Display text
						// Allow list items of different colors.
						RGBCOLOR rgbTextColor = _cTextColor;
						if (_cTextItems.GetNodeItem(i + _n1stVisible)._nTextLineColor != COLOR_USE_DEFAULT) {
							rgbTextColor = _cTextItems.GetNodeItem(i + _n1stVisible)._nTextLineColor;
						}

						PaintText(_pWork,
						          &cRect,
						          *(_cTextItems.GetNodeItem(i + _n1stVisible)._pTextStr),
						          _nTextSize,
						          _nTextWeight,
						          rgbTextColor,
						          JUSTIFY_LEFT,
						          FORMAT_TOP_LEFT | FORMAT_SINGLE_LINE,
						          getFont());
					}

					CBofPoint bl(cRect.BottomLeft()), br(cRect.BottomRight());
					_pWork->line(&bl, &br, (byte)nIndexedColor);
				}
			}

			// Show final image on screen
			_pWork->paint(this);

			_pWork->unlock();
		}

		// Reset the font
		setFont(nCurFont);
	}

	return _errCode;
}


ErrorCode CBofListBox::repaintItem(int nIndex) {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		Assert(nIndex >= 0 && nIndex < _nNumItems);
		int nCurFont = getFont();
		setFont(_nTextFont);           // Set the proper font

		// If this item is visible, then repaint it.
		if (nIndex >= _n1stVisible && nIndex <= _n1stVisible + _nPageSize) {
			int i = nIndex - _n1stVisible;

			createWorkArea();
			int nIndexedColor = _pWork->getPalette()->getNearestIndex(_cTextColor);

			_pWork->lock();

			// Calculate area for this text item
			CBofRect cRect;
			cRect.SetRect(0, i * _nItemHeight, width() - 1, (i + 1) * _nItemHeight - 1);

			// Prepare the background
			Assert(_pBackdrop != nullptr);
			_pBackdrop->paint(_pWork, &cRect, &cRect);

			// If this item is currently selected and we have a high color
			if ((nIndex == _nSelectedItem) && (_cHighColor != _cTextColor)) {
				// Display text highlighted
				PaintText(_pWork,
				          &cRect,
				          *(_cTextItems.GetNodeItem(nIndex)._pTextStr),
				          _nTextSize,
				          _nTextWeight,
				          _cHighColor,
				          JUSTIFY_LEFT,
				          FORMAT_TOP_LEFT | FORMAT_SINGLE_LINE,
				          getFont());

			} else {
				// Display text
				// Allow list items of different colors.
				RGBCOLOR rgbTextColor = _cTextColor;
				if (_cTextItems.GetNodeItem(i + _n1stVisible)._nTextLineColor != COLOR_USE_DEFAULT) {
					rgbTextColor = _cTextItems.GetNodeItem(i + _n1stVisible)._nTextLineColor;
				}
				PaintText(_pWork,
				          &cRect,
				          *(_cTextItems.GetNodeItem(nIndex)._pTextStr),
				          _nTextSize,
				          _nTextWeight,
				          rgbTextColor,
				          JUSTIFY_LEFT,
				          FORMAT_TOP_LEFT | FORMAT_SINGLE_LINE,
				          getFont());
			}
			CBofPoint bl(cRect.BottomLeft()), br(cRect.BottomRight());
			_pWork->line(&bl, &br, (byte)nIndexedColor);

			// Show final image on screen
			_pWork->paint(this, &cRect, &cRect);

			_pWork->unlock();
		}

		// Reset the font
		setFont(nCurFont);
	}

	return _errCode;
}


void CBofListBox::setSelectedItem(int nItem, bool bRepaint) {
	Assert(IsValidObject(this));

	// Set highlighted item
	_nSelectedItem = nItem;

	if (bRepaint) {
		repaintAll();
	}
}


CBofString CBofListBox::getText(int nIndex) {
	return *(_cTextItems.GetNodeItem(nIndex)._pTextStr);
}


void CBofListBox::setText(int nIndex, const CBofString &cStr) {

	ListBoxItem lbi = _cTextItems.GetNodeItem(nIndex);
	*lbi._pTextStr = cStr;

	_cTextItems.SetNodeItem(nIndex, lbi);
}


void CBofListBox::setTextLineColor(int nIndex, RGBCOLOR rgbColor) {

	ListBoxItem lbi = _cTextItems.GetNodeItem(nIndex);
	lbi._nTextLineColor = rgbColor;

	_cTextItems.SetNodeItem(nIndex, lbi);
}

} // namespace Bagel
