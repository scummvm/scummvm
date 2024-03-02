
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
	enum OPERATION { NONE,
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
					 STATUS };
private:
	CBofList<CBagVar *> m_cVarList;    // Right hand operator
	CBofList<OPERATION> m_cOperList;   // Operation to be preformed
	CBagExpression *m_xPrevExpression; // Not null when when this is an enclosed expression

	UBYTE m_bPrevNegative; // True if the operation should return Negative results
	UBYTE m_bNegative;     // True if the operation should return Negative results

	BOOL Evaluate(CBagVar *xLHOper, CBagVar *xRHOper, OPERATION xOper, CBagVar &xResult);

public:
	static CBagVar m_xTempVar; // used as a default param

	// mdm else/if bug 5/28 -- added member to track prev sign
	CBagExpression(CBagExpression *pPrevExpr = NULL, BOOL bPrevNeg = FALSE);
	~CBagExpression();

	BOOL Evaluate(BOOL bNeg = FALSE, CBagVar &xResult = m_xTempVar);
	BOOL EvalLeftToRight(BOOL bNeg = FALSE, CBagVar &xResult = m_xTempVar);

	BOOL NegEvaluate(CBagVar &xResult = m_xTempVar);

	VOID SetNegative(BOOL b = TRUE) { m_bNegative = (UBYTE)b; }
	BOOL IsNegative() { return (m_bNegative); }

	CBagVar *GetVariable(int nPos);
	OPERATION GetOperation(int nPos);

	PARSE_CODES SetInfo(bof_ifstream &istr);
	ERROR_CODE GetOperatorFromStream(bof_ifstream &istr, OPERATION &xOper);

	CBagExpression *GetPrevExpression() { return m_xPrevExpression; }
	VOID SetPrevExpression(CBagExpression *pExpr) { m_xPrevExpression = pExpr; }

	virtual BOOL OnAssign(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnNotEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnLessThan(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnGreaterThan(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnLessThanEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnGreaterThanEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnPlusAssign(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnMinusAssign(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnContains(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnHas(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnCurrSDev(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnPlus(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnMinus(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnMultiply(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnDivide(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnMod(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnAnd(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnOr(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
	virtual BOOL OnStatus(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper);
};

} // namespace Bagel

#endif
