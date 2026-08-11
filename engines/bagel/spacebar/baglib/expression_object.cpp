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

#include "bagel/spacebar/baglib/expression_object.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/baglib/var.h"
#include "bagel/spacebar/baglib/storage_dev_win.h"

namespace Bagel {
namespace SpaceBar {

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

			const int nIndex = getFileName().find("~~");
			if (nIndex > 0) {
				// This is a reference
				const CBofString objectStr = getFileName().left(nIndex);
				const CBofString propertyStr = getFileName().mid(nIndex + 2);

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
		const char ch = (char)istr.peek();
		if (ch == '(') {
			//
			//  AS  - n number of slides in sprite
			//
			_expression = new CBagExpression();
			_expression->setInfo(istr);
			objectUpdatedFl = true;
		} else {
			const ParseCodes parseCode = CBagObject::setInfo(istr);
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
	}
	return PARSING_DONE;
}

} // namespace SpaceBar
} // namespace Bagel
