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
	m_cTextColor = CTEXT_COLOR;
	m_cHighColor = CTEXT_COLOR;
	m_nTextSize = 10;
	m_nTextWeight = TEXT_NORMAL;
	m_nNumItems = 0;
	m_n1stVisible = 0;
	m_nPageSize = 0;
	m_pWork = nullptr;
	m_nItemHeight = TEXT_ITEM_HEIGHT;
	m_nState = LISTBOX_NORMAL;

	// Initialized the selected item
	clearSelection();
}


CBofListBox::~CBofListBox() {
	// Kill the temporary work area
	if (m_pWork != nullptr) {
		delete m_pWork;
		m_pWork  = nullptr;
	}

	deleteAll(false);
}

void CBofListBox::clearSelection() {
	m_nSelectedItem = -1;
}

void CBofListBox::insertBefore(int nIndex, const CBofString &cString, bool bRepaint) {
	Assert(IsValidObject(this));

	ListBoxItem lbi;
	lbi.m_pTextStr          = new CBofString(cString);
	lbi.m_nTextLineColor    = COLOR_USE_DEFAULT;

	m_cTextItems.insertBefore(nIndex, lbi);

	// One more item
	m_nNumItems++;

	clearSelection();

	if (bRepaint && IsCreated() && IsVisible()) {
		repaintAll();
	}
}


void CBofListBox::insertAfter(int nIndex, const CBofString &cString, bool bRepaint) {
	Assert(IsValidObject(this));

	ListBoxItem lbi;
	lbi.m_pTextStr          = new CBofString(cString);
	lbi.m_nTextLineColor    = COLOR_USE_DEFAULT;

	m_cTextItems.insertAfter(nIndex, lbi);

	// One more item
	m_nNumItems++;

	if (bRepaint && IsCreated() && IsVisible()) {
		repaintAll();
	}
}


void CBofListBox::addToHead(const CBofString &cString, bool bRepaint) {
	Assert(IsValidObject(this));

	ListBoxItem lbi;
	lbi.m_pTextStr          = new CBofString(cString);
	lbi.m_nTextLineColor    = COLOR_USE_DEFAULT;

	m_cTextItems.addToHead(lbi);

	// One more item
	m_nNumItems++;

	clearSelection();

	if (bRepaint && IsCreated() && IsVisible()) {
		repaintAll();
	}
}


void CBofListBox::addToTail(const CBofString &cString, bool bRepaint) {
	Assert(IsValidObject(this));

	ListBoxItem lbi;
	lbi.m_pTextStr          = new CBofString(cString);
	lbi.m_nTextLineColor    = COLOR_USE_DEFAULT;

	m_cTextItems.addToTail(lbi);

	// One more item
	m_nNumItems++;

	clearSelection();

	if (bRepaint && IsCreated() && IsVisible()) {
		repaintAll();
	}
}


ErrorCode CBofListBox::delItem(int nIndex, bool bRepaint) {
	Assert(IsValidObject(this));
	Assert(nIndex >= 0 && nIndex < m_nNumItems);

	m_cTextItems.Remove(nIndex);

	// One less item
	m_nNumItems--;

	if (m_n1stVisible >= m_nNumItems) {
		m_n1stVisible = m_nNumItems - 1;
		if (m_n1stVisible < 0)
			m_n1stVisible = 0;
	}

	clearSelection();


	if (bRepaint && IsCreated() && IsVisible()) {
		repaintAll();
	}

	return _errCode;
}

ErrorCode CBofListBox::deleteAll(bool bRepaint) {
	Assert(IsValidObject(this));

	// Switch item to be pointer to cbofstring instead of the item itself
	int nCount = m_cTextItems.GetCount();

	for (int i = 0; i < nCount; i++) {
		ListBoxItem lbi = m_cTextItems.GetNodeItem(i);
		delete lbi.m_pTextStr;
	}

	m_cTextItems.RemoveAll();
	m_nNumItems = 0;
	m_n1stVisible = 0;

	clearSelection();

	if (bRepaint && IsCreated() && IsVisible()) {
		repaintAll();
	}

	return _errCode;
}


void CBofListBox::onLButtonDown(uint32 /*nFlags*/, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	int nIndex = (pPoint->y / m_nItemHeight) + m_n1stVisible;
	if (nIndex < m_nNumItems) {

		m_nSelectedItem = nIndex; // Set the selected item
		m_nState = LISTBOX_SELECT;

		if (_parent != nullptr) {
			_parent->SetPrevMouseDown(*pPoint);
			_parent->OnBofListBox(this, nIndex);
		}
	}
}


void CBofListBox::onLButtonDblClk(uint32 /*nFlags*/, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	int nIndex = (pPoint->y / m_nItemHeight) + m_n1stVisible;

	if (nIndex < m_nNumItems) {
		m_nSelectedItem = nIndex; // Set the selected item
		m_nState = LISTBOX_USENOW;

		if (_parent != nullptr) {
			_parent->SetPrevMouseDown(*pPoint);
			_parent->OnBofListBox(this, nIndex);
		}
	}

	SetFocus();
}


void CBofListBox::onKeyHit(uint32 lKey, uint32 lRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {
	case BKEY_HOME:
		scrollTo(0);
		break;

	case BKEY_END:
		scrollTo(m_nNumItems);
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
		CBofWindow *pParent = GetParent();
		if (pParent && pParent != this) {
			pParent->onKeyHit(lKey, lRepCount);
		}
		break;
	}
}


ErrorCode CBofListBox::scrollUp(const int nLines) {
	Assert(IsValidObject(this));

	// If all the items fit on a single page, make this operation a no-op.
	if (m_nNumItems <= m_nPageSize) {
		return scrollTo(m_n1stVisible);
	}

	int nNewLine = m_n1stVisible - nLines;

	if (nNewLine < 0) {
		nNewLine = 0;

	} else if (nNewLine > (m_nNumItems - m_nPageSize)) {
		// If the line requested to be the top of the page
		// would cause fewer than m_nPageSize lines to be displayed,
		// snap nNewLine to be equal to the top of the last full page.
		nNewLine = (m_nNumItems - m_nPageSize);
	}

	return scrollTo(nNewLine);
}

ErrorCode CBofListBox::scrollTo(const int nLine) {
	Assert(IsValidObject(this));
	Assert(nLine >= 0 && nLine <= m_nNumItems);

	// Only update the screen if the list actually moved
	if (m_n1stVisible != nLine) {
		m_n1stVisible = nLine;

		if (nLine >= m_nNumItems) {
			Assert(m_nNumItems > 0);
			m_n1stVisible--;
		}

		// Show the text box
		repaintAll();
	}

	return _errCode;
}


void CBofListBox::onPaint(CBofRect * /*pRect*/) {
	Assert(IsValidObject(this));

	m_nPageSize = Height() / m_nItemHeight;

	if (m_pBackdrop == nullptr) {
		saveBackground();
	}

	repaintAll();
}


void CBofListBox::killBackground() {
	Assert(IsValidObject(this));

	if (m_pBackdrop != nullptr) {
		delete m_pBackdrop;
		m_pBackdrop = nullptr;
	}
}


ErrorCode CBofListBox::saveBackground() {
	Assert(IsValidObject(this));

	killBackground();

	if ((m_pBackdrop = new CBofBitmap(Width(), Height(), CBofApp::GetApp()->GetPalette())) != nullptr) {
		if ((_parent != nullptr) && (_parent->GetBackdrop() != nullptr)) {
			CBofRect cRect = m_pBackdrop->GetRect();

			_parent->GetBackdrop()->paint(m_pBackdrop, &cRect, &m_cWindowRect);

		} else {
			m_pBackdrop->CaptureScreen(this, &_cRect);
		}

	} else {
		ReportError(ERR_MEMORY, "Unable to allocate a %d x %d CBofBitmap", Width(), Height());
	}

	return _errCode;
}

ErrorCode CBofListBox::createWorkArea() {
	Assert(IsValidObject(this));

	if (m_pBackdrop == nullptr) {
		saveBackground();
	}

	if (m_pWork == nullptr) {
		Assert(m_pBackdrop != nullptr);

		if ((m_pWork = new CBofBitmap(Width(), Height(), m_pBackdrop->GetPalette())) == nullptr) {
			ReportError(ERR_MEMORY, "Unable to allocate a %d x %d CBofBitmap", Width(), Height());
		}
	}

	return _errCode;
}


ErrorCode CBofListBox::repaintAll() {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		Assert(IsCreated());

		int nCurFont = getFont();
		setFont(m_nTextFont);

		createWorkArea();

		if (m_pWork != nullptr) {
			m_pWork->Lock();

			int nIndexedColor = m_pWork->GetPalette()->GetNearestIndex(m_cTextColor);

			// prepare the background
			//
			Assert(m_pBackdrop != nullptr);
			m_pBackdrop->paint(m_pWork);

			for (int i = 0; i < m_nPageSize; i++) {
				CBofRect cRect;
				cRect.SetRect(0, i * m_nItemHeight, Width() - 1, (i + 1) * m_nItemHeight - 1);

				if (i + m_n1stVisible < m_nNumItems) {
					// If this item is currently selected and we have a high color
					if ((i + m_n1stVisible == m_nSelectedItem) && (m_cHighColor != m_cTextColor)) {
						// display text highlighted
						PaintText(m_pWork,
						          &cRect,
						          *(m_cTextItems.GetNodeItem(i + m_n1stVisible).m_pTextStr),
						          m_nTextSize,
						          m_nTextWeight,
						          m_cHighColor,
						          JUSTIFY_LEFT,
						          FORMAT_TOP_LEFT | FORMAT_SINGLE_LINE,
						          getFont());
					} else {
						// Display text
						// Allow list items of different colors.
						RGBCOLOR rgbTextColor = m_cTextColor;
						if (m_cTextItems.GetNodeItem(i + m_n1stVisible).m_nTextLineColor != COLOR_USE_DEFAULT) {
							rgbTextColor = m_cTextItems.GetNodeItem(i + m_n1stVisible).m_nTextLineColor;
						}

						PaintText(m_pWork,
						          &cRect,
						          *(m_cTextItems.GetNodeItem(i + m_n1stVisible).m_pTextStr),
						          m_nTextSize,
						          m_nTextWeight,
						          rgbTextColor,
						          JUSTIFY_LEFT,
						          FORMAT_TOP_LEFT | FORMAT_SINGLE_LINE,
						          getFont());
					}

					CBofPoint bl(cRect.BottomLeft()), br(cRect.BottomRight());
					m_pWork->Line(&bl, &br, (byte)nIndexedColor);
				}
			}

			// Show final image on screen
			m_pWork->paint(this);

			m_pWork->UnLock();
		}

		// Reset the font
		setFont(nCurFont);
	}

	return _errCode;
}


ErrorCode CBofListBox::repaintItem(int nIndex) {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		Assert(nIndex >= 0 && nIndex < m_nNumItems);
		int nCurFont = getFont();
		setFont(m_nTextFont);           // Set the proper font

		// If this item is visible, then repaint it.
		if (nIndex >= m_n1stVisible && nIndex <= m_n1stVisible + m_nPageSize) {
			int i = nIndex - m_n1stVisible;

			createWorkArea();
			int nIndexedColor = m_pWork->GetPalette()->GetNearestIndex(m_cTextColor);

			m_pWork->Lock();

			// Calculate area for this text item
			CBofRect cRect;
			cRect.SetRect(0, i * m_nItemHeight, Width() - 1, (i + 1) * m_nItemHeight - 1);

			// Prepare the background
			Assert(m_pBackdrop != nullptr);
			m_pBackdrop->paint(m_pWork, &cRect, &cRect);

			// If this item is currently selected and we have a high color
			if ((nIndex == m_nSelectedItem) && (m_cHighColor != m_cTextColor)) {
				// Display text highlighted
				PaintText(m_pWork,
				          &cRect,
				          *(m_cTextItems.GetNodeItem(nIndex).m_pTextStr),
				          m_nTextSize,
				          m_nTextWeight,
				          m_cHighColor,
				          JUSTIFY_LEFT,
				          FORMAT_TOP_LEFT | FORMAT_SINGLE_LINE,
				          getFont());

			} else {
				// Display text
				// Allow list items of different colors.
				RGBCOLOR rgbTextColor = m_cTextColor;
				if (m_cTextItems.GetNodeItem(i + m_n1stVisible).m_nTextLineColor != COLOR_USE_DEFAULT) {
					rgbTextColor = m_cTextItems.GetNodeItem(i + m_n1stVisible).m_nTextLineColor;
				}
				PaintText(m_pWork,
				          &cRect,
				          *(m_cTextItems.GetNodeItem(nIndex).m_pTextStr),
				          m_nTextSize,
				          m_nTextWeight,
				          rgbTextColor,
				          JUSTIFY_LEFT,
				          FORMAT_TOP_LEFT | FORMAT_SINGLE_LINE,
				          getFont());
			}
			CBofPoint bl(cRect.BottomLeft()), br(cRect.BottomRight());
			m_pWork->Line(&bl, &br, (byte)nIndexedColor);

			// Show final image on screen
			m_pWork->paint(this, &cRect, &cRect);

			m_pWork->UnLock();
		}

		// Reset the font
		setFont(nCurFont);
	}

	return _errCode;
}


void CBofListBox::setSelectedItem(int nItem, bool bRepaint) {
	Assert(IsValidObject(this));

	// Set highlighted item
	m_nSelectedItem = nItem;

	if (bRepaint) {
		repaintAll();
	}
}


CBofString CBofListBox::getText(int nIndex) {
	return *(m_cTextItems.GetNodeItem(nIndex).m_pTextStr);
}


void CBofListBox::setText(int nIndex, const CBofString &cStr) {

	ListBoxItem lbi = m_cTextItems.GetNodeItem(nIndex);
	*lbi.m_pTextStr = cStr;

	m_cTextItems.SetNodeItem(nIndex, lbi);
}


void CBofListBox::setTextLineColor(int nIndex, RGBCOLOR rgbColor) {

	ListBoxItem lbi = m_cTextItems.GetNodeItem(nIndex);
	lbi.m_nTextLineColor = rgbColor;

	m_cTextItems.SetNodeItem(nIndex, lbi);
}

} // namespace Bagel
