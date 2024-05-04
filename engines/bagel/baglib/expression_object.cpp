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
#include "bagel/baglib/storage_dev_win.h"

namespace Bagel {

CBagExpressionObject::CBagExpressionObject() : CBagObject() {
	m_xObjType = EXPRESSOBJ;
	_expression = nullptr;
	setConditional(false);
	SetVisible(false);
	SetTimeless(true);
}

CBagExpressionObject::~CBagExpressionObject() {
	if (_expression != nullptr) {
		delete _expression;
		_expression = nullptr;
	}
	CBagObject::detach();
}

bool CBagExpressionObject::runObject() {
	if (_expression != nullptr) {
		CBagVar localVar;
		_expression->evaluate(false, localVar);

		if (!isConditional()) {
			if (getFileName().isEmpty())
				return false;

			int nIndex = getFileName().Find("~~");
			if (nIndex > 0) {
				// This is a reference
				CBofString objectStr = getFileName().Left(nIndex);
				CBofString propertyStr = getFileName().Mid(nIndex + 2);

				SDEV_MANAGER->SetObjectValue(objectStr, propertyStr, localVar.GetNumValue());

			} else {
				CBagVar *pVar = VAR_MANAGER->GetVariable(getFileName());
				if (pVar)
					pVar->SetValue(localVar.GetValue());
			}
		}
	}

	return CBagObject::runObject();
}

PARSE_CODES CBagExpressionObject::setInfo(CBagIfstream &istr) {
	bool objectUpdatedFl = false;

	while (!istr.eof()) {
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  AS  - n number of slides in sprite
		//
		case '(': {
			_expression = new CBagExpression();
			if (_expression) {
				_expression->setInfo(istr);
				objectUpdatedFl = true;
			} else {
				// there was an error
			}
		} break;
		//
		//  No match return from function
		//
		default: {
			PARSE_CODES rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				objectUpdatedFl = true;
			} else { // rc==UNKNOWN_TOKEN
				if (objectUpdatedFl)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
		}
		break;
		}
	}
	return PARSING_DONE;
}

} // namespace Bagel
