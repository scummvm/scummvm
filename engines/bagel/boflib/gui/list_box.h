
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
#define COLOR_USE_DEFAULT 0xFFFFFFFF

class ListBoxItem {
public:
	ListBoxItem() {
		_pTextStr = nullptr;
		_nTextLineColor = COLOR_USE_DEFAULT;
	}
	
	
	CBofString *_pTextStr;
	RGBCOLOR _nTextLineColor;
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
		return _nNumItems;
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
		return scrollUp(_nPageSize);
	}
	ErrorCode pageDown() {
		return scrollDown(_nPageSize);
	}

	ErrorCode scrollUp(int nLines);
	ErrorCode scrollDown(const int nLines) {
		return scrollUp(-nLines);
	}

	ErrorCode scrollTo(int nLine);

	ErrorCode createWorkArea();
	ErrorCode saveBackground();
	void killBackground();

	void setHighlightColor(RGBCOLOR cHighColor) {
		_cHighColor = cHighColor;
	}
	RGBCOLOR getHighlightColor() {
		return _cHighColor;
	}

	void setTextColor(RGBCOLOR cColor) {
		_cTextColor = cColor;
	}
	RGBCOLOR getTextColor() {
		return _cTextColor;
	}

	void setPointSize(int nSize) {
		_nTextSize = nSize;
	}
	int getPointSize() {
		return _nTextSize;
	}

	void setWeight(int nWeight) {
		_nTextWeight = nWeight;
	}
	int getWeight() {
		return _nTextWeight;
	}

	void setItemHeight(int nHeight) {
		_nItemHeight = nHeight;
	}
	int getItemHeight() {
		return _nItemHeight;
	}

	void setFont(int nFont) {
		_nTextFont = nFont;
	}
	int getFont() {
		return _nTextFont;
	}

	int getState() {
		return _nState;
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

	CBofList<ListBoxItem> _cTextItems;
	CBofBitmap *_pWork;
	int _nNumItems;
	int _n1stVisible;

	int _nPageSize;

	int _nTextSize;
	int _nTextWeight;
	RGBCOLOR _cTextColor;
	RGBCOLOR _cHighColor;
	int _nSelectedItem;
	int _nItemHeight;
	int _nTextFont;
	int _nState;
};

} // namespace Bagel

#endif
