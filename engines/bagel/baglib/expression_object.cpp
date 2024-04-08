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

#include "bagel/baglib/expression_object.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/var.h"

namespace Bagel {

CBagExpressionObject::CBagExpressionObject() : CBagObject() {
	m_xObjType = EXPRESSOBJ;
	m_xExpression = nullptr;
	SetConditional(FALSE);
	SetVisible(FALSE);
	SetTimeless(TRUE);
}

CBagExpressionObject::~CBagExpressionObject() {
	if (m_xExpression != nullptr) {
		delete m_xExpression;
		m_xExpression = nullptr;
	}
	Detach();
}

/*CRect CBagExpressionObject::GetRect()
{
    CPoint p = GetPosition();
    CSize  s = GetSize();
    CRect r = CRect(p,s);
    return(r);
}*/

bool CBagExpressionObject::RunObject() {
	if (m_xExpression != nullptr) {
		CBagVar xVar;
		m_xExpression->Evaluate(FALSE, xVar);

		if (!IsConditional()) {
			if (GetFileName().IsEmpty())
				return FALSE;

			int nIndex = GetFileName().Find("~~");
			if (nIndex > 0) { // this is a reference
				CBofString sObject = GetFileName().Left(nIndex);
				CBofString sProperty = GetFileName().Mid(nIndex + 2);

				SDEVMNGR->SetObjectValue(sObject, sProperty, xVar.GetNumValue());

			} else {
				CBagVar *pVar = VARMNGR->GetVariable(GetFileName());
				if (pVar)
					pVar->SetValue(xVar.GetValue());
			}
		}
	}

	return CBagObject::RunObject();
}

PARSE_CODES CBagExpressionObject::SetInfo(bof_ifstream &istr) {
	int nChanged;
	bool nObjectUpdated = FALSE;
	char ch;

	while (!istr.eof()) {
		nChanged = 0;

		switch (ch = (char)istr.peek()) {
		//
		//  AS  - n number of slides in sprite
		//
		case '(': {
			m_xExpression = new CBagExpression();
			if (m_xExpression) {
				m_xExpression->SetInfo(istr);
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				// there was an error
			}
		} break;
		//
		//  no match return from funtion
		//
		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = TRUE;
			} else if (!nChanged) { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;
				else
					return UNKNOWN_TOKEN;
			}
		}
		break;
		}
	}
	return PARSING_DONE;
}

} // namespace Bagel
