
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

#ifndef BAGEL_BAGLIB_BUTTON_OBJECT_H
#define BAGEL_BAGLIB_BUTTON_OBJECT_H

#include "bagel/baglib/sprite_object.h"

namespace Bagel {

/**
 * CBagButtonObject is an object that can be place within the slide window.
 */
class CBagButtonObject : public CBagSpriteObject {
public:
	enum BUTTON_TYPE {
		BTN_PUSH = 0,
		BTN_CHECKBOX = 1,
		BTN_HLEVER = 2,
		BTN_VLEVER = 3,
		BTN_DIAL = 4,
		BTN_SLIDER = 5
	};

private:
	bool _dragging;
	CBofRect _slideRect;
	int _numPos;

	BUTTON_TYPE _buttonType;
	bool _activeDown;
	bool _activeUp;
	bool _active;

	CBofPoint _midPoint;
	BAGFUNCPTR _callbackFct;
	void *_callbackInfo; // Data to be passed to the function

public:
	CBagButtonObject();
	virtual ~CBagButtonObject();

	ErrorCode attach();
	ErrorCode detach();

	BUTTON_TYPE getButtonType() {
		return _buttonType;
	}

	/**
	 * Takes in info and then removes the relative information and returns
	 * the info without the relevant info.
	 */
	PARSE_CODES setInfo(bof_ifstream &istr);
	virtual bool RunObject();

	// Callback function functionality - probably can be phased out
	void SetCallBack(BAGFUNCPTR func, void *funcInfo) {
		_callbackFct = func;
		_callbackInfo = funcInfo;
	}
	virtual BAGFUNCPTR getCallBack() {
		return _callbackFct;
	}

	void *GetCallBackInfo() {
		return _callbackInfo;
	}
	virtual bool RunCallBack() {
		if (_callbackFct) {
			_callbackFct((int)GetRefId(), _callbackInfo);
			return true;
		}

		return false;
	}

	void OnLButtonDown(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	void OnLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr);
	bool OnMouseMove(uint32 /*nFlags*/, CBofPoint /*xPoint*/, void *);

	ErrorCode update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect = nullptr, int /*nMaskColor*/ = -1);

	void setSize(const CBofSize &size);

	void SetProperty(const CBofString &prop, int val);
	int GetProperty(const CBofString &prop);
};

} // namespace Bagel

#endif
