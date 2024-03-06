
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
#include "bagel/boflib/llist.h"
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

	VOID SetSelectedItem(INT nItem, BOOL bRepaint = TRUE);

	VOID InsertBefore(INT nIndex, const CBofString &cString, BOOL bRepaint = TRUE);
	VOID InsertAfter(INT nIndex, const CBofString &cString, BOOL bRepaint = TRUE);

	VOID AddToHead(const CBofString &cString, BOOL bRepaint = TRUE);
	VOID AddToTail(const CBofString &cString, BOOL bRepaint = TRUE);

	ERROR_CODE DelItem(INT nIndex, BOOL bRepaint = TRUE);
	ERROR_CODE DeleteAll(BOOL bRepaint = TRUE);

	INT GetNumItems(VOID) {
		return (m_nNumItems);
	}
	CBofString GetText(INT nIndex);
	VOID SetText(INT nIndex, const CBofString &cStr);

	VOID SetTextLineColor(INT nIndex, RGBCOLOR rgbColor);

	ERROR_CODE LineUp(VOID) {
		return (ScrollUp(1));
	}
	ERROR_CODE LineDown(VOID) {
		return (ScrollDown(1));
	}

	ERROR_CODE PageUp(VOID) {
		return (ScrollUp(m_nPageSize));
	}
	ERROR_CODE PageDown(VOID) {
		return (ScrollDown(m_nPageSize));
	}

	ERROR_CODE ScrollUp(const INT nLines);
	ERROR_CODE ScrollDown(const INT nLines) {
		return (ScrollUp(-nLines));
	}

	ERROR_CODE ScrollTo(const INT nLine);

	ERROR_CODE CreateWorkArea(VOID);
	ERROR_CODE SaveBackground(VOID);
	VOID KillBackground(VOID);

	VOID SetHighlightColor(RGBCOLOR cHighColor) {
		m_cHighColor = cHighColor;
	}
	RGBCOLOR GetHighlightColor(VOID) {
		return (m_cHighColor);
	}

	VOID SetTextColor(RGBCOLOR cColor) {
		m_cTextColor = cColor;
	}
	RGBCOLOR GetTextColor(VOID) {
		return (m_cTextColor);
	}

	VOID SetPointSize(INT nSize) {
		m_nTextSize = nSize;
	}
	INT GetPointSize(VOID) {
		return (m_nTextSize);
	}

	VOID SetWeight(INT nWeight) {
		m_nTextWeight = nWeight;
	}
	INT GetWeight(VOID) {
		return (m_nTextWeight);
	}

	VOID SetItemHeight(INT nHeight) {
		m_nItemHeight = nHeight;
	}
	INT GetItemHeight(VOID) {
		return (m_nItemHeight);
	}

	VOID SetFont(INT nFont) {
		m_nTextFont = nFont;
	}
	INT GetFont(VOID) {
		return (m_nTextFont);
	}

	INT GetState(VOID) {
		return (m_nState);
	}

	virtual ERROR_CODE RepaintItem(INT nIndex);
	virtual ERROR_CODE RepaintAll(VOID);

protected:
	virtual VOID OnLButtonDown(UINT nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual VOID OnLButtonDblClk(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnKeyHit(ULONG lKey, ULONG lRepCount);
	virtual VOID OnPaint(CBofRect *pRect);

	virtual VOID ClearSelection();

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
