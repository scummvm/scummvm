
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

#ifndef BAGEL_BAGLIB_DOSSIER_OBJECT_H
#define BAGEL_BAGLIB_DOSSIER_OBJECT_H

#include "bagel/baglib/text_object.h"
#include "bagel/baglib/log_msg.h"

namespace Bagel {

class CBagDossierObject : public CBagTextObject {
private:
	CBofString m_sIndexLine;
	CBofRect m_cIndexRect;
	CBofRect m_cDossierRect;
	CBagObject *m_pRPObj;

	BOOL m_bDossierSet : 1;
	BOOL m_bNotActive : 1;
	BOOL m_bShowIndex : 1;
	BOOL m_bDosRectInit : 1;

public:
	CBofString m_sSuspectVar;

public:
	CBagDossierObject();
	virtual ~CBagDossierObject();

	/**
	 * Takes in info and then removes the relative information and returns
	 * the info without the relevant info.
	 */
	PARSE_CODES SetInfo(bof_ifstream &istr);

	ERROR_CODE Attach();
	ERROR_CODE Detach();

	virtual ERROR_CODE Update(CBofBitmap *, CBofPoint, CBofRect * = nullptr, INT = -1);

	virtual void OnLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr); //{ return FALSE; } run menu if available

	/**
	 * Arrange the float rects for the script rpo objects
	 */
	CBofPoint ArrangeFloater(CBofPoint nPos, CBagObject *pObj);
	CBofString &GetDossierLine() {
		return m_sIndexLine;
	}
	CBofRect GetRect();

	void SetNotActive(BOOL b) {
		m_bNotActive = b;
	}
	BOOL GetNotActive() {
		return m_bNotActive;
	}

	void ActivateDosObject(CBagLog *pLogWld);
	void DeactivateDosObject(CBagLog *pLogWld);

	void SetRPObj(CBagObject *p) {
		m_pRPObj = p;
	};
	CBagObject *GetRPObj() {
		return m_pRPObj;
	};

	static void DeactivateCurDossier();
	void ShowDosText();

	static void initialize();
	static CBagDossierObject *m_pCurDossier;
};

} // namespace Bagel

#endif
