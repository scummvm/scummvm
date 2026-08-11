
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

#ifndef BAGEL_BAGLIB_TEXT_OBJECT_H
#define BAGEL_BAGLIB_TEXT_OBJECT_H

#include "bagel/spacebar/baglib/object.h"
#include "bagel/spacebar/boflib/gfx/text.h"

namespace Bagel {
namespace SpaceBar {

// local prototypes...
int MapFont(int nFont);

/**
 * CBagTextObject is an object that can be place within the slide window.
 */
class CBagTextObject : public CBagObject {
private:
	CBofString *_psInitInfo; // Aux info

	COLORREF _nFGColor;

	int16 _nDX;
	int16 _nDY;

	byte _nPointSize;
	int _nTextFont;

	CBagObject *_pRPObject;

protected:
	bool _bCaption : true;
	bool _bTitle : true;
	bool _bReAttach : true;
	CBofString *_psText;

public:
	CBagTextObject();
	virtual ~CBagTextObject();

	CBofRect getRect() override;

	CBofSize getSize() const {
		return CBofSize(_nDX, _nDY);
	}
	void setSize(const CBofSize &xSize) override {
		_nDX = (int16)xSize.cx;
		_nDY = (int16)xSize.cy;
	}

	int getPointSize() const {
		return _nPointSize;
	}
	void setPointSize(int xSize) {
		assert(xSize >= 0 && xSize <= 255);
		_nPointSize = (byte)xSize;
	}
	int getColor() const {
		return 0;
	}
	void setColor(int nColor);

	// Font support for text objects
	int getFont() const {
		return _nTextFont;
	}
	void setFont(int nFont) {
		_nTextFont = nFont;
	}

	bool runObject() override;

	const CBofString *getInitInfo() const override {
		return _psInitInfo;
	}
	void setInitInfo(const CBofString &info) override {
		delete _psInitInfo;
		_psInitInfo = new CBofString();
		*_psInitInfo = info;
	}

	CBofString *getPSText() const {
		return _psText;
	}
	void setPSText(CBofString *p);

	const CBofString &getText();
	void setText(const CBofString &s);

	/**
	 * Calculate the required bounds to display text
	 */
	void recalcTextRect(bool bTextFromFile);

	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = nullptr, int /*nMaskColor*/ = -1) override;

	ErrorCode attach() override;
	ErrorCode detach() override;

	ParseCodes setInfo(CBagIfstream &istr) override;

	void setProperty(const CBofString &sProp, int nVal) override;
	int getProperty(const CBofString &sProp) override;

	bool isCaption() const {
		return _bCaption;
	}

	void setTitle(bool b = true) {
		_bTitle = b;
	}
	bool isTitle() const {
		return _bTitle;
	}

	// Extra stuff to handle mouse downs on floaters in the log pda/residue printing code.
	void onLButtonUp(uint32, CBofPoint *, void * = nullptr) override;

	void setRPObject(CBagObject *prp) {
		_pRPObject = prp;
	}
	CBagObject *getRPObject() const {
		return _pRPObject;
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
