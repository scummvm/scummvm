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

#include "bagel/baglib/expression.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/sound_object.h"
#include "bagel/boflib/sound.h"
#include "bagel/bagel.h"

namespace Bagel {

CBagVar *CBagExpression::_tempVar;	// Used as a default param

void CBagExpression::initialize() {
	// used as a default param
	_tempVar = new CBagVar("CBagExpr::TempVar", "", false);
}

void CBagExpression::shutdown() {
	delete _tempVar;
}

CBagExpression::CBagExpression(CBagExpression *prevExpr, bool prevNegFl) {
	_negativeFl = false;
	_prevExpression = prevExpr;
	_prevNegativeFl = prevNegFl;
}

CBagExpression::~CBagExpression() {
}

bool CBagExpression::evaluate(CBagVar *leftHandOper, CBagVar *rightHandOper, OPERATION oper, CBagVar &result) {
	bool retVal = false;

	// If the variable is named "RANDOM", generate a random number for its value
	if (leftHandOper->getName() == "RANDOM")
		leftHandOper->setValue(g_engine->getRandomNumber());
	if (rightHandOper->getName() == "RANDOM")
		rightHandOper->setValue(g_engine->getRandomNumber());

	switch (oper) {
	case OP_NONE:
		break;

	case OP_ASSIGN:
		retVal = onAssign(leftHandOper, rightHandOper, result);
		break;

	case OP_EQUAL:
		retVal = onEqual(leftHandOper, rightHandOper, result);
		break;

	case OP_NOT_EQUAL:
		retVal = onNotEqual(leftHandOper, rightHandOper, result);
		break;

	case OP_LESS_THAN:
		retVal = onLessThan(leftHandOper, rightHandOper, result);
		break;

	case OP_LESS_THAN_EQUAL:
		retVal = onLessThanEqual(leftHandOper, rightHandOper, result);
		break;

	case OP_GREATER_THAN:
		retVal = onGreaterThan(leftHandOper, rightHandOper, result);
		break;

	case OP_GREATER_THAN_EQUAL:
		retVal = onGreaterThanEqual(leftHandOper, rightHandOper, result);
		break;

	case OP_PLUS_ASSIGN:
		retVal = onPlusAssign(leftHandOper, rightHandOper, result);
		break;

	case OP_MINUS_ASSIGN:
		retVal = onMinusAssign(leftHandOper, rightHandOper, result);
		break;

	case OP_PLUS:
		retVal = onPlus(leftHandOper, rightHandOper, result);
		break;

	case OP_MINUS:
		retVal = onMinus(leftHandOper, rightHandOper, result);
		break;

	case OP_MULTIPLY:
		retVal = onMultiply(leftHandOper, rightHandOper, result);
		break;

	case OP_DIVIDE:
		retVal = onDivide(leftHandOper, rightHandOper, result);
		break;

	case OP_MOD:
		retVal = onMod(leftHandOper, rightHandOper, result);
		break;

	case OP_CONTAINS:
		retVal = onContains(leftHandOper, rightHandOper, result);
		break;

	case OP_HAS:
		retVal = onHas(leftHandOper, rightHandOper, result);
		break;

	case OP_STATUS:
		retVal = onStatus(leftHandOper, rightHandOper, result);
		break;

	default:
		break;
	}

	return retVal;
}


CBagVar *CBagExpression::getVariable(int itemPos) {
	CBagVar *curVar = _varList.getNodeItem(itemPos);

	// If the variable is a reference (OBJ.PROPERTY)
	if (curVar->isReference()) {
		char frontStr[256];
		Common::strcpy_s(frontStr, curVar->getName());

		char *p = strstr(frontStr, "~~");
		if (p != nullptr) {
			char backStr[256];
			Common::strcpy_s(backStr, p + 2);
			*p = '\0';

			const CBofString stringObject(frontStr, 256);
			const CBofString stringProperty(backStr, 256);

			const int newVal = g_SDevManager->getObjectValue(stringObject, stringProperty);
			curVar->setValue(newVal);
		}
	}

	return curVar;
}


CBagExpression::OPERATION CBagExpression::getOperation(int itemPos) {
	assert(false);
	return _operList.getNodeItem(itemPos);
}


bool CBagExpression::evaluate(bool negFl, CBagVar &result) {
	bool retVal = false;

	// There must be an expression for every variable after the first
	assert(_varList.getCount() - 1 == _operList.getCount());

	int count = 0;

	CBagVar *leftHandOper = getVariable(count++);
	result = *leftHandOper;

	bool parentCheckFl = true;
	if (_prevExpression) {
		parentCheckFl = _prevExpression->evaluate(_prevNegativeFl, result);
	}

	if (parentCheckFl) {
		bool subValFl;
		int nodeCount = 0;

		while (count < _varList.getCount()) {
			CBagVar *rightHandOper = getVariable(count++);
			OPERATION oper = _operList.getNodeItem(nodeCount++);
			CBagVar *rightHandOper2;

			switch (oper) {
			case OP_AND:
				rightHandOper2 = getVariable(count++);
				oper = _operList.getNodeItem(nodeCount++);
				subValFl = evaluate(rightHandOper, rightHandOper2, oper, result);

				retVal &= subValFl;
				break;

			case OP_OR:
				rightHandOper2 = getVariable(count++);
				oper = _operList.getNodeItem(nodeCount++);
				subValFl = evaluate(rightHandOper, rightHandOper2, oper, result);

				retVal |= subValFl;
				break;

			default:
				retVal = evaluate(leftHandOper, rightHandOper, oper, result);
				break;
			}
		}

		if (negFl)
			// Evaluate before and with parent
			retVal = !retVal;

		retVal &= parentCheckFl;
	}

	return retVal;
}

bool CBagExpression::evalLeftToRight(bool negFl, CBagVar &result) {
	bool retVal = false;
	OPERATION oper = OP_NONE;

	// There must be an expression for every variable after the first
	assert(_varList.getCount() - 1 == _operList.getCount());

	int varCount = 0;

	CBagVar *leftHandOper = getVariable(varCount++);
	result = *leftHandOper;

	bool parentCheckFl = true;
	if (_prevExpression) {
		parentCheckFl = _prevExpression->evaluate(_prevNegativeFl, result);
	}

	if (parentCheckFl) {
		bool bFirstTime = true;
		int nodeCount = 0;
		while (varCount < _varList.getCount()) {
			CBagVar compLeftHandOper;
			CBagVar *rightHandOper = getVariable(varCount++);
			const OPERATION prevOper = oper;      // save previous operator
			oper = _operList.getNodeItem(nodeCount++);

			if (bFirstTime) {
				compLeftHandOper = *leftHandOper;
				bFirstTime = false;
			} else {
				// Based on what we have for a previous operator, either use
				// the left hand expression or the result of the previous expression.
				switch (prevOper) {
				case OP_MINUS:
				case OP_MULTIPLY:
				case OP_DIVIDE:
				case OP_MOD:
				case OP_PLUS:
					compLeftHandOper = result;
					break;
				case OP_NONE:
				case OP_ASSIGN:
				case OP_EQUAL:
				case OP_NOT_EQUAL:
				case OP_LESS_THAN:
				case OP_LESS_THAN_EQUAL:
				case OP_GREATER_THAN:
				case OP_GREATER_THAN_EQUAL:
				case OP_PLUS_ASSIGN:
				case OP_MINUS_ASSIGN:
				case OP_CONTAINS:
				case OP_HAS:
				case OP_STATUS:
				default:
					compLeftHandOper = *leftHandOper;
					break;
				}
			}

			bool boolVal;
			CBagVar *rightHandOper2;
			switch (oper) {

			case OP_AND:
				rightHandOper2 = getVariable(varCount++);
				oper = _operList.getNodeItem(nodeCount++);
				boolVal = evaluate(rightHandOper, rightHandOper2, oper, result);

				retVal &= boolVal;
				break;

			case OP_OR:
				rightHandOper2 = getVariable(varCount++);
				oper = _operList.getNodeItem(nodeCount++);
				boolVal = evaluate(rightHandOper, rightHandOper2, oper, result);

				// or this don't not it!!!
				retVal |= boolVal;
				break;

			default:
				retVal = evaluate(&compLeftHandOper, rightHandOper, oper, result);
				break;
			}
		}

		if (negFl)
			// Evaluate before and with parent
			retVal = !retVal;

		retVal &= parentCheckFl;
	}

	return retVal;
}


bool CBagExpression::negEvaluate(CBagVar &result) {
	return evaluate(false, result);
}


bool CBagExpression::onAssign(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar & /* resultOper, unused*/) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	char buffer[256];
	Common::strcpy_s(buffer, rightHandOper->getValue());
	assert(strlen(buffer) < 256);
	const CBofString newLeftHandValue(buffer, 256);

	leftHandOper->setValue(newLeftHandValue);

	return true;
}


bool CBagExpression::onEqual(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	const bool retVal = leftHandOper->getValue() == rightHandOper->getValue();
	resultOper.setBoolValue(retVal);

	return retVal;
}


bool CBagExpression::onNotEqual(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));
	const bool retVal = leftHandOper->getValue() != rightHandOper->getValue();
	resultOper.setBoolValue(retVal);

	return retVal;
}


bool CBagExpression::onLessThan(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));
	const bool retVal = leftHandOper->getNumValue() < rightHandOper->getNumValue();
	resultOper.setBoolValue(retVal);
	return retVal;
}


bool CBagExpression::onGreaterThan(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));
	const bool retVal = leftHandOper->getNumValue() > rightHandOper->getNumValue();
	resultOper.setBoolValue(retVal);
	return retVal;
}


bool CBagExpression::onLessThanEqual(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));
	const bool retVal = leftHandOper->getNumValue() <= rightHandOper->getNumValue();
	resultOper.setBoolValue(retVal);
	return retVal;
}


bool CBagExpression::onGreaterThanEqual(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	const bool retVal = leftHandOper->getNumValue() >= rightHandOper->getNumValue();
	resultOper.setBoolValue(retVal);
	return retVal;
}


bool CBagExpression::onPlusAssign(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	if (leftHandOper->isNumeric() && rightHandOper->isNumeric()) {
		const int leftHandNum = leftHandOper->getNumValue();
		const int rightHandNum = rightHandOper->getNumValue();
		leftHandOper->setValue(leftHandNum + rightHandNum);
		resultOper.setValue(leftHandOper->getNumValue());
	}

	return true;
}


bool CBagExpression::onMinusAssign(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	if (leftHandOper->isNumeric() && rightHandOper->isNumeric()) {
		const int leftHandNum = leftHandOper->getNumValue();
		const int rightHandNum = rightHandOper->getNumValue();
		leftHandOper->setValue(leftHandNum - rightHandNum);
		resultOper.setValue(leftHandOper->getNumValue());
	}

	return true;
}


bool CBagExpression::onContains(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar & /* resultOper, unused */) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	CBagStorageDev *sDev = g_SDevManager->getStorageDevice(leftHandOper->getValue());
	if (sDev == nullptr)
		return false;

	CBagObject *curObj = sDev->getObject(rightHandOper->getValue());
	if ((curObj != nullptr) && curObj->isActive())
		return true;

	return false;
}

bool CBagExpression::onHas(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar & /* resultOper, unused */) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	CBagStorageDev *sDev = g_SDevManager->getStorageDevice(leftHandOper->getValue());
	if (sDev == nullptr)
		return false;

	CBagObject *curObj = sDev->getObjectByType(rightHandOper->getValue(), true);
	if (curObj == nullptr)
		return false;

	return true;
}

bool CBagExpression::onStatus(CBagVar *pLHOper, CBagVar * /* rightHandOper, unused */, CBagVar & /* resultOper, unused */) {
	assert(pLHOper != nullptr);

	CBagStorageDev *sDev = g_SDevManager->getStorageDeviceContaining(pLHOper->getValue());
	if (sDev == nullptr)
		return false;

	CBagObject *curObj = sDev->getObject(pLHOper->getValue());
	if (curObj == nullptr)
		return false;

	return false;

}
bool CBagExpression::onCurrSDev(CBagVar * /* leftHandOper, unused*/, CBagVar * /* rightHandOper, unused */, CBagVar & /* resultOper, unused */) {
	return true;
}


bool CBagExpression::onPlus(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	if (leftHandOper->isNumeric() && rightHandOper->isNumeric()) {
		const int leftHandNum = leftHandOper->getNumValue();
		const int rightHandNum = rightHandOper->getNumValue();
		resultOper.setValue(leftHandNum + rightHandNum);
	}

	return true;
}


bool CBagExpression::onMinus(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	if (leftHandOper->isNumeric() && rightHandOper->isNumeric()) {
		const int leftHandNum = leftHandOper->getNumValue();
		const int rightHandNum = rightHandOper->getNumValue();
		resultOper.setValue(leftHandNum - rightHandNum);
	}

	return true;
}


bool CBagExpression::onMultiply(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	if (leftHandOper->isNumeric() && rightHandOper->isNumeric()) {
		const int leftHandNum = leftHandOper->getNumValue();
		const int rightHandNum = rightHandOper->getNumValue();

		resultOper.setValue(leftHandNum * rightHandNum);
	}

	return true;
}


bool CBagExpression::onDivide(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	if (leftHandOper->isNumeric() && rightHandOper->isNumeric()) {
		const int leftHandNum = leftHandOper->getNumValue();
		const int rightHandNum = rightHandOper->getNumValue();

		// Divide by Zero error?
		assert(rightHandNum != 0);

		resultOper.setValue(leftHandNum / rightHandNum);
	}

	return true;
}


bool CBagExpression::onMod(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	if (leftHandOper->isNumeric() && rightHandOper->isNumeric()) {
		const int leftHandNum = leftHandOper->getNumValue();
		const int rightHandNum = rightHandOper->getNumValue();

		// Divide by Zero error?
		assert(rightHandNum != 0);

		resultOper.setValue(leftHandNum % rightHandNum);
	}

	return true;
}


bool CBagExpression::onAnd(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar & /* resultOper, unused */) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	return (!leftHandOper->getValue().find("true") && !rightHandOper->getValue().find("true"));
}


bool CBagExpression::onOr(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar & /* resultOper, unused */) {
	assert((leftHandOper != nullptr) && (rightHandOper != nullptr));

	return (!leftHandOper->getValue().find("true") || !rightHandOper->getValue().find("true"));
}


ParseCodes CBagExpression::setInfo(CBagIfstream &istr) {
	char buffer[256];
	buffer[0] = 0;
	CBofString tmpStr(buffer, 256);

	ParseCodes parseCode = PARSING_DONE;
	bool doneFl = false;

	while (!doneFl && parseCode == PARSING_DONE) {
		istr.eatWhite();
		int ch = istr.peek();
		switch (ch) {
		case '(': {
			istr.getCh();
			istr.eatWhite();

			getAlphaNumFromStream(istr, tmpStr);
			CBagVar *curVar = g_VarManager->getVariable(tmpStr);
			if (!curVar) {
				// This must be a reference, make a new variable
				if (tmpStr.find("~~") > 0) {
					curVar = new CBagVar;
					curVar->setName(tmpStr);
					curVar->setReference();
				} else {
					// This is an error condition, constants can only be rhopers
					curVar = new CBagVar;
					curVar->setName(tmpStr);
					curVar->setValue(tmpStr);
					curVar->setConstant();
				}
			}
			_varList.addToTail(curVar);

			istr.eatWhite();
			ch = istr.peek();
			while ((ch != ')') && parseCode == PARSING_DONE) {
				OPERATION curOper;
				getOperatorFromStream(istr, curOper);
				if (curOper == OP_NONE) {
					parseCode = UNKNOWN_TOKEN;
					break;
				}
				_operList.addToTail(curOper);

				istr.eatWhite();
				getAlphaNumFromStream(istr, tmpStr);
				curVar = g_VarManager->getVariable(tmpStr);
				if (!curVar) {
					if (tmpStr.find("~~") > 0) {
						// This must be a reference, make a new variable
						curVar = new CBagVar;
						curVar->setName(tmpStr);
						curVar->setReference();
					} else {
						// This must be a constant, make a new variable
						curVar = new CBagVar;
						curVar->setName(tmpStr);
						curVar->setValue(tmpStr);
						curVar->setConstant();
					}
				}
				_varList.addToTail(curVar);

				istr.eatWhite();
				ch = istr.peek();
			} // while parsing inner circle

			if (ch == ')') {
				istr.getCh();
				doneFl = true;
			}
			break;
		}

		case 'N':
			getAlphaNumFromStream(istr, tmpStr);
			if (!tmpStr.find("NOT")) {
				_negativeFl = !_negativeFl;
				istr.eatWhite();
				break;
			}
			// FIXME: Is this intentional?
			// fallthrough
		default:
			parseCode = UNKNOWN_TOKEN;
			break;
		}
	}

	if (parseCode != PARSING_DONE) {
		parseAlertBox(istr, "Error in expression:", __FILE__, __LINE__);
	}

	istr.eatWhite();

	return parseCode;
}


ErrorCode CBagExpression::getOperatorFromStream(CBagIfstream &istr, OPERATION &oper) {
	ErrorCode errorCode = ERR_NONE;

	char localBuff[256];
	localBuff[0] = 0;

	CBofString localStr(localBuff, 256);

	oper = OP_NONE;

	istr.eatWhite();
	getOperStrFromStream(istr, localStr);

	if (localStr.isEmpty()) {
		getAlphaNumFromStream(istr, localStr);
		istr.eatWhite();
	}

	if (!localStr.find("-=")) {
		oper = OP_MINUS_ASSIGN;

	} else if (!localStr.find("+=")) {
		oper = OP_PLUS_ASSIGN;

	} else if (!localStr.find(">=")) {
		oper = OP_GREATER_THAN_EQUAL;

	} else if (!localStr.find("<=")) {
		oper = OP_LESS_THAN_EQUAL;

	} else if (!localStr.find("!=")) {
		oper = OP_NOT_EQUAL;

	} else if (!localStr.find("==")) {
		oper = OP_EQUAL;

	} else if (!localStr.find(">")) {
		oper = OP_GREATER_THAN;

	} else if (!localStr.find("<")) {
		oper = OP_LESS_THAN;

	} else if (!localStr.find("=")) {
		oper = OP_ASSIGN;

	} else if (!localStr.find("+")) {
		oper = OP_PLUS;

	} else if (!localStr.find("-")) {
		oper = OP_MINUS;

	} else if (!localStr.find("*")) {
		oper = OP_MULTIPLY;

	} else if (!localStr.find("/")) {
		oper = OP_DIVIDE;

	} else if (!localStr.find("%")) {
		oper = OP_MOD;

	} else if (!localStr.find("CONTAINS")) {
		// SDev contains object
		oper = OP_CONTAINS;

	} else if (!localStr.find("HAS")) {
		// SDev has type of object
		oper = OP_HAS;

	} else if (!localStr.find("OR")) {
		oper = OP_OR;

	} else if (!localStr.find("STATUS")) {
		oper = OP_STATUS;

	} else if (!localStr.find("AND")) {
		oper = OP_AND;
	}

	if (oper == OP_NONE)
		errorCode = ERR_UNKNOWN;

	return errorCode;
}

} // namespace Bagel
