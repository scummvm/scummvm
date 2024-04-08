
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

#include "bagel/baglib/parse_object.h"
#include "bagel/baglib/var.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/list.h"

namespace Bagel {

class CBagExpression : public CBagParseObject, public CBofObject {
public:
	enum OPERATION {
		NONE,
		ASSIGN,
		EQUAL,
		NOTEQUAL,
		LESSTHAN,
		LESSTHANEQUAL,
		GREATERTHAN,
		GREATERTHANEQUAL,
		PLUSASSIGN,
		MINUSASSIGN,
		CONTAINS,
		HAS,
		CURRSDEV,
		PLUS,
		MINUS,
		MULTIPLY,
		DIVIDE,
		AND,
		OR,
		MOD,
		STATUS
	};

private:
	CBofList<CBagVar *> m_cVarList;    // Right hand operator
	CBofList<OPERATION> m_cOperList;   // Operation to be preformed
	CBagExpression *m_xPrevExpression; // Not null when when this is an enclosed expression

	byte m_bPrevNegative; // True if the operation should return Negative results
	byte m_bNegative;     // True if the operation should return Negative results

	bool Evaluate(CBagVar *xLHOper, CBagVar *xRHOper, OPERATION xOper, CBagVar &xResult);

public:
	static CBagVar m_xTempVar; // used as a default param

	CBagExpression(CBagExpression *pPrevExpr = nullptr, bool bPrevNeg = FALSE);
	virtual ~CBagExpression();

	bool Evaluate(bool bNeg = FALSE, CBagVar &xResult = m_xTempVar);

	/**
	 * Same as evaluate above except it evaluates left to right and preserves
	 * the value of the previous evaluation in the loop.  Provides partial
	 * evaluation hierarchy support.
	 */
	bool EvalLeftToRight(bool bNeg = FALSE, CBagVar &xResult = m_xTempVar);

	bool NegEvaluate(CBagVar &xResult = m_xTempVar);

	void SetNegative(bool b = TRUE) {
		m_bNegative = (byte)b;
	}
	bool IsNegative() {
		return m_bNegative;
	}

	CBagVar *GetVariable(int nPos);
	OPERATION GetOperation(int nPos);

	PARSE_CODES SetInfo(bof_ifstream &istr);
	ERROR_CODE GetOperatorFromStream(bof_ifstream &istr, OPERATION &xOper);

	CBagExpression *GetPrevExpression() {
		return m_xPrevExpression;
	}
	void SetPrevExpression(CBagExpression *pExpr) {
		m_xPrevExpression = pExpr;
	}

	virtual bool OnAssign(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnNotEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnLessThan(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnGreaterThan(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnLessThanEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnGreaterThanEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnPlusAssign(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnMinusAssign(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnContains(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnHas(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnCurrSDev(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnPlus(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnMinus(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnMultiply(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnDivide(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnMod(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnAnd(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnOr(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual bool OnStatus(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
};

} // namespace Bagel

#endif
