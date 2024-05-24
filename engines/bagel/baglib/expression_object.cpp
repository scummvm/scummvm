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
	_xObjType = EXPRESS_OBJ;
	_expression = nullptr;
	setConditional(false);
	setVisible(false);
	setTimeless(true);
}

CBagExpressionObject::~CBagExpressionObject() {
	delete _expression;
	_expression = nullptr;

	CBagObject::detach();
}

bool CBagExpressionObject::runObject() {
	if (_expression != nullptr) {
		CBagVar localVar;
		_expression->evaluate(false, localVar);

		if (!isConditional()) {
			if (getFileName().isEmpty())
				return false;

			int nIndex = getFileName().find("~~");
			if (nIndex > 0) {
				// This is a reference
				CBofString objectStr = getFileName().left(nIndex);
				CBofString propertyStr = getFileName().mid(nIndex + 2);

				g_SDevManager->setObjectValue(objectStr, propertyStr, localVar.getNumValue());

			} else {
				CBagVar *pVar = g_VarManager->getVariable(getFileName());
				if (pVar)
					pVar->setValue(localVar.getValue());
			}
		}
	}

	return CBagObject::runObject();
}

ParseCodes CBagExpressionObject::setInfo(CBagIfstream &istr) {
	bool objectUpdatedFl = false;

	while (!istr.eof()) {
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  AS  - n number of slides in sprite
		//
		case '(':
			_expression = new CBagExpression();
			_expression->setInfo(istr);
			objectUpdatedFl = true;
			break;
		//
		//  No match return from function
		//
		default: {
			ParseCodes parseCode = CBagObject::setInfo(istr);
			if (parseCode == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (parseCode == UPDATED_OBJECT) {
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
