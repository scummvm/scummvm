
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

#ifndef BAGEL_BAGLIB_EXPRESSION_H
#define BAGEL_BAGLIB_EXPRESSION_H

#include "bagel/spacebar/baglib/parse_object.h"
#include "bagel/spacebar/baglib/var.h"
#include "bagel/spacebar/boflib/list.h"

namespace Bagel {
namespace SpaceBar {

class CBagExpression : public CBagParseObject, public CBofObject {
public:
	enum OPERATION {
		OP_NONE,
		OP_ASSIGN,
		OP_EQUAL,
		OP_NOT_EQUAL,
		OP_LESS_THAN,
		OP_LESS_THAN_EQUAL,
		OP_GREATER_THAN,
		OP_GREATER_THAN_EQUAL,
		OP_PLUS_ASSIGN,
		OP_MINUS_ASSIGN,
		OP_CONTAINS,
		OP_HAS,
		OP_CURR_SDEV,
		OP_PLUS,
		OP_MINUS,
		OP_MULTIPLY,
		OP_DIVIDE,
		OP_AND,
		OP_OR,
		OP_MOD,
		OP_STATUS
	};

private:
	CBofList<CBagVar *> _varList;    // Right hand operator
	CBofList<OPERATION> _operList;   // Operation to be preformed
	CBagExpression *_prevExpression; // Not null when when this is an enclosed expression

	bool _prevNegativeFl; // True if the operation should return Negative results
	bool _negativeFl;     // True if the operation should return Negative results

	bool evaluate(CBagVar *leftHandOper, CBagVar *rightHandOper, OPERATION oper, CBagVar &result);

public:
	static CBagVar *_tempVar; // Used as a default param
	static void initialize();
	static void shutdown();

	CBagExpression(CBagExpression *prevExpr = nullptr, bool prevNegFl = false);
	virtual ~CBagExpression();

	bool evaluate(bool negFl = false, CBagVar &result = *_tempVar);

	/**
	 * Same as evaluate above except it evaluates left to right and preserves
	 * the value of the previous evaluation in the loop.  Provides partial
	 * evaluation hierarchy support.
	 */
	bool evalLeftToRight(bool negFl = false, CBagVar &result = *_tempVar);

	bool negEvaluate(CBagVar &result = *_tempVar);

	void setNegative(bool b = true) {
		_negativeFl = b;
	}
	bool isNegative() const {
		return _negativeFl;
	}

	CBagVar *getVariable(int itemPos);
	OPERATION getOperation(int itemPos);

	ParseCodes setInfo(CBagIfstream &istr) override;
	ErrorCode getOperatorFromStream(CBagIfstream &istr, OPERATION &oper);

	CBagExpression *getPrevExpression() const {
		return _prevExpression;
	}
	void setPrevExpression(CBagExpression *expr) {
		_prevExpression = expr;
	}

	virtual bool onAssign(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onEqual(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onNotEqual(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onLessThan(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onGreaterThan(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onLessThanEqual(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onGreaterThanEqual(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onPlusAssign(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onMinusAssign(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onContains(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onHas(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onCurrSDev(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onPlus(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onMinus(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onMultiply(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onDivide(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onMod(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onAnd(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onOr(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
	virtual bool onStatus(CBagVar *leftHandOper, CBagVar *rightHandOper, CBagVar &resultOper);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
