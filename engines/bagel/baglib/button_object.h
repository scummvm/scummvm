
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

#define SAVE_BTN 0
#define CANCEL_BTN 1
#define RESTORE_BTN 2
#define RESTART_BTN 3
#define QUIT_BTN 4

#define LINEUP_BTN 10
#define LINEDN_BTN 11
#define PAGEUP_BTN 12
#define PAGEDN_BTN 13

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
	BagFuncPtr _callbackFct;
	void *_callbackInfo; // Data to be passed to the function

public:
	CBagButtonObject();
	virtual ~CBagButtonObject();

	ErrorCode attach() override;
	ErrorCode detach() override;

	BUTTON_TYPE getButtonType() const {
		return _buttonType;
	}

	/**
	 * Takes in info and then removes the relative information and returns
	 * the info without the relevant info.
	 */
	ParseCodes setInfo(CBagIfstream &istr) override;
	bool runObject() override;

	// Callback function functionality - probably can be phased out
	void setCallBack(BagFuncPtr func, void *funcInfo) {
		_callbackFct = func;
		_callbackInfo = funcInfo;
	}

	BagFuncPtr getCallBack() override {
		return _callbackFct;
	}

	void *getCallBackInfo() const {
		return _callbackInfo;
	}

	bool runCallBack() override {
		if (_callbackFct) {
			_callbackFct((int)getRefId(), _callbackInfo);
			return true;
		}

		return false;
	}

	void onLButtonDown(uint32 /*nFlags*/, CBofPoint *point, void *) override;
	void onLButtonUp(uint32 flags, CBofPoint *point, void *extraInfo) override;
	bool onMouseMove(uint32 /*nFlags*/, CBofPoint point, void *extraInfo) override;

	ErrorCode update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect = nullptr, int maskColor = -1) override;

	void setSize(const CBofSize &size) override;

	void setProperty(const CBofString &prop, int val) override;
	int getProperty(const CBofString &prop) override;
};

} // namespace Bagel

#endif
