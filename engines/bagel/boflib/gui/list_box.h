
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

	void SetSelectedItem(INT nItem, BOOL bRepaint = TRUE);

	void InsertBefore(INT nIndex, const CBofString &cString, BOOL bRepaint = TRUE);
	void InsertAfter(INT nIndex, const CBofString &cString, BOOL bRepaint = TRUE);

	void AddToHead(const CBofString &cString, BOOL bRepaint = TRUE);
	void AddToTail(const CBofString &cString, BOOL bRepaint = TRUE);

	ERROR_CODE DelItem(INT nIndex, BOOL bRepaint = TRUE);
	ERROR_CODE DeleteAll(BOOL bRepaint = TRUE);

	INT GetNumItems() {
		return (m_nNumItems);
	}
	CBofString GetText(INT nIndex);
	void SetText(INT nIndex, const CBofString &cStr);

	void SetTextLineColor(INT nIndex, RGBCOLOR rgbColor);

	ERROR_CODE LineUp() {
		return (ScrollUp(1));
	}
	ERROR_CODE LineDown() {
		return (ScrollDown(1));
	}

	ERROR_CODE PageUp() {
		return (ScrollUp(m_nPageSize));
	}
	ERROR_CODE PageDown() {
		return (ScrollDown(m_nPageSize));
	}

	ERROR_CODE ScrollUp(const INT nLines);
	ERROR_CODE ScrollDown(const INT nLines) {
		return (ScrollUp(-nLines));
	}

	ERROR_CODE ScrollTo(const INT nLine);

	ERROR_CODE CreateWorkArea();
	ERROR_CODE SaveBackground();
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

	void SetPointSize(INT nSize) {
		m_nTextSize = nSize;
	}
	INT GetPointSize() {
		return (m_nTextSize);
	}

	void SetWeight(INT nWeight) {
		m_nTextWeight = nWeight;
	}
	INT GetWeight() {
		return (m_nTextWeight);
	}

	void SetItemHeight(INT nHeight) {
		m_nItemHeight = nHeight;
	}
	INT GetItemHeight() {
		return (m_nItemHeight);
	}

	void SetFont(INT nFont) {
		m_nTextFont = nFont;
	}
	INT GetFont() {
		return (m_nTextFont);
	}

	INT GetState() {
		return (m_nState);
	}

	virtual ERROR_CODE RepaintItem(INT nIndex);
	virtual ERROR_CODE RepaintAll();

protected:
	virtual void OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnLButtonDblClk(uint32 nFlags, CBofPoint *pPoint);
	virtual void OnKeyHit(uint32 lKey, uint32 lRepCount);
	virtual void OnPaint(CBofRect *pRect);

	virtual void ClearSelection();

	CBofList<ListBoxItem> m_cTextItems;
	CBofBitmap *m_pWork;
	INT m_nNumItems;
	INT m_n1stVisible;

	INT m_nPageSize;

	INT m_nTextSize;
	INT m_nTextWeight;
	RGBCOLOR m_cTextColor;
	RGBCOLOR m_cHighColor;
	INT m_nSelectedItem;
	INT m_nItemHeight;
	INT m_nTextFont;
	INT m_nState;
};

} // namespace Bagel

#endif
