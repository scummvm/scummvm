
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

#ifndef BAGEL_BOFLIB_GUI_LIST_BOX_H
#define BAGEL_BOFLIB_GUI_LIST_BOX_H

#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/list.h"
#include "bagel/boflib/string.h"

namespace Bagel {

#define LISTBOX_NORMAL 0
#define LISTBOX_SELECT 1
#define LISTBOX_USENOW 2

class ListBoxItem {
public:
	CBofString *m_pTextStr;
	RGBCOLOR m_nTextLineColor;
};

class CBofListBox : public CBofWindow {
public:
	CBofListBox();
	~CBofListBox();

	void SetSelectedItem(int nItem, bool bRepaint = true);

	void InsertBefore(int nIndex, const CBofString &cString, bool bRepaint = true);
	void InsertAfter(int nIndex, const CBofString &cString, bool bRepaint = true);

	void AddToHead(const CBofString &cString, bool bRepaint = true);
	void AddToTail(const CBofString &cString, bool bRepaint = true);

	ErrorCode DelItem(int nIndex, bool bRepaint = true);
	ErrorCode DeleteAll(bool bRepaint = true);

	int GetNumItems() {
		return (m_nNumItems);
	}
	CBofString GetText(int nIndex);
	void SetText(int nIndex, const CBofString &cStr);

	void SetTextLineColor(int nIndex, RGBCOLOR rgbColor);

	ErrorCode LineUp() {
		return (ScrollUp(1));
	}
	ErrorCode LineDown() {
		return (ScrollDown(1));
	}

	ErrorCode PageUp() {
		return (ScrollUp(m_nPageSize));
	}
	ErrorCode PageDown() {
		return (ScrollDown(m_nPageSize));
	}

	ErrorCode ScrollUp(const int nLines);
	ErrorCode ScrollDown(const int nLines) {
		return (ScrollUp(-nLines));
	}

	ErrorCode ScrollTo(const int nLine);

	ErrorCode CreateWorkArea();
	ErrorCode SaveBackground();
	void KillBackground();

	void SetHighlightColor(RGBCOLOR cHighColor) {
		m_cHighColor = cHighColor;
	}
	RGBCOLOR GetHighlightColor() {
		return (m_cHighColor);
	}

	void SetTextColor(RGBCOLOR cColor) {
		m_cTextColor = cColor;
	}
	RGBCOLOR GetTextColor() {
		return (m_cTextColor);
	}

	void SetPointSize(int nSize) {
		m_nTextSize = nSize;
	}
	int GetPointSize() {
		return (m_nTextSize);
	}

	void SetWeight(int nWeight) {
		m_nTextWeight = nWeight;
	}
	int GetWeight() {
		return (m_nTextWeight);
	}

	void SetItemHeight(int nHeight) {
		m_nItemHeight = nHeight;
	}
	int GetItemHeight() {
		return (m_nItemHeight);
	}

	void SetFont(int nFont) {
		m_nTextFont = nFont;
	}
	int GetFont() {
		return (m_nTextFont);
	}

	int GetState() {
		return (m_nState);
	}

	virtual ErrorCode RepaintItem(int nIndex);
	virtual ErrorCode RepaintAll();

protected:
	virtual void OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnLButtonDblClk(uint32 nFlags, CBofPoint *pPoint);
	virtual void OnKeyHit(uint32 lKey, uint32 lRepCount);
	virtual void OnPaint(CBofRect *pRect);

	/**
	 * Clears the currently selected item
	 */
	virtual void ClearSelection();

	CBofList<ListBoxItem> m_cTextItems;
	CBofBitmap *m_pWork;
	int m_nNumItems;
	int m_n1stVisible;

	int m_nPageSize;

	int m_nTextSize;
	int m_nTextWeight;
	RGBCOLOR m_cTextColor;
	RGBCOLOR m_cHighColor;
	int m_nSelectedItem;
	int m_nItemHeight;
	int m_nTextFont;
	int m_nState;
};

} // namespace Bagel

#endif
