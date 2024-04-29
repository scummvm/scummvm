
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

	void setSelectedItem(int nItem, bool bRepaint = true);

	void insertBefore(int nIndex, const CBofString &cString, bool bRepaint = true);
	void insertAfter(int nIndex, const CBofString &cString, bool bRepaint = true);

	void addToHead(const CBofString &cString, bool bRepaint = true);
	void addToTail(const CBofString &cString, bool bRepaint = true);

	ErrorCode delItem(int nIndex, bool bRepaint = true);
	ErrorCode deleteAll(bool bRepaint = true);

	int getNumItems() {
		return m_nNumItems;
	}
	CBofString getText(int nIndex);
	void setText(int nIndex, const CBofString &cStr);

	void setTextLineColor(int nIndex, RGBCOLOR rgbColor);

	ErrorCode lineUp() {
		return scrollUp(1);
	}
	ErrorCode lineDown() {
		return scrollDown(1);
	}

	ErrorCode pageUp() {
		return scrollUp(m_nPageSize);
	}
	ErrorCode pageDown() {
		return scrollDown(m_nPageSize);
	}

	ErrorCode scrollUp(const int nLines);
	ErrorCode scrollDown(const int nLines) {
		return scrollUp(-nLines);
	}

	ErrorCode scrollTo(const int nLine);

	ErrorCode createWorkArea();
	ErrorCode saveBackground();
	void killBackground();

	void setHighlightColor(RGBCOLOR cHighColor) {
		m_cHighColor = cHighColor;
	}
	RGBCOLOR getHighlightColor() {
		return m_cHighColor;
	}

	void setTextColor(RGBCOLOR cColor) {
		m_cTextColor = cColor;
	}
	RGBCOLOR getTextColor() {
		return m_cTextColor;
	}

	void setPointSize(int nSize) {
		m_nTextSize = nSize;
	}
	int getPointSize() {
		return m_nTextSize;
	}

	void setWeight(int nWeight) {
		m_nTextWeight = nWeight;
	}
	int getWeight() {
		return m_nTextWeight;
	}

	void setItemHeight(int nHeight) {
		m_nItemHeight = nHeight;
	}
	int getItemHeight() {
		return m_nItemHeight;
	}

	void setFont(int nFont) {
		m_nTextFont = nFont;
	}
	int getFont() {
		return m_nTextFont;
	}

	int getState() {
		return m_nState;
	}

	virtual ErrorCode repaintItem(int nIndex);
	virtual ErrorCode repaintAll();

protected:
	virtual void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onLButtonDblClk(uint32 nFlags, CBofPoint *pPoint);
	virtual void onKeyHit(uint32 lKey, uint32 lRepCount);
	virtual void onPaint(CBofRect *pRect);

	/**
	 * Clears the currently selected item
	 */
	virtual void clearSelection();

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
