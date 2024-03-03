
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
		PUSH = 0,
		CHECKBOX = 1,
		HLEVER = 2,
		VLEVER = 3,
		DIAL = 4,
		SLIDER = 5
	};

private:
	BOOL m_bDragging;
	CBofRect m_SlideRect;
	INT m_nNumPos;

	BUTTON_TYPE m_xButtonType;
	BOOL m_bActiveDown;
	BOOL m_bActiveUp;
	BOOL m_bActive;

	CBofPoint m_MidPoint;

#if BOF_MAC && __POWERPC__
	UniversalProcPtr m_vpFunc; // Function
#else
	void *(*m_vpFunc)(int, void *); // Function
#endif
	void *m_vpFuncInfo; // Data to be passed to the function

public:
	CBagButtonObject();
	virtual ~CBagButtonObject();

	// Return TRUE if the Object had members that are properly initialized/de-initialized
	ERROR_CODE Attach();
	ERROR_CODE Detach();

	BUTTON_TYPE GetButtonType() { return m_xButtonType; }

	/**
	 * Takes in info and then removes the relative information and returns
	 * the info without the relevant info.
	 */
	PARSE_CODES SetInfo(bof_ifstream & /*istr*/);
	virtual BOOL RunObject();

	// Callback function functionality - probably can be phased out
	VOID SetCallBack(void *(func)(int, void *), void *vpFuncInfo = nullptr) {
#if BOF_MAC && __POWERPC__
		m_vpFunc = NewRoutineDescriptor((ProcPtr)func,
			uppCBagObjectProcInfo, GetCurrentArchitecture());
		m_vpFuncInfo = vpFuncInfo;
	}
	virtual UniversalProcPtr GetCallBack() { return m_vpFunc; }
#else
		m_vpFunc = func;
		m_vpFuncInfo = vpFuncInfo;
	}
	virtual BAGFUNCPTR GetCallBack() { return m_vpFunc; }
#endif
	VOID *GetCallBackInfo() { return m_vpFuncInfo; }
	virtual BOOL RunCallBack() {
		if (m_vpFunc) {
#if BOF_MAC && __POWERPC__
			CallUniversalProc(m_vpFunc, uppCBagObjectProcInfo,
				(INT)GetRefId(), m_vpFuncInfo);
#else
			m_vpFunc((INT)GetRefId(), m_vpFuncInfo);
#endif
			return TRUE;
		} else
			return FALSE;
	}

	BOOL OnLButtonDown(UINT /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr);
	BOOL OnLButtonUp(UINT /*nFlags*/, CBofPoint /*xPoint*/, void * = nullptr);
	BOOL OnMouseMove(UINT /*nFlags*/, CBofPoint /*xPoint*/, void *);

	ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, INT /*nMaskColor*/ = -1);

	void SetSize(const CBofSize &xSize);

	VOID SetProperty(const CBofString &sProp, int nVal);
	INT GetProperty(const CBofString &sProp);
};

} // namespace Bagel

#endif
